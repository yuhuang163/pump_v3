/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_font_img_tip.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_LEVEL_BOTH_GUIDE_PAGE_NUM_CURRENT             (5) //(6)
#define GUIDE_LEVEL_BOTH_GUIDE_PAGE_NUM_TOTAL               (6) //(7)




#define GUIDE_LEVEL_BOTH_GUIDE_TEXT_COLOR                   (0xFFFFFF)

#define GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_X                  (255)
#define GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_Y                  (121)
#define GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_RES_ID             (BASE_RES_ID_GUIDE_LEFT_KEY_MIN_ICON)

#define GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_X                 (138)
#define GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_Y                 (121)
#define GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_RES_ID            (BASE_RES_ID_GUIDE_RIGHT_KEY_MIN_ICON)

#define GUIDE_LEVEL_BOTH_GUIDE_TIP_H                        (63)
#define GUIDE_LEVEL_BOTH_GUIDE_TIP_Y                        (PANLE_DISPLAY_H - GUIDE_LEVEL_BOTH_GUIDE_TIP_H - 8)
#define GUIDE_LEVEL_BOTH_GUIDE_TIP_BASE_OPA                 (LV_OPA_70)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t guide_level_both_guide_bg_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_MODE_BG;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return BASE_RES_ID_PROGRAM_MODE_BG;
        case BREAST_PUMP_STIMULATION_MODE:
        default:
            return BASE_RES_ID_SIMULATION_MODE_BG;
    }
}

static uint8_t guide_level_both_guide_mode_from_info(const guide_base_ui_info_t *info)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;

    if (info != NULL && info->ui != NULL) {
        ui_proc_guide_t *proc = (ui_proc_guide_t *)info->ui->priv.play_info.user_data;
        if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
            slot = (uint8_t)proc->ui_mode_index;
        }
    }
    return guide_mode_switch_mode_from_slot(slot);
}

static void guide_level_both_guide_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->level_both_guide.bg == NULL || !lv_obj_is_valid(info->level_both_guide.bg)) {
        return;
    }
    src = lv_image_get_src(info->level_both_guide.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->level_both_guide.shown_bg_id) == src) {
        lv_image_set_src(info->level_both_guide.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->level_both_guide.bg);
}

static void guide_level_both_guide_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    const uint16_t res_id = guide_level_both_guide_bg_res_id_get(
        guide_level_both_guide_mode_from_info(info));
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->level_both_guide.bg == NULL || !lv_obj_is_valid(info->level_both_guide.bg)) {
        info->level_both_guide.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->level_both_guide.bg);
        lv_obj_move_background(info->level_both_guide.bg);
    }

    guide_level_both_guide_bg_detach(info);
    predecoded = get_predecoded_bg(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(info->level_both_guide.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->level_both_guide.bg, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->level_both_guide.bg, LV_ALIGN_CENTER, 0, 0);
    info->level_both_guide.shown_bg_id = res_id;
}

