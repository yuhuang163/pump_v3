#ifndef __BATTERY_UI_H__
#define __BATTERY_UI_H__

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
void battery_icon_init(ui_manager_context_t *thiz);
void battery_icon_deinit(ui_manager_context_t *thiz);
int battery_icon_update(ui_manager_context_t *thiz, lv_obj_t *root, uint8_t percent, uint8_t is_charging);

/// 充电中页面
void ui_display_charge_screen_prepare(ui_manager_context_t *thiz);
void ui_display_charge_screen_remove(ui_manager_context_t *thiz);
int ui_display_charge_screen_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

