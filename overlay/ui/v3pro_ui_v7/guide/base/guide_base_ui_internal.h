#ifndef __GUIDE_BASE_UI_INTERNAL_H__
#define __GUIDE_BASE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_BASE_GUIDE_COMP_BG                   (0)
#define GUIDE_PAGE_INDEX_INVALID                        (0xFF)

typedef struct {
    lv_obj_t *qr_cont;
    lv_obj_t *qrcode;
    lv_obj_t *desc;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    char qr_str[120];
} guide_base_qrcode_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    uint16_t shown_bg_id;
} guide_base_intro_info_t;

typedef struct {
    lv_obj_t *current;
    lv_obj_t *total;
} guide_base_page_num_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    lv_obj_t *icon;
    lv_draw_buf_t *icon_buf;
    uint16_t shown_bg_id;
} guide_base_pump_start_guide_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *mode_title;
    lv_obj_t *time_m[2];
    lv_obj_t *time_s[2];
    lv_obj_t *time_colon;
    lv_obj_t *wave;
    res_img_info_t wave_info;
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;
    lv_obj_t *right_level;
    uint16_t shown_bg_id;
    uint16_t frame_res_id;
    uint16_t shown_frame_res_id;
    uint32_t last_frame_tick_ms;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t time_inited;
} guide_base_pump_start_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    lv_obj_t *icon;
    lv_draw_buf_t *icon_buf;
    uint16_t shown_bg_id;
} guide_base_pump_pause_guide_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *mode_title;
    lv_obj_t *wave;
    res_img_info_t wave_info;
    lv_obj_t *play_icon;
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;
    lv_obj_t *right_level;
    uint16_t shown_bg_id;
} guide_base_pump_pause_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    lv_obj_t *icon;
    lv_draw_buf_t *icon_buf;
    uint16_t shown_bg_id;
} guide_base_pump_finish_guide_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *mode_title;
    lv_obj_t *time_m[2];
    lv_obj_t *time_s[2];
    lv_obj_t *time_colon;
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;
    lv_obj_t *right_level;
    uint16_t shown_bg_id;
} guide_base_pump_home_info_t;

typedef guide_base_pump_home_info_t guide_base_mode_home_info_t;
typedef guide_base_pump_home_info_t guide_base_level_side_done_info_t;
typedef guide_base_pump_home_info_t guide_base_level_both_done_info_t;
typedef guide_base_pump_home_info_t guide_base_speed_done_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    lv_obj_t *icon;
    lv_draw_buf_t *icon_buf;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint16_t shown_bg_id;
} guide_base_mode_switch_guide_info_t;

typedef struct {
    lv_obj_t *circle_bg;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *duration;
    uint8_t slot;
    uint8_t mode;
    uint16_t icon_id;
    uint8_t icon_predecoded;
} guide_base_mode_switch_card_t;

typedef struct {
    lv_obj_t *page;
    lv_obj_t *peer_page;
    lv_obj_t *dots[GUIDE_MODE_SWITCH_SLOT_COUNT];
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint8_t slot_index;
    uint8_t shown_slot_index;
    uint8_t selected_dot;
    uint8_t transitioning;
    uint8_t slide_dir;
    uint8_t ready;
    int16_t progress;
    uint32_t start_ms;
} guide_base_mode_switch_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *press;
    lv_obj_t *or_text;
    lv_obj_t *adjust;
    lv_obj_t *left_icon;
    lv_obj_t *right_icon;
    lv_draw_buf_t *left_icon_buf;
    lv_draw_buf_t *right_icon_buf;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint16_t shown_bg_id;
} guide_base_level_side_guide_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *press;
    lv_obj_t *and_text;
    lv_obj_t *adjust;
    lv_obj_t *left_icon;
    lv_obj_t *right_icon;
    lv_draw_buf_t *left_icon_buf;
    lv_draw_buf_t *right_icon_buf;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint16_t shown_bg_id;
} guide_base_level_both_guide_info_t;

