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
int bsp_motor_pump_init(void);
int bsp_motor_pump_deinit(void);
int bsp_motor_pump_parameter_set(void *args);
int bsp_motor_pump_is_running(void *args);
int bsp_motor_pump_add_timer_event_cb(pump_timer_event_cb cb);
int bsp_motor_pump_add_stalled_cb(pump_stalled_cb callback);

#ifdef __cplusplus
}
#endif

#endif 

