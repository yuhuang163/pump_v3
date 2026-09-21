#ifndef __HOME_CUSTOM_UI_INTERNAL_H__
#define __HOME_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../common/home_common_ui.h" // IWYU pragma: keep

#define IS_SUPPORT_HOME_CUSTOM_MODE_CMP              (0)
#define IS_SUPPORT_CUSTOM_HOME_COMP_BG               (0)

/// custom 主题首页扫光文案颜色（按模式，对齐 IP 背景色相）
#define HOME_CUSTOM_HINT_COLOR_STIMULATION           (0xAD7C89)
#define HOME_CUSTOM_HINT_COLOR_EXPRESSION            (0x547D83)
#define HOME_CUSTOM_HINT_COLOR_PROGRAM               (0x8474C0)

void home_custom_main_page_ui_build(home_base_ui_info_t *info);
void home_custom_main_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
void home_custom_main_page_ui_remove(home_base_ui_info_t *info);
void home_custom_main_page_transition_apply(home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning);

void home_custom_ui_prepare(ui_manager_context_t *thiz);
void home_custom_ui_remove(ui_manager_context_t *thiz);
int home_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __HOME_CUSTOM_UI_INTERNAL_H__
