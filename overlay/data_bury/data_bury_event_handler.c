/*
 * overlay 数据埋点事件处理：外部 trigger -> 埋点 payload 采集与上报
 */

#include "root_common.h"
#include "root_common_api.h"
#include "data_bury_persist_defs.h"
#include "data_bury_event_handler.h"
#include "app_data_bury_protocol_defs.h"
#include "persist_storage.h" // IWYU pragma: keep

typedef struct {
    uint8_t session_start_mode;
    uint8_t bt_session_cached;
    uint8_t bt_remote_initiated;
    uint32_t bt_connect_timestamp_utc;
    uint16_t bt_connection_setup_duration;
    int8_t bt_signal_strength_rssi;
    uint8_t mode_queue_tracking;
    uint8_t mode_queue_count;
    data_bury_evt_program_duration_t mode_queue[DATA_BURY_PUMP_SESSION_MAX_MODES];
    uint8_t pump_session_start_pending;
    uint32_t pending_pump_session_start_ms;
    data_bury_evt_pump_session_start_t pending_pump_session_start;
} data_bury_event_runtime_t;

static data_bury_event_runtime_t s_event_rt;

static void data_bury_session_mode_queue_reset(void)
{
    s_event_rt.mode_queue_tracking = 0u;
    s_event_rt.mode_queue_count = 0u;
    memset(s_event_rt.mode_queue, 0, sizeof(s_event_rt.mode_queue));
}

static void data_bury_session_mode_queue_begin(void)
{
    data_bury_session_mode_queue_reset();
    s_event_rt.mode_queue_tracking = 1u;
}

static int data_bury_session_mode_queue_push(uint32_t mode_or_program_id, uint16_t duration_sec)
{
    uint8_t idx;

    if (!s_event_rt.mode_queue_tracking || s_event_rt.mode_queue_count >= DATA_BURY_PUMP_SESSION_MAX_MODES) {
        return -1;
    }
    idx = s_event_rt.mode_queue_count;
    s_event_rt.mode_queue[idx].mode_or_program_id = mode_or_program_id;
    s_event_rt.mode_queue[idx].duration_sec = duration_sec;
    s_event_rt.mode_queue_count++;
    return 0;
}

static int data_bury_event_publish(data_bury_manager_context_t *ctx, servers_publish_data_id_e id,
                                   void *payload, uint16_t payload_len, uint32_t delay_ms)
{
    servers_publish_data_args_t args;

    if (!ctx || !ctx->data_bury_publish || !payload || payload_len == 0u) {
        return -1;
    }
    args.arg1 = (int)payload_len;
    args.arg2 = (int)delay_ms;
    args.val = payload;
    return ctx->data_bury_publish(id, &args);
}

static uint8_t data_bury_event_publish_id_to_cmd(servers_publish_data_id_e id)
{
    switch (id) {
    case SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN:
        return APP_BURY_CMD_PUMP_SHUTDOWN;
    case SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT:
        return APP_BURY_CMD_BT_CONNECT_EVENT;
    default:
        return 0u;
    }
}

static int data_bury_event_cache_publish(data_bury_manager_context_t *ctx, servers_publish_data_id_e id,
                                         void *payload, uint16_t payload_len)
{
    storage_manager_context_t *st;
    data_bury_ring_append_args_t append;
    uint8_t cmd_id;

    if (!ctx || !payload || payload_len == 0u) {
        return -1;
    }
    cmd_id = data_bury_event_publish_id_to_cmd(id);
    if (cmd_id == 0u) {
        LOGE("data bury event cache unknown id=%d", (int)id);
        return -2;
    }
    if (ctx->data_bury_fill_evt_common_hdr) {
        ctx->data_bury_fill_evt_common_hdr(payload, payload_len, id);
    }
    st = ctx->priv.storage_ctx;
    if (!st) {
        st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        ctx->priv.storage_ctx = st;
    }
    if (!st || !st->put_store_ram_val) {
        LOGE("data bury event cache storage not ready id=%d", (int)id);
        return -3;
    }
    append.svc_id = APP_DATA_BURY_PROTOCOL_SVC_DATA;
    append.cmd_id = cmd_id;
    append.payload_len = payload_len;
    append.payload = payload;
    if (st->put_store_ram_val(STORE_ID_DATA_BURY_RING_APPEND, 0, 0, (void *)&append) != 0) {
        LOGE("data bury event cache fail id=%d cmd=0x%02x len=%u", (int)id, cmd_id, (unsigned)payload_len);
        return -4;
    }
    LOG_BURY("data bury event cached id=%d cmd=0x%02x len=%u", (int)id, cmd_id, (unsigned)payload_len);
    return 0;
}

