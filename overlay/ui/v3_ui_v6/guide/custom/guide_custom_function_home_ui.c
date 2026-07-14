/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "guide_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_FUNCTION_HOME_DESC              (0)

#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
#define GUIDE_FUNCTION_HOME_TITLE_COLOR            (0x00FF00)
#define GUIDE_FUNCTION_HOME_DESC_COLOR             (0x00FF00)
#define GUIDE_FUNCTION_HOME_HINT_COLOR             (0x00FF00)
#else
#define GUIDE_FUNCTION_HOME_TITLE_COLOR            (0xBD4060)
#define GUIDE_FUNCTION_HOME_DESC_COLOR             (0xCA5C77)
#define GUIDE_FUNCTION_HOME_HINT_COLOR             (0xBD4060)
#endif

#if IS_SUPPORT_FUNCTION_HOME_DESC
#define GUIDE_FUNCTION_HOME_TITLE_Y                (184)
#define GUIDE_FUNCTION_HOME_DESC_Y                 (224)
#define GUIDE_FUNCTION_HOME_HINT_Y                 (304)
#define GUIDE_FUNCTION_HOME_START_ICON_X           (157)
#define GUIDE_FUNCTION_HOME_START_ICON_Y           (296)
#else
#define GUIDE_FUNCTION_HOME_TITLE_Y                (204)
#define GUIDE_FUNCTION_HOME_DESC_Y                 (224)
#define GUIDE_FUNCTION_HOME_HINT_Y                 (254)
#define GUIDE_FUNCTION_HOME_START_ICON_X           (157)
#define GUIDE_FUNCTION_HOME_START_ICON_Y           (246)
#endif
/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_function_home_bg_create(guide_custom_ui_info_t *info)
{
    info->function_home.bg = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->function_home.bg);
    ui_img_png_theme_set_src(info->ui, info->function_home.bg, CUSTOM_RES_ID_GUIDE_BG,
        RES_THEME_TYPE_CUSTOM, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->function_home.bg, LV_ALIGN_CENTER, 0, 0);
}

static void guide_function_home_title_create(guide_custom_ui_info_t *info)
{
    info->function_home.title = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->function_home.title);
    lv_obj_set_style_text_color(info->function_home.title, lv_color_hex(GUIDE_FUNCTION_HOME_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->function_home.title, &lv_font_aeonik_soft_24_semibold, 0);
    lv_obj_set_style_text_align(info->function_home.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->function_home.title, "Pumping walkthrough");
    lv_obj_align(info->function_home.title, LV_ALIGN_TOP_MID, 0, GUIDE_FUNCTION_HOME_TITLE_Y);
}

#if IS_SUPPORT_FUNCTION_HOME_DESC
static void guide_function_home_desc_create(guide_custom_ui_info_t *info)
{
    info->function_home.desc = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->function_home.desc);
    lv_obj_set_style_text_color(info->function_home.desc, lv_color_hex(GUIDE_FUNCTION_HOME_DESC_COLOR), 0);
    lv_obj_set_style_text_font(info->function_home.desc, &lv_font_aeonik_soft_20_medium, 0);
    lv_obj_set_style_text_align(info->function_home.desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->function_home.desc, "Next, get familiar with each\npumping step");
    lv_obj_set_style_text_line_space(info->function_home.desc, 4, 0);
    lv_obj_align(info->function_home.desc, LV_ALIGN_TOP_MID, 0, GUIDE_FUNCTION_HOME_DESC_Y);
}
#endif

static void guide_function_home_hint_create(guide_custom_ui_info_t *info)
{
    info->function_home.hint = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->function_home.hint);
    lv_obj_set_style_text_color(info->function_home.hint, lv_color_hex(GUIDE_FUNCTION_HOME_HINT_COLOR), 0);
    lv_obj_set_style_text_font(info->function_home.hint, &lv_font_aeonik_soft_20_medium, 0);
    lv_obj_set_style_text_align(info->function_home.hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->function_home.hint, "Press         to start");
    lv_obj_align(info->function_home.hint, LV_ALIGN_TOP_MID, 0, GUIDE_FUNCTION_HOME_HINT_Y);
}

static void guide_function_home_start_icon_create(guide_custom_ui_info_t *info)
{
    info->function_home.start_icon = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->function_home.start_icon);
    ui_img_baj_theme_set_src(info->ui, info->function_home.start_icon, CUSTOM_RES_ID_GUIDE_START_BUTTON_ICON,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->function_home.start_icon, LV_ALIGN_TOP_LEFT,
        GUIDE_FUNCTION_HOME_START_ICON_X, GUIDE_FUNCTION_HOME_START_ICON_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_function_home_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_png_release_decoded_buf(info->function_home.bg);
    ui_img_baj_release_decoded_buf(info->function_home.start_icon);
}

void guide_custom_function_home_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    guide_function_home_bg_create(info);
#endif
    guide_function_home_title_create(info);
#if IS_SUPPORT_FUNCTION_HOME_DESC
    guide_function_home_desc_create(info);
#endif
    guide_function_home_hint_create(info);
    guide_function_home_start_icon_create(info);
}
