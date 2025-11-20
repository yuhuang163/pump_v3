#ifndef __BSP_INTERNAL_H__
#define __BSP_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include "stdbool.h"

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void key_irq_event_callback(int index);
void imu_irq_event_callback(void);
void bsp_irq_msg_send(uint8_t index);
bool is_full_load_running(void);

#ifdef __cplusplus
}
#endif

#endif 
