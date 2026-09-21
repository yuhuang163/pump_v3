#ifndef __HOME_BASE_UI_INTERNAL_H__
#define __HOME_BASE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../common/home_common_ui.h" // IWYU pragma: keep

#define IS_SUPPORT_BASE_HOME_COMP_BG                 (0)

void home_main_page_ui_build(home_base_ui_info_t *info);
void home_main_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
void home_main_page_ui_remove(home_base_ui_info_t *info);
void home_main_page_transition_apply(home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning);

void home_base_ui_prepare(ui_manager_context_t *thiz);
void home_base_ui_remove(ui_manager_context_t *thiz);
int home_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __HOME_BASE_UI_INTERNAL_H__
