#ifndef __MOTOR_MANAGER_H__
#define __MOTOR_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void motor_policy_init(struct motor_manager_context *thiz);
void motor_mode_change_handler(struct motor_manager_context *thiz, int breast_pump_mode);
#ifdef __cplusplus
}
#endif

#endif 







