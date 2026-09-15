/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
#define IS_SUPPORT_PUMP_HOME_UI_TEST                (0)
#else
#define IS_SUPPORT_PUMP_HOME_UI_TEST                (0)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t pump_home_bg_data[] = {
    #include "comparison_png2txt/pump_home_20_352x352.txt"
    // #include "comparison_png2txt/pump_home_21_352x352.txt"
};
#endif

static pump_home_base_ui_info_t *pump_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_home_root_init(void)
{
    if (pump_info->root == NULL || !lv_obj_is_valid(pump_info->root)) {
        pump_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(pump_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(pump_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(pump_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pump_info->root, 0, 0);
    lv_obj_set_style_pad_all(pump_info->root, 0, 0);
    lv_obj_align(pump_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(pump_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(pump_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
static void pump_home_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (pump_info->background == NULL || !lv_obj_is_valid(pump_info->background)) {
        pump_info->background = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->background);
        lv_obj_move_background(pump_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(pump_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(pump_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, pump_home_bg_data,
            sizeof(pump_home_bg_data), "png");
        lv_image_set_src(pump_info->background, (const void *)&bg_img_path);
        lv_obj_align(pump_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_home_root_bg_update(ui_manager_context_t *thiz)
{
    /// 吸奶主页使用模式背景 PNG，root 保持黑底即可
    (void)thiz;
}
#endif

static void pump_home_base_ui_apply_custom_mode_from_proc(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(pump_info && proc);
    if (pump_info->mode.custom_mode_id != proc->custom_mode_id) {
        LOG_UI("pump home custom refresh id %u -> %u",
            pump_info->mode.custom_mode_id, proc->custom_mode_id);
    }
    pump_info->mode.custom_mode_active = proc->custom_mode_active;
    pump_info->mode.custom_mode_id = proc->custom_mode_id;
    memset(pump_info->mode.custom_id_str, 0, sizeof(pump_info->mode.custom_id_str));
    strncpy(pump_info->mode.custom_id_str, proc->custom_mode_name,
        sizeof(pump_info->mode.custom_id_str) - 1);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_info == NULL) {
        pump_info = (pump_home_base_ui_info_t *)u_malloz(sizeof(pump_home_base_ui_info_t));
        EXCEPTION_CHECK_VOID(pump_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    LOG_UI("pump home prepare start t=%ld", get_timestamp());
    pump_home_root_init();
    ble_ui_init(thiz);
    battery_ui_init(thiz);
    pump_info->ui = thiz;
    pump_info->force_update = true;
    pump_info->popup_ui_index = ID_POPUP_UI_NONE;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    pump_info->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    pump_home_base_ui_apply_custom_mode_from_proc(proc);
    LOG_UI("pump home prepare: page begin t=%ld", get_timestamp());
    pump_home_page_ui_prepare(pump_info);
    LOG_UI("pump home prepare done t=%ld", get_timestamp());
}

void pump_home_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && pump_info && pump_info->root);
    ble_ui_deinit(thiz);
    battery_ui_deinit(thiz);
    pump_home_page_ui_remove(pump_info);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(pump_info->root);
    u_free(pump_info);
    pump_info = NULL;
}

int pump_home_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && pump_info && pump_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble && proc->battery && proc->pump && proc->motor);
    pump_setting_t setting;
    int time_index = 0;
    int pump_mode = 0;

    const uint8_t first_frame = pump_info->force_update;

    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, setting);
    pump_info->level.left = setting.level[0];
    pump_info->level.right = setting.level[1];
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    pump_info->mode.new_index = (uint8_t)pump_mode;
    pump_info->mode.new_mode_type = proc->motor->get_runtime_mode_type(false);
    pump_info->animation.anim_play_state = (proc->pump_status == BREAST_PUMP_PAUSE) ? 1 : 0;

    pump_home_base_ui_apply_custom_mode_from_proc(proc);

    pump_home_root_bg_update(thiz);
#if IS_SUPPORT_PUMP_HOME_UI_TEST
    (void)proc;
#else
    pump_home_page_ui_update(pump_info, proc);
    if (pump_info->page.shown_page_index == PUMP_HOME_PAGE_MAIN &&
        !pump_home_page_ui_is_transitioning(pump_info)) {
        battery_ui_update(thiz, pump_info->root, proc->battery, true,
            proc->ble->ble_is_connected() ? 1 : 0);
        ble_ui_update(thiz, pump_info->root, proc->ble,
            battery_ui_ble_x_get(BASE_PUMP_HOME_BLE_UI_X),
            battery_ui_ble_y_get(BASE_PUMP_HOME_BLE_UI_Y), NULL);
    } else {
        battery_ui_update(thiz, pump_info->root, proc->battery, false, 1);
        ble_ui_hide();
    }
#endif
    ui_popup_update(thiz, proc->popup_ui_type, pump_info->root, &pump_info->popup_ui_index);
    pump_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump home fps: %d, mode:%d/%d, page:%d", thiz->priv.fps_count, pump_mode,
            proc->popup_ui_type, proc->ui_mode_page_index);
        thiz->priv.fps_count = 0;
#if IS_SUPPORT_BASE_TIME_3SEG_TEST
        pump_info->test_hours += 1;
        if (pump_info->test_hours >= 59) {
            pump_info->test_hours = 0;
        }
#endif
    }
    return 0;
}

void pump_home_base_ui_page_switch(pump_home_page_slide_dir_e dir)
{
    if (pump_info == NULL) {
        return;
    }
    pump_home_page_ui_request_switch(pump_info, dir);
}
