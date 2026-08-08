/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_PUMP_PAUSE_GUIDE_BG_RES_ID                        (BASE_RES_ID_SIMULATION_MODE_BG)

#define GUIDE_PUMP_PAUSE_GUIDE_PAGE_NUM_CURRENT                 (2)
#define GUIDE_PUMP_PAUSE_GUIDE_PAGE_NUM_TOTAL                   (7)

#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_Y                          (79)
#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_COLOR                      (0xFFFFFF)
#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_TEXT                       "Press the knob to\npause."
#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_LINE_SPACE                 (8)

#define GUIDE_PUMP_PAUSE_GUIDE_ICON_Y                           (169)
#define GUIDE_PUMP_PAUSE_GUIDE_ICON_RES_ID                      (BASE_RES_ID_GUIDE_POWER_KEY_ICON)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_pump_pause_guide_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->pump_pause_guide.bg == NULL || !lv_obj_is_valid(info->pump_pause_guide.bg)) {
        return;
    }
    src = lv_image_get_src(info->pump_pause_guide.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->pump_pause_guide.shown_bg_id) == src) {
        lv_image_set_src(info->pump_pause_guide.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->pump_pause_guide.bg);
}

static void guide_pump_pause_guide_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_pause_guide.bg == NULL || !lv_obj_is_valid(info->pump_pause_guide.bg)) {
        info->pump_pause_guide.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause_guide.bg);
        lv_obj_move_background(info->pump_pause_guide.bg);
    }

    guide_pump_pause_guide_bg_detach(info);
    predecoded = get_predecoded_bg(GUIDE_PUMP_PAUSE_GUIDE_BG_RES_ID);
    if (predecoded != NULL) {
        lv_image_set_src(info->pump_pause_guide.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->pump_pause_guide.bg, GUIDE_PUMP_PAUSE_GUIDE_BG_RES_ID,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_pause_guide.bg, LV_ALIGN_CENTER, 0, 0);
    info->pump_pause_guide.shown_bg_id = GUIDE_PUMP_PAUSE_GUIDE_BG_RES_ID;
}

static void guide_pump_pause_guide_icon_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->pump_pause_guide.icon == NULL || !lv_obj_is_valid(info->pump_pause_guide.icon)) {
        info->pump_pause_guide.icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause_guide.icon);
    }

    if (info->pump_pause_guide.icon_buf == NULL) {
        info->pump_pause_guide.icon_buf = ui_img_baj_theme_decode_draw_buf(info->ui,
            GUIDE_PUMP_PAUSE_GUIDE_ICON_RES_ID, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->pump_pause_guide.icon_buf != NULL) {
        lv_image_set_src(info->pump_pause_guide.icon, info->pump_pause_guide.icon_buf);
    } else {
        ui_img_baj_theme_set_src(info->ui, info->pump_pause_guide.icon, GUIDE_PUMP_PAUSE_GUIDE_ICON_RES_ID,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_pause_guide.icon, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_PAUSE_GUIDE_ICON_Y);
}

static void guide_pump_pause_guide_title_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_pause_guide.title == NULL || !lv_obj_is_valid(info->pump_pause_guide.title)) {
        info->pump_pause_guide.title = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause_guide.title);
    }
    lv_label_set_text(info->pump_pause_guide.title, GUIDE_PUMP_PAUSE_GUIDE_TITLE_TEXT);
    lv_obj_set_style_text_color(info->pump_pause_guide.title, lv_color_hex(GUIDE_PUMP_PAUSE_GUIDE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->pump_pause_guide.title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(info->pump_pause_guide.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(info->pump_pause_guide.title, LV_OPA_COVER, 0);
    lv_obj_align(info->pump_pause_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_PAUSE_GUIDE_TITLE_Y);
    lv_obj_set_style_text_line_space(info->pump_pause_guide.title, GUIDE_PUMP_PAUSE_GUIDE_TITLE_LINE_SPACE, 0);
    lv_obj_move_foreground(info->pump_pause_guide.title);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_pause_guide_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_pause_guide_bg_detach(info);
    if (info->pump_pause_guide.icon != NULL && lv_obj_is_valid(info->pump_pause_guide.icon)) {
        lv_image_set_src(info->pump_pause_guide.icon, NULL);
    }
    if (info->pump_pause_guide.icon_buf != NULL) {
        lv_draw_buf_destroy(info->pump_pause_guide.icon_buf);
        info->pump_pause_guide.icon_buf = NULL;
    }
    memset(&info->pump_pause_guide, 0, sizeof(info->pump_pause_guide));
}

void guide_pump_pause_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_pause_guide_bg_create(info);
    guide_pump_pause_guide_icon_create(info);
    guide_page_num_ui_build(&info->page_num, guide_page_parent_get(info),
        GUIDE_PUMP_PAUSE_GUIDE_PAGE_NUM_CURRENT, GUIDE_PUMP_PAUSE_GUIDE_PAGE_NUM_TOTAL);
    guide_pump_pause_guide_title_create(info);
}
