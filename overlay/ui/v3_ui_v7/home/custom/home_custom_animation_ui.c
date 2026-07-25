/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "home_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_IP_ANIM_X                             (0)
#define HOME_IP_ANIM_Y                             (214)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_animation_update_frame(home_custom_ui_info_t *info)
{
    home_custom_animation_info_t *anim = &info->animation;
    ui_manager_context_t *thiz = info->ui;
    const uint16_t start_resource_id = CUSTOM_RES_ID_HD_ANIM_IP_START;
    const uint16_t animation_size =
        CUSTOM_RES_ID_HD_ANIM_IP_END - CUSTOM_RES_ID_HD_ANIM_IP_START + 1;

    if (anim->ip == NULL || !lv_obj_is_valid(anim->ip)) {
        anim->ip = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(anim->ip);
    }

    if (anim->animation_index >= animation_size) {
        anim->animation_index = 0;
    }

    lv_obj_align(anim->ip, LV_ALIGN_TOP_MID, HOME_IP_ANIM_X, HOME_IP_ANIM_Y);
    if (anim->ip_info.data != NULL) {
        ui_img_resource_deinit(thiz, &anim->ip_info);
        memset(&anim->ip_info, 0, sizeof(anim->ip_info));
    }
    ui_theme_img_resource_init(thiz, start_resource_id + anim->animation_index,
        RES_THEME_TYPE_CUSTOM, &anim->ip_info);
    EXCEPTION_CHECK_VOID(anim->ip_info.data);

    lv_fs_path_ex_t animation_img_path;
    lv_fs_make_path_from_buffer(&animation_img_path, LV_FS_MEMFS_LETTER,
        anim->ip_info.data, anim->ip_info.size, "baj");
    lv_image_set_src(anim->ip, (const void *)&animation_img_path);
    lv_obj_clear_flag(anim->ip, LV_OBJ_FLAG_HIDDEN);
    anim->animation_index++;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_animation_ui_remove(home_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    if (info->animation.ip_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->animation.ip_info);
    }
    memset(&info->animation, 0, sizeof(info->animation));
}

void home_custom_animation_ui_update(home_custom_ui_info_t *info)
{
    home_custom_animation_info_t *anim = &info->animation;

    EXCEPTION_CHECK_VOID(info);
    if (info->force_update || anim->initialized == 0) {
        anim->animation_index = 0;
        anim->initialized = 1;
    }
    home_animation_update_frame(info);
}
