#ifndef __UI_POPUP_INTERNAL_H__
#define __UI_POPUP_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_popup.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define UI_POPUP_TEXT_COUNT                                        3
#define UI_POPUP_ICON_COUNT                                        2
#define UI_POPUP_STATUS_DISPLAY_STEP_TIME                          (100)
/// 圆环进度提前结束的步数（每步 UI_POPUP_STATUS_DISPLAY_STEP_TIME ms）
#define UI_POPUP_STATUS_ARC_RING_PROGRESS_LEAD_STEPS               (4)

typedef struct {
    lv_obj_t *obj;
} ui_popup_root_info_t;

typedef struct {
    lv_obj_t *label[UI_POPUP_TEXT_COUNT];
} ui_popup_label_info_t;

typedef struct {
    lv_obj_t *obj[UI_POPUP_ICON_COUNT];
} ui_popup_icon_info_t;

typedef struct {
    lv_obj_t *layer;
} ui_popup_mask_info_t;

typedef struct {
    lv_obj_t *obj;
    uint16_t ring_progress;
    uint16_t ring_progress_max;
} ui_popup_arc_info_t;

typedef struct {
    lv_obj_t *label;
    uint8_t shown_value;
    uint8_t max_value;
} ui_popup_countdown_info_t;

typedef struct {
    uint32_t start_time;
    uint16_t target_time;
    uint8_t force_update;
    uint8_t hold_countdown_active; /// 长按倒计时是否已启动（手动结束确认等）
} ui_popup_runtime_info_t;

typedef struct {
    ui_popup_root_info_t root;
    ui_popup_label_info_t label;
    ui_popup_icon_info_t icon;
    ui_popup_mask_info_t mask;
    ui_popup_arc_info_t arc;
    ui_popup_countdown_info_t countdown;
    ui_popup_runtime_info_t runtime;
} ui_popup_disp_info_t;

typedef struct {
    const char *name;
    int16_t x_offset;
    int16_t y_offset;
    const lv_font_t *font;
    uint16_t text_opa;
    uint32_t color;
    uint8_t line_space;
} ui_popup_text_info_t;

typedef struct {
    uint16_t size;
    uint8_t width;
    uint32_t bg_color;
    uint8_t bg_opa;
    uint32_t color;
    uint16_t loop_ms_per_deg;
    uint16_t angle_span;
    lv_align_t align;
    int16_t x_offset;
    int16_t y_offset;
} ui_popup_spin_arc_info_t;

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
ui_popup_disp_info_t *ui_popup_info_get(void);

void ui_popup_root_init(ui_manager_context_t *thiz, lv_obj_t *parent);
void ui_popup_components_hide_all(void);

void ui_popup_text_update(ui_manager_context_t *thiz, const ui_popup_text_info_t *text_info);
void ui_popup_icon_update(ui_manager_context_t *thiz, uint8_t index, uint16_t img_id,
    int16_t x_offset, uint16_t y_offset);
void ui_popup_single_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset);
void ui_popup_mask_layer_update(ui_manager_context_t *thiz);
void ui_popup_arc_update(ui_manager_context_t *thiz);
void ui_popup_spin_arc_update(ui_manager_context_t *thiz, const ui_popup_spin_arc_info_t *cfg);
void ui_popup_num_update(ui_manager_context_t *thiz, int16_t y_offset);
void ui_popup_tip_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const ui_popup_text_info_t *text_info, uint16_t img_id, uint16_t icon_y);
void ui_popup_arc_countdown_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const ui_popup_text_info_t *text_info);
void ui_popup_display_complete(ui_manager_context_t *thiz, uint16_t popup_type);

const char *ui_popup_name_get(ui_manager_context_t *thiz, uint16_t popup_type);
uint16_t ui_popup_target_time_get(popup_ui_id_e type);

void ui_popup_pump_runing_30m_tip_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type);

#ifdef __cplusplus
}
#endif

#endif // __UI_POPUP_INTERNAL_H__
