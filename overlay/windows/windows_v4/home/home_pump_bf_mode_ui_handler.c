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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_bf_mode_key_click_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int mode_index = 0;
    READ_PUMP_EXPERT_MODE(proc->pump, false, mode_index);
    proc->ui_bf_mode_index = PUMP_IS_BREASTFEEDING_MODE(mode_index) == 0 ? 
        BREAST_PUMP_EXPERT_MODE_DEFAULT : mode_index; 
    int activate_status = 0;
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    uint8_t mode_upper_boundary = activate_status > 0 ? 
        BREAST_PUMP_CUSTOM_MODE : (BREAST_PUMP_CUSTOM_MODE - 1); /// 获取韵律模式上限边界
    READ_PUMP_WORK_MODE(proc->pump, mode_index);
    if (PUMP_IS_BREASTFEEDING_MODE(mode_index) == 0) {
        if (proc->ui_bf_mode_index == BREAST_PUMP_CUSTOM_MODE && activate_status == 0) {
            proc->ui_bf_mode_index = BREAST_PUMP_EXPERT_MODE_DEFAULT;
        }
    } else {   
        proc->ui_bf_mode_index = proc->ui_bf_mode_index < mode_upper_boundary ? 
                (proc->ui_bf_mode_index + 1) : BREAST_PUMP_EXPERT_MODE_LOWER;   
    }    
    WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_bf_mode_index, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, proc->ui_bf_mode_index, &mode_name);
    LOG_UI("switch %s rhythm: %d", mode_name, proc->ui_bf_mode_index); /// 切换韵律模式
    proc->exit_current_ui = false;
    switch (proc->ui_bf_mode_index) {
        case BREAST_PUMP_MILK_BOOST_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_MILK_BOOST_DESC;
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_POWER_PUMPING_DESC;
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_CUSTOMIZE_DESC;
            break;
    }
    home_mode_change_ui_update(proc);
}

void home_pump_bf_mode_ui_exit(ui_proc_home_t *proc)
{

}
