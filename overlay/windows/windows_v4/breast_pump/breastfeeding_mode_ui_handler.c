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
#define BREASTFEEDING_MODE_DISPLAY_TASK_NAME       "bf_mode_display_cmp"
#define BREASTFEEDING_MODE_SYNC_TASK_NAME          "bf_mode_page_sync"

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
    EXCEPTION_CHECK_VOID(proc);
    LOG_WM("ignore key event.");
}

void breastfeeding_mode_ui_exit(ui_proc_pump_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(BREASTFEEDING_MODE_SYNC_TASK_NAME);
}
