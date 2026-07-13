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
#if IS_SUPPORT_ROOT_GUIDE_COMP_BG
#define GUIDE_KNOB_GUIDE_TITLE_COLOR               (0x00FF00)
#define GUIDE_KNOB_GUIDE_HINT_COLOR                (0x00FF00)
#else
#define GUIDE_KNOB_GUIDE_TITLE_COLOR               (0xBD4060)
#define GUIDE_KNOB_GUIDE_HINT_COLOR                (0xCA5C77)
#endif
#define GUIDE_KNOB_GUIDE_TITLE_Y                   (54)
#define GUIDE_KNOB_GUIDE_HINT_Y                    (90)
#define GUIDE_KNOB_GUIDE_ICON_X                    (48)
#define GUIDE_KNOB_GUIDE_ICON_Y                    (137)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_knob_guide_bg_create(guide_base_ui_info_t *info)
{
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    info->knob_guide.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_guide.bg);
    lv_image_set_src(info->knob_guide.bg, background_snapshot);
    lv_obj_align(info->knob_guide.bg, LV_ALIGN_CENTER, 0, 0);
#else
    info->knob_guide.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_guide.bg);
    ui_img_png_theme_set_src(info->ui, info->knob_guide.bg, BASE_RES_ID_COMMON_BG,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->knob_guide.bg, LV_ALIGN_CENTER, 0, 0);
#endif
}

static void guide_knob_guide_title_create(guide_base_ui_info_t *info)
{
    info->knob_guide.title = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_guide.title);
    lv_obj_set_style_text_color(info->knob_guide.title, lv_color_hex(GUIDE_KNOB_GUIDE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->knob_guide.title, &lv_font_aeonik_soft_24_semibold, 0);
    lv_obj_set_style_text_align(info->knob_guide.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->knob_guide.title, "Display");
    lv_obj_align(info->knob_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_KNOB_GUIDE_TITLE_Y);
}

static void guide_knob_guide_hint_create(guide_base_ui_info_t *info)
{
    info->knob_guide.hint = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_guide.hint);
    lv_obj_set_style_text_color(info->knob_guide.hint, lv_color_hex(GUIDE_KNOB_GUIDE_HINT_COLOR), 0);
    lv_obj_set_style_text_font(info->knob_guide.hint, &lv_font_aeonik_soft_20_medium, 0);
    lv_obj_set_style_text_align(info->knob_guide.hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->knob_guide.hint, "Rotate the knob");
    lv_obj_align(info->knob_guide.hint, LV_ALIGN_TOP_MID, 0, GUIDE_KNOB_GUIDE_HINT_Y);
}

static void guide_knob_guide_icon_create(guide_base_ui_info_t *info)
{
    info->knob_guide.icon = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_guide.icon);
    ui_img_baj_theme_set_src(info->ui, info->knob_guide.icon, BASE_RES_ID_GUIDE_KNOB_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->knob_guide.icon, LV_ALIGN_TOP_LEFT, GUIDE_KNOB_GUIDE_ICON_X, GUIDE_KNOB_GUIDE_ICON_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_knob_guide_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
#if !LV_USE_SNAPSHOT
    ui_img_png_release_decoded_buf(info->knob_guide.bg);
#endif
    ui_img_baj_release_decoded_buf(info->knob_guide.icon);
}

void guide_knob_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
#if (IS_SUPPORT_ROOT_GUIDE_COMP_BG == 0)
    guide_knob_guide_bg_create(info);
#endif
    guide_knob_guide_title_create(info);
    guide_knob_guide_hint_create(info);
    guide_knob_guide_icon_create(info);
}
