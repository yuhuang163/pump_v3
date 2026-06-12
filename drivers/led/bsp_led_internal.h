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
void bsp_led_resume(void);
void bsp_led_suspend(void);
int bsp_led_status_set(led_status_msg_t *msg);
int bsp_led_level_max_update(led_level_msg_t *msg);

#if IS_SUPPORT_NIGHT_LIGHT
void bsp_night_light_init(void);
void bsp_night_light_resume(void);
void bsp_night_light_suspend(void);
int bsp_night_light_status_set(led_status_msg_t *msg);
int bsp_night_light_level_max_update(led_level_msg_t *msg);
#endif

int bsp_led_is_ready(void *args);

#ifdef __cplusplus
}
#endif

#endif 
