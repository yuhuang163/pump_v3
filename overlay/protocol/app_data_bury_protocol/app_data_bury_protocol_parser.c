/*
 * Momcozy 数据埋点协议 v1.0.x
 * 解析入口：链路层 0x5A 帧封装 + CRC16-CCITT-FALSE + ACK
 *
 * 接入方式：
 *  1) BLE 收到 DEVICE_BLE_DATA_BURY_MSG_CHANNEL 上的字节片段
 *  2) 调用 app_data_bury_protocol_assemble_packet() 进行组帧
 *  3) 帧完整后回调 app_data_bury_protocol_frame_deserialize() 或业务回调
 */

#include "root_common_api.h"
#include "ble_manager_api.h"

#include "app_data_bury_protocol_parser.h"

#include <string.h>

#define RX_BUF_SIZE 512

static uint8_t s_rx_buf[RX_BUF_SIZE];
static uint16_t s_rx_len;

static ble_manager_context_t *s_ble_ctx;

static uint16_t app_data_bury_protocol_frame_pack(uint8_t *out, uint16_t out_cap, uint8_t ctrl,
                                                    uint8_t fsn_val, const uint8_t *payload,
                                                    uint16_t payload_len)
{
    if (!out || out_cap < APP_DATA_BURY_PROTOCOL_HEADER_MIN + APP_DATA_BURY_PROTOCOL_FOOTER_SIZE) {
        return 0;
    }

    uint16_t pos = 0;
    out[pos++] = APP_DATA_BURY_PROTOCOL_SOF;

    uint8_t has_fsn = ((ctrl & APP_DATA_BURY_PROTOCOL_CTRL_FSN_MASK) != APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE);
    uint16_t len_field = (uint16_t)(1u + (has_fsn ? 1u : 0u) + payload_len); /* Control + [FSN] + Payload */

    out[pos++] = (uint8_t)(len_field >> 8);
    out[pos++] = (uint8_t)(len_field & 0xFF);
    out[pos++] = ctrl;

    if (has_fsn) {
        out[pos++] = fsn_val;
    }

    if (payload && payload_len) {
        if ((uint32_t)pos + payload_len + APP_DATA_BURY_PROTOCOL_FOOTER_SIZE > out_cap) {
            return 0;
        }
        memcpy(&out[pos], payload, payload_len);
        pos = (uint16_t)(pos + payload_len);
    }

    /* CRC: 从 Length 字段开始到 payload 末尾（不含 SOF 不含 CRC 自身） */
    uint16_t crc = CRC16(&out[1], (uint16_t)(pos - 1));
    out[pos++] = (uint8_t)(crc >> 8);
    out[pos++] = (uint8_t)(crc & 0xFF);
    return pos;
}

static int app_data_bury_protocol_send_raw_frame(const uint8_t *frame, uint16_t frame_len)
{
    if (!s_ble_ctx || !frame || frame_len == 0) {
        return -1;
    }
    if (!s_ble_ctx->ble_is_connected()) {
        return -2;
    }
    return s_ble_ctx->dispatch_ble_event_wrapper(s_ble_ctx, BLE_MSG_DATA_BURY_SEND_DATA,
                                                  (uint8_t *)frame, frame_len);
}

void app_data_bury_protocol_init(void)
{
    s_rx_len = 0;
    s_ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
}

int app_data_bury_protocol_frame_deserialize(const uint8_t *buffer, uint16_t buffer_size,
                                               app_data_bury_protocol_frame_node_t *frame)
{
    if (!buffer || !frame) {
        return -1;
    }
    if (buffer_size < APP_DATA_BURY_PROTOCOL_HEADER_MIN + APP_DATA_BURY_PROTOCOL_FOOTER_SIZE) {
        return -2;
    }

    frame->sof = buffer[0];
    if (frame->sof != APP_DATA_BURY_PROTOCOL_SOF) {
        return -3;
    }

    uint16_t len_field = (uint16_t)buffer[1] << 8 | buffer[2];
    frame->ctrl = buffer[3];
    frame->fsn_present = ((frame->ctrl & APP_DATA_BURY_PROTOCOL_CTRL_FSN_MASK) != APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE);
    frame->fsn = frame->fsn_present ? buffer[4] : 0;

    uint16_t total = (uint16_t)(APP_DATA_BURY_PROTOCOL_HEADER_MIN + (len_field - 1u));
    /* 与 assemble_packet 中保持一致：total = 5 + len_field */
    total = (uint16_t)(5u + len_field);

    if (buffer_size < total) {
        return -4;
    }

    uint16_t crc_recv = (uint16_t)buffer[total - 2] << 8 | buffer[total - 1];
    uint16_t crc_calc = CRC16(&buffer[1], (uint16_t)(2u + len_field));
    frame->crc_ok = (crc_calc == crc_recv);

    uint16_t pay_off = (uint16_t)(4u + (frame->fsn_present ? 1u : 0u));
    uint16_t pay_len = (uint16_t)(len_field - 1u - (frame->fsn_present ? 1u : 0u));

    if (pay_len < 2u) {
        return -5;
    }
    ///埋点只有单服务ID 0x07 不对数据做服务拆包
    frame->svc_id = buffer[pay_off];
    frame->cmd_id = buffer[pay_off + 1];
    frame->payload = &buffer[pay_off + 2];
    frame->payload_len = (uint16_t)(pay_len - 2u);
    frame->ack_requested = (frame->ctrl & APP_DATA_BURY_PROTOCOL_CTRL_ACK) ? 1u : 0u;

    return 0;
}

