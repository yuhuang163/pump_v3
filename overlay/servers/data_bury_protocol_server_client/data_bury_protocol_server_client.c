/*
 * 数据埋点协议 server client
 * 订阅 DEVICE_BLE_DATA_BURY_MSG_CHANNEL，触发埋点链路层帧解析（0x5A + CRC + ACK）
 */

#include "root_common_api.h" /* IWYU pragma: keep */
#include "ble_manager_api.h" /* IWYU pragma: keep */

#include "app_data_bury_protocol/app_data_bury_protocol.h" /* IWYU pragma: keep */
#include "data_bury_protocol_server_client.h" /* IWYU pragma: keep */

/*
 * 完整帧回调：解析出 svc/cmd/payload 后交给后续“协议解析管理模块”
 */
static void on_data_bury_protocol_frame_rx(const uint8_t *frame_data, uint16_t frame_len)
{
    app_data_bury_protocol_frame_node_t frame = {0};
    int ret = app_data_bury_protocol_frame_deserialize(frame_data, frame_len, &frame);
    if (ret != 0) {
        LOGE("data bury frame_deserialize failed: ret=%d", ret);
        return;
    }
    if (!frame.crc_ok) {
        LOGW("data bury frame crc not ok (svc=0x%02x cmd=0x%02x)", frame.svc_id, frame.cmd_id);
        return;
    }

    LOG_BLE("data bury rx frame svc=0x%02x cmd=0x%02x payload_len=%u",
            frame.svc_id, frame.cmd_id, (unsigned)frame.payload_len);
    LOG_HEX("data bury payload", frame.payload, frame.payload_len);

    /* TODO: 协议解析管理模块对 payload TLV 做业务分发 */
}

static void on_ble_data_bury_server_event_cb(void *user_data, uint8_t *data, uint16_t data_len)
{
    (void)user_data;
    app_data_bury_protocol_assemble_packet(data, data_len, on_data_bury_protocol_frame_rx);
}

void data_bury_protocol_server_client_init(void)
{
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_VOID(ble_ctx);

    app_data_bury_protocol_init();
    ble_ctx->add_ble_evt_lisenser(ble_ctx, DEVICE_BLE_DATA_BURY_MSG_CHANNEL,
                                   on_ble_data_bury_server_event_cb);
}

