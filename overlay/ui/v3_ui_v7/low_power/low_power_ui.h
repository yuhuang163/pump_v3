#ifndef __LOW_POWER_UI_H__
#define __LOW_POWER_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz, uint8_t show_countdown,
    const char *text);
void ui_display_battery_low_power_restart_countdown(void);
void ui_display_battery_low_power_remove(ui_manager_context_t *thiz);
int ui_display_battery_low_power_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __LOW_POWER_UI_H__
