#ifndef __HAL_BLE_H__
#define __HAL_BLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述BLE读取码
*/
typedef enum {
    READ_CODE_BLE_MAC_ADDR_GET = 1,
    READ_CODE_BLE_END
} ble_dev_read_code_e;

/**
 * @brief 描述BLE操作码
*/
typedef enum {
    // adv
    OP_CODE_BLE_ADV_OPEN = 1,
    OP_CODE_BLE_ADV_CLOSE = 2,
    OP_CODE_BLE_ADV_PARAM_UPDATE = 3,
    // gatt
    OP_CODE_BLE_DISCONNECT = 4,
    OP_CODE_BLE_MAC_ADDR_SET = 5,
    OP_CODE_BLE_ADD_EVENT_CALLBACK = 6,
    OP_CODE_BLE_ADD_DATA_CALLBACK = 7,
    OP_CODE_BLE_END
} ble_dev_op_code_e;

/**
 * @brief 描述BLE服务ID
*/
typedef enum {
    BLE_SRV_ID_MASTER = 0,  /**< 主服务 */
    BLE_SRV_ID_FACTORY,     /**< 工厂配置服务 */
    BLE_SRV_ID_REMOT,       /**< 远程服务 */
    BLE_SRV_ID_MAX,
} ble_service_id_e;

/**
 * @brief 描述BLE外设事件
*/
typedef enum {
    BLE_EVENT_ADVERTISE_STARTED = 0,
    BLE_EVENT_ADVERTISE_STOPPED,
    BLE_EVENT_CONN_OPENED,
    BLE_EVENT_CONN_CLOSED,
    BLE_EVENT_CONN_FAILED,
    BLE_EVENT_CONN_MTU_EXCHANGED,
    BLE_EVENT_USER_CONFIRM,
    BLE_EVENT_CONN_ENCRYPTED,
    BLE_EVENT_CONN_UPDATE_REQ,
    BLE_EVENT_CONN_PARAMS_UPDATE,
} ble_event_e;

/**
 * @brief 描述BLE事件回调
*/
typedef void (*ble_event_cb_func)(ble_event_e event, void *param);

/**
 * @brief 描述BLE数据回调
*/
typedef void (*ble_data_cb_func)(ble_service_id_e service_id, void *data, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif
