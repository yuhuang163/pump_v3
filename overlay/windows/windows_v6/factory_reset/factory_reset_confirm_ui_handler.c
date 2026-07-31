/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "window_screen_factory_reset.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define FACTORY_RESET_CONFIRM_TASK_NAME             "factory_reset_confirm_task"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void factory_reset_confirm_ui_blink_update(ui_proc_factory_reset_t *proc)
{
    const uint8_t blink_visible =
        ((get_timestamp_ms(proc->blink_start_time) / FACTORY_RESET_BLINK_PERIOD) % 2U) == 0U ? 1 : 0;
    if (proc->state_blink_visible == blink_visible) {
        return;
    }
    proc->state_blink_visible = blink_visible;
}

static void factory_reset_confirm_idle_exit_cb(hdr_post_args_t *args)
{
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_FACTORY_RESET);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore confirm idle exit.");
        return;
    }
    if (proc->current_ui_type != FACTORY_RESET_UI_TYPE_FACTORY_RESET_CONFIRM) {
        return;
    }
    if (proc->popup_ui_type == ID_POPUP_UI_FACTORY_RESET_CONFIRM) {
        LOG_WM("holding confirm, ignore idle exit.");
        return;
    }
    LOG_WM("factory reset confirm timeout, request exit.");
    factory_reset_request_exit(proc);
}

static void factory_reset_confirm_idle_timeout_check(ui_proc_factory_reset_t *proc)
{
    if (proc->popup_ui_type == ID_POPUP_UI_FACTORY_RESET_CONFIRM) {
        return;
    }
    if (get_timestamp_ms(proc->last_progress_ani_time) < FACTORY_RESET_CONFIRM_TIMEOUT_MS) {
        return;
    }
    proc->last_progress_ani_time = get_timestamp();
    POST_WM_TASK_HANDLER_NO_DELAY(factory_reset_confirm_idle_exit_cb, proc,
        FACTORY_RESET_CONFIRM_TASK_NAME);
}

static void factory_reset_confirm_ui_finish_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_FACTORY_RESET);
    /// 触发清库/Flash/重启埋点 (EVT_010: 清库/Flash/重启)
    {
        data_bury_trigger_user_evt_t bury_evt = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_USER_EVT_RESET,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
    }
    factory_reset_handle(proc, 10);
}

static void factory_reset_confirm_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_FACTORY_RESET);
    factory_reset_confirm_ui_blink_update(proc);
    ui_display_factory_reset_confirm_process(ui, proc->state_blink_visible != 0);
    factory_reset_confirm_idle_timeout_check(proc);
}

static void factory_reset_confirm_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            factory_reset_confirm_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH: {
            ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)ui->priv.play_info.user_data;
            if (proc && proc->popup_ui_type == ID_POPUP_UI_FACTORY_RESET_CONFIRM) {
                factory_reset_confirm_ui_finish_policy(ui, index);
            }
            break;
        }
        default:
            break;
    }
}

static void factory_reset_confirm_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_confirm_prepare(ui);
}

static void factory_reset_confirm_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_confirm_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_factory_reset_confirm_ui(ui_proc_factory_reset_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == FACTORY_RESET_UI_TYPE_FACTORY_RESET_CONFIRM) {
        return;
    }
    proc->blink_start_time = get_timestamp();
    proc->state_blink_visible = 1;
    /// 确认页所有按键灯常亮
    factory_reset_confirm_led_restore(proc);
    ui_play_info_t play_info = {
        .tag = "factory_reset_confirm_screen",
        .fps = FACTORY_RESET_ANIMATION_FREQ,
        .progress_cb = factory_reset_confirm_animation_progress_cb,
        .display_prepare_cb = factory_reset_confirm_ui_display_prepare,
        .display_remove_cb = factory_reset_confirm_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = FACTORY_RESET_UI_TYPE_FACTORY_RESET_CONFIRM;
}

void factory_reset_confirm_led_restore(ui_proc_factory_reset_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->led);
    if (proc->led->hook && proc->led->hook->set_key_backlight) {
        proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
    } else {
        for (uint8_t i = 0; i < PUMP_LED_INDEX_MAX; i++) {
            proc->led->set_led_level(i, LED_LEVEL_MAX);
        }
    }
}

void factory_reset_confirm_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(FACTORY_RESET_CONFIRM_TASK_NAME);
}