static uint8_t data_bury_event_get_battery_percent(void)
{
    battery_manager_context_t *bat = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);

    if (bat && bat->get_battery_percent) {
        return bat->get_battery_percent();
    }
    return 0u;
}

static data_bury_pump_mode_e data_bury_convert_pump_mode(uint8_t app_mode)
{
    switch (app_mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return DATA_BURY_PUMP_MODE_STIMULATION;
        case BREAST_PUMP_LACTATION_MODE:
            return DATA_BURY_PUMP_MODE_EXPRESSION;
        case BREAST_PUMP_MIX_MODE:
            return DATA_BURY_PUMP_MODE_MIX;
        case BREAST_PUMP_CUSTOM_MODE:
        case BREAST_PUMP_CUSTOM_TRIAL_MODE:
            return DATA_BURY_PUMP_MODE_RHYTHM;
        case BREAST_PUMP_MILK_BOOST_MODE:
            return DATA_BURY_PUMP_MODE_MILK_BOOST;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return DATA_BURY_PUMP_MODE_MILK_TRACK;
        default:
            return DATA_BURY_PUMP_MODE_STIMULATION;
    }
}

typedef struct {
    uint32_t percent;
    uint32_t voltage_mv;
    uint32_t current_ma;
    int32_t temp_c;
} data_bury_battery_charge_metrics_t;

static data_bury_battery_charge_metrics_t data_bury_event_sample_battery_charge_metrics(void)
{
    data_bury_battery_charge_metrics_t metrics = {0};
    peripheral_bus_context_t *pbus;
    rt_device_node_t *battery_dev;
    uint16_t ibat_ma = 0;

    metrics.percent = data_bury_event_get_battery_percent();

    pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    if (!pbus || !pbus->find_device) {
        return metrics;
    }
    battery_dev = pbus->find_device(ID_DEVICE_BATTERY);
    if (!battery_dev || !battery_dev->ioctl) {
        return metrics;
    }

    DEVICE_IOCTL_NO_RET(battery_dev, OP_CODE_BAT_POLL_VOLTAGE);

    DEVICE_IOCTL_HAS_RET(battery_dev, OP_CODE_BAT_GET_VOLTAGE, voltage_ret);
    if (voltage_ret >= 0) {
        metrics.voltage_mv = (uint16_t)voltage_ret;
    }

    DEVICE_IOCTL_HAS_RET(battery_dev, OP_CODE_BAT_GET_CHARGE_TEMPERATURE, temp_ret);
    if (temp_ret >= 0) {
        metrics.temp_c = (int8_t)temp_ret;
    }

    DEVICE_IOCTL_ARGS_HAS_RET(battery_dev, OP_CODE_BAT_GET_CHARGE_IBAT_MA, &ibat_ma, current_ret);
    if (current_ret == 0) {
        metrics.current_ma = (int16_t)ibat_ma;
    }
    return metrics;
}

static uint16_t data_bury_event_get_suction_level_desktop(breast_pump_manager_context_t *pump)
{
    pump_setting_t setting = {0};
    int time_index = 0;

    if (!pump) {
        return 0u;
    }
    READ_PUMP_MODE_STEP_INDEX(pump, time_index);
    READ_PUMP_SETTING_INFO(pump, time_index, setting);
    return DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(setting.level[0], setting.level[1]);
}

static void data_bury_event_publish_pump_startup(data_bury_manager_context_t *ctx)
{
    data_bury_evt_pump_startup_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.firmware_version = (uint16_t)ROOT_FIRMWARE_VERSION;
    evt.battery_level_startup = data_bury_event_get_battery_percent();
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_STARTUP,
                                    &evt, (uint16_t)sizeof(evt), 100u);
}

static void data_bury_event_cache_pump_shutdown(data_bury_manager_context_t *ctx, uint8_t reason)
{
    data_bury_evt_pump_shutdown_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.battery_level_shutdown = data_bury_event_get_battery_percent();
    evt.shutdown_reason = reason;
    (void)data_bury_event_cache_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN,
                                        &evt, (uint16_t)sizeof(evt));
}

static int8_t data_bury_event_sample_ble_rssi(data_bury_manager_context_t *ctx)
{
    ble_manager_context_t *ble = ctx->priv.ble_ctx;
    int32_t rssi = 0;

    if (!ble) {
        ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
        ctx->priv.ble_ctx = ble;
    }
    if (!ble || !ble->priv.ble_dev || !ble->priv.ble_dev->ioctl) {
        return (int8_t)0xFF;
    }
    if (ble->priv.connect_idx == 0u) {
        return (int8_t)0xFF;
    }
    (void)DEVICE_IOCTL_ARGS(ble->priv.ble_dev, OP_CODE_BLE_TEST_GET_RSSI_VALUE, &rssi);
    return (int8_t)rssi;
}

