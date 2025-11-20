/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"
#include "app_protocol_parse.h"
#include "protocol_manager_api.h"
#include <stdio.h>

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static protocol_manager_context_t *thiz = NULL;
static AppOldCommRingQueue app_old_comm_rx_queue;
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
static AppOldCommParseResult app_old_comm_protocol_parse(uint8_t *frame_buf, uint16_t frame_len);

// Get parse result string for logging
static const char* app_old_comm_get_parse_result_str(AppOldCommParseResult result)
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

// Initialize ring queue
static void app_old_comm_ring_queue_init(AppOldCommRingQueue *queue)
{
    memset(queue->buffer, 0, APP_OLD_COMM_RX_QUEUE_SIZE);
    queue->head = 0;
    queue->tail = 0;
    queue->len = 0;
    queue->last_recv_time = 0;
    LOG_PROTOCOL(" Ring queue initialized\n");
}

// Add data to queue
static bool app_old_comm_ring_queue_push(AppOldCommRingQueue *queue, uint8_t *data, uint16_t len)
{
    if (queue->len + len > APP_OLD_COMM_RX_QUEUE_SIZE) {
        LOG_PROTOCOL(" Queue overflow - cannot add %d bytes (current len: %d)\n", len, queue->len);
        return false;
    }

    for (uint16_t i = 0; i < len; i++) {
        queue->buffer[queue->tail] = data[i];
        queue->tail = (queue->tail + 1) % APP_OLD_COMM_RX_QUEUE_SIZE;
    }
    queue->len += len;
    queue->last_recv_time = get_timestamp();
    LOG_PROTOCOL(" Added %d bytes to queue (new len: %d)\n", len, queue->len);
    return true;
}

// Extract data from queue
static uint16_t app_old_comm_ring_queue_pop(AppOldCommRingQueue *queue, uint8_t *buf, uint16_t max_len)
{
    uint16_t take_len = (queue->len < max_len) ? queue->len : max_len;
    if (take_len == 0) return 0;

    for (uint16_t i = 0; i < take_len; i++) {
        buf[i] = queue->buffer[queue->head];
        queue->head = (queue->head + 1) % APP_OLD_COMM_RX_QUEUE_SIZE;
    }
    queue->len -= take_len;
    LOG_PROTOCOL(" Popped %d bytes from queue (remaining len: %d)\n", take_len, queue->len);
    return take_len;
}

// Clear queue data
static void app_old_comm_ring_queue_clear(AppOldCommRingQueue *queue)
{
    uint16_t cleared_len = queue->len;
    queue->head = 0;
    queue->tail = 0;
    queue->len = 0;
    LOG_PROTOCOL(" Cleared queue (removed %d bytes)\n", cleared_len);
}

// Peek queue header data (without removing)
static uint8_t app_old_comm_ring_queue_peek(AppOldCommRingQueue *queue, uint16_t offset)
{
    if (offset >= queue->len) return 0;
    uint16_t pos = (queue->head + offset) % APP_OLD_COMM_RX_QUEUE_SIZE;
    return queue->buffer[pos];
}

// Frame timeout detection
static bool app_old_comm_is_frame_timeout(AppOldCommRingQueue *queue)
{
    uint32_t timeout_ms = get_timestamp();
    bool timeout = (queue->len > 0) &&
                  ((timeout_ms=get_timestamp_ms(queue->last_recv_time)) > APP_OLD_COMM_FRAME_TIMEOUT_MS);

    if (timeout) {
        LOG_PROTOCOL(" Frame timeout detected (last recv: %ums ago)\n", timeout_ms);
    }
    return timeout;
}

