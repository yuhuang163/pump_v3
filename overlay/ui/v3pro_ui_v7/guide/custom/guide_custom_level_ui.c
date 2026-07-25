/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_LEVEL_MAX                             (15)

#define PUMP_LEVEL_RING_SIZE                       (310)
#define PUMP_LEVEL_RING_ADJUST_SIZE                (316)

#define PUMP_LEFT_ARC_SIZE                         (50)
#define PUMP_LEFT_ANGLE_OFFSET                     (37)
#define PUMP_LEFT_ARC_START_ANGLE                  (90 + PUMP_LEFT_ANGLE_OFFSET)
#define PUMP_LEFT_ARC_END_ANGLE                    (PUMP_LEFT_ARC_START_ANGLE + PUMP_LEFT_ARC_SIZE)
#define PUMP_RIGHT_ARC_END_ANGLE                   (450 - PUMP_LEFT_ANGLE_OFFSET)
#define PUMP_RIGHT_ARC_START_ANGLE                 (PUMP_RIGHT_ARC_END_ANGLE - PUMP_LEFT_ARC_SIZE)

#define PUMP_LEVEL_ARC_WIDTH                       (9)
#define PUMP_LEVEL_ARC_WIDTH_ADJUST                (16)

#define PUMP_LEVEL_INDICATOR_SIZE                  (16)
#define PUMP_LEVEL_INDICATOR_ADJUST_SIZE           (24)
#define PUMP_LEVEL_INDICATOR_EXT_DRAW              (PUMP_LEVEL_INDICATOR_ADJUST_SIZE / 2)
#define PUMP_LEVEL_IMAGE_SCALE_NORMAL              (256)
#define PUMP_LEVEL_ARC_BG_COLOR                    (0xE5C1C5)
#define PUMP_LEVEL_ARC_ACTIVE_COLOR                (0xBD4060)
#define PUMP_LEVEL_TEXT_COLOR                      (0xC94265)

#define PUMP_LEFT_LEVEL_BG_X                       (12)
#define PUMP_LEFT_LEVEL_BG_Y                       (137)
#define PUMP_RIGHT_LEVEL_BG_X                      (308)
#define PUMP_RIGHT_LEVEL_BG_Y                      (PUMP_LEFT_LEVEL_BG_Y)

#define PUMP_LEFT_LEVEL_ADJUST_BG_X                (10)
#define PUMP_LEFT_LEVEL_ADJUST_BG_Y                (133)
#define PUMP_RIGHT_LEVEL_ADJUST_BG_X               (306)
#define PUMP_RIGHT_LEVEL_ADJUST_BG_Y               (PUMP_LEFT_LEVEL_ADJUST_BG_Y)

#if IS_SUPPORT_PUMP_LEVEL_ADJUST_BLINK
#define GUIDE_CUSTOM_LEVEL_BLINK_ENABLE             (1) /// 0: 关闭力度闪烁, 1: 开启力度闪烁(不影响调节放大).
#else
#define GUIDE_CUSTOM_LEVEL_BLINK_ENABLE             (0)
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_level_obj_visible_set(lv_obj_t *obj, bool visible)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    if (visible) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_coord_t guide_level_arc_width_get(bool adjust)
{
    return adjust ? PUMP_LEVEL_ARC_WIDTH_ADJUST : PUMP_LEVEL_ARC_WIDTH;
}

static lv_coord_t guide_level_ring_size_get(bool adjust)
{
    return adjust ? PUMP_LEVEL_RING_ADJUST_SIZE : PUMP_LEVEL_RING_SIZE;
}

static uint16_t guide_level_indicator_size_get(bool adjust)
{
    return adjust ? PUMP_LEVEL_INDICATOR_ADJUST_SIZE : PUMP_LEVEL_INDICATOR_SIZE;
}

static lv_coord_t guide_level_left_bg_x_get(bool adjust)
{
    return adjust ? PUMP_LEFT_LEVEL_ADJUST_BG_X : PUMP_LEFT_LEVEL_BG_X;
}

static lv_coord_t guide_level_left_bg_y_get(bool adjust)
{
    return adjust ? PUMP_LEFT_LEVEL_ADJUST_BG_Y : PUMP_LEFT_LEVEL_BG_Y;
}

static lv_coord_t guide_level_right_bg_x_get(bool adjust)
{
    return adjust ? PUMP_RIGHT_LEVEL_ADJUST_BG_X : PUMP_RIGHT_LEVEL_BG_X;
}

static lv_coord_t guide_level_right_bg_y_get(bool adjust)
{
    return adjust ? PUMP_RIGHT_LEVEL_ADJUST_BG_Y : PUMP_RIGHT_LEVEL_BG_Y;
}

