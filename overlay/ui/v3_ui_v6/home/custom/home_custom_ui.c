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
#define IS_SUPPORT_ROOT_HOME_COMP_BG                (1)
#if IS_SUPPORT_ROOT_HOME_COMP_BG
#define IS_SUPPORT_HOME_UI_TEST                     (0)
#else
#define IS_SUPPORT_HOME_UI_TEST                     (0)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_HOME_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_bg_data[] = {
    // #include "comparison_png2txt/home_ip11_360x360.txt"
    // #include "comparison_png2txt/home_ip12_360x360.txt"
    // #include "comparison_png2txt/home_ip13_360x360.txt"
    #include "comparison_png2txt/home_ip14_360x360.txt"
    // #include "comparison_png2txt/LR_level_adjust11_360x360.txt"
};
#endif

static home_custom_ui_info_t *home_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_blink_state_update(home_custom_ui_info_t *info, ui_proc_home_t *proc)
{
    uint8_t new_mode_blink = 0;
    uint8_t new_icon_blink = 0;
    uint8_t new_left_level_blink = 0;
    uint8_t new_right_level_blink = 0;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(info && proc && proc->pump);
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);

    if (proc->pump_mode_display_type == HOME_PUMP_MODE_DISPLAY_NORMAL_MODE &&
        PUMP_IS_COMMON_MODE(pump_mode)) {
        new_mode_blink = 1;
    } else if (proc->pump_mode_display_type == HOME_PUMP_MODE_DISPLAY_BF_MODE &&
        PUMP_IS_BREASTFEEDING_MODE(pump_mode)) {
        new_mode_blink = 1;
        new_icon_blink = 1;
    }

    switch (proc->pump_level_display_type) {
        case HOME_PUMP_LEVEL_DISPLAY_LEFT_ADJUST:
            new_left_level_blink = 1;
            break;
        case HOME_PUMP_LEVEL_DISPLAY_RIGHT_ADJUST:
            new_right_level_blink = 1;
            break;
        case HOME_PUMP_LEVEL_DISPLAY_BOTH_ADJUST:
            new_left_level_blink = 1;
            new_right_level_blink = 1;
            break;
        default:
            break;
    }

    info->mode.mode_blink_state = new_mode_blink;
    info->mode.icon_blink_state = new_icon_blink;
    info->level.left_blink_state = new_left_level_blink;
    info->level.right_blink_state = new_right_level_blink;
}

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

#if IS_SUPPORT_ROOT_HOME_COMP_BG
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (home_info->background == NULL || !lv_obj_is_valid(home_info->background)) {
        home_info->background = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->background);
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
    #if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    #endif
    if (home_info->background == NULL || !lv_obj_is_valid(home_info->background)) {
        home_info->background = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->background);
        lv_obj_align(home_info->background, LV_ALIGN_CENTER, 0, 0);
        #if LV_USE_SNAPSHOT
        lv_image_set_src(home_info->background, background_snapshot);
        #else
        lv_obj_add_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
        #endif
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
        home_info = (home_custom_ui_info_t *)u_malloz(sizeof(home_custom_ui_info_t));
        EXCEPTION_CHECK_VOID(home_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_root_init();
    ble_ui_init(thiz);
    home_info->ui = thiz;
    home_info->force_update = true;
    home_info->label.force_update = true;
    home_info->popup_ui_index = ID_POPUP_UI_NONE;
    home_info->sub_ui_status = 1;
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    home_info->mode.index = pump_mode;
    home_info->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
}

void home_custom_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && home_info && home_info->root);
    ble_ui_deinit(thiz);
    home_custom_pump_level_ui_remove(home_info);
    home_custom_pump_mode_ui_remove(home_info);
    home_custom_animation_ui_remove(home_info);
    home_custom_pump_label_ui_remove(home_info);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(home_info->root);
    u_free(home_info);
    home_info = NULL;
}

int home_custom_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && home_info && home_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble && proc->pump);
    pump_setting_t setting;
    READ_PUMP_SETTING_INFO(proc->pump, 0, setting);
    home_info->level.left.new_value = setting.level[0];
    home_info->level.right.new_value = setting.level[1];
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    home_info->mode.new_index = pump_mode;
    if (pump_mode == BREAST_PUMP_CUSTOM_MODE && home_info->st) {
        int custom_mode_id = 0;
        home_info->st->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO_ID, BREAST_PUMP_CUSTOM_MODE,
            0, &custom_mode_id);
        if (custom_mode_id != home_info->mode.custom_mode_id) {
            home_info->mode.custom_mode_id = custom_mode_id;
            home_info->mode.index = BREAST_PUMP_MODE_INVALID;
        }
    }
    home_root_bg_update(thiz);
    #if IS_SUPPORT_HOME_UI_TEST
    proc->popup_ui_type = ID_POPUP_UI_PUMP_PAUSE_TIMEOUT;
    #else
    home_custom_blink_state_update(home_info, proc);
    const bool blink_visible = proc->state_blink_visible != 0;
    home_custom_pump_label_ui_update(home_info);
    home_custom_animation_ui_update(home_info);
    home_custom_pump_mode_ui_update(home_info, blink_visible);
    home_custom_pump_level_ui_update(home_info, blink_visible);
    const ble_ui_blink_sync_t ble_sync = {
        .sync_enable = (home_info->mode.mode_blink_state != 0 ||
            home_info->mode.icon_blink_state != 0 ||
            home_info->level.left_blink_state != 0 ||
            home_info->level.right_blink_state != 0) ? 1 : 0,
        .blink_time = 0,
    };
    ble_ui_update(thiz, home_info->root, proc->ble,
        CUSTOM_PUMP_HOME_BLE_UI_X, CUSTOM_PUMP_HOME_BLE_UI_Y, &ble_sync);
    #endif

    ui_popup_update(thiz, proc->popup_ui_type,
        home_info->root, &home_info->popup_ui_index);
    home_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
