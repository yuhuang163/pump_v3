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
static void app_old_comm_set_night_light(uint8_t adaptive_dimming_switch)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
    if (led->hook->set_night_light_adaptive_switch) {
        led->hook->set_night_light_adaptive_switch(led, adaptive_dimming_switch);
    }
    LOG_SRV(" Night light set: adaptive=%d", adaptive_dimming_switch);
}

static void app_old_comm_set_device_bind_status(uint8_t device_bind_status)
{
    ble_manager_context_t *ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_VOID(ble);
    ///底层有三个状态 1需要加1对齐到绑定状态
    if (device_bind_status == true) {
        device_bind_status = BLE_BIND_STATE_COMPLETE;
    }
    //if (ble->ble_bind_state_get() != BLE_BIND_STATE_COMPLETE) {
        ble->ble_bind_state_set(device_bind_status);
        LOG_SRV(" Device bind status set: %d", device_bind_status);
    //}
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
    sys_status.adaptive_dimming_switch = cab[5];

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
    app_old_comm_set_night_light(sys_status.adaptive_dimming_switch);
    // Apply device bind status
    app_old_comm_set_device_bind_status(sys_status.device_bind_status);
    // TODO: Apply other sys_status settings (language, screen lock, brightness, massage time)

    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_CTRL_DEV_STATUS, cal, cab);
}

void app_old_comm_handle_pump_ble_rgb_set_ctrl_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling night light control request (data len: %d)", cal);
    EXCEPTION_CHECK_VOID(cab);
    
    uint8_t result = 0x00; // 默认失败
    
    /// 检查数据长度：需要9字节参数
    if (cal < 9) {
        LOGE(" Invalid data length for night light control (expected >=9, got %d)", cal);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL, 1, &result);
        return;
    }
    
    /// 解析参数
    /// byte0: 自动亮灯（未使用，跳过）
    uint8_t night_light_switch = cab[1];      /// byte1: 夜灯开关（0=关闭，1=打开）
    uint8_t night_light_level = cab[2];      /// byte2: 夜灯亮度等级（1-10）
    /// byte3-4: 夜灯灯光控制时间（2字节，未使用，跳过）
    
    LOG_SRV(" Night light control: switch=%d, level=%d", night_light_switch, night_light_level);
    
    /// 验证参数范围
    if (night_light_switch > 1) {
        LOGE(" Invalid night light switch value: %d (expected 0 or 1)", night_light_switch);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL, 1, &result);
        return;
    }
    
    if (night_light_level < 1 || night_light_level > 10) {
        LOGE(" Invalid night light level: %d (expected 1-10)", night_light_level);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL, 1, &result);
        return;
    }
    
    /// 获取LED管理器
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    if (led == NULL || led->hook == NULL) {
        LOGE(" LED manager not found or hook is NULL");
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL, 1, &result);
        return;
    }
    
    /// 关闭夜灯自适应调光（如果支持）
    //if (led->hook->set_night_light_adaptive_dimming) {
      //led->hook->set_night_light_adaptive_dimming(led, 0);
    //  }
    
    /// 计算实际亮度值：将1-10映射到LED_LEVEL_MAX（100）
    /// 映射公式：level = (协议等级 - 1) * (LED_LEVEL_MAX - 1) / 9 + 1
    /// 简化：level = (协议等级 - 1) * 11 + 1，或者 level = 协议等级 * 10
    /// 使用线性映射：1->10, 2->20, ..., 10->100
    uint8_t actual_level = night_light_level * 10;
    if (actual_level > LED_LEVEL_MAX) {
        actual_level = LED_LEVEL_MAX;
    }
    
    /// 如果开关为关闭，则亮度设为0
    if (night_light_switch == 0) {
        actual_level = 0;
    }
    
    /// 设置夜灯亮度
    if (led->hook->set_night_light_level) {
        led->hook->set_night_light_level(led, actual_level);
        result = 0x01; // 成功
        LOG_SRV(" Night light set successfully: switch=%d, level=%d (actual=%d)", 
                night_light_switch, night_light_level, actual_level);
    } else {
        LOGE(" set_night_light_level hook not available");
    }
    
    /// 发送响应（1字节状态：0x00=错误，0x01=成功）
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL, 1, &result);
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

