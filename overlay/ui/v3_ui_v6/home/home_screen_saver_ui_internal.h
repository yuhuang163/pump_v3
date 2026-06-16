#ifndef __HOME_SCREEN_SAVER_UI_INTERNAL_H__
#define __HOME_SCREEN_SAVER_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void home_screen_saver_base_ui_prepare(ui_manager_context_t *thiz);
void home_screen_saver_base_ui_remove(ui_manager_context_t *thiz);
int home_screen_saver_base_ui_process(ui_manager_context_t *thiz);

void home_screen_saver_custom_ui_prepare(ui_manager_context_t *thiz);
void home_screen_saver_custom_ui_remove(ui_manager_context_t *thiz);
int home_screen_saver_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __HOME_SCREEN_SAVER_UI_INTERNAL_H__

