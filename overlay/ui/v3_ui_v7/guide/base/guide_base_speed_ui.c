/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_font_img_tip.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
/** 与 home_speed_adjust_ui 一致；力度沿用第十八页 setting.level */
#define GUIDE_SPEED_ARC_SIZE                            (352)
#define GUIDE_SPEED_ARC_WIDTH                           (11)
#define GUIDE_SPEED_ARC_SEG_COUNT                       (5)
#define GUIDE_SPEED_ARC_GAP_DEG                         (8)
#define GUIDE_SPEED_ARC_SEG_DEG                         (26)
#define GUIDE_SPEED_ARC_REMAIN_COLOR                    (0x252525)
#define GUIDE_SPEED_ARC_TRACK_DEG \
    (GUIDE_SPEED_ARC_SEG_COUNT * GUIDE_SPEED_ARC_SEG_DEG + \
     (GUIDE_SPEED_ARC_SEG_COUNT - 1) * GUIDE_SPEED_ARC_GAP_DEG)
#define GUIDE_SPEED_ARC_BOTTOM_GAP_DEG \
    (360 - GUIDE_SPEED_ARC_TRACK_DEG)
#define GUIDE_SPEED_ARC_ROTATION \
    (90 + (GUIDE_SPEED_ARC_BOTTOM_GAP_DEG / 2))

#define GUIDE_SPEED_VALUE_COLOR                         (0xFFFFFF)

#define GUIDE_SPEED_LABEL_COLOR                         (0x999999)

#define GUIDE_SPEED_TIP_Y                               (PANLE_DISPLAY_H - GUIDE_SPEED_TIP_H - 8)
#define GUIDE_SPEED_TIP_BASE_OPA                        (LV_OPA_70)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t guide_speed_mode_get(const ui_proc_guide_t *proc)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;

    if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
        slot = (uint8_t)proc->ui_mode_index;
    }
    return guide_mode_switch_mode_from_slot(slot);
}

static void guide_speed_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, GUIDE_SPEED_ARC_SIZE, GUIDE_SPEED_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, GUIDE_SPEED_ARC_ROTATION);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, GUIDE_SPEED_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, GUIDE_SPEED_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static uint8_t guide_speed_selected_count_get(uint8_t speed)
{
    uint8_t selected = speed + 1;

    if (selected > GUIDE_SPEED_ARC_SEG_COUNT) {
        selected = GUIDE_SPEED_ARC_SEG_COUNT;
    }
    if (selected > BREAST_PUMP_SPEED_MAX) {
        selected = BREAST_PUMP_SPEED_MAX;
    }
    return selected;
}

static void guide_speed_arc_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_speed_info_t *page = &info->speed;
    lv_obj_t *parent = guide_page_parent_get(info);
    uint32_t accent = home_level_adjust_accent_color_get(guide_speed_mode_get(proc));
    uint8_t selected = guide_speed_selected_count_get(proc->setting.speed);
    bool need_update;
    uint8_t i;

    EXCEPTION_CHECK_VOID(parent);
    need_update = (info->force_update || page->shown_accent_color != accent ||
        page->shown_selected != selected);

    for (i = 0; i < GUIDE_SPEED_ARC_SEG_COUNT; i++) {
        uint16_t start = (uint16_t)(i * (GUIDE_SPEED_ARC_SEG_DEG + GUIDE_SPEED_ARC_GAP_DEG));
        uint16_t end = (uint16_t)(start + GUIDE_SPEED_ARC_SEG_DEG);
        uint32_t color = ((i + 1) == selected) ? accent : GUIDE_SPEED_ARC_REMAIN_COLOR;
        bool created = false;

        if (page->arcs[i] == NULL || !lv_obj_is_valid(page->arcs[i])) {
            page->arcs[i] = lv_arc_create(parent);
            EXCEPTION_CHECK_VOID(page->arcs[i]);
            guide_speed_arc_style_init(page->arcs[i], color);
            created = true;
        }
        if (created || need_update) {
            lv_obj_set_style_arc_color(page->arcs[i], lv_color_hex(color), LV_PART_INDICATOR);
            lv_arc_set_angles(page->arcs[i], start, end);
            lv_obj_invalidate(page->arcs[i]);
        }
    }
    page->shown_selected = selected;
    page->shown_accent_color = accent;
    page->accent_color = accent;
}

