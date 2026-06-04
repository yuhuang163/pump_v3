/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_BF_MODE_DISPLAY_TASK_NAME       "home_bf_mode_display_cmp"

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

static void home_pump_bf_mode_ui_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_bf_mode_key_click_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc); 
    proc->exit_current_ui = false; 
}

void home_pump_bf_mode_ui_exit(ui_proc_home_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_BF_MODE_DISPLAY_TASK_NAME);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
}
