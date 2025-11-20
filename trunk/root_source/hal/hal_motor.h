#ifndef __HAL_MOTOR_H__
#define __HAL_MOTOR_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>

/**
 * @brief 描述电机操作码
*/
typedef enum {
    OP_CODE_MOTOR_START_PLAY = 1,
    OP_CODE_MOTOR_STOP_PLAY = 2,
    OP_CODE_MOTOR_SET_PARAMETER = 3,
    OP_CODE_MOTOR_GET_STATUS = 4,
    OP_CODE_MOTOR_RESET = 5,
    OP_CODE_MOTOR_SET_POWER = 6,
    OP_CODE_MOTOR_ADD_CB_UPGRADE = 7,
    OP_CODE_MOTOR_ADD_CB_CALI = 8,
    OP_CODE_MOTOR_ADD_CB_ERROR = 9,
    OP_CODE_MOTOR_SET_PWM_DUTY = 10,
    OP_CODE_MOTOR_DUTY_CLIMB = 11,
} motor_dev_op_code_e;

/**
 * @brief 描述电机控制类型
*/
typedef enum {
    MOTOR_PWM_PUMP_1 = 0,
    MOTOR_PWM_PUMP_2,
    MOTOR_PWM_VALVE_1,
    MOTOR_PWM_VALVE_2,
    MOTOR_PWM_ID_MAX,
    MOTOR_PWM_ALL_ID,
} motor_pwm_control_id_e;

/**
 * @brief 描述pwm配置
*/
typedef struct {
    uint8_t duty;
    uint16_t freq;
} pwm_cfg_t;

/**
 * @brief 描述电机混合参数
*/
typedef struct {
    union {
        pwm_cfg_t pwm;
    } un; 
} motor_para_t;

#ifdef __cplusplus
}
#endif

#endif 