static void guide_speed_value_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_speed_info_t *page = &info->speed;
    lv_obj_t *parent = guide_page_parent_get(info);
    char num_buf[8] = {0};
    uint8_t display_speed = proc->setting.speed + 1;

    EXCEPTION_CHECK_VOID(parent);
    if (page->value_label == NULL || !lv_obj_is_valid(page->value_label)) {
        page->value_label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(page->value_label);
        page->shown_speed = 0xFF;
    }
    if (info->force_update == false && page->shown_speed == proc->setting.speed) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), "%u", display_speed);
    lv_label_set_text(page->value_label, num_buf);
    lv_obj_set_style_text_color(page->value_label, lv_color_hex(GUIDE_SPEED_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(page->value_label, UI_FONT_NUM_LG, 0);
    lv_obj_set_style_text_align(page->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(page->value_label, LV_ALIGN_TOP_MID, 0, GUIDE_SPEED_VALUE_Y);
    page->shown_speed = proc->setting.speed;
}

static void guide_speed_label_update(guide_base_ui_info_t *info)
{
    guide_base_speed_info_t *page = &info->speed;
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (page->speed_label == NULL || !lv_obj_is_valid(page->speed_label)) {
        page->speed_label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(page->speed_label);
        lv_label_set_text(page->speed_label, GUIDE_SPEED_LABEL_TEXT);
        lv_obj_set_style_text_color(page->speed_label, lv_color_hex(GUIDE_SPEED_LABEL_COLOR), 0);
        lv_obj_set_style_text_font(page->speed_label, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_align(page->speed_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(page->speed_label, LV_ALIGN_TOP_MID, 0, GUIDE_SPEED_LABEL_Y);
    }
}

static lv_draw_buf_t *guide_speed_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->speed.tip_buf != NULL) {
        return info->speed.tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->speed.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->speed.tip_buf;
}

static void guide_speed_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_speed_info_t *page = &info->speed;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (page->tip_img != NULL && lv_obj_is_valid(page->tip_img) &&
        page->tip_shine.mask != NULL && lv_obj_is_valid(page->tip_shine.mask) &&
        page->tip_shine.shine != NULL && lv_obj_is_valid(page->tip_shine.shine)) {
        return;
    }

    predecoded = guide_speed_tip_predecode(info);
    if (page->tip_img == NULL || !lv_obj_is_valid(page->tip_img)) {
        page->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(page->tip_img);
        lv_obj_clear_flag(page->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(page->tip_img, predecoded);
            page->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(page->tip_img, GUIDE_SPEED_TIP_BASE_OPA, 0);
        lv_obj_align(page->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_SPEED_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_SPEED_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_prepare_from_tip(&page->tip_shine, &style, predecoded);
    ui_shine_update(&page->tip_shine, parent, &style, page->tip_img, 1);
    lv_obj_move_foreground(page->tip_img);
    ui_shine_show(&page->tip_shine);
}

static void guide_speed_tip_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    if (info->force_update) {
        guide_speed_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_SPEED_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->speed.tip_shine, &style, info->speed.tip_img, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_speed_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_shine_remove(&info->speed.tip_shine);
    if (info->speed.tip_img != NULL && lv_obj_is_valid(info->speed.tip_img)) {
        lv_image_set_src(info->speed.tip_img, NULL);
    }
    if (info->speed.tip_buf != NULL) {
        lv_draw_buf_destroy(info->speed.tip_buf);
        info->speed.tip_buf = NULL;
    }
    memset(&info->speed, 0, sizeof(info->speed));
}

void guide_speed_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    memset(&info->speed, 0, sizeof(info->speed));
    info->speed.shown_speed = 0xFF;
    info->speed.shown_selected = 0xFF;
}

void guide_speed_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(info && proc);
    guide_speed_arc_update(info, proc);
    guide_speed_value_update(info, proc);
    guide_speed_label_update(info);
    guide_speed_tip_update(info);
}
