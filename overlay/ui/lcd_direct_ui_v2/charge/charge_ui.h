#ifndef __CHARGE_UI_H__
#define __CHARGE_UI_H__

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
/// 充电中页面
void ui_display_charge_screen_prepare(ui_manager_context_t *thiz);
void ui_display_charge_screen_remove(ui_manager_context_t *thiz);
int ui_display_charge_screen_process(ui_manager_context_t *thiz);

/// 充电适配器页面
void ui_display_charge_adapter_screen_prepare(ui_manager_context_t *thiz);
void ui_display_charge_adapter_screen_remove(ui_manager_context_t *thiz);
int ui_display_charge_adapter_process(ui_manager_context_t *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

