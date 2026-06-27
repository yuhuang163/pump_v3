/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_GUIDE_COMP_BG
static PSRAM_DATA_LOC uint8_t guide_bg_data[] = {
    #include "comparison_png2txt/guide_mode_select_360x360.txt"
};
#endif

const static uint16_t guide_page_img_ids[GUIDE_PAGE_IMG_NUM] = {
    BASE_RES_ID_GUIDE_PAGE1_IMG,
    BASE_RES_ID_GUIDE_PAGE2_IMG,
    BASE_RES_ID_GUIDE_PAGE3_IMG,
    BASE_RES_ID_GUIDE_PAGE4_IMG,
    BASE_RES_ID_GUIDE_PAGE5_IMG,
    BASE_RES_ID_GUIDE_PAGE6_IMG,
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_base_page_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    guide_base_ui_info_t *guide_info = guide_base_info_get();

    EXCEPTION_CHECK_VOID(guide_info);
    if (page_index >= GUIDE_PAGE_IMG_NUM) {
        LOG_UI("===> guide page index out of range: %d", page_index);
        return;
    }

    if (guide_info->page_img == NULL || !lv_obj_is_valid(guide_info->page_img)) {
        guide_info->page_img = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->page_img);
        lv_obj_align(guide_info->page_img, LV_ALIGN_CENTER, 0, 0);
    }
#if IS_SUPPORT_GUIDE_COMP_BG
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(guide_info->page_img, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->page_img, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, guide_bg_data,
            sizeof(guide_bg_data), "png");
        lv_image_set_src(guide_info->page_img, (const void *)&bg_img_path);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
#else
    ui_theme_img_resource_init(thiz, guide_page_img_ids[page_index], RES_THEME_TYPE_BASE,
        &guide_info->img_info);
    EXCEPTION_CHECK_VOID(guide_info->img_info.data);
    if (ui_img_png_set_src(guide_info->page_img, guide_info->img_info.data,
            guide_info->img_info.size, UI_IMG_PNG_DECODE_RGBA8888) != 0) {
        LOG_UI("guide png sw decode failed");
    }
#endif
}
