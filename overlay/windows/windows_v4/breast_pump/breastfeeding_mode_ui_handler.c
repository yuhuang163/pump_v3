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
void breastfeeding_mode_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int mode_index = 0;
    READ_PUMP_EXPERT_MODE(proc->pump, false, mode_index);
    proc->ui_bf_mode_index = PUMP_IS_BREASTFEEDING_MODE(mode_index) == 0 ? 
        BREAST_PUMP_EXPERT_MODE_DEFAULT : mode_index; 
    int activate_status = 0;
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    uint8_t mode_upper_boundary = activate_status > 0 ? 
        BREAST_PUMP_CUSTOM_MODE : (BREAST_PUMP_CUSTOM_MODE - 1); /// 获取韵律模式上限边界
    uint8_t mode_from = proc->ui_bf_mode_index;
    READ_PUMP_WORK_MODE(proc->pump, mode_index);
    if (PUMP_IS_BREASTFEEDING_MODE(mode_index) == 0) {
        if (proc->ui_bf_mode_index == BREAST_PUMP_CUSTOM_MODE && activate_status == 0) {
            proc->ui_bf_mode_index = BREAST_PUMP_EXPERT_MODE_DEFAULT;
        }
    } else {   
        proc->ui_bf_mode_index = proc->ui_bf_mode_index < mode_upper_boundary ? 
                (proc->ui_bf_mode_index + 1) : BREAST_PUMP_EXPERT_MODE_LOWER;   
    }  
    ///触发埋点本地模式切换
    if (mode_from != proc->ui_bf_mode_index) {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_PUMP_MODE,
            .adjust_from = mode_from,
            .adjust_to = proc->ui_bf_mode_index,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
    }

    WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_bf_mode_index, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, proc->ui_bf_mode_index, &mode_name);
    LOG_UI("switch %s rhythm: %d", mode_name, proc->ui_bf_mode_index); /// 切换韵律模式
    proc->exit_current_ui = false;
    proc->forbid_start_pump = false;
}

void breastfeeding_mode_ui_exit(ui_proc_pump_t *proc)
{

}
