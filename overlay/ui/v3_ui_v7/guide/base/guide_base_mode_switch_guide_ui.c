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
#define GUIDE_MODE_SWITCH_GUIDE_BG_RES_ID                       (BASE_RES_ID_SIMULATION_MODE_BG)

#define GUIDE_MODE_SWITCH_GUIDE_PAGE_NUM_CURRENT                (3) //(4)
#define GUIDE_MODE_SWITCH_GUIDE_PAGE_NUM_TOTAL                  (6) //(7)

#define GUIDE_MODE_SWITCH_GUIDE_TITLE_COLOR                     (0xFFFFFF)

#define GUIDE_MODE_SWITCH_GUIDE_ICON_Y                          (143)
#define GUIDE_MODE_SWITCH_GUIDE_ICON_RES_ID                     (BASE_RES_ID_GUIDE_KNOB_KEY_ICON)

#define GUIDE_MODE_SWITCH_GUIDE_TIP_H                           (63)
#define GUIDE_MODE_SWITCH_GUIDE_TIP_Y                           (PANLE_DISPLAY_H - GUIDE_MODE_SWITCH_GUIDE_TIP_H - 8)
#define GUIDE_MODE_SWITCH_GUIDE_TIP_BASE_OPA                    (LV_OPA_70)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_mode_switch_guide_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->mode_switch_guide.bg == NULL || !lv_obj_is_valid(info->mode_switch_guide.bg)) {
        return;
    }
    src = lv_image_get_src(info->mode_switch_guide.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->mode_switch_guide.shown_bg_id) == src) {
        lv_image_set_src(info->mode_switch_guide.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->mode_switch_guide.bg);
}

static void guide_mode_switch_guide_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->mode_switch_guide.bg == NULL || !lv_obj_is_valid(info->mode_switch_guide.bg)) {
        info->mode_switch_guide.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->mode_switch_guide.bg);
        lv_obj_move_background(info->mode_switch_guide.bg);
    }

    guide_mode_switch_guide_bg_detach(info);
    predecoded = get_predecoded_bg(GUIDE_MODE_SWITCH_GUIDE_BG_RES_ID);
    if (predecoded != NULL) {
        lv_image_set_src(info->mode_switch_guide.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->mode_switch_guide.bg,
            GUIDE_MODE_SWITCH_GUIDE_BG_RES_ID, RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->mode_switch_guide.bg, LV_ALIGN_CENTER, 0, 0);
    info->mode_switch_guide.shown_bg_id = GUIDE_MODE_SWITCH_GUIDE_BG_RES_ID;
}

static void guide_mode_switch_guide_icon_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->mode_switch_guide.icon == NULL || !lv_obj_is_valid(info->mode_switch_guide.icon)) {
        info->mode_switch_guide.icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->mode_switch_guide.icon);
    }

    if (info->mode_switch_guide.icon_buf == NULL) {
        info->mode_switch_guide.icon_buf = ui_img_baj_theme_decode_draw_buf(info->ui,
            GUIDE_MODE_SWITCH_GUIDE_ICON_RES_ID, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->mode_switch_guide.icon_buf != NULL) {
        lv_image_set_src(info->mode_switch_guide.icon, info->mode_switch_guide.icon_buf);
    } else {
        ui_img_baj_theme_set_src(info->ui, info->mode_switch_guide.icon,
            GUIDE_MODE_SWITCH_GUIDE_ICON_RES_ID, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->mode_switch_guide.icon, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SWITCH_GUIDE_ICON_Y);
}

static void guide_mode_switch_guide_title_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->mode_switch_guide.title == NULL || !lv_obj_is_valid(info->mode_switch_guide.title)) {
        info->mode_switch_guide.title = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->mode_switch_guide.title);
    }
    lv_label_set_text(info->mode_switch_guide.title, GUIDE_MODE_SWITCH_GUIDE_TITLE_TEXT);
    lv_obj_set_style_text_color(info->mode_switch_guide.title,
        lv_color_hex(GUIDE_MODE_SWITCH_GUIDE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->mode_switch_guide.title, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(info->mode_switch_guide.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(info->mode_switch_guide.title,
        GUIDE_MODE_SWITCH_GUIDE_TITLE_LINE_SPACE, 0);
    lv_obj_set_style_text_opa(info->mode_switch_guide.title, LV_OPA_COVER, 0);
    lv_obj_align(info->mode_switch_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SWITCH_GUIDE_TITLE_Y);
    lv_obj_move_foreground(info->mode_switch_guide.title);
}

static lv_draw_buf_t *guide_mode_switch_guide_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->mode_switch_guide.tip_buf != NULL) {
        return info->mode_switch_guide.tip_buf;
    }
    tip = ui_font_img_guide_skip_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->mode_switch_guide.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->mode_switch_guide.tip_buf;
}

static void guide_mode_switch_guide_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_mode_switch_guide_info_t *page = &info->mode_switch_guide;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (page->tip_img != NULL && lv_obj_is_valid(page->tip_img) &&
        page->tip_shine.mask != NULL && lv_obj_is_valid(page->tip_shine.mask) &&
        page->tip_shine.shine != NULL && lv_obj_is_valid(page->tip_shine.shine)) {
        return;
    }

    predecoded = guide_mode_switch_guide_tip_predecode(info);

    if (page->tip_img == NULL || !lv_obj_is_valid(page->tip_img)) {
        page->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(page->tip_img);
        lv_obj_clear_flag(page->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(page->tip_img, predecoded);
            page->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(page->tip_img, GUIDE_MODE_SWITCH_GUIDE_TIP_BASE_OPA, 0);
        lv_obj_align(page->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SWITCH_GUIDE_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_MODE_SWITCH_GUIDE_TIP_Y;
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
void guide_mode_switch_guide_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_mode_switch_guide_bg_detach(info);
    ui_shine_remove(&info->mode_switch_guide.tip_shine);
    if (info->mode_switch_guide.tip_img != NULL && lv_obj_is_valid(info->mode_switch_guide.tip_img)) {
        lv_image_set_src(info->mode_switch_guide.tip_img, NULL);
    }
    if (info->mode_switch_guide.tip_buf != NULL) {
        lv_draw_buf_destroy(info->mode_switch_guide.tip_buf);
        info->mode_switch_guide.tip_buf = NULL;
    }
    if (info->mode_switch_guide.icon != NULL && lv_obj_is_valid(info->mode_switch_guide.icon)) {
        lv_image_set_src(info->mode_switch_guide.icon, NULL);
    }
    if (info->mode_switch_guide.icon_buf != NULL) {
        lv_draw_buf_destroy(info->mode_switch_guide.icon_buf);
        info->mode_switch_guide.icon_buf = NULL;
    }
    memset(&info->mode_switch_guide, 0, sizeof(info->mode_switch_guide));
}

void guide_mode_switch_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_mode_switch_guide_bg_create(info);
    guide_mode_switch_guide_icon_create(info);
    guide_page_num_ui_build(&info->page_num, guide_page_parent_get(info),
        GUIDE_MODE_SWITCH_GUIDE_PAGE_NUM_CURRENT, GUIDE_MODE_SWITCH_GUIDE_PAGE_NUM_TOTAL);
    guide_mode_switch_guide_title_create(info);
    guide_mode_switch_guide_tip_rebuild(info);
}

void guide_mode_switch_guide_ui_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(info);
    if (info->force_update) {
        guide_mode_switch_guide_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_MODE_SWITCH_GUIDE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->mode_switch_guide.tip_shine, &style,
        info->mode_switch_guide.tip_img, 0);
}
