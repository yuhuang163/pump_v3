/*
 * Momcozy 数据埋点 Data Service 对外上报接口
 * 参照 tests_encoder_status_publish + factory_server_data_publish 分发
 */

#include "root_common_api.h"
#include "common_service.h"
#include "data_bury_publish_protocol.h"
#include "data_bury_server_protocol.h"
#include "data_bury_persist_defs.h"

#define DATA_BURY_DEFINE_FIXED_PUBLISH(fn, cmd, struct_type)                    \
    int data_bury_##fn##_publish(servers_publish_data_args_t *args)           \
    {                                                                           \
        uint16_t len;                                                           \
        EXCEPTION_CHECK_INT(args && args->val);                                 \
        len = (args->arg1 > 0) ? (uint16_t)args->arg1 : (uint16_t)sizeof(struct_type); \
        return data_bury_server_push_event((cmd), args->val, len);              \
    }

#define DATA_BURY_DEFINE_VAR_PUBLISH(fn, cmd)                                   \
    int data_bury_##fn##_publish(servers_publish_data_args_t *args)             \
    {                                                                           \
        EXCEPTION_CHECK_INT(args && args->val && args->arg1 > 0);               \
        return data_bury_server_push_event((cmd), args->val, (uint16_t)args->arg1); \
    }

DATA_BURY_DEFINE_FIXED_PUBLISH(pump_startup, APP_BURY_CMD_PUMP_STARTUP, data_bury_evt_pump_startup_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(pump_shutdown, APP_BURY_CMD_PUMP_SHUTDOWN, data_bury_evt_pump_shutdown_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(bt_connect_event, APP_BURY_CMD_BT_CONNECT_EVENT, data_bury_evt_bt_connect_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(pump_session_start, APP_BURY_CMD_PUMP_SESSION_START, data_bury_evt_pump_session_start_t)
DATA_BURY_DEFINE_VAR_PUBLISH(pump_session_end, APP_BURY_CMD_PUMP_SESSION_END)
DATA_BURY_DEFINE_FIXED_PUBLISH(pump_manual_adjust, APP_BURY_CMD_PUMP_MANUAL_ADJUST, data_bury_evt_pump_manual_adjust_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(pump_user_event, APP_BURY_CMD_PUMP_USER_EVENT, data_bury_evt_pump_user_evt_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(milk_ejection, APP_BURY_CMD_MILK_EJECTION, data_bury_evt_milk_ejection_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(bat_charge_start, APP_BURY_CMD_BAT_CHARGE_START, data_bury_evt_bat_charge_start_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(bat_charge_end, APP_BURY_CMD_BAT_CHARGE_END, data_bury_evt_bat_charge_end_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(exception, APP_BURY_CMD_EXCEPTION, data_bury_evt_exception_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(firmware_update, APP_BURY_CMD_FIRMWARE_UPDATE, data_bury_evt_firmware_update_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(overflow, APP_BURY_CMD_OVERFLOW, data_bury_evt_overflow_t)
DATA_BURY_DEFINE_VAR_PUBLISH(milk_volume, APP_BURY_CMD_MILK_VOLUME)
DATA_BURY_DEFINE_FIXED_PUBLISH(heat_session_start, APP_BURY_CMD_HEAT_SESSION_START, data_bury_evt_heat_session_start_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(heat_session_end, APP_BURY_CMD_HEAT_SESSION_END, data_bury_evt_heat_session_end_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(massage_session_start, APP_BURY_CMD_MASSAGE_SESSION_START, data_bury_evt_massage_session_start_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(massage_session_end, APP_BURY_CMD_MASSAGE_SESSION_END, data_bury_evt_massage_session_end_t)
DATA_BURY_DEFINE_FIXED_PUBLISH(wear_detection, APP_BURY_CMD_WEAR_DETECTION, data_bury_evt_wear_detection_t)

#undef DATA_BURY_DEFINE_VAR_PUBLISH
#undef DATA_BURY_DEFINE_FIXED_PUBLISH

static const comm_protocol_publish_handle_t s_data_bury_publish_list[] = {
    {SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_STARTUP, data_bury_pump_startup_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN, data_bury_pump_shutdown_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT, data_bury_bt_connect_event_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START, data_bury_pump_session_start_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END, data_bury_pump_session_end_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST, data_bury_pump_manual_adjust_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT, data_bury_pump_user_event_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_MILK_EJECTION, data_bury_milk_ejection_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START, data_bury_bat_charge_start_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_END, data_bury_bat_charge_end_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_EXCEPTION, data_bury_exception_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_FIRMWARE_UPDATE, data_bury_firmware_update_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_OVERFLOW, data_bury_overflow_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_MILK_VOLUME, data_bury_milk_volume_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_START, data_bury_heat_session_start_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_END, data_bury_heat_session_end_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_START, data_bury_massage_session_start_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_END, data_bury_massage_session_end_publish},
    {SERVERS_PUBLISH_DATA_DATA_BURY_WEAR_DETECTION, data_bury_wear_detection_publish},
};

comm_protocol_publish_func get_data_bury_publish_func(servers_publish_data_id_e id)
{
    const comm_protocol_publish_handle_t *list = s_data_bury_publish_list;
    uint16_t list_len = LENGTH_OF_ARRAY(s_data_bury_publish_list);
    int left = 0;
    int right = (int)list_len - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (list[mid].id == id) {
            return list[mid].publish_func;
        }
        if (list[mid].id < id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    LOGE("==> get data bury publish func failed: %d", id);
    return NULL;
}