static uint8_t data_bury_event_hci_to_app_disconnect_reason(uint8_t hci_reason)
{
    switch (hci_reason) {
    case 0x00:
    case 0x13: /* Remote User Terminated Connection */
        return DATA_BURY_BT_DISCONNECT_REASON_USER;
    case 0x08: /* Connection Timeout */
        return DATA_BURY_BT_DISCONNECT_REASON_TIMEOUT;
    default:
        return DATA_BURY_BT_DISCONNECT_REASON_SKIP;
    }
}

static void data_bury_event_bt_session_cache_clear(void)
{
    s_event_rt.bt_session_cached = 0u;
    s_event_rt.bt_remote_initiated = 0u;
    s_event_rt.bt_connect_timestamp_utc = 0u;
    s_event_rt.bt_connection_setup_duration = 0u;
    s_event_rt.bt_signal_strength_rssi = (int8_t)0xFF;
}

static void data_bury_event_bt_session_cache_save(data_bury_manager_context_t *ctx,
                                                  const data_bury_trigger_args_t *args)
{
    int8_t rssi = (int8_t)(args->arg2 & 0xFF);

    (void)ctx;
    data_bury_event_bt_session_cache_clear();
    uint32_t setup_ms = (args->arg1 > 0) ? (uint32_t)args->arg1 : 0u;

    if (setup_ms > 0xFFFFu) {
        setup_ms = 0xFFFFu;
    }
    s_event_rt.bt_session_cached = 1u;
    s_event_rt.bt_remote_initiated = 1u;
    s_event_rt.bt_connect_timestamp_utc = platform_get_utc_time();
    s_event_rt.bt_connection_setup_duration = (uint16_t)setup_ms;
    if (rssi == 0 || rssi == (int8_t)0xFF) {
        rssi = data_bury_event_sample_ble_rssi(ctx);
    }
    s_event_rt.bt_signal_strength_rssi = rssi;
    LOG_BURY("bt session cache: ts=%u setup_ms=%u rssi=%d",
             (unsigned)s_event_rt.bt_connect_timestamp_utc,
             (unsigned)s_event_rt.bt_connection_setup_duration,
             (int)s_event_rt.bt_signal_strength_rssi);
}

static int data_bury_event_build_bt_session_end_evt(data_bury_manager_context_t *ctx, uint8_t hci_reason,
                                                    data_bury_evt_bt_connect_t *evt)
{
    uint8_t app_reason;

    if (!ctx || !evt) {
        return -1;
    }
    if (!s_event_rt.bt_session_cached || !s_event_rt.bt_remote_initiated) {
        return 0;
    }
    app_reason = data_bury_event_hci_to_app_disconnect_reason(hci_reason);
    if (app_reason == DATA_BURY_BT_DISCONNECT_REASON_SKIP) {
        LOG_BURY("bt session end skip: hci_reason=0x%02x", (unsigned)hci_reason);
        data_bury_event_bt_session_cache_clear();
        return 0;
    }

    memset(evt, 0, sizeof(*evt));
    evt->device_side = ctx->data_bury_get_device_side();
    evt->use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt->evt_timestamp = s_event_rt.bt_connect_timestamp_utc;
    evt->connect_result = DATA_BURY_BT_CONNECT_RESULT_SESSION_END;
    evt->connection_setup_duration = s_event_rt.bt_connection_setup_duration;
    evt->bt_signal_strength_rssi = s_event_rt.bt_signal_strength_rssi;
    evt->disconnect_timestamp = platform_get_utc_time();
    evt->disconnect_reason = app_reason;
    LOG_BURY("bt session end: connect_ts=%u setup_ms=%u rssi=%d reason=0x%02x",
             (unsigned)evt->evt_timestamp,
             (unsigned)evt->connection_setup_duration,
             (int)evt->bt_signal_strength_rssi,
             (unsigned)evt->disconnect_reason);
    return 1;
}

static int data_bury_event_cache_bt_session_end(data_bury_manager_context_t *ctx, uint8_t hci_reason)
{
    data_bury_evt_bt_connect_t evt = {0};
    int built;

    built = data_bury_event_build_bt_session_end_evt(ctx, hci_reason, &evt);
    if (built <= 0) {
        return built;
    }
    (void)data_bury_event_cache_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT,
                                          &evt, (uint16_t)sizeof(evt));
    data_bury_event_bt_session_cache_clear();
    return 0;
}

