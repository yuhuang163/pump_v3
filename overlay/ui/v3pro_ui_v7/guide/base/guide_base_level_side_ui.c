/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_font_img_tip.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_LEVEL_SIDE_VALUE_Y                        (127)
#define GUIDE_LEVEL_SIDE_VALUE_COLOR                    (0xFFFFFF)

#define GUIDE_LEVEL_SIDE_DIR_Y                          (210)
#define GUIDE_LEVEL_SIDE_DIR_COLOR                      (0x999999)
#define GUIDE_LEVEL_SIDE_DIR_LINE_SPACE                 (8)
#define GUIDE_LEVEL_SIDE_DIR_LEFT_TEXT                  "Left Level"
#define GUIDE_LEVEL_SIDE_DIR_RIGHT_TEXT                 "Right Level"
#define GUIDE_LEVEL_SIDE_OVER_LIMIT_TEXT                "For safety,\nUp to 3 levels at a time."

#define GUIDE_LEVEL_SIDE_TIP_H                          (63)
#define GUIDE_LEVEL_SIDE_TIP_Y                          (PANLE_DISPLAY_H - GUIDE_LEVEL_SIDE_TIP_H - 8)
#define GUIDE_LEVEL_SIDE_TIP_BASE_OPA                   (LV_OPA_70)

#define GUIDE_LEVEL_SIDE_ARC_ANIM_MS                    (220)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t guide_level_side_mode_get(const ui_proc_guide_t *proc)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;

    if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
        slot = (uint8_t)proc->ui_mode_index;
    }
    return guide_mode_switch_mode_from_slot(slot);
}

static uint8_t guide_level_side_level_max_get(const ui_proc_guide_t *proc)
{
    uint8_t mode = guide_level_side_mode_get(proc);
    uint8_t mode_type = home_level_adjust_mode_type_resolve(mode, PUMP_MODE_LACTATION);

    return home_level_adjust_max_get(mode_type);
}

static uint16_t guide_level_side_arc_visual_sweep_get(guide_base_level_side_info_t *page)
{
    uint32_t elapsed;
    uint16_t from = page->anim_from_sweep;
    uint16_t to = page->anim_to_sweep;

    if (from == to) {
        return to;
    }
    elapsed = get_timestamp_ms(page->anim_start_time);
    if (elapsed >= GUIDE_LEVEL_SIDE_ARC_ANIM_MS) {
        page->anim_from_sweep = to;
        return to;
    }
    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) / GUIDE_LEVEL_SIDE_ARC_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) / GUIDE_LEVEL_SIDE_ARC_ANIM_MS);
}

static void guide_level_side_arc_anim_sync(guide_base_ui_info_t *info, ui_proc_guide_t *proc,
    uint8_t level_max, bool snap)
{
    guide_base_level_side_info_t *page = &info->level_side;
    uint16_t target_sweep = ui_progress_arc_sweep_from_progress(proc->level_arc_level, level_max);

    if (info->force_update || snap) {
        page->anim_from_sweep = target_sweep;
        page->anim_to_sweep = target_sweep;
        page->anim_to_level = proc->level_arc_level;
        page->anim_start_time = get_timestamp();
        return;
    }
    if (page->anim_to_level == proc->level_arc_level && page->anim_to_sweep == target_sweep) {
        return;
    }
    page->anim_from_sweep = guide_level_side_arc_visual_sweep_get(page);
    page->anim_to_sweep = target_sweep;
    page->anim_to_level = proc->level_arc_level;
    page->anim_start_time = get_timestamp();
}

static void guide_level_side_arc_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_level_side_info_t *page = &info->level_side;
    lv_obj_t *parent = guide_page_parent_get(info);
    ui_progress_arc_style_t style;
    uint8_t level_max = guide_level_side_level_max_get(proc);
    uint32_t accent = home_level_adjust_accent_color_get(guide_level_side_mode_get(proc));
    bool side_changed;
    uint16_t done_sweep;

    EXCEPTION_CHECK_VOID(parent);
    side_changed = (page->shown_side != 0xFF && page->shown_side != proc->level_adjust_side);
    guide_level_side_arc_anim_sync(info, proc, level_max, side_changed);
    done_sweep = guide_level_side_arc_visual_sweep_get(page);

    ui_progress_arc_style_init(&style, accent);
    style.show_countdown = false;
    ui_progress_arc_update(&page->arc, parent, &style, done_sweep, info->force_update);
    page->accent_color = accent;
    page->level_max = level_max;
}

