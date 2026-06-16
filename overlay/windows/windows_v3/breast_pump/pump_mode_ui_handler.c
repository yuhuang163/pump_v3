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
#define PUMP_MODE_DISPLAY_TASK_NAME                     "pump_mode_display_cmp"
#define PUMP_MODE_SYNC_TASK_NAME                        "pump_mode_page_sync"

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
void pump_common_mode_change(ui_proc_pump_t *proc, uint8_t mode)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int mode_from = 0;
    READ_PUMP_WORK_MODE(proc->pump, mode_from);
    if (mode_from != (int)mode) {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_PUMP_MODE,
            .adjust_from = (uint8_t)mode_from,
            .adjust_to = mode,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
    }
    WRITE_PUMP_WORK_MODE(proc->pump, mode, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, mode);
}