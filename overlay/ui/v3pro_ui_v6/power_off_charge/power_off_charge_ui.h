#ifndef __POWER_OFF_CHARGE_UI_H__
#define __POWER_OFF_CHARGE_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void ui_display_power_off_charge_screen_prepare(ui_manager_context_t *thiz);
void ui_display_power_off_charge_screen_remove(ui_manager_context_t *thiz);
int ui_display_power_off_charge_screen_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __POWER_OFF_CHARGE_UI_H__
