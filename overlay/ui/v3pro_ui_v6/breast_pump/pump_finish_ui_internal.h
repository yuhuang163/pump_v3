#ifndef __PUMP_FINISH_UI_INTERNAL_H__
#define __PUMP_FINISH_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void pump_finish_base_ui_prepare(ui_manager_context_t *thiz);
void pump_finish_custom_ui_prepare(ui_manager_context_t *thiz);
void pump_finish_base_ui_remove(ui_manager_context_t *thiz);
void pump_finish_custom_ui_remove(ui_manager_context_t *thiz);
int pump_finish_base_ui_process(ui_manager_context_t *thiz);
int pump_finish_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_FINISH_UI_INTERNAL_H__
