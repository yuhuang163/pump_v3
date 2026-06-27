#ifndef __THEME_GUIDE_UI_H__
#define __THEME_GUIDE_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep

typedef enum {
    THEME_GUIDE_SELECT_SIMPLE = 0,
    THEME_GUIDE_SELECT_IP = 1,
    THEME_GUIDE_SELECT_MAX,
} theme_guide_select_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 主题引导页面
void ui_display_theme_guide_prepare(ui_manager_context_t *thiz);
void ui_display_theme_guide_remove(ui_manager_context_t *thiz);
int ui_display_theme_guide_process(ui_manager_context_t *thiz);

void theme_guide_ui_set_selected_index(uint8_t index);
uint8_t theme_guide_ui_get_selected_index(void);
res_theme_type_e theme_guide_ui_get_selected_theme_type(void);
void theme_guide_ui_knob_event(bool is_left);

#ifdef __cplusplus
}
#endif
#endif// __THEME_GUIDE_UI_H__
