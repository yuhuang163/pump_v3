#ifndef __AIRPLANE_MODE_UI_INTERNAL_H__
#define __AIRPLANE_MODE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void airplane_mode_base_ui_prepare(ui_manager_context_t *thiz);
void airplane_mode_custom_ui_prepare(ui_manager_context_t *thiz);
void airplane_mode_base_ui_remove(ui_manager_context_t *thiz);
void airplane_mode_custom_ui_remove(ui_manager_context_t *thiz);
int airplane_mode_base_ui_process(ui_manager_context_t *thiz);
int airplane_mode_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __AIRPLANE_MODE_UI_INTERNAL_H__
