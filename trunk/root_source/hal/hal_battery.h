#ifndef __HAL_BATTERY_H__
#define __HAL_BATTERY_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述充电状态.
*/
typedef enum {
    CHARGE_STATE_UNCHARGE,
    CHARGE_STATE_FULL,
    CHARGE_STATE_CHARGING,
    CHARGE_STATE_ERROR,
    CHARGE_STATE_NO_BATT,
    CHARGE_STATE_MAX,
} charge_state_e;

/**
 * @brief 描述电池操作码
*/
typedef enum {
    OP_CODE_BAT_GET_LEVEL = 1,
    OP_CODE_BAT_GET_VOLTAGE = 2,
    OP_CODE_BAT_POLL_VOLTAGE = 3,
    OP_CODE_BAT_HEALTH_SET = 4,
    OP_CODE_BAT_VBUS_IS_PLUG_IN = 5,
    OP_CODE_BAT_ADD_CHARGE_EVENT_CALLBACK = 6,
    OP_CODE_BAT_GET_CHARGE_STATUS = 7,
    OP_CODE_BAT_GET_CHARGE_TEMPERATURE = 8,
    OP_CODE_BAT_IS_READY = 9,
    OP_CODE_BAT_CHARGE_CURRENT_ADJUST = 10, /// 充电电流调节
    OP_CODE_BAT_GET_CHARGE_IBAT_MA = 11,    /// 读取充电芯片采样的电池端充电电流
    OP_CODE_BAT_CHARGE_FORCE_OPEN = 12,     /// 强制开启充电
    OP_CODE_BAT_PD_IS_READY = 13,           /// 查询PD充电状态
    OP_CODE_BAT_PERCENT_UPDATE = 14,        /// 同步电池电量到系统状态
    OP_CODE_BAT_ADD_HEALTH_CHANGE_CALLBACK = 15, /// 注册电池健康变化回调
    OP_CODE_BAT_END
} batery_dev_op_code_e;

/**
 * @brief 描述电池充电事件
*/
typedef enum {
    CHARGE_EVENT_PLUG_IN = 0,
    CHARGE_EVENT_PLUG_OUT = 1,
    CHARGE_EVENT_FULL = 2,
    CHARGE_EVENT_UNCHARGE = 3,
    CHARGE_EVENT_ADAPTER_MISMATCH = 4, /// 适配器不匹配
    CHARGE_EVENT_TEMPERATURE_ERROR = 5, /// 温度异常
    CHARGE_EVENT_FORCE_PLUG_IN = 6, /// 强制触发充电
    CHARGE_EVENT_MAX,
} charge_event_e;

typedef void (*charge_event_cb)(charge_event_e event);
typedef void (*battery_health_cb)(uint8_t health);

#ifdef __cplusplus
}
#endif

#endif 
