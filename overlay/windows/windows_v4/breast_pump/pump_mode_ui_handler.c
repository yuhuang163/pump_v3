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
    uint8_t mode_from = (uint8_t)mode_index;
    if (proc->ui_normal_mode_index == mode_index) {
        proc->ui_normal_mode_index = proc->ui_normal_mode_index < BREAST_PUMP_NORMAL_MODE_UPPER ? 
                (proc->ui_normal_mode_index + 1) : BREAST_PUMP_NORMAL_MODE_LOWER;
    }
    if (mode_from != proc->ui_normal_mode_index) {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_PUMP_MODE,
            .adjust_from = mode_from,
            .adjust_to = proc->ui_normal_mode_index,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
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