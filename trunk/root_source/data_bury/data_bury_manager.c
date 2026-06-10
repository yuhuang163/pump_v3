/*
 * Momcozy 数据埋点 Manager：对外 publish / flush，线程与广播分发由 overlay hook 扩展
 */

#include "root_common_api.h"
#include "data_bury_manager.h"
#include "data_bury_msg_manager.h"
#include "data_bury_persist_defs.h"
#include "data_bury_persist_data.h"

static data_bury_manager_context_t *thiz = NULL;
/* 合并 50ms 定时 LOOP 投递，避免 bury 线程繁忙时队列被重复 tick 塞满 */
static volatile uint8_t s_msg_loop_queued;

static void data_bury_use_session_boot_seq_once(void)
{
    uint16_t seq = 0u;
    storage_manager_context_t *st;

    if (!thiz || thiz->priv.use_session_boot_inited) {
        return;
    }
    thiz->priv.use_session_boot_inited = 1u;

    st = thiz->priv.storage_ctx;
    if (!st) {
        st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        thiz->priv.storage_ctx = st;
    }
    if (st) {
        seq = st->priv.data_bury_info.boot_seq;
    }
    if (seq == 0xFFFFu) {
        seq = 0u;
    }
    seq++;
    if (seq == 0u) {
        seq = 1u;
    }
    thiz->priv.use_session_boot_seq = seq;
    if (st) {
        store_func_args_t args = {
            .ctx  = st,
            .arg1 = 0,
            .arg2 = 0,
            .val  = &seq,
        };

        st->priv.data_bury_info.boot_seq = seq;
        if (write_data_bury_use_session_boot_seq_to_flash(args) != 0) {
            LOGW("data bury boot_seq flash write fail, seq=%u (ram only)", (unsigned)seq);
        }
    }
    LOG_BURY("data bury use_session boot_seq=%u", (unsigned)seq);
}

static void data_bury_resolve_mac_for_session(uint8_t mac[6])
{
    ble_manager_context_t *ble;

    memset(mac, 0, 6);
    if (!thiz) {
        return;
    }
    ble = thiz->priv.ble_ctx;
    if (!ble) {
        ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
        thiz->priv.ble_ctx = ble;
    }
    if (ble && ble->get_mac_addr && ble->get_mac_addr(mac) == 0) {
        return;
    }
}

static uint8_t data_bury_publish_id_has_pump_session_id(servers_publish_data_id_e id)
{
    switch (id) {
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START:
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END:
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST:
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT:
    case SERVERS_PUBLISH_DATA_DATA_BURY_MILK_EJECTION:
    case SERVERS_PUBLISH_DATA_DATA_BURY_OVERFLOW:
    case SERVERS_PUBLISH_DATA_DATA_BURY_MILK_VOLUME:
        return 1u;
    default:
        return 0u;
    }
}

static uint32_t data_bury_get_use_session_id(void)
{
    uint8_t mac[6];

    if (!thiz) {
        return 0u;
    }
    data_bury_use_session_boot_seq_once();
    data_bury_resolve_mac_for_session(mac);
    return ((uint32_t)mac[1] << 24) | ((uint32_t)mac[0] << 16)
           | (uint32_t)(thiz->priv.use_session_boot_seq & 0xFFFFu);
}

static uint32_t data_bury_get_evt_timestamp(void)
{
    return platform_get_utc_time();
}

uint8_t data_bury_get_device_side(void)
{
    return (uint8_t)DATA_BURY_DEVICE_SIDE_DEFAULT;
}

uint8_t data_bury_get_pump_side(void)
{
    return (uint8_t)DATA_BURY_PUMP_SIDE_LEFT;
}

static uint16_t data_bury_get_current_pump_session_id(void)
{
    return thiz ? thiz->priv.current_pump_session_id : 0u;
}

static uint16_t data_bury_alloc_pump_session_id(void)
{
    uint16_t id;

    if (!thiz) {
        return 0u;
    }
    id = thiz->priv.pump_session_counter;
    thiz->priv.pump_session_counter++;
    thiz->priv.current_pump_session_id = id;
    LOG_BURY("data bury alloc pump_session_id=%u", (unsigned)id);
    return id;
}

static void data_bury_pump_session_begin(void)
{
    if (!thiz) {
        return;
    }
    thiz->priv.pump_session_active = 1u;
    thiz->priv.current_pump_session_id = 0u;
    LOG_BURY("data bury pump session begin");
}

static void data_bury_pump_session_end(void)
{
    if (!thiz) {
        return;
    }
    thiz->priv.pump_session_active = 0u;
    thiz->priv.current_pump_session_id = 0u;
    LOG_BURY("data bury pump session end");
}

