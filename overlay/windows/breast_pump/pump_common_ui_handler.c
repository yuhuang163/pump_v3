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
    } else if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        pump_speed_ui_hdr_remove();
    } else if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        pump_result_ui_hdr_remove();
    } 
}

/// 其他状态下，编码器的作用是回到 home 页和返回
void pump_power_key_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    int status = proc->pump->get_breast_pump_status();
    if (PUMP_NO_FINISH(status)) {
        pump_ui_change_hdr_remove(proc);
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
}
