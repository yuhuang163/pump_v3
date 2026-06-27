#ifndef __BATTERY_CUSTOM_UI_INTERNAL_H__
#define __BATTERY_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "battery_manager_api.h" // IWYU pragma: keep

void battery_custom_ui_init(ui_manager_context_t *thiz);
void battery_custom_ui_deinit(ui_manager_context_t *thiz);
int battery_custom_ui_update(ui_manager_context_t *thiz, lv_obj_t *root,
    battery_manager_context_t *battery, uint8_t is_display_state);

#ifdef __cplusplus
}
#endif

#endif // __BATTERY_CUSTOM_UI_INTERNAL_H__
