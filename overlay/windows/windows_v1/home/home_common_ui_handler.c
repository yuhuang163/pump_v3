/*
* INCLUDE FILES
*****************************************************************************************
*/
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
void stop_home_common_ui(ui_proc_home_t *proc)
{
    home_bind_ui_hdr_remove();
    home_pump_bf_mode_ui_exit(proc);
    home_pump_normal_mode_ui_exit(proc);
    home_pump_level_ui_exit(proc);
    home_pump_speed_ui_exit(proc);
    home_pump_result_ui_exit(proc);
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

void home_ui_change_hdr_remove(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case HOME_UI_TYPE_BIND_QRCODE:
            home_bind_ui_hdr_remove();
            break;  
        case HOME_UI_TYPE_BF_MODE_SELECT:
            home_pump_bf_mode_ui_exit(proc);
            break;
        case HOME_UI_TYPE_NORMAL_MODE_SELECT:
            home_pump_normal_mode_ui_exit(proc);
            break;
        case HOME_UI_TYPE_LEVEL_ADJUST:
            home_pump_level_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SPEED_ADJUST:
            home_pump_speed_ui_exit(proc);
            break;
        case HOME_UI_TYPE_PUMP_RESULT:
            home_pump_result_ui_exit(proc);
            break;
        default:
            break;
    }
}

void home_page_display_type_update(ui_proc_home_t *proc)
{
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