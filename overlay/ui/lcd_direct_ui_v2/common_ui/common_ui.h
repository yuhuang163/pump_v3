#ifndef __COMMON_UI_H__
#define __COMMON_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void common_ui_img_resource_init(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info);
void common_ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info);
void common_ui_text_place_in_circle(lv_obj_t * lab, uint16_t angle, uint16_t ring_r, uint8_t is_reverse);
void common_ui_text_place_in_circle_log(char *log, uint8_t is_left);
void common_ui_place_dot_on_arc(lv_obj_t * arc, lv_obj_t * dot, uint16_t angle_deg,
    uint16_t arc_w, uint16_t dot_d, int16_t extra_inset, bool is_reverse);

/// 背景相关
void common_ui_bg_init(ui_manager_context_t *thiz);
void common_ui_bg_update(ui_manager_context_t *thiz, lv_obj_t *parent);
void common_ui_bg_deinit(ui_manager_context_t *thiz);
/// 泵速度转换为UI索引
uint8_t pump_speed_convert_to_ui_speed_idx(uint8_t speed);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

