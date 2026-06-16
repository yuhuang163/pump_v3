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
#include "ui_page_define.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep
#include "common_ui_png_decode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

typedef enum {
    UI_SNAP_SHOT_COMMON_BG = 0, /// 通用PNG背景
    UI_SNAP_SHOT_PUMP_LEVEL_BG,  /// 力度调节页面背景
    UI_SNAP_SHOT_REGULAR_PUMPING_BG,  /// 常规泵奶背景
    UI_SNAP_SHOT_PUMP_BOTH_LEVEL_BG,  /// 双侧泵奶档位调节背景
    UI_SNAP_SHOT_ID_MAX
} ui_snap_shot_id_e;

typedef struct {
    uint16_t img_id;
    uint8_t is_need_alpha;
} ui_snap_shot_info_t;

/*
* GLOBAL FUNCTION DECLARATION
*********************************************************************************
*/

/// 图片资源相关
void common_ui_img_resource_init(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info);
void common_ui_img_resource_init_with_theme(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info);
void common_ui_theme_img_resource_init(ui_manager_context_t *thiz, uint16_t id,
    res_theme_type_e theme_type, res_img_info_t *info);
void common_ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info);
void common_ui_text_place_in_circle(lv_obj_t * lab, uint16_t angle, uint16_t ring_r, uint8_t is_reverse);
void common_ui_text_place_in_circle_log(char *log, uint8_t is_left);
void common_ui_place_dot_on_arc(lv_obj_t * arc, lv_obj_t * dot, uint16_t angle_deg,
    uint16_t arc_w, uint16_t dot_d, float extra_inset, bool is_reverse);

/// 背景相关
void common_ui_bg_init(ui_manager_context_t *thiz);
void common_ui_bg_update(ui_manager_context_t *thiz, lv_obj_t *parent);
void common_ui_bg_deinit(ui_manager_context_t *thiz);

/// 通用操作提示界面
void ui_common_operation_tip_prepare(ui_manager_context_t *thiz, lv_obj_t *parent, popup_ui_id_e type);
void ui_common_operation_tip_remove(ui_manager_context_t *thiz);
void ui_common_operation_tip_process(ui_manager_context_t *thiz, popup_ui_id_e type);
void ui_common_operation_tip_update(ui_manager_context_t *thiz, 
    popup_ui_id_e type, lv_obj_t *parent, uint8_t *ui_index);

/// 泵奶档位调节界面
void common_pump_both_level_ui_prepare(ui_manager_context_t *thiz);
void common_pump_both_level_ui_remove(ui_manager_context_t *thiz);
int common_pump_both_level_ui_process(ui_manager_context_t *thiz, uint8_t *level,
    int8_t level_change, uint16_t popup_ui_type);

/// 泵奶档位调节界面
void common_pump_level_ui_prepare(ui_manager_context_t *thiz);
void common_pump_level_ui_remove(ui_manager_context_t *thiz);
int common_pump_level_ui_process(ui_manager_context_t *thiz, uint8_t level,
    int8_t level_change, bool is_left, uint8_t popup_ui_type);

/// 背景快照
lv_draw_buf_t *get_lvgl_snap_shot(ui_snap_shot_id_e snapshot_id);
void ui_snap_shot_perpare(struct ui_manager_context *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

