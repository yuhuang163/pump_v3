/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_LEVEL_MAX                             (15)
#define PUMP_LEVEL_RING_SIZE                       (320)

#define PUMP_LEFT_ARC_SIZE                         (47)
#define PUMP_LEFT_ANGLE_OFFSET                     (11)
#define PUMP_LEFT_ARC_START_ANGLE                  (90 + PUMP_LEFT_ANGLE_OFFSET)
#define PUMP_LEFT_ARC_END_ANGLE                    (PUMP_LEFT_ARC_START_ANGLE + PUMP_LEFT_ARC_SIZE)
#define PUMP_RIGHT_ARC_END_ANGLE                   (450 - PUMP_LEFT_ANGLE_OFFSET)
#define PUMP_RIGHT_ARC_START_ANGLE                 (PUMP_RIGHT_ARC_END_ANGLE - PUMP_LEFT_ARC_SIZE)
#define PUMP_LEVEL_ARC_WIDTH                       (8)
#define PUMP_LEVEL_INDICATOR_SIZE                  (16)
#define PUMP_LEVEL_ARC_BG_COLOR                    (0xE5C1C5)
#define PUMP_LEVEL_ARC_ACTIVE_COLOR                (0xBD4060)
#define PUMP_LEVEL_TEXT_COLOR                      (0xBD4060)
#define PUMP_LEFT_LEVEL_BG_X                       (18)
#define PUMP_LEFT_LEVEL_BG_Y                       (208)
#define PUMP_RIGHT_LEVEL_BG_X                      (306)
#define PUMP_RIGHT_LEVEL_BG_Y                      (PUMP_LEFT_LEVEL_BG_Y)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void home_left_level_indicator_update(home_base_ui_info_t *info, uint16_t end_angle)
{
    home_base_level_t *level = &info->level.left;

    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        level->indicator = lv_image_create(level->arc);
        EXCEPTION_CHECK_VOID(level->indicator);
        ui_img_baj_theme_set_src(info->ui, level->indicator, BASE_RES_ID_PUMP_LEVEL_INDICATOR,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    ui_place_dot_on_arc(level->arc, level->indicator, end_angle,
        PUMP_LEVEL_ARC_WIDTH, PUMP_LEVEL_INDICATOR_SIZE, 0, false);
}

static void home_left_level_progress_update(home_base_ui_info_t *info, uint8_t level_val, uint16_t end_angle)
{
    char level_text[4] = {0};
    home_base_level_t *level = &info->level.left;
    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        level->progress_bg = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(level->progress_bg);
        ui_img_baj_theme_set_src(info->ui, level->progress_bg, BASE_RES_ID_PUMP_LEVEL_BG,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(level->progress_bg, LV_ALIGN_TOP_LEFT, PUMP_LEFT_LEVEL_BG_X, PUMP_LEFT_LEVEL_BG_Y);
    }
    if (level->progress == NULL || !lv_obj_is_valid(level->progress)) {
        level->progress = lv_label_create(level->progress_bg);
        EXCEPTION_CHECK_VOID(level->progress);
        lv_obj_set_style_text_color(level->progress, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level->progress, &lv_font_aeonik_soft_20_semibold, 0);
        lv_obj_align(level->progress, LV_ALIGN_CENTER, -1, 0);
    }
    snprintf(level_text, sizeof(level_text), "%d", level_val);
    lv_label_set_text(level->progress, level_text);
}

static void home_left_level_arc_update(home_base_ui_info_t *info, uint16_t end_angle)
{
    home_base_level_t *level = &info->level.left;
    if (level->arc == NULL || !lv_obj_is_valid(level->arc)) {
        level->arc = lv_arc_create(info->root);
        EXCEPTION_CHECK_VOID(level->arc);
        lv_obj_set_size(level->arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE);
        lv_obj_center(level->arc);
        lv_arc_set_bg_angles(level->arc, PUMP_LEFT_ARC_START_ANGLE, PUMP_LEFT_ARC_END_ANGLE);
        lv_arc_set_angles(level->arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_BG_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_width(level->arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(level->arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(level->arc, NULL, LV_PART_KNOB);
    } else {
        lv_arc_set_angles(level->arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);
    }
}

static void home_right_level_indicator_update(home_base_ui_info_t *info, uint16_t end_angle)
{
    home_base_level_t *level = &info->level.right;
    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        level->indicator = lv_image_create(level->arc);
        EXCEPTION_CHECK_VOID(level->indicator);
        ui_img_baj_theme_set_src(info->ui, level->indicator, BASE_RES_ID_PUMP_LEVEL_INDICATOR,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    ui_place_dot_on_arc(level->arc, level->indicator, end_angle,
        PUMP_LEVEL_ARC_WIDTH, PUMP_LEVEL_INDICATOR_SIZE, -0.5f, false);
}

static void home_right_level_progress_update(home_base_ui_info_t *info, uint8_t level_val, uint16_t end_angle)
{
    char level_text[4] = {0};
    home_base_level_t *level = &info->level.right;

    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        level->progress_bg = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(level->progress_bg);
        ui_img_baj_theme_set_src(info->ui, level->progress_bg, BASE_RES_ID_PUMP_LEVEL_BG,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(level->progress_bg, LV_ALIGN_TOP_LEFT, PUMP_RIGHT_LEVEL_BG_X, PUMP_RIGHT_LEVEL_BG_Y);
    }
    if (level->progress == NULL || !lv_obj_is_valid(level->progress)) {
        level->progress = lv_label_create(level->progress_bg);
        EXCEPTION_CHECK_VOID(level->progress);
        lv_obj_set_style_text_color(level->progress, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level->progress, &lv_font_aeonik_soft_20_semibold, 0);
        lv_obj_align(level->progress, LV_ALIGN_CENTER, -1, 0);
    }
    snprintf(level_text, sizeof(level_text), "%d", level_val);
    lv_label_set_text(level->progress, level_text);
}

static void home_right_level_arc_update(home_base_ui_info_t *info, uint16_t end_angle)
{
    home_base_level_t *level = &info->level.right;
    if (level->arc == NULL || !lv_obj_is_valid(level->arc)) {
        level->arc = lv_arc_create(info->root);
        EXCEPTION_CHECK_VOID(level->arc);
        lv_obj_set_size(level->arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE);
        lv_obj_center(level->arc);
        lv_arc_set_bg_angles(level->arc, PUMP_RIGHT_ARC_START_ANGLE, PUMP_RIGHT_ARC_END_ANGLE);
        lv_arc_set_angles(level->arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_BG_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_width(level->arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(level->arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(level->arc, NULL, LV_PART_KNOB);
    } else {
        lv_arc_set_angles(level->arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);
    }
}

static void home_left_level_update(home_base_ui_info_t *info)
{
    home_base_level_t *level = &info->level.left;

    if (level->value == level->new_value && info->force_update == false) {
        return;
    }

    uint8_t level_val = level->new_value;
    uint16_t end_angle = 0;
    if (level_val > PUMP_LEVEL_MAX) {
        end_angle = PUMP_LEFT_ARC_END_ANGLE;
        level_val = PUMP_LEVEL_MAX;
    } else if (level_val == 0) {
        end_angle = PUMP_LEFT_ARC_START_ANGLE;
    } else {
        end_angle = ((PUMP_LEFT_ARC_END_ANGLE - PUMP_LEFT_ARC_START_ANGLE) * level_val / PUMP_LEVEL_MAX) +
            PUMP_LEFT_ARC_START_ANGLE;
    }

    home_left_level_arc_update(info, end_angle);
    home_left_level_indicator_update(info, end_angle);
    home_left_level_progress_update(info, level_val, end_angle);
    level->value = level_val;
}

static void home_right_level_update(home_base_ui_info_t *info)
{
    home_base_level_t *level = &info->level.right;

    if (level->value == level->new_value && info->force_update == false) {
        return;
    }

    uint8_t level_val = level->new_value;
    uint16_t end_angle = 0;
    if (level_val > PUMP_LEVEL_MAX) {
        end_angle = PUMP_RIGHT_ARC_START_ANGLE;
        level_val = PUMP_LEVEL_MAX;
    } else if (level_val == 0) {
        end_angle = PUMP_RIGHT_ARC_END_ANGLE;
    } else {
        end_angle = ((PUMP_RIGHT_ARC_END_ANGLE - PUMP_RIGHT_ARC_START_ANGLE) *
            (PUMP_LEVEL_MAX - level_val) / PUMP_LEVEL_MAX) + PUMP_RIGHT_ARC_START_ANGLE;
    }

    home_right_level_arc_update(info, end_angle);
    home_right_level_indicator_update(info, end_angle);
    home_right_level_progress_update(info, level_val, end_angle);
    level->value = level_val;
}

static void home_level_separator_update(home_base_ui_info_t *info)
{
    if (info->level.separator == NULL || !lv_obj_is_valid(info->level.separator)) {
        info->level.separator = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->level.separator);
        lv_obj_align(info->level.separator, LV_ALIGN_BOTTOM_MID, 2, -16); /// 设置分隔符位置
        ui_img_baj_theme_set_src(info->ui, info->level.separator, BASE_RES_ID_PUMP_LEVEL_SEPARATOR,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_level_ui_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->level.separator);
    ui_img_baj_release_decoded_buf(info->level.left.indicator);
    ui_img_baj_release_decoded_buf(info->level.left.progress_bg);
    ui_img_baj_release_decoded_buf(info->level.right.indicator);
    ui_img_baj_release_decoded_buf(info->level.right.progress_bg);
}

void home_pump_level_ui_update(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    home_level_separator_update(info);
    home_left_level_update(info);
    home_right_level_update(info);
}
