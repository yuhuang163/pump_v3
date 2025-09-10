#ifndef __BSP_INTERNAL_H__
#define __BSP_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdio.h> // IWYU pragma: keep
#include <stdint.h> // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void key_irq_event_callback(int index);
void charge_irq_event_callback(void);
void imu_irq_event_callback(void);
void bsp_irq_msg_send(uint8_t index);

#ifdef __cplusplus
}
#endif

#endif 
