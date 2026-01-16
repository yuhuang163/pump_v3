#ifndef __MOTOR_PWM_H__
#define __MOTOR_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include <stddef.h> // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep
#include "hwtimer_list.h"
#include "hal_pwm.h"

/**
 * @brief 描述定时器配置
*/
typedef struct {
    HWTIMER_ID time_id;
    void (*time_handler)(void *param);
    uint16_t time_period;
    uint8_t time_status;   // 定时器状态，0：停止，1：运行
    struct HAL_PWM_CFG_T pwm_cfg;
    uint8_t init_duty;
    uint8_t target_duty;
    uint8_t duty_step;      // 步长（无符号，总是增加）
    uint8_t current_duty;   // 当前占空比
    uint8_t gpio_irq_update;    // GPIO中断更新标志，0：不更新，1：更新    
} motor_pwm_timer_cfg_t;
/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
/**
 * @brief 测试所有可能的步数组合（1-128）
 * 系统地测试各种 step_count 值，确保计算逻辑正确
 */
void bsp_motor_pwm_test_all_step_counts(void);
#endif

#ifdef __cplusplus
}
#endif

#endif 
