#ifndef __SETTINGS_SUCCESS_BASE_UI_INTERNAL_H__
#define __SETTINGS_SUCCESS_BASE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void settings_success_base_ui_prepare(ui_manager_context_t *thiz, uint8_t style);
void settings_success_base_ui_remove(ui_manager_context_t *thiz);
int settings_success_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __SETTINGS_SUCCESS_BASE_UI_INTERNAL_H__