int app_data_bury_protocol_assemble_packet(const uint8_t *data, uint16_t len,
                                            app_data_bury_protocol_frame_callback_t callback)
{
    if (!data || len == 0) {
        return -1;
    }

    for (uint16_t i = 0; i < len; i++) {
        /* 同步到 SOF */
        if (s_rx_len == 0 && data[i] != APP_DATA_BURY_PROTOCOL_SOF) {
            continue;
        }

        if (s_rx_len >= RX_BUF_SIZE) {
            s_rx_len = 0;
            continue;
        }

        s_rx_buf[s_rx_len++] = data[i];

        /* 至少需要读取 Header 中的 SOF/Length/Control */
        if (s_rx_len < APP_DATA_BURY_PROTOCOL_HEADER_MIN) {
            continue;
        }

        uint16_t len_field = (uint16_t)s_rx_buf[1] << 8 | s_rx_buf[2];
        uint8_t ctrl = s_rx_buf[3];
        uint8_t has_fsn = ((ctrl & APP_DATA_BURY_PROTOCOL_CTRL_FSN_MASK) != APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE);

        /* 整帧长度：5 + len_field */
        uint16_t total = (uint16_t)(5u + len_field);
        if (total > RX_BUF_SIZE) {
            s_rx_len = 0;
            continue;
        }

        if (s_rx_len < total) {
            continue;
        }

        /* 帧完整 */
        uint16_t crc_recv = (uint16_t)s_rx_buf[total - 2] << 8 | s_rx_buf[total - 1];
        uint16_t crc_calc = CRC16(&s_rx_buf[1], (uint16_t)(2u + len_field));
        uint8_t crc_ok = (crc_calc == crc_recv);

        uint16_t pay_off = (uint16_t)(4u + (has_fsn ? 1u : 0u));
        uint16_t pay_len = (uint16_t)(len_field - 1u - (has_fsn ? 1u : 0u));

        uint8_t svc = 0;
        uint8_t cmd = 0;
        if (pay_len >= 2u) {
            svc = s_rx_buf[pay_off];
            cmd = s_rx_buf[pay_off + 1];
        }

        /* ACK 回复：只要控制位请求 ACK，就返回成功/失败标识 */
        if (ctrl & APP_DATA_BURY_PROTOCOL_CTRL_ACK) {
            uint8_t tlv_rsp[6];
            tlv_rsp[0] = APP_DATA_BURY_PROTOCOL_TLV_TYPE_ERROR; /* type 0x7F */
            tlv_rsp[1] = 0x04;                                  /* length=4 */
            uint32_t val = crc_ok ? 0x00100000u : 0x00100001u;
            tlv_rsp[2] = (uint8_t)(val >> 24);
            tlv_rsp[3] = (uint8_t)(val >> 16);
            tlv_rsp[4] = (uint8_t)(val >> 8);
            tlv_rsp[5] = (uint8_t)(val & 0xFF);

            uint8_t app_rsp[2 + 6];
            app_rsp[0] = svc;
            app_rsp[1] = cmd;
            memcpy(&app_rsp[2], tlv_rsp, sizeof(tlv_rsp));

            uint8_t frame_rsp[APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX + 16];
            uint16_t flen = app_data_bury_protocol_frame_pack(frame_rsp, (uint16_t)sizeof(frame_rsp),
                                                               APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE, 0,
                                                               app_rsp, (uint16_t)sizeof(app_rsp));
            if (flen > 0) {
                (void)app_data_bury_protocol_send_raw_frame(frame_rsp, flen);
            }
        }

        /* 回调：仅当 CRC 正确且 payload 至少包含 svc/cmd 两字节 */
        if (crc_ok && pay_len >= 2u && callback) {
            callback(s_rx_buf, total);
        }

        /* 清空接收缓冲，准备下一帧 */
        s_rx_len = 0;
    }

    return 0;
}

