#ifndef __BLE_FACTORY_MESSAGE_H__
#define __BLE_FACTORY_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep

#define BLE_FACTORY_CONFIG_HEAD          (0xC0)

int ble_factory_srv_recv_data(void *context, ble_service_data_t *p_data);
int ble_factory_srv_send_data(void *context, ble_service_data_t *p_data);
#ifdef __cplusplus
}
#endif

#endif 


