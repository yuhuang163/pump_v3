/*
 * overlay 数据埋点事件处理：外部 trigger -> 埋点 payload 采集与上报
 */

#include "root_common_api.h"
#include "hal_battery.h"
#include "hal_ble.h"
#include "data_bury_persist_defs.h"
#include "data_bury_event_handler.h"

typedef struct {
    uint8_t last_charge_status;
    uint8_t charge_tracked;
    uint8_t session_start_mode;
    uint8_t session_start_level;
    uint8_t bt_session_cached;
    uint8_t bt_remote_initiated;
    uint32_t bt_connect_timestamp_utc;
    uint16_t bt_connection_setup_duration;
    int8_t bt_signal_strength_rssi;
} data_bury_event_runtime_t;

static data_bury_event_runtime_t s_event_rt;

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

static uint8_t data_bury_event_get_suction_level(breast_pump_manager_context_t *pump)
{
    pump_setting_t setting = {0};

    if (!pump) {
        return 0u;
    }
    READ_PUMP_SETTING_INFO(pump, 0, setting);
    return setting.level[0] > setting.level[1] ? setting.level[0] : setting.level[1];
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

static void data_bury_event_publish_pump_session_start(data_bury_manager_context_t *ctx)
{
    breast_pump_manager_context_t *pump =
        (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    data_bury_evt_pump_session_start_t evt = {0};
    uint8_t mode = 0;
    uint16_t session_id = 0;

    if (!pump) {
        return;
    }
    READ_PUMP_WORK_MODE(pump, mode);
    if (ctx->data_bury_pump_session_begin) {
        ctx->data_bury_pump_session_begin();
    }
    if (ctx->data_bury_alloc_pump_session_id) {
        session_id = ctx->data_bury_alloc_pump_session_id();
    }
    s_event_rt.session_start_mode = mode;
    s_event_rt.session_start_level = data_bury_event_get_suction_level(pump);

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.pump_session_id = session_id;
    evt.initial_mode_type = mode;
    evt.initial_suction_level = s_event_rt.session_start_level;
    evt.initial_battery_level = data_bury_event_get_battery_percent();
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START,
                                    &evt, (uint16_t)sizeof(evt), 0u);
}

static void data_bury_event_publish_pump_session_end(data_bury_manager_context_t *ctx, uint8_t end_reason)
{
    breast_pump_manager_context_t *pump =
        (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    uint8_t buf[DATA_BURY_EVT_PUMP_SESSION_END_FIXED_LEN + 4u];
    data_bury_evt_pump_session_end_fixed_t *evt =
        (data_bury_evt_pump_session_end_fixed_t *)buf;
    pump_result_info_t result = {0};
    uint8_t mode = 0;
    uint32_t *mode_durations;
    uint16_t session_id = 0;
    uint16_t payload_len;

    if (!pump) {
        return;
    }
    if (ctx->data_bury_get_current_pump_session_id) {
        session_id = ctx->data_bury_get_current_pump_session_id();
    }
    if (session_id == 0u) {
        return;
    }

    READ_PUMP_WORK_MODE(pump, mode);
    READ_PUMP_RESULT_INFO(pump, BREAST_PUMP_RESULT_PUMPING, &result);

    memset(buf, 0, sizeof(buf));
    evt->device_side = ctx->data_bury_get_device_side();
    evt->use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt->evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt->pump_session_id = session_id;
    evt->final_mode_type = mode;
    evt->final_suction_level = data_bury_event_get_suction_level(pump);
    evt->final_battery_level = data_bury_event_get_battery_percent();
    evt->end_reason = end_reason;
    evt->used_mode_count = 1u;
    evt->pump_side = ctx->data_bury_get_pump_side();
    mode_durations = (uint32_t *)(buf + DATA_BURY_EVT_PUMP_SESSION_END_MODE_OFF);
    mode_durations[0] = result.work_valid_time / 1000u;

    payload_len = (uint16_t)(DATA_BURY_EVT_PUMP_SESSION_END_FIXED_LEN + sizeof(uint32_t));
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END,
                                    buf, payload_len, 0u);
    if (ctx->data_bury_pump_session_end) {
        ctx->data_bury_pump_session_end();
    }
}

static void data_bury_event_publish_charge_start(data_bury_manager_context_t *ctx)
{
    data_bury_evt_bat_charge_start_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.start_battery_percent = data_bury_event_get_battery_percent();
    (void)data_bury_event_publish(ctx, SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START,
                                    &evt, (uint16_t)sizeof(evt), 0u);
}

static void data_bury_event_publish_charge_end(data_bury_manager_context_t *ctx, int8_t end_reason)
{
    data_bury_evt_bat_charge_end_t evt = {0};

    evt.device_side = ctx->data_bury_get_device_side();
    evt.use_session_id = DATA_BURY_EVT_USE_SESSION_PLACEHOLDER;
    evt.evt_timestamp = DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER;
    evt.end_battery_percent = data_bury_event_get_battery_percent();
    evt.end_reason = end_reason;
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
    data_bury_event_publish_pump_session_start(ctx);
    return 0;
}

int data_bury_event_process_pump_session_end(data_bury_manager_context_t *ctx,
                                                    const data_bury_trigger_args_t *args)
{
    data_bury_event_publish_pump_session_end(ctx, (uint8_t)args->arg1);
    return 0;
}

int data_bury_event_process_power_on(data_bury_manager_context_t *ctx,
                                            const data_bury_trigger_args_t *args)
{
    (void)args;
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

int data_bury_event_process_bat_charge_status(data_bury_manager_context_t *ctx,
                                                   const data_bury_trigger_args_t *args)
{
    uint8_t now_status = (uint8_t)args->arg1;
    uint8_t prev = (uint8_t)args->arg2;
    uint8_t now_charging;
    uint8_t prev_charging;

    if (!s_event_rt.charge_tracked) {
        s_event_rt.last_charge_status = now_status;
        s_event_rt.charge_tracked = 1u;
        return 0;
    }
    if (prev == now_status) {
        return 0;
    }

    now_charging = BATTERY_IS_CHARGING(now_status) ? 1u : 0u;
    prev_charging = BATTERY_IS_CHARGING(prev) ? 1u : 0u;
    if (!prev_charging && now_charging) {
        data_bury_event_publish_charge_start(ctx);
    } else if (prev_charging && !now_charging) {
        int8_t end_reason = (prev == CHARGE_STATE_FULL)
            ? DATA_BURY_BAT_CHARGE_END_REASON_FULL
            : DATA_BURY_BAT_CHARGE_END_REASON_UNPLUG;
        data_bury_event_publish_charge_end(ctx, end_reason);
    }
    s_event_rt.last_charge_status = now_status;
    return 0;
}

void data_bury_event_handler_init_params(void)
{
    memset(&s_event_rt, 0, sizeof(s_event_rt));
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
