/*
 * overlay 数据埋点事件处理：外部 trigger -> 埋点 payload 采集与上报
 */

#include "root_common_api.h"
#include "data_bury_persist_defs.h"
#include "data_bury_event_handler.h"

typedef struct {
    uint8_t session_start_mode;
    uint8_t session_start_level;
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

static uint8_t data_bury_event_get_battery_percent(void)
{
    battery_manager_context_t *bat = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);

    if (bat && bat->get_battery_percent) {
        return bat->get_battery_percent();
    }
    return 0u;
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

static uint8_t data_bury_event_get_suction_level(breast_pump_manager_context_t *pump)
{
    pump_setting_t setting = {0};

    if (!pump) {
        return 0u;
    }
    READ_PUMP_SETTING_INFO(pump, 0, setting);
    return DATA_BURY_PACK_SUCTION_LEVEL(setting.level[0], setting.level[1]);
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

static void data_bury_event_publish_pump_shutdown(data_bury_manager_context_t *ctx, uint8_t reason)
{
    data_bury_evt_pump_shutdown_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.battery_level_shutdown = data_bury_event_get_battery_percent();
    evt.shutdown_reason = reason;
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN,
                                    &evt, (uint16_t)sizeof(evt), 0u);
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

static int data_bury_event_publish_bt_session_end(data_bury_manager_context_t *ctx, uint8_t hci_reason)
{
    data_bury_evt_bt_connect_t evt = {0};
    uint8_t app_reason;

    if (!s_event_rt.bt_session_cached || !s_event_rt.bt_remote_initiated) {
        return 0;
    }
    app_reason = data_bury_event_hci_to_app_disconnect_reason(hci_reason);
    if (app_reason == DATA_BURY_BT_DISCONNECT_REASON_SKIP) {
        LOG_BURY("bt session end skip: hci_reason=0x%02x", (unsigned)hci_reason);
        data_bury_event_bt_session_cache_clear();
        return 0;
    }

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = s_event_rt.bt_connect_timestamp_utc;
    evt.connect_result = DATA_BURY_BT_CONNECT_RESULT_SESSION_END;
    evt.connection_setup_duration = s_event_rt.bt_connection_setup_duration;
    evt.bt_signal_strength_rssi = s_event_rt.bt_signal_strength_rssi;
    evt.disconnect_timestamp = platform_get_utc_time();
    evt.disconnect_reason = app_reason;
    LOG_BURY("bt session end publish: connect_ts=%u setup_ms=%u rssi=%d reason=0x%02x",
             (unsigned)evt.evt_timestamp,
             (unsigned)evt.connection_setup_duration,
             (int)evt.bt_signal_strength_rssi,
             (unsigned)evt.disconnect_reason);
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
    if (ctx->data_bury_pump_session_begin) {
        ctx->data_bury_pump_session_begin();
    }
    s_event_rt.session_start_mode = (uint8_t)mode;
    s_event_rt.session_start_level = data_bury_event_get_suction_level(pump);

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = platform_get_utc_time();
    evt.pump_session_id = 0u;
    evt.initial_mode_type = (uint8_t)mode;
    evt.initial_suction_level = s_event_rt.session_start_level;
    evt.initial_battery_level = data_bury_event_get_battery_percent();

    s_event_rt.pending_pump_session_start = evt;
    s_event_rt.pending_pump_session_start_ms = get_timestamp_ms(0);
    s_event_rt.pump_session_start_pending = 1u;
    data_bury_session_mode_queue_begin();
    LOG_BURY("pump session start buffered ts=%u (commit on valid end >= %us)",
             (unsigned)evt.evt_timestamp, (unsigned)DATA_BURY_PUMP_SESSION_MIN_DURATION_SEC);
}

static void data_bury_event_record_pump_mode_switch(uint8_t adjust_from, uint8_t adjust_to,
                                                    uint16_t duration_sec)
{
    if (!s_event_rt.mode_queue_tracking || adjust_from == adjust_to) {
        return;
    }

    (void)data_bury_session_mode_queue_push((uint32_t)adjust_from, duration_sec);
    LOG_BURY("mode switch queue: from=%u to=%u dur=%u count=%u",
             (unsigned)adjust_from, (unsigned)adjust_to, (unsigned)duration_sec,
             (unsigned)s_event_rt.mode_queue_count);
}

static void data_bury_event_publish_pump_manual_adjust(data_bury_manager_context_t *ctx,
                                                       uint8_t evt_source, uint8_t evt_type,
                                                       uint8_t adjust_from, uint8_t adjust_to)
{
    data_bury_evt_pump_manual_adjust_t evt = {0};
    uint16_t session_id = 0u;

    if (adjust_from == adjust_to) {
        return;
    }

    if (ctx->data_bury_get_current_pump_session_id) {
        session_id = ctx->data_bury_get_current_pump_session_id();
    }

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.pump_session_id = session_id;
    evt.evt_source = evt_source;
    evt.evt_type = evt_type;
    evt.adjust_from = adjust_from;
    evt.adjust_to = adjust_to;
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST,
                                    &evt, (uint16_t)sizeof(evt), 0u);
    LOG_BURY("manual adjust publish: sid=%u src=%u type=%u from=%u to=%u",
             (unsigned)session_id, (unsigned)evt_source, (unsigned)evt_type,
             (unsigned)adjust_from, (unsigned)adjust_to);
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

static void data_bury_event_commit_pump_session_end(data_bury_manager_context_t *ctx, uint8_t end_reason)
{
    breast_pump_manager_context_t *pump =
        (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    uint8_t buf[DATA_BURY_EVT_PUMP_SESSION_END_MAX_LEN];
    data_bury_evt_pump_session_end_fixed_t *evt =
        (data_bury_evt_pump_session_end_fixed_t *)buf;
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

    if (ctx->data_bury_alloc_pump_session_id_on_commit) {
        session_id = ctx->data_bury_alloc_pump_session_id_on_commit();
    }
    if (session_id == 0u && ctx->data_bury_get_current_pump_session_id) {
        session_id = ctx->data_bury_get_current_pump_session_id();
    }

    READ_PUMP_WORK_MODE(pump, mode);
    (void)pump->get_pump_composite_state(BREAST_PUMP_RESULT_INFO, BREAST_PUMP_RESULT_PUMPING, 0,
                                          &result, (int)sizeof(result));
    duration_sec = (uint16_t)(result.current_mode_work_time / 1000u);
    if (s_event_rt.mode_queue_tracking) {
        (void)data_bury_session_mode_queue_push((uint32_t)mode, duration_sec);
    }

    used_mode_count = s_event_rt.mode_queue_count;
    if (used_mode_count == 0u) {
        used_mode_count = 1u;
    }

    memset(buf, 0, sizeof(buf));
    evt->device_side = ctx->data_bury_get_device_side();
    evt->use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt->evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt->pump_session_id = session_id;
    evt->final_mode_type = (uint8_t)mode;
    evt->final_suction_level = data_bury_event_get_suction_level(pump);
    evt->final_battery_level = data_bury_event_get_battery_percent();
    evt->end_reason = end_reason;
    evt->used_mode_count = used_mode_count;

    program_durations = (data_bury_evt_program_duration_t *)(buf + DATA_BURY_EVT_PUMP_SESSION_END_PROGRAM_OFF);
    if (s_event_rt.mode_queue_count > 0u) {
        for (i = 0; i < s_event_rt.mode_queue_count; i++) {
            program_durations[i] = s_event_rt.mode_queue[i];
        }
    } else {
        program_durations[0].mode_or_program_id = (uint32_t)mode;
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
             (unsigned)session_id, (unsigned)session_duration_sec, (unsigned)used_mode_count);
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
    evt.update_from = (uint16_t)ROOT_FIRMWARE_VERSION;
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
    uint8_t adjust_from;
    uint8_t adjust_to;
    uint16_t duration_sec = 0u;

    (void)ctx;
    if (!args || !args->val) {
        return -1;
    }
    adjust_from = (uint8_t)args->arg1;
    adjust_to = (uint8_t)args->arg2;
    duration_sec = *(const uint16_t *)args->val;
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
    data_bury_event_publish_pump_manual_adjust(ctx, adj->evt_source, adj->evt_type,
                                               adj->adjust_from, adj->adjust_to);
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
    data_bury_event_publish_pump_shutdown(ctx, reason);
    if (ctx->priv.ble_ctx && ctx->priv.ble_ctx->ble_is_connected
        && ctx->priv.ble_ctx->ble_is_connected()) {
        (void)data_bury_event_publish_bt_session_end(ctx, 0x13u);
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

void data_bury_event_handler_init_params(void)
{
    memset(&s_event_rt, 0, sizeof(s_event_rt));
    data_bury_session_mode_queue_reset();
}

int data_bury_event_handler_on_ble_notify_broadcast(data_bury_manager_context_t *ctx, int arg1, int arg2)
{
    (void)arg2;
    if (arg1 != BLE_BROADCAST_NOTIFY_ENABLED) {
        return 0;
    }
    data_bury_event_flush_pending_if_connected(ctx);
    return 0;
}
