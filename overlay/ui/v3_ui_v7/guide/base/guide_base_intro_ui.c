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
#define GUIDE_INTRO_BG_RES_ID                       (BASE_RES_ID_SIMULATION_MODE_BG)
#define GUIDE_INTRO_TITLE_COLOR                     (0xFFFFFF)
#define GUIDE_INTRO_TITLE_LINE_SPACE                (8)
#define GUIDE_INTRO_TITLE_TEXT                      "Let's get started!"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_intro_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->intro.bg == NULL || !lv_obj_is_valid(info->intro.bg)) {
        return;
    }
    src = lv_image_get_src(info->intro.bg);
    if (src == NULL) {
        return;
    }
    /* 预解码背景由 snapshot 模块持有，页面只解绑，不 destroy */
    if (get_predecoded_bg(info->intro.shown_bg_id) == src) {
        lv_image_set_src(info->intro.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->intro.bg);
}

static void guide_intro_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->intro.bg == NULL || !lv_obj_is_valid(info->intro.bg)) {
        info->intro.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->intro.bg);
        lv_obj_move_background(info->intro.bg);
    }

    guide_intro_bg_detach(info);
    predecoded = get_predecoded_bg(GUIDE_INTRO_BG_RES_ID);
    if (predecoded != NULL) {
        lv_image_set_src(info->intro.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->intro.bg, GUIDE_INTRO_BG_RES_ID,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->intro.bg, LV_ALIGN_CENTER, 0, 0);
    info->intro.shown_bg_id = GUIDE_INTRO_BG_RES_ID;
}

static void guide_intro_title_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->intro.title != NULL && lv_obj_is_valid(info->intro.title)) {
        return;
    }

    info->intro.title = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(info->intro.title);
    lv_label_set_text(info->intro.title, GUIDE_INTRO_TITLE_TEXT);
    lv_obj_set_style_text_color(info->intro.title, lv_color_hex(GUIDE_INTRO_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->intro.title, &lv_font_aeonik_soft_29_medium, 0);
    lv_obj_set_style_text_line_space(info->intro.title, GUIDE_INTRO_TITLE_LINE_SPACE, 0);
    lv_obj_set_style_text_align(info->intro.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(info->intro.title, LV_OPA_COVER, 0);
    lv_obj_align(info->intro.title, LV_ALIGN_CENTER, 0, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_intro_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_intro_bg_detach(info);
    memset(&info->intro, 0, sizeof(info->intro));
}

void guide_intro_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_intro_bg_create(info);
    guide_intro_title_create(info);
}
