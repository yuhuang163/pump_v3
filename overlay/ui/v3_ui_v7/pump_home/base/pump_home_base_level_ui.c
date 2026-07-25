/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep

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
#define PUMP_LEVEL_ARC_WIDTH_ADJUST                (16)
#define PUMP_LEVEL_INDICATOR_SIZE                  (16)
#define PUMP_LEVEL_INDICATOR_SIZE_ADJUST           (24)
#define PUMP_LEVEL_INDICATOR_EXT_DRAW              (PUMP_LEVEL_INDICATOR_SIZE_ADJUST / 2)
#define PUMP_LEVEL_BG_SIZE                         (40)
#define PUMP_LEVEL_BG_SIZE_ADJUST                  (49)
#define PUMP_LEVEL_IMAGE_SCALE_NORMAL              (256)
#define PUMP_LEVEL_ARC_BG_COLOR                    (0xE5C1C5)
#define PUMP_LEVEL_ARC_ACTIVE_COLOR                (0xBD4060)
#define PUMP_LEVEL_TEXT_COLOR                      (0xBD4060)
#define PUMP_LEFT_LEVEL_BG_X                       (18)
#define PUMP_LEFT_LEVEL_BG_Y                       (208)
#define PUMP_RIGHT_LEVEL_BG_X                      (306)
#define PUMP_RIGHT_LEVEL_BG_Y                      (PUMP_LEFT_LEVEL_BG_Y)

#if IS_SUPPORT_PUMP_LEVEL_ADJUST_BLINK
#define PUMP_HOME_BASE_LEVEL_BLINK_ENABLE          (1) /// 0: 关闭力度闪烁, 1: 开启力度闪烁(不影响调节放大).
#else
#define PUMP_HOME_BASE_LEVEL_BLINK_ENABLE          (0)
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_home_level_obj_visible_set(lv_obj_t *obj, bool visible)
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

static lv_coord_t pump_home_level_arc_width_get(bool adjust)
{
    return adjust ? PUMP_LEVEL_ARC_WIDTH_ADJUST : PUMP_LEVEL_ARC_WIDTH;
}

static uint16_t pump_home_level_indicator_size_get(bool adjust)
{
    return adjust ? PUMP_LEVEL_INDICATOR_SIZE_ADJUST : PUMP_LEVEL_INDICATOR_SIZE;
}

static lv_coord_t pump_home_level_bg_offset_get(bool adjust)
{
    return adjust ? (lv_coord_t)((PUMP_LEVEL_BG_SIZE - PUMP_LEVEL_BG_SIZE_ADJUST) / 2) : 0;
}

static uint16_t pump_home_level_indicator_res_id_get(bool adjust)
{
    // return adjust ? BASE_RES_ID_PUMP_LEVEL_BIG_INDICATOR : BASE_RES_ID_PUMP_LEVEL_INDICATOR;
    return 0;
}

static uint16_t pump_home_level_bg_res_id_get(bool adjust)
{
    // return adjust ? BASE_RES_ID_PUMP_LEVEL_BIG_BG : BASE_RES_ID_PUMP_LEVEL_BG;
    return 0;
}

static void pump_home_level_indicator_res_apply(pump_home_base_ui_info_t *info,
    pump_home_base_level_t *level, bool adjust)
{
    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        return;
    }
    if (level->adjust_active == adjust && info->force_update == false) {
        return;
    }
    ui_img_baj_release_decoded_buf(level->indicator);
    ui_img_baj_theme_set_src(info->ui, level->indicator, pump_home_level_indicator_res_id_get(adjust),
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_image_set_scale(level->indicator, PUMP_LEVEL_IMAGE_SCALE_NORMAL);
}

