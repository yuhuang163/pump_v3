/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_server_client.h"
#include "master_server_client.h"
#include "app_protocol_handler.h"
#include "data_bury_protocol_server_client.h"


/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    uint8_t force_update_flag;
    breast_pump_result_t pump;
    battery_state_info_t battery;
    led_state_info_t light;
} servers_system_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static servers_system_info_t *servers_sys_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int on_servers_manager_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
   return on_master_app_old_protocol_receiver_route(id, context, arg1, arg2, user_data);
}

static void on_airplane_mode_state_report(servers_manager_context_t *ctx, int state)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args_temp = {
        .arg1 = OLD_PUBLISH_ABNORMAL_TRAVEL_LOCK,
        .arg2 = state > 0 ? OLD_PUBLISH_ABNORMAL_STATE_ACTIVE : OLD_PUBLISH_ABNORMAL_STATE_CLEARED,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
}

static void on_ble_state_report(servers_manager_context_t *ctx, int state)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args = {
        .arg1 = state,
        .arg2 = 0,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_DISCON, &args);
}

static void on_battery_low_power_shutdown_report(servers_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args_temp = {
        .arg1 = OLD_PUBLISH_ABNORMAL_LOW_BAT_SHUTDOWN_SOON,
        .arg2 = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
}

static void on_charge_error_report(servers_manager_context_t *ctx, int state)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args_temp = {
        .arg1 = OLD_PUBLISH_ABNORMAL_ADAPTER_ERROR,
        .arg2 =  state ? OLD_PUBLISH_ABNORMAL_STATE_ACTIVE : OLD_PUBLISH_ABNORMAL_STATE_CLEARED,
        .val = NULL,
    };  
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
}

static void on_charge_status_report(servers_manager_context_t *ctx, int state)
{

}

static void on_pump_status_report(servers_manager_context_t *ctx, int state)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args = {
        .arg1 = state,
        .arg2 = 0,
        .val = NULL,
    };
    LOG_SRV("on_pump_status_report: %d", state);
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);
}

static void on_pump_custom_mode_report(servers_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args = {
        .arg1 = 0,
        .arg2 = 0,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_CUSTOM_MODE_ID, &args);
}

static void on_motor_taste_start_report(servers_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_TASTE_START_REPORT,
        .arg2 = 0,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_TASTE_REPORT, &args);
}

static void on_motor_taste_stop_report(servers_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_TASTE_STOP_REPORT,
        .arg2 = 0,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_TASTE_REPORT, &args);
}

static void on_motor_pump_step_all_param_change_report(servers_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_PUMP_STEP_ALL_PARAM_CHANGE,
        .arg2 = 0,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);
}

static void on_light_status_report(servers_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_LIGHT_STATUS_CHANGE,
        .arg2 = 0,
        .val = NULL,
    };
    app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_LIGHT_STATUS, &args);
}

static void on_motor_status_evt_cb(void *user_data, motor_status_evt_type_e evt_type)
{
    servers_manager_context_t *ctx = (servers_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(ctx && ctx->dispatch_servers_event_wrapper);
    switch (evt_type) {
        case MOTOR_STATUS_EVT_TASTE_START:
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_MOTOR_TASTE_START_REPORT);
            break;
        case MOTOR_STATUS_EVT_TASTE_STOP:
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_MOTOR_TASTE_STOP_REPORT);
            break;
        case MOTOR_STATUS_EVT_PUMP_STEP_ALL_PARAM_CHANGE:
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_PUMP_STEP_CHANGE);
            break;
        default:
            break;
    }
}

