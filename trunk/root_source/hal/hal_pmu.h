#ifndef __HAL_PMU_H__
#define __HAL_PMU_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述PMU操作码
*/
typedef enum {
    OP_CODE_PMU_SLEEP_POLL = 1,
    OP_CODE_PMU_REGISTER_TIMER_WAKEUP = 2,
    OP_CODE_PMU_UNREGISTER_TIMER_WAKEUP = 3,
    OP_CODE_PMU_GET_RESET_REASON = 4,
    OP_CODE_PMU_ADD_WAKEUP_CB = 5,
    OP_CODE_PMU_REMOVE_WAKEUP_CB = 6,
    OP_CODE_PMU_GET_DP_WAKEUP_REASON = 7,
    OP_CODE_PMU_GET_RTC_MEMORY = 8,
    OP_CODE_PMU_SET_IMU_WAKEUP = 9,
    OP_CODE_PMU_SET_SLEEP_MODE = 10,
    OP_CODE_PMU_SET_ACTIVE_MODE = 11,
} pmu_dev_op_code_e;

/**
 * @brief 描述唤醒类型
*/
typedef enum 
{
    WAKEUP_TYPE_UNKNOWN = 0,
    WAKEUP_TYPE_POWER_KEY,
    WAKEUP_TYPE_IMU_INTR,
    WAKEUP_TYPE_CHARGING,
    WAKEUP_TYPE_TIMING,
    WAKEUP_TYPE_BLE,
    WAKEUP_TYPE_RTC,
    WAKEUP_TYPE_EXT_BLE,

} wakeup_type_e;

/**
 * @brief 描述复位类型
*/
typedef enum 
{
    RESET_TYPE_OTHER = 0,
    RESET_TYPE_DEEPSLEEP,
} reset_type_e;

/**
 * @brief 描述休眠结果返回
*/
typedef enum {
    PMU_SLEEP_RESULT_SUCCESS = 0,
    PMU_SLEEP_RESULT_IGNORE = -1,
    PMU_SLEEP_RESULT_EXCEPTION = -2,
} pmu_sleep_result_e;

#ifdef __cplusplus
}
#endif

#endif 
