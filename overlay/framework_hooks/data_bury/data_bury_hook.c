/*
 * 数据埋点 Manager overlay hook：线程事件路由、离线环 flush
 */

#include "root_common_api.h"
#include "app_data_bury_protocol_defs.h"
#include "data_bury_event_handler.h"

#define DATA_BURY_LOG_DEBUG 1

const static data_bury_event_process_handle_t data_bury_event_process_list[] = {
    { DATA_BURY_TRIGGER_EVT_BLE_CONNECTED, data_bury_event_process_ble_connected },
    { DATA_BURY_TRIGGER_EVT_BLE_DISCONNECT, data_bury_event_process_ble_disconnect },
    { DATA_BURY_TRIGGER_EVT_PUMP_SESSION_START, data_bury_event_process_pump_session_start },
    { DATA_BURY_TRIGGER_EVT_PUMP_MODE_SWITCH, data_bury_event_process_pump_mode_switch },
    { DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END, data_bury_event_process_pump_session_end },
    { DATA_BURY_TRIGGER_EVT_POWER_ON, data_bury_event_process_power_on },
    { DATA_BURY_TRIGGER_EVT_POWER_OFF, data_bury_event_process_power_off },
    { DATA_BURY_TRIGGER_EVT_OTA_START, data_bury_event_process_ota_start },
    { DATA_BURY_TRIGGER_EVT_BAT_CHARGE_START, data_bury_event_process_bat_charge_start },
    { DATA_BURY_TRIGGER_EVT_BAT_CHARGE_END, data_bury_event_process_bat_charge_end },
};

static servers_manager_context_t *data_bury_hook_get_servers_ctx(data_bury_manager_context_t *ctx)
{
    servers_manager_context_t *servers;

    if (!ctx) {
        return NULL;
    }
    servers = ctx->priv.servers_ctx;
    if (!servers) {
        servers = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
        ctx->priv.servers_ctx = servers;
    }
    return servers;
}

static int on_data_bury_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2,
                                          broadcast_priv_data_t *pdata)
{
    data_bury_manager_context_t *ctx = context;

    (void)pdata;
    if (!ctx) {
        return -1;
    }
    if (ctx->data_bury_is_enable && !ctx->data_bury_is_enable()) {
        return 0;
    }
    if (id.code == ID_BROADCAST_BLE_STATE_CHANGE) {
        return data_bury_event_handler_on_ble_notify_broadcast(ctx, arg1, arg2);
    }
    return 0;
}

static uint8_t data_bury_hook_publish_id_to_cmd(servers_publish_data_id_e id)
{
    switch (id) {
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_STARTUP:
        return APP_BURY_CMD_PUMP_STARTUP;
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN:
        return APP_BURY_CMD_PUMP_SHUTDOWN;
    case SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT:
        return APP_BURY_CMD_BT_CONNECT_EVENT;
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START:
        return APP_BURY_CMD_PUMP_SESSION_START;
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END:
        return APP_BURY_CMD_PUMP_SESSION_END;
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST:
        return APP_BURY_CMD_PUMP_MANUAL_ADJUST;
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT:
        return APP_BURY_CMD_PUMP_USER_EVENT;
    case SERVERS_PUBLISH_DATA_DATA_BURY_MILK_EJECTION:
        return APP_BURY_CMD_MILK_EJECTION;
    case SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START:
        return APP_BURY_CMD_BAT_CHARGE_START;
    case SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_END:
        return APP_BURY_CMD_BAT_CHARGE_END;
    case SERVERS_PUBLISH_DATA_DATA_BURY_EXCEPTION:
        return APP_BURY_CMD_EXCEPTION;
    case SERVERS_PUBLISH_DATA_DATA_BURY_FIRMWARE_UPDATE:
        return APP_BURY_CMD_FIRMWARE_UPDATE;
    case SERVERS_PUBLISH_DATA_DATA_BURY_OVERFLOW:
        return APP_BURY_CMD_OVERFLOW;
    case SERVERS_PUBLISH_DATA_DATA_BURY_MILK_VOLUME:
        return APP_BURY_CMD_MILK_VOLUME;
    case SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_START:
        return APP_BURY_CMD_HEAT_SESSION_START;
    case SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_END:
        return APP_BURY_CMD_HEAT_SESSION_END;
    case SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_START:
        return APP_BURY_CMD_MASSAGE_SESSION_START;
    case SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_END:
        return APP_BURY_CMD_MASSAGE_SESSION_END;
    case SERVERS_PUBLISH_DATA_DATA_BURY_WEAR_DETECTION:
        return APP_BURY_CMD_WEAR_DETECTION;
    default:
        return 0u;
    }
}

