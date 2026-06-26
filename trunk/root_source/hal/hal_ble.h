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
    /// adv
    OP_CODE_BLE_ADV_OPEN = 1,
    OP_CODE_BLE_ADV_CLOSE = 2,
    OP_CODE_BLE_ADV_PARAM_UPDATE = 3,
    OP_CODE_BLE_ADV_SPEED_SET = 4,
    /// gatt
    OP_CODE_BLE_DISCONNECT = 4,
    OP_CODE_BLE_MAC_ADDR_SET = 5,
    OP_CODE_BLE_ADD_EVENT_CALLBACK = 6,
    OP_CODE_BLE_ADD_DATA_CALLBACK = 7,
    OP_CODE_BLE_MAC_ADDR_GET = 8,

    /// ota
    OP_CODE_BLE_ADD_OTA_EVENT_CALLBACK = 9,

    /// bt test
    OP_CODE_BLE_TEST_ENTER_SIGNALINGTEST_MODE = 10,
    OP_CODE_BLE_TEST_ENTER_NOSIGNALINGTEST_MODE = 11,
    OP_CODE_BLE_TEST_XTAL_CALIB_PROC_START = 12,
    OP_CODE_BLE_TEST_XTAL_CALIB_SET = 13,
    OP_CODE_BLE_TEST_XTAL_CALIB_GET = 14,
    OP_CODE_BLE_TEST_XTAL_CALIB_END = 15,
    OP_CODE_BLE_TEST_GET_RSSI_VALUE = 16,

    OP_CODE_BLE_END
} ble_dev_op_code_e;

/**
 * @brief 描述BLE服务ID
*/
typedef enum {
    BLE_SRV_ID_MASTER = 0,  /**< 主服务 */
    BLE_SRV_ID_FACTORY,     /**< 工厂配置服务 */
    BLE_SRV_ID_REMOTE,      /**< 远程服务 */
    BLE_SRV_ID_DATA_BURY,   /**< Momcozy 数据埋点服务 9F6B1A20 */
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
    BLE_EVENT_TX_CCC_ENABLED,
    BLE_EVENT_TX_CCC_DISABLED,
} ble_event_e;

typedef struct {
    uint8_t srv_id;
    uint8_t bind_state;
    uint8_t *p_data;
} ble_service_send_data_t;

/**
 * @brief BLE 连接类事件参数（由 driver 从 GAP 回调填充，供 trunk 使用）
 */
typedef struct {
    uint16_t connhdl;
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t mtu;
    uint8_t service_id;
} ble_hal_event_param_t;

/**
 * @brief 描述BLE事件回调
 * @param connhdl 连接句柄（无连接上下文时为 0）
 * @param param   事件扩展参数；连接类事件为 ble_hal_event_param_t*，其余可为 NULL
 */
typedef void (*ble_event_cb_func)(ble_event_e event, uintptr_t connhdl, void *param);

/**
 * @brief 描述BLE数据回调
*/
typedef void (*ble_data_cb_func)(ble_service_id_e service_id, uint8_t *p_data, uint16_t size);

/// OTA 事件类型
typedef enum {
    OTA_EVENT_GET_OTA_VERSION = 0, /// 获取OTA协议版本
    OTA_EVENT_SET_USER_TYPE, /// 设置OTA类型，升级主固件、语言包、组合固件等不同类型
    OTA_EVENT_GET_USER_VERSION, /// 获取用户版本，主固件、语言包、组合固件等不同版本
    OTA_EVENT_START_OTA, /// 开始OTA，开始升级
    OTA_EVENT_RESUME_OTA, /// 恢复OTA，从断点继续升级
    OTA_EVENT_OTA_PROGRESS, /// OTA进度
    OTA_EVENT_OTA_FAIL, /// OTA失败
    OTA_EVENT_UPDATE_IMG_TYPE, /// 更新镜像类型
    OTA_EVENT_OTA_SUCCESS, /// OTA成功
} ota_event_type_e;

/**
 * @brief 描述OTA事件回调
*/
typedef void (*ota_event_cb_func)(ota_event_type_e event, int arg1, int arg2);

#ifdef __cplusplus
}
#endif

#endif
