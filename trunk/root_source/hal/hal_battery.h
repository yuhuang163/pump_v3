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
    CHARGE_STATE_ERROR = 0,
    CHARGE_STATE_FULL,
    CHARGE_STATE_CHARGING,
    CHARGE_STATE_UNCHARGE,
    CHARGE_STATE_NO_BATT,
    CHARGE_STATE_MAX,
} charge_state_e;

/**
 * @brief 描述电池操作码
*/
typedef enum {
    OP_CODE_BAT_GET_LEVEL = (1 << 1),
    OP_CODE_BAT_GET_VOLTAGE = (1 << 2),
    OP_CODE_BAT_POLL_VOLTAGE = (1 << 3),
    OP_CODE_BAT_HEALTH_SET = (1 << 4),
    OP_CODE_BAT_CHARGE_IS_PLUG_IN = (1 << 5),
    OP_CODE_BAT_CHARGE_ADD_IRQ_CALLBACK = (1 << 6),
    OP_CODE_BAT_GET_CHARGE_STATUS = (1 << 7),
    OP_CODE_BAT_END
} batery_dev_op_code_e;

#ifdef __cplusplus
}
#endif

#endif 
