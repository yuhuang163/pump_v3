/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"  // IWYU pragma: keep
#include "master_service.h" // IWYU pragma: keep
#include "factory_service.h" // IWYU pragma: keep
#include "remot_service.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint8_t conn_idx = 0;
static ble_data_cb_func s_ble_data_callback = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
bool ble_service_notify_status_get(uint8_t srv_id)
{
    if (BLE_SRV_ID_MASTER == srv_id) {
        return ble_master_service_notify_status_get();
    } else if (BLE_SRV_ID_FACTORY == srv_id) {
        return ble_factory_service_notify_status_get();
    } else if (BLE_SRV_ID_REMOT == srv_id) {
        return ble_remot_service_notify_status_get();
    }
    LOGE("err ble srv_id %d", srv_id);

    return false;
}

void ble_service_data_recv(uint8_t srv_id, uint8_t *p_data, uint16_t data_len)
{
    if (s_ble_data_callback) {
        s_ble_data_callback(srv_id, p_data, data_len);
    }
}

int ble_service_data_send(uint8_t srv_id, uint8_t *p_data, uint16_t size, uint8_t bind_state)
{
    if (BLE_SRV_ID_MASTER == srv_id) {
        return ble_master_service_notify_data_send(p_data, size);
    } else if (BLE_SRV_ID_FACTORY == srv_id) {
        return ble_factory_service_notify_data_send(p_data, size);
    } else if (BLE_SRV_ID_REMOT == srv_id) {
        return ble_remot_service_notify_data_send(p_data, size);
    }
    LOGE("err ble srv_id %d", srv_id);  
    
    return -1;
}

void add_ble_custom_data_callback(ble_data_cb_func cb)
{
    if (cb) {
        s_ble_data_callback = cb;
    }
}