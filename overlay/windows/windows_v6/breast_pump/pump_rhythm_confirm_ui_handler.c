/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/
#if PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE
#define PUMP_RHYTHM_CONFIRM_IDLE_TASK_NAME          "pump_rhythm_confirm_idle"
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE
static void pump_rhythm_confirm_idle_timeout_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        LOG_UI("rhythm confirm ui, ignore idle timeout.");
        return;
    }
    LOG_WM("rhythm confirm idle timeout, back without mode switch.");
    /// 静默返回：不落地模式，清预览页回首页
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
}
#endif

static void pump_rhythm_confirm_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_rhythm_confirm_process(ui);
    pump_pause_led_tick(proc);
#if PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_RHYTHM_CONFIRM_IDLE_TIMEOUT_MS) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("rhythm confirm idle timeout: %u", (unsigned)time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(pump_rhythm_confirm_idle_timeout_cb, proc,
            PUMP_RHYTHM_CONFIRM_IDLE_TASK_NAME);
    }
#endif
}

static void pump_rhythm_confirm_animation_progress_cb(void *user_data, int index,
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
            pump_rhythm_confirm_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_rhythm_confirm_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_rhythm_confirm_prepare(ui);
}

static void pump_rhythm_confirm_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_rhythm_confirm_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_rhythm_confirm_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        return;
    }
    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);

    ui_play_info_t play_info = {
        .tag = "pump_rhythm_confirm_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_rhythm_confirm_animation_progress_cb,
        .display_prepare_cb = pump_rhythm_confirm_ui_display_prepare,
        .display_remove_cb = pump_rhythm_confirm_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_RHYTHM_CONFIRM;
#if !PUMP_RHYTHM_CONFIRM_INVALID_EXIT_ENABLE
    /// 一次完整泵奶最多展示一次（方案1）：展示即记为已提示
    proc->rhythm_confirm_shown = 1;
#endif
    /// PUMP_RHYTHM_CONFIRM_INVALID_EXIT_ENABLE=1 时：展示不计次，
    /// 仅短按 power 确认后置位（见 pump_power_key_event_handler）
}

void pump_rhythm_confirm_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;
#if PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_RHYTHM_CONFIRM_IDLE_TASK_NAME);
#endif
}

void pump_rhythm_confirm_start_key_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        return;
    }
    /// start：退出确认页（不落地模式）并执行启停
    LOG_WM("rhythm confirm start key, exit without mode switch then toggle.");
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
    switch_pump_status_ui(proc);
}
