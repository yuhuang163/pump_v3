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
/// 电池
void battery_ui_init(ui_manager_context_t *thiz);
void battery_ui_deinit(ui_manager_context_t *thiz);
int battery_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, 
    battery_manager_context_t *battery, uint8_t is_display_state);

/// 低电量页面
void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz);
void ui_display_battery_low_power_remove(ui_manager_context_t *thiz);
int ui_display_battery_low_power_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

