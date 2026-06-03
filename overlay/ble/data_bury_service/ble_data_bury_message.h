#ifndef __BLE_DATA_BURY_MESSAGE_H__
#define __BLE_DATA_BURY_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep

int ble_data_bury_srv_recv_data(void *context, ble_service_data_t *srv_data);
int ble_data_bury_srv_send_data(void *context, ble_service_data_t *srv_data);

#ifdef __cplusplus
}
#endif

#endif
