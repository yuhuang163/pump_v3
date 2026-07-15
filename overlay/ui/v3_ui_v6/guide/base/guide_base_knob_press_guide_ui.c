/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
#define GUIDE_KNOB_PRESS_GUIDE_TITLE_COLOR         (0x00FF00)
#define GUIDE_KNOB_PRESS_GUIDE_HINT_COLOR          (0x00FF00)
#else
#define GUIDE_KNOB_PRESS_GUIDE_TITLE_COLOR         (0xBD4060)
#define GUIDE_KNOB_PRESS_GUIDE_HINT_COLOR          (0xCA5C77)
#endif
#define GUIDE_KNOB_PRESS_GUIDE_TITLE_Y             (54)
#define GUIDE_KNOB_PRESS_GUIDE_HINT_Y              (90)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_knob_press_guide_bg_create(guide_base_ui_info_t *info)
{
    info->knob_press_guide.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_press_guide.bg);
    /// TODO: 后续替换为旋钮按下引导专用背景资源 ID
    ui_img_png_theme_set_src(info->ui, info->knob_press_guide.bg, BASE_RES_ID_GUIDE_KNOB_PRESS_BG,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->knob_press_guide.bg, LV_ALIGN_CENTER, 0, 0);
}

static void guide_knob_press_guide_title_create(guide_base_ui_info_t *info)
{
    info->knob_press_guide.title = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_press_guide.title);
    lv_obj_set_style_text_color(info->knob_press_guide.title,
        lv_color_hex(GUIDE_KNOB_PRESS_GUIDE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->knob_press_guide.title, &lv_font_aeonik_soft_24_semibold, 0);
    lv_obj_set_style_text_align(info->knob_press_guide.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->knob_press_guide.title, "Display");
    lv_obj_align(info->knob_press_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_KNOB_PRESS_GUIDE_TITLE_Y);
}

static void guide_knob_press_guide_hint_create(guide_base_ui_info_t *info)
{
    info->knob_press_guide.hint = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->knob_press_guide.hint);
    lv_obj_set_style_text_color(info->knob_press_guide.hint,
        lv_color_hex(GUIDE_KNOB_PRESS_GUIDE_HINT_COLOR), 0);
    lv_obj_set_style_text_font(info->knob_press_guide.hint, &lv_font_aeonik_soft_20_medium, 0);
    lv_obj_set_style_text_align(info->knob_press_guide.hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->knob_press_guide.hint, "Press the knob for the next page");
    lv_obj_align(info->knob_press_guide.hint, LV_ALIGN_TOP_MID, 0, GUIDE_KNOB_PRESS_GUIDE_HINT_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_knob_press_guide_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_png_release_decoded_buf(info->knob_press_guide.bg);
}

void guide_knob_press_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    guide_knob_press_guide_bg_create(info);
#endif
    guide_knob_press_guide_title_create(info);
    guide_knob_press_guide_hint_create(info);
}
