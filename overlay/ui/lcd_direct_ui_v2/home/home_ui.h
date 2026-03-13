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

/// 主页二维码
void ui_display_home_qrcode_prepare(ui_manager_context_t *thiz);
void ui_display_home_qrcode_remove(ui_manager_context_t *thiz);
int ui_display_home_qrcode_process(ui_manager_context_t *thiz);

/// 关机倒计时页面
void ui_prompt_home_will_power_off_prepare(ui_manager_context_t *thiz, lv_obj_t *parent);
void ui_prompt_home_will_power_off_remove(ui_manager_context_t *thiz);
void ui_prompt_home_will_power_off_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

