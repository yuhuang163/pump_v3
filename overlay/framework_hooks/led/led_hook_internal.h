#ifndef __LED_HOOK_INTERNAL_H__
#define __LED_HOOK_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_api.h" // IWYU pragma: keep
#include "led_manager.h" // IWYU pragma: keep
#if IS_SUPPORT_ADAPTIVE_DIMMING
#include "adaptive_dimming.h" // IWYU pragma: keep
#endif

#define FORBID_START_BRIGHTNESS_MAX_VALUE                 (40)
#define LOW_POWER_RUN_BRIGHTNESS_MAX_VALUE                (60)

void led_hook_notify_state(led_manager_context_t *thiz, led_broadcast_id_e evt);

void led_key_backlight_bind(led_manager_context_t *ctx);
void key_backlight_on_post_init(led_manager_context_t *thiz);
void key_light_adaptive_thread(void *user_data);
void set_key_backlight_level_rate(led_manager_context_t *thiz, uint8_t rate);
void key_backlight_on_battery_state_change(led_manager_context_t *thiz, battery_state_info_t *info);
void key_backlight_on_broadcast(led_manager_context_t *thiz, broadcast_mask_id_t id, int arg1);
#if IS_SUPPORT_ADAPTIVE_DIMMING
void panel_backlight_apply_app_rate(uint8_t rate);
void led_key_backlight_als_init(void);
void led_key_backlight_als_rebuild_knots(void);
void led_key_backlight_als_on_lux(uint16_t lux);
void led_key_backlight_als_apply(led_manager_context_t *led);
uint8_t led_key_backlight_als_duty_from_lux(uint16_t lux);
#endif

void led_ui_backlight_bind(led_manager_context_t *ctx);
void ui_backlight_on_post_init(led_manager_context_t *thiz);
void ui_backlight_on_battery_state_change(led_manager_context_t *thiz, battery_state_info_t *info);
void ui_backlight_on_broadcast(led_manager_context_t *thiz, broadcast_mask_id_t id, int arg1);
#if IS_SUPPORT_ADAPTIVE_DIMMING
void ui_backlight_adaptive_thread(void *user_data);
void led_ui_backlight_als_init(void);
void led_ui_backlight_als_on_lux(uint16_t lux);
void led_ui_backlight_als_apply(led_manager_context_t *led);
#endif

#if IS_SUPPORT_NIGHT_LIGHT
void led_night_light_bind(led_manager_context_t *ctx);
int night_light_on_post_init(led_manager_context_t *thiz);
void night_light_on_battery_state_change(led_manager_context_t *thiz, battery_state_info_t *info);
#if IS_SUPPORT_ADAPTIVE_DIMMING
void led_night_light_als_init(void);
void led_night_light_als_rebuild_knots(void);
void led_night_light_als_on_lux(uint16_t lux);
void led_night_light_als_apply(led_manager_context_t *led);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
