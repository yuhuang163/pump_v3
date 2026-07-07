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
#include "ble_ui.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef enum {
    GUIDE_BASE_LEVEL_DISPLAY_DEFAULT = 0,
    GUIDE_BASE_LEVEL_DISPLAY_LEFT_ADJUST,
    GUIDE_BASE_LEVEL_DISPLAY_RIGHT_ADJUST,
    GUIDE_BASE_LEVEL_DISPLAY_BOTH_ADJUST,
} guide_base_level_display_type_e;

typedef enum {
    GUIDE_BASE_TEXT_DOUBLE_ADJUST = GUIDE_PAGE_PUMP_LEVEL_DOUBLE_ADJUST,
    GUIDE_BASE_TEXT_SINGLE_SELECT = GUIDE_PAGE_PUMP_LEVEL_SINGLE_SELECT,
    GUIDE_BASE_TEXT_LEFT_ADJUST = GUIDE_PAGE_PUMP_LEVEL_LEFT_ADJUST,
    GUIDE_BASE_TEXT_RIGHT_ADJUST = GUIDE_PAGE_PUMP_LEVEL_RIGHT_ADJUST,
    GUIDE_BASE_TEXT_MODE_SELECT = GUIDE_PAGE_PUMP_MODE_SELECT,
    GUIDE_BASE_TEXT_MODE_UNSELECT = GUIDE_PAGE_PUMP_MODE_UNSELECT,
    GUIDE_BASE_TEXT_MAX = GUIDE_PAGE_MAX,
} guide_base_text_id_e;

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
    uint8_t page_index;
    uint8_t force_update;
    uint8_t text_id;
    uint16_t button_img_id[2];
} guide_base_label_info_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    guide_base_level_info_t level;
    guide_base_label_info_t label;
    ui_manager_context_t *ui;
    uint8_t force_update;
    uint8_t page_index;
    guide_base_level_display_type_e level_display_type;
} guide_base_ui_info_t;

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
guide_base_ui_info_t *guide_base_info_get(void);

void guide_base_blink_state_update(guide_base_ui_info_t *info, uint8_t page_index);

void guide_pump_level_ui_remove(guide_base_ui_info_t *info);
void guide_pump_level_ui_update(guide_base_ui_info_t *info, bool blink_visible);

void guide_pump_label_ui_remove(guide_base_ui_info_t *info);
void guide_pump_label_ui_update(guide_base_ui_info_t *info, uint8_t page_index);

void guide_base_ui_prepare(ui_manager_context_t *thiz);
void guide_base_ui_remove(ui_manager_context_t *thiz);
int guide_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_BASE_UI_INTERNAL_H__
