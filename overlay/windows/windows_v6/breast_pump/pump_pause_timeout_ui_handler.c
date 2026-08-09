/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"

#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_PAUSE_TIMEOUT_COMPLETE_TASK_NAME          "pump_pause_timeout_complete"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_pause_idle_acquire_wakelock(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pm == NULL || proc->pm->acquire_wakelock == NULL) {
        return;
    }
    if (proc->pm->acquire_wakelock(PUMP_PAUSE_IDLE_WAKELOCK_NAME, -1) == 0) {
        proc->pause_idle_wakelock_held = 1;
    }
}

static void pump_pause_idle_release_wakelock(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pause_idle_wakelock_held == 0) {
        return;
    }
    if (proc->pm && proc->pm->release_wakelock) {
        proc->pm->release_wakelock(PUMP_PAUSE_IDLE_WAKELOCK_NAME);
    }
    proc->pause_idle_wakelock_held = 0;
}

static void pump_pause_timeout_mark_session_finish(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END,
        DATA_BURY_END_REASON_TIMEOUT, 0);
    WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, 1);
    proc->pump_status = BREAST_PUMP_FINISH;
}

static void pump_pause_idle_trigger_settle(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    LOG_WM("pump pause idle timeout, show pause timeout ui. scheme:%d",
        PUMP_PAUSE_TIMEOUT_SCHEME);
    /// 先切结算页，再结束会话，避免同帧旧页把 FINISH 当成运行态画出时长
    show_pump_pause_timeout_ui(proc);
#if PUMP_PAUSE_TIMEOUT_SCHEME == PUMP_PAUSE_TIMEOUT_SCHEME_AUTO_OFF
    /// 方案1：进入结算即结束会话，展示完成后自动关机
    pump_pause_timeout_mark_session_finish(proc);
#endif
}

static void pump_pause_timeout_complete_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_PAUSE_TIMEOUT) {
        LOG_UI("pause timeout ui, ignore complete.");
        return;
    }
    LOG_WM("pump pause timeout complete, goto auto power off.");
#if PUMP_PAUSE_TIMEOUT_SCHEME == PUMP_PAUSE_TIMEOUT_SCHEME_KEY_EXIT
    /// 方案2：无按键退出时，展示完成再结束会话并关机
    pump_pause_timeout_mark_session_finish(proc);
#endif
    pump_pause_idle_stop(proc);

    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
    msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_AUTO;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void pump_pause_timeout_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_pause_timeout_process(ui);
}

static void pump_pause_timeout_animation_progress_cb(void *user_data, int index,
    ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_pause_timeout_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            POST_WM_TASK_HANDLER_NO_DELAY(pump_pause_timeout_complete_cb,
                ui->priv.play_info.user_data, PUMP_PAUSE_TIMEOUT_COMPLETE_TASK_NAME);
            break;
        default:
            break;
    }
}

static void pump_pause_timeout_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_pause_timeout_prepare(ui);
}

static void pump_pause_timeout_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_pause_timeout_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_pause_idle_start(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);

    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT) {
        return;
    }

    pump_pause_idle_acquire_wakelock(proc);
    proc->pause_idle_time = get_timestamp();
    proc->pause_idle_log_time = proc->pause_idle_time;
    LOG_WM("pump pause idle start, timeout:%ums", (unsigned)PUMP_PAUSE_IDLE_TIMEOUT_MS);
}

void pump_pause_idle_touch(ui_proc_pump_t *proc)
{
    int pump_status = 0;

    EXCEPTION_CHECK_VOID(proc && proc->pump);

    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT) {
        return;
    }
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status != BREAST_PUMP_PAUSE) {
        return;
    }
    pump_pause_idle_start(proc);
}

void pump_pause_idle_stop(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pause_idle_time != 0) {
        LOG_WM("pump pause idle stop.");
    }
    proc->pause_idle_time = 0;
    proc->pause_idle_log_time = 0;
    pump_pause_idle_release_wakelock(proc);
}

