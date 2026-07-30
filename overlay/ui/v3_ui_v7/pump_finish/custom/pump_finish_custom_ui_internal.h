#ifndef __PUMP_FINISH_CUSTOM_UI_INTERNAL_H__
#define __PUMP_FINISH_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "pump_finish_ui.h" // IWYU pragma: keep

void pump_finish_custom_ui_prepare(ui_manager_context_t *thiz);
void pump_finish_custom_ui_remove(ui_manager_context_t *thiz);
int pump_finish_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_FINISH_CUSTOM_UI_INTERNAL_H__
