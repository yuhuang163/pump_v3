/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"
#include "protocol_manager_api.h"
#include "storage_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
app_old_protocol_handler_context_t ctx_app_old_protocol_handler;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
// Module internal global variables
static AppOldCommDeviceInfo app_old_comm_dev_info;
static AppOldCommDeviceStatus app_old_comm_dev_status;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void app_old_comm_handle_heartbeat_req();
static void app_old_comm_handle_get_dev_info_req(uint8_t cal, uint8_t *cab);
static void app_old_comm_handle_set_rtc_req(uint8_t cal, uint8_t *cab);
static void app_old_comm_handle_sys_ctrl_req(uint8_t cal, uint8_t *cab);
static void app_old_comm_handle_get_encrypt_req();
static void app_old_comm_handle_get_status_req();
static void app_old_comm_handle_ctrl_dev_req(uint8_t cal, uint8_t *cab);
static void app_old_comm_handle_set_bind_req(uint8_t cal, uint8_t *cab);
static void app_old_comm_handle_set_heat_req(uint8_t cal, uint8_t *cab);
static void app_old_comm_handle_set_travel_lock_req(uint8_t cal, uint8_t *cab);
static void app_old_comm_parse_notify_status(uint8_t cal, uint8_t *cab);
static void app_old_comm_parse_notify_bind(uint8_t cal, uint8_t *cab);
static void app_old_comm_parse_notify_discon();
static void app_old_comm_report_device_status();
static void app_old_comm_report_bind_status(uint8_t bind_status, uint8_t left_right);

// -------------------------- Request command handling implementations --------------------------
static void app_old_comm_handle_heartbeat_req()
{
    LOG_PROTOCOL(" Sending heartbeat ACK\n");
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_HEARTBEAT, 0, NULL);
}

static void app_old_comm_handle_get_dev_info_req(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling get device info request\n");
    if (cal != 0x04) {
        LOG_PROTOCOL(" Invalid data length for GET_DEV_INFO (expected 4, got %d)\n", cal);
        return;
    }

    uint8_t ack_data[64] = {0};
    uint16_t data_len = 0;

    ack_data[data_len++] = app_old_comm_dev_info.product_type;
    ack_data[data_len++] = app_old_comm_dev_info.hardware_platform;
    ack_data[data_len++] = app_old_comm_dev_info.hardware_ver;
    ack_data[data_len++] = app_old_comm_dev_info.software_ver;
    ack_data[data_len++] = (app_old_comm_dev_info.custom_mode_id >> 0) & 0xFF;
    ack_data[data_len++] = (app_old_comm_dev_info.custom_mode_id >> 8) & 0xFF;
    ack_data[data_len++] = (app_old_comm_dev_info.custom_mode_id >> 16) & 0xFF;
    ack_data[data_len++] = (app_old_comm_dev_info.custom_mode_id >> 24) & 0xFF;
    ack_data[data_len++] = app_old_comm_dev_info.milk_full_ver;
    ack_data[data_len++] = app_old_comm_dev_info.milk_full_switch;
    memcpy(ack_data + data_len, app_old_comm_dev_info.sn, strlen(app_old_comm_dev_info.sn));
    data_len += strlen(app_old_comm_dev_info.sn);
#if BLE_PAYLOAD_CRYPTION
    ack_data[data_len++] = app_old_comm_dev_info.encrypt_status;
    ack_data[data_len++] = app_old_comm_dev_info.encrypt_key;
#endif

    LOG_PROTOCOL(" Sending device info ACK (data len: %d)\n", data_len);
   app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_DEV_INFO, data_len, ack_data);
}

static void app_old_comm_handle_set_rtc_req(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling set RTC request\n");
    if (cal != 0x04) {
        LOG_PROTOCOL(" Invalid data length for SET_RTC (expected 4, got %d)\n", cal);
        return;
    }
    uint32_t utc_time = (cab[3] << 24) | (cab[2] << 16) | (cab[1] << 8) | cab[0];
    LOG_PROTOCOL(" Setting RTC time to 0x%08X\n", utc_time);
    // TODO: Call RTC setting interface
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_RTC, 0, NULL);
}