// Extract complete frame from queue
static AppOldCommParseResult app_old_comm_extract_complete_frame(AppOldCommRingQueue *queue, uint8_t *frame_buf,
                                                                 uint16_t *frame_len)
{
    LOG_PROTOCOL(" Starting frame extraction (queue len: %d)\n", queue->len);

    // 1. Check frame timeout
    if (app_old_comm_is_frame_timeout(queue)) {
        app_old_comm_ring_queue_clear(queue);
        return APP_OLD_COMM_PARSE_ERR_TIMEOUT;
    }

    // 2. Find frame header (0xAA55)
    uint16_t header_pos = 0;
    bool header_found = false;
    uint16_t max_search = (queue->len >= 2) ? (queue->len - 1) : 0;

    while (header_pos < max_search) {
        uint8_t byte1 = app_old_comm_ring_queue_peek(queue, header_pos);
        uint8_t byte2 = app_old_comm_ring_queue_peek(queue, header_pos + 1);

        if ((byte1 << 8 | byte2) == APP_OLD_COMM_SOP_FCF) {
            header_found = true;
            break;
        }
        header_pos++;
    }

    if (!header_found) {
        LOG_PROTOCOL(" Frame header (0xAA55) not found\n");
        // Clear if exceeding half max frame length
        if (queue->len > APP_OLD_COMM_MAX_PACKET_LEN / 2) {
            app_old_comm_ring_queue_clear(queue);
        }
        return APP_OLD_COMM_PARSE_ERR_HEADER;
    }

    LOG_PROTOCOL(" Found frame header at position %d\n", header_pos);

    // 3. Clear invalid data before header
    if (header_pos > 0) {
        uint8_t dummy[APP_OLD_COMM_RX_QUEUE_SIZE];
        app_old_comm_ring_queue_pop(queue, dummy, header_pos);
        LOG_PROTOCOL(" Removed %d invalid bytes before header\n", header_pos);
    }

    // 4. Check minimum length for parsing
    if (queue->len < 6) {  // Header(2) + CT(1) + CID(1) + CAL(1) + crc = 6 bytes
        LOG_PROTOCOL(" Incomplete frame - need at least 5 bytes, have %d\n", queue->len);
        return APP_OLD_COMM_PARSE_ERR_INCOMPLETE;
    }

    // 5. Parse parameter length field (CAL)
    uint8_t cal = app_old_comm_ring_queue_peek(queue, 4);
    uint16_t total_frame_len = 2 + 1 + 1 + 1 + cal + 1;  // header+CT+CID+CAL+data+checksum

    LOG_PROTOCOL(" Parsed frame length: %d bytes (CAL: %d)\n", total_frame_len, cal);

    // Verify frame length
    if (total_frame_len < 6 || total_frame_len > APP_OLD_COMM_MAX_PACKET_LEN) {
        LOG_PROTOCOL(" Invalid frame length %d (min:6, max:%d)\n",
               total_frame_len, APP_OLD_COMM_MAX_PACKET_LEN);
        // Remove invalid header
        uint8_t dummy[2];
        app_old_comm_ring_queue_pop(queue, dummy, 2);
        return APP_OLD_COMM_PARSE_ERR_LENGTH;
    }

    // 6. Check if complete frame exists
    if (queue->len < total_frame_len) {
        LOG_PROTOCOL(" Incomplete frame - need %d bytes, have %d\n",
               total_frame_len, queue->len);
        return APP_OLD_COMM_PARSE_ERR_INCOMPLETE;
    }

    // 7. Extract complete frame data
    uint16_t read_len = app_old_comm_ring_queue_pop(queue, frame_buf, total_frame_len);
    if (read_len != total_frame_len) {
        LOG_PROTOCOL(" Failed to extract frame - read %d of %d bytes\n",
               read_len, total_frame_len);
        return APP_OLD_COMM_PARSE_ERR_LENGTH;
    }

    // 8. Verify checksum
    uint8_t recv_checksum = frame_buf[total_frame_len - 1];
    uint8_t calc_check = app_old_comm_calc_checksum(frame_buf, total_frame_len - 1);
    if (recv_checksum != calc_check) {
        LOG_PROTOCOL(" Checksum mismatch - received 0x%02X, calculated 0x%02X\n",
               recv_checksum, calc_check);
        return APP_OLD_COMM_PARSE_ERR_CHECKSUM;
    }

    *frame_len = total_frame_len;
    LOG_PROTOCOL(" Successfully extracted frame (len: %d)\n", total_frame_len);
    return APP_OLD_COMM_PARSE_SUCCESS;
}

