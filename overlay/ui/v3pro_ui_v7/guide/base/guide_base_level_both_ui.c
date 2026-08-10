/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "home_both_level_adjust_ui.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_font_img_tip.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
/** 与 home_both_level_adjust_ui 一致 */
#define GUIDE_BOTH_LEVEL_ARC_SIZE                       (352)
#define GUIDE_BOTH_LEVEL_ARC_WIDTH                      (11)
#define GUIDE_BOTH_LEVEL_ARC_REMAIN_COLOR               (0x252525)
#define GUIDE_BOTH_LEVEL_ARC_SEG_GAP_DEG                (8)

#define GUIDE_BOTH_LEVEL_LEFT_START                     (136)
#define GUIDE_BOTH_LEVEL_LEFT_TRACK                     (122)
#define GUIDE_BOTH_LEVEL_RIGHT_END                      (540 - GUIDE_BOTH_LEVEL_LEFT_START)
#define GUIDE_BOTH_LEVEL_RIGHT_TRACK                    (GUIDE_BOTH_LEVEL_LEFT_TRACK)
#define GUIDE_BOTH_LEVEL_RIGHT_START                    (GUIDE_BOTH_LEVEL_RIGHT_END - GUIDE_BOTH_LEVEL_RIGHT_TRACK)

#define GUIDE_BOTH_LEVEL_VALUE_W                        (150)
#define GUIDE_BOTH_LEVEL_VALUE_H                        (69)
#define GUIDE_BOTH_LEVEL_VALUE_Y                        (122)
#define GUIDE_BOTH_LEVEL_VALUE_LEFT_X                   (29)
#define GUIDE_BOTH_LEVEL_VALUE_RIGHT_X                  (179)
#define GUIDE_BOTH_LEVEL_VALUE_COLOR                    (0xFFFFFF)

#define GUIDE_BOTH_LEVEL_DIR_W                          (150)
#define GUIDE_BOTH_LEVEL_DIR_H                          (34)
#define GUIDE_BOTH_LEVEL_DIR_Y                          (203)
#define GUIDE_BOTH_LEVEL_DIR_LEFT_X                     (29)
#define GUIDE_BOTH_LEVEL_DIR_RIGHT_X                    (179)
#define GUIDE_BOTH_LEVEL_DIR_COLOR                      (0x999999)
#define GUIDE_BOTH_LEVEL_DIR_LEFT_TEXT                  "L"
#define GUIDE_BOTH_LEVEL_DIR_RIGHT_TEXT                 "R"

#define GUIDE_BOTH_LEVEL_OVER_LIMIT_W                   (220)
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_H                   (50)
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_Y                   (60)
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE          (8)
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_TEXT \
    "For safety,\nUp to 3 levels at a time."

#define GUIDE_BOTH_LEVEL_TIP_H                          (63)
#define GUIDE_BOTH_LEVEL_TIP_Y                          (PANLE_DISPLAY_H - GUIDE_BOTH_LEVEL_TIP_H - 8)
#define GUIDE_BOTH_LEVEL_TIP_BASE_OPA                   (LV_OPA_70)

#define GUIDE_BOTH_LEVEL_ARC_ANIM_MS                    (220)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t guide_both_level_mode_get(const ui_proc_guide_t *proc)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;

    if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
        slot = (uint8_t)proc->ui_mode_index;
    }
    return guide_mode_switch_mode_from_slot(slot);
}

static uint8_t guide_both_level_level_max_get(const ui_proc_guide_t *proc)
{
    uint8_t mode = guide_both_level_mode_get(proc);
    uint8_t mode_type = home_level_adjust_mode_type_resolve(mode, PUMP_MODE_LACTATION);

    return home_level_adjust_max_get(mode_type);
}

