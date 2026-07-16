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
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_BASE_GUIDE_COMP_BG                   (0)
#define GUIDE_PAGE_INDEX_INVALID                        (0xFF)

#define IS_SUPPORT_BASE_GUIDE_LABEL_TEXT_CMP            (0) /// 支持label文本对比
#if IS_SUPPORT_BASE_GUIDE_LABEL_TEXT_CMP
#define GUIDE_BASE_LABEL_CARD_TEXT_COLOR                (0x00FF00)
#else
#define GUIDE_BASE_LABEL_CARD_TEXT_COLOR                (0xA0183B)
#endif

typedef enum {
    GUIDE_BASE_LEVEL_DISPLAY_DEFAULT = 0,
    GUIDE_BASE_LEVEL_DISPLAY_LEFT_ADJUST,
    GUIDE_BASE_LEVEL_DISPLAY_RIGHT_ADJUST,
    GUIDE_BASE_LEVEL_DISPLAY_BOTH_ADJUST,
} guide_base_level_display_type_e;

typedef struct {
    lv_obj_t *arc;
    lv_obj_t *indicator;
    lv_obj_t *progress;
    lv_obj_t *progress_bg;
    uint8_t value;
    uint8_t new_value;
    uint8_t adjust_active;
} guide_base_level_t;

typedef struct {
    guide_base_level_t left;
    guide_base_level_t right;
    lv_obj_t *separator;
    uint8_t left_blink_state;
    uint8_t right_blink_state;
} guide_base_level_info_t;

typedef struct {
    lv_obj_t *title;
    lv_obj_t *label;
    lv_obj_t *text;
    lv_obj_t *exit_text;
    lv_obj_t *return_icon;
    lv_obj_t *button[2];
    lv_obj_t *icon;
    uint8_t page_index;
    uint8_t force_update;
    uint8_t text_id;
    uint16_t button_img_id[2];
    uint16_t icon_img_id;
} guide_base_label_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *home_icon;
    lv_obj_t *title;
    lv_obj_t *hint;
    lv_obj_t *start_icon;
} guide_base_intro_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    lv_obj_t *hint;
    lv_obj_t *icon;
} guide_base_key_guide_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    lv_obj_t *hint;
    lv_obj_t *icon;
} guide_base_knob_guide_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *home_icon;
    lv_obj_t *title;
    lv_obj_t *desc;
    lv_obj_t *hint;
    lv_obj_t *start_icon;
} guide_base_function_home_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *step;
    lv_obj_t *title;
    lv_obj_t *card;
    lv_obj_t *text;
    lv_obj_t *button_icon;
    lv_obj_t *countdown_num;
    uint8_t page_index;
    uint8_t shown_countdown_sec;
} guide_base_level_guide_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *wave;
    lv_obj_t *select_titles[3];
    lv_obj_t *countdown_num;
    uint8_t shown_mode_index;
    uint8_t shown_countdown_sec;
    uint16_t shown_wave_res_id;
    uint16_t shown_label_res_id;
} guide_base_mode_select_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *wave;
    lv_obj_t *time;
    lv_obj_t *select_titles[3];
    lv_obj_t *countdown_num;
    uint8_t shown_mode_index;
    uint8_t shown_countdown_sec;
    uint16_t shown_wave_res_id;
    uint16_t shown_label_res_id;
    char shown_time_text[16];
} guide_base_rhythm_select_info_t;

#if IS_SUPPORT_BASE_GUIDE_WELCOME_ANIM
typedef struct {
    lv_obj_t *bg;
    lv_obj_t *frame;
    lv_obj_t *text;
    res_img_info_t frame_info;
    uint16_t shown_frame_res_id;
} guide_base_animation_info_t;
#endif

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *hint;
} guide_base_finish_info_t;

typedef struct {
    lv_obj_t *current;
    lv_obj_t *suffix;
} guide_base_page_num_info_t;

typedef struct {
    lv_obj_t *layer;
    lv_obj_t *bg;
} guide_base_level_theme_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *comp_bg;
    lv_obj_t *page;
    guide_base_level_theme_t level_theme;
#if IS_SUPPORT_BASE_GUIDE_WELCOME_ANIM
    guide_base_animation_info_t animation;
