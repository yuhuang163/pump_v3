/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_old_protocol_parser.h"
#include "protocol_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static app_old_comm_ring_queue_t app_old_comm_rx_queue = {0};
static uint8_t app_old_comm_frame_buf[APP_OLD_COMM_MAX_PACKET_LEN];


/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
// Function prototype declarations
static app_old_comm_par_re_result_e app_old_comm_protocol_parse(uint8_t *frame_buf, uint16_t frame_len);

// Checksum calculation (sum bytes and invert)
static uint8_t app_old_comm_calc_checksum(uint8_t *data, uint16_t len)
{
    uint32_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += data[i];
    }
    uint8_t checksum = ~((uint8_t)sum);
    LOG_PROTOCOL(" Calculated checksum: 0x%02X (for %d bytes)", checksum, len);
    return checksum;
}

// Get parse result string for logging
static const char* app_old_comm_get_parse_result_str(app_old_comm_par_re_result_e result)
{
    switch (result) {
        case APP_OLD_COMM_PARSE_SUCCESS: return "SUCCESS";
        case APP_OLD_COMM_PARSE_ERR_HEADER: return "HEADER_ERROR";
        case APP_OLD_COMM_PARSE_ERR_CHECKSUM: return "CHECKSUM_ERROR";
        case APP_OLD_COMM_PARSE_ERR_LENGTH: return "LENGTH_ERROR";
        case APP_OLD_COMM_PARSE_ERR_CMD_UNKNOWN: return "UNKNOWN_COMMAND";
        case APP_OLD_COMM_PARSE_ERR_INCOMPLETE: return "INCOMPLETE_DATA";
        case APP_OLD_COMM_PARSE_ERR_TIMEOUT: return "FRAME_TIMEOUT";
        default: return "UNKNOWN_ERROR";
    }
}

// Get command ID string for logging
const char* app_old_comm_get_cmd_id_str(app_old_comm_cmd_id_e cid)
{
    switch (cid) {
        case APP_OLD_COMM_CID_HEARTBEAT: return "HEARTBEAT";
        case APP_OLD_COMM_CID_GET_SN: return "GET_SN";
        case APP_OLD_COMM_CID_SET_SN: return "SET_SN";
        case APP_OLD_COMM_CID_GET_STATUS: return "GET_STATUS";
        case APP_OLD_COMM_CID_GET_BAND: return "GET_BAND";
        case APP_OLD_COMM_CID_SET_PRIVATE_PARAM: return "SET_PRIVATE_PARAM";
        case APP_OLD_COMM_CID_GET_UD_ID: return "GET_UD_ID";
        case APP_OLD_COMM_CID_GET_TASTE_MODE_STATE: return "GET_TASTE_MODE_STATE";
        case APP_OLD_COMM_CID_GET_BURY_POINT_NUM: return "GET_BURY_POINT_NUM";
        case APP_OLD_COMM_CID_GET_BURY_POINT_DATA: return "GET_BURY_POINT_DATA";
        case APP_OLD_COMM_CID_CTRL_DEV: return "CTRL_DEV";
        case APP_OLD_COMM_CID_SET_BIND: return "SET_BIND";
        case APP_OLD_COMM_CID_CTRL_DEV_STATUS: return "CTRL_DEV_STATUS";
        case APP_OLD_COMM_CID_USER_DEFINE: return "USER_DEFINE";
        case APP_OLD_COMM_CID_PARA_DOWNLOAD: return "PARA_DOWNLOAD";
        case APP_OLD_COMM_CID_CLEAR_CUSTOM_MODE: return "CLEAR_CUSTOM_MODE";
        case APP_OLD_COMM_CID_NOTIFY_CUSTOM_MODE_ID: return "NOTIFY_CUSTOM_MODE_ID";
        case APP_OLD_COMM_CID_SET_BILATERAL_PUMP_PARAM: return "SET_BILATERAL_PUMP_PARAM";
        case APP_OLD_COMM_CID_TASTE_CTRL: return "TASTE_CTRL";
        case APP_OLD_COMM_CID_SET_TRAVEL_LOCK: return "SET_TRAVEL_LOCK";
        case APP_OLD_COMM_CID_NOTIFY_STATUS: return "NOTIFY_STATUS";
        case APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL: return "NOTIFY_V3_DEV_ABNORMAL";
        case APP_OLD_COMM_CID_NOTIFY_BIND: return "NOTIFY_BIND";
        case APP_OLD_COMM_CID_NOTIFY_DISCON: return "NOTIFY_DISCON";
        case APP_OLD_COMM_CID_TASTE_REPORT: return "TASTE_REPORT";
        case APP_OLD_COMM_CID_BURY_POINT_REPORT: return "BURY_POINT_REPORT";
        case APP_OLD_COMM_CID_GET_DEV_INFO: return "GET_DEV_INFO";
        case APP_OLD_COMM_CID_GET_DEV_SN: return "GET_DEV_SN";
        case APP_OLD_COMM_CID_SET_RTC: return "SET_RTC";
        case APP_OLD_COMM_CID_GET_TRIPLE: return "GET_TRIPLE";
        case APP_OLD_COMM_CID_SET_PRODUCTID: return "SET_PRODUCTID";
        case APP_OLD_COMM_CID_SET_DEVICEID: return "SET_DEVICEID";
        case APP_OLD_COMM_CID_SET_ENCRYPTIONKEY: return "SET_ENCRYPTIONKEY";
        case APP_OLD_COMM_CID_SYS_CTRL: return "SYS_CTRL";
        case APP_OLD_COMM_CID_GET_ENCRYPT: return "GET_ENCRYPT";
        case APP_OLD_COMM_CID_GET_ABNORMAL_INFOR: return "GET_ABNORMAL_INFOR";
        case APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL: return "LIGHT_CTRL";
        case APP_OLD_COMM_CID_GET_LIGHT_STATUS: return "GET_LIGHT_STATUS";
        case APP_OLD_COMM_CID_NOTIFY_LIGHT_STATUS: return "NOTIFY_LIGHT_STATUS";
        default: return "UNKNOWN";
    }
}

