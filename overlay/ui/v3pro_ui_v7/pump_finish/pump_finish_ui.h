#ifndef __PUMP_FINISH_UI_H__
#define __PUMP_FINISH_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

/// 泵奶完成提示界面
void ui_display_pump_complete_tip_prepare(ui_manager_context_t *thiz);
void ui_display_pump_complete_tip_remove(ui_manager_context_t *thiz);
int ui_display_pump_complete_tip_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_FINISH_UI_INTERNAL_H__

