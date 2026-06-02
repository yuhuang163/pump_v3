/*
 * Momcozy 数据埋点协议 v1.0.x
 * 发送入口：组装链路层帧（单帧为主，ACK 置位）
 */

#include "root_common_api.h"
#include "ble_manager_api.h"

#include "app_data_bury_protocol_builder.h"

#include <string.h>

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
    uint16_t len_field = (uint16_t)(1u + (has_fsn ? 1u : 0u) + payload_len);

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

    uint16_t crc = CRC16(&out[1], (uint16_t)(pos - 1));
    out[pos++] = (uint8_t)(crc >> 8);
    out[pos++] = (uint8_t)(crc & 0xFF);
    return pos;
}

static int app_data_bury_protocol_send_raw_frame(const uint8_t *frame, uint16_t frame_len)
{
    if (!frame || frame_len == 0) {
        return -1;
    }
    if (!s_ble_ctx) {
        s_ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    }
    if (!s_ble_ctx) {
        return -2;
    }
    if (!s_ble_ctx->ble_is_connected()) {
        return -3;
    }
    return s_ble_ctx->dispatch_ble_event_wrapper(s_ble_ctx, BLE_MSG_DATA_BURY_SEND_DATA,
                                                  (uint8_t *)frame, frame_len);
}

int app_data_bury_protocol_frame_builder_send(uint8_t svc_id, uint8_t cmd_id,
                                              const uint8_t *payload, uint16_t payload_len)
{
    if (payload_len > APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX) {
        return -1;
    }

    /* 单帧策略：避免链路层多帧分片与上层 svc/cmd 假设不一致的问题 */
    uint16_t app_len = (uint16_t)(2u + payload_len);
    if (app_len > APP_DATA_BURY_PROTOCOL_MFS_DEFAULT) {
        return -2;
    }

    uint8_t app_buf[APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX];
    app_buf[0] = svc_id;
    app_buf[1] = cmd_id;
    if (payload_len > 0 && payload) {
        memcpy(&app_buf[2], payload, payload_len);
    }

    /* 单帧：Control = FSN_COMPLETE | ACK */
    uint8_t frame_buf[APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX + 16];
    uint16_t frame_len = app_data_bury_protocol_frame_pack(frame_buf, (uint16_t)sizeof(frame_buf),
                                                           (uint8_t)(APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE |
                                                                     APP_DATA_BURY_PROTOCOL_CTRL_ACK),
                                                           0, app_buf, app_len);
    if (frame_len == 0) {
        return -3;
    }

    return app_data_bury_protocol_send_raw_frame(frame_buf, frame_len);
}

uint16_t app_data_bury_protocol_tlv_put_u8(uint8_t *buf, uint16_t cap, uint8_t type, uint8_t val)
{
    if (cap < (APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + 1u)) {
        return 0;
    }
    buf[0] = type;
    buf[1] = 1u;
    buf[2] = val;
    return 3u;
}

uint16_t app_data_bury_protocol_tlv_put_u16(uint8_t *buf, uint16_t cap, uint8_t type, uint16_t val)
{
    if (cap < (APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + 2u)) {
        return 0;
    }
    buf[0] = type;
    buf[1] = 2u;
    buf[2] = (uint8_t)(val >> 8);
    buf[3] = (uint8_t)(val & 0xFFu);
    return 4u;
}

uint16_t app_data_bury_protocol_tlv_put_u32(uint8_t *buf, uint16_t cap, uint8_t type, uint32_t val)
{
    if (cap < (APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + 4u)) {
        return 0;
    }
    buf[0] = type;
    buf[1] = 4u;
    buf[2] = (uint8_t)(val >> 24);
    buf[3] = (uint8_t)((val >> 16) & 0xFFu);
    buf[4] = (uint8_t)((val >> 8) & 0xFFu);
    buf[5] = (uint8_t)(val & 0xFFu);
    return 6u;
}

uint16_t app_data_bury_protocol_tlv_put_raw(uint8_t *buf, uint16_t cap, uint8_t type, const uint8_t *val, uint8_t vlen)
{
    if (!buf || cap < (uint16_t)(APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + vlen) || (!val && vlen > 0)) {
        return 0;
    }
    buf[0] = type;
    buf[1] = vlen;
    if (vlen > 0) {
        memcpy(&buf[2], val, vlen);
    }
    return (uint16_t)(APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + vlen);
}

