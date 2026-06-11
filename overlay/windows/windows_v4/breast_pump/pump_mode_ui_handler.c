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
void pump_mode_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump); 
    int mode_index = 0;
    READ_PUMP_COMMON_MODE(proc->pump, false, mode_index);
    proc->ui_normal_mode_index = PUMP_IS_COMMON_MODE(mode_index) == 0 ? 
        BREAST_PUMP_NORMAL_MODE_DEFAULT : mode_index;
    READ_PUMP_WORK_MODE(proc->pump, mode_index);
    if (proc->ui_normal_mode_index == mode_index) {
        proc->ui_normal_mode_index = proc->ui_normal_mode_index < BREAST_PUMP_NORMAL_MODE_UPPER ? 
                (proc->ui_normal_mode_index + 1) : BREAST_PUMP_NORMAL_MODE_LOWER;
    }
    WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_normal_mode_index, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, proc->ui_normal_mode_index, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, proc->ui_normal_mode_index);
    proc->exit_current_ui = false;
    proc->forbid_start_pump = false;
}

void pump_mode_ui_exit(ui_proc_pump_t *proc)
{

}