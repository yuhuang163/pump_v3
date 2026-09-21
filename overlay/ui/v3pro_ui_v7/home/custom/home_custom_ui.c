/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_custom_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_CUSTOM_HOME_COMP_BG
#define IS_SUPPORT_HOME_UI_TEST                     (0)
#else
#define IS_SUPPORT_HOME_UI_TEST                     (0)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_CUSTOM_HOME_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_bg_data[] = {
    #include "comparison_png2txt/home_custom_31_360x360.txt"
};
#endif

static home_base_ui_info_t *home_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_root_init(void)
{
    if (home_info->root == NULL || !lv_obj_is_valid(home_info->root)) {
        home_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(home_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(home_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(home_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(home_info->root, 0, 0);
    lv_obj_set_style_pad_all(home_info->root, 0, 0);
    lv_obj_align(home_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_CUSTOM_HOME_COMP_BG
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (home_info->background == NULL || !lv_obj_is_valid(home_info->background)) {
        home_info->background = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->background);
        lv_obj_move_background(home_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(home_info->background, (const void *)&bg_img_path);
        lv_obj_align(home_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
    if (home_info->background != NULL && lv_obj_is_valid(home_info->background)) {
        lv_obj_add_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
    }
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_info == NULL) {
        home_info = (home_base_ui_info_t *)u_malloz(sizeof(home_base_ui_info_t));
        EXCEPTION_CHECK_VOID(home_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_root_init();
    ble_ui_init(thiz);
    battery_ui_init(thiz);
    battery_ui_y_offset_set((int16_t)(HOME_CUSTOM_MAIN_STATUS_Y - HOME_MAIN_STATUS_Y));
    home_info->ui = thiz;
    home_info->theme_type = RES_THEME_TYPE_CUSTOM;
    home_info->main_ops = &home_custom_main_page_ops;
    home_info->force_update = true;
    home_info->popup_ui_index = ID_POPUP_UI_NONE;
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    home_info->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    home_ui_sync_work_mode(home_info);
    home_page_ui_prepare(home_info);
}

void home_custom_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && home_info && home_info->root);
    ble_ui_deinit(thiz);
    battery_ui_deinit(thiz);
    home_page_ui_remove(home_info);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(home_info->root);
    u_free(home_info);
    home_info = NULL;
}

int home_custom_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && home_info && home_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble && proc->battery && proc->pump);
    pump_setting_t setting;
    READ_PUMP_SETTING_INFO(proc->pump, 0, setting);
    home_info->level.left = setting.level[0];
    home_info->level.right = setting.level[1];
    home_ui_sync_work_mode(home_info);
    home_root_bg_update(thiz);
    #if IS_SUPPORT_HOME_UI_TEST
    proc->popup_ui_type = ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN;
    #else
    home_page_ui_update(home_info, proc);
    if (home_info->page.shown_page_index == HOME_PAGE_MAIN &&
        !home_page_ui_is_transitioning(home_info)) {
        battery_ui_update(thiz, home_info->root, proc->battery, true,
            proc->ble->ble_is_connected() ? 1 : 0);
        ble_ui_update(thiz, home_info->root, proc->ble,
            battery_ui_ble_x_get(CUSTOM_PUMP_HOME_BLE_UI_X),
            battery_ui_ble_y_get(CUSTOM_PUMP_HOME_BLE_UI_Y), NULL);
    } else {
        battery_ui_update(thiz, home_info->root, proc->battery, false, 1);
        ble_ui_hide();
    }
    #endif
    ui_popup_update(thiz, proc->popup_ui_type, home_info->root, &home_info->popup_ui_index);
    home_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home custom fps: %d, page:%d", thiz->priv.fps_count, proc->ui_mode_page_index);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
