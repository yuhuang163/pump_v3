#ifndef __BATTERY_LOW_POWER_UI_INTERNAL_H__
#define __BATTERY_LOW_POWER_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void battery_low_power_base_ui_prepare(ui_manager_context_t *thiz);
void battery_low_power_custom_ui_prepare(ui_manager_context_t *thiz);
void battery_low_power_base_ui_remove(ui_manager_context_t *thiz);
void battery_low_power_custom_ui_remove(ui_manager_context_t *thiz);
int battery_low_power_base_ui_process(ui_manager_context_t *thiz);
int battery_low_power_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __BATTERY_LOW_POWER_UI_INTERNAL_H__
