#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"
#include "app_protocol_sys_ctrl_server.h"

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
/// 协议亮度 0-100 与驱动亮度 0-100 一一对应
static uint8_t app_old_comm_proto_level_to_percent(uint8_t proto_level)
{
    if (proto_level > APP_OLD_COMM_LIGHT_LEVEL_MAX) {
        proto_level = APP_OLD_COMM_LIGHT_LEVEL_MAX;
    }
    return (proto_level > LED_LEVEL_MAX) ? LED_LEVEL_MAX : proto_level;
}

static uint8_t app_old_comm_percent_to_proto_level(uint8_t percent)
{
    if (percent > APP_OLD_COMM_LIGHT_LEVEL_MAX) {
        percent = APP_OLD_COMM_LIGHT_LEVEL_MAX;
    }
    return percent;
}

static void app_old_comm_light_status_from_cab(const uint8_t *cab, app_old_comm_light_status_t *status)
{
    memcpy(status, cab, sizeof(app_old_comm_light_status_t));
}

static uint8_t app_old_comm_light_status_validate_set_req(const app_old_comm_light_status_t *status)
{
    if (status->night_light_switch > 1 || status->adaptive_switch > 1 ||
        status->night_light_startup_switch > 1) {
        return 0;
    }
    if (status->night_light_gear > 4 ||
        status->screen_key_level > APP_OLD_COMM_LIGHT_LEVEL_MAX) {
        return 0;
    }
    return 1;
}

static void app_old_comm_fill_light_status(app_old_comm_light_status_t *status)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);

    memset(status, 0, sizeof(*status));
    if (led && led->hook) {
#if IS_SUPPORT_NIGHT_LIGHT
        if (led->hook->get_night_light_onoff) {
            status->night_light_switch = led->hook->get_night_light_onoff(led) > 0 ? 1 : 0;
        }
        
        if (led->hook->get_night_light_gear) {
            status->night_light_gear = (uint8_t)led->hook->get_night_light_gear(led);
        }
        if (led->hook->get_night_light_startup_switch) {
            status->night_light_startup_switch =
                led->hook->get_night_light_startup_switch(led) > 0 ? 1 : 0;
        }
#endif

        if (led->hook->get_global_adaptive_dimming_switch) {
            status->adaptive_switch = led->hook->get_global_adaptive_dimming_switch(led) > 0 ? 1 : 0;
        }
        if (led->hook->get_global_backlight_level) {
            status->screen_key_level = (uint8_t)led->hook->get_global_backlight_level(led);
        }
    }
}

static uint8_t app_old_comm_apply_light_control(const app_old_comm_light_status_t *status)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    uint8_t want_adaptive;
    uint8_t cur_adaptive = 0;

    if (led == NULL || led->hook == NULL) {
        LOG_SRV("led is null or hook is null");
        return 0;
    }
    if (!app_old_comm_light_status_validate_set_req(status)) {
        LOG_SRV("invalid light status");
        return 0;
    }

    want_adaptive = status->adaptive_switch > 0 ? 1 : 0;
    if (led->hook->get_global_adaptive_dimming_switch) {
        cur_adaptive = led->hook->get_global_adaptive_dimming_switch(led) > 0 ? 1 : 0;
    }

    /// App 一包下发全量快照。adaptive=1 时同包的 gear / screen_key_level 不能走手动通路：
    /// set_night_light_gear、改屏键亮度都会关 ALS，否则「开自适应」会被后写的档位/亮度打成手动。
    /// 开机夜灯默认开关与自适应无关，两条路径都要落地。
#if IS_SUPPORT_NIGHT_LIGHT
    if (led->hook->set_night_light_startup_switch) {
        led->hook->set_night_light_startup_switch(led, status->night_light_startup_switch > 0 ? 1 : 0);
    }
#endif

    if (want_adaptive) {
        if (cur_adaptive != 1 && led->hook->set_global_adaptive_dimming_switch) {
            led->hook->set_global_adaptive_dimming_switch(led, 1);
        }
#if IS_SUPPORT_NIGHT_LIGHT
        if (led->hook->set_night_light_onoff) {
            uint8_t nl_on = status->night_light_switch > 0 ? 1 : 0;

            led->hook->set_night_light_onoff(led, nl_on);
            if (nl_on && led->hook->set_night_light_adaptive_dimming_switch) {
                led->hook->set_night_light_adaptive_dimming_switch(led, 1);
            }
        }
#endif
        return 1;
    }

    if (cur_adaptive != 0 && led->hook->set_global_adaptive_dimming_switch) {
        led->hook->set_global_adaptive_dimming_switch(led, 0);
    }