static void app_old_comm_handle_sys_ctrl_req(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling system control request\n");
    if (cal != 0x01) {
        LOG_PROTOCOL(" Invalid data length for SYS_CTRL (expected 1, got %d)\n", cal);
        return;
    }
    uint8_t ctrl_cmd = cab[0];
    LOG_PROTOCOL(" Executing system control command 0x%02X\n", ctrl_cmd);
    // TODO: Execute system control logic
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SYS_CTRL, 1, &ctrl_cmd);
}

static void app_old_comm_handle_get_encrypt_req()
{
    LOG_PROTOCOL(" Handling get encryption info request\n");
    uint8_t ack_data[2] = {0};
    ack_data[0] = app_old_comm_dev_info.encrypt_status;
    ack_data[1] = app_old_comm_dev_info.encrypt_key;
    LOG_PROTOCOL(" Sending encryption info ACK (status: 0x%02X, key: 0x%02X)\n",
           ack_data[0], ack_data[1]);
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_ENCRYPT, 2, ack_data);
}

static void app_old_comm_handle_get_status_req()
{
    LOG_PROTOCOL(" Handling get device status request\n");
    uint8_t ack_data[45] = {0};
    uint16_t data_len = 0;

    ack_data[data_len++] = app_old_comm_dev_status.battery;
    ack_data[data_len++] = (uint8_t)app_old_comm_dev_status.status;
    ack_data[data_len++] = app_old_comm_dev_status.left_right;
    ack_data[data_len++] = (uint8_t)app_old_comm_dev_status.milk_mode;
    ack_data[data_len++] = app_old_comm_dev_status.milk_gear;
    ack_data[data_len++] = app_old_comm_dev_status.custom_mode;
    ack_data[data_len++] = app_old_comm_dev_status.custom_gear;
    ack_data[data_len++] = app_old_comm_dev_status.custom_freq;
    ack_data[data_len++] = (app_old_comm_dev_status.total_time >> 0) & 0xFF;
    ack_data[data_len++] = (app_old_comm_dev_status.total_time >> 8) & 0xFF;
    ack_data[data_len++] = (app_old_comm_dev_status.mode_time >> 0) & 0xFF;
    ack_data[data_len++] = (app_old_comm_dev_status.mode_time >> 8) & 0xFF;
    ack_data[data_len++] = app_old_comm_dev_status.milk_volume;
    ack_data[data_len++] = app_old_comm_dev_status.error_flag;
    ack_data[data_len++] = (app_old_comm_dev_status.custom_total_time >> 0) & 0xFF;
    ack_data[data_len++] = (app_old_comm_dev_status.custom_total_time >> 8) & 0xFF;
    ack_data[data_len++] = app_old_comm_dev_status.travel_lock;

    LOG_PROTOCOL(" Sending device status ACK (data len: %d)\n", data_len);
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_STATUS, data_len, ack_data);
}

static void app_old_comm_handle_ctrl_dev_req(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling device control request\n");
    if (cal < 0x06) {
        LOG_PROTOCOL(" Invalid data length for CTRL_DEV (expected >=6, got %d)\n", cal);
        return;
    }

    AppOldCommDevStatus new_status = (AppOldCommDevStatus)cab[0];
    AppOldCommMilkMode new_mode = (AppOldCommMilkMode)cab[1];
    uint8_t new_gear = cab[2];

    LOG_PROTOCOL(" Updating device status - Status: %d, Mode: %d, Gear: %d\n",
           new_status, new_mode, new_gear);

    app_old_comm_dev_status.status = new_status;
    app_old_comm_dev_status.milk_mode = new_mode;
    app_old_comm_dev_status.milk_gear = new_gear;
    app_old_comm_dev_status.custom_mode = cab[3];
    app_old_comm_dev_status.custom_gear = cab[4];
    app_old_comm_dev_status.custom_freq = cab[5];

    // TODO: Send parameters to pump valve component
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_CTRL_DEV, 0, NULL);
    app_old_comm_report_device_status();
}

