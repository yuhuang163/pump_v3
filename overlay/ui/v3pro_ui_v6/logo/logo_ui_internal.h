#ifndef __LOGO_UI_INTERNAL_H__
#define __LOGO_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void logo_base_ui_prepare(ui_manager_context_t *thiz);
void logo_base_ui_remove(ui_manager_context_t *thiz);
int logo_base_ui_process(ui_manager_context_t *thiz);

void logo_custom_ui_prepare(ui_manager_context_t *thiz);
void logo_custom_ui_remove(ui_manager_context_t *thiz);
int logo_custom_ui_process(ui_manager_context_t *thiz);

void logo_test_ui_prepare(ui_manager_context_t *thiz);
void logo_test_ui_remove(ui_manager_context_t *thiz);
int logo_test_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __LOGO_UI_INTERNAL_H__

