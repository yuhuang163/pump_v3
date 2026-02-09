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
#include "battery_manager_api.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void battery_icon_init(ui_manager_context_t *thiz);
int battery_icon_update(ui_manager_context_t *thiz, lv_obj_t *root, battery_manager_context_t *battery);
void battery_icon_deinit(ui_manager_context_t *thiz);

/// 低电量提示页面
void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz);
void ui_display_battery_low_power_remove(ui_manager_context_t *thiz);
int ui_display_battery_low_power_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