static int data_bury_event_publish_bt_session_end(data_bury_manager_context_t *ctx, uint8_t hci_reason)
{
    data_bury_evt_bt_connect_t evt = {0};
    int built;

    built = data_bury_event_build_bt_session_end_evt(ctx, hci_reason, &evt);
    if (built <= 0) {
        return built;
    }
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT,
                                    &evt, (uint16_t)sizeof(evt), 0u);
    data_bury_event_bt_session_cache_clear();
    return 0;
}

static void data_bury_pump_session_pending_discard(data_bury_manager_context_t *ctx)
{
    s_event_rt.pump_session_start_pending = 0u;
    s_event_rt.pending_pump_session_start_ms = 0u;
    memset(&s_event_rt.pending_pump_session_start, 0, sizeof(s_event_rt.pending_pump_session_start));
    data_bury_session_mode_queue_reset();
    if (ctx && ctx->data_bury_pump_session_end) {
        ctx->data_bury_pump_session_end();
    }
}

static void data_bury_event_buffer_pump_session_start(data_bury_manager_context_t *ctx)
{
    breast_pump_manager_context_t *pump =
        (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    data_bury_evt_pump_session_start_t evt = {0};
    int mode = 0;

    if (!pump) {
        return;
    }
    READ_PUMP_WORK_MODE(pump, mode);
    uint8_t bury_mode = (uint8_t)data_bury_convert_pump_mode((uint8_t)mode);
    if (ctx->data_bury_pump_session_begin) {
        ctx->data_bury_pump_session_begin();
    }
    s_event_rt.session_start_mode = bury_mode;

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = platform_get_utc_time();
    evt.pump_session_id = 0u;
    evt.initial_mode_type = bury_mode;
    evt.initial_suction_level = 0u;
    evt.initial_suction_level_desktop = data_bury_event_get_suction_level_desktop(pump);
    evt.initial_battery_level = data_bury_event_get_battery_percent();

    s_event_rt.pending_pump_session_start = evt;
    s_event_rt.pending_pump_session_start_ms = get_timestamp_ms(0);
    s_event_rt.pump_session_start_pending = 1u;
    data_bury_session_mode_queue_begin();
    LOG_BURY("pump session start buffered ts=%u (commit on valid end >= %us)",
             (unsigned)evt.evt_timestamp, (unsigned)DATA_BURY_PUMP_SESSION_MIN_DURATION_SEC);
}

static void data_bury_event_record_pump_mode_switch(uint32_t adjust_from, uint32_t adjust_to,
                                                    uint16_t duration_sec)
{
    if (!s_event_rt.mode_queue_tracking || adjust_from == adjust_to) {
        return;
    }

    (void)data_bury_session_mode_queue_push(adjust_from, duration_sec);
    LOG_BURY("mode switch queue: from=%u to=%u dur=%u count=%u",
             (unsigned)adjust_from, (unsigned)adjust_to, (unsigned)duration_sec,
             (unsigned)s_event_rt.mode_queue_count);
}

static void data_bury_event_publish_pump_manual_adjust(data_bury_manager_context_t *ctx,
                                                       const data_bury_trigger_manual_adjust_t *adj)
{
    data_bury_evt_pump_manual_adjust_t evt = {0};
    uint16_t session_id = 0u;

    if (!adj) {
        return;
    }
    if (adj->evt_type == DATA_BURY_ADJUST_SUCTION_LEVEL) {
        if (adj->adjust_from_desktop == adj->adjust_to_desktop) {
            return;
        }
    } else if (adj->adjust_from == adj->adjust_to) {
        return;
    }

    if (ctx->data_bury_get_current_pump_session_id) {
        session_id = ctx->data_bury_get_current_pump_session_id();
    }

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.pump_session_id = session_id;
    evt.evt_source = adj->evt_source;
    evt.evt_type = adj->evt_type;
    if (adj->evt_type == DATA_BURY_ADJUST_SUCTION_LEVEL) {
        evt.adjust_from_desktop = adj->adjust_from_desktop;
        evt.adjust_to_desktop = adj->adjust_to_desktop;
    } else if (adj->evt_type == DATA_BURY_ADJUST_PUMP_MODE) {
        evt.adjust_from = (uint8_t)data_bury_convert_pump_mode(adj->adjust_from);
        evt.adjust_to = (uint8_t)data_bury_convert_pump_mode(adj->adjust_to);
    } else {
        evt.adjust_from = adj->adjust_from;
        evt.adjust_to = adj->adjust_to;
    }
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST,
                                    &evt, (uint16_t)sizeof(evt), 0u);
    LOG_BURY("manual adjust publish: sid=%u src=%u type=%u from=%u to=%u desktop=0x%04x->0x%04x",
             (unsigned)session_id, (unsigned)adj->evt_source, (unsigned)adj->evt_type,
             (unsigned)evt.adjust_from, (unsigned)evt.adjust_to,
             (unsigned)evt.adjust_from_desktop, (unsigned)evt.adjust_to_desktop);
}

