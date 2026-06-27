#ifndef __LOW_POWER_BASE_UI_INTERNAL_H__
#define __LOW_POWER_BASE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void low_power_base_ui_prepare(ui_manager_context_t *thiz);
void low_power_base_ui_remove(ui_manager_context_t *thiz);
int low_power_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __LOW_POWER_BASE_UI_INTERNAL_H__