static void app_old_comm_handle_set_bind_req(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling set bind status request\n");
    if (cal != 0x02) {
        LOG_PROTOCOL(" Invalid data length for SET_BIND (expected 2, got %d)\n", cal);
        return;
    }
    uint8_t bind_status = cab[0];
    uint8_t left_right = cab[1];
    LOG_PROTOCOL(" Setting bind status - Status: %d, Side: %d\n", bind_status, left_right);
    // TODO: Update binding status
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_BIND, 2, cab);
    app_old_comm_report_bind_status(bind_status, left_right);
}

static void app_old_comm_handle_set_heat_req(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling set heat request\n");
    if (cal != 0x05) {
        LOG_PROTOCOL(" Invalid data length for SET_HEAT (expected 5, got %d)\n", cal);
        return;
    }
    uint8_t auto_heat = cab[0];
    uint8_t heat_switch = cab[1];
    uint8_t heat_gear = cab[2];
    LOG_PROTOCOL(" Setting heat parameters - Auto: %d, Switch: %d, Gear: %d\n",
           auto_heat, heat_switch, heat_gear);
    // TODO: Execute heating control logic
    //app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_HEAT, 0, NULL);
}

static void app_old_comm_handle_set_travel_lock_req(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling set travel lock request\n");
    if (cal != 0x01) {
        LOG_PROTOCOL(" Invalid data length for SET_TRAVEL_LOCK (expected 1, got %d)\n", cal);
        return;
    }
    app_old_comm_dev_status.travel_lock = cab[0];
    LOG_PROTOCOL(" Setting travel lock status to %d\n", app_old_comm_dev_status.travel_lock);
    // TODO: Execute travel lock control logic
    app_old_comm_cmd_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_TRAVEL_LOCK, 0, NULL);
}

// -------------------------- Report command parsing implementations --------------------------
static void app_old_comm_parse_notify_status(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Parsing status notification (data len: %d)\n", cal);
    if (cal < 0x2D) {
        LOG_PROTOCOL(" Invalid data length for NOTIFY_STATUS (expected >=45, got %d)\n", cal);
        return;
    }

    uint16_t offset = 0;
    app_old_comm_dev_status.battery = cab[offset++];
    app_old_comm_dev_status.status = (AppOldCommDevStatus)cab[offset++];
    app_old_comm_dev_status.left_right = cab[offset++];
    app_old_comm_dev_status.milk_mode = (AppOldCommMilkMode)cab[offset++];
    app_old_comm_dev_status.milk_gear = cab[offset++];

    LOG_PROTOCOL(" Updated status - Battery: %d%%, Status: %d, Mode: %d, Gear: %d\n",
           app_old_comm_dev_status.battery,
           app_old_comm_dev_status.status,
           app_old_comm_dev_status.milk_mode,
           app_old_comm_dev_status.milk_gear);

    // Parse remaining fields
    app_old_comm_dev_status.custom_mode = cab[offset++];
    app_old_comm_dev_status.custom_gear = cab[offset++];
    app_old_comm_dev_status.custom_freq = cab[offset++];
    app_old_comm_dev_status.total_time = (cab[offset+1] << 8) | cab[offset];
    offset += 2;
    app_old_comm_dev_status.mode_time = (cab[offset+1] << 8) | cab[offset];
    offset += 2;
    app_old_comm_dev_status.milk_volume = cab[offset++];
    app_old_comm_dev_status.error_flag = cab[offset++];
    app_old_comm_dev_status.custom_total_time = (cab[offset+1] << 8) | cab[offset];
    offset += 2;
    app_old_comm_dev_status.travel_lock = cab[offset++];
}

static void app_old_comm_parse_notify_bind(uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Parsing bind status notification (data len: %d)\n", cal);
    if (cal != 0x01) {
        LOG_PROTOCOL(" Invalid data length for NOTIFY_BIND (expected 1, got %d)\n", cal);
        return;
    }
    // TODO: Handle binding status update
}