static void data_bury_event_publish_pump_user_evt(data_bury_manager_context_t *ctx,
                                                  uint8_t evt_source, uint8_t evt_type)
{
    data_bury_evt_pump_user_evt_t evt = {0};
    uint16_t session_id = 0u;

    if (ctx->data_bury_get_current_pump_session_id) {
        session_id = ctx->data_bury_get_current_pump_session_id();
    }

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.pump_session_id = session_id;
    evt.evt_source = evt_source;
    evt.evt_type = evt_type;
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT,
                                  &evt, (uint16_t)sizeof(evt), 0u);
    LOG_BURY("user evt publish: sid=%u src=%u type=%u",
             (unsigned)session_id, (unsigned)evt_source, (unsigned)evt_type);
}

static void data_bury_event_publish_wear_detection(data_bury_manager_context_t *ctx,
                                                   uint8_t detection_type,
                                                   uint32_t detection_data)
{
    data_bury_evt_wear_detection_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.detection_type = detection_type;
    evt.detection_data = detection_data;
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_WEAR_DETECTION,
                                  &evt, (uint16_t)sizeof(evt), 0u);
    LOG_BURY("wear detection publish: type=%u data=0x%08x",
             (unsigned)detection_type, (unsigned)detection_data);
}

static void data_bury_event_publish_exception(data_bury_manager_context_t *ctx,
                                              uint16_t error_code, uint32_t error_data)
{
    data_bury_evt_exception_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.error_code = error_code;
    evt.error_data = error_data;
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_EXCEPTION,
                                  &evt, (uint16_t)sizeof(evt), 0u);
    LOG_BURY("exception publish: code=0x%04x data=0x%08x",
             (unsigned)error_code, (unsigned)error_data);
}

