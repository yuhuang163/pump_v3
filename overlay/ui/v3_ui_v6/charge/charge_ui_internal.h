#ifndef __CHARGE_UI_INTERNAL_H__
#define __CHARGE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void charge_base_ui_prepare(ui_manager_context_t *thiz);
void charge_base_ui_remove(ui_manager_context_t *thiz);
int charge_base_ui_process(ui_manager_context_t *thiz);

void charge_custom_ui_prepare(ui_manager_context_t *thiz);
void charge_custom_ui_remove(ui_manager_context_t *thiz);
int charge_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __CHARGE_UI_INTERNAL_H__

