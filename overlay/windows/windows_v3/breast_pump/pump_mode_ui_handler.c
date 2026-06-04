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
    WRITE_PUMP_WORK_MODE(proc->pump, mode, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, mode);
}