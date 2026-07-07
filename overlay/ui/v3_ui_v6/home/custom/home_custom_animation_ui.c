/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_img_baj_decode.h" // IWYU pragma: keep
#include "home_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_IP_ANIM_X                             (0)
#define HOME_IP_ANIM_Y                             (214)
#define HOME_ANIM_FRAME_INTERVAL_MS                (1000 / HOME_ANIMATION_FREQ)
#define HOME_ANIM_FRAME_RES_INVALID                (0xFFFF)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_animation_state_reset(home_custom_animation_info_t *anim)
{
    anim->frame_res_id = CUSTOM_RES_ID_HD_ANIM_IP_START;
    anim->frame_end_res_id = CUSTOM_RES_ID_HD_ANIM_IP_END;
    anim->shown_frame_res_id = HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    anim->initialized = 0;
}

static void home_animation_res_unload(home_custom_ui_info_t *info, res_img_info_t *img_info)
{
    resource_manager_context_t *res = info->ui ? info->ui->priv.res : NULL;

    if (res != NULL && img_info->data != NULL) {
        res->res_data_unload(img_info);
        return;
    }
    memset(img_info, 0, sizeof(*img_info));
}

static void home_animation_ip_prepare(home_custom_ui_info_t *info)
{
    if (info->animation.ip == NULL || !lv_obj_is_valid(info->animation.ip)) {
        info->animation.ip = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->animation.ip);
        lv_obj_align(info->animation.ip, LV_ALIGN_TOP_MID, HOME_IP_ANIM_X, HOME_IP_ANIM_Y);
    }
}

static void home_animation_frame_show(home_custom_ui_info_t *info, uint16_t frame_id)
{
    home_custom_animation_info_t *anim = &info->animation;
    res_img_info_t img_info = {0};

    if (anim->shown_frame_res_id == frame_id) {
        return;
    }

    ui_theme_img_resource_init(info->ui, frame_id, RES_THEME_TYPE_CUSTOM, &img_info);
    if (img_info.data == NULL) {
        return;
    }

    ui_img_baj_release_decoded_buf(anim->ip);
    if (ui_img_baj_set_src(anim->ip, img_info.data, img_info.size, UI_IMG_BAJ_DECODE_RGBA8888) != 0) {
        LOG_UI("home_anim: set_src failed frame=%u size=%u", frame_id, img_info.size);
        home_animation_res_unload(info, &img_info);
        return;
    }

    home_animation_res_unload(info, &img_info);
    anim->shown_frame_res_id = frame_id;
}

static void home_animation_apply(home_custom_ui_info_t *info)
{
    home_custom_animation_info_t *anim = &info->animation;

    home_animation_ip_prepare(info);
    anim->frame_res_id = CUSTOM_RES_ID_HD_ANIM_IP_START;
    anim->frame_end_res_id = CUSTOM_RES_ID_HD_ANIM_IP_END;
    anim->shown_frame_res_id = HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    home_animation_frame_show(info, anim->frame_res_id);
    lv_obj_clear_flag(anim->ip, LV_OBJ_FLAG_HIDDEN);
    anim->initialized = 1;
}

static void home_animation_frame_update(home_custom_ui_info_t *info)
{
    home_custom_animation_info_t *anim = &info->animation;
    const uint32_t now = get_timestamp();

    if (anim->last_frame_tick_ms != 0 &&
        (now - anim->last_frame_tick_ms) < HOME_ANIM_FRAME_INTERVAL_MS) {
        return;
    }
    anim->last_frame_tick_ms = now;

    if (anim->frame_res_id > anim->frame_end_res_id) {
        anim->frame_res_id = CUSTOM_RES_ID_HD_ANIM_IP_START;
    }

    home_animation_frame_show(info, anim->frame_res_id);
    anim->frame_res_id++;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_animation_ui_remove(home_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->animation.ip);
    home_animation_res_unload(info, &info->animation.ip_info);
    home_animation_state_reset(&info->animation);
}

void home_custom_animation_ui_update(home_custom_ui_info_t *info)
{
    home_custom_animation_info_t *anim = &info->animation;

    EXCEPTION_CHECK_VOID(info);
    if (info->force_update || anim->initialized == 0) {
        home_animation_apply(info);
    }
    home_animation_frame_update(info);
}