static servers_publish_data_id_e data_bury_hook_cmd_to_publish_id(uint8_t cmd_id)
{
    switch (cmd_id) {
    case APP_BURY_CMD_PUMP_STARTUP:
        return SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_STARTUP;
    case APP_BURY_CMD_PUMP_SHUTDOWN:
        return SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN;
    case APP_BURY_CMD_BT_CONNECT_EVENT:
        return SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT;
    case APP_BURY_CMD_PUMP_SESSION_START:
        return SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START;
    case APP_BURY_CMD_PUMP_SESSION_END:
        return SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END;
    case APP_BURY_CMD_PUMP_MANUAL_ADJUST:
        return SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST;
    case APP_BURY_CMD_PUMP_USER_EVENT:
        return SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT;
    case APP_BURY_CMD_MILK_EJECTION:
        return SERVERS_PUBLISH_DATA_DATA_BURY_MILK_EJECTION;
    case APP_BURY_CMD_BAT_CHARGE_START:
        return SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START;
    case APP_BURY_CMD_BAT_CHARGE_END:
        return SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_END;
    case APP_BURY_CMD_EXCEPTION:
        return SERVERS_PUBLISH_DATA_DATA_BURY_EXCEPTION;
    case APP_BURY_CMD_FIRMWARE_UPDATE:
        return SERVERS_PUBLISH_DATA_DATA_BURY_FIRMWARE_UPDATE;
    case APP_BURY_CMD_OVERFLOW:
        return SERVERS_PUBLISH_DATA_DATA_BURY_OVERFLOW;
    case APP_BURY_CMD_MILK_VOLUME:
        return SERVERS_PUBLISH_DATA_DATA_BURY_MILK_VOLUME;
    case APP_BURY_CMD_HEAT_SESSION_START:
        return SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_START;
    case APP_BURY_CMD_HEAT_SESSION_END:
        return SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_END;
    case APP_BURY_CMD_MASSAGE_SESSION_START:
        return SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_START;
    case APP_BURY_CMD_MASSAGE_SESSION_END:
        return SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_END;
    case APP_BURY_CMD_WEAR_DETECTION:
        return SERVERS_PUBLISH_DATA_DATA_BURY_WEAR_DETECTION;
    default:
        return (servers_publish_data_id_e)0;
    }
}

static const char *data_bury_hook_cmd_to_str(uint8_t cmd_id)
{
    switch (cmd_id) {
    case APP_BURY_CMD_PUMP_STARTUP:
        return "PUMP_STARTUP";
    case APP_BURY_CMD_PUMP_SHUTDOWN:
        return "PUMP_SHUTDOWN";
    case APP_BURY_CMD_BT_CONNECT_EVENT:
        return "BT_CONNECT_EVENT";
    case APP_BURY_CMD_PUMP_SESSION_START:
        return "PUMP_SESSION_START";
    case APP_BURY_CMD_PUMP_SESSION_END:
        return "PUMP_SESSION_END";
    case APP_BURY_CMD_PUMP_MANUAL_ADJUST:
        return "PUMP_MANUAL_ADJUST";
    case APP_BURY_CMD_PUMP_USER_EVENT:
        return "PUMP_USER_EVENT";
    case APP_BURY_CMD_MILK_EJECTION:
        return "MILK_EJECTION";
    case APP_BURY_CMD_BAT_CHARGE_START:
        return "BAT_CHARGE_START";
    case APP_BURY_CMD_BAT_CHARGE_END:
        return "BAT_CHARGE_END";
    case APP_BURY_CMD_EXCEPTION:
        return "EXCEPTION";
    case APP_BURY_CMD_FIRMWARE_UPDATE:
        return "FIRMWARE_UPDATE";
    case APP_BURY_CMD_OVERFLOW:
        return "OVERFLOW";
    case APP_BURY_CMD_MILK_VOLUME:
        return "MILK_VOLUME";
    case APP_BURY_CMD_HEAT_SESSION_START:
        return "HEAT_SESSION_START";
    case APP_BURY_CMD_HEAT_SESSION_END:
        return "HEAT_SESSION_END";
    case APP_BURY_CMD_MASSAGE_SESSION_START:
        return "MASSAGE_SESSION_START";
    case APP_BURY_CMD_MASSAGE_SESSION_END:
        return "MASSAGE_SESSION_END";
    case APP_BURY_CMD_WEAR_DETECTION:
        return "WEAR_DETECTION";
    default:
        return "UNKNOWN";
    }
}

