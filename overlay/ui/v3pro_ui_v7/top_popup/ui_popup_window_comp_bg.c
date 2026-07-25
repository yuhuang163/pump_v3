/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

#if IS_SUPPORT_POPUP_WINDOW_COMP_BG

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static PSRAM_NC_DATA_LOC uint8_t popup_window_comp_bg_data[] = {
    // #include "comparison_png2txt/sys_error_360x360.txt"
    // #include "comparison_png2txt/auto_power_off_popup_360x360.txt"
    // #include "comparison_png2txt/count_down_360x360.txt"
    // #include "comparison_png2txt/ble_disconnect_popup_360x360.txt"
    #include "comparison_png2txt/ble_connected_popup_360x360.txt"
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_comp_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    popup_window_disp_info_t *info = popup_window_info_get();

    (void)thiz;
    EXCEPTION_CHECK_VOID(info && info->root.obj);
    if (info->root.comp_bg == NULL || !lv_obj_is_valid(info->root.comp_bg)) {
        info->root.comp_bg = lv_image_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->root.comp_bg);
        lv_obj_move_background(info->root.comp_bg);
        ui_img_png_set_src(info->root.comp_bg, popup_window_comp_bg_data,
            sizeof(popup_window_comp_bg_data), UI_IMG_PNG_DECODE_RGBA8888);
        lv_obj_align(info->root.comp_bg, LV_ALIGN_CENTER, 0, 0);
    }
    if (status == 0) {
        lv_obj_add_flag(info->root.comp_bg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(info->root.comp_bg, LV_OBJ_FLAG_HIDDEN);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}

#endif