// Get command type string for logging
const char* app_old_comm_get_cmd_type_str(app_old_commcmd_type_e ct)
{
    switch (ct) {
        case APP_OLD_COMM_CT_REQ: return "REQ";
        case APP_OLD_COMM_CT_ACK: return "ACK";
        case APP_OLD_COMM_CT_NACK: return "NACK";
        case APP_OLD_COMM_CT_NOTIFY: return "NOTIFY";
        case APP_OLD_COMM_CT_DEVICE: return "DEVICE";
        default: return "UNKNOWN";
    }
}

// Initialize ring queue
static void app_old_comm_ring_queue_reset(void)
{
    memset(app_old_comm_rx_queue.buffer, 0, APP_OLD_COMM_RX_QUEUE_SIZE);
    app_old_comm_rx_queue.head = 0;
    app_old_comm_rx_queue.tail = 0;
    app_old_comm_rx_queue.len = 0;
    app_old_comm_rx_queue.last_recv_time = 0;
    LOG_PROTOCOL(" Ring queue initialized");
}

// Add data to queue
static bool app_old_comm_ring_queue_push(uint8_t *data, uint16_t len)
{
    if (app_old_comm_rx_queue.len + len > APP_OLD_COMM_RX_QUEUE_SIZE) {
        LOG_PROTOCOL(" Queue overflow - cannot add %d bytes (current len: %d)", len, app_old_comm_rx_queue.len);
        return false;
    }

    for (uint16_t i = 0; i < len; i++) {
        app_old_comm_rx_queue.buffer[app_old_comm_rx_queue.tail] = data[i];
        app_old_comm_rx_queue.tail = (app_old_comm_rx_queue.tail + 1) % APP_OLD_COMM_RX_QUEUE_SIZE;
    }
    app_old_comm_rx_queue.len += len;
    app_old_comm_rx_queue.last_recv_time = get_timestamp();
    LOG_PROTOCOL(" Added %d bytes to queue (new len: %d)", len, app_old_comm_rx_queue.len);
    return true;
}

// Extract data from queue
static uint16_t app_old_comm_ring_queue_pop(uint8_t *buf, uint16_t max_len)
{
    uint16_t take_len = (app_old_comm_rx_queue.len < max_len) ? app_old_comm_rx_queue.len : max_len;
    if (take_len == 0) return 0;

    for (uint16_t i = 0; i < take_len; i++) {
        buf[i] = app_old_comm_rx_queue.buffer[app_old_comm_rx_queue.head];
        app_old_comm_rx_queue.head = (app_old_comm_rx_queue.head + 1) % APP_OLD_COMM_RX_QUEUE_SIZE;
    }
    app_old_comm_rx_queue.len -= take_len;
    LOG_PROTOCOL(" Popped %d bytes from queue (remaining len: %d)", take_len, app_old_comm_rx_queue.len);
    return take_len;
}