static void pump_home_level_progress_bg_res_apply(pump_home_base_ui_info_t *info,
    pump_home_base_level_t *level, bool adjust)
{
    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        return;
    }
    if (level->adjust_active == adjust && info->force_update == false) {
        return;
    }
    ui_img_baj_release_decoded_buf(level->progress_bg);
    ui_img_baj_theme_set_src(info->ui, level->progress_bg, pump_home_level_bg_res_id_get(adjust),
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_image_set_scale(level->progress_bg, PUMP_LEVEL_IMAGE_SCALE_NORMAL);
}

static void pump_home_level_arc_ext_draw_event_cb(lv_event_t *e)
{
    lv_event_set_ext_draw_size(e, PUMP_LEVEL_INDICATOR_EXT_DRAW);
}

static void pump_home_level_arc_clip_fix_apply(lv_obj_t *arc)
{
    if (arc == NULL || !lv_obj_is_valid(arc)) {
        return;
    }
    lv_obj_add_flag(arc, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_event_cb(arc, pump_home_level_arc_ext_draw_event_cb, LV_EVENT_REFR_EXT_DRAW_SIZE, NULL);
    lv_obj_refresh_ext_draw_size(arc);
}

static void pump_home_level_arc_style_apply(lv_obj_t *arc, bool adjust)
{
    if (arc == NULL || !lv_obj_is_valid(arc)) {
        return;
    }
    const lv_coord_t arc_width = pump_home_level_arc_width_get(adjust);
    lv_obj_set_style_arc_width(arc, arc_width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, arc_width, LV_PART_INDICATOR);
}

static void pump_home_level_progress_style_apply(pump_home_base_ui_info_t *info,
    pump_home_base_level_t *level, lv_coord_t bg_x, lv_coord_t bg_y, bool adjust)
{
    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        return;
    }
    pump_home_level_progress_bg_res_apply(info, level, adjust);
    lv_obj_align(level->progress_bg, LV_ALIGN_TOP_LEFT,
        bg_x + pump_home_level_bg_offset_get(adjust), bg_y + pump_home_level_bg_offset_get(adjust));
    if (level->progress != NULL && lv_obj_is_valid(level->progress)) {
        lv_obj_set_style_text_font(level->progress, adjust ?
            &lv_font_aeonik_soft_26_bold : &lv_font_aeonik_soft_20_semibold, 0);
    }
}

static void pump_home_level_side_blink_apply(pump_home_base_level_t *level,
    uint8_t blink_state, bool blink_visible)
{
#if PUMP_HOME_BASE_LEVEL_BLINK_ENABLE
    const bool visible = (blink_state == 0) || blink_visible;
#else
    (void)blink_state;
    const bool visible = true;
#endif
    pump_home_level_obj_visible_set(level->arc, visible);
    // pump_home_level_obj_visible_set(level->progress_bg, visible);
}

static void pump_home_left_level_indicator_update(pump_home_base_ui_info_t *info, uint16_t end_angle,
    bool adjust)
{
    pump_home_base_level_t *level = &info->level.left;

    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        level->indicator = lv_image_create(level->arc);
        EXCEPTION_CHECK_VOID(level->indicator);
        // ui_img_baj_theme_set_src(info->ui, level->indicator, BASE_RES_ID_PUMP_LEVEL_INDICATOR,
        //     RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    pump_home_level_indicator_res_apply(info, level, adjust);
    ui_place_dot_on_arc(level->arc, level->indicator, end_angle,
        pump_home_level_arc_width_get(adjust), pump_home_level_indicator_size_get(adjust), 0, false);
}

static void pump_home_left_level_progress_update(pump_home_base_ui_info_t *info, uint8_t level_val,
    bool adjust)
{
    char level_text[4] = {0};
    pump_home_base_level_t *level = &info->level.left;

    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        level->progress_bg = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(level->progress_bg);
        // ui_img_baj_theme_set_src(info->ui, level->progress_bg, BASE_RES_ID_PUMP_LEVEL_BG,
        //     RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (level->progress == NULL || !lv_obj_is_valid(level->progress)) {
        level->progress = lv_label_create(level->progress_bg);
        EXCEPTION_CHECK_VOID(level->progress);
        lv_obj_set_style_text_color(level->progress, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level->progress, &lv_font_aeonik_soft_20_semibold, 0);
        lv_obj_align(level->progress, LV_ALIGN_CENTER, -1, 1);
    }
    pump_home_level_progress_style_apply(info, level, PUMP_LEFT_LEVEL_BG_X, PUMP_LEFT_LEVEL_BG_Y, adjust);
    snprintf(level_text, sizeof(level_text), "%d", level_val);
    lv_label_set_text(level->progress, level_text);
}

