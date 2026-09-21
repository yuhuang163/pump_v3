#ifndef __PUMP_HOME_BASE_UI_INTERNAL_H__
#define __PUMP_HOME_BASE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../common/pump_home_common_ui.h" // IWYU pragma: keep

#define IS_SUPPORT_BASE_PUMP_HOME_COMP_BG                (0)

void pump_home_main_page_ui_build(pump_home_base_ui_info_t *info);
void pump_home_main_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);
void pump_home_main_page_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_main_page_transition_apply(pump_home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning);

void pump_home_base_ui_prepare(ui_manager_context_t *thiz);
void pump_home_base_ui_remove(ui_manager_context_t *thiz);
int pump_home_base_ui_process(ui_manager_context_t *thiz);
void pump_home_base_ui_page_switch(pump_home_page_slide_dir_e dir);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_HOME_BASE_UI_INTERNAL_H__
