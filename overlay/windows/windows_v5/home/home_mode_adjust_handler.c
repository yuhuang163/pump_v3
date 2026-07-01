/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager_api.h"
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_PUMP_MODE_UPDATE_DISPLAY_TASK_NAME                     "home_pump_mode_update_display_cmp"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_pump_mode_update_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    // proc->home_display_type = HOME_UI_DISPLAY_DEFAULT;
    // home_page_display_type_update(proc);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
}

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

static void update_home_display_for_mode(ui_proc_home_t *proc)
{
    switch (proc->ui_mode_index) {
        default:
        case BREAST_PUMP_STIMULATION_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_SIMULATE_MODE_DESC;
            break;
        case BREAST_PUMP_LACTATION_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_EXPRESSION_MODE_DESC;
            break;
        case BREAST_PUMP_MIX_MODE:
            proc->home_display_type = HOME_UI_DISPLAY_MIXED_MODE_DESC;
            break;
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
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_mode_adjust_ui_exit(ui_proc_home_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_PUMP_MODE_UPDATE_DISPLAY_TASK_NAME);
}

int home_pump_mode_adjust_handler(ui_proc_home_t *proc, bool is_bf_mode)
{
    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        start_home_ui_animation(proc);
    }

    bool entering_select = (proc->pump_mode_display_type == HOME_PUMP_MODE_DISPLAY_DEFAULT);
    if (entering_select) {
        home_pump_mode_adjust_ui_exit(proc);
    }
    proc->pump_mode_display_type = is_bf_mode ?
            HOME_PUMP_MODE_DISPLAY_BF_MODE : HOME_PUMP_MODE_DISPLAY_NORMAL_MODE;
    proc->pump_mode_adjust_time = get_timestamp();

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
    WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_mode_index, true, 1);
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, proc->ui_mode_index, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, proc->ui_mode_index);
    proc->exit_current_ui = false;
    update_home_display_for_mode(proc);
    return 0;
}

void home_pump_mode_adjust_timeout_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pump_mode_display_type == HOME_PUMP_MODE_DISPLAY_DEFAULT) {
        return;
    }

    if (get_timestamp_ms(proc->pump_mode_adjust_time) < HOME_MODE_SELECT_DISPLAY_TIME) {
        return;
    }
    proc->pump_mode_display_type = HOME_PUMP_MODE_DISPLAY_DEFAULT;
    proc->pump_mode_adjust_time = get_timestamp();
    POST_WM_TASK_HANDLER_NO_DELAY(home_pump_mode_update_display_completed_cb,
         proc, HOME_PUMP_MODE_UPDATE_DISPLAY_TASK_NAME);
}


void home_pump_mode_adjust_exit_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->pump_mode_display_type == HOME_PUMP_MODE_DISPLAY_DEFAULT) {
        return;
    }
    proc->pump_mode_display_type = HOME_PUMP_MODE_DISPLAY_DEFAULT;
    proc->pump_mode_adjust_time = get_timestamp();
    POST_WM_TASK_HANDLER_NO_DELAY(home_pump_mode_update_display_completed_cb,
        proc, HOME_PUMP_MODE_UPDATE_DISPLAY_TASK_NAME);
}