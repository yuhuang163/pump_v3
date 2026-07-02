#ifndef __GUIDE_CUSTOM_UI_INTERNAL_H__
#define __GUIDE_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

void guide_custom_ui_prepare(ui_manager_context_t *thiz);
void guide_custom_ui_remove(ui_manager_context_t *thiz);
int guide_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_CUSTOM_UI_INTERNAL_H__