static void data_bury_event_commit_pump_session_end(data_bury_manager_context_t *ctx, uint8_t end_reason)
{
    breast_pump_manager_context_t *pump =
        (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    uint8_t *buf = NULL;
    data_bury_evt_pump_session_end_fixed_t *evt;
    data_bury_evt_program_duration_t *program_durations;
    pump_result_info_t result = {0};
    int mode = 0;
    uint16_t session_id = 0;
    uint8_t used_mode_count;
    uint16_t duration_sec;
    uint16_t payload_len;
    uint32_t session_duration_ms;
    uint8_t i;

    if (!pump) {
        return;
    }
    if (!s_event_rt.pump_session_start_pending) {
        LOG_BURY("pump session end: no pending start, discard");
        data_bury_pump_session_pending_discard(ctx);
        return;
    }

    session_duration_ms = get_timestamp_ms(s_event_rt.pending_pump_session_start_ms);
    if (session_duration_ms < (DATA_BURY_PUMP_SESSION_MIN_DURATION_SEC * 1000u)) {
        LOG_BURY("pump session end: duration=%u ms < %u s, discard",
                 (unsigned)session_duration_ms, (unsigned)DATA_BURY_PUMP_SESSION_MIN_DURATION_SEC);
        data_bury_pump_session_pending_discard(ctx);
        return;
    }

    buf = u_malloz(DATA_BURY_EVT_PUMP_SESSION_END_MAX_LEN);
    if (!buf) {
        LOGE("pump session end: alloc buf fail len=%u",
             (unsigned)DATA_BURY_EVT_PUMP_SESSION_END_MAX_LEN);
        data_bury_pump_session_pending_discard(ctx);
        return;
    }
    evt = (data_bury_evt_pump_session_end_fixed_t *)buf;

    if (ctx->data_bury_alloc_pump_session_id_on_commit) {
        session_id = ctx->data_bury_alloc_pump_session_id_on_commit();
    }
    if (session_id == 0u && ctx->data_bury_get_current_pump_session_id) {
        session_id = ctx->data_bury_get_current_pump_session_id();
    }

    READ_PUMP_WORK_MODE(pump, mode);
    const pump_result_info_t *result_info = NULL;
    READ_PUMP_RESULT_INFO(pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    duration_sec = result_info ? (uint16_t)(result_info->current_mode_work_time / 1000u) : 0u;
    uint8_t bury_mode = (uint8_t)data_bury_convert_pump_mode((uint8_t)mode);
    uint32_t mode_or_program_id = (uint32_t)bury_mode;
    if (s_event_rt.mode_queue_tracking) {
        ///如果是custom，需要填写program id
        if (mode == BREAST_PUMP_CUSTOM_MODE) {
            pump_mode_steps_info_t steps_info;
            storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (storage_ctx) {
                storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &steps_info);
                mode_or_program_id = steps_info.id.dat32;
            }
        }
        (void)data_bury_session_mode_queue_push(mode_or_program_id, duration_sec);
    }

    used_mode_count = s_event_rt.mode_queue_count;
    if (used_mode_count == 0u) {
        used_mode_count = 1u;
    }

    memset(buf, 0, DATA_BURY_EVT_PUMP_SESSION_END_MAX_LEN);
    evt->device_side = ctx->data_bury_get_device_side();
    evt->use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt->evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt->pump_session_id = session_id;
    evt->final_mode_type = bury_mode;
    evt->final_suction_level = 0u;
    evt->final_battery_level = data_bury_event_get_battery_percent();
    evt->end_reason = end_reason;
    evt->used_mode_count = used_mode_count;
    evt->final_suction_level_desktop = data_bury_event_get_suction_level_desktop(pump);

    program_durations = (data_bury_evt_program_duration_t *)(buf + DATA_BURY_EVT_PUMP_SESSION_END_PROGRAM_OFF);
    if (s_event_rt.mode_queue_count > 0u) {
        for (i = 0; i < s_event_rt.mode_queue_count; i++) {
            program_durations[i] = s_event_rt.mode_queue[i];
        }
    } else {
        program_durations[0].mode_or_program_id = mode_or_program_id;
        program_durations[0].duration_sec = (uint16_t)(result.work_valid_time / 1000u);
    }

    payload_len = DATA_BURY_EVT_PUMP_SESSION_END_PAYLOAD_LEN(used_mode_count);
    s_event_rt.pending_pump_session_start.pump_session_id = session_id;
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START,
                                    &s_event_rt.pending_pump_session_start,
                                    (uint16_t)sizeof(s_event_rt.pending_pump_session_start), 0u);
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END,
                                    buf, payload_len, 100u);

    s_event_rt.pump_session_start_pending = 0u;
    s_event_rt.pending_pump_session_start_ms = 0u;
    memset(&s_event_rt.pending_pump_session_start, 0, sizeof(s_event_rt.pending_pump_session_start));
    data_bury_session_mode_queue_reset();
    if (ctx->data_bury_pump_session_end) {
        ctx->data_bury_pump_session_end();
    }
    LOG_BURY("pump session end committed id=%u duration=%us modes=%u",
             (unsigned)session_id, (unsigned)(session_duration_ms / 1000u),
             (unsigned)used_mode_count);
    u_free(buf);
}

static void data_bury_event_publish_charge_start(data_bury_manager_context_t *ctx)
{
    data_bury_evt_bat_charge_start_t evt = {0};
    data_bury_battery_charge_metrics_t metrics = data_bury_event_sample_battery_charge_metrics();

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.start_battery_percent = metrics.percent;
    evt.start_battery_voltage = metrics.voltage_mv;
    evt.start_battery_current = metrics.current_ma;
    evt.start_battery_temp = metrics.temp_c;
    LOG_BURY("charge start publish: ts=%u percent=%u voltage=%u current=%d temp=%d",
             (unsigned)evt.evt_timestamp, (unsigned)evt.start_battery_percent,
             (unsigned)evt.start_battery_voltage, (int)evt.start_battery_current,
             (int)evt.start_battery_temp);
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START,
                                    &evt, (uint16_t)sizeof(evt), 0u);
}

static void data_bury_event_publish_charge_end(data_bury_manager_context_t *ctx, int8_t end_reason)
{
    data_bury_evt_bat_charge_end_t evt = {0};
    data_bury_battery_charge_metrics_t metrics = data_bury_event_sample_battery_charge_metrics();

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.end_battery_percent = metrics.percent;
    evt.end_battery_voltage = metrics.voltage_mv;
    evt.end_battery_current = metrics.current_ma;
    evt.end_battery_temp = metrics.temp_c;
    evt.end_reason = end_reason;
    LOG_BURY("charge end publish: ts=%u percent=%u voltage=%u current=%d temp=%d reason=%d",
             (unsigned)evt.evt_timestamp, (unsigned)evt.end_battery_percent,
             (unsigned)evt.end_battery_voltage, (int)evt.end_battery_current,
             (int)evt.end_battery_temp, (int)evt.end_reason);
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_END,
                                    &evt, (uint16_t)sizeof(evt), 0u);
}