// Clear queue data
static void app_old_comm_ring_queue_clear(void)
{
    uint16_t cleared_len = app_old_comm_rx_queue.len;
    app_old_comm_rx_queue.head = 0;
    app_old_comm_rx_queue.tail = 0;
    app_old_comm_rx_queue.len = 0;
    LOG_PROTOCOL(" Cleared queue (removed %d bytes)", cleared_len);
}

// Peek queue header data (without removing)
static uint8_t app_old_comm_ring_queue_peek(uint16_t offset)
{
    if (offset >= app_old_comm_rx_queue.len) return 0;
    uint16_t pos = (app_old_comm_rx_queue.head + offset) % APP_OLD_COMM_RX_QUEUE_SIZE;
    return app_old_comm_rx_queue.buffer[pos];
}

// Frame timeout detection
static bool app_old_comm_is_frame_timeout(void)
{
    uint32_t timeout_ms = get_timestamp();
    bool timeout = (app_old_comm_rx_queue.len > 0) &&
                  ((timeout_ms=get_timestamp_ms(app_old_comm_rx_queue.last_recv_time)) > APP_OLD_COMM_FRAME_TIMEOUT_MS);

    if (timeout) {
        LOG_PROTOCOL(" Frame timeout detected (last recv: %ums ago)", timeout_ms);
    }
    return timeout;
}

// Extract complete frame from queue
int app_old_protocol_assemble_packet(uint8_t *data, uint16_t len, app_old_protocol_frame_callback_t callback)
{
    LOG_PROTOCOL(" Starting frame extraction (queue len: %d)", app_old_comm_rx_queue.len);
    app_old_comm_ring_queue_push(data, len);

    int parse_result = APP_OLD_COMM_PARSE_ERR_HEADER; // Default result if no frame found
    uint16_t frames_parsed = 0;                       // Count of successfully parsed frames

    // Limit to parse maximum 4 frames per call
    const uint16_t MAX_PARSE_FRAMES = 4;

    // Loop to try multiple headers if needed - limit to 4 frames
    for (uint16_t i = 0; i < MAX_PARSE_FRAMES; i++) {

        // 1 Find frame header (0xAA55)
        uint16_t header_pos = 0;
        bool header_found = false;
        uint16_t max_search = (app_old_comm_rx_queue.len >= 2) ? (app_old_comm_rx_queue.len - 1) : 0;

        while (header_pos < max_search) {
            uint8_t byte1 = app_old_comm_ring_queue_peek(header_pos);
            uint8_t byte2 = app_old_comm_ring_queue_peek(header_pos + 1);

            if ((byte1 << 8 | byte2) == APP_OLD_COMM_SOP_FCF) {
                header_found = true;
                break;
            }
            header_pos++;
        }

        if (!header_found) {
            LOG_PROTOCOL(" Frame header (0xAA55) not found (queue len: %d)", app_old_comm_rx_queue.len);

            // Clear invalid/garbage packets if queue is too large
            // Criteria:
            // 1. Queue length >= MAX_PACKET_LEN (256 bytes) - definitely garbage
            // 2. Queue length >= MAX_PACKET_LEN/2 (128 bytes) - likely garbage if no header found
            // This prevents invalid data from occupying queue space indefinitely
            if (app_old_comm_rx_queue.len >= APP_OLD_COMM_MAX_PACKET_LEN / 2) {
                LOG_PROTOCOL(" Queue too large (%d >= %d bytes) with no header, clearing garbage data",
                             app_old_comm_rx_queue.len, APP_OLD_COMM_MAX_PACKET_LEN / 2);
                app_old_comm_ring_queue_clear();
            }

            // If we already parsed some frames, return success; otherwise return header error
            if (frames_parsed > 0) {
                LOG_PROTOCOL(" No more headers found, returning success (parsed %d frames)", frames_parsed);
                return APP_OLD_COMM_PARSE_SUCCESS;
            }
            return APP_OLD_COMM_PARSE_ERR_HEADER;
        }

        LOG_PROTOCOL(" Found frame header at position %d", header_pos);

        // 2 Clear invalid data before header
        if (header_pos > 0) {
            uint8_t dummy[APP_OLD_COMM_RX_QUEUE_SIZE];
            app_old_comm_ring_queue_pop(dummy, header_pos);
            LOG_PROTOCOL(" Removed %d invalid bytes before header", header_pos);
        }

        // 3 Check minimum length for parsing (need at least 5 bytes to read CAL field)
        // Header(2) + CT(1) + CID(1) + CAL(1) = 5 bytes minimum
        if (app_old_comm_rx_queue.len < 5) {
            app_old_comm_ring_queue_clear();
            LOG_PROTOCOL(" Incomplete frame - need at least 5 bytes to read CAL, have %d", app_old_comm_rx_queue.len);
            return APP_OLD_COMM_PARSE_ERR_INCOMPLETE;
        }

        // 4 Parse parameter length field (CAL)
        uint8_t cal = app_old_comm_ring_queue_peek(4);
        uint16_t total_frame_len = 2 + 1 + 1 + 1 + cal + 1; // header+CT+CID+CAL+data+checksum

        LOG_PROTOCOL(" Parsed frame length: %d bytes (CAL: %d)", total_frame_len, cal);

        // Verify frame length - if invalid, clear queue and return error
        if (total_frame_len < 6 || total_frame_len > APP_OLD_COMM_MAX_PACKET_LEN) {
            app_old_comm_ring_queue_clear();
            LOG_PROTOCOL("Invalid frame length: %d, clearing queue", total_frame_len);
            return APP_OLD_COMM_PARSE_ERR_LENGTH;
        }
        // 5 Extract complete frame data
        uint16_t read_len = app_old_comm_ring_queue_pop(app_old_comm_frame_buf, total_frame_len);
        if (read_len != total_frame_len) {
            LOG_PROTOCOL(" Failed to extract frame - read %d of %d bytes", read_len, total_frame_len);
            return APP_OLD_COMM_PARSE_ERR_LENGTH;
        }

        // 6 Verify checksum
        uint8_t recv_checksum = app_old_comm_frame_buf[total_frame_len - 1];
        uint8_t calc_check = app_old_comm_calc_checksum(app_old_comm_frame_buf, total_frame_len - 1);
        if (recv_checksum != calc_check) {
            LOG_PROTOCOL(" Checksum mismatch - received 0x%02X, calculated 0x%02X, clearing queue", recv_checksum,
                         calc_check);
            // Checksum error indicates data corruption, clear entire queue to avoid parsing garbage data
            app_old_comm_ring_queue_clear();
            return APP_OLD_COMM_PARSE_ERR_CHECKSUM;
        }

        // Successfully parsed a complete and valid frame
        callback(app_old_comm_frame_buf, total_frame_len);
        frames_parsed++;
        parse_result = APP_OLD_COMM_PARSE_SUCCESS;
        LOG_PROTOCOL(" Successfully extracted frame #%d (len: %d, remaining queue: %d)", frames_parsed,
                     total_frame_len, app_old_comm_rx_queue.len);

        // If queue is empty, no need to continue
        if (app_old_comm_rx_queue.len == 0) {
            LOG_PROTOCOL(" Queue empty after parsing %d frame(s), returning success", frames_parsed);
            return APP_OLD_COMM_PARSE_SUCCESS;
        }

        // Continue loop to try parsing next frame (up to MAX_PARSE_FRAMES)
    }

    // Loop completed - return result based on what was parsed
    if (frames_parsed > 0) {
        LOG_PROTOCOL(" Parsed %d frame(s) (max %d reached or no more frames)", frames_parsed, MAX_PARSE_FRAMES);
        return APP_OLD_COMM_PARSE_SUCCESS;
    }

    return parse_result;
}

