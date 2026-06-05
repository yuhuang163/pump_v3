/*
 * 数据埋点 Data Service 业务处理：TLV 组包推送 / RAM 环 flush
 */

#include "root_common_api.h"
#include "resource_manager_api.h"
#include "data_bury_server_protocol.h"
#include "data_bury_tlvs_protocol.h"
#include "app_data_bury_protocol_builder.h"
#include "data_bury_persist_data.h"

static int data_bury_server_send_tlv(uint8_t svc_id, uint8_t cmd_id,
                                       const uint8_t *tlv, uint16_t tlv_len)
{
    uint16_t app_len = (uint16_t)(2u + tlv_len);

    if (app_len > APP_DATA_BURY_PROTOCOL_MFS_DEFAULT) {
        //return app_data_bury_protocol_frame_builder_send_split(svc_id, cmd_id, tlv, tlv_len, 0);
    }
    return app_data_bury_protocol_frame_builder_send(svc_id, cmd_id, tlv, tlv_len);
}

static int data_bury_server_try_send_packed(uint8_t svc_id, uint8_t cmd_id,
                                            const uint8_t *packed, uint16_t packed_len)
{
    uint8_t *tlv_buf = NULL;
    uint8_t tlv_buf_in_psram = 0;
    uint16_t tlv_len;
    int ret;

#if PSRAM_ENABLE
    tlv_buf = (uint8_t *)u_malloz_psram(APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX, RES_PSRAM_TYPE);
    if (tlv_buf) {
        tlv_buf_in_psram = 1;
    }
#endif
    if (!tlv_buf) {
        tlv_buf = (uint8_t *)u_malloz(APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX);
    }
    if (!tlv_buf) {
        LOGE("data bury tlv buf alloc failed");
        return -4;
    }

    tlv_len = data_bury_tlv_packed_to_wire(cmd_id, packed, packed_len, tlv_buf,
                                           APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX);
    if (tlv_len == 0u) {
        LOGE("data bury tlv pack failed cmd=0x%02x plen=%u", cmd_id, (unsigned)packed_len);
        if (tlv_buf_in_psram) {
            u_free_psram(tlv_buf, RES_PSRAM_TYPE);
        } else {
            u_free(tlv_buf);
        }
        return -1;
    }

    ret = data_bury_server_send_tlv(svc_id, cmd_id, tlv_buf, tlv_len);
    if (tlv_buf_in_psram) {
        u_free_psram(tlv_buf, RES_PSRAM_TYPE);
    } else {
        u_free(tlv_buf);
    }
    return ret;
}

void data_bury_server_protocol_init(void)
{
    LOG_SRV("data bury server protocol init");
}

void data_bury_server_protocol_on_frame(const app_data_bury_protocol_frame_node_t *frame)
{
    if (!frame) {
        return;
    }
    if (frame->svc_id != APP_DATA_BURY_PROTOCOL_SVC_DATA) {
        LOG_BLE("data bury skip non-data svc=0x%02x cmd=0x%02x", frame->svc_id, frame->cmd_id);
        return;
    }
    LOG_SRV("data bury rx svc=0x%02x cmd=0x%02x payload_len=%u",
            frame->svc_id, frame->cmd_id, (unsigned)frame->payload_len);
    if (frame->payload && frame->payload_len > 0u) {
        LOG_HEX("data bury rx tlv", frame->payload, frame->payload_len);
    }
}

int data_bury_server_push_event(uint8_t cmd_id, const void *packed, uint16_t packed_len)
{
    storage_manager_context_t *st;
    int ret;

    if (!packed || packed_len == 0u) {
        return -1;
    }
    if (data_bury_event_payload_max_len(cmd_id) == 0u || packed_len > data_bury_event_payload_max_len(cmd_id)) {
       LOGE("data bury event payload max len error cmd=0x%02x packed_len=%u", cmd_id, (unsigned)packed_len);
        return -2;
    }

    return data_bury_server_try_send_packed(APP_DATA_BURY_PROTOCOL_SVC_DATA, cmd_id, (const uint8_t *)packed, packed_len);
}

/**
int data_bury_server_flush_pending(void)
{
    storage_manager_context_t *st;
    ble_manager_context_t *ble;
    data_bury_rec_hdr_t hdr;
    uint8_t packed[DATA_BURY_EVENT_PAYLOAD_MAX];
    data_bury_ring_peek_args_t peek;
    int sent = 0;

    st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    if (!st || !ble || !ble->ble_is_connected()) {
        return -1;
    }

    peek.hdr = &hdr;
    peek.payload = packed;
    peek.payload_buf_len = sizeof(packed);

    for (;;) {
        uint16_t pending = 0;

        if (READ_DATA_BURY_RING_PENDING_COUNT(st, &pending) != 0 || pending == 0u) {
            break;
        }
        if (READ_DATA_BURY_RING_PEEK_OLDEST(st, &peek) != 0) {
            break;
        }
        if (data_bury_server_try_send_packed(hdr.svc_id, hdr.cmd_id, packed, hdr.payload_len) != 0) {
            break;
        }
        if (WRITE_DATA_BURY_RING_POP_OLDEST(st) != 0) {
            break;
        }
        sent++;
    }

    if (sent > 0) {
        LOG_SRV("data bury flush sent %d events", sent);
    }
    return sent;
}/
*/