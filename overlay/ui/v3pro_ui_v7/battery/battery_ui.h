#ifndef __BATTERY_UI_H__
#define __BATTERY_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "battery_manager_api.h" // IWYU pragma: keep

#define BATTERY_UI_SCREEN_W                                (PANLE_DISPLAY_W)

void battery_ui_init(ui_manager_context_t *thiz);
void battery_ui_deinit(ui_manager_context_t *thiz);
/** 相对系统主题的垂直偏移（负值上移）；deinit 时复位 */
void battery_ui_y_offset_set(int16_t y_offset);
int battery_ui_update(ui_manager_context_t *thiz, lv_obj_t *root,
    battery_manager_context_t *battery, uint8_t is_display_state, uint8_t is_ble_connected);
uint16_t battery_ui_ble_x_get(uint16_t default_x);
uint16_t battery_ui_ble_y_get(uint16_t default_y);

#ifdef __cplusplus
}
#endif

#endif // __BATTERY_UI_H__