typedef struct {
    lv_obj_t *done_arc;
    lv_obj_t *remain_arc;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint16_t anim_visual_sweep;
    uint32_t anim_start_time;
    uint8_t anim_to_level;
} guide_base_level_both_side_arc_t;

typedef struct {
    guide_base_level_both_side_arc_t side[2];
    lv_obj_t *value_box[2];
    lv_obj_t *value_label[2];
    lv_obj_t *dir_box[2];
    lv_obj_t *dir_label[2];
    lv_obj_t *over_limit_box;
    lv_obj_t *over_limit_label;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint8_t shown_level[2];
    uint8_t shown_over_limit;
    uint8_t level_max;
    uint32_t accent_color;
    uint32_t shown_accent_color;
    uint32_t shown_over_limit_color;
} guide_base_level_both_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *press;
    lv_obj_t *adjust;
    lv_obj_t *icon;
    lv_draw_buf_t *icon_buf;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint16_t shown_bg_id;
} guide_base_speed_guide_info_t;

typedef struct {
    lv_obj_t *arcs[5];
    lv_obj_t *value_label;
    lv_obj_t *speed_label;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint8_t shown_speed;
    uint8_t shown_selected;
    uint32_t accent_color;
    uint32_t shown_accent_color;
} guide_base_speed_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *qr_cont;
    lv_obj_t *qrcode;
    lv_obj_t *desc;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint16_t shown_bg_id;
    char qr_str[120];
} guide_base_finish_info_t;

typedef struct {
    ui_progress_arc_info_t arc;
    lv_obj_t *value_label;
    lv_obj_t *dir_label;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint8_t shown_level;
    uint8_t shown_side;
    uint8_t shown_over_limit;
    uint8_t anim_to_level;
    uint8_t level_max;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint32_t anim_start_time;
    uint32_t accent_color;
    uint32_t shown_dir_color;
} guide_base_level_side_info_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *comp_bg;
    lv_obj_t *page;
    guide_base_qrcode_info_t qrcode;
    guide_base_intro_info_t intro;
    guide_base_page_num_info_t page_num;
    guide_base_pump_start_guide_info_t pump_start_guide;
    guide_base_pump_start_info_t pump_start;
    guide_base_pump_pause_guide_info_t pump_pause_guide;
    guide_base_pump_pause_info_t pump_pause;
    guide_base_pump_finish_guide_info_t pump_finish_guide;
    guide_base_pump_home_info_t pump_home;
    guide_base_mode_home_info_t mode_home;
    guide_base_mode_switch_guide_info_t mode_switch_guide;
    guide_base_mode_switch_info_t mode_switch;
    guide_base_level_side_guide_info_t level_side_guide;
    guide_base_level_side_info_t level_side;
    guide_base_level_side_done_info_t level_side_done;
    guide_base_level_both_guide_info_t level_both_guide;
    guide_base_level_both_info_t level_both;
    guide_base_level_both_done_info_t level_both_done;
    guide_base_speed_guide_info_t speed_guide;
    guide_base_speed_info_t speed;
    guide_base_speed_done_info_t speed_done;
    guide_base_finish_info_t finish;
    ui_manager_context_t *ui;
    uint8_t force_update;
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t popup_ui_index;
    uint8_t stop_hold_active;
} guide_base_ui_info_t;

static inline lv_obj_t *guide_page_parent_get(const guide_base_ui_info_t *info)
{
    if (info != NULL && info->page != NULL && lv_obj_is_valid(info->page)) {
        return info->page;
    }
    return info != NULL ? info->root : NULL;
}

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
void guide_qrcode_ui_remove(guide_base_ui_info_t *info);
void guide_qrcode_ui_build(guide_base_ui_info_t *info);
void guide_qrcode_ui_update(guide_base_ui_info_t *info);

void guide_intro_ui_remove(guide_base_ui_info_t *info);
void guide_intro_ui_build(guide_base_ui_info_t *info);

void guide_page_num_ui_remove(guide_base_page_num_info_t *page_num);
void guide_page_num_ui_build(guide_base_page_num_info_t *page_num, lv_obj_t *parent,
    uint8_t current, uint8_t total);