static uint16_t guide_both_level_arc_sweep_from_level(uint8_t level, uint8_t level_max,
    uint16_t track_deg)
{
    uint16_t split_track;

    if (level_max == 0 || level == 0) {
        return 0;
    }
    if (level >= level_max) {
        return track_deg;
    }
    split_track = track_deg - GUIDE_BOTH_LEVEL_ARC_SEG_GAP_DEG;
    return (uint16_t)((split_track * level) / level_max);
}

static uint16_t guide_both_level_arc_visual_sweep_get(guide_base_level_both_side_arc_t *side)
{
    uint32_t elapsed;
    uint16_t from = side->anim_from_sweep;
    uint16_t to = side->anim_to_sweep;

    if (from == to) {
        return to;
    }
    elapsed = get_timestamp_ms(side->anim_start_time);
    if (elapsed >= GUIDE_BOTH_LEVEL_ARC_ANIM_MS) {
        side->anim_from_sweep = to;
        return to;
    }
    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) / GUIDE_BOTH_LEVEL_ARC_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) / GUIDE_BOTH_LEVEL_ARC_ANIM_MS);
}

static void guide_both_level_arc_anim_sync(guide_base_ui_info_t *info,
    guide_base_level_both_side_arc_t *side, uint8_t target_level, uint8_t level_max,
    uint16_t track_deg)
{
    uint16_t target_sweep = guide_both_level_arc_sweep_from_level(target_level, level_max,
        track_deg);

    if (info->force_update) {
        side->anim_from_sweep = target_sweep;
        side->anim_to_sweep = target_sweep;
        side->anim_visual_sweep = target_sweep;
        side->anim_to_level = target_level;
        side->anim_start_time = get_timestamp();
        return;
    }
    if (side->anim_to_level == target_level && side->anim_to_sweep == target_sweep) {
        return;
    }
    side->anim_from_sweep = guide_both_level_arc_visual_sweep_get(side);
    side->anim_to_sweep = target_sweep;
    side->anim_to_level = target_level;
    side->anim_start_time = get_timestamp();
}

