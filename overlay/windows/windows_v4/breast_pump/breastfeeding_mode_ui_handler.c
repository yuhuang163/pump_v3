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
    uint8_t mode = proc->ui_bf_mode_index;
    int activate_status = 0;
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    uint8_t mode_upper_boundary = activate_status > 0 ? 
        BREAST_PUMP_CUSTOM_MODE : (BREAST_PUMP_CUSTOM_MODE - 1); /// 获取韵律模式上限边界
    mode = mode < mode_upper_boundary ? (mode + 1) : BREAST_PUMP_EXPERT_MODE_LOWER;
    proc->ui_bf_mode_index = mode;
    WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_bf_mode_index, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
    LOG_UI("switch %s rhythm: %d", mode_name, mode); /// 切换韵律模式
    proc->exit_current_ui = false;
    proc->forbid_start_pump = false;
}

void breastfeeding_mode_ui_exit(ui_proc_pump_t *proc)
{

}
