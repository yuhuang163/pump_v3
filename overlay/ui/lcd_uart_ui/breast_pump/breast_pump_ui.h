#ifndef __BREAST_PUMP_UI_H__
#define __BREAST_PUMP_UI_H__

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
/// 泵奶主界面
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz);
void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz);
int ui_display_pump_home_screen(ui_manager_context_t *thiz);
/// 泵奶左档位调节界面
void ui_display_pump_level_prepare(ui_manager_context_t *thiz);
void ui_display_pump_level_remove(ui_manager_context_t *thiz);
int ui_display_pump_level(ui_manager_context_t *thiz);
/// 泵奶完成提示界面
void ui_display_pump_complete_tip_prepare(ui_manager_context_t *thiz);
void ui_display_pump_complete_tip_remove(ui_manager_context_t *thiz);
int ui_display_pump_complete_tip(ui_manager_context_t *thiz);
/// 泵奶模式界面
void ui_display_pump_mode_prepare(ui_manager_context_t *thiz);
void ui_display_pump_mode_remove(ui_manager_context_t *thiz);
int ui_display_pump_mode(ui_manager_context_t *thiz);
/// 追奶模式界面
void ui_display_breastfeeding_mode_prepare(ui_manager_context_t *thiz);
void ui_display_breastfeeding_mode_remove(ui_manager_context_t *thiz);
int ui_display_breastfeeding_mode(ui_manager_context_t *thiz);
/// 泵奶速度界面
void ui_display_pump_speed_prepare(ui_manager_context_t *thiz);
void ui_display_pump_speed_remove(ui_manager_context_t *thiz);
int ui_display_pump_speed(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