static void guide_level_both_guide_icon_create(guide_base_ui_info_t *info, lv_obj_t **icon,
    lv_draw_buf_t **icon_buf, uint16_t res_id, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui && icon && icon_buf);
    if (*icon == NULL || !lv_obj_is_valid(*icon)) {
        *icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(*icon);
    }

    if (*icon_buf == NULL) {
        *icon_buf = ui_img_baj_theme_decode_draw_buf(info->ui, res_id, RES_THEME_TYPE_BASE,
            UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (*icon_buf != NULL) {
        lv_image_set_src(*icon, *icon_buf);
    } else {
        ui_img_baj_theme_set_src(info->ui, *icon, res_id, RES_THEME_TYPE_BASE,
            UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(*icon, LV_ALIGN_TOP_LEFT, x, y);
}

static void guide_level_both_guide_label_create(lv_obj_t **label, lv_obj_t *parent,
    const char *text, lv_coord_t x, lv_coord_t y, int32_t line_space, bool center_x)
{
    EXCEPTION_CHECK_VOID(parent && label);
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
    }
    lv_label_set_text(*label, text);
    lv_obj_set_style_text_color(*label, lv_color_hex(GUIDE_LEVEL_BOTH_GUIDE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(*label, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(*label, line_space, 0);
    lv_obj_set_style_text_opa(*label, LV_OPA_COVER, 0);
    if (center_x) {
        lv_obj_align(*label, LV_ALIGN_TOP_MID, 0, y);
    } else {
        lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
    }
    lv_obj_move_foreground(*label);
}

static void guide_level_both_guide_texts_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    guide_level_both_guide_label_create(&info->level_both_guide.press, parent,
        GUIDE_LEVEL_BOTH_GUIDE_PRESS_TEXT, GUIDE_LEVEL_BOTH_GUIDE_PRESS_X,
        GUIDE_LEVEL_BOTH_GUIDE_PRESS_Y, GUIDE_LEVEL_BOTH_GUIDE_PRESS_LINE_SPACE, false);
    guide_level_both_guide_label_create(&info->level_both_guide.and_text, parent,
        GUIDE_LEVEL_BOTH_GUIDE_AND_TEXT, GUIDE_LEVEL_BOTH_GUIDE_AND_X,
        GUIDE_LEVEL_BOTH_GUIDE_AND_Y, GUIDE_LEVEL_BOTH_GUIDE_AND_LINE_SPACE, false);
    guide_level_both_guide_label_create(&info->level_both_guide.adjust, parent,
        GUIDE_LEVEL_BOTH_GUIDE_ADJUST_TEXT, 0, GUIDE_LEVEL_BOTH_GUIDE_ADJUST_Y,
        GUIDE_LEVEL_BOTH_GUIDE_ADJUST_LINE_SPACE, true);
}

static lv_draw_buf_t *guide_level_both_guide_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->level_both_guide.tip_buf != NULL) {
        return info->level_both_guide.tip_buf;
    }
    tip = ui_font_img_guide_skip_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->level_both_guide.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->level_both_guide.tip_buf;
}

static void guide_level_both_guide_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_level_both_guide_info_t *page = &info->level_both_guide;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (page->tip_img != NULL && lv_obj_is_valid(page->tip_img) &&
        page->tip_shine.mask != NULL && lv_obj_is_valid(page->tip_shine.mask) &&
        page->tip_shine.shine != NULL && lv_obj_is_valid(page->tip_shine.shine)) {
        return;
    }

    predecoded = guide_level_both_guide_tip_predecode(info);
    if (page->tip_img == NULL || !lv_obj_is_valid(page->tip_img)) {
        page->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(page->tip_img);
        lv_obj_clear_flag(page->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(page->tip_img, predecoded);
            page->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(page->tip_img, GUIDE_LEVEL_BOTH_GUIDE_TIP_BASE_OPA, 0);
        lv_obj_align(page->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_BOTH_GUIDE_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_LEVEL_BOTH_GUIDE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_prepare_from_tip(&page->tip_shine, &style, predecoded);
    ui_shine_update(&page->tip_shine, parent, &style, page->tip_img, 1);
    lv_obj_move_foreground(page->tip_img);
    ui_shine_show(&page->tip_shine);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_level_both_guide_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_level_both_guide_bg_detach(info);
    ui_shine_remove(&info->level_both_guide.tip_shine);
    if (info->level_both_guide.tip_img != NULL && lv_obj_is_valid(info->level_both_guide.tip_img)) {
        lv_image_set_src(info->level_both_guide.tip_img, NULL);
    }
    if (info->level_both_guide.tip_buf != NULL) {
        lv_draw_buf_destroy(info->level_both_guide.tip_buf);
        info->level_both_guide.tip_buf = NULL;
    }
    if (info->level_both_guide.left_icon != NULL &&
        lv_obj_is_valid(info->level_both_guide.left_icon)) {
        lv_image_set_src(info->level_both_guide.left_icon, NULL);
    }
    if (info->level_both_guide.right_icon != NULL &&
        lv_obj_is_valid(info->level_both_guide.right_icon)) {
        lv_image_set_src(info->level_both_guide.right_icon, NULL);
    }
    if (info->level_both_guide.left_icon_buf != NULL) {
        lv_draw_buf_destroy(info->level_both_guide.left_icon_buf);
        info->level_both_guide.left_icon_buf = NULL;
    }
    if (info->level_both_guide.right_icon_buf != NULL) {
        lv_draw_buf_destroy(info->level_both_guide.right_icon_buf);
        info->level_both_guide.right_icon_buf = NULL;
    }
    memset(&info->level_both_guide, 0, sizeof(info->level_both_guide));
}

void guide_level_both_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_level_both_guide_bg_create(info);
    guide_level_both_guide_icon_create(info, &info->level_both_guide.right_icon,
        &info->level_both_guide.right_icon_buf, GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_RES_ID,
        GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_X, GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_Y);
    guide_level_both_guide_icon_create(info, &info->level_both_guide.left_icon,
        &info->level_both_guide.left_icon_buf, GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_RES_ID,
        GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_X, GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_Y);
    guide_page_num_ui_build(&info->page_num, guide_page_parent_get(info),
        GUIDE_LEVEL_BOTH_GUIDE_PAGE_NUM_CURRENT, GUIDE_LEVEL_BOTH_GUIDE_PAGE_NUM_TOTAL);
    guide_level_both_guide_texts_create(info);
    guide_level_both_guide_tip_rebuild(info);
}

void guide_level_both_guide_ui_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(info);
    if (info->force_update) {
        guide_level_both_guide_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_LEVEL_BOTH_GUIDE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->level_both_guide.tip_shine, &style,
        info->level_both_guide.tip_img, 0);
}
