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

/// 其他状态下，编码器的作用是回到 home 页和返回
void pump_power_key_handler(ui_proc_pump_t *proc)
{
#if 0
    EXCEPTION_CHECK_VOID(proc);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (PUMP_NO_FINISH(pump_status)) {
        pump_ui_change_hdr_remove(proc);
        if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
            WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_common_mode_index, true, 1);
            pump_popup_ui_type_update(proc, PUMP_POPUP_UI_OPERATION_SUCCESS);
        } else if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
            WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_breastfeeding_mode_index, true, 1);
            pump_popup_ui_type_update(proc, PUMP_POPUP_UI_OPERATION_SUCCESS);
        }
        show_pump_home_ui(proc);
    } else {
        pump_ui_change_hdr_remove(proc);
        multi_func_msg_data_t msg = {0};
        LOG_WM("goto home window.");
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        if (proc->wm) {
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
        }
    }
#else
    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE ||
        proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        proc->goto_pump_home_ui = true;
    } else {
        pump_popup_ui_type_update(proc, PUMP_POPUP_UI_OPERATION_FAILED);
    }
#endif
}

void pump_popup_ui_type_update(ui_proc_pump_t *proc, pump_popup_ui_action_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_start_time = get_timestamp();
    proc->popup_ui_type = type;
}