static uint16_t guide_level_indicator_res_id_get(bool adjust)
{
    return adjust ? CUSTOM_RES_ID_PUMP_LEVEL_BIG_INDICATOR : CUSTOM_RES_ID_PUMP_LEVEL_INDICATOR;
}

static uint16_t guide_level_bg_res_id_get(bool adjust)
{
    return adjust ? CUSTOM_RES_ID_PUMP_LEVEL_BIG_BG : CUSTOM_RES_ID_PUMP_LEVEL_BG;
}

static void guide_level_indicator_res_apply(guide_custom_ui_info_t *info, guide_custom_level_t *level,
    bool adjust)
{
    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        return;
    }
    if (level->adjust_active == adjust) {
        return;
    }
    ui_img_baj_release_decoded_buf(level->indicator);
    ui_img_baj_theme_set_src(info->ui, level->indicator, guide_level_indicator_res_id_get(adjust),
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_image_set_scale(level->indicator, PUMP_LEVEL_IMAGE_SCALE_NORMAL);
}

static void guide_level_progress_bg_res_apply(guide_custom_ui_info_t *info, guide_custom_level_t *level,
    bool adjust)
{
    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        return;
    }
    if (level->adjust_active == adjust) {
        return;
    }
    ui_img_baj_release_decoded_buf(level->progress_bg);
    ui_img_baj_theme_set_src(info->ui, level->progress_bg, guide_level_bg_res_id_get(adjust),
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_image_set_scale(level->progress_bg, PUMP_LEVEL_IMAGE_SCALE_NORMAL);
}

static void guide_level_arc_ext_draw_event_cb(lv_event_t *e)
{
    lv_event_set_ext_draw_size(e, PUMP_LEVEL_INDICATOR_EXT_DRAW);
}

static void guide_level_arc_clip_fix_apply(lv_obj_t *arc)
{
    if (arc == NULL || !lv_obj_is_valid(arc)) {
        return;
    }
    lv_obj_add_flag(arc, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_event_cb(arc, guide_level_arc_ext_draw_event_cb, LV_EVENT_REFR_EXT_DRAW_SIZE, NULL);
    lv_obj_refresh_ext_draw_size(arc);
}

static void guide_level_arc_style_apply(lv_obj_t *arc, bool adjust)
{
    if (arc == NULL || !lv_obj_is_valid(arc)) {
        return;
    }
    const lv_coord_t ring_size = guide_level_ring_size_get(adjust);
    lv_obj_set_size(arc, ring_size, ring_size);
    lv_obj_center(arc);
    const lv_coord_t arc_width = guide_level_arc_width_get(adjust);
    lv_obj_set_style_arc_width(arc, arc_width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, arc_width, LV_PART_INDICATOR);
}

static void guide_level_progress_style_apply(guide_custom_ui_info_t *info, guide_custom_level_t *level,
    lv_coord_t bg_x, lv_coord_t bg_y, bool adjust)
{
    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        return;
    }
    guide_level_progress_bg_res_apply(info, level, adjust);
    lv_obj_align(level->progress_bg, LV_ALIGN_TOP_LEFT, bg_x, bg_y);
    if (level->progress != NULL && lv_obj_is_valid(level->progress)) {
        lv_obj_set_style_text_font(level->progress, adjust ?
            &lv_font_aeonik_soft_26_bold : &lv_font_aeonik_soft_20_semibold, 0);
    }
}

static void guide_level_side_blink_apply(guide_custom_level_t *level,
    uint8_t blink_state, bool blink_visible)
{
#if GUIDE_CUSTOM_LEVEL_BLINK_ENABLE
    const bool visible = (blink_state == 0) || blink_visible;
#else
    (void)blink_state;
    const bool visible = true;
#endif
    guide_level_obj_visible_set(level->arc, visible);
    // guide_level_obj_visible_set(level->progress_bg, visible);
}

static void guide_left_level_indicator_update(guide_custom_ui_info_t *info, uint16_t end_angle,
    bool adjust)
{
    guide_custom_level_t *level = &info->level.left;

    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        level->indicator = lv_image_create(level->arc);
        EXCEPTION_CHECK_VOID(level->indicator);
        ui_img_baj_theme_set_src(info->ui, level->indicator, CUSTOM_RES_ID_PUMP_LEVEL_INDICATOR,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
        level->shown_end_angle = 0xFFFF;
    }
    if (level->shown_end_angle == end_angle && level->adjust_active == adjust) {
        return;
    }
    guide_level_indicator_res_apply(info, level, adjust);
    ui_place_dot_on_arc(level->arc, level->indicator, end_angle,
        guide_level_arc_width_get(adjust), guide_level_indicator_size_get(adjust), 0, false);
    level->shown_end_angle = end_angle;
}

