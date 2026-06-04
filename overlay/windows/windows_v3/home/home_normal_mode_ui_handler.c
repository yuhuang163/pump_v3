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
#define HOME_NORMAL_MODE_DISPLAY_TASK_NAME                     "home_normal_mode_display_cmp"

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

static void home_pump_normal_mode_ui_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    proc->home_display_type = HOME_UI_DISPLAY_DEFAULT;
    home_page_display_type_update(proc);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_normal_mode_change(ui_proc_home_t *proc, uint8_t mode)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    WRITE_PUMP_WORK_MODE(proc->pump, mode, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, mode);
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_SIMULATE_MODE_DESC;
            break;
        case BREAST_PUMP_LACTATION_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_EXPRESSION_MODE_DESC;
            break;
        case BREAST_PUMP_MIX_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_MIXED_MODE_DESC;
            break;
    }
    
    if (proc->led) {
        proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
    }
    POST_WM_TASK_HANDLER_DELAYED_ARG1(home_pump_normal_mode_ui_display_completed_cb, 5000,
         proc, HOME_NORMAL_MODE_DISPLAY_TASK_NAME);
}

void home_pump_normal_mode_ui_exit(ui_proc_home_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_NORMAL_MODE_DISPLAY_TASK_NAME);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
}