static void data_bury_fill_pump_session_id(void *payload, uint16_t payload_len, uint16_t pump_session_id)
{
    uint8_t *buf;

    if (!payload || payload_len < DATA_BURY_EVT_PUMP_SESSION_ID_OFF + 2u) {
        return;
    }
    buf = (uint8_t *)payload;
    buf[DATA_BURY_EVT_PUMP_SESSION_ID_OFF] = (uint8_t)(pump_session_id & 0xFFu);
    buf[DATA_BURY_EVT_PUMP_SESSION_ID_OFF + 1u] = (uint8_t)(pump_session_id >> 8);
}

static void data_bury_fill_evt_common_hdr(void *payload, uint16_t payload_len, servers_publish_data_id_e id)
{
    data_bury_evt_common_hdr_t *hdr;
    uint16_t pump_session_id;

    if (!payload || payload_len < DATA_BURY_EVT_COMMON_HDR_LEN) {
        return;
    }
    hdr = (data_bury_evt_common_hdr_t *)payload;
    if (hdr->use_session_id == 0u || hdr->use_session_id == DATA_BURY_EVT_USE_SESSION_PLACEHOLDER) {
        hdr->use_session_id = data_bury_get_use_session_id();
    }
    if (hdr->evt_timestamp == 0u || hdr->evt_timestamp == DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER) {
        hdr->evt_timestamp = data_bury_get_evt_timestamp();
    }
    if (!data_bury_publish_id_has_pump_session_id(id)
        || payload_len < DATA_BURY_EVT_PUMP_SESSION_ID_OFF + 2u) {
        return;
    }
    pump_session_id = *(uint16_t *)((uint8_t *)payload + DATA_BURY_EVT_PUMP_SESSION_ID_OFF);
    if (pump_session_id != DATA_BURY_EVT_PUMP_SESSION_ID_PLACEHOLDER && pump_session_id != 0u) {
        return;
    }
    if (thiz && thiz->priv.pump_session_active && thiz->priv.current_pump_session_id != 0u) {
        data_bury_fill_pump_session_id(payload, payload_len, thiz->priv.current_pump_session_id);
    }
}

static uint8_t data_bury_is_enable(void)
{
    return thiz ? thiz->priv.enabled : 0u;
}

static uint8_t data_bury_set_enable(uint8_t enable)
{
    if (!thiz) {
        return 0;
    }
    thiz->priv.enabled = enable ? 1u : 0u;
#if DATA_BURY_MSG_POOL_ENABLE
    if (enable) {
        if (thiz->priv.bury_timer) {
            START_TIMER(thiz->priv.bury_timer, DATA_BURY_MSG_LOOP_PERIOD_MS);
        }
    } else {
        if (thiz->priv.bury_timer) {
            STOP_TIMER(thiz->priv.bury_timer);
        }
        data_bury_msg_mgr_clear(thiz);
    }
#endif
    LOG_BURY("data bury manager enable=%u", (unsigned)thiz->priv.enabled);
    return thiz->priv.enabled;
}

