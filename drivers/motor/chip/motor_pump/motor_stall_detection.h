#ifndef __PUMP_MOTOR_STALL_DETECTION_H__
#define __PUMP_MOTOR_STALL_DETECTION_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int pump_motor_stall_detection_start(uint8_t pump_id, uint16_t duration_ms, uint16_t pulse_width);
int pump_motor_stall_detection_stop(uint8_t pump_id);
bool pump_motor_is_stalled(uint8_t pump_id);

#ifdef __cplusplus
}
#endif

#endif 
