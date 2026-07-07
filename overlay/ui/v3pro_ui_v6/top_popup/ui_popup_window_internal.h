#ifndef __UI_POPUP_WINDOW_INTERNAL_H__
#define __UI_POPUP_WINDOW_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_popup_window.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POPUP_WINDOW_TEXT_COUNT                                    2
#define POPUP_WINDOW_STATUS_DISPLAY_STEP_TIME                      (100)
/// 圆环进度提前结束的步数（每步 POPUP_WINDOW_STATUS_DISPLAY_STEP_TIME ms）
#define POPUP_WINDOW_STATUS_ARC_RING_PROGRESS_LEAD_STEPS           (4)

/// 支持窗口背景图片
#define IS_SUPPORT_POPUP_WINDOW_COMP_BG                            (0)

typedef struct {
    lv_obj_t *obj;
#if IS_SUPPORT_POPUP_WINDOW_COMP_BG
    lv_obj_t *comp_bg;
#endif
} popup_window_root_info_t;

typedef struct {
    lv_obj_t *label[POPUP_WINDOW_TEXT_COUNT];
} popup_window_label_info_t;

typedef struct {
    lv_obj_t *obj;
} popup_window_icon_info_t;

typedef struct {
    lv_obj_t *layer;
} popup_window_mask_info_t;

typedef struct {
    lv_obj_t *obj;
    uint16_t ring_progress;
    uint16_t ring_progress_max;
} popup_window_arc_info_t;

typedef struct {
    lv_obj_t *label;
    uint8_t shown_value;
    uint8_t max_value;
} popup_window_countdown_info_t;

typedef struct {
    uint32_t start_time;
    uint16_t target_time;
    uint8_t force_update;
} popup_window_runtime_info_t;

typedef struct {
    popup_window_root_info_t root;
    popup_window_label_info_t label;
    popup_window_icon_info_t icon;
    popup_window_mask_info_t mask;
    popup_window_arc_info_t arc;
    popup_window_countdown_info_t countdown;
    popup_window_runtime_info_t runtime;
} popup_window_disp_info_t;

typedef struct {
    const char *name;
    int16_t x_offset;
    int16_t y_offset;
    const lv_font_t *font;
    uint16_t text_opa;
    uint32_t color;
    uint8_t line_space;
} popup_window_text_info_t;

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
popup_window_disp_info_t *popup_window_info_get(void);

void popup_window_root_init(ui_manager_context_t *thiz);
#if IS_SUPPORT_POPUP_WINDOW_COMP_BG
void popup_window_comp_bg_update(ui_manager_context_t *thiz);
#endif
void popup_window_components_hide_all(void);
void popup_window_text_update(ui_manager_context_t *thiz, const popup_window_text_info_t *text_info);
void popup_window_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset);
void popup_window_mask_layer_update(ui_manager_context_t *thiz);
void popup_window_arc_update(ui_manager_context_t *thiz);
void popup_window_num_update(ui_manager_context_t *thiz, int16_t x_offset, int16_t y_offset);
void popup_window_tip_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const popup_window_text_info_t *text_info, uint16_t img_id, uint16_t icon_y);
void popup_window_static_tip_ui_update(ui_manager_context_t *thiz,
    const popup_window_text_info_t *text_info, uint16_t img_id, uint16_t icon_y);
void popup_window_arc_countdown_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const popup_window_text_info_t *text_info, int16_t x_offset, int16_t y_offset);
void popup_window_display_complete(ui_manager_context_t *thiz, uint16_t popup_type);

const char *popup_window_name_get(ui_manager_context_t *thiz, uint16_t popup_type);

void popup_window_ble_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type);
void popup_window_device_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type);
void popup_window_power_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type);
void popup_window_pump_stop_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type);
void popup_window_error_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type);

#ifdef __cplusplus
}
#endif

#endif // __UI_POPUP_WINDOW_INTERNAL_H__