static void app_old_comm_parse_notify_discon()
{
    LOG_PROTOCOL(" Parsing disconnection notification\n");
    // TODO: Handle BLE disconnection logic
}

static void app_old_comm_report_device_status()
{
    LOG_PROTOCOL(" Reporting device status\n");
    uint8_t report_data[45] = {0};
    uint16_t data_len = 0;

    report_data[data_len++] = app_old_comm_dev_status.battery;
    report_data[data_len++] = (uint8_t)app_old_comm_dev_status.status;
    report_data[data_len++] = app_old_comm_dev_status.left_right;
    report_data[data_len++] = (uint8_t)app_old_comm_dev_status.milk_mode;
    report_data[data_len++] = app_old_comm_dev_status.milk_gear;
    report_data[data_len++] = app_old_comm_dev_status.custom_mode;
    report_data[data_len++] = app_old_comm_dev_status.custom_gear;
    report_data[data_len++] = app_old_comm_dev_status.custom_freq;
    report_data[data_len++] = (app_old_comm_dev_status.total_time >> 0) & 0xFF;
    report_data[data_len++] = (app_old_comm_dev_status.total_time >> 8) & 0xFF;
    report_data[data_len++] = (app_old_comm_dev_status.mode_time >> 0) & 0xFF;
    report_data[data_len++] = (app_old_comm_dev_status.mode_time >> 8) & 0xFF;
    report_data[data_len++] = app_old_comm_dev_status.milk_volume;
    report_data[data_len++] = app_old_comm_dev_status.error_flag;
    report_data[data_len++] = (app_old_comm_dev_status.custom_total_time >> 0) & 0xFF;
    report_data[data_len++] = (app_old_comm_dev_status.custom_total_time >> 8) & 0xFF;
    report_data[data_len++] = app_old_comm_dev_status.travel_lock;

    app_old_comm_cmd_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_STATUS, data_len, report_data);
}

static void app_old_comm_report_bind_status(uint8_t bind_status, uint8_t left_right)
{
    LOG_PROTOCOL(" Reporting bind status - Status: %d, Side: %d\n", bind_status, left_right);
    uint8_t report_data[2] = {bind_status, left_right};
    app_old_comm_cmd_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_BIND, 2, report_data);
}

static void app_old_comm_poll_timer_handler(void *handle)
{
    LOG_PROTOCOL(" Poll timer triggered - reporting device status\n");
    app_old_comm_report_device_status();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
// Handle request commands (APP->device)
void app_old_comm_handle_request_cmd(AppOldCommCmdId cid, uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling %s request (data len: %d)\n",
           app_old_comm_get_cmd_id_str(cid), cal);

    // Print command data if available
    if (cal > 0 && cab != NULL) {
        LOG_PROTOCOL(" Command data: ");
        LOG_HEX("app_recv_data hex", cab, cal);
    }

    switch (cid) {
        case APP_OLD_COMM_CID_HEARTBEAT:
            app_old_comm_handle_heartbeat_req();
            break;
        case APP_OLD_COMM_CID_GET_DEV_INFO:
            app_old_comm_handle_get_dev_info_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_DEV_SN:
        case APP_OLD_COMM_CID_GET_SN:
            app_old_comm_handle_get_sn_req();
        case APP_OLD_COMM_CID_SET_SN:
            app_old_comm_handle_set_sn_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_RTC:
            app_old_comm_handle_set_rtc_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SYS_CTRL:
            app_old_comm_handle_sys_ctrl_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_ENCRYPT:
            app_old_comm_handle_get_encrypt_req();
            break;
        case APP_OLD_COMM_CID_GET_STATUS:
            app_old_comm_handle_get_status_req();
            break;
        case APP_OLD_COMM_CID_CTRL_DEV:
            app_old_comm_handle_ctrl_dev_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_BIND:
            app_old_comm_handle_set_bind_req(cal, cab);
            break;
        //case APP_OLD_COMM_CID_SET_HEAT:
        //    app_old_comm_handle_set_heat_req(cal, cab);
        //    break;
        case APP_OLD_COMM_CID_SET_TRAVEL_LOCK:
            app_old_comm_handle_set_travel_lock_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_TRIPLE:
            app_old_comm_handle_get_triple_req();
            break;
        case APP_OLD_COMM_CID_SET_PRODUCTID:
            app_old_comm_handle_set_productid_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_DEVICEID:
            app_old_comm_handle_set_deviceid_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_ENCRYPTIONKEY:
            app_old_comm_handle_set_encryption_key_req(cal, cab);
            break;
            break;
        default:
            LOG_PROTOCOL(" Unknown request command 0x%02X\n", cid);
            break;
    }
}

