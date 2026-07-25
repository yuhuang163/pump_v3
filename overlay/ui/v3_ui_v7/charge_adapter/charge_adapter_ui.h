#ifndef __CHARGE_ADAPTER_UI_H__
#define __CHARGE_ADAPTER_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void ui_display_charge_adapter_screen_prepare(ui_manager_context_t *thiz);
void ui_display_charge_adapter_screen_remove(ui_manager_context_t *thiz);
int ui_display_charge_adapter_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __CHARGE_ADAPTER_UI_H__
