#ifndef __BSP_LED_INTERNAL_H__
#define __BSP_LED_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep

/**@brief Firmware information define*/
#define LED_NUM_SIZE                          5

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_led_init(void);
int bsp_led_status_set(void *args);
void *bsp_led_status_get(void *args);

#ifdef __cplusplus
}
#endif

#endif 
