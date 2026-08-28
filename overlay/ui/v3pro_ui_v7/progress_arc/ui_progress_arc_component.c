/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_progress_arc_component.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "lvgl.h" // IWYU pragma: keep
#include "lv_user_font.h" // IWYU pragma: keep
#include "ui_lang.h" // IWYU pragma: keep

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_progress_arc_obj_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, UI_PROGRESS_ARC_SIZE, UI_PROGRESS_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, UI_PROGRESS_ARC_ROTATION);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, UI_PROGRESS_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, UI_PROGRESS_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_progress_arc_style_init(ui_progress_arc_style_t *style, uint32_t done_color)
{
    EXCEPTION_CHECK_VOID(style);
    style->done_color = done_color;
    style->remain_color = UI_PROGRESS_ARC_REMAIN_COLOR_DEFAULT;
    style->countdown_color = UI_PROGRESS_ARC_COUNTDOWN_COLOR_DEFAULT;
    style->countdown_y = UI_PROGRESS_ARC_COUNTDOWN_Y_DEFAULT;
    style->show_countdown = true;
}

void ui_progress_arc_reset(ui_progress_arc_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    info->shown_done_color = 0;
    info->shown_remain_color = 0;
    info->shown_done_sweep = 0xFFFF;
    info->shown_remain_sec = 0xFF;
}

