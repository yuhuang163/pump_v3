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
#define PUMP_LOW_POWER_DISPLAY_TIMEOUT_MS               (3000)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_low_power_back_home(ui_proc_pump_t *proc)
{
    uint16_t restore_popup = proc->popup_ui_type;
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
    if (restore_popup == ID_POPUP_UI_PUMP_RUNING_30M_TIP) {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_RUNING_30M_TIP);
        LOG_WM("restore pump running 30m tip popup.");
    }
}

static void pump_low_power_complete_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_LOW_POWER) {
        LOG_UI("low power ui, ignore complete.");
        return;
    }
    LOG_WM("pump low power countdown complete, back to pump home.");
    pump_low_power_back_home(proc);
}

static void pump_low_power_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_battery_low_power_process(ui);
}

static void pump_low_power_animation_progress_cb(void *user_data, int index,
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
            pump_low_power_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_low_power_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_battery_low_power_prepare(ui, 1);
}

static void pump_low_power_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_battery_low_power_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_low_power_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->popup_ui_type != ID_POPUP_UI_PUMP_RUNING_30M_TIP) {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_LOW_POWER) {
        return;
    }
    pump_ui_change_hdr_remove(proc);

    proc->current_ui_type = PUMP_UI_TYPE_LOW_POWER;
    ui_play_info_t play_info = {
        .tag = "pump_low_power_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_low_power_animation_progress_cb,
        .display_prepare_cb = pump_low_power_ui_display_prepare,
        .display_remove_cb = pump_low_power_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    POST_WM_TASK_HANDLER_DELAYED_ARG1(pump_low_power_complete_cb,
        PUMP_LOW_POWER_DISPLAY_TIMEOUT_MS, proc, PUMP_LOW_POWER_COMPLETE_TASK_NAME);
}

void pump_low_power_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LOW_POWER_COMPLETE_TASK_NAME);
}

void pump_low_power_ui_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_LOW_POWER) {
        return;
    }
    LOG_WM("dismiss pump low power ui by key, back to pump home.");
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LOW_POWER_COMPLETE_TASK_NAME);
    pump_low_power_back_home(proc);
}