#if IS_SUPPORT_NIGHT_LIGHT
    if (led->hook->set_night_light_onoff) {
        led->hook->set_night_light_onoff(led, status->night_light_switch > 0 ? 1 : 0);
    }
    if (led->hook->set_night_light_gear) {
        led->hook->set_night_light_gear(led, status->night_light_gear);
    }
#endif

    if (led->hook->get_global_backlight_level && led->hook->set_global_backlight_level &&
        status->screen_key_level != (uint8_t)led->hook->get_global_backlight_level(led)) {
        uint8_t level = status->screen_key_level;

        if (level < 20) {
            level = 20;
        } else if (level > LED_LEVEL_MAX) {
            level = LED_LEVEL_MAX;
        }
        led->hook->set_global_backlight_level(led, level);
    }

    return 1;
}

static void app_old_comm_set_device_bind_status(uint8_t device_bind_status)
{
    ble_manager_context_t *ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    uint8_t local_state;

    EXCEPTION_CHECK_VOID(ble);
    /// 协议 app_old_comm_bind_status_e → 本地 BLE_BIND_STATE_*
    switch (device_bind_status) {
        case APP_OLD_COMM_BIND_STATUS_UNBIND:
            local_state = BLE_BIND_STATE_INIT;
            break;
        case APP_OLD_COMM_BIND_STATUS_BINDING:
            local_state = BLE_BIND_STATE_KEY_GENERATE;
            break;
        case APP_OLD_COMM_BIND_STATUS_COMPLETE:
            local_state = BLE_BIND_STATE_COMPLETE;
            break;
        default:
            LOG_SRV(" Device bind status invalid: %d", device_bind_status);
            return;
    }
    ble->ble_bind_state_set(local_state);
    LOG_SRV(" Device bind status set: proto=%d local=%d", device_bind_status, local_state);
}

// Get parse result string for logging
static const char* app_old_comm_get_abnormal_type_str(old_publish_abnormal_type_e abnormal_type)
{
    switch (abnormal_type) {
        case OLD_PUBLISH_ABNORMAL_LOW_BAT_CANNOT_START: return "LOW_BAT_CANNOT_START";
        case OLD_PUBLISH_ABNORMAL_LOW_BATTERY: return "LOW_BATTERY";
        case OLD_PUBLISH_ABNORMAL_LOW_BAT_SHUTDOWN_SOON: return "LOW_BAT_SHUTDOWN_SOON";
        case OLD_PUBLISH_ABNORMAL_AIR_LEAK: return "AIR_LEAK";
        case OLD_PUBLISH_ABNORMAL_TRAVEL_LOCK: return "TRAVEL_LOCK";
        case OLD_PUBLISH_ABNORMAL_ADAPTER_ERROR: return "ADAPTER_ERROR";
        case OLD_PUBLISH_ABNORMAL_GUIDE_WINDOW: return "GUIDE_WINDOW";
        case OLD_PUBLISH_ABNORMAL_FAULT_WARNING: return "FAULT_WARNING";
        case OLD_PUBLISH_ABNORMAL_RHYTHM_SWITCHED: return "RHYTHM_SWITCHED";
        case OLD_PUBLISH_ABNORMAL_PUMP_OVER_30MIN: return "PUMP_OVER_30MIN";
        default: return "UNKNOWN_ERROR";
    }
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void app_old_comm_handle_set_bind_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling set bind status request");
    if (cal != 0x02) {
        LOG_SRV(" Invalid data length for SET_BIND (expected 2, got %d)", cal);
        return;
    }
    uint8_t bind_status = cab[0];
    uint8_t left_right = cab[1];
    LOG_SRV(" Setting bind status - Status: %d, Side: %d", bind_status, left_right);
    // TODO: Update binding status
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_BIND, 2, cab);
    app_old_comm_report_bind_status(bind_status, left_right);
}

// -------------------------- Request command handling implementations --------------------------
void app_old_comm_handle_heartbeat_req(void)
{
    LOG_SRV(" Sending heartbeat ACK");
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_HEARTBEAT, 0, NULL);
}

