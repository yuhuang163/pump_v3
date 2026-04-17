#ifndef __HAL_INPUT_H__
#define __HAL_INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/// 按键固件升级事件
typedef enum {
    KEY_OTA_EVENT_UPDATE_PROGRESS,
    KEY_OTA_EVENT_UPDATE_FAIL,
    KEY_OTA_EVENT_UPDATE_SUCCESS,
    KEY_OTA_EVENT_UPDATE_START,
    KEY_OTA_EVENT_MAX,
} key_ota_event_e;

/// 按键固件版本类型
typedef enum {
    KEY_FW_VERSION_CURRENT = 0,
    KEY_FW_VERSION_TARGET = 1,
} key_fw_version_type_e;

/// 按键固件升级操作码
typedef enum {
    OP_CODE_INPUT_ADD_CALLBACK = 1,
    OP_CODE_INPUT_IS_TRIGGERED = 2,
    OP_CODE_INPUT_GET_KEY_STATE = 3,
    OP_CODE_INPUT_SET_KEY_SENSITIVITY = 4,
    OP_CODE_INPUT_IS_READY = 5,
    OP_CODE_INPUT_TOUCH_IS_READY = 6,
    OP_CODE_INPUT_ADD_KEY_OTA_EVENT_CB = 7,
    OP_CODE_INPUT_KEY_FW_VERSION_CHECK = 8, /// 检查按键固件版本
    OP_CODE_INPUT_GET_KEY_FW_VERSION = 9, /// 获取按键固件版本
    OP_CODE_INPUT_TOUCH_SIGNAL_READ = 10, /// 读取触摸信号
} input_dev_op_code_e;

/// 按键固件升级事件回调函数
typedef void (*key_ota_event_cb_func)(key_ota_event_e event, uint8_t progress);

#ifdef __cplusplus
}
#endif

#endif 