bool pump_pause_idle_timeout_handler(ui_proc_pump_t *proc)
{
    int pump_status = 0;
    uint32_t elapsed_ms;
    uint32_t remain_ms;

    EXCEPTION_CHECK_BOOL(proc && proc->pump);

    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        return false;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT) {
        return false;
    }
    if (proc->pause_idle_time == 0) {
        return false;
    }

    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status != BREAST_PUMP_PAUSE) {
        pump_pause_idle_stop(proc);
        return false;
    }

    elapsed_ms = get_timestamp_ms(proc->pause_idle_time);
    if (elapsed_ms >= PUMP_PAUSE_IDLE_TIMEOUT_MS) {
        proc->pause_idle_time = 0;
        pump_pause_idle_trigger_settle(proc);
        return true;
    }

    remain_ms = PUMP_PAUSE_IDLE_TIMEOUT_MS - elapsed_ms;
    if (proc->pause_idle_log_time == 0 ||
        get_timestamp_ms(proc->pause_idle_log_time) >= PUMP_PAUSE_IDLE_LOG_INTERVAL_MS) {
        proc->pause_idle_log_time = get_timestamp();
        LOG_WM("pump pause idle countdown remain:%u.%03us (elapsed:%u.%03us / %us) ui:%u",
            (unsigned)(remain_ms / 1000U), (unsigned)(remain_ms % 1000U),
            (unsigned)(elapsed_ms / 1000U), (unsigned)(elapsed_ms % 1000U),
            (unsigned)(PUMP_PAUSE_IDLE_TIMEOUT_MS / 1000U),
            (unsigned)proc->current_ui_type);
    }
    return false;
}

void show_pump_pause_timeout_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT) {
        return;
    }
    pump_ui_change_hdr_remove(proc);

    /// 展示结算页期间继续持锁，避免系统抢先休眠
    pump_pause_idle_acquire_wakelock(proc);
    proc->pause_idle_time = 0;
    proc->pause_idle_log_time = 0;

    /// 先切类型再改灯，避免 pause LED tick 抢控
    proc->current_ui_type = PUMP_UI_TYPE_PAUSE_TIMEOUT;
    proc->pump_light_status = PUMP_LIGHT_STATUS_DEFAULT;
    proc->pump_pause_blink_state = 0;
    proc->state_blink_visible = 0;
    if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
#if PUMP_PAUSE_TIMEOUT_SCHEME == PUMP_PAUSE_TIMEOUT_SCHEME_KEY_EXIT
        /// 方案2：按键灯全亮，提示可按键退出
        proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
#else
        /// 方案1：按键灯全灭
        proc->led->hook->set_key_backlight(proc->led, 0);
#endif
    }

    ui_play_info_t play_info = {
        .tag = "pump_pause_timeout_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_pause_timeout_animation_progress_cb,
        .display_prepare_cb = pump_pause_timeout_ui_display_prepare,
        .display_remove_cb = pump_pause_timeout_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
}

void pump_pause_timeout_ui_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_PAUSE_TIMEOUT_COMPLETE_TASK_NAME);
}

#if PUMP_PAUSE_TIMEOUT_SCHEME == PUMP_PAUSE_TIMEOUT_SCHEME_KEY_EXIT
void pump_pause_timeout_ui_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_PAUSE_TIMEOUT) {
        return;
    }

    LOG_WM("dismiss pump pause timeout ui by key, keep pause and back home.");
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_PAUSE_TIMEOUT_COMPLETE_TASK_NAME);
    pump_pause_idle_stop(proc);
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
    /// 仍处于暂停：恢复无操作倒计时，灯效交由首页 pause blink 接管
    pump_pause_idle_start(proc);
}
#endif

#endif /* PUMP_PAUSE_IDLE_TIMEOUT_ENABLE */
