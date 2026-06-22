/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_MODE_UPDATE_UI_DISPLAY_TASK_NAME                     "home_mode_update_ui_dispaly_cmp"

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
static void home_pump_mode_update_display_completed_cb(hdr_post_args_t *args)
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
void stop_home_common_ui(ui_proc_home_t *proc)
{
    home_pump_bf_mode_ui_exit(proc);
    home_pump_normal_mode_ui_exit(proc);
    home_pump_level_ui_exit(proc);
    home_pump_both_level_ui_exit(proc);
    home_pump_result_ui_exit(proc);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_MODE_UPDATE_UI_DISPLAY_TASK_NAME);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}

void home_cancel_key_event_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->exit_current_ui = true;
}

void home_ui_change_hdr_remove(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case HOME_UI_TYPE_BF_MODE_SELECT:
            home_pump_bf_mode_ui_exit(proc);
            break;
        case HOME_UI_TYPE_NORMAL_MODE_SELECT:
            home_pump_normal_mode_ui_exit(proc);
            break;
        case HOME_UI_TYPE_LEVEL_ADJUST:
            home_pump_level_ui_exit(proc);
            break;
        case HOME_UI_TYPE_PUMP_RESULT:
            home_pump_result_ui_exit(proc);
            break;
        case HOME_UI_TYPE_BOTH_LEVEL_ADJUST:
            home_pump_both_level_ui_exit(proc);
            break;
        default:
            break;
    }
}

void home_page_display_type_update(ui_proc_home_t *proc)
{
    if (proc->home_display_type == HOME_UI_DISPLAY_SIMULATE_MODE_DESC ||
        proc->home_display_type == HOME_UI_DISPLAY_EXPRESSION_MODE_DESC ||
        proc->home_display_type == HOME_UI_DISPLAY_MIXED_MODE_DESC ||
        proc->home_display_type == HOME_UI_DISPLAY_MILK_BOOST_DESC ||
        proc->home_display_type == HOME_UI_DISPLAY_POWER_PUMPING_DESC ||
        proc->home_display_type == HOME_UI_DISPLAY_CUSTOMIZE_DESC) {
        return;
    }

    proc->home_display_type = HOME_UI_DISPLAY_DEFAULT;
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    uint8_t is_utc_time_calibrated = 0, pump_record_flag = 0;
    uint32_t timestamp = 0;
    uint32_t rtc_timestamp = platform_get_utc_time();
    if (st && st->get_store_val) {
        st->get_store_val(STORE_ID_SYS_UTC_TIMESTAMP, 0, 0, &timestamp);
    }

    int diff_time = rtc_timestamp - timestamp;
    if (diff_time < 0 || diff_time > HOME_UTC_CALIB_EXPIRE_INTERVAL_SEC ||
        timestamp == PRODUCT_BUILD_TIMESTAMP) {
        is_utc_time_calibrated = 0;
    } else {
        is_utc_time_calibrated = 1;
    }

    const pump_result_info_t *result_info = NULL;
    READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_HISTORY, &result_info);
    proc->pump_record_time = result_info->finish_timestamp;
    if (proc->pump_record_time > timestamp &&
        rtc_timestamp >= proc->pump_record_time) {
        pump_record_flag = 1;
    } else {
        pump_record_flag = 0;
    }
    
    uint8_t battery_state = BATTERY_STATE_NONE;
    if (proc->battery) {
        battery_state = proc->battery->get_battery_state();
    }

    if (battery_state == BATTERY_STATE_EMPTY ||
        battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        battery_state == BATTERY_STATE_FORBID_START) {
        if (is_utc_time_calibrated) {
            proc->home_display_type = HOME_UI_DISPLAY_CLOCK_LOW_POWER;
        } else {
            proc->home_display_type = HOME_UI_DISPLAY_LOW_POWER;
        }
    } else if (is_utc_time_calibrated && pump_record_flag) {
        proc->home_display_type = HOME_UI_DISPLAY_CLOCK_AND_RECORD;
    } else if (is_utc_time_calibrated && !pump_record_flag) {
        proc->home_display_type = HOME_UI_DISPLAY_CLOCK;
    } else if (!is_utc_time_calibrated && pump_record_flag) {
        proc->home_display_type = HOME_UI_DISPLAY_RECORD;
    } else {
        proc->home_display_type = HOME_UI_DISPLAY_DEFAULT;
    }
    LOG_WM("type: %d, timestamp: %d, diff_time: %d, pump_record:%d, battery_state:%d", 
        proc->home_display_type, timestamp, diff_time, proc->pump_record_time,
        battery_state);
}

void home_mode_change_ui_update(ui_proc_home_t *proc)
{
    if (proc->led) {
        proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
    }
    POST_WM_TASK_HANDLER_DELAYED_ARG1(home_pump_mode_update_display_completed_cb, 5000,
         proc, HOME_MODE_UPDATE_UI_DISPLAY_TASK_NAME);
}