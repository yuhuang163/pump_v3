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
#define GUIDE_FINISH_ICON_X                         (148)
#define GUIDE_FINISH_ICON_Y                         (104)
#define GUIDE_FINISH_TITLE_Y                        (192)
#define GUIDE_FINISH_HINT_Y                         (232)
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
#define GUIDE_FINISH_TITLE_COLOR                    (0x00FF00)
#define GUIDE_FINISH_HINT_COLOR                     (0x00FF00)
#else
#define GUIDE_FINISH_TITLE_COLOR                    (0x240F1B)
#define GUIDE_FINISH_HINT_COLOR                     (0x7C6971)
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_finish_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
#if !LV_USE_SNAPSHOT
    ui_img_png_release_decoded_buf(info->finish.bg);
#endif
    ui_img_baj_release_decoded_buf(info->finish.icon);
}

void guide_finish_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    info->finish.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->finish.bg);
    lv_image_set_src(info->finish.bg, background_snapshot);
    lv_obj_align(info->finish.bg, LV_ALIGN_CENTER, 0, 0);
#else
    info->finish.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->finish.bg);
    ui_img_png_theme_set_src(info->ui, info->finish.bg, UI_IMG_RES_ID_NONE,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->finish.bg, LV_ALIGN_CENTER, 0, 0);
#endif
#endif

    info->finish.icon = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->finish.icon);
    ui_img_baj_theme_set_src(info->ui, info->finish.icon, 0,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->finish.icon, LV_ALIGN_TOP_LEFT, GUIDE_FINISH_ICON_X, GUIDE_FINISH_ICON_Y);

    info->finish.title = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->finish.title);
    lv_obj_set_style_text_color(info->finish.title, lv_color_hex(GUIDE_FINISH_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->finish.title, &lv_font_aeonik_soft_28_semibold, 0);
    lv_obj_set_style_text_align(info->finish.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->finish.title, "Guide complete");
    lv_obj_align(info->finish.title, LV_ALIGN_TOP_MID, 0, GUIDE_FINISH_TITLE_Y);

    info->finish.hint = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->finish.hint);
    lv_obj_set_style_text_color(info->finish.hint, lv_color_hex(GUIDE_FINISH_HINT_COLOR), 0);
    lv_obj_set_style_text_font(info->finish.hint, &lv_font_aeonik_soft_24_medium, 0);
    lv_obj_set_style_text_align(info->finish.hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->finish.hint, "Happy pumping !");
    lv_obj_align(info->finish.hint, LV_ALIGN_TOP_MID, 0, GUIDE_FINISH_HINT_Y);
}
