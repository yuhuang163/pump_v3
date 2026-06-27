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
int home_pump_level_adjust_type_update(ui_proc_home_t *proc, uint8_t is_left)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        start_home_ui_animation(proc);
    }
    proc->level_adjust_time = get_timestamp();
    if (is_left) {
        proc->pump_level_display_type = HOME_PUMP_LEVEL_DISPLAY_LEFT_ADJUST;
        proc->level_adjust_pump_num = 1;
        proc->level_adjust_pump_index = 0;
    } else {
        proc->pump_level_display_type = HOME_PUMP_LEVEL_DISPLAY_RIGHT_ADJUST;
        proc->level_adjust_pump_num = 1;
        proc->level_adjust_pump_index = 1;
    }
    return 0;
}

int home_pump_level_adjust_handler(ui_proc_home_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        return -1;
    }
    proc->level_adjust_time = get_timestamp();
    if (proc->pump_level_display_type == HOME_PUMP_LEVEL_DISPLAY_DEFAULT) {
        proc->pump_level_display_type = HOME_PUMP_LEVEL_DISPLAY_BOTH_ADJUST;
        proc->level_adjust_pump_num = 2;
        proc->level_adjust_pump_index = 0;
    }
    READ_PUMP_SETTING_INFO(proc->pump, 0, proc->setting);
    if (is_increase) {
        for (uint8_t i = 0; i < proc->level_adjust_pump_num; i++) {
            if (i >= BREAST_PUMP_NUM) {
                break;
            }
            if (proc->setting.level[proc->level_adjust_pump_index + i] < BREAST_PUMP_LEVEL_15) {
                proc->setting.level[proc->level_adjust_pump_index + i]++;
            }
        }
    } else {
        for (uint8_t i = 0; i < proc->level_adjust_pump_num; i++) {
            if (i >= BREAST_PUMP_NUM) {
                break;
            }
            if (proc->setting.level[proc->level_adjust_pump_index + i] > BREAST_PUMP_LEVEL_0) {
                proc->setting.level[proc->level_adjust_pump_index + i]--;
            }
        }
    }

    WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
    return 0;
}

void home_pump_level_adjust_timeout_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pump_level_display_type == HOME_PUMP_LEVEL_DISPLAY_DEFAULT) {
        return;
    }

    if (get_timestamp_ms(proc->level_adjust_time) < HOME_PUMP_LEVEL_DISPLAY_TIMEOUT) {
        return;
    }
    proc->pump_level_display_type = HOME_PUMP_LEVEL_DISPLAY_DEFAULT;
    proc->level_adjust_time = get_timestamp();
}

void home_pump_level_adjust_exit_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pump_level_display_type == HOME_PUMP_LEVEL_DISPLAY_DEFAULT) {
        return;
    }
    proc->pump_level_display_type = HOME_PUMP_LEVEL_DISPLAY_DEFAULT;
    proc->level_adjust_time = get_timestamp();
}