// Process frame data in queue
static void app_old_comm_protocol_process_queue(void)
{
    LOG_PROTOCOL(" Starting queue processing (queue len: %d)\n", app_old_comm_rx_queue.len);

    // Try to extract max 3 frames
    for (uint8_t i = 0; i < 3; i++) {
        if (app_old_comm_rx_queue.len < 6) {
            LOG_PROTOCOL(" Queue has less than 6 bytes, stopping processing\n");
            break;
        }

        uint16_t frame_len = 0;
        AppOldCommParseResult result = app_old_comm_extract_complete_frame(
            &app_old_comm_rx_queue,
            app_old_comm_frame_buf,
            &frame_len
        );

        LOG_PROTOCOL(" Frame extraction result: %s\n", app_old_comm_get_parse_result_str(result));

        switch (result) {
            case APP_OLD_COMM_PARSE_SUCCESS:
                // Print frame content
                LOG_PROTOCOL(" Parsing frame (len: %d): ", frame_len);
                for (uint16_t j = 0; j < frame_len; j++) {
                    LOG_PROTOCOL("%02X ", app_old_comm_frame_buf[j]);
                }
                LOG_PROTOCOL("\n");

                // Parse and process frame
                app_old_comm_protocol_parse(app_old_comm_frame_buf, frame_len);
                break;

            case APP_OLD_COMM_PARSE_ERR_INCOMPLETE:
                // Wait for more data
                return;

            case APP_OLD_COMM_PARSE_ERR_TIMEOUT:
                // Timeout handled in extraction function
                return;

            default:
                // Continue to next frame
                continue;
        }
    }
}

// Protocol parsing core interface
static AppOldCommParseResult app_old_comm_protocol_parse(uint8_t *frame_buf, uint16_t frame_len)
{
    // 1. Verify frame header
    uint16_t header = (frame_buf[0] << 8) | frame_buf[1];
    if (header != APP_OLD_COMM_SOP_FCF) {
        LOG_PROTOCOL(" Invalid frame header 0x%04X (expected 0x%04X)\n",
               header, APP_OLD_COMM_SOP_FCF);
        return APP_OLD_COMM_PARSE_ERR_HEADER;
    }

    // 2. Parse core fields
    AppOldCommCmdType ct = (AppOldCommCmdType)frame_buf[2];
    AppOldCommCmdId cid = (AppOldCommCmdId)frame_buf[3];
    uint8_t cal = frame_buf[4];
    uint8_t *cab = frame_buf + 5;

#if BLE_PAYLOAD_CRYPTION
    // 对除了传输加密key和测试以外的所有指令数据进行加密
    AppOldCommDeviceInfo* comm_dev_info = app_old_comm_dev_info_get();
    if (comm_dev_info->encrypt_status == 1) {
        if ((cid != APP_OLD_COMM_CID_GET_ENCRYPT) && (cid != APP_OLD_COMM_CID_GET_DEV_INFO)) {
            app_old_protocol_data_cryption(comm_dev_info->encrypt_key, frame_buf+5, cal); //解密密ble数据
        }
    }
    LOG_HEX("app_recv_data cryption hex", frame_buf, frame_len);
#endif
    LOG_PROTOCOL(" Parsed frame - Type: %s (0x%02X), Command: %s (0x%02X), Data len: %d\n",
           app_old_comm_get_cmd_type_str(ct), ct,
           app_old_comm_get_cmd_id_str(cid), cid,
           cal);

    // 3. Dispatch processing
    switch (ct) {
        case APP_OLD_COMM_CT_REQ:
            app_old_comm_handle_request_cmd(cid, cal, cab);
            break;
        case APP_OLD_COMM_CT_ACK:
        case APP_OLD_COMM_CT_NACK:
        case APP_OLD_COMM_CT_NOTIFY:
        case APP_OLD_COMM_CT_DEVICE:
            app_old_comm_handle_response_cmd(ct, cid, cal, cab);
            break;
        default:
            LOG_PROTOCOL(" Unknown command type 0x%02X\n", ct);
            return APP_OLD_COMM_PARSE_ERR_CMD_UNKNOWN;
    }

    return APP_OLD_COMM_PARSE_SUCCESS;
}

// Initialize device information and communication module
static void app_old_comm_init(void)
{
    app_old_comm_ring_queue_init(&app_old_comm_rx_queue);
    app_old_comm_variable_init();
    app_old_protocol_ctx_init();
}

