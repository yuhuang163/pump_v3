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
 * @brief 无刷/扫震电机参数表大小
*/
#define MOTOR_PARA_TABLE_SIZE              102

/**
 * @brief 微振参数
*/
#define SHAKE_PARA_TABLE_INDEX            101 

/**
 * @brief 描述电机操作码
*/
typedef enum {
    OP_CODE_MOTOR_START_PLAY = (1 << 1),
    OP_CODE_MOTOR_STOP_PLAY = (1 << 2),
    OP_CODE_MOTOR_SET_PARAMETER = (1 << 3),
    OP_CODE_MOTOR_GET_STATUS = (1 << 4),
    OP_CODE_MOTOR_RESET = (1 << 5),
    OP_CODE_MOTOR_SET_POWER = (1 << 6),
    OP_CODE_MOTOR_ADD_CB_UPGRADE = (1 << 7),
    OP_CODE_MOTOR_ADD_CB_CALI = (1 << 8),
    OP_CODE_MOTOR_ADD_CB_ERROR = (1 << 9),
    OP_CODE_MOTOR_SET_PWM_DUTY = (1 << 10),
    OP_CODE_MOTOR_DUTY_CLIMB = (1 << 11),
} motor_dev_op_code_e;

/**
 * @brief 描述无刷/扫震电机模式
*/
typedef enum {
    BLDC_COMMAND_MODE = 0,  // 指令模式，传递指令，吸奶，校准等
    BLDC_DATA_FLOW_MODE,    // 数据流模式，用于传输音频数据流
} bldc_mode_e;

/**
 * @brief 描述无刷/扫震电机控制类型
*/
typedef enum {
    BLDC_DATA_CONTROL = 0,        // 透传数据控制
    BLDC_LOOKUP_TABLE_CONTROL,    // 查表控制
} bldc_contorl_type_e;

/**
 * @brief 描述无刷/扫震电机参数
*/
typedef struct {
    uint16_t shake_angle;
    uint16_t shake_freq;
    uint16_t sweep_angle;
    uint16_t sweep_freq;
} bldc_para_t;

/**
 * @brief 描述无刷/扫震电机参数表
*/
typedef struct {
    uint16_t index;
    uint16_t map;
} bldc_table_t;

/**
 * @brief 描述有刷电机参数
*/
typedef struct {
    uint16_t frequency;
    uint16_t duty_start;
    uint16_t duty_hold;
    uint16_t climb_time;
} dcm_cfg_t;

/**
 * @brief 描述电机混合参数
*/
typedef struct {
    union {
        bldc_para_t bldc;
        bldc_table_t table;
        dcm_cfg_t   dcm;
    } item; 
} motor_para_t;

/**
 * @brief 描述电机配置
*/
typedef struct __motor_cfg {
    motor_para_t motor;
    uint32_t hold_time;
} motor_cfg_t;

#ifdef __cplusplus
}
#endif

#endif 
