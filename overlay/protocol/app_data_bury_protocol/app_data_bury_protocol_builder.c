/*
 * Momcozy 数据埋点协议 v1.0.x
 * 发送入口：单帧（默认）/ MFS 分帧 两套接口；MTU 切片由 ble_service 层完成
 */

#include "root_common_api.h"
#include "resource_manager_api.h"
#include "ble_manager_api.h"

#include "app_data_bury_protocol_builder.h"

#include <string.h>
#define DEBUG_DATA_BURY_ENABLE 0

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
#if DEBUG_DATA_BURY_ENABLE
    LOG_HEX("bury_send_data hex", frame, frame_len);
#endif
    return s_ble_ctx->dispatch_ble_event_wrapper(s_ble_ctx, BLE_MSG_DATA_BURY_SEND_DATA,
                                                  (uint8_t *)frame, frame_len);
}

static int app_data_bury_protocol_app_buf_alloc(uint8_t **out_buf, uint8_t *out_in_psram)
{
    uint8_t *buf = NULL;
    uint8_t in_psram = 0;

#if PSRAM_ENABLE
    buf = (uint8_t *)u_malloz_psram(APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX, RES_PSRAM_TYPE);
    if (buf) {
        in_psram = 1;
    }
#endif
    if (!buf) {
        buf = (uint8_t *)u_malloz(APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX);
    }
    if (!buf) {
        return -4;
    }
    *out_buf = buf;
    *out_in_psram = in_psram;
    return 0;
}

static void app_data_bury_protocol_app_buf_free(uint8_t *buf, uint8_t in_psram)
{
    if (!buf) {
        return;
    }
    if (in_psram) {
        u_free_psram(buf, RES_PSRAM_TYPE);
    } else {
        u_free(buf);
    }
}

static int app_data_bury_protocol_build_app_buf(uint8_t *app_buf, uint16_t app_buf_cap,
                                                uint8_t svc_id, uint8_t cmd_id,
                                                const uint8_t *payload, uint16_t payload_len,
                                                uint16_t *app_len_out)
{
    uint16_t app_len;

    if (!app_buf || !app_len_out) {
        return -1;
    }
    if (payload_len > (app_buf_cap - 2u)) {
        return -1;
    }

    app_len = (uint16_t)(2u + payload_len);
    app_buf[0] = svc_id;
    app_buf[1] = cmd_id;
    if (payload_len > 0u && payload) {
        memcpy(&app_buf[2], payload, payload_len);
    }
    *app_len_out = app_len;
    return 0;
}

static int app_data_bury_protocol_send_single_frame(const uint8_t *app_packet, uint16_t app_len)
{
    uint8_t frame_buf[APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX + 16];
    uint16_t frame_len;

    if (!app_packet || app_len == 0u) {
        return -1;
    }
    if (app_len > APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX) {
        return -1;
    }

    frame_len = app_data_bury_protocol_frame_pack(frame_buf, (uint16_t)sizeof(frame_buf),
                                                  (uint8_t)(APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE |
                                                            APP_DATA_BURY_PROTOCOL_CTRL_ACK),
                                                  0, app_packet, app_len);
    if (frame_len == 0u) {
        return -4;
    }
    return app_data_bury_protocol_send_raw_frame(frame_buf, frame_len);
}

static int app_data_bury_protocol_send_by_mfs(const uint8_t *app_packet, uint16_t app_len, uint16_t mfs)
{
    uint8_t frame_buf[APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX + 16];
    uint16_t pos;
    uint8_t fsn;
    uint8_t ctrl;

    if (!app_packet || app_len == 0u) {
        return -1;
    }
    if (app_len > APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX) {
        return -1;
    }
    if (mfs == 0u) {
        mfs = APP_DATA_BURY_PROTOCOL_MFS_DEFAULT;
    }

    if (app_len <= mfs) {
        uint16_t frame_len = app_data_bury_protocol_frame_pack(frame_buf, (uint16_t)sizeof(frame_buf),
                                                               (uint8_t)(APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE |
                                                                         APP_DATA_BURY_PROTOCOL_CTRL_ACK),
                                                               0, app_packet, app_len);
        if (frame_len == 0u) {
            return -4;
        }
        return app_data_bury_protocol_send_raw_frame(frame_buf, frame_len);
    }

    pos = 0;
    fsn = 0;
    while (pos < app_len) {
        uint16_t chunk = (uint16_t)(app_len - pos);
        uint16_t frame_len;
        int ret;

        if (chunk > mfs) {
            chunk = mfs;
        }

        if (pos == 0) {
            ctrl = APP_DATA_BURY_PROTOCOL_CTRL_FSN_START;
        } else if (pos + chunk >= app_len) {
            ctrl = APP_DATA_BURY_PROTOCOL_CTRL_FSN_END;
        } else {
            ctrl = APP_DATA_BURY_PROTOCOL_CTRL_FSN_MID;
        }
        ctrl |= APP_DATA_BURY_PROTOCOL_CTRL_ACK;

        frame_len = app_data_bury_protocol_frame_pack(frame_buf, (uint16_t)sizeof(frame_buf),
                                                      ctrl,
                                                      (ctrl & APP_DATA_BURY_PROTOCOL_CTRL_FSN_MASK) !=
                                                          APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE ? fsn : 0,
                                                      &app_packet[pos], chunk);
        if (frame_len == 0u) {
            return -4;
        }
        ret = app_data_bury_protocol_send_raw_frame(frame_buf, frame_len);
        if (ret != 0) {
            return ret;
        }
        pos = (uint16_t)(pos + chunk);
        if ((ctrl & APP_DATA_BURY_PROTOCOL_CTRL_FSN_MASK) != APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE) {
            fsn++;
        }
    }
    return 0;
}

int app_data_bury_protocol_frame_builder_send(uint8_t svc_id, uint8_t cmd_id,
                                              const uint8_t *payload, uint16_t payload_len)
{
    uint8_t *app_buf = NULL;
    uint8_t app_buf_in_psram = 0;
    uint16_t app_len;
    int ret;

    ret = app_data_bury_protocol_app_buf_alloc(&app_buf, &app_buf_in_psram);
    if (ret != 0) {
        return ret;
    }

    ret = app_data_bury_protocol_build_app_buf(app_buf, APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX,
                                               svc_id, cmd_id, payload, payload_len, &app_len);
    if (ret != 0) {
        app_data_bury_protocol_app_buf_free(app_buf, app_buf_in_psram);
        return ret;
    }

    ret = app_data_bury_protocol_send_single_frame(app_buf, app_len);
    app_data_bury_protocol_app_buf_free(app_buf, app_buf_in_psram);
    return ret;
}

int app_data_bury_protocol_frame_builder_send_split(uint8_t svc_id, uint8_t cmd_id,
                                                    const uint8_t *payload, uint16_t payload_len,
                                                    uint16_t mfs)
{
    uint8_t *app_buf = NULL;
    uint8_t app_buf_in_psram = 0;
    uint16_t app_len;
    int ret;

    ret = app_data_bury_protocol_app_buf_alloc(&app_buf, &app_buf_in_psram);
    if (ret != 0) {
        return ret;
    }

    ret = app_data_bury_protocol_build_app_buf(app_buf, APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX,
                                               svc_id, cmd_id, payload, payload_len, &app_len);
    if (ret != 0) {
        app_data_bury_protocol_app_buf_free(app_buf, app_buf_in_psram);
        return ret;
    }

    ret = app_data_bury_protocol_send_by_mfs(app_buf, app_len, mfs);
    app_data_bury_protocol_app_buf_free(app_buf, app_buf_in_psram);
    return ret;
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