// Protocol parsing core interface
int app_old_protocol_frame_deserialize(uint8_t *frame_buf, uint16_t len, old_protocol_frame_node_t *frame)
{
    // 1. Verify frame header
    uint16_t header = (frame_buf[0] << 8) | frame_buf[1];
    if (header != APP_OLD_COMM_SOP_FCF) {
        LOG_PROTOCOL(" Invalid frame header 0x%04X (expected 0x%04X)",
               header, APP_OLD_COMM_SOP_FCF);
        return APP_OLD_COMM_PARSE_ERR_HEADER;
    }

    // 2. Parse core fields
    app_old_commcmd_type_e ct = (app_old_commcmd_type_e)frame_buf[2];
    app_old_comm_cmd_id_e cid = (app_old_comm_cmd_id_e)frame_buf[3];
    uint8_t cal = frame_buf[4];
    uint8_t *cab = frame_buf + 5;
#if 0
    // 对除了传输加密key和测试以外的所有指令数据进行加密
    AppOldCommDeviceInfo* comm_dev_info = app_old_comm_dev_info_get();
    if (comm_dev_info->encrypt_status == 1) {
        if ((cid != APP_OLD_COMM_CID_GET_ENCRYPT) && (cid != APP_OLD_COMM_CID_GET_DEV_INFO)) {
            app_old_protocol_data_cryption(comm_dev_info->encrypt_key, frame_buf+5, cal); //解密密ble数据
        }
    }
    LOG_HEX("app_recv_data cryption hex", frame_buf, frame_len);
#endif
    frame->sof = ((uint16_t)frame_buf[1] << 8) | frame_buf[0];
    frame->ct = frame_buf[2];
    frame->cid = frame_buf[3];
    frame->cal = frame_buf[4];
    frame->cab = frame_buf + 5;
    frame->checksum = frame_buf[4 + frame->cal + 1];
    LOG_PROTOCOL(" Parsed frame - Type: %s (0x%02X), Command: %s (0x%02X), Data len: %d",
           app_old_comm_get_cmd_type_str(ct), frame->ct,
           app_old_comm_get_cmd_id_str(cid), frame->cid,
           frame->cal);

    // 3. Dispatch processing
    switch (ct) {
        case APP_OLD_COMM_CT_REQ:
            break;
        case APP_OLD_COMM_CT_ACK:
        case APP_OLD_COMM_CT_NACK:
        case APP_OLD_COMM_CT_NOTIFY:
        case APP_OLD_COMM_CT_DEVICE:
            break;
        default:
            LOG_PROTOCOL(" Unknown command type 0x%02X", ct);
            return APP_OLD_COMM_PARSE_ERR_CMD_UNKNOWN;
    }

    return APP_OLD_COMM_PARSE_SUCCESS;
}

