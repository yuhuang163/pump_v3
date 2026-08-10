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
#define PUMP_STOP_HOLD_COMPLETE_TASK_NAME           "pump_stop_hold_complete"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_stop_hold_complete_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_STOP_HOLD) {
        LOG_UI("stop hold ui, ignore complete.");
        return;
    }
    LOG_WM("pump stop hold complete, goto home pump result.");
    proc->stop_hold_holding = 0;
    proc->stop_hold_ignore_click = 1;
    DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END, DATA_BURY_END_REASON_MANUAL, 0);
    pump_complete_event_handler(proc);
}

static void pump_stop_hold_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_stop_hold_process(ui);
}

static void pump_stop_hold_animation_progress_cb(void *user_data, int index,
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
            pump_stop_hold_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            POST_WM_TASK_HANDLER_NO_DELAY(pump_stop_hold_complete_cb,
                ui->priv.play_info.user_data, PUMP_STOP_HOLD_COMPLETE_TASK_NAME);
            break;
        default:
            break;
    }
}

static void pump_stop_hold_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_stop_hold_prepare(ui);
}

static void pump_stop_hold_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_stop_hold_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_stop_hold_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        return;
    }
    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);

    ui_play_info_t play_info = {
        .tag = "pump_stop_hold_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_stop_hold_animation_progress_cb,
        .display_prepare_cb = pump_stop_hold_ui_display_prepare,
        .display_remove_cb = pump_stop_hold_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_STOP_HOLD;
    proc->stop_hold_holding = 1;
    proc->stop_hold_ignore_click = 0;
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
}

void pump_stop_hold_ui_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->stop_hold_holding = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_STOP_HOLD_COMPLETE_TASK_NAME);
}

void pump_stop_hold_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_STOP_HOLD) {
        return;
    }
    proc->stop_hold_holding = 0;
    proc->stop_hold_ignore_click = 1;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_STOP_HOLD_COMPLETE_TASK_NAME);
    LOG_WM("dismiss pump stop hold, nav back history.");
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HISTORY);
}

void pump_stop_hold_key_long_2s_handler(ui_proc_pump_t *proc)
{
    int pump_status = 0;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
        return;
    }
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_TIMEOUT ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_LOW_POWER ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        LOG_WM("finish popup showing, ignore stop hold.");
        return;
    }

    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (!PUMP_NO_FINISH(pump_status)) {
        LOG_WM("pump status:%d, ignore stop hold.", pump_status);
        return;
    }

    LOG_WM("power long 2s, show pump stop hold ui.");
    show_pump_stop_hold_ui(proc);
}

void pump_stop_hold_key_up_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_STOP_HOLD) {
        return;
    }
    if (!proc->stop_hold_holding) {
        return;
    }
    LOG_WM("power key up before stop hold complete, dismiss.");
    pump_stop_hold_dismiss(proc);
}