static void pump_home_left_level_arc_update(pump_home_base_ui_info_t *info, uint16_t end_angle,
    bool adjust)
{
    pump_home_base_level_t *level = &info->level.left;

    if (level->arc == NULL || !lv_obj_is_valid(level->arc)) {
        level->arc = lv_arc_create(info->root);
        EXCEPTION_CHECK_VOID(level->arc);
        lv_obj_set_size(level->arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE);
        lv_obj_center(level->arc);
        lv_arc_set_bg_angles(level->arc, PUMP_LEFT_ARC_START_ANGLE, PUMP_LEFT_ARC_END_ANGLE);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_BG_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(level->arc, NULL, LV_PART_KNOB);
        pump_home_level_arc_clip_fix_apply(level->arc);
    }
    pump_home_level_arc_style_apply(level->arc, adjust);
    lv_arc_set_angles(level->arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);
}

static void pump_home_right_level_indicator_update(pump_home_base_ui_info_t *info, uint16_t end_angle,
    bool adjust)
{
    pump_home_base_level_t *level = &info->level.right;

    if (level->indicator == NULL || !lv_obj_is_valid(level->indicator)) {
        level->indicator = lv_image_create(level->arc);
        EXCEPTION_CHECK_VOID(level->indicator);
        // ui_img_baj_theme_set_src(info->ui, level->indicator, BASE_RES_ID_PUMP_LEVEL_INDICATOR,
        //     RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    pump_home_level_indicator_res_apply(info, level, adjust);
    ui_place_dot_on_arc(level->arc, level->indicator, end_angle,
        pump_home_level_arc_width_get(adjust), pump_home_level_indicator_size_get(adjust), -0.5f, false);
}

static void pump_home_right_level_progress_update(pump_home_base_ui_info_t *info, uint8_t level_val,
    bool adjust)
{
    char level_text[4] = {0};
    pump_home_base_level_t *level = &info->level.right;

    if (level->progress_bg == NULL || !lv_obj_is_valid(level->progress_bg)) {
        level->progress_bg = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(level->progress_bg);
        // ui_img_baj_theme_set_src(info->ui, level->progress_bg, BASE_RES_ID_PUMP_LEVEL_BG,
        //     RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (level->progress == NULL || !lv_obj_is_valid(level->progress)) {
        level->progress = lv_label_create(level->progress_bg);
        EXCEPTION_CHECK_VOID(level->progress);
        lv_obj_set_style_text_color(level->progress, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level->progress, &lv_font_aeonik_soft_20_semibold, 0);
        lv_obj_align(level->progress, LV_ALIGN_CENTER, -1, 1);
    }
    pump_home_level_progress_style_apply(info, level, PUMP_RIGHT_LEVEL_BG_X, PUMP_RIGHT_LEVEL_BG_Y, adjust);
    snprintf(level_text, sizeof(level_text), "%d", level_val);
    lv_label_set_text(level->progress, level_text);
}

static void pump_home_right_level_arc_update(pump_home_base_ui_info_t *info, uint16_t end_angle,
    bool adjust)
{
    pump_home_base_level_t *level = &info->level.right;

    if (level->arc == NULL || !lv_obj_is_valid(level->arc)) {
        level->arc = lv_arc_create(info->root);
        EXCEPTION_CHECK_VOID(level->arc);
        lv_obj_set_size(level->arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE);
        lv_obj_center(level->arc);
        lv_arc_set_bg_angles(level->arc, PUMP_RIGHT_ARC_START_ANGLE, PUMP_RIGHT_ARC_END_ANGLE);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_BG_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_arc_color(level->arc, lv_color_hex(PUMP_LEVEL_ARC_ACTIVE_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(level->arc, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(level->arc, NULL, LV_PART_KNOB);
        pump_home_level_arc_clip_fix_apply(level->arc);
    }
    pump_home_level_arc_style_apply(level->arc, adjust);
    lv_arc_set_angles(level->arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);
}

static uint16_t pump_home_left_level_end_angle_get(uint8_t level_val)
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

static uint16_t pump_home_right_level_end_angle_get(uint8_t level_val)
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

static void pump_home_left_level_update(pump_home_base_ui_info_t *info, bool blink_visible)
{
    pump_home_base_level_t *level = &info->level.left;
    const bool adjust = info->level.left_blink_state != 0;
    const bool need_content_update = (level->value != level->new_value) ||
        (level->adjust_active != adjust) || info->force_update;

    if (need_content_update == false) {
        pump_home_level_side_blink_apply(level, info->level.left_blink_state, blink_visible);
        return;
    }

    uint8_t level_val = level->new_value;
    if (level_val > PUMP_LEVEL_MAX) {
        level_val = PUMP_LEVEL_MAX;
    }
    const uint16_t end_angle = pump_home_left_level_end_angle_get(level_val);

    pump_home_left_level_arc_update(info, end_angle, adjust);
    pump_home_left_level_progress_update(info, level_val, adjust);
    pump_home_left_level_indicator_update(info, end_angle, adjust);
    level->value = level_val;
    level->adjust_active = adjust;
    pump_home_level_side_blink_apply(level, info->level.left_blink_state, blink_visible);
}

static void pump_home_right_level_update(pump_home_base_ui_info_t *info, bool blink_visible)
{
    pump_home_base_level_t *level = &info->level.right;
    const bool adjust = info->level.right_blink_state != 0;
    const bool need_content_update = (level->value != level->new_value) ||
        (level->adjust_active != adjust) || info->force_update;

    if (need_content_update == false) {
        pump_home_level_side_blink_apply(level, info->level.right_blink_state, blink_visible);
        return;
    }

    uint8_t level_val = level->new_value;
    if (level_val > PUMP_LEVEL_MAX) {
        level_val = PUMP_LEVEL_MAX;
    }
    const uint16_t end_angle = pump_home_right_level_end_angle_get(level_val);

    pump_home_right_level_arc_update(info, end_angle, adjust);
    pump_home_right_level_progress_update(info, level_val, adjust);
    pump_home_right_level_indicator_update(info, end_angle, adjust);
    level->value = level_val;
    level->adjust_active = adjust;
    pump_home_level_side_blink_apply(level, info->level.right_blink_state, blink_visible);
}

static void pump_home_level_separator_update(pump_home_base_ui_info_t *info)
{
    if (info->level.separator == NULL || !lv_obj_is_valid(info->level.separator)) {
        info->level.separator = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->level.separator);
        lv_obj_align(info->level.separator, LV_ALIGN_BOTTOM_MID, 2, -16);
        // ui_img_baj_theme_set_src(info->ui, info->level.separator, BASE_RES_ID_PUMP_LEVEL_SEPARATOR,
        //     RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_level_ui_remove(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->level.separator);
    ui_img_baj_release_decoded_buf(info->level.left.indicator);
    ui_img_baj_release_decoded_buf(info->level.left.progress_bg);
    ui_img_baj_release_decoded_buf(info->level.right.indicator);
    ui_img_baj_release_decoded_buf(info->level.right.progress_bg);
}

void pump_home_base_level_ui_update(pump_home_base_ui_info_t *info, bool blink_visible)
{
    EXCEPTION_CHECK_VOID(info);
    pump_home_level_separator_update(info);
    pump_home_left_level_update(info, blink_visible);
    pump_home_right_level_update(info, blink_visible);
}
