/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_pump.h"

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
void stop_pump_ui_animation(ui_proc_pump_t *proc)
{
    pump_result_ui_hdr_remove();
    pump_level_ui_hdr_remove();
    pump_both_level_ui_hdr_remove();
    pump_speed_ui_hdr_remove();
    pump_mode_ui_hdr_remove();
    breastfeeding_mode_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void pump_ui_change_hdr_remove(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
        pump_mode_ui_hdr_remove();
    } else if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        breastfeeding_mode_ui_hdr_remove();
    } else if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL) {
        pump_level_ui_hdr_remove();
    } else if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL) {
        pump_both_level_ui_hdr_remove();
    } else if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        pump_speed_ui_hdr_remove();
    } else if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        pump_result_ui_hdr_remove();
    } 
}

void pump_power_key_handler(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE ||
        proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE ||
        proc->current_ui_type == PUMP_UI_TYPE_SPEED ) {
        proc->goto_pump_home_ui = true;
    } 
}

void pump_popup_ui_type_update(ui_proc_pump_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}