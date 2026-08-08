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
#define GUIDE_SPEED_GUIDE_PAGE_NUM_CURRENT                  (7)
#define GUIDE_SPEED_GUIDE_PAGE_NUM_TOTAL                    (7)

#define GUIDE_SPEED_GUIDE_PRESS_X                           (109)
#define GUIDE_SPEED_GUIDE_PRESS_Y                           (146 + 2)
#define GUIDE_SPEED_GUIDE_PRESS_TEXT                        "Press"
#define GUIDE_SPEED_GUIDE_PRESS_LINE_SPACE                  (4)

#define GUIDE_SPEED_GUIDE_ADJUST_Y                          (199 + 2)
#define GUIDE_SPEED_GUIDE_ADJUST_TEXT                       "to adjust cycle speed."
#define GUIDE_SPEED_GUIDE_ADJUST_LINE_SPACE                 (8)

#define GUIDE_SPEED_GUIDE_TEXT_COLOR                        (0xFFFFFF)

#define GUIDE_SPEED_GUIDE_ICON_X                            (187)
#define GUIDE_SPEED_GUIDE_ICON_Y                            (129)
#define GUIDE_SPEED_GUIDE_ICON_RES_ID                       (BASE_RES_ID_GUIDE_SPEED_KEY_ICON)

#define GUIDE_SPEED_GUIDE_TIP_H                             (63)
#define GUIDE_SPEED_GUIDE_TIP_Y                             (PANLE_DISPLAY_H - GUIDE_SPEED_GUIDE_TIP_H - 8)
#define GUIDE_SPEED_GUIDE_TIP_BASE_OPA                      (LV_OPA_70)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t guide_speed_guide_bg_res_id_get(uint8_t mode)
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

static uint8_t guide_speed_guide_mode_from_info(const guide_base_ui_info_t *info)
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

static void guide_speed_guide_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->speed_guide.bg == NULL || !lv_obj_is_valid(info->speed_guide.bg)) {
        return;
    }
    src = lv_image_get_src(info->speed_guide.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->speed_guide.shown_bg_id) == src) {
        lv_image_set_src(info->speed_guide.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->speed_guide.bg);
}

static void guide_speed_guide_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    const uint16_t res_id = guide_speed_guide_bg_res_id_get(
        guide_speed_guide_mode_from_info(info));
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->speed_guide.bg == NULL || !lv_obj_is_valid(info->speed_guide.bg)) {
        info->speed_guide.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->speed_guide.bg);
        lv_obj_move_background(info->speed_guide.bg);
    }

    guide_speed_guide_bg_detach(info);
    predecoded = get_predecoded_bg(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(info->speed_guide.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->speed_guide.bg, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->speed_guide.bg, LV_ALIGN_CENTER, 0, 0);
    info->speed_guide.shown_bg_id = res_id;
}