static int servers_work_thread_route(servers_manager_context_t *ctx, servers_publish_data_id_e msg_id)
{
    switch (msg_id) {
        case SERVERS_PUBLISH_DATA_AIRPLANE_MODE_ENTER_REPORT:
            on_airplane_mode_state_report(ctx, 1);
            break;
        case SERVERS_PUBLISH_DATA_AIRPLANE_MODE_EXIT_REPORT:
            on_airplane_mode_state_report(ctx, 0);
            break;
        case SERVERS_PUBLISH_DATA_BLE_CONNECT_REPORT:
            on_ble_state_report(ctx, 1);
            break;
        case SERVERS_PUBLISH_DATA_BLE_DISCONNECT_REPORT:
            on_ble_state_report(ctx, 0);
            break;
        case SERVERS_PUBLISH_DATA_MOTOR_TASTE_START_REPORT:
            on_motor_taste_start_report(ctx);
            break;
        case SERVERS_PUBLISH_DATA_MOTOR_TASTE_STOP_REPORT:
            on_motor_taste_stop_report(ctx);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_SYSTEM_STATUS_RESET:
            on_pump_status_report(ctx, OLD_PUBLISH_DATA_SYSTEM_STATUS_RESET);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_SYSTEM_STATUS_CHANGE:
            on_pump_status_report(ctx, OLD_PUBLISH_DATA_SYSTEM_STATUS_CHANGE);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_MODE_CHANGE:
            on_pump_status_report(ctx, OLD_PUBLISH_DATA_PUMP_MODE_CHANGE);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_CONTROL_TYPE_CHANGE:
            on_pump_status_report(ctx, OLD_PUBLISH_DATA_PUMP_CONTROL_TYPE_CHANGE);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_STEP_CHANGE:
            on_motor_pump_step_all_param_change_report(ctx);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_LEVEL_CHANGE:
            on_pump_status_report(ctx, OLD_PUBLISH_DATA_PUMP_LEVEL_CHANGE);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_SPEED_CHANGE:
            on_pump_status_report(ctx, OLD_PUBLISH_DATA_PUMP_SPEED_CHANGE);
            break;
        case SERVERS_PUBLISH_DATA_PUMP_CUSTOM_MODE_REPORT:
            on_pump_custom_mode_report(ctx);
            break;
        case SERVERS_PUBLISH_DATA_BATTERY_LOW_POWER_SHUTDOWN_REPORT:
            on_battery_low_power_shutdown_report(ctx);
            break;
        case SERVERS_PUBLISH_DATA_BATTERY_CHARGE_ERROR_REPORT:
            on_charge_error_report(ctx, servers_sys_info->battery.charge_status == CHARGE_STATE_ERROR);
            break;
        case SERVERS_PUBLISH_DATA_BATTERY_CHARGE_NORMAL_REPORT:
            on_charge_status_report(ctx, servers_sys_info->battery.charge_status);
            break;
        case SERVERS_PUBLISH_DATA_LIGHT_STATUS_CHANGE:
            on_light_status_report(ctx);
            break;
        default:
            break;
    }
    return 0;
}

static void on_breast_pump_result_cb(void *user_data, breast_pump_result_t *result)
{
    static uint32_t last_report_time = 0;
    servers_manager_context_t *ctx = (servers_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(ctx && servers_sys_info && result);
    breast_pump_result_t *pump = &servers_sys_info->pump;
    uint8_t is_need_publish = false;
    if (pump->status != result->status) {
        pump->status = result->status;
        LOG_SRV("on_breast_pump_result_cb: %d", result->status);
        if (result->status == BREAST_PUMP_FINISH) {
            LOG_SRV("FINISH");
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_PUMP_SYSTEM_STATUS_RESET);
        }
        else if (result->status == BREAST_PUMP_RESET) {
            LOG_SRV("RESET");
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_PUMP_SYSTEM_STATUS_RESET);
            /// 重置pump状态到IDLE
            breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
            EXCEPTION_CHECK_VOID(pump_ctx);
            WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_IDLE, 1);
        }
        is_need_publish = true;
        goto TO_PUBLISH;
    }

    if (PUMP_IS_GOING(result->status)) {
        if (pump->working_mode != result->working_mode) {
            pump->working_mode = result->working_mode;
            pump->setting_mode = result->setting_mode;
            if (pump->setting_mode == BREAST_PUMP_CUSTOM_MODE) {
                ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_PUMP_CUSTOM_MODE_REPORT);
            }
            is_need_publish = true;
            goto TO_PUBLISH;
        }
    } else {
        if (pump->setting_mode != result->setting_mode) {
            pump->setting_mode = result->setting_mode;
            if (pump->setting_mode == BREAST_PUMP_CUSTOM_MODE) {
                ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_PUMP_CUSTOM_MODE_REPORT);
            }
            is_need_publish = true;
            goto TO_PUBLISH;
        }
    }

    if (get_timestamp_ms(last_report_time) < 300) {
        return;
    }

    if (pump->control_type != result->control_type) {
        pump->control_type = result->control_type;
        is_need_publish = true;
    }

    if (pump->setting.speed != result->setting.speed) {
        pump->setting.speed = result->setting.speed;
        is_need_publish = true;
    }

    if (memcmp(&pump->setting.level, &result->setting.level, BREAST_PUMP_NUM ) != 0) {
        memcpy(&pump->setting.level, &result->setting.level, BREAST_PUMP_NUM);
        is_need_publish = true;
    }

