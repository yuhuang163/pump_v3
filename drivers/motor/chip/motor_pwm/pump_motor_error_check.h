#ifndef __PUMP_MOTOR_CHECK_H__
#define __PUMP_MOTOR_CHECK_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "hal_motor.h"

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int pump_motor_error_check_init(void);/// 初始化堵转检测
int pump_motor_error_check_stop(void);/// 停止堵转检测
int pump_motor_error_check_start(void);/// 启动堵转检测
int pump_motor_error_check_is_running(void);/// 判断堵转检测是否正在运行
int pump_motor_error_check_register_callback(void (*callback)(uint8_t pump_id, motor_dev_status_e status));/// 注册堵转检测回调函数
#ifdef __cplusplus
}
#endif

#endif 
