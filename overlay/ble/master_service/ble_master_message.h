#ifndef __BLE_MASTER_MESSAGE_H__
#define __BLE_MASTER_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep

typedef enum {
    BLE_SIID_BIND_CONFIG    = 1,
    BLE_SIID_BATTERY_REQ    = 2,
    BLE_SIID_SWITCH_REQ     = 3,
    BLE_SIID_SUPPLY_REQ     = 4,
    BLE_SIID_MODE_PARAM     = 5,
    BLE_SIID_RECORD_REQ     = 6,
    BLE_SIID_TIMESTAMP_REQ  = 7,
    BLE_SIID_REPORT_EVT     = 8,
    BLE_SIID_LANGUAGE_INFO  = 10,
} ble_msg_siid_e;

typedef int (*ble_msg_protocol_handle)(void *context, void *message);

typedef struct {
    ble_msg_siid_e id;
    ble_msg_protocol_handle handle_func;
} ble_protocol_list_t;

int ble_master_srv_recv_data(void *context, ble_service_data_t *srv_data);
int ble_master_srv_send_data(void *context, ble_service_data_t *srv_data);

#ifdef __cplusplus
}
#endif

#endif 
