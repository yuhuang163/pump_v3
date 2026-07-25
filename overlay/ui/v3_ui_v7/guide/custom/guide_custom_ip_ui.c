/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_CUSTOM_IP_X                            (0)
#define GUIDE_CUSTOM_IP_Y                            (214)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_custom_ip_show_at(guide_custom_ui_info_t *info, lv_coord_t x, lv_coord_t y)
{
    ui_manager_context_t *thiz = info->ui;
    lv_obj_t *parent = guide_custom_page_parent_get(info);

    EXCEPTION_CHECK_VOID(thiz && parent);
    if (info->ip.ip != NULL && (!lv_obj_is_valid(info->ip.ip) ||
        lv_obj_get_parent(info->ip.ip) != parent)) {
        ui_img_baj_release_decoded_buf(info->ip.ip);
        info->ip.ip = NULL;
    }

    if (info->ip.ip == NULL || !lv_obj_is_valid(info->ip.ip)) {
        info->ip.ip = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->ip.ip);
    }

    const void *src = lv_image_get_src(info->ip.ip);
    if (src != NULL && info->force_update == false) {
        lv_obj_align(info->ip.ip, LV_ALIGN_TOP_MID, x, y);
        lv_obj_clear_flag(info->ip.ip, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(info->ip.ip);
        return;
    }

    ui_img_baj_release_decoded_buf(info->ip.ip);
    ui_img_baj_theme_set_src(thiz, info->ip.ip, CUSTOM_RES_ID_HD_ANIM_IP_START,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->ip.ip, LV_ALIGN_TOP_MID, x, y);
    lv_obj_clear_flag(info->ip.ip, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info->ip.ip);
}

static void guide_custom_ip_show(guide_custom_ui_info_t *info)
{
    guide_custom_ip_show_at(info, GUIDE_CUSTOM_IP_X, GUIDE_CUSTOM_IP_Y);
}

static void guide_custom_ip_hide(guide_custom_ui_info_t *info)
{
    if (info->ip.ip != NULL && lv_obj_is_valid(info->ip.ip)) {
        lv_obj_add_flag(info->ip.ip, LV_OBJ_FLAG_HIDDEN);
    }
}

static bool guide_custom_page_show_ip(uint8_t page_index)
{
    return GUIDE_PAGE_SHOW_LEVEL(page_index) ||
        page_index == GUIDE_PAGE_PUMP_STOP_GUIDE;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_ip_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    ui_img_baj_release_decoded_buf(info->ip.ip);
    info->ip.ip = NULL;
}

void guide_custom_ip_ui_on_page_reset(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    ui_img_baj_release_decoded_buf(info->ip.ip);
    info->ip.ip = NULL;
}

void guide_custom_ip_ui_update(guide_custom_ui_info_t *info, uint8_t page_index)
{
    EXCEPTION_CHECK_VOID(info);
    if (page_index == GUIDE_PAGE_FINISH) {
        guide_custom_ip_show_at(info, GUIDE_CUSTOM_FINISH_IP_X, GUIDE_CUSTOM_FINISH_IP_Y);
    } else if (guide_custom_page_show_ip(page_index)) {
        guide_custom_ip_show(info);
    } else {
        guide_custom_ip_hide(info);
    }
}
