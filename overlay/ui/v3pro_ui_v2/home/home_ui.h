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
    HOME_UI_DISPLAY_DEFAULT = 0,       /// 默认 UI
    HOME_UI_DISPLAY_CLOCK_AND_RECORD,  /// 显示时钟和记录 UI
    HOME_UI_DISPLAY_CLOCK,             /// 显示时钟 UI
    HOME_UI_DISPLAY_RECORD,            /// 显示记录 UI
    HOME_UI_DISPLAY_CLOCK_ADJUST_TIP,  /// 显示时钟校准提示
    HOME_UI_DISPLAY_CLOCK_LIGHT_TIP,   /// 显示时钟夜灯提示
    HOME_UI_DISPLAY_MAX
} home_ui_display_type_e;

typedef enum {
    HOME_SCREEN_SAVER_DISPLAY_DEFAULT = 0,       /// 默认 UI
    HOME_SCREEN_SAVER_DISPLAY_CLOCK,             /// 显示时钟 UI
    HOME_SCREEN_SAVER_DISPLAY_MAX
} home_screen_saver_display_type_e;

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

/// 泵奶速度界面
void ui_display_home_pump_speed_prepare(ui_manager_context_t *thiz);
void ui_display_home_pump_speed_remove(ui_manager_context_t *thiz);
int ui_display_home_pump_speed_process(ui_manager_context_t *thiz);

/// 泵奶档位调节界面
void ui_display_home_pump_level_prepare(ui_manager_context_t *thiz);
void ui_display_home_pump_level_remove(ui_manager_context_t *thiz);
int ui_display_home_pump_level_process(ui_manager_context_t *thiz);
   
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