// Handle response/report commands (device->APP)
void app_old_comm_handle_response_cmd(AppOldCommCmdType ct, AppOldCommCmdId cid, uint8_t cal, uint8_t *cab)
{
    LOG_PROTOCOL(" Handling %s (data len: %d)\n", app_old_comm_get_cmd_id_str(cid), cal);

    switch (cid) {
        case APP_OLD_COMM_CID_NOTIFY_STATUS:
            app_old_comm_parse_notify_status(cal, cab);
            break;
        case APP_OLD_COMM_CID_NOTIFY_BIND:
            app_old_comm_parse_notify_bind(cal, cab);
            break;
        case APP_OLD_COMM_CID_NOTIFY_DISCON:
            app_old_comm_parse_notify_discon();
            break;
        default:
            LOG_PROTOCOL(" Unhandled response command %s (0x%02X)\n",
                   app_old_comm_get_cmd_id_str(cid), cid);
            break;
    }
}

AppOldCommDeviceInfo* app_old_comm_dev_info_get(void)
{
    return &app_old_comm_dev_info;
}

AppOldCommDeviceStatus* app_old_comm_dev_status_get(void)
{
    return &app_old_comm_dev_status;
}

// Initialize device information structure
void app_old_comm_variable_init(void)
{
    memset(&app_old_comm_dev_info, 0, sizeof(AppOldCommDeviceInfo));
    memset(&app_old_comm_dev_status, 0, sizeof(AppOldCommDeviceStatus));
        // Initialize device information
    AppOldCommDeviceInfo init_info = {
        .product_type = 0x01,          // M9 product
        .hardware_platform = 0x01,     // JL platform
        .hardware_ver = 0x02,          // Hardware version V2
        .software_ver = 0x03,          // Software version V3
        .custom_mode_id = 0x12345678,  // Custom mode ID
        .milk_full_ver = 0x01,         // Milk full detection version
        .milk_full_switch = 0x01,      // Milk full detection enabled
        .sn = "DEVICE1234567890ABCDEF",// SN code
        .encrypt_status = 0x00,        // Encrypted
        .encrypt_key = 0xAB            // Encryption key
    };
    memcpy(&app_old_comm_dev_info, &init_info, sizeof(AppOldCommDeviceInfo));
    // Simulate device status
    AppOldCommDeviceStatus dev_status = {
        .battery = 80,                 // 80% battery
        .status = APP_OLD_COMM_DEV_RUNNING, // Running
        .left_right = 0x00,            // Left side
        .milk_mode = APP_OLD_COMM_MODE_SUCTION, // Suction mode
        .milk_gear = 0x05,             // Gear 5
        .travel_lock = 0x00            // Not locked
    };
    memcpy(&app_old_comm_dev_status, &dev_status, sizeof(AppOldCommDeviceStatus));
}

void app_old_protocol_ctx_init(void)
{
    memset(&ctx_app_old_protocol_handler, 0, sizeof(app_old_protocol_handler_context_t));
    CREATE_TIMER(&ctx_app_old_protocol_handler.poll_timer, "protocol_poll_timer", &ctx_app_old_protocol_handler,
                 OS_TIMER_PERIODIC, app_old_comm_poll_timer_handler);
    START_TIMER(ctx_app_old_protocol_handler.poll_timer, APP_OLD_COMM_HEARTBEAT_INTERVAL);
}