void ui_progress_arc_update(ui_progress_arc_info_t *info, lv_obj_t *parent,
    const ui_progress_arc_style_t *style, uint16_t done_sweep, uint8_t force_update)
{
    uint16_t remain_sweep;
    uint16_t remain_start;
    uint16_t draw_done;
    bool color_changed;

    EXCEPTION_CHECK_VOID(info && parent && style);

    if (info->done_arc == NULL || !lv_obj_is_valid(info->done_arc)) {
        info->done_arc = lv_arc_create(parent);
        EXCEPTION_CHECK_VOID(info->done_arc);
        ui_progress_arc_obj_style_init(info->done_arc, style->done_color);
        info->shown_done_color = style->done_color;
        info->shown_done_sweep = 0xFFFF;
    }
    if (info->remain_arc == NULL || !lv_obj_is_valid(info->remain_arc)) {
        info->remain_arc = lv_arc_create(parent);
        EXCEPTION_CHECK_VOID(info->remain_arc);
        ui_progress_arc_obj_style_init(info->remain_arc, style->remain_color);
        info->shown_remain_color = style->remain_color;
        info->shown_done_sweep = 0xFFFF;
    }

    color_changed = (info->shown_done_color != style->done_color) ||
        (info->shown_remain_color != style->remain_color);
    if (info->shown_done_color != style->done_color || force_update) {
        lv_obj_set_style_arc_color(info->done_arc, lv_color_hex(style->done_color),
            LV_PART_INDICATOR);
        info->shown_done_color = style->done_color;
    }
    if (info->shown_remain_color != style->remain_color || force_update) {
        lv_obj_set_style_arc_color(info->remain_arc, lv_color_hex(style->remain_color),
            LV_PART_INDICATOR);
        info->shown_remain_color = style->remain_color;
    }

    if (force_update == false && color_changed == false &&
        info->shown_done_sweep == done_sweep) {
        /* hide 后双弧均隐藏时需重绘，避免仅靠 shown 缓存误跳过 */
        bool both_hidden =
            (info->done_arc != NULL && lv_obj_is_valid(info->done_arc) &&
                lv_obj_has_flag(info->done_arc, LV_OBJ_FLAG_HIDDEN)) &&
            (info->remain_arc != NULL && lv_obj_is_valid(info->remain_arc) &&
                lv_obj_has_flag(info->remain_arc, LV_OBJ_FLAG_HIDDEN));
        if (both_hidden == false) {
            return;
        }
    }

    draw_done = done_sweep;
    if (draw_done >= UI_PROGRESS_ARC_TRACK_DEG) {
        draw_done = UI_PROGRESS_ARC_TRACK_DEG;
        remain_sweep = 0;
    } else if (draw_done == 0) {
        remain_sweep = UI_PROGRESS_ARC_TRACK_DEG;
    } else {
        uint16_t split_track = UI_PROGRESS_ARC_TRACK_DEG - UI_PROGRESS_ARC_SEG_GAP_DEG;
        if (draw_done > split_track) {
            draw_done = split_track;
        }
        remain_sweep = split_track - draw_done;
    }

    if (draw_done > 0) {
        lv_arc_set_angles(info->done_arc, 0, draw_done);
        lv_obj_clear_flag(info->done_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(info->done_arc, 0, 0);
        lv_obj_add_flag(info->done_arc, LV_OBJ_FLAG_HIDDEN);
    }

    if (remain_sweep > 0) {
        if (draw_done > 0) {
            remain_start = draw_done + UI_PROGRESS_ARC_SEG_GAP_DEG;
        } else {
            remain_start = 0;
            remain_sweep = UI_PROGRESS_ARC_TRACK_DEG;
        }
        lv_arc_set_angles(info->remain_arc, remain_start, remain_start + remain_sweep);
        lv_obj_clear_flag(info->remain_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(info->remain_arc, 0, 0);
        lv_obj_add_flag(info->remain_arc, LV_OBJ_FLAG_HIDDEN);
    }

    info->shown_done_sweep = done_sweep;
    lv_obj_invalidate(info->done_arc);
    lv_obj_invalidate(info->remain_arc);
}

void ui_progress_arc_hide(ui_progress_arc_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->done_arc != NULL && lv_obj_is_valid(info->done_arc)) {
        lv_arc_set_angles(info->done_arc, 0, 0);
        lv_obj_add_flag(info->done_arc, LV_OBJ_FLAG_HIDDEN);
    }
    if (info->remain_arc != NULL && lv_obj_is_valid(info->remain_arc)) {
        lv_arc_set_angles(info->remain_arc, 0, 0);
        lv_obj_add_flag(info->remain_arc, LV_OBJ_FLAG_HIDDEN);
    }
    info->shown_done_sweep = 0xFFFF;
}

void ui_progress_arc_countdown_hide(ui_progress_arc_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->countdown != NULL && lv_obj_is_valid(info->countdown)) {
        lv_obj_add_flag(info->countdown, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_progress_arc_countdown_update(ui_progress_arc_info_t *info, lv_obj_t *parent,
    const ui_progress_arc_style_t *style, uint8_t remain_sec, uint8_t force_update)
{
    char num_buf[8] = {0};

    EXCEPTION_CHECK_VOID(info && parent && style);

    if (style->show_countdown == false) {
        ui_progress_arc_countdown_hide(info);
        return;
    }

    if (info->countdown == NULL || !lv_obj_is_valid(info->countdown)) {
        info->countdown = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->countdown);
        info->shown_remain_sec = 0xFF;
    }

    if (force_update == false && info->shown_remain_sec == remain_sec) {
        lv_obj_clear_flag(info->countdown, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    snprintf(num_buf, sizeof(num_buf), UI_STR_COUNTDOWN_SEC_FMT, remain_sec);
    lv_label_set_text(info->countdown, num_buf);
    lv_obj_set_style_text_color(info->countdown, lv_color_hex(style->countdown_color), 0);
    lv_obj_set_style_text_font(info->countdown, UI_FONT_HINT, 0);
    lv_obj_set_style_text_align(info->countdown, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(info->countdown, LV_ALIGN_TOP_MID, 0, style->countdown_y);
    lv_obj_clear_flag(info->countdown, LV_OBJ_FLAG_HIDDEN);
    info->shown_remain_sec = remain_sec;
}

uint16_t ui_progress_arc_sweep_from_progress(uint32_t progress, uint32_t total)
{
    uint16_t split_track;

    if (total == 0 || progress == 0) {
        return 0;
    }
    if (progress >= total) {
        return UI_PROGRESS_ARC_TRACK_DEG;
    }
    split_track = UI_PROGRESS_ARC_TRACK_DEG - UI_PROGRESS_ARC_SEG_GAP_DEG;
    return (uint16_t)((split_track * progress) / total);
}

uint8_t ui_progress_arc_remain_sec_get(uint32_t elapsed_ms, uint32_t duration_ms,
    uint8_t countdown_max)
{
    uint8_t remain;

    if (countdown_max == 0) {
        return 1;
    }
    if (elapsed_ms >= duration_ms) {
        return 1;
    }
    remain = (uint8_t)(countdown_max - ((elapsed_ms * countdown_max) / duration_ms));
    if (remain < 1) {
        remain = 1;
    }
    if (remain > countdown_max) {
        remain = countdown_max;
    }
    return remain;
}

void ui_progress_arc_time_update(ui_progress_arc_info_t *info, lv_obj_t *parent,
    const ui_progress_arc_style_t *style, uint32_t start_time, uint32_t duration_ms,
    uint8_t countdown_max, uint8_t force_update)
{
    uint32_t elapsed;
    uint16_t done_sweep;
    uint8_t remain_sec;

    EXCEPTION_CHECK_VOID(info && parent && style);

    elapsed = get_timestamp_ms(start_time);
    if (elapsed >= duration_ms) {
        done_sweep = UI_PROGRESS_ARC_TRACK_DEG;
    } else {
        done_sweep = ui_progress_arc_sweep_from_progress(elapsed, duration_ms);
    }
    ui_progress_arc_update(info, parent, style, done_sweep, force_update);

    if (style->show_countdown) {
        remain_sec = ui_progress_arc_remain_sec_get(elapsed, duration_ms, countdown_max);
        ui_progress_arc_countdown_update(info, parent, style, remain_sec, force_update);
    } else {
        ui_progress_arc_countdown_hide(info);
    }
}
