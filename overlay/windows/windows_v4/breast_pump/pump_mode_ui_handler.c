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
void pump_mode_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    LOG_WM("ignore key event.");
}

void pump_mode_ui_exit(ui_proc_pump_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_MODE_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_MODE_SYNC_TASK_NAME);
}