void app_old_comm_handle_set_rtc_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling set RTC request\n");
    if (cal != 0x04) {
        LOG_SRV(" Invalid data length for SET_RTC (expected 4, got %d)", cal);
        return;
    }
    uint32_t utc_time = (cab[3] << 24) | (cab[2] << 16) | (cab[1] << 8) | cab[0];
    LOG_SRV(" Setting RTC time to 0x%08X", utc_time);
    // TODO: Call RTC setting interface
    time_system_t *time_system = (time_system_t *)find_module(ID_MODULE_TIME);
    if (time_system) {
        time_system->sync_utc_time(utc_time);
    }
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_RTC, 0, NULL);
}

void app_old_comm_handle_sys_ctrl_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling system control request");
    if (cal != 0x01) {
        LOG_SRV(" Invalid data length for SYS_CTRL (expected 1, got %d)", cal);
        return;
    }
    uint8_t ctrl_cmd = cab[0];
    LOG_SRV(" Executing system control command 0x%02X", ctrl_cmd);
    if (ctrl_cmd == APP_OLD_COMM_SYS_POWER_OFF) {
        LOG_SRV("CMD-poweroff");
        peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
        EXCEPTION_CHECK_VOID(pbus);
        struct rt_device_node *battery = pbus->find_device(ID_DEVICE_BATTERY);
        uint8_t chage_state = 0;
        //如果V3电池不存在，则认为电池充满，
        if (battery) {
            chage_state = DEVICE_IOCTL(battery, OP_CODE_BAT_VBUS_IS_PLUG_IN);
        } else {
            chage_state = CHARGE_STATE_CHARGING;
        }
        LOG_SRV("chage_state=%d", chage_state);
        //if (USB_IS_PLUGIN(chage_state)) {
            //LOG_SRV("CMD-poweroff, but charging, not power off");
            //return;
        //}
        SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_OFF);
    } else if (ctrl_cmd == APP_OLD_COMM_SYS_REBOOT) {
        LOG_SRV("CMD-softreset");
    } else if (ctrl_cmd == APP_OLD_COMM_SYS_OTA) {
        LOG_SRV("CMD-ota");
    } else if (ctrl_cmd == APP_OLD_COMM_SYS_OFF_OTA_RECOVERY) {
        LOG_SRV("CMD-factory recover");
        //先暂停pump
        /*breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
        EXCEPTION_CHECK_VOID(pump_ctx);
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_PAUSE, 1);
        msleep(200);//确保motor线程退出，泄压完成
        storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (st && st->factory_reset) {
            st->factory_reset(RESET_AND_REBOOT);
        }
        else {
            LOG_SRV("factory_reset failed");
        }*/
        /// 触发清库/Flash/重启埋点 (EVT_010: 清库/Flash/重启)
        {
            data_bury_trigger_user_evt_t bury_evt = {
                .evt_source = DATA_BURY_EVT_SOURCE_REMOTE_APP,
                .evt_type = DATA_BURY_USER_EVT_RESET,
            };
            DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
        }   
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        EXCEPTION_CHECK_VOID(wm);
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_FACTORY_RESET;
        msg.union_body.extra.arg1 = 0;
        wm->dispatch_new_window_args(wm, &msg);
    }
    // TODO: Execute system control logic
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SYS_CTRL, 1, &ctrl_cmd);
}

void app_old_comm_handle_get_encrypt_req()
{
    LOG_SRV(" Handling get encryption info request\n");
    uint8_t ack_data[2] = {0};
    app_old_protocol_handler_context_t *handler_ctx = app_old_protocol_handler_ctx_get();
    ack_data[0] = handler_ctx->encrypt_status;
    ack_data[1] = handler_ctx->encrypt_key;
    LOG_SRV(" Sending encryption info ACK (status: 0x%02X, key: 0x%02X)",
           ack_data[0], ack_data[1]);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_ENCRYPT, 2, ack_data);
}

void app_old_comm_handle_set_travel_lock_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling set travel lock request");
    if (cal != 0x01) {
        LOG_SRV(" Invalid data length for SET_TRAVEL_LOCK (expected 1, got %d)", cal);
        return;
    }
    uint8_t travel_lock = cab[0];
    LOG_SRV(" Setting travel lock status to %d", travel_lock);
    // TODO: Execute travel lock control logic
    airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    EXCEPTION_CHECK_VOID(ap);
    if (true == travel_lock) {
        ap->enter(AIRPLANE_MODE_BY_USER_ACTIVE_ENTER);
    } else {
        ap->exit(AIRPLANE_MODE_EXIT);
    }
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_TRAVEL_LOCK, 0, NULL);
}