static int data_bury_hook_append_to_ring(data_bury_manager_context_t *ctx, uint8_t cmd_id,
                                         const void *packed, uint16_t packed_len)
{
    storage_manager_context_t *st;
    data_bury_ring_append_args_t append;

    if (!ctx || !packed || packed_len == 0u) {
        LOGE("data bury hook append invalid ctx=%p packed=%p packed_len=%u", ctx, packed, packed_len);
        return -1;
    }
    if (packed_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
        LOGE("data bury hook append invalid cmd=0x%02x len=%u max=%u",
             cmd_id, (unsigned)packed_len, (unsigned)DATA_BURY_EVENT_PAYLOAD_MAX);
        return -2;
    }

    st = ctx->priv.storage_ctx;
    if (!st) {
        st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        ctx->priv.storage_ctx = st;
    }
    if (!st) {
        LOGE("data bury hook append invalid storage ctx=%p", ctx);
        return -3;
    }

    append.svc_id = APP_DATA_BURY_PROTOCOL_SVC_DATA;
    append.cmd_id = cmd_id;
    append.payload_len = packed_len;
    append.payload = packed;
    if (st->put_store_ram_val(STORE_ID_DATA_BURY_RING_APPEND, 0, 0, (void *)&append) != 0) {
        LOGE("data bury hook append ring failed cmd=0x%02x len=%u", cmd_id, (unsigned)packed_len);
        return -4;
    }
    LOG_BURY("data bury cached offline cmd=0x%02x len=%u", cmd_id, (unsigned)packed_len);
    return 0;
}

static int data_bury_hook_publish_event(data_bury_manager_context_t *ctx, data_bury_entry_extra_t *extra)
{
    ble_manager_context_t *ble;
    servers_manager_context_t *servers;
    servers_publish_data_args_t pub_args;
    uint8_t cmd_id;
    int ret;

    if (!ctx || !extra || !extra->val || extra->arg1 <= 0) {
        LOGE("data bury hook publish event invalid ctx=%p extra=%p", ctx, extra);
        return -1;
    }

    cmd_id = data_bury_hook_publish_id_to_cmd(extra->publish_id);
    if (cmd_id == 0u) {
        LOGE("data bury hook unknown publish id=%d", (int)extra->publish_id);
        return -2;
    }

#if DATA_BURY_LOG_DEBUG
    {
        const uint8_t *payload = (const uint8_t *)extra->val;
        LOG_BURY("data bury publish cmd=%s len=%d", data_bury_hook_cmd_to_str(cmd_id), extra->arg1);
        LOG_HEX("data bury publish data", payload, (size_t)extra->arg1);
    }
#endif

    ble = ctx->priv.ble_ctx;
    if (!ble) {
        ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
        ctx->priv.ble_ctx = ble;
    }

    pub_args.arg1 = extra->arg1;
    pub_args.val = extra->val;

    if (!ble || !ble->ble_is_connected()) {
        LOG_BURY("data bury offline cache cmd=%s (ble not connected)", data_bury_hook_cmd_to_str(cmd_id));
        return data_bury_hook_append_to_ring(ctx, cmd_id, extra->val, (uint16_t)extra->arg1);
    }

    servers = data_bury_hook_get_servers_ctx(ctx);
    if (!servers || !servers->hook || !servers->hook->data_bury_server_data_publish) {
        LOGE("data bury hook server not ready servers=%p", servers);
        return -3;
    }
    ret = servers->hook->data_bury_server_data_publish(extra->publish_id, &pub_args);
    if (ret == 0) {
        LOG_BURY("data bury online sent cmd=%s len=%d",
                 data_bury_hook_cmd_to_str(cmd_id), extra->arg1);
        return 0;
    }
    /* 发送过程中 BLE 断开，回落写环 */
    if (ret != -3) {
        LOGE("data bury online send fail id=%d cmd=0x%02x len=%d ret=%d",
             (int)extra->publish_id, cmd_id, extra->arg1, ret);
        return ret;
    }
    LOGW("data bury online send ret=-3 fallback ring id=%d cmd=0x%02x", (int)extra->publish_id, cmd_id);
    return data_bury_hook_append_to_ring(ctx, cmd_id, extra->val, (uint16_t)extra->arg1);
}

