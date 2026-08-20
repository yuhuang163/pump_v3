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
#include "project_support_overlay.h"

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void key_irq_event_callback(int index);
void imu_irq_event_callback(void);
void bsp_irq_msg_send(uint8_t index);
bool is_full_load_running(void);
void bsp_set_full_load_running(bool is_running);
void knob_button_event_handler(int arg1, int arg2, void *param);
#if IS_SUPPORT_DEVICE_KNOB
uint16_t bsp_knob_get_count(uint8_t index);
#endif
bool touch_key_is_upgrading(void);
#ifdef __cplusplus
}
#endif

#endif 
