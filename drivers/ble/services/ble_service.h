#ifndef __BLE_SERVICE_H__
#define __BLE_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep

typedef int (*ble_service_notify_data_send)(uint8_t *p_data, uint32_t size);

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

int ble_service_data_send(uint8_t srv_id, uint8_t *p_data, uint16_t size, uint8_t bind_state);
void ble_service_data_recv(uint8_t srv_id, uint8_t *p_data, uint16_t data_len);
bool ble_service_notify_status_get(uint8_t srv_id);
void add_ble_custom_data_callback(ble_data_cb_func cb);

#ifdef __cplusplus
}
#endif

#endif 
