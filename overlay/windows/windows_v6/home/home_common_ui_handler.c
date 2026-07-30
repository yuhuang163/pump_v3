/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_home.h"

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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void stop_home_common_ui(ui_proc_home_t *proc)
{
    home_pump_result_ui_exit(proc);
    home_settings_success_ui_exit(proc);
    home_power_off_hold_ui_exit(proc);
    home_level_adjust_ui_exit(proc);
    home_both_level_adjust_ui_exit(proc);
    home_speed_adjust_ui_exit(proc);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}


void home_ui_change_hdr_remove(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case HOME_UI_TYPE_PUMP_RESULT:
            home_pump_result_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SETTINGS_SUCCESS:
            home_settings_success_ui_exit(proc);
            break;
        case HOME_UI_TYPE_NIGHT_LIGHT_ADJUST:
            home_night_light_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_POWER_OFF_HOLD:
            home_power_off_hold_ui_exit(proc);
            break;
        case HOME_UI_TYPE_LEVEL_ADJUST:
            home_level_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_BOTH_LEVEL_ADJUST:
            home_both_level_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SPEED_ADJUST:
            home_speed_adjust_ui_exit(proc);
            break;
        default:
            break;
    }
}