static void guide_both_level_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, GUIDE_BOTH_LEVEL_ARC_SIZE, GUIDE_BOTH_LEVEL_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, GUIDE_BOTH_LEVEL_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, GUIDE_BOTH_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static void guide_both_level_side_arc_apply(guide_base_level_both_side_arc_t *side,
    uint16_t track_start, uint16_t track_deg, bool from_bottom, uint16_t done_sweep)
{
    uint16_t remain_sweep;
    uint16_t done_start;
    uint16_t done_end;
    uint16_t remain_start;
    uint16_t remain_end;

    if (done_sweep >= track_deg) {
        done_sweep = track_deg;
        remain_sweep = 0;
    } else if (done_sweep == 0) {
        remain_sweep = track_deg;
    } else {
        uint16_t split_track = track_deg - GUIDE_BOTH_LEVEL_ARC_SEG_GAP_DEG;
        if (done_sweep > split_track) {
            done_sweep = split_track;
        }
        remain_sweep = split_track - done_sweep;
    }

    if (from_bottom) {
        done_end = track_start + track_deg;
        done_start = done_end - done_sweep;
        if (remain_sweep > 0 && done_sweep > 0) {
            remain_end = done_start - GUIDE_BOTH_LEVEL_ARC_SEG_GAP_DEG;
            remain_start = track_start;
            remain_sweep = remain_end - remain_start;
        } else if (remain_sweep > 0) {
            remain_start = track_start;
            remain_end = track_start + track_deg;
        } else {
            remain_start = 0;
            remain_end = 0;
        }
    } else {
        done_start = track_start;
        done_end = track_start + done_sweep;
        if (remain_sweep > 0 && done_sweep > 0) {
            remain_start = done_end + GUIDE_BOTH_LEVEL_ARC_SEG_GAP_DEG;
            remain_end = track_start + track_deg;
            remain_sweep = remain_end - remain_start;
        } else if (remain_sweep > 0) {
            remain_start = track_start;
            remain_end = track_start + track_deg;
        } else {
            remain_start = 0;
            remain_end = 0;
        }
    }

    if (done_sweep > 0) {
        lv_arc_set_angles(side->done_arc, done_start, done_end);
        lv_obj_clear_flag(side->done_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(side->done_arc, 0, 0);
        lv_obj_add_flag(side->done_arc, LV_OBJ_FLAG_HIDDEN);
    }

    if (remain_sweep > 0) {
        lv_arc_set_angles(side->remain_arc, remain_start, remain_end);
        lv_obj_clear_flag(side->remain_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(side->remain_arc, 0, 0);
        lv_obj_add_flag(side->remain_arc, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_invalidate(side->done_arc);
    lv_obj_invalidate(side->remain_arc);
}

static void guide_both_level_arc_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_level_both_info_t *page = &info->level_both;
    lv_obj_t *parent = guide_page_parent_get(info);
    uint8_t level_max = guide_both_level_level_max_get(proc);
    uint32_t accent = home_level_adjust_accent_color_get(guide_both_level_mode_get(proc));
    bool color_changed = (page->shown_accent_color != accent);
    uint8_t i;

    EXCEPTION_CHECK_VOID(parent);
    for (i = 0; i < 2; i++) {
        guide_base_level_both_side_arc_t *side = &page->side[i];
        uint16_t track_deg = (i == 0) ? GUIDE_BOTH_LEVEL_LEFT_TRACK : GUIDE_BOTH_LEVEL_RIGHT_TRACK;
        uint16_t track_start = (i == 0) ? GUIDE_BOTH_LEVEL_LEFT_START : GUIDE_BOTH_LEVEL_RIGHT_START;
        uint16_t done_sweep;
        bool need_draw;

        if (side->done_arc == NULL || !lv_obj_is_valid(side->done_arc)) {
            side->done_arc = lv_arc_create(parent);
            EXCEPTION_CHECK_VOID(side->done_arc);
            guide_both_level_arc_style_init(side->done_arc, accent);
        }
        if (side->remain_arc == NULL || !lv_obj_is_valid(side->remain_arc)) {
            side->remain_arc = lv_arc_create(parent);
            EXCEPTION_CHECK_VOID(side->remain_arc);
            guide_both_level_arc_style_init(side->remain_arc, GUIDE_BOTH_LEVEL_ARC_REMAIN_COLOR);
        }
        if (color_changed || info->force_update) {
            lv_obj_set_style_arc_color(side->done_arc, lv_color_hex(accent), LV_PART_INDICATOR);
        }

        guide_both_level_arc_anim_sync(info, side, proc->both_level_arc[i], level_max, track_deg);
        done_sweep = guide_both_level_arc_visual_sweep_get(side);
        need_draw = (info->force_update || color_changed || side->anim_visual_sweep != done_sweep);
        if (!need_draw) {
            continue;
        }
        side->anim_visual_sweep = done_sweep;
        guide_both_level_side_arc_apply(side, track_start, track_deg, (i == 1), done_sweep);
    }
    page->shown_accent_color = accent;
    page->accent_color = accent;
    page->level_max = level_max;
}

static lv_obj_t *guide_both_level_box_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
    lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *box = lv_obj_create(parent);
    EXCEPTION_CHECK_POINT(box);
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, w, h);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_CLICKABLE);
    return box;
}