static void guide_level_side_value_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_level_side_info_t *page = &info->level_side;
    lv_obj_t *parent = guide_page_parent_get(info);
    char num_buf[8] = {0};

    EXCEPTION_CHECK_VOID(parent);
    if (page->value_label == NULL || !lv_obj_is_valid(page->value_label)) {
        page->value_label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(page->value_label);
        page->shown_level = 0xFF;
    }
    if (info->force_update == false && page->shown_level == proc->level_value) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), "%u", proc->level_value);
    lv_label_set_text(page->value_label, num_buf);
    lv_obj_set_style_text_color(page->value_label, lv_color_hex(GUIDE_LEVEL_SIDE_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(page->value_label, &lv_font_aeonik_soft_69_semibold, 0);
    lv_obj_set_style_text_align(page->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(page->value_label, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_SIDE_VALUE_Y);
    page->shown_level = proc->level_value;
}

static void guide_level_side_dir_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_level_side_info_t *page = &info->level_side;
    lv_obj_t *parent = guide_page_parent_get(info);
    const char *text;
    const lv_font_t *font;
    uint32_t color;
    uint32_t expect_color;
    uint32_t accent = home_level_adjust_accent_color_get(guide_level_side_mode_get(proc));

    EXCEPTION_CHECK_VOID(parent);
    if (page->dir_label == NULL || !lv_obj_is_valid(page->dir_label)) {
        page->dir_label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(page->dir_label);
        page->shown_side = 0xFF;
        page->shown_over_limit = 0xFF;
    }

    if (proc->level_over_limit) {
        expect_color = accent;
    } else {
        expect_color = GUIDE_LEVEL_SIDE_DIR_COLOR;
    }
    if (info->force_update == false &&
        page->shown_side == proc->level_adjust_side &&
        page->shown_over_limit == proc->level_over_limit &&
        page->shown_dir_color == expect_color) {
        return;
    }

    if (proc->level_over_limit) {
        text = GUIDE_LEVEL_SIDE_OVER_LIMIT_TEXT;
        color = accent;
        font = &lv_font_aeonik_soft_22_medium;
    } else if (proc->level_adjust_side == GUIDE_LEVEL_ADJUST_SIDE_RIGHT) {
        text = GUIDE_LEVEL_SIDE_DIR_RIGHT_TEXT;
        color = GUIDE_LEVEL_SIDE_DIR_COLOR;
        font = &lv_font_aeonik_soft_26_medium;
    } else {
        text = GUIDE_LEVEL_SIDE_DIR_LEFT_TEXT;
        color = GUIDE_LEVEL_SIDE_DIR_COLOR;
        font = &lv_font_aeonik_soft_26_medium;
    }

    lv_label_set_text(page->dir_label, text);
    lv_obj_set_style_text_color(page->dir_label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(page->dir_label, font, 0);
    lv_obj_set_style_text_align(page->dir_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(page->dir_label, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_SIDE_DIR_Y);
    lv_obj_set_style_text_line_space(page->dir_label, GUIDE_LEVEL_SIDE_DIR_LINE_SPACE, 0);
    page->shown_side = proc->level_adjust_side;
    page->shown_over_limit = proc->level_over_limit;
    page->shown_dir_color = color;
    page->accent_color = accent;
}

static lv_draw_buf_t *guide_level_side_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->level_side.tip_buf != NULL) {
        return info->level_side.tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->level_side.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->level_side.tip_buf;
}

static void guide_level_side_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_level_side_info_t *page = &info->level_side;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (page->tip_img != NULL && lv_obj_is_valid(page->tip_img) &&
        page->tip_shine.mask != NULL && lv_obj_is_valid(page->tip_shine.mask) &&
        page->tip_shine.shine != NULL && lv_obj_is_valid(page->tip_shine.shine)) {
        return;
    }

    predecoded = guide_level_side_tip_predecode(info);
    if (page->tip_img == NULL || !lv_obj_is_valid(page->tip_img)) {
        page->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(page->tip_img);
        lv_obj_clear_flag(page->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(page->tip_img, predecoded);
            page->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(page->tip_img, GUIDE_LEVEL_SIDE_TIP_BASE_OPA, 0);
        lv_obj_align(page->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_SIDE_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_LEVEL_SIDE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_prepare_from_tip(&page->tip_shine, &style, predecoded);
    ui_shine_update(&page->tip_shine, parent, &style, page->tip_img, 1);
    lv_obj_move_foreground(page->tip_img);
    ui_shine_show(&page->tip_shine);
}

static void guide_level_side_tip_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    if (info->force_update) {
        guide_level_side_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_LEVEL_SIDE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->level_side.tip_shine, &style, info->level_side.tip_img, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_level_side_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_shine_remove(&info->level_side.tip_shine);
    if (info->level_side.tip_img != NULL && lv_obj_is_valid(info->level_side.tip_img)) {
        lv_image_set_src(info->level_side.tip_img, NULL);
    }
    if (info->level_side.tip_buf != NULL) {
        lv_draw_buf_destroy(info->level_side.tip_buf);
        info->level_side.tip_buf = NULL;
    }
    ui_progress_arc_hide(&info->level_side.arc);
    memset(&info->level_side, 0, sizeof(info->level_side));
}

void guide_level_side_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    memset(&info->level_side, 0, sizeof(info->level_side));
    info->level_side.shown_level = 0xFF;
    info->level_side.shown_side = 0xFF;
    info->level_side.shown_over_limit = 0xFF;
    ui_progress_arc_reset(&info->level_side.arc);
}

void guide_level_side_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(info && proc);
    guide_level_side_arc_update(info, proc);
    guide_level_side_value_update(info, proc);
    guide_level_side_dir_update(info, proc);
    guide_level_side_tip_update(info);
}
