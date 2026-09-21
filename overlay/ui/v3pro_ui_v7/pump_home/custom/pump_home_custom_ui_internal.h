#ifndef __PUMP_HOME_CUSTOM_UI_INTERNAL_H__
#define __PUMP_HOME_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "../common/pump_home_common_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG              (0)

/// custom 主题泵奶首页扫光文案颜色（与 custom home 对齐）
#define PUMP_HOME_CUSTOM_HINT_COLOR_STIMULATION          (0xAD7C89)
#define PUMP_HOME_CUSTOM_HINT_COLOR_EXPRESSION           (0x547D83)
#define PUMP_HOME_CUSTOM_HINT_COLOR_PROGRAM              (0x8474C0)

/// custom 主题计时颜色（按背景）
#define PUMP_HOME_CUSTOM_TIME_COLOR_STIMULATION          (0xFF899B)
#define PUMP_HOME_CUSTOM_TIME_COLOR_EXPRESSION           (0x64A4F2)
#define PUMP_HOME_CUSTOM_TIME_COLOR_PROGRAM              (0x9C78E8)

void pump_home_custom_main_page_ui_build(pump_home_base_ui_info_t *info);
void pump_home_custom_main_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);
void pump_home_custom_main_page_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_custom_main_page_transition_apply(pump_home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning);

void pump_home_custom_ui_prepare(ui_manager_context_t *thiz);
void pump_home_custom_ui_remove(ui_manager_context_t *thiz);
int pump_home_custom_ui_process(ui_manager_context_t *thiz);
void pump_home_custom_ui_page_switch(pump_home_page_slide_dir_e dir);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_HOME_CUSTOM_UI_INTERNAL_H__
