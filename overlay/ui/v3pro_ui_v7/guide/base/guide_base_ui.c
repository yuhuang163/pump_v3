/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t guide_bg_data[] = {
    /* 对比图：guide_base_p70.png（二维码页），需先转 comparison_png2txt */
    #include "comparison_png2txt/guide_base_p62_360x360.txt"
};
#endif

static guide_base_ui_info_t *guide_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_base_root_init(void)
{
    if (guide_info->root == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(guide_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(guide_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(guide_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(guide_info->root, 0, 0);
    lv_obj_set_style_pad_all(guide_info->root, 0, 0);
    lv_obj_align(guide_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_BASE_GUIDE_COMP_BG
static void guide_root_comp_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    lv_fs_path_ex_t bg_img_path;

    if (guide_info->comp_bg == NULL || !lv_obj_is_valid(guide_info->comp_bg)) {
        guide_info->comp_bg = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->comp_bg);
        lv_obj_align(guide_info->comp_bg, LV_ALIGN_CENTER, 0, 0);
        lv_obj_move_background(guide_info->comp_bg);
    }

    if (status == 0) {
        lv_obj_add_flag(guide_info->comp_bg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->comp_bg, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, guide_bg_data,
            sizeof(guide_bg_data), "png");
        lv_image_set_src(guide_info->comp_bg, (const void *)&bg_img_path);
        lv_obj_move_background(guide_info->comp_bg);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
    (void)thiz;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_base_ui_info_t *)u_malloz(sizeof(guide_base_ui_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    guide_base_root_init();
    guide_info->ui = thiz;
    guide_info->force_update = true;
    guide_info->popup_ui_index = ID_POPUP_UI_NONE;
    guide_info->shown_page_index = GUIDE_PAGE_INDEX_INVALID;
    guide_info->page_index = GUIDE_PAGE_MAX;
}

void guide_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && guide_info && guide_info->root);
    guide_page_ui_remove(guide_info);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(guide_info->root);
    u_free(guide_info);
    guide_info = NULL;
}

int guide_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && guide_info && guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);

#if IS_SUPPORT_BASE_GUIDE_COMP_BG
    guide_root_comp_bg_update(thiz);
    proc->page_index = GUIDE_PAGE_QRCODE;
#endif
    guide_page_ui_update(guide_info, proc, thiz->priv.play_info.played_count);
    ui_popup_update(thiz, proc->popup_ui_type,
        guide_info->root, &guide_info->popup_ui_index);
    guide_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d, page: %u", thiz->priv.fps_count, proc->page_index);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
