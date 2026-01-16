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
void ui_fill_battery_status(lcd_tlvs_t *tlv);
void ui_fill_charge_status(lcd_tlvs_t *tlv);

/// 充电中页面
void ui_display_charge_screen_prepare(ui_manager_context_t *thiz);
void ui_display_charge_screen_remove(ui_manager_context_t *thiz);
int ui_display_charge_screen(ui_manager_context_t *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

