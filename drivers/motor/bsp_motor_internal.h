#ifndef __BSP_MOTOR_INTERNAL_H__
#define __BSP_MOTOR_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
#if (MOTOR_DRIVER_VERSION == MOTOR_DRIVER_V1)
int bsp_motor_init(void);
int bsp_motor_deinit(void);
int bsp_motor_start(void *args);
int bsp_motor_stop(void *args);
int bsp_motor_parameter_set(void *args);
int bsp_motor_is_running(void *args);
int bsp_motor_get_pump_target_duty(motor_pwm_control_id_e id);
int pump_motor_error_check_register_callback(void (*callback)(uint8_t pump_id, motor_dev_status_e status));/// 注册堵转检测回调函数
#else
int bsp_motor_pump_init(void);
int bsp_motor_pump_deinit(void);
int bsp_motor_pump_parameter_set(void *args);
int bsp_motor_pump_is_running(void *args);
int bsp_motor_pump_add_timer_event_cb(pump_timer_event_cb cb);
int bsp_motor_pump_add_stalled_cb(pump_stalled_cb callback);
#endif




#ifdef __cplusplus
}
#endif

#endif 

