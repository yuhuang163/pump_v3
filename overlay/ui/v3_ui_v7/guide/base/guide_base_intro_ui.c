/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
#define GUIDE_INTRO_TEXT_COLOR                     (0x00FF00)
#else
#define GUIDE_INTRO_TEXT_COLOR                     (0xBD4060)
#endif
#define GUIDE_INTRO_HOME_ICON_X                    (143)
#define GUIDE_INTRO_HOME_ICON_Y                    (92)
#define GUIDE_INTRO_TITLE_Y                        (200)
#define GUIDE_INTRO_HINT_Y                         (248)
#define GUIDE_INTRO_START_ICON_X                   (155)
#define GUIDE_INTRO_START_ICON_Y                   (240)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_intro_bg_create(guide_base_ui_info_t *info)
{
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    info->intro.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->intro.bg);
    lv_image_set_src(info->intro.bg, background_snapshot);
    lv_obj_align(info->intro.bg, LV_ALIGN_CENTER, 0, 0);
#else
    info->intro.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->intro.bg);
    ui_img_png_theme_set_src(info->ui, info->intro.bg, UI_IMG_RES_ID_NONE,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->intro.bg, LV_ALIGN_CENTER, 0, 0);
#endif
}

static void guide_intro_home_icon_create(guide_base_ui_info_t *info)
{
    info->intro.home_icon = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->intro.home_icon);
    ui_img_png_theme_set_src(info->ui, info->intro.home_icon, UI_IMG_RES_ID_NONE,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->intro.home_icon, LV_ALIGN_TOP_LEFT,
        GUIDE_INTRO_HOME_ICON_X, GUIDE_INTRO_HOME_ICON_Y);
}

static void guide_intro_title_create(guide_base_ui_info_t *info)
{
    info->intro.title = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->intro.title);
    lv_obj_set_style_text_color(info->intro.title, lv_color_hex(GUIDE_INTRO_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->intro.title, &lv_font_aeonik_soft_28_semibold, 0);
    lv_obj_set_style_text_align(info->intro.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->intro.title, "Get to know V3 in 2 mins");
    lv_obj_align(info->intro.title, LV_ALIGN_TOP_MID, 0, GUIDE_INTRO_TITLE_Y);
}

static void guide_intro_hint_create(guide_base_ui_info_t *info)
{
    info->intro.hint = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->intro.hint);
    lv_obj_set_style_text_color(info->intro.hint, lv_color_hex(GUIDE_INTRO_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->intro.hint, &lv_font_aeonik_soft_24_medium, 0);
    lv_obj_set_style_text_align(info->intro.hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->intro.hint, "Press         to start");
    lv_obj_align(info->intro.hint, LV_ALIGN_TOP_MID, 0, GUIDE_INTRO_HINT_Y);
}

static void guide_intro_start_icon_create(guide_base_ui_info_t *info)
{
    info->intro.start_icon = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->intro.start_icon);
    // ui_img_baj_theme_set_src(info->ui, info->intro.start_icon, BASE_RES_ID_GUIDE_START_BUTTON_ICON,
    //     RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->intro.start_icon, LV_ALIGN_TOP_LEFT,
        GUIDE_INTRO_START_ICON_X, GUIDE_INTRO_START_ICON_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_intro_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
#if !LV_USE_SNAPSHOT
    ui_img_png_release_decoded_buf(info->intro.bg);
#endif
    ui_img_png_release_decoded_buf(info->intro.home_icon);
    ui_img_baj_release_decoded_buf(info->intro.start_icon);
}

void guide_intro_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    guide_intro_bg_create(info);
#endif
    guide_intro_home_icon_create(info);
    guide_intro_title_create(info);
    // guide_intro_hint_create(info);
    guide_intro_start_icon_create(info);
}
