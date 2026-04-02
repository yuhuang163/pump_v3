#ifndef __POPUP_UI_H__
#define __POPUP_UI_H__

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

/// 低电量弹窗页面
void ui_display_low_power_popup_prepare(ui_manager_context_t *thiz);
void ui_display_low_power_popup_remove(ui_manager_context_t *thiz);
int ui_display_low_power_popup_process(ui_manager_context_t *thiz);

/// 关机弹窗页面
void ui_display_power_off_popup_prepare(ui_manager_context_t *thiz);
void ui_display_power_off_popup_remove(ui_manager_context_t *thiz);
int ui_display_power_off_popup_process(ui_manager_context_t *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