void guide_pump_start_guide_ui_remove(guide_base_ui_info_t *info);
void guide_pump_start_guide_ui_build(guide_base_ui_info_t *info);

void guide_pump_start_ui_remove(guide_base_ui_info_t *info);
void guide_pump_start_ui_build(guide_base_ui_info_t *info);
void guide_pump_start_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_pump_pause_guide_ui_remove(guide_base_ui_info_t *info);
void guide_pump_pause_guide_ui_build(guide_base_ui_info_t *info);

void guide_pump_pause_ui_remove(guide_base_ui_info_t *info);
void guide_pump_pause_ui_build(guide_base_ui_info_t *info);

void guide_pump_finish_guide_ui_remove(guide_base_ui_info_t *info);
void guide_pump_finish_guide_ui_build(guide_base_ui_info_t *info);

void guide_pump_finish_hold_ui_remove(guide_base_ui_info_t *info);
void guide_pump_finish_hold_ui_build(guide_base_ui_info_t *info);
void guide_pump_finish_hold_ui_update(guide_base_ui_info_t *info);

void guide_pump_home_ui_remove(guide_base_ui_info_t *info);
void guide_pump_home_ui_build(guide_base_ui_info_t *info);
void guide_pump_home_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_mode_home_ui_remove(guide_base_ui_info_t *info);
void guide_mode_home_ui_build(guide_base_ui_info_t *info);
void guide_mode_home_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_mode_switch_guide_ui_remove(guide_base_ui_info_t *info);
void guide_mode_switch_guide_ui_build(guide_base_ui_info_t *info);
void guide_mode_switch_guide_ui_update(guide_base_ui_info_t *info);

void guide_mode_switch_ui_remove(guide_base_ui_info_t *info);
void guide_mode_switch_ui_build(guide_base_ui_info_t *info);
void guide_mode_switch_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_level_side_guide_ui_remove(guide_base_ui_info_t *info);
void guide_level_side_guide_ui_build(guide_base_ui_info_t *info);
void guide_level_side_guide_ui_update(guide_base_ui_info_t *info);

void guide_level_side_ui_remove(guide_base_ui_info_t *info);
void guide_level_side_ui_build(guide_base_ui_info_t *info);
void guide_level_side_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_level_side_done_ui_remove(guide_base_ui_info_t *info);
void guide_level_side_done_ui_build(guide_base_ui_info_t *info);
void guide_level_side_done_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_level_both_guide_ui_remove(guide_base_ui_info_t *info);
void guide_level_both_guide_ui_build(guide_base_ui_info_t *info);
void guide_level_both_guide_ui_update(guide_base_ui_info_t *info);

void guide_level_both_ui_remove(guide_base_ui_info_t *info);
void guide_level_both_ui_build(guide_base_ui_info_t *info);
void guide_level_both_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_level_both_done_ui_remove(guide_base_ui_info_t *info);
void guide_level_both_done_ui_build(guide_base_ui_info_t *info);
void guide_level_both_done_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_speed_guide_ui_remove(guide_base_ui_info_t *info);
void guide_speed_guide_ui_build(guide_base_ui_info_t *info);
void guide_speed_guide_ui_update(guide_base_ui_info_t *info);

void guide_speed_ui_remove(guide_base_ui_info_t *info);
void guide_speed_ui_build(guide_base_ui_info_t *info);
void guide_speed_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_speed_done_ui_remove(guide_base_ui_info_t *info);
void guide_speed_done_ui_build(guide_base_ui_info_t *info);
void guide_speed_done_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_finish_ui_remove(guide_base_ui_info_t *info);
void guide_finish_ui_build(guide_base_ui_info_t *info);
void guide_finish_ui_update(guide_base_ui_info_t *info);

void guide_page_ui_remove(guide_base_ui_info_t *info);
void guide_page_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc, uint16_t frame_index);

void guide_base_ui_prepare(ui_manager_context_t *thiz);
void guide_base_ui_remove(ui_manager_context_t *thiz);
int guide_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_BASE_UI_INTERNAL_H__
