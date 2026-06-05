#ifndef __DATA_BURY_PUBLISH_PROTOCOL_H__
#define __DATA_BURY_PUBLISH_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_service.h"
#include "servers_manager_api.h"

/*
 * 埋点事件上报（参照 tests_encoder_status_publish）
 *
 * 定长事件：args->val 指向 packed 结构体，arg1=0 时使用默认 sizeof
 * 变长事件（EVT_005/EVT_014）：args->val 指向 packed 缓冲，arg1=总字节数（必填）
 */

int data_bury_pump_startup_publish(servers_publish_data_args_t *args);
int data_bury_pump_shutdown_publish(servers_publish_data_args_t *args);
int data_bury_bt_connect_event_publish(servers_publish_data_args_t *args);
int data_bury_pump_session_start_publish(servers_publish_data_args_t *args);
int data_bury_pump_session_end_publish(servers_publish_data_args_t *args);
int data_bury_pump_manual_adjust_publish(servers_publish_data_args_t *args);
int data_bury_pump_user_event_publish(servers_publish_data_args_t *args);
int data_bury_milk_ejection_publish(servers_publish_data_args_t *args);
int data_bury_bat_charge_start_publish(servers_publish_data_args_t *args);
int data_bury_bat_charge_end_publish(servers_publish_data_args_t *args);
int data_bury_exception_publish(servers_publish_data_args_t *args);
int data_bury_firmware_update_publish(servers_publish_data_args_t *args);
int data_bury_overflow_publish(servers_publish_data_args_t *args);
int data_bury_milk_volume_publish(servers_publish_data_args_t *args);
int data_bury_heat_session_start_publish(servers_publish_data_args_t *args);
int data_bury_heat_session_end_publish(servers_publish_data_args_t *args);
int data_bury_massage_session_start_publish(servers_publish_data_args_t *args);
int data_bury_massage_session_end_publish(servers_publish_data_args_t *args);
int data_bury_wear_detection_publish(servers_publish_data_args_t *args);

comm_protocol_publish_func get_data_bury_publish_func(servers_publish_data_id_e id);

/** 定长事件快捷上报 */
#define DATA_BURY_PUBLISH(ctx, publish_id, evt_ptr)                             \
    do {                                                                        \
        servers_publish_data_args_t _bury_args = { .val = (void *)(evt_ptr) };  \
        (ctx)->hook->data_bury_server_data_publish((publish_id), &_bury_args);  \
    } while (0)

/** 变长事件快捷上报（arg1=payload 总字节数） */
#define DATA_BURY_PUBLISH_LEN(ctx, publish_id, buf_ptr, payload_len)            \
    do {                                                                        \
        servers_publish_data_args_t _bury_args = {                              \
            .arg1 = (int)(payload_len),                                         \
            .val = (void *)(buf_ptr),                                           \
        };                                                                      \
        (ctx)->hook->data_bury_server_data_publish((publish_id), &_bury_args);  \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_PUBLISH_PROTOCOL_H__ */