static void guide_speed_guide_icon_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->speed_guide.icon == NULL || !lv_obj_is_valid(info->speed_guide.icon)) {
        info->speed_guide.icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->speed_guide.icon);
    }

    if (info->speed_guide.icon_buf == NULL) {
        info->speed_guide.icon_buf = ui_img_baj_theme_decode_draw_buf(info->ui,
            GUIDE_SPEED_GUIDE_ICON_RES_ID, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->speed_guide.icon_buf != NULL) {
        lv_image_set_src(info->speed_guide.icon, info->speed_guide.icon_buf);
    } else {
        ui_img_baj_theme_set_src(info->ui, info->speed_guide.icon,
            GUIDE_SPEED_GUIDE_ICON_RES_ID, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->speed_guide.icon, LV_ALIGN_TOP_LEFT,
        GUIDE_SPEED_GUIDE_ICON_X, GUIDE_SPEED_GUIDE_ICON_Y);
}

static void guide_speed_guide_label_create(lv_obj_t **label, lv_obj_t *parent,
    const char *text, lv_coord_t x, lv_coord_t y, int32_t line_space, bool center_x)
{
    EXCEPTION_CHECK_VOID(parent && label);
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
    }
    lv_label_set_text(*label, text);
    lv_obj_set_style_text_color(*label, lv_color_hex(GUIDE_SPEED_GUIDE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(*label, &lv_font_aeonik_soft_26_medium, 0);
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

static void guide_speed_guide_texts_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    guide_speed_guide_label_create(&info->speed_guide.press, parent,
        GUIDE_SPEED_GUIDE_PRESS_TEXT, GUIDE_SPEED_GUIDE_PRESS_X, GUIDE_SPEED_GUIDE_PRESS_Y,
        GUIDE_SPEED_GUIDE_PRESS_LINE_SPACE, false);
    guide_speed_guide_label_create(&info->speed_guide.adjust, parent,
        GUIDE_SPEED_GUIDE_ADJUST_TEXT, 0, GUIDE_SPEED_GUIDE_ADJUST_Y,
        GUIDE_SPEED_GUIDE_ADJUST_LINE_SPACE, true);
}

static lv_draw_buf_t *guide_speed_guide_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->speed_guide.tip_buf != NULL) {
        return info->speed_guide.tip_buf;
    }
    tip = ui_font_img_guide_skip_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->speed_guide.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->speed_guide.tip_buf;
}

static void guide_speed_guide_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_speed_guide_info_t *page = &info->speed_guide;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (page->tip_img != NULL && lv_obj_is_valid(page->tip_img) &&
        page->tip_shine.mask != NULL && lv_obj_is_valid(page->tip_shine.mask) &&
        page->tip_shine.shine != NULL && lv_obj_is_valid(page->tip_shine.shine)) {
        return;
    }

    predecoded = guide_speed_guide_tip_predecode(info);
    if (page->tip_img == NULL || !lv_obj_is_valid(page->tip_img)) {
        page->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(page->tip_img);
        lv_obj_clear_flag(page->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(page->tip_img, predecoded);
            page->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(page->tip_img, GUIDE_SPEED_GUIDE_TIP_BASE_OPA, 0);
        lv_obj_align(page->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_SPEED_GUIDE_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_SPEED_GUIDE_TIP_Y;
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
void guide_speed_guide_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_speed_guide_bg_detach(info);
    ui_shine_remove(&info->speed_guide.tip_shine);
    if (info->speed_guide.tip_img != NULL && lv_obj_is_valid(info->speed_guide.tip_img)) {
        lv_image_set_src(info->speed_guide.tip_img, NULL);
    }
    if (info->speed_guide.tip_buf != NULL) {
        lv_draw_buf_destroy(info->speed_guide.tip_buf);
        info->speed_guide.tip_buf = NULL;
    }
    if (info->speed_guide.icon != NULL && lv_obj_is_valid(info->speed_guide.icon)) {
        lv_image_set_src(info->speed_guide.icon, NULL);
    }
    if (info->speed_guide.icon_buf != NULL) {
        lv_draw_buf_destroy(info->speed_guide.icon_buf);
        info->speed_guide.icon_buf = NULL;
    }
    memset(&info->speed_guide, 0, sizeof(info->speed_guide));
}

void guide_speed_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_speed_guide_bg_create(info);
    guide_speed_guide_icon_create(info);
    guide_page_num_ui_build(&info->page_num, guide_page_parent_get(info),
        GUIDE_SPEED_GUIDE_PAGE_NUM_CURRENT, GUIDE_SPEED_GUIDE_PAGE_NUM_TOTAL);
    guide_speed_guide_texts_create(info);
    guide_speed_guide_tip_rebuild(info);
}

void guide_speed_guide_ui_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(info);
    if (info->force_update) {
        guide_speed_guide_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_SPEED_GUIDE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->speed_guide.tip_shine, &style, info->speed_guide.tip_img, 0);
}
