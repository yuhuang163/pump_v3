/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager_api.h"
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t normalize_common_mode(int mode)
{
    return PUMP_IS_COMMON_MODE(mode) ? (uint8_t)mode : BREAST_PUMP_NORMAL_MODE_DEFAULT;
}

static uint8_t normalize_expert_mode(int mode, int activate_status)
{
    if (!PUMP_IS_BREASTFEEDING_MODE(mode)) {
        return BREAST_PUMP_EXPERT_MODE_DEFAULT;
    }
    if (mode == BREAST_PUMP_CUSTOM_MODE && activate_status == 0) {
        return BREAST_PUMP_EXPERT_MODE_DEFAULT;
    }
    return (uint8_t)mode;
}

static uint8_t next_common_mode(int mode)
{
    uint8_t common = normalize_common_mode(mode);
    return common < BREAST_PUMP_NORMAL_MODE_UPPER ?
            (common + 1) : BREAST_PUMP_NORMAL_MODE_LOWER;
}

static uint8_t next_expert_mode(int mode, int activate_status)
{
    if (!PUMP_IS_BREASTFEEDING_MODE(mode)) {
        mode = BREAST_PUMP_EXPERT_MODE_DEFAULT;
    }
    if (mode == BREAST_PUMP_CUSTOM_MODE && activate_status == 0) {
        return BREAST_PUMP_EXPERT_MODE_LOWER;
    }
    uint8_t upper = activate_status > 0 ?
            BREAST_PUMP_CUSTOM_MODE : (BREAST_PUMP_CUSTOM_MODE - 1);
    return (uint8_t)mode < upper ? (uint8_t)(mode + 1) : BREAST_PUMP_EXPERT_MODE_LOWER;
}

static uint8_t enter_select_pump_mode(int work_mode, bool is_bf_mode,
        int last_expert_mode, int last_common_mode, int activate_status)
{
    bool mode_matches_key = is_bf_mode ?
            PUMP_IS_BREASTFEEDING_MODE(work_mode) : PUMP_IS_COMMON_MODE(work_mode);
    if (mode_matches_key) {
        return (uint8_t)work_mode;
    }
    return is_bf_mode ?
            normalize_expert_mode(last_expert_mode, activate_status) :
            normalize_common_mode(last_common_mode);
}

static uint8_t cycle_pump_mode(int work_mode, bool is_bf_mode,
        int last_expert_mode, int last_common_mode, int activate_status)
{
    if (PUMP_IS_BREASTFEEDING_MODE(work_mode)) {
        if (is_bf_mode) {
            return next_expert_mode(last_expert_mode, activate_status);
        }
        return normalize_common_mode(last_common_mode);
    }

    if (is_bf_mode) {
        return normalize_expert_mode(last_expert_mode, activate_status);
    }
    return next_common_mode(last_common_mode);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int pump_mode_adjust_handler(ui_proc_pump_t *proc, bool is_bf_mode)
{
    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        show_pump_home_ui(proc); /// 显示泵奶主界面
    }
    proc->pump_mode_adjust_time = get_timestamp();
    bool entering_select = (proc->pump_mode_display_type == PUMP_MODE_DISPLAY_DEFAULT);
    if (entering_select) {
        pump_level_adjust_exit_handler(proc);
    }
    proc->pump_mode_display_type = is_bf_mode ?
            PUMP_MODE_DISPLAY_BF_MODE : PUMP_MODE_DISPLAY_NORMAL_MODE;
    int work_mode = 0, last_expert_mode = 0, last_common_mode = 0, activate_status = 0;
    READ_PUMP_WORK_MODE(proc->pump, work_mode);
    READ_PUMP_EXPERT_MODE(proc->pump, false, last_expert_mode);
    READ_PUMP_COMMON_MODE(proc->pump, false, last_common_mode);
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    if (entering_select) {
        proc->ui_mode_index = enter_select_pump_mode(work_mode, is_bf_mode,
                last_expert_mode, last_common_mode, activate_status);
    } else {
        proc->ui_mode_index = cycle_pump_mode(work_mode, is_bf_mode,
                last_expert_mode, last_common_mode, activate_status);
    }
    if (work_mode != proc->ui_mode_index) {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_PUMP_MODE,
            .adjust_from = work_mode,
            .adjust_to = proc->ui_mode_index,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
    }
    WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_mode_index, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, proc->ui_mode_index, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, proc->ui_mode_index);
    proc->exit_current_ui = false;
    return 0;
}

void pump_mode_adjust_timeout_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pump_mode_display_type == PUMP_MODE_DISPLAY_DEFAULT) {
        return;
    }

    if (get_timestamp_ms(proc->pump_mode_adjust_time) < PUMP_HOME_MODE_SELECT_DISPLAY_TIME) {
        return;
    }
    proc->pump_mode_display_type = PUMP_MODE_DISPLAY_DEFAULT;
    proc->pump_mode_adjust_time = get_timestamp();
}

int pump_mode_adjust_exit_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->pump_mode_display_type == PUMP_MODE_DISPLAY_DEFAULT) {
        return -1;
    }
    proc->pump_mode_display_type = PUMP_MODE_DISPLAY_DEFAULT;
    proc->pump_mode_adjust_time = get_timestamp();
    return 0;
}