static void guide_both_level_value_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_level_both_info_t *page = &info->level_both;
    lv_obj_t *parent = guide_page_parent_get(info);
    uint8_t i;
    char num_buf[8] = {0};
    static const lv_coord_t value_x[2] = {
        GUIDE_BOTH_LEVEL_VALUE_LEFT_X, GUIDE_BOTH_LEVEL_VALUE_RIGHT_X
    };

    EXCEPTION_CHECK_VOID(parent);
    for (i = 0; i < 2; i++) {
        if (page->value_box[i] == NULL || !lv_obj_is_valid(page->value_box[i])) {
            page->value_box[i] = guide_both_level_box_create(parent, value_x[i],
                GUIDE_BOTH_LEVEL_VALUE_Y, GUIDE_BOTH_LEVEL_VALUE_W, GUIDE_BOTH_LEVEL_VALUE_H);
            EXCEPTION_CHECK_VOID(page->value_box[i]);
            page->shown_level[i] = 0xFF;
        }
        if (page->value_label[i] == NULL || !lv_obj_is_valid(page->value_label[i])) {
            page->value_label[i] = lv_label_create(page->value_box[i]);
            EXCEPTION_CHECK_VOID(page->value_label[i]);
        }
        if (info->force_update == false && page->shown_level[i] == proc->both_level_value[i]) {
            continue;
        }
        snprintf(num_buf, sizeof(num_buf), "%u", proc->both_level_value[i]);
        lv_label_set_text(page->value_label[i], num_buf);
        lv_obj_set_style_text_color(page->value_label[i],
            lv_color_hex(GUIDE_BOTH_LEVEL_VALUE_COLOR), 0);
        lv_obj_set_style_text_font(page->value_label[i], &lv_font_aeonik_soft_69_semibold, 0);
        lv_obj_set_style_text_align(page->value_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(page->value_label[i]);
        page->shown_level[i] = proc->both_level_value[i];
    }
}