static void data_bury_event_publish_firmware_update_start(data_bury_manager_context_t *ctx, uint8_t is_success, uint16_t update_err_code)
{
    data_bury_evt_firmware_update_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.update_from = (uint16_t)(((ROOT_FIRMWARE_VERSION / 100) << 8) | (ROOT_FIRMWARE_VERSION % 100));
    evt.update_result = is_success ? DATA_BURY_FIRMWARE_UPDATE_RESULT_SUCCESS : DATA_BURY_FIRMWARE_UPDATE_RESULT_FAILURE;
    evt.update_err = update_err_code;
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_FIRMWARE_UPDATE,
                                    &evt, (uint16_t)sizeof(evt), 0u);
}

static void data_bury_event_flush_pending_if_connected(data_bury_manager_context_t *ctx)
{
    ble_manager_context_t *ble = ctx->priv.ble_ctx;

    if (!ble) {
        ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
        ctx->priv.ble_ctx = ble;
    }
    if (!ble || !ble->ble_is_connected() || !ctx->data_bury_sys_msg_put) {
        return;
    }
    LOG_BURY("data bury event flush pending start");
    (void)ctx->data_bury_sys_msg_put(DATA_BURY_EVT_FLUSH_PENDING, 200u, NULL);
}

int data_bury_event_process_ble_connected(data_bury_manager_context_t *ctx,
                                                 const data_bury_trigger_args_t *args)
{
    if (!args) {
        return -1;
    }
    data_bury_event_bt_session_cache_save(ctx, args);
    return 0;
}

int data_bury_event_process_ble_disconnect(data_bury_manager_context_t *ctx,
                                                  const data_bury_trigger_args_t *args)
{
    uint8_t hci_reason = 0x13u;

    if (!args) {
        return -1;
    }
    if (args->arg1 >= 0) {
        hci_reason = (uint8_t)args->arg1;
    }
    return data_bury_event_publish_bt_session_end(ctx, hci_reason);
}

int data_bury_event_process_pump_session_start(data_bury_manager_context_t *ctx,
                                                      const data_bury_trigger_args_t *args)
{
    (void)args;
    data_bury_event_buffer_pump_session_start(ctx);
    return 0;
}

int data_bury_event_process_pump_mode_switch(data_bury_manager_context_t *ctx,
                                                    const data_bury_trigger_args_t *args)
{
    uint32_t adjust_from;
    uint32_t adjust_to;
    uint16_t duration_sec = 0u;
    pump_mode_steps_info_t steps_info;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_INT(storage_ctx);
    (void)ctx;
    if (!args || !args->val) {
        return -1;
    }
    adjust_from = args->arg1;
    adjust_to = args->arg2;
    duration_sec = *(const uint16_t *)args->val;

    if (adjust_from <= BREAST_PUMP_MODE_MAX) {
        adjust_from = (uint32_t)data_bury_convert_pump_mode((uint8_t)adjust_from);
        if ((uint8_t)args->arg1 == BREAST_PUMP_CUSTOM_MODE) {
            storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &steps_info);
            adjust_from = steps_info.id.dat32;
        }
    }

    if (adjust_to <= BREAST_PUMP_MODE_MAX) {
        adjust_to = (uint32_t)data_bury_convert_pump_mode((uint8_t)adjust_to);
        if ((uint8_t)args->arg2 == BREAST_PUMP_CUSTOM_MODE) {
            storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &steps_info);
            adjust_to = steps_info.id.dat32;
        }
    }

    data_bury_event_record_pump_mode_switch(adjust_from, adjust_to, duration_sec);
    return 0;
}

int data_bury_event_process_pump_manual_adjust(data_bury_manager_context_t *ctx,
                                                      const data_bury_trigger_args_t *args)
{
    const data_bury_trigger_manual_adjust_t *adj;

    if (!ctx || !args || !args->val) {
        return -1;
    }
    adj = (const data_bury_trigger_manual_adjust_t *)args->val;
    data_bury_event_publish_pump_manual_adjust(ctx, adj);
    return 0;
}