static uint16_t data_bury_publish_payload_len(servers_publish_data_id_e id, servers_publish_data_args_t *args)
{
    if (args && args->arg1 > 0) {
        return (uint16_t)args->arg1;
    }
    switch (id) {
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_STARTUP:
        return (uint16_t)sizeof(data_bury_evt_pump_startup_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN:
        return (uint16_t)sizeof(data_bury_evt_pump_shutdown_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT:
        return (uint16_t)sizeof(data_bury_evt_bt_connect_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START:
        return (uint16_t)sizeof(data_bury_evt_pump_session_start_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST:
        return (uint16_t)sizeof(data_bury_evt_pump_manual_adjust_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT:
        return (uint16_t)sizeof(data_bury_evt_pump_user_evt_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_MILK_EJECTION:
        return (uint16_t)sizeof(data_bury_evt_milk_ejection_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START:
        return (uint16_t)sizeof(data_bury_evt_bat_charge_start_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_END:
        return (uint16_t)sizeof(data_bury_evt_bat_charge_end_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_EXCEPTION:
        return (uint16_t)sizeof(data_bury_evt_exception_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_FIRMWARE_UPDATE:
        return (uint16_t)sizeof(data_bury_evt_firmware_update_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_OVERFLOW:
        return (uint16_t)sizeof(data_bury_evt_overflow_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_START:
        return (uint16_t)sizeof(data_bury_evt_heat_session_start_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_END:
        return (uint16_t)sizeof(data_bury_evt_heat_session_end_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_START:
        return (uint16_t)sizeof(data_bury_evt_massage_session_start_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_END:
        return (uint16_t)sizeof(data_bury_evt_massage_session_end_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_WEAR_DETECTION:
        return (uint16_t)sizeof(data_bury_evt_wear_detection_t);
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END:
    case SERVERS_PUBLISH_DATA_DATA_BURY_MILK_VOLUME:
    default:
        return 0u;
    }
}

static int data_bury_publish(servers_publish_data_id_e id, servers_publish_data_args_t *args)
{
    data_bury_msg_pool_payload_t payload = {0};
    uint16_t payload_len;
    int ret;

    EXCEPTION_CHECK_INT(thiz);
    if (!thiz->priv.enabled) {
        return -5;
    }
    EXCEPTION_CHECK_INT(args);
    if (!args->val) {
        return -1;
    }

    payload_len = data_bury_publish_payload_len(id, args);
    if (payload_len == 0u || payload_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
        LOGE("data bury publish invalid len id=%d len=%u", (int)id, (unsigned)payload_len);
        return -3;
    }

    data_bury_fill_evt_common_hdr(args->val, payload_len, id);

    payload.publish_id = id;
    payload.arg1 = (int)payload_len;
    payload.val = args->val;
#if DATA_BURY_MSG_POOL_ENABLE
    ret = data_bury_sys_msg_put(thiz, DATA_BURY_EVT_PUBLISH, 0u, &payload);
    if (ret != 0) {
        LOGE("data bury publish queue fail id=%d len=%u ret=%d", (int)id, (unsigned)payload_len, ret);
    } else {
        LOG_BURY("data bury publish queued id=%d len=%u", (int)id, (unsigned)payload_len);
    }
    return ret;
#else
    {
        multi_func_msg_data_t msg = {0};
        void *payload_copy = u_malloz(payload_len);

        if (!payload_copy) {
            return -4;
        }
        memcpy(payload_copy, args->val, payload_len);
        msg.union_body.data_bury_extra.event_type = DATA_BURY_EVT_PUBLISH;
        msg.union_body.data_bury_extra.publish_id = id;
        msg.union_body.data_bury_extra.arg1 = (int)payload_len;
        msg.union_body.data_bury_extra.val = payload_copy;
        if (MSG_QUEUE_SEND(thiz->priv.bury_thd.msg_id, &msg, 0) != osOK) {
            u_free(payload_copy);
            return -5;
        }
        return 0;
    }
#endif
}

static int data_bury_flush_pending(void)
{
    EXCEPTION_CHECK_INT(thiz);
    if (!thiz->priv.enabled) {
        return -5;
    }
#if DATA_BURY_MSG_POOL_ENABLE
    return data_bury_sys_msg_put(thiz, DATA_BURY_EVT_FLUSH_PENDING, 0, NULL);
#else
    multi_func_msg_data_t msg = {0};
    msg.union_body.data_bury_extra.event_type = DATA_BURY_EVT_FLUSH_PENDING;
    return MSG_QUEUE_SEND(thiz->priv.bury_thd.msg_id, &msg, 0);
#endif
}

static int data_bury_sys_msg_put_wrapper(uint16_t event, uint32_t delay_ms,
                                         data_bury_msg_pool_payload_t *payload)
{
    EXCEPTION_CHECK_INT(thiz);
#if DATA_BURY_MSG_POOL_ENABLE
    return data_bury_sys_msg_put(thiz, event, delay_ms, payload);
#else
    multi_func_msg_data_t msg = {0};

    (void)delay_ms;
    msg.union_body.data_bury_extra.event_type = event;
    if (event == DATA_BURY_EVT_PUBLISH && payload) {
        msg.union_body.data_bury_extra.publish_id = payload->publish_id;
        msg.union_body.data_bury_extra.arg1 = payload->arg1;
        msg.union_body.data_bury_extra.val = payload->val;
    }
    return MSG_QUEUE_SEND(thiz->priv.bury_thd.msg_id, &msg, 0);
#endif
}

static void data_bury_sys_msg_remove_wrapper(uint16_t event)
{
    if (!thiz) {
        return;
    }
#if DATA_BURY_MSG_POOL_ENABLE
    data_bury_sys_msg_remove(thiz, event);
#endif
}

static void data_bury_msg_timer_handler(void *handle)
{
    data_bury_manager_context_t *obj = (data_bury_manager_context_t *)handle;
    multi_func_msg_data_t msg = {0};

    if (!obj || !obj->priv.enabled || !obj->priv.bury_thd.msg_id) {
        return;
    }
    if (s_msg_loop_queued) {
        return;
    }
    msg.union_body.data_bury_extra.event_type = DATA_BURY_EVT_MSG_POOL_LOOP;
    if (MSG_QUEUE_SEND(obj->priv.bury_thd.msg_id, &msg, 0) == osOK) {
        s_msg_loop_queued = 1u;
    } else {
        LOGW("data bury msg loop queue full");
    }
}

static void on_data_bury_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2,
                                    broadcast_priv_data_t *pdata)
{
    data_bury_manager_context_t *ctx = context;

    if (!ctx || !ctx->hook || !ctx->hook->on_data_bury_br_receiver_route) {
        return;
    }
    ctx->hook->on_data_bury_br_receiver_route(id, context, arg1, arg2, pdata);
}

static void data_bury_manager_internal_dispatch_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    multi_func_msg_data_t msg = {0};
    data_bury_manager_context_t *obj = thiz;

    CREATE_MSG_QUEUE(&thd->msg_id, DATA_BURY_EVT_MESQ_NUM);
#if DATA_BURY_MSG_POOL_ENABLE
    CREATE_TIMER(&obj->priv.bury_timer, "data_bury_msg_timer", obj, OS_TIMER_PERIODIC,
                 data_bury_msg_timer_handler);
    if (obj->priv.enabled) {
        START_TIMER(obj->priv.bury_timer, DATA_BURY_MSG_LOOP_PERIOD_MS);
    }
#endif
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        if (msg.union_body.data_bury_extra.event_type == DATA_BURY_EVT_MSG_POOL_LOOP) {
            s_msg_loop_queued = 0u;
            data_bury_msg_mgr_process_due(obj, 1u);
            continue;
        }
        if (obj && obj->hook && obj->hook->data_bury_thread_route) {
            obj->hook->data_bury_thread_route(obj, &msg);
        }
    }
}

static int on_prepare_data_bury_init(void *self)
{
    data_bury_manager_context_t *ctx = (data_bury_manager_context_t *)self;

    if (ctx->hook && ctx->hook->on_prepare_init_route) {
        ctx->hook->on_prepare_init_route(ctx);
    }
    ctx->priv.enabled = 1;
    ctx->priv.use_session_boot_inited = 0u;
    ctx->priv.pump_session_counter = 0u;
    ctx->priv.current_pump_session_id = 0u;
    ctx->priv.pump_session_active = 0u;
    MUTEX_INIT(&ctx->priv.m_lock, data_bury_manager_mutex);
#if DATA_BURY_MSG_POOL_ENABLE
    data_bury_msg_mgr_init(ctx);
#endif
    return 0;
}

static int on_post_data_bury_init(void *self)
{
    data_bury_manager_context_t *ctx = (data_bury_manager_context_t *)self;

    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }

    ctx->priv.servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    ctx->priv.storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    ctx->priv.ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);

    ctx->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    if (ctx->priv.bcd) {
        ctx->priv.bcd->context = ctx;
        ctx->priv.bcd->name = "data_bury_thread";
        ctx->priv.bcd->id.code |= ID_BROADCAST_BLE_STATE_CHANGE;
        ctx->priv.bcd->receiver_cb = on_data_bury_receiver;
        register_broadcast_receiver(ctx->priv.bcd);
    }

    RUN_NEW_THREAD(&ctx->priv.bury_thd, "data_bury_thd",
                   data_bury_manager_internal_dispatch_thread,
                   RT_PRIO_DATA_BURY_DISPATCH_THREAD, INPUT_DATA_BURY_THREAD_STACKSIZE);
    LOG_BURY("data bury manager init ok");
    return 0;
}

void *register_module_for_data_burial_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, data_bury_manager_context_t);

    ptr->data_bury_is_enable = data_bury_is_enable;
    ptr->data_bury_set_enable = data_bury_set_enable;
    ptr->data_bury_publish = data_bury_publish;
    ptr->data_bury_flush_pending = data_bury_flush_pending;
    ptr->data_bury_sys_msg_put = data_bury_sys_msg_put_wrapper;
    ptr->data_bury_sys_msg_remove = data_bury_sys_msg_remove_wrapper;
    ptr->data_bury_get_use_session_id = data_bury_get_use_session_id;
    ptr->data_bury_get_evt_timestamp = data_bury_get_evt_timestamp;
    ptr->data_bury_get_device_side = data_bury_get_device_side;
    ptr->data_bury_get_pump_side = data_bury_get_pump_side;
    ptr->data_bury_get_current_pump_session_id = data_bury_get_current_pump_session_id;
    ptr->data_bury_alloc_pump_session_id = data_bury_alloc_pump_session_id;
    ptr->data_bury_pump_session_begin = data_bury_pump_session_begin;
    ptr->data_bury_pump_session_end = data_bury_pump_session_end;
    ptr->data_bury_fill_evt_common_hdr = data_bury_fill_evt_common_hdr;
    ptr->data_bury_fill_pump_session_id = data_bury_fill_pump_session_id;
    thiz = ptr;
    return ADD_MODULE("data_bury", ID_MODULE_DATA_BURIAL, on_prepare_data_bury_init, on_post_data_bury_init,
                      ptr, list_head, sizeof(data_bury_manager_context_t));
}
