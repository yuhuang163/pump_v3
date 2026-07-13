/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_GUIDE_COMP_BG
#define GUIDE_ANIM_TEXT_COLOR                      (0x00FF00)
#else
#define GUIDE_ANIM_TEXT_COLOR                      (0xBD4060)
#endif
#define GUIDE_ANIM_TEXT_Y                          (243)
#define GUIDE_ANIM_FRAME_RES_INVALID               (0xFFFF)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_animation_frame_show(guide_base_ui_info_t *info, uint16_t frame_index)
{
    if (frame_index >= BASE_GUIDE_ANIM_NUM) {
        frame_index = BASE_GUIDE_ANIM_NUM - 1;
    }

    const uint16_t frame_id = BASE_RES_ID_GUIDE_ANIM_START + frame_index;
    if (info->animation.shown_frame_res_id == frame_id) {
        return;
    }

    if (info->animation.frame == NULL || !lv_obj_is_valid(info->animation.frame)) {
        info->animation.frame = lv_image_create(guide_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->animation.frame);
        lv_obj_align(info->animation.frame, LV_ALIGN_CENTER, 0, 0);
    }

    if (info->animation.frame_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->animation.frame_info);
        memset(&info->animation.frame_info, 0, sizeof(info->animation.frame_info));
    }

    ui_theme_img_resource_init(info->ui, frame_id, RES_THEME_TYPE_BASE, &info->animation.frame_info);
    EXCEPTION_CHECK_VOID(info->animation.frame_info.data);

    lv_fs_path_ex_t img_path;
    lv_fs_make_path_from_buffer(&img_path, LV_FS_MEMFS_LETTER,
        info->animation.frame_info.data, info->animation.frame_info.size, "jpg");
    lv_image_set_src(info->animation.frame, (const void *)&img_path);
    info->animation.shown_frame_res_id = frame_id;
}

static void guide_animation_text_create(guide_base_ui_info_t *info)
{
    if (info->animation.text == NULL || !lv_obj_is_valid(info->animation.text)) {
        info->animation.text = lv_label_create(guide_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->animation.text);
        lv_obj_set_style_text_color(info->animation.text, lv_color_hex(GUIDE_ANIM_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(info->animation.text, &lv_font_aeonik_soft_24_semibold, 0);
        lv_obj_set_style_text_align(info->animation.text, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(info->animation.text, "Welcome to Momcozy");
        lv_obj_align(info->animation.text, LV_ALIGN_TOP_MID, 0, GUIDE_ANIM_TEXT_Y);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_animation_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    if (info->animation.frame_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->animation.frame_info);
    }
    info->animation.shown_frame_res_id = GUIDE_ANIM_FRAME_RES_INVALID;
}

void guide_pump_animation_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    guide_animation_frame_show(info, 0);
    guide_animation_text_create(info);
}

void guide_pump_animation_ui_update(guide_base_ui_info_t *info, uint16_t frame_index)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    guide_animation_frame_show(info, frame_index);
}