static uint16_t app_commication_ble_data_recv_cb(void *ctx, uint8_t *p_data, uint16_t ulLen)
{
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(p_data);
    if (!p_data || ulLen== 0) {
        LOGE("recv data is NULL or size is 0");
        return 0;
    }
    LOG_PROTOCOL("app_commication_ble_recv[%d]", ulLen);
    LOG_HEX("app_recv_data hex", p_data, ulLen);
    LOG_PROTOCOL("app_recv_data str %s", p_data);

    //选择协议解析器
    if (thiz->protocol_parse == NULL)
    {
        LOGE("protocol_parse is NULL");
        return 0;
    }
    thiz->protocol_parse(p_data, ulLen, PROTO_TYPE_APP_COMMINICATION);

    return 1;
}

static void app_commication_send_data_via_ble(uint8_t *data, uint16_t data_len)
{
    const uint16_t MAX_SEND_LEN = 128;

    /* 参数验证 */
    if (!data || data_len == 0) {
        LOG_PROTOCOL("invalid data or length");
        return;
    }

    if (!thiz) {
        LOG_PROTOCOL("protocol manager context is NULL");
        return;
    }

    /* 长度限制 */
    if (data_len > MAX_SEND_LEN) {
        LOG_PROTOCOL("data_len(%d) > MAX_SEND_LEN(%d), truncate", data_len, MAX_SEND_LEN);
        data_len = MAX_SEND_LEN;
    }

    /* 通过 BLE 发送 */
    if (thiz->priv.ble_ctx && thiz->priv.ble_ctx->ble_is_connected &&
        thiz->priv.ble_ctx->ble_is_connected()) {
        if (thiz->priv.ble_ctx->dispatch_ble_event_wrapper) {
            int ret = thiz->priv.ble_ctx->dispatch_ble_event_wrapper(
                thiz->priv.ble_ctx, BLE_MSG_MASTER_SEND_DATA, data, data_len);
            if (ret < 0) {
                LOG_PROTOCOL("send via BLE failed, ret=%d", ret);
            } else {
                LOG_PROTOCOL("send data via BLE[%d]", data_len);
            }
        } else {
            LOG_PROTOCOL("ble dispatch wrapper missing");
        }
    }

}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

// Get command ID string for logging
const char* app_old_comm_get_cmd_id_str(AppOldCommCmdId cid)
{
    switch (cid) {
        case APP_OLD_COMM_CID_HEARTBEAT: return "HEARTBEAT";
        case APP_OLD_COMM_CID_GET_DEV_INFO: return "GET_DEV_INFO";
        case APP_OLD_COMM_CID_GET_SN: return "GET_SN";
        case APP_OLD_COMM_CID_SET_RTC: return "SET_RTC";
        case APP_OLD_COMM_CID_SYS_CTRL: return "SYS_CTRL";
        case APP_OLD_COMM_CID_GET_ENCRYPT: return "GET_ENCRYPT";
        case APP_OLD_COMM_CID_GET_STATUS: return "GET_STATUS";
        case APP_OLD_COMM_CID_CTRL_DEV: return "CTRL_DEV";
        case APP_OLD_COMM_CID_SET_BIND: return "SET_BIND";
        case APP_OLD_COMM_CID_SET_TRAVEL_LOCK: return "SET_TRAVEL_LOCK";
        case APP_OLD_COMM_CID_NOTIFY_STATUS: return "NOTIFY_STATUS";
        case APP_OLD_COMM_CID_NOTIFY_BIND: return "NOTIFY_BIND";
        case APP_OLD_COMM_CID_NOTIFY_DISCON: return "NOTIFY_DISCON";
        default: return "UNKNOWN";
    }
}

// Get command type string for logging
const char* app_old_comm_get_cmd_type_str(AppOldCommCmdType ct)
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

// Checksum calculation (sum bytes and invert)
uint8_t app_old_comm_calc_checksum(uint8_t *data, uint16_t len)
{
    uint32_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += data[i];
    }
    uint8_t checksum = ~((uint8_t)sum);
    LOG_PROTOCOL(" Calculated checksum: 0x%02X (for %d bytes)\n", checksum, len);
    return checksum;
}

void set_app_commication_ble_register(protocol_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);

    /* 注册 BLE 回调（存在时） */
    if (ctx->priv.ble_ctx && ctx->priv.ble_ctx->set_master_ble_register_cb) {
        ctx->priv.ble_ctx->set_master_ble_register_cb(app_commication_ble_data_recv_cb);
        LOG_PROTOCOL("register app_commication ble cb");
    } else {
        LOG_PROTOCOL("ble ctx or register cb not available");
    }

}