static void guide_both_level_dir_update(guide_base_ui_info_t *info)
{
    guide_base_level_both_info_t *page = &info->level_both;
    lv_obj_t *parent = guide_page_parent_get(info);
    uint8_t i;
    static const lv_coord_t dir_x[2] = {
        GUIDE_BOTH_LEVEL_DIR_LEFT_X, GUIDE_BOTH_LEVEL_DIR_RIGHT_X
    };
    static const char *dir_text[2] = {
        GUIDE_BOTH_LEVEL_DIR_LEFT_TEXT, GUIDE_BOTH_LEVEL_DIR_RIGHT_TEXT
    };

    EXCEPTION_CHECK_VOID(parent);
    for (i = 0; i < 2; i++) {
        if (page->dir_box[i] == NULL || !lv_obj_is_valid(page->dir_box[i])) {
            page->dir_box[i] = guide_both_level_box_create(parent, dir_x[i],
                GUIDE_BOTH_LEVEL_DIR_Y, GUIDE_BOTH_LEVEL_DIR_W, GUIDE_BOTH_LEVEL_DIR_H);
            EXCEPTION_CHECK_VOID(page->dir_box[i]);
        }
        if (page->dir_label[i] == NULL || !lv_obj_is_valid(page->dir_label[i])) {
            page->dir_label[i] = lv_label_create(page->dir_box[i]);
            EXCEPTION_CHECK_VOID(page->dir_label[i]);
            lv_label_set_text(page->dir_label[i], dir_text[i]);
            lv_obj_set_style_text_color(page->dir_label[i],
                lv_color_hex(GUIDE_BOTH_LEVEL_DIR_COLOR), 0);
            lv_obj_set_style_text_font(page->dir_label[i], &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_align(page->dir_label[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_center(page->dir_label[i]);
        }
    }
}

static void guide_both_level_over_limit_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_level_both_info_t *page = &info->level_both;
    lv_obj_t *parent = guide_page_parent_get(info);
    uint32_t color = home_both_level_adjust_over_limit_color_get(guide_both_level_mode_get(proc));

    EXCEPTION_CHECK_VOID(parent);
    if (page->over_limit_box == NULL || !lv_obj_is_valid(page->over_limit_box)) {
        page->over_limit_box = guide_both_level_box_create(parent,
            (PANLE_DISPLAY_W - GUIDE_BOTH_LEVEL_OVER_LIMIT_W) / 2,
            GUIDE_BOTH_LEVEL_OVER_LIMIT_Y, GUIDE_BOTH_LEVEL_OVER_LIMIT_W,
            GUIDE_BOTH_LEVEL_OVER_LIMIT_H);
        EXCEPTION_CHECK_VOID(page->over_limit_box);
        lv_obj_add_flag(page->over_limit_box, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        page->shown_over_limit = 0xFF;
    }
    if (page->over_limit_label == NULL || !lv_obj_is_valid(page->over_limit_label)) {
        page->over_limit_label = lv_label_create(page->over_limit_box);
        EXCEPTION_CHECK_VOID(page->over_limit_label);
        lv_label_set_text(page->over_limit_label, GUIDE_BOTH_LEVEL_OVER_LIMIT_TEXT);
        lv_obj_set_style_text_font(page->over_limit_label, &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(page->over_limit_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(page->over_limit_label, GUIDE_BOTH_LEVEL_OVER_LIMIT_W);
        lv_obj_set_style_text_line_space(page->over_limit_label,
            GUIDE_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE, 0);
        lv_obj_center(page->over_limit_label);
    }

    if (info->force_update == false && page->shown_over_limit == proc->both_level_over_limit &&
        page->shown_over_limit_color == color) {
        return;
    }
    lv_obj_set_style_text_color(page->over_limit_label, lv_color_hex(color), 0);
    if (proc->both_level_over_limit) {
        lv_obj_clear_flag(page->over_limit_box, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(page->over_limit_box, LV_OBJ_FLAG_HIDDEN);
    }
    page->shown_over_limit = proc->both_level_over_limit;
    page->shown_over_limit_color = color;
}

static lv_draw_buf_t *guide_both_level_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->level_both.tip_buf != NULL) {
        return info->level_both.tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->level_both.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->level_both.tip_buf;
}

static void guide_both_level_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_level_both_info_t *page = &info->level_both;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (page->tip_img != NULL && lv_obj_is_valid(page->tip_img) &&
        page->tip_shine.mask != NULL && lv_obj_is_valid(page->tip_shine.mask) &&
        page->tip_shine.shine != NULL && lv_obj_is_valid(page->tip_shine.shine)) {
        return;
    }

    predecoded = guide_both_level_tip_predecode(info);
    if (page->tip_img == NULL || !lv_obj_is_valid(page->tip_img)) {
        page->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(page->tip_img);
        lv_obj_clear_flag(page->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(page->tip_img, predecoded);
            page->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(page->tip_img, GUIDE_BOTH_LEVEL_TIP_BASE_OPA, 0);
        lv_obj_align(page->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_BOTH_LEVEL_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_BOTH_LEVEL_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_prepare_from_tip(&page->tip_shine, &style, predecoded);
    ui_shine_update(&page->tip_shine, parent, &style, page->tip_img, 1);
    lv_obj_move_foreground(page->tip_img);
    ui_shine_show(&page->tip_shine);
}

static void guide_both_level_tip_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    if (info->force_update) {
        guide_both_level_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_BOTH_LEVEL_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->level_both.tip_shine, &style, info->level_both.tip_img, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_level_both_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_shine_remove(&info->level_both.tip_shine);
    if (info->level_both.tip_img != NULL && lv_obj_is_valid(info->level_both.tip_img)) {
        lv_image_set_src(info->level_both.tip_img, NULL);
    }
    if (info->level_both.tip_buf != NULL) {
        lv_draw_buf_destroy(info->level_both.tip_buf);
        info->level_both.tip_buf = NULL;
    }
    memset(&info->level_both, 0, sizeof(info->level_both));
}

void guide_level_both_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    memset(&info->level_both, 0, sizeof(info->level_both));
    info->level_both.shown_level[0] = 0xFF;
    info->level_both.shown_level[1] = 0xFF;
    info->level_both.shown_over_limit = 0xFF;
}

void guide_level_both_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(info && proc);
    guide_both_level_arc_update(info, proc);
    guide_both_level_value_update(info, proc);
    guide_both_level_dir_update(info);
    guide_both_level_over_limit_update(info, proc);
    guide_both_level_tip_update(info);
}