static void guide_left_level_progress_update(guide_custom_ui_info_t *info, uint8_t level_val, bool adjust)
{
    char level_text[4] = {0};
    guide_custom_level_t *level = &info->level.left;

    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        level->progress_bg = lv_image_create(guide_custom_level_theme_parent_get(info));
        EXCEPTION_CHECK_VOID(level->progress_bg);
        ui_img_baj_theme_set_src(info->ui, level->progress_bg, CUSTOM_RES_ID_PUMP_LEVEL_BG,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (level->progress == NULL || !lv_obj_is_valid(level->progress)) {
        level->progress = lv_label_create(level->progress_bg);
        EXCEPTION_CHECK_VOID(level->progress);
        lv_obj_set_style_text_color(level->progress, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level->progress, &lv_font_aeonik_soft_20_semibold, 0);
        lv_obj_align(level->progress, LV_ALIGN_CENTER, -1, 1);
    }
    guide_level_progress_style_apply(info, level, guide_level_left_bg_x_get(adjust),
        guide_level_left_bg_y_get(adjust), adjust);
    snprintf(level_text, sizeof(level_text), "%d", level_val);
    lv_label_set_text(level->progress, level_text);
}

static void guide_left_level_arc_update(guide_custom_ui_info_t *info, uint16_t end_angle, bool adjust)
{
    guide_custom_level_t *level = &info->level.left;

    if (level->arc == NULL || !lv_obj_is_valid(level->arc)) {
        level->arc = lv_arc_create(guide_custom_level_theme_parent_get(info));
        EXCEPTION_CHECK_VOID(level->arc);
        lv_arc_set_bg_angles(level->arc, PUMP_LEFT_ARC_START_ANGLE, PUMP_LEFT_ARC_END_ANGLE);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_20, LV_PART_MAIN);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(level->arc, NULL, LV_PART_KNOB);
        guide_level_arc_clip_fix_apply(level->arc);
        level->shown_end_angle = 0xFFFF;
    }
    guide_level_arc_style_apply(level->arc, adjust);
    if (level->shown_end_angle != end_angle || level->adjust_active != adjust) {
        lv_arc_set_angles(level->arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);
    }
}

static void guide_right_level_indicator_update(guide_custom_ui_info_t *info, uint16_t end_angle,
    bool adjust)
{
    guide_custom_level_t *level = &info->level.right;

    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        level->indicator = lv_image_create(level->arc);
        EXCEPTION_CHECK_VOID(level->indicator);
        ui_img_baj_theme_set_src(info->ui, level->indicator, CUSTOM_RES_ID_PUMP_LEVEL_INDICATOR,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
        level->shown_end_angle = 0xFFFF;
    }
    if (level->shown_end_angle == end_angle && level->adjust_active == adjust) {
        return;
    }
    guide_level_indicator_res_apply(info, level, adjust);
    ui_place_dot_on_arc(level->arc, level->indicator, end_angle,
        guide_level_arc_width_get(adjust), guide_level_indicator_size_get(adjust), -0.5f, false);
    level->shown_end_angle = end_angle;
}

static void guide_right_level_progress_update(guide_custom_ui_info_t *info, uint8_t level_val, bool adjust)
{
    char level_text[4] = {0};
    guide_custom_level_t *level = &info->level.right;

    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        level->progress_bg = lv_image_create(guide_custom_level_theme_parent_get(info));
        EXCEPTION_CHECK_VOID(level->progress_bg);
        ui_img_baj_theme_set_src(info->ui, level->progress_bg, CUSTOM_RES_ID_PUMP_LEVEL_BG,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (level->progress == NULL || !lv_obj_is_valid(level->progress)) {
        level->progress = lv_label_create(level->progress_bg);
        EXCEPTION_CHECK_VOID(level->progress);
        lv_obj_set_style_text_color(level->progress, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level->progress, &lv_font_aeonik_soft_20_semibold, 0);
        lv_obj_align(level->progress, LV_ALIGN_CENTER, -1, 1);
    }
    guide_level_progress_style_apply(info, level, guide_level_right_bg_x_get(adjust),
        guide_level_right_bg_y_get(adjust), adjust);
    snprintf(level_text, sizeof(level_text), "%d", level_val);
    lv_label_set_text(level->progress, level_text);
}

static void guide_right_level_arc_update(guide_custom_ui_info_t *info, uint16_t end_angle, bool adjust)
{
    guide_custom_level_t *level = &info->level.right;

    if (level->arc == NULL || !lv_obj_is_valid(level->arc)) {
        level->arc = lv_arc_create(guide_custom_level_theme_parent_get(info));
        EXCEPTION_CHECK_VOID(level->arc);
        lv_arc_set_bg_angles(level->arc, PUMP_RIGHT_ARC_START_ANGLE, PUMP_RIGHT_ARC_END_ANGLE);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_20, LV_PART_MAIN);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(level->arc, NULL, LV_PART_KNOB);
        guide_level_arc_clip_fix_apply(level->arc);
        level->shown_end_angle = 0xFFFF;
    }
    guide_level_arc_style_apply(level->arc, adjust);
    if (level->shown_end_angle != end_angle || level->adjust_active != adjust) {
        lv_arc_set_angles(level->arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);
    }
}