TO_PUBLISH:        
    if (is_need_publish) {
        ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_PUMP_SYSTEM_STATUS_CHANGE);
    }
    last_report_time = get_timestamp();
}

static void on_servers_led_state_cb(void *user_data, led_state_info_t *info)
{
    servers_manager_context_t *ctx = (servers_manager_context_t *)user_data;
    led_state_info_t *light = NULL;

    EXCEPTION_CHECK_VOID(ctx && ctx->dispatch_servers_event_wrapper);
    EXCEPTION_CHECK_VOID(servers_sys_info && info);
    light = &servers_sys_info->light;
    if (memcmp(light, info, sizeof(led_state_info_t)) == 0) {
        //LOG_SRV("led state is the same");
        return;
    }
    LOG_SRV("led state change: evt=%d onoff=%d level=%d adaptive=%d/%d",
            info->evt_id, info->night_light_onoff, info->night_light_level,
            info->night_light_adaptive_switch, info->night_light_adaptive_dimming);
    memcpy(light, info, sizeof(led_state_info_t));
    ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_LIGHT_STATUS_CHANGE);
}

static void on_servers_battery_state_cb(void *user_data, battery_state_info_t *info)
{
    servers_manager_context_t *ctx = (servers_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(ctx && ctx->dispatch_servers_event_wrapper);
    EXCEPTION_CHECK_VOID(servers_sys_info && info);
    battery_state_info_t *battery = &servers_sys_info->battery;
    if (info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        info->battery_state == BATTERY_STATE_EMPTY) {
        if (battery->battery_state != info->battery_state) {
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_BATTERY_LOW_POWER_SHUTDOWN_REPORT);
            battery->battery_state = info->battery_state;
        }
        return;
    }

    if (battery->charge_status != info->charge_status) {
        if (info->charge_status == CHARGE_STATE_ERROR) {
            battery->charge_status = info->charge_status;
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_BATTERY_CHARGE_ERROR_REPORT);
        } else if (battery->charge_status == CHARGE_STATE_ERROR) {
            battery->charge_status = info->charge_status;
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_BATTERY_CHARGE_ERROR_REPORT);
        } else {
            battery->charge_status = info->charge_status;
            ctx->dispatch_servers_event_wrapper(ctx, SERVERS_PUBLISH_DATA_BATTERY_CHARGE_NORMAL_REPORT);
        }
    }
}

static int on_post_servers_init_route(struct servers_manager_context *ctx)
{
    factory_server_client_init();
    master_server_client_init();
    data_bury_protocol_server_client_init();
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_INT(pump_ctx);
    ADD_BREAST_PUMP_RESULT_CB(pump_ctx, ctx, on_breast_pump_result_cb);
    battery_manager_context_t *battery_ctx = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    ADD_BATTERY_STATE_CB(battery, ctx, on_servers_battery_state_cb);
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    ADD_MOTOR_STATUS_EVT_CB(motor_ctx, ctx, on_motor_status_evt_cb);
    servers_sys_info = u_malloz(sizeof(servers_system_info_t));
    led_manager_context_t *led_ctx = (led_manager_context_t *)find_module(ID_MODULE_LED);
    ADD_LED_STATE_CB(led_ctx, ctx, on_servers_led_state_cb);

    return 0;
}

static int app_server_data_publish(servers_publish_data_id_e id, servers_publish_data_args_t *args)
{
    (void)id;
    (void)args;
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_servers_manager_hook(servers_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __servers_manager_hook));
    ctx->hook->on_post_init_route = on_post_servers_init_route;
    ctx->hook->on_servers_manager_receiver_route = on_servers_manager_receiver_route;
    ctx->hook->factory_server_data_publish = factory_server_data_publish;
    ctx->hook->app_server_data_publish = app_server_data_publish;
    ctx->hook->data_bury_server_data_publish = data_bury_server_data_publish;
    ctx->hook->servers_work_thread_route = servers_work_thread_route;
}
