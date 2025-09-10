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
    READ_CODE_BLE_MAC_ADDR_GET = (1 << 0),
    READ_CODE_BLE_END
} ble_dev_read_code_e;

/**
 * @brief 描述BLE操作码
*/
typedef enum {
    // adv
    OP_CODE_BLE_ADV_OPEN = (1 << 0),
    OP_CODE_BLE_ADV_CLOSE = (1 << 1),
    OP_CODE_BLE_ADV_SPEED_SET = (1 << 2),
    OP_CODE_BLE_ADV_PARAM_UPDATE = (1 << 3),
    // gatt
    OP_CODE_BLE_DISCONNECT = (1 << 4),
    OP_CODE_BLE_MAC_ADDR_SET = (1 << 5),
    OP_CODE_BLE_END
} ble_dev_op_code_e;

/**
 * @brief 描述BLE服务ID
*/
typedef enum {
    BLE_SRV_ID_MASTER = 0,  /**< 主服务 */
    BLE_SRV_ID_FACTORY,     /**< 工厂配置服务 */
    BLE_SRV_ID_MAX,
} ble_service_id_e;

#ifdef __cplusplus
}
#endif

#endif
