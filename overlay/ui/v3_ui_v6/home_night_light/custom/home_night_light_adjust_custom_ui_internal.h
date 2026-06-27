#ifndef __HOME_NIGHT_LIGHT_ADJUST_CUSTOM_UI_INTERNAL_H__
#define __HOME_NIGHT_LIGHT_ADJUST_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void home_night_light_adjust_custom_ui_prepare(ui_manager_context_t *thiz);
void home_night_light_adjust_custom_ui_remove(ui_manager_context_t *thiz);
int home_night_light_adjust_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __HOME_NIGHT_LIGHT_ADJUST_CUSTOM_UI_INTERNAL_H__
