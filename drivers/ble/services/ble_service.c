/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"  // IWYU pragma: keep
#include "master_service.h" // IWYU pragma: keep
#include "factory_service.h" // IWYU pragma: keep
#include "remot_service.h" // IWYU pragma: keep
#include "data_bury_service.h" // IWYU pragma: keep
#include "ble_service.h" // IWYU pragma: keep
#include "bsp_ble_internal.h" // IWYU pragma: keep
#include "root_common.h"

/*
* DEFINES
*****************************************************************************************
*/
#define BLE_ATT_HEADER_SIZE      3 /// ATT头大小

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint8_t conn_idx = 0;
static ble_data_cb_func s_ble_data_callback = NULL;
static void *s_ble_mtu_size = NULL;
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
    } else if (BLE_SRV_ID_REMOTE == srv_id) {
        return ble_remot_service_notify_status_get();
    } else if (BLE_SRV_ID_DATA_BURY == srv_id) {
        return ble_data_bury_service_notify_status_get();
    }
    LOGE("err ble srv_id %d", srv_id);

    return false;
}

void ble_service_data_recv(uint8_t srv_id, uint8_t *p_data, uint16_t data_len)
{
    if (s_ble_data_callback) {
        s_ble_data_callback(srv_id, p_data, data_len);

    } else{
        LOGE("err ble data callback is NULL for srv_id %d", srv_id);
    }
}

int ble_service_data_send(uint8_t srv_id, uint8_t *p_data, uint16_t size, uint8_t bind_state)
{
    EXCEPTION_CHECK_INT(p_data); 
    if (!ble_service_notify_status_get(srv_id)) {
        LOGE("notify not enabled, srv_id %d", srv_id);
        return -1;
    }
    /// 获取notify回调函数
    ble_service_notify_data_send notify_cb = NULL;
    if (BLE_SRV_ID_MASTER == srv_id) {
        notify_cb = ble_master_service_notify_data_send;
    } else if (BLE_SRV_ID_FACTORY == srv_id) {
        notify_cb = ble_factory_service_notify_data_send;
    } else if (BLE_SRV_ID_REMOTE == srv_id) {
        notify_cb = ble_remot_service_notify_data_send;
    } else if (BLE_SRV_ID_DATA_BURY == srv_id) {
        notify_cb = ble_data_bury_service_notify_data_send;
    }
    EXCEPTION_CHECK_INT(notify_cb);
    /// 获取MTU大小并校验
    uint16_t mtu_size = ble_custom_mtu_size_get() - BLE_ATT_HEADER_SIZE;
    if (mtu_size == 0) {
        LOGE("invalid mtu_size 0");
        return -1;
    }
    /// 分包发送
    int ret = 0;
    uint8_t *pos = NULL;
    uint16_t pos_offset = 0;
    uint16_t data_size = 0;
    for (;;) {
        if (pos_offset >= size) {
            break;
        }
        pos = p_data + pos_offset;
        data_size = size - pos_offset;
        if (data_size > mtu_size) {
            data_size = mtu_size;
        }
        ret = notify_cb(pos, data_size);
        if (ret != 0) {
            LOGE("notify_cb failed at offset %d, ret=%d", pos_offset, ret);
            break;
        }
        BSP_LOG_HEX("ble notify data", pos, data_size);
        //BSP_LOG("ble", "idx %d, data_size %d, pos_offset %d, total %d", conn_idx, data_size, pos_offset, size);
        pos_offset += data_size;
    }
    return ret;
}

void add_ble_custom_data_callback(ble_data_cb_func cb)
{
    if (cb) {
        s_ble_data_callback = cb;
    }
}