#endif
    guide_base_intro_info_t intro;
    guide_base_key_guide_info_t key_guide;
    guide_base_knob_guide_info_t knob_guide;
    guide_base_function_home_info_t function_home;
    guide_base_level_guide_info_t level_guide;
    guide_base_mode_select_info_t mode_select;
    guide_base_rhythm_select_info_t rhythm_select;
    guide_base_finish_info_t finish;
    guide_base_level_info_t level;
    guide_base_label_info_t label;
    guide_base_page_num_info_t page_num;
    ui_manager_context_t *ui;
    uint8_t force_update;
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t popup_ui_index;
    guide_base_level_display_type_e level_display_type;
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
guide_base_ui_info_t *guide_base_info_get(void);

bool guide_base_level_theme_page_is(uint8_t page_index);
bool guide_base_level_theme_is_active(const guide_base_ui_info_t *info);
void guide_base_level_theme_ensure(guide_base_ui_info_t *info);
void guide_base_level_theme_remove(guide_base_ui_info_t *info);
lv_obj_t *guide_base_level_theme_parent_get(const guide_base_ui_info_t *info);

void guide_base_blink_state_update(guide_base_ui_info_t *info, uint8_t page_index);

void guide_pump_animation_ui_remove(guide_base_ui_info_t *info);
void guide_pump_animation_ui_build(guide_base_ui_info_t *info);
void guide_pump_animation_ui_update(guide_base_ui_info_t *info, uint16_t frame_index);

void guide_intro_ui_remove(guide_base_ui_info_t *info);
void guide_intro_ui_build(guide_base_ui_info_t *info);

void guide_key_guide_ui_remove(guide_base_ui_info_t *info);
void guide_key_guide_ui_build(guide_base_ui_info_t *info);

void guide_knob_guide_ui_remove(guide_base_ui_info_t *info);
void guide_knob_guide_ui_build(guide_base_ui_info_t *info);

void guide_function_home_ui_remove(guide_base_ui_info_t *info);
void guide_function_home_ui_build(guide_base_ui_info_t *info);

void guide_label_level_header_create(guide_base_ui_info_t *info, lv_obj_t **title);
void guide_label_instruction_card_create(guide_base_ui_info_t *info, lv_obj_t **card,
    uint16_t res_id, int16_t x, int16_t y);
void guide_label_instruction_text_create(guide_base_ui_info_t *info, lv_obj_t **text,
    const char *content, int16_t y, uint8_t line_space);
void guide_label_inline_icon_create(guide_base_ui_info_t *info, lv_obj_t **icon,
    uint16_t res_id, int16_t x, int16_t y);

void guide_level_guide_ui_remove(guide_base_ui_info_t *info);
void guide_level_guide_ui_build(guide_base_ui_info_t *info, uint8_t page_index);
void guide_level_idle_countdown_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);
void guide_mode_select_guide_ui_build(guide_base_ui_info_t *info);
void guide_rhythm_select_guide_ui_build(guide_base_ui_info_t *info);
void guide_pump_stop_guide_ui_build(guide_base_ui_info_t *info);
void guide_page_num_ui_build(guide_base_ui_info_t *info, uint8_t page_index);
void guide_page_num_ui_set_visible(guide_base_ui_info_t *info, bool visible);

void guide_mode_select_ui_remove(guide_base_ui_info_t *info);
void guide_mode_select_ui_build(guide_base_ui_info_t *info);
void guide_mode_select_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);
void guide_mode_select_idle_countdown_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_rhythm_select_ui_remove(guide_base_ui_info_t *info);
void guide_rhythm_select_ui_build(guide_base_ui_info_t *info);
void guide_rhythm_select_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);
void guide_rhythm_select_idle_countdown_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc);

void guide_pump_level_ui_remove(guide_base_ui_info_t *info);
void guide_pump_level_ui_build(guide_base_ui_info_t *info);
void guide_pump_level_ui_update(guide_base_ui_info_t *info, bool blink_visible);

void guide_pump_label_ui_remove(guide_base_ui_info_t *info);
void guide_pump_label_ui_build(guide_base_ui_info_t *info, uint8_t page_index);

void guide_finish_ui_remove(guide_base_ui_info_t *info);
void guide_finish_ui_build(guide_base_ui_info_t *info);

void guide_page_ui_remove(guide_base_ui_info_t *info);
void guide_page_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc, uint16_t frame_index);

void guide_base_ui_prepare(ui_manager_context_t *thiz);
void guide_base_ui_remove(ui_manager_context_t *thiz);
int guide_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_BASE_UI_INTERNAL_H__
