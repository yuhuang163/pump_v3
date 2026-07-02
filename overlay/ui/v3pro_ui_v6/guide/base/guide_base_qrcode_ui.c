/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_base_qrcode_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    guide_base_ui_info_t *guide_info = guide_base_info_get();

    EXCEPTION_CHECK_VOID(guide_info);
    (void)thiz;
    if (guide_info->qrcode == NULL || !lv_obj_is_valid(guide_info->qrcode)) {
        guide_info->qrcode = lv_qrcode_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->qrcode);
    }

    if (page_index != GUIDE_PAGE_BIND_GUIDE) {
        lv_obj_add_flag(guide_info->qrcode, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(guide_info->qrcode, LV_OBJ_FLAG_HIDDEN);
    lv_qrcode_set_size(guide_info->qrcode, GUIDE_QRCODE_SIZE);
    lv_qrcode_set_dark_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_FG_COLOR));
    lv_qrcode_set_light_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_BG_COLOR));
    lv_qrcode_update(guide_info->qrcode, guide_info->qr_str, strlen(guide_info->qr_str));
    lv_obj_align(guide_info->qrcode, LV_ALIGN_TOP_MID, 0, GUIDE_QRCODE_ALIGN_Y);
}