static void data_bury_hook_flush_pending_reschedule(data_bury_manager_context_t *ctx)
{
    if (ctx && ctx->data_bury_sys_msg_put) {
        (void)ctx->data_bury_sys_msg_put(DATA_BURY_EVT_FLUSH_PENDING, 100u, NULL);
    }
}

static int data_bury_hook_flush_pending(data_bury_manager_context_t *ctx)
{
    storage_manager_context_t *st;
    ble_manager_context_t *ble;
    servers_manager_context_t *servers;
    data_bury_rec_hdr_t hdr;
    uint8_t packed[DATA_BURY_EVENT_PAYLOAD_MAX];
    data_bury_ring_peek_args_t peek;
    servers_publish_data_id_e publish_id;
    servers_publish_data_args_t pub_args;
    uint16_t pending = 0;
    int ret;

    if (!ctx) {
        return -1;
    }

    servers = data_bury_hook_get_servers_ctx(ctx);
    if (!servers || !servers->hook || !servers->hook->data_bury_server_data_publish) {
        return -1;
    }

    st = ctx->priv.storage_ctx;
    if (!st) {
        st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        ctx->priv.storage_ctx = st;
    }
    ble = ctx->priv.ble_ctx;
    if (!ble) {
        ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
        ctx->priv.ble_ctx = ble;
    }
    if (!st || !ble || !ble->ble_is_connected()) {
        LOG_BURY("data bury flush pending failed st=%p ble=%p", st, ble);
        return -1;
    }

    if (st->get_store_ram_val(STORE_ID_DATA_BURY_RING_PENDING_COUNT, 0, 0, (void *)&pending) != 0
        || pending == 0u) {
        LOG_BURY("data bury flush pending no pending");
        return 0;
    }

    peek.hdr = &hdr;
    peek.payload = packed;
    peek.payload_buf_len = sizeof(packed);
    if (st->get_store_ram_val(STORE_ID_DATA_BURY_RING_PEEK_OLDEST, 0, 0, (void *)&peek) != 0) {
        LOGE("data bury flush peek fail pending=%u", (unsigned)pending);
        return -2;
    }
    if (hdr.svc_id != APP_DATA_BURY_PROTOCOL_SVC_DATA) {
        LOGE("data bury flush skip unknown svc=0x%02x cmd=0x%02x", hdr.svc_id, hdr.cmd_id);
        return -4;
    }

    publish_id = data_bury_hook_cmd_to_publish_id(hdr.cmd_id);
    if (publish_id == 0) {
        LOGE("data bury flush unknown cmd=0x%02x", hdr.cmd_id);
        return -5;
    }

    #if DATA_BURY_LOG_DEBUG
    LOG_BURY("data bury flush peek cmd=%s id=%d len=%u pending=%u",
             data_bury_hook_cmd_to_str(hdr.cmd_id),
             (int)publish_id, (unsigned)hdr.payload_len, (unsigned)pending);
    if (hdr.payload_len > 0u) {
        LOG_HEX("data bury flush packed", packed, hdr.payload_len);
    }
    #endif
    pub_args.arg1 = (int)hdr.payload_len;
    pub_args.val = packed;
    ret = servers->hook->data_bury_server_data_publish(publish_id, &pub_args);
    if (ret != 0) {
        LOGE("data bury flush send fail cmd=0x%02x id=%d len=%u ret=%d",
             hdr.cmd_id, (int)publish_id, (unsigned)hdr.payload_len, ret);
        return ret;
    }
    if (st->put_store_ram_val(STORE_ID_DATA_BURY_RING_POP_OLDEST, 0, 0, NULL) != 0) {
        LOGE("data bury flush pop fail cmd=0x%02x pending=%u", hdr.cmd_id, (unsigned)pending);
        return -3;
    }

    LOG_BURY("data bury flush sent one cmd=0x%02x len=%u pending_left=%u",
            hdr.cmd_id, (unsigned)hdr.payload_len, (unsigned)(pending > 0u ? pending - 1u : 0u));

    pending = 0;
    if (st->get_store_ram_val(STORE_ID_DATA_BURY_RING_PENDING_COUNT, 0, 0, (void *)&pending) == 0
        && pending > 0u) {
        LOG_BURY("data bury flush pending reschedule");
        data_bury_hook_flush_pending_reschedule(ctx);
    }
    return 1;
}