void set_app_commication_ble_unregister(protocol_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);

    if (ctx->priv.ble_ctx && ctx->priv.ble_ctx->set_master_ble_register_cb) {
        ctx->priv.ble_ctx->set_master_ble_register_cb(NULL); /// 清除工厂模式蓝牙数据接收回调函数.
        LOG_PROTOCOL("unregister app_commication ble cb");
    } else {
        LOG_PROTOCOL("ble ctx or unregister cb not available");
    }
 
}

void app_old_comm_cmd_send(AppOldCommCmdType cmdtype, AppOldCommCmdId cid, uint8_t data_len, uint8_t *data)
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
#if BLE_PAYLOAD_CRYPTION
        AppOldCommDeviceInfo *app_old_comm_dev_info = app_old_comm_dev_info_get();
        // 对除了传输加密key和测试以外的所有指令数据进行加密
        if (app_old_comm_dev_info->encrypt_status == 1) {
            if ((cid != APP_OLD_COMM_CID_GET_ENCRYPT) && (cid != APP_OLD_COMM_CID_GET_DEV_INFO)) {
                app_old_protocol_data_cryption(app_old_comm_dev_info->encrypt_key, &send_buf[len], data_len); // 加密ble数据
            }
        }
#endif
        len += data_len;
    }
    send_buf[len++] = app_old_comm_calc_checksum(send_buf, len);
#if BLE_PAYLOAD_CRYPTION
    LOG_HEX("app_send_data hex", send_buf, len);
#endif
    LOG_PROTOCOL(" Sending cmdtype %s for %s (len: %d)\n", app_old_comm_get_cmd_type_str(cmdtype), app_old_comm_get_cmd_id_str(cid), len);
    app_commication_send_data_via_ble(send_buf, len);
}


uint8_t app_commication_protocol_parse_route(uint8_t *data, uint16_t len)
{
    // 这里添加APP协议的解析逻辑
    LOG_PROTOCOL("app_commication protocol parse data len: %d", len);

    if (len == 0 || len > APP_OLD_COMM_MAX_PACKET_LEN) {
        LOG_PROTOCOL(" Invalid data length %d (max: %d)\n", len, APP_OLD_COMM_MAX_PACKET_LEN);
        return 0;
    }
    // Add received data to queue
    app_old_comm_ring_queue_push(&app_old_comm_rx_queue, data, len);
    app_old_comm_protocol_process_queue();
    return 1;
}

int on_app_commication_protocol_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    protocol_manager_context_t *ctx = context;

    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            {

            }
            break;
            case ID_BROADCAST_BLE_CONNECT_STATE_CHANGE:
            {
                #if BLE_PAYLOAD_CRYPTION
                AppOldCommDeviceInfo *comm_dev_info = app_old_comm_dev_info_get();
                comm_dev_info->encrypt_status = 0; //断开或连接后重置加密状态
                if (arg1) {
                    do {
                        comm_dev_info->encrypt_key = (uint8_t)rand();
                    } while ((comm_dev_info->encrypt_key == 0) || (comm_dev_info->encrypt_key == 0xFF)); // 不能为0或者0xFF
                } else {
                    ;
                }
                LOG_PROTOCOL("comm_dev_info->encrypt_key %d", comm_dev_info->encrypt_key);
                #endif
            }
            break;
        case ID_BROADCAST_SYS_POWER_STATE_CHANGE:
            {

            }
            break;
        case ID_BROADCAST_SYS_BOOT_COMPLETED:
            {

            }
            break;
        default:
            break;
    }
}

/// 初始化工厂协议模块
void on_perpare_app_commication_protocol_init(protocol_manager_context_t *ctx)
{
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    ctx->priv.ble_ctx = ble_ctx;
    LOG_PROTOCOL("ble ctx: %p", ctx->priv.ble_ctx);
    thiz = ctx;
}

/// 初始化工厂协议模块
void on_post_app_commication_protocol_init(protocol_manager_context_t *ctx)
{
    set_app_commication_ble_register(ctx);
    app_old_comm_init();
    thiz = ctx;
}
