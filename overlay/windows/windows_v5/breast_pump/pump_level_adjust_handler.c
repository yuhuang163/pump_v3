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
int pump_level_adjust_type_update(ui_proc_pump_t *proc, uint8_t is_left)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        show_pump_home_ui(proc);
    }
    pump_mode_adjust_exit_handler(proc);
    proc->level_adjust_time = get_timestamp();
    if (is_left) {
        proc->pump_level_display_type = PUMP_LEVEL_DISPLAY_LEFT_ADJUST;
        proc->level_adjust_pump_num = 1;
        proc->level_adjust_pump_index = 0;
    } else {
        proc->pump_level_display_type = PUMP_LEVEL_DISPLAY_RIGHT_ADJUST;
        proc->level_adjust_pump_num = 1;
        proc->level_adjust_pump_index = 1;
    }
    return 0;
}

int pump_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        return -1;
    }
    proc->level_adjust_time = get_timestamp();
    if (proc->pump_level_display_type == PUMP_LEVEL_DISPLAY_DEFAULT) {
        proc->pump_level_display_type = PUMP_LEVEL_DISPLAY_BOTH_ADJUST;
        proc->level_adjust_pump_num = 2;
        proc->level_adjust_pump_index = 0;
        pump_mode_adjust_exit_handler(proc);
    }
    READ_PUMP_SETTING_INFO(proc->pump, 0, proc->setting);
    pump_setting_t setting_before = proc->setting;
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

    {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_SUCTION_LEVEL,
            .adjust_from_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(setting_before.level[0], setting_before.level[1]),
            .adjust_to_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(proc->setting.level[0], proc->setting.level[1]),
        };

        if (bury_adj.adjust_from_desktop != bury_adj.adjust_to_desktop) {
            DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
        }
    }
    WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
    return 0;
}

void pump_level_adjust_timeout_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pump_level_display_type == PUMP_LEVEL_DISPLAY_DEFAULT) {
        return;
    }

    if (get_timestamp_ms(proc->level_adjust_time) < PUMP_LEVEL_DISPLAY_TIMEOUT) {
        return;
    }
    proc->pump_level_display_type = PUMP_LEVEL_DISPLAY_DEFAULT;
    proc->level_adjust_time = get_timestamp();
}

void pump_level_adjust_exit_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pump_level_display_type == PUMP_LEVEL_DISPLAY_DEFAULT) {
        return;
    }
    proc->pump_level_display_type = PUMP_LEVEL_DISPLAY_DEFAULT;
    proc->level_adjust_time = get_timestamp();
}