static void app_commication_send_data_via_ble(uint8_t *data, uint16_t data_len)
{
    const uint16_t MAX_SEND_LEN = 128;
    protocol_manager_context_t *ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    /* 参数验证 */
    if (!data || data_len == 0) {
        LOG_PROTOCOL("invalid data or length");
        return;
    }

    if (!ctx) {
        LOG_PROTOCOL("protocol manager context is NULL");
        return;
    }

    /* 长度限制 */
    if (data_len > MAX_SEND_LEN) {
        LOG_PROTOCOL("data_len(%d) > MAX_SEND_LEN(%d), truncate", data_len, MAX_SEND_LEN);
        data_len = MAX_SEND_LEN;
    }

    /* 通过 BLE 发送 */
    if (ctx->priv.ble_ctx && ctx->priv.ble_ctx->ble_is_connected &&
        ctx->priv.ble_ctx->ble_is_connected()) {
        if (ctx->priv.ble_ctx->dispatch_ble_event_wrapper) {
            int ret = ctx->priv.ble_ctx->dispatch_ble_event_wrapper(
                ctx->priv.ble_ctx, BLE_MSG_MASTER_SEND_DATA, data, data_len);
            if (ret < 0) {
                LOG_PROTOCOL("send via BLE failed, ret=%d", ret);
            } else {
                LOG_PROTOCOL("send data via BLE[%d]", data_len);
            }
        } else {
            LOG_PROTOCOL("ble dispatch wrapper missing");
        }
    }
    else {
        LOG_PROTOCOL("ble is not connected");
    }

}

int app_old_protocol_frame_builder_send(app_old_commcmd_type_e cmdtype, app_old_comm_cmd_id_e cid, uint8_t data_len, uint8_t *data)
{
    uint8_t send_buf[APP_OLD_COMM_MAX_PACKET_LEN] = {0};
    uint16_t len = 0;

    send_buf[len++] = (APP_OLD_COMM_SOP_FCF >> 8) & 0xFF;
    send_buf[len++] = (APP_OLD_COMM_SOP_FCF >> 0) & 0xFF;
    send_buf[len++] = cmdtype;
    send_buf[len++] = (uint8_t)cid;
    send_buf[len++] = data_len;
    if (data_len > 0 && data != NULL) {
        memcpy(send_buf + len, data, data_len);
        len += data_len;
    }
#if 0
        AppOldCommDeviceInfo *app_old_comm_dev_info = app_old_comm_dev_info_get();
        // 对除了传输加密key和测试以外的所有指令数据进行加密
        if (app_old_comm_dev_info->encrypt_status == 1) {
            if ((cid != APP_OLD_COMM_CID_GET_ENCRYPT) && (cid != APP_OLD_COMM_CID_GET_DEV_INFO)) {
                app_old_protocol_data_cryption(app_old_comm_dev_info->encrypt_key, &send_buf[len], data_len); // 加密ble数据
            }
        }
#endif
    send_buf[len++] = app_old_comm_calc_checksum(send_buf, len);
#if BLE_PAYLOAD_CRYPTION
    LOG_HEX("app_send_data hex", send_buf, len);
#endif
    LOG_PROTOCOL(" Sending cmdtype %s for %s (len: %d)", app_old_comm_get_cmd_type_str(cmdtype), app_old_comm_get_cmd_id_str(cid), len);
    app_commication_send_data_via_ble(send_buf, len);
    return true;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/