int data_bury_event_process_pump_user_event(data_bury_manager_context_t *ctx,
                                            const data_bury_trigger_args_t *args)
{
    const data_bury_trigger_user_evt_t *user_evt;

    if (!ctx || !args || !args->val) {
        return -1;
    }
    user_evt = (const data_bury_trigger_user_evt_t *)args->val;
    data_bury_event_publish_pump_user_evt(ctx, user_evt->evt_source, user_evt->evt_type);
    return 0;
}

int data_bury_event_process_pump_session_end(data_bury_manager_context_t *ctx,
                                                    const data_bury_trigger_args_t *args)
{
    uint8_t end_reason = DATA_BURY_END_REASON_MANUAL;

    if (args && args->arg1 >= 0) {
        end_reason = (uint8_t)args->arg1;
    }
    data_bury_event_commit_pump_session_end(ctx, end_reason);
    return 0;
}

int data_bury_event_process_power_on(data_bury_manager_context_t *ctx,
                                            const data_bury_trigger_args_t *args)
{
    (void)args;
    if (ctx->data_bury_reset_pump_session_state) {
        ctx->data_bury_reset_pump_session_state();
    }
    data_bury_pump_session_pending_discard(ctx);
    data_bury_event_publish_pump_startup(ctx);
    return 0;
}

int data_bury_event_process_power_off(data_bury_manager_context_t *ctx,
                                             const data_bury_trigger_args_t *args)
{
    uint8_t reason = (uint8_t)args->arg1;

    if (reason == 0u) {
        reason = DATA_BURY_SHUTDOWN_USER_MANUAL;
    }
    data_bury_pump_session_pending_discard(ctx);
    data_bury_event_cache_pump_shutdown(ctx, reason);
    if (ctx->priv.ble_ctx && ctx->priv.ble_ctx->ble_is_connected
        && ctx->priv.ble_ctx->ble_is_connected()) {
        ///关机动作，先不加入ble断开事件
        //(void)data_bury_event_cache_bt_session_end(ctx, 0x13u);
    }
    return 0;
}

int data_bury_event_process_ota_start(data_bury_manager_context_t *ctx,
                                             const data_bury_trigger_args_t *args)
{
    uint8_t is_success = (uint8_t)args->arg1;
    uint16_t update_err_code = (uint16_t)args->arg2;
    data_bury_event_publish_firmware_update_start(ctx, is_success, update_err_code);
    return 0;
}

int data_bury_event_process_bat_charge_start(data_bury_manager_context_t *ctx,
                                                    const data_bury_trigger_args_t *args)
{
    (void)args;
    data_bury_event_publish_charge_start(ctx);
    return 0;
}

int data_bury_event_process_bat_charge_end(data_bury_manager_context_t *ctx,
                                                  const data_bury_trigger_args_t *args)
{
    int8_t end_reason = DATA_BURY_BAT_CHARGE_END_REASON_UNPLUG;

    if (args && args->arg1 >= 0) {
        end_reason = (int8_t)args->arg1;
    }
    data_bury_event_publish_charge_end(ctx, end_reason);
    return 0;
}

int data_bury_event_process_wear_detection(data_bury_manager_context_t *ctx,
                                           const data_bury_trigger_args_t *args)
{
    const data_bury_trigger_wear_detection_t *wear;

    if (!ctx || !args || !args->val) {
        return -1;
    }
    wear = (const data_bury_trigger_wear_detection_t *)args->val;
    data_bury_event_publish_wear_detection(ctx, wear->detection_type, wear->detection_data);
    return 0;
}

int data_bury_event_process_exception(data_bury_manager_context_t *ctx,
                                      const data_bury_trigger_args_t *args)
{
    const data_bury_trigger_exception_t *exc;

    if (!ctx || !args || !args->val) {
        return -1;
    }
    exc = (const data_bury_trigger_exception_t *)args->val;
    if (exc->error_code == 0u) {
        return -1;
    }
    data_bury_event_publish_exception(ctx, exc->error_code, exc->error_data);
    return 0;
}

void data_bury_event_handler_init_params(void)
{
    memset(&s_event_rt, 0, sizeof(s_event_rt));
    data_bury_session_mode_queue_reset();
}

int data_bury_event_handler_on_ble_notify_broadcast(data_bury_manager_context_t *ctx, int arg1, int arg2)
{
    if (arg1 != BLE_BROADCAST_NOTIFY_ENABLED) {
        return 0;
    }
    if (arg2 != BLE_SRV_ID_DATA_BURY) {
        LOG_BLE("data bury event handler on ble notify broadcast, service_id: %d", arg2);
        return 0;
    }
    data_bury_event_flush_pending_if_connected(ctx);
    return 0;
}