void app_old_comm_report_discon_status(void)
{
    LOG_SRV(" Parsing disconnection notification");
    // TODO: Handle BLE disconnection logic
    //app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_DISCON, 0, NULL);
}

void app_old_comm_report_bind_status(uint8_t bind_status, uint8_t left_right)
{
    LOG_SRV(" Reporting bind status - Status: %d, Side: %d", bind_status, left_right);
    uint8_t report_data[2] = {bind_status, left_right};
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_BIND, 2, report_data);
}

void app_old_comm_handle_ctrl_dev_sys_status_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling ctrl_dev_sys_status request");
    EXCEPTION_CHECK_VOID(cab);

    if (cal != 0x07) {
        LOG_SRV(" Invalid data length for CTRL_DEV_STATUS (expected 7, got %d)", cal);
        return;
    }

    app_old_comm_sys_status_ctrl sys_status;

    // Byte 0: Language type (0x00-English, 0x01-Chinese, 0x02-French, 0x03-German, 
    //                        0x04-Spanish, 0x05-Italian, 0x06-Japanese)
    sys_status.sys_language_e = cab[0];

    // Byte 1: Screen lock time (5-30s, 0xFF = no lock)
    sys_status.screem_lock_time = cab[1];

    // Byte 2: Screen brightness level (0:25%, 1:50%, 2:75%, 3:100%)
    sys_status.screem_lock_light_level = cab[2];

    // Byte 3-4: Massage duration in seconds (Little Endian, range: 30s - 600s)
    sys_status.motor_Duration = (uint16_t)(cab[3] | (cab[4] << 8));

    // Byte 5: Adaptive dimming switch (0-off, 1-on)
    //sys_status.adaptive_dimming_switch = cab[5];

    // Byte 6: Device bind status (0-unbound, 1-bound)
    sys_status.device_bind_status = cab[6];

    LOG_SRV(" Sys status: lang=%d, lock_time=%d, brightness=%d, massage_time=%d, bind_status=%d, adaptive_dimming_sw=%d\n",
                 sys_status.sys_language_e,
                 sys_status.screem_lock_time,
                 sys_status.screem_lock_light_level,
                 sys_status.motor_Duration,
                 sys_status.device_bind_status,
                 sys_status.adaptive_dimming_switch);

    // Apply night light settings
    //app_old_comm_set_night_light(sys_status.adaptive_dimming_switch);
    // Apply device bind status
    app_old_comm_set_device_bind_status(sys_status.device_bind_status);
    // TODO: Apply other sys_status settings (language, screen lock, brightness, massage time)

    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_CTRL_DEV_STATUS, cal, cab);
}

void app_old_comm_handle_pump_ble_rgb_set_ctrl_req(uint8_t cal, uint8_t *cab)
{
    uint8_t result = 0x00;
    app_old_comm_light_status_t light_status = {0};

    LOG_SRV(" Handling light control request (CAL=%d)", cal);
    EXCEPTION_CHECK_VOID(cab);
    if (cal != APP_OLD_COMM_LIGHT_STATUS_CAL) {
        LOGE(" Invalid CAL for light control (expected %d, got %d)",
             APP_OLD_COMM_LIGHT_STATUS_CAL, cal);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL,
                                                   1, &result);
        return;
    }

    app_old_comm_light_status_from_cab(cab, &light_status);
    LOG_SRV(" light ctrl: switch=%d level=%d adaptive=%d screen_key=%d startup=%d",
            light_status.night_light_switch, light_status.night_light_gear,
            light_status.adaptive_switch, light_status.screen_key_level,
            light_status.night_light_startup_switch);
    /// 触发灯光状态设置埋点 (EVT_008: 灯光状态设置)
    {
        data_bury_trigger_user_evt_t bury_evt = {
            .evt_source = DATA_BURY_EVT_SOURCE_REMOTE_APP,
            .evt_type = DATA_BURY_USER_EVT_NIGHT_LIGHT,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
    }
    result = app_old_comm_apply_light_control(&light_status) ? 0x01 : 0x00;
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL,
                                               1, &result);
}