static uint16_t guide_left_level_end_angle_get(uint8_t level_val)
{
    if (level_val > PUMP_LEVEL_MAX) {
        return PUMP_LEFT_ARC_END_ANGLE;
    }
    if (level_val == 0) {
        return PUMP_LEFT_ARC_START_ANGLE;
    }
    return ((PUMP_LEFT_ARC_END_ANGLE - PUMP_LEFT_ARC_START_ANGLE) * level_val / PUMP_LEVEL_MAX) +
        PUMP_LEFT_ARC_START_ANGLE;
}

static uint16_t guide_right_level_end_angle_get(uint8_t level_val)
{
    if (level_val > PUMP_LEVEL_MAX) {
        return PUMP_RIGHT_ARC_START_ANGLE;
    }
    if (level_val == 0) {
        return PUMP_RIGHT_ARC_END_ANGLE;
    }
    return ((PUMP_RIGHT_ARC_END_ANGLE - PUMP_RIGHT_ARC_START_ANGLE) *
        (PUMP_LEVEL_MAX - level_val) / PUMP_LEVEL_MAX) + PUMP_RIGHT_ARC_START_ANGLE;
}

static void guide_left_level_update(guide_custom_ui_info_t *info, bool blink_visible)
{
    guide_custom_level_t *level = &info->level.left;
    const bool adjust = info->level.left_blink_state != 0;
    const bool need_content_update = (level->value != level->new_value) ||
        (level->adjust_active != adjust) || info->force_update;

    if (need_content_update == false) {
        guide_level_side_blink_apply(level, info->level.left_blink_state, blink_visible);
        return;
    }

    uint8_t level_val = level->new_value;
    if (level_val > PUMP_LEVEL_MAX) {
        level_val = PUMP_LEVEL_MAX;
    }
    const uint16_t end_angle = guide_left_level_end_angle_get(level_val);
    guide_left_level_arc_update(info, end_angle, adjust);
    guide_left_level_progress_update(info, level_val, adjust);
    guide_left_level_indicator_update(info, end_angle, adjust);
    level->value = level_val;
    level->adjust_active = adjust;
    guide_level_side_blink_apply(level, info->level.left_blink_state, blink_visible);
}

static void guide_right_level_update(guide_custom_ui_info_t *info, bool blink_visible)
{
    guide_custom_level_t *level = &info->level.right;
    const bool adjust = info->level.right_blink_state != 0;
    const bool need_content_update = (level->value != level->new_value) ||
        (level->adjust_active != adjust) || info->force_update;

    if (need_content_update == false) {
        guide_level_side_blink_apply(level, info->level.right_blink_state, blink_visible);
        return;
    }

    uint8_t level_val = level->new_value;
    if (level_val > PUMP_LEVEL_MAX) {
        level_val = PUMP_LEVEL_MAX;
    }
    const uint16_t end_angle = guide_right_level_end_angle_get(level_val);
    guide_right_level_arc_update(info, end_angle, adjust);
    guide_right_level_progress_update(info, level_val, adjust);
    guide_right_level_indicator_update(info, end_angle, adjust);
    level->value = level_val;
    level->adjust_active = adjust;
    guide_level_side_blink_apply(level, info->level.right_blink_state, blink_visible);
}

#if 0
static void guide_level_separator_update(guide_custom_ui_info_t *info)
{
    if (info->level.separator == NULL || !lv_obj_is_valid(info->level.separator)) {
        info->level.separator = lv_image_create(guide_custom_level_theme_parent_get(info));
        EXCEPTION_CHECK_VOID(info->level.separator);
        lv_obj_align(info->level.separator, LV_ALIGN_BOTTOM_MID, 2, -16);
        ui_img_baj_theme_set_src(info->ui, info->level.separator, CUSTOM_RES_ID_PUMP_LEVEL_SEPARATOR,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    }
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_pump_level_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->level.separator);
    ui_img_baj_release_decoded_buf(info->level.left.indicator);
    ui_img_baj_release_decoded_buf(info->level.left.progress_bg);
    ui_img_baj_release_decoded_buf(info->level.right.indicator);
    ui_img_baj_release_decoded_buf(info->level.right.progress_bg);
}

void guide_custom_pump_level_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_left_level_update(info, true);
    guide_right_level_update(info, true);
}

void guide_custom_pump_level_ui_update(guide_custom_ui_info_t *info, bool blink_visible)
{
    EXCEPTION_CHECK_VOID(info);
    // guide_level_separator_update(info);
    guide_left_level_update(info, blink_visible);
    guide_right_level_update(info, blink_visible);
}
