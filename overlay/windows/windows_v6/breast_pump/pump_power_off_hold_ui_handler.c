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
#define PUMP_POWER_OFF_HOLD_COMPLETE_TASK_NAME      "pump_power_off_hold_complete"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void power_off_hold_complete_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_POWER_OFF_HOLD) {
        LOG_UI("pump power off hold ui, ignore complete.");
        return;
    }
    LOG_WM("pump power off hold complete, goto power off.");
    proc->power_off_hold_holding = 0;
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
    msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_MANUAL;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void power_off_hold_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_power_off_hold_process(ui);
}

static void power_off_hold_animation_progress_cb(void *user_data, int index,
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
            power_off_hold_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            POST_WM_TASK_HANDLER_NO_DELAY(power_off_hold_complete_cb,
                ui->priv.play_info.user_data, PUMP_POWER_OFF_HOLD_COMPLETE_TASK_NAME);
            break;
        default:
            break;
    }
}

static void power_off_hold_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_off_hold_prepare(ui);
}

static void power_off_hold_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_off_hold_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_power_off_hold_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }
    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);

    ui_play_info_t play_info = {
        .tag = "pump_power_off_hold_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = power_off_hold_animation_progress_cb,
        .display_prepare_cb = power_off_hold_ui_display_prepare,
        .display_remove_cb = power_off_hold_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_POWER_OFF_HOLD;
    proc->power_off_hold_holding = 1;
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
}

void pump_power_off_hold_ui_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->power_off_hold_holding = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_POWER_OFF_HOLD_COMPLETE_TASK_NAME);
}

void pump_power_off_hold_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }
    LOG_WM("dismiss pump power off hold, nav back history.");
    proc->power_off_hold_holding = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_POWER_OFF_HOLD_COMPLETE_TASK_NAME);
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HISTORY);
}

void pump_power_off_hold_key_long_2s_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type == PUMP_UI_TYPE_POWER_OFF_HOLD ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
        return;
    }

    LOG_WM("power long 2s, show pump power off hold ui.");
    show_pump_power_off_hold_ui(proc);
}

void pump_power_off_hold_key_up_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }
    if (!proc->power_off_hold_holding) {
        return;
    }
    LOG_WM("power key up before pump power off hold complete, dismiss.");
    pump_power_off_hold_dismiss(proc);
}