void app_old_comm_handle_get_light_status_req(uint8_t cal, uint8_t *cab)
{
    app_old_comm_light_status_t light_status = {0};

    (void)cab;
    if (cal != 0x00) {
        LOGE(" Invalid CAL for get light status (expected 0, got %d)", cal);
    }
    app_old_comm_fill_light_status(&light_status);
    LOG_SRV(" get light status: switch=%d level=%d adaptive=%d screen_key=%d startup=%d",
            light_status.night_light_switch, light_status.night_light_gear,
            light_status.adaptive_switch, light_status.screen_key_level,
            light_status.night_light_startup_switch);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_LIGHT_STATUS,
                                               APP_OLD_COMM_LIGHT_STATUS_CAL, (uint8_t *)&light_status);
}

void app_old_comm_report_light_status(uint8_t arg1, void *val)
{
    app_old_comm_light_status_t light_status = {0};
    ble_manager_context_t *ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);

    if (ble == NULL || ble->ble_is_connected() == false) {
        return;
    }
    app_old_comm_fill_light_status(&light_status);
    LOG_SRV(" report light status: switch=%d level=%d adaptive=%d screen_key=%d startup=%d",
            light_status.night_light_switch, light_status.night_light_gear,
            light_status.adaptive_switch, light_status.screen_key_level,
            light_status.night_light_startup_switch);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_LIGHT_STATUS,
                                               APP_OLD_COMM_LIGHT_STATUS_CAL, (uint8_t *)&light_status);
}

void app_old_comm_handle_dev_abnormal_report(void *cab)
{
    uint8_t report_data[2] = {0};
    LOG_SRV(" Handling dev abnormal report");
    EXCEPTION_CHECK_VOID(cab);
    servers_publish_data_args_t *args_temp = (servers_publish_data_args_t *)cab;
    report_data[0] = args_temp->arg1;
    report_data[1] = args_temp->arg2;
    LOG_SRV(" Abnormal type: %s %d", app_old_comm_get_abnormal_type_str(report_data[0]), report_data[1]);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, 2, report_data);
}

bool app_old_comm_check_if_fault_warning(void)
{
    abnormal_manager_context_t *abnormal =
        (abnormal_manager_context_t *)find_module(ID_MODULE_SYS_ABNORMAL);

    if (abnormal == NULL) {
        return false;
    }
    return abnormal->is_app_communication_forbidden() != 0;
}

void app_old_comm_report_abnormal(uint8_t abnormal_type, uint8_t abnormal_state)
{
    servers_publish_data_args_t args = {
        .arg1 = abnormal_type,
        .arg2 = abnormal_state,
        .val = NULL,
    };
    app_old_comm_handle_dev_abnormal_report(&args);
}

/// APP检查异常
void app_old_comm_handle_dev_abnormal_app_req(uint8_t cal, uint8_t *cab)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    struct rt_device_node *battery = pbus->find_device(ID_DEVICE_BATTERY);
    airplane_mode_context_t *air_ctx = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    EXCEPTION_CHECK_VOID(air_ctx);

    battery_manager_context_t *batt_ctx = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    EXCEPTION_CHECK_VOID(batt_ctx);
 
    int percent = 0;
    if (battery) {
        DEVICE_READ_ARGS(battery, &percent, sizeof(percent));
    } else {
        percent = 100;
    }
    uint8_t report_data[10] = {0};
    uint8_t data_len = 0;
    ///电量不足无法开始
    if (percent <= FORBID_START_BAT_PERCENTAGE) {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE;
    }else {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_CLEARED;
    }
    ///设备低电
    report_data[data_len++] = 0;
    ///设备低电即将关机
    if (percent <= LOW_BAT_PERCENTAGE) {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE;
    }else {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_CLEARED;
    }
    ///设备漏气提醒
    report_data[data_len++] = 0;
    ///旅行锁
    if (air_ctx->is_on_airplane_mode()) {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE;
    }
    else {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_CLEARED;
    }
    ///适配器
    if (batt_ctx->get_charge_status() == CHARGE_STATE_ERROR) {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE;
    }
    else {
        report_data[data_len++] = OLD_PUBLISH_ABNORMAL_STATE_CLEARED;
    }
    /// Log report_data: low bat cannot start, low battery, low bat shutdown soon, air leak, travel lock, adapter error
    LOG_SRV("low_bat_cnst=%d, low_bat=%d, low_bat_shut_soon=%d, air_leak=%d, travel_lock=%d, adapter_error=%d",
        report_data[0], report_data[1], report_data[2], report_data[3], report_data[4], report_data[5]);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_ABNORMAL_INFOR, \
        data_len, report_data);
}

