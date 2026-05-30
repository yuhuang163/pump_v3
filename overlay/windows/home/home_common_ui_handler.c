/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/

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
    home_bind_ui_hdr_remove();
    home_pump_bf_mode_ui_exit(proc);
    home_pump_normal_mode_ui_exit(proc);
    home_pump_level_ui_exit(proc);
    #if V3_UX_VERSION == V3_UX_V1
    home_pump_speed_ui_exit(proc);
    #endif
    home_pump_result_ui_exit(proc);
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

void home_cancel_key_event_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
#if 0
    if (proc->current_ui_type == HOME_UI_TYPE_BF_MODE_SELECT || 
        proc->current_ui_type == HOME_UI_TYPE_NORMAL_MODE_SELECT ||
        proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST) {
        #if V3_UX_VERSION == V3_UX_V2
        proc->exit_current_ui = true;
        #endif
        return;
    }
#else
    if (proc->current_ui_type == HOME_UI_TYPE_BF_MODE_SELECT || 
        proc->current_ui_type == HOME_UI_TYPE_NORMAL_MODE_SELECT ||
        proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST || 
        proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT || 
        proc->current_ui_type == HOME_UI_TYPE_NIGHT_LIGHT_ADJUST || 
        proc->current_ui_type == HOME_UI_TYPE_BIND_QRCODE) {
        #if V3_UX_VERSION == V3_UX_V2
        proc->exit_current_ui = true;
        #endif
        return;
    }
#endif
    LOG_WM("ignore cancel key event.");
}

void home_ui_change_hdr_remove(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case HOME_UI_TYPE_BIND_QRCODE:
            home_bind_ui_hdr_remove();
            break;
        case HOME_UI_TYPE_BF_MODE_SELECT:
            home_pump_bf_mode_ui_exit(proc);
            break;
        case HOME_UI_TYPE_NORMAL_MODE_SELECT:
            home_pump_normal_mode_ui_exit(proc);
            break;
        case HOME_UI_TYPE_LEVEL_ADJUST:
            home_pump_level_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SPEED_ADJUST:
            #if V3_UX_VERSION == V3_UX_V1
            home_pump_speed_ui_exit(proc);
            #endif
            break;
        case HOME_UI_TYPE_PUMP_RESULT:
            home_pump_result_ui_exit(proc);
            break;
        default:
            break;
    }
}