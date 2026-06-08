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

/// 追奶模式界面
void ui_display_home_pump_bf_mode_prepare(ui_manager_context_t *thiz);
void ui_display_home_pump_bf_mode_remove(ui_manager_context_t *thiz);
int ui_display_home_pump_bf_mode_process(ui_manager_context_t *thiz);

/// 泵奶模式界面
void ui_display_home_pump_normal_mode_prepare(ui_manager_context_t *thiz);
void ui_display_home_pump_normal_mode_remove(ui_manager_context_t *thiz);
int ui_display_home_pump_normal_mode_process(ui_manager_context_t *thiz);

/// 泵奶档位调节界面
void ui_display_home_pump_level_prepare(ui_manager_context_t *thiz);
void ui_display_home_pump_level_remove(ui_manager_context_t *thiz);
int ui_display_home_pump_level_process(ui_manager_context_t *thiz);

/// 夜灯调节界面
void ui_display_home_night_light_adjust_prepare(ui_manager_context_t *thiz);
void ui_display_home_night_light_adjust_remove(ui_manager_context_t *thiz);
int ui_display_home_night_light_adjust_process(ui_manager_context_t *thiz);
   
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

