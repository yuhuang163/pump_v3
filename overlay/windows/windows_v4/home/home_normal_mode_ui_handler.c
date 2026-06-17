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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_normal_mode_key_click_handler(ui_proc_home_t *proc)
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

    switch (proc->ui_normal_mode_index) {
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
    home_mode_change_ui_update(proc);
}

void home_pump_normal_mode_ui_exit(ui_proc_home_t *proc)
{

}
