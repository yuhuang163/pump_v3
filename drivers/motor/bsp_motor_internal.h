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
int bsp_motor_init(void);
int bsp_motor_deinit(void);
int bsp_motor_start(void *args);
int bsp_motor_stop(void *args);
int bsp_motor_parameter_set(void *args);
int bsp_motor_is_running(void *args);

#ifdef __cplusplus
}
#endif

#endif 

