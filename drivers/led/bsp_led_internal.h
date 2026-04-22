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
int bsp_led_level_rate_update(led_level_rate_msg_t *msg);
/** 全键背光统一亮度：BLINK 灯保留模式并按当前相位刷新；非 BLINK 灯切换到 LEVEL */
void bsp_led_key_backlight_uniform_level(uint8_t level);
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
/** 仅更新光感目标占空比，由 LED 线程渐变逼近，避免亮度跳变 */
void bsp_led_key_adaptive_duty_set(uint8_t duty);
/** 当前/目标立即设为 duty 并刷新硬件（关闭自适应、手动档位等） */
void bsp_led_key_adaptive_duty_snap(uint8_t duty);
#endif

void bsp_night_light_init(void);
void bsp_night_light_resume(void);
void bsp_night_light_suspend(void);
int bsp_night_light_status_set(led_status_msg_t *msg);
int bsp_night_light_rate_update(led_level_rate_msg_t *msg);

int bsp_led_is_ready(void *args);

#ifdef __cplusplus
}
#endif

#endif 
