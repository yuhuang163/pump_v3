#ifndef __UI_PAGE_DEFINE_H__
#define __UI_PAGE_DEFINE_H__

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
/// 填充TLV数据
void ui_fill_ble_status(lcd_tlvs_t *tlv);
void ui_fill_battery_status(lcd_tlvs_t *tlv);
void ui_fill_charge_status(lcd_tlvs_t *tlv);
void ui_fill_img_id(lcd_tlvs_t *tlv, uint16_t img_id);

/// 电池页面
int ui_display_charge_screen(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);

/// 泵奶页面
int ui_display_pump_home_screen(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
int ui_display_pump_left_level(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
int ui_display_pump_right_level(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
int ui_display_pump_complete_tip(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
int ui_display_pump_mode(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
int ui_display_chasing_milk_mode(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
int ui_display_pump_speed(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
#ifdef __cplusplus
}
#endif

#endif 


