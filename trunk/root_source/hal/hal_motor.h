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

    OP_CODE_MOTOR_ADD_STALLED_CALLBACK = 11,
    OP_CODE_MOTOR_ADD_TIMER_EVENT_CALLBACK = 12,
    OP_CODE_MOTOR_SET_PUMP_PARAMETER = 13,
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
 * @brief 描述电机堵转状态
*/
typedef enum {
    MOTOR_STATUS_OK = 0,
    MOTOR_STATUS_STALL,
    MOTOR_STATUS_MAX,
} motor_dev_status_e;

/**
 * @brief 描述pwm配置
*/
typedef struct {
    uint8_t  pwm_id;
    uint8_t  duty;
    uint16_t freq;
} pwm_cfg_t;

/**
 * @brief 描述泵ID
*/
typedef enum {
    MOTOR_PUMP_1 = 0,
    MOTOR_PUMP_2,
    MOTOR_PUMP_ID_MAX,
} motor_pump_id_e;

/**
 * @brief 描述泵状态
*/
typedef enum {
    MOTOR_PUMP_IDLE = 0, /// 泵空闲
    MOTOR_PUMP_ON = 100, /// 泵运行
    MOTOR_PUMP_OFF, /// 泵停止
    MOTOR_PUMP_OFF_AND_VALVE_ON, /// 泵停止并电磁阀门打开
    MOTOR_VALVE_ON, /// 电磁阀门打开
    MOTOR_VALVE_OFF, /// 电磁阀门关闭
    MOTOR_PUMP_OFF_AND_VALVE_OFF, /// 泵停止并电磁阀门打开
} motor_pump_status_e;

#if (MOTOR_DRIVER_VERSION == MOTOR_DRIVER_V1)
#define PUMP_DUTY_SCALE_FACTOR                 (1)  /// 占空比缩放因子
#else
#define PUMP_DUTY_SCALE_FACTOR                 (100)  /// 占空比缩放因子
#endif

/**
 * @brief 描述马达参数
*/
typedef struct {
    uint16_t pulse_width;
    uint16_t freq;
    uint16_t duration_ms;
    motor_pump_status_e action;
    uint16_t pump_id;
} motor_pump_para_t;

/**
 * @brief 描述泵定时器事件回调
*/
typedef void (*pump_timer_event_cb)(uint8_t pump_id, motor_pump_status_e action);

/**
 * @brief 描述泵堵转回调
*/
typedef void (*pump_stalled_cb)(uint8_t pump_id, motor_dev_status_e status);

/**
 * @brief 描述电机混合参数
*/
typedef struct {
    union {
        pwm_cfg_t pwm;
        motor_pump_para_t pump;
    } un; 
} motor_para_t;

#ifdef __cplusplus
}
#endif

#endif 