static int data_bury_thread_route(data_bury_manager_context_t *ctx, void *msg)
{
    multi_func_msg_data_t *m = (multi_func_msg_data_t *)msg;

    if (!ctx || !m) {
        return -1;
    }
    if (m->union_body.data_bury_extra.event_type == DATA_BURY_EVT_FLUSH_PENDING) {
        return data_bury_hook_flush_pending(ctx);
    }
    if (m->union_body.data_bury_extra.event_type == DATA_BURY_EVT_PUBLISH) {
        data_bury_entry_extra_t *extra = &m->union_body.data_bury_extra;
        int ret = data_bury_hook_publish_event(ctx, extra);
        if (ret != 0) {
            LOGE("data bury hook publish fail id=%d len=%d ret=%d",
                 (int)extra->publish_id, extra->arg1, ret);
        }
        if (extra->val) {
            if (extra->val_in_psram) {
                u_free_psram(extra->val, RES_PSRAM_TYPE);
            } else {
                u_free(extra->val);
            }
            extra->val = NULL;
            extra->val_in_psram = 0u;
        }
        return ret;
    }
    if (m->union_body.data_bury_extra.event_type == DATA_BURY_EVT_SESSION_END_COMMIT) {
        /* overlay 可扩展：泵奶会话结束延迟落盘/上报 */
        return 0;
    }
    return 0;
}

static int on_prepare_data_bury_hook_route(data_bury_manager_context_t *ctx)
{
    if (!ctx) {
        return -1;
    }
    data_bury_event_handler_init_params();
    ctx->priv.event_process_list = data_bury_event_process_list;
    ctx->priv.event_process_list_len = (uint16_t)LENGTH_OF_ARRAY(data_bury_event_process_list);
    return 0;
}

static int on_post_data_bury_hook_route(data_bury_manager_context_t *ctx)
{
    if (!ctx) {
        return -1;
    }
    ctx->priv.servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    ctx->priv.storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    ctx->priv.ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    return 0;
}

void register_data_bury_hook(data_bury_manager_context_t *ctx)
{
    if (!ctx) {
        return;
    }
    ctx->hook = u_malloz(sizeof(struct data_bury_manager_hook));
    if (!ctx->hook) {
        return;
    }
    ctx->hook->on_prepare_init_route = on_prepare_data_bury_hook_route;
    ctx->hook->on_post_init_route = on_post_data_bury_hook_route;
    ctx->hook->on_data_bury_br_receiver_route = on_data_bury_br_receiver_route;
    ctx->hook->data_bury_thread_route = data_bury_thread_route;
}
