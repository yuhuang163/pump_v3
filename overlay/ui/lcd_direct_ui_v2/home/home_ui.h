#ifndef __HOME_UI_H__
#define __HOME_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef enum {
    HOME_OPERATION_SUCCESS = 0,
    HOME_OPERATION_FAILED = 1,
} home_operation_type_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 开机LOGO页面
void ui_display_home_prepare(ui_manager_context_t *thiz);
void ui_display_home_remove(ui_manager_context_t *thiz);
int ui_display_home_process(ui_manager_context_t *thiz);

/// 主页面屏幕保护页面
void ui_display_home_screen_saver_prepare(ui_manager_context_t *thiz);
void ui_display_home_screen_saver_remove(ui_manager_context_t *thiz);
int ui_display_home_screen_saver_process(ui_manager_context_t *thiz);

/// 主页绑定
void ui_display_home_bind_prepare(ui_manager_context_t *thiz);
void ui_display_home_bind_remove(ui_manager_context_t *thiz);
int ui_display_home_bind_process(ui_manager_context_t *thiz);

/// 通用操作提示界面
void home_tip_popup_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc,
    lv_obj_t *parent, uint8_t *ui_index);
void home_tip_popup_ui_remove(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

