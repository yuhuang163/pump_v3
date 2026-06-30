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
#define IS_SUPPORT_ROOT_PUMP_HOME_COMP_BG            (0)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_PUMP_HOME_COMP_BG
static PSRAM_DATA_LOC uint8_t pump_home_bg_data[] = {
    #include "comparison_png2txt/pump_home5_360x360.txt"
};
#endif

static pump_home_base_ui_info_t *pump_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_blink_state_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc)
{
    uint8_t new_mode_blink = 0;
    uint8_t new_time_blink = 0;
    uint8_t new_left_level_blink = 0;
    uint8_t new_right_level_blink = 0;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(info && proc && proc->pump);
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);

    if (proc->pump_mode_display_type == PUMP_MODE_DISPLAY_NORMAL_MODE &&
        PUMP_IS_COMMON_MODE(pump_mode)) {
        new_mode_blink = 1;
    }
    if (proc->pump_status == BREAST_PUMP_PAUSE) {
        new_time_blink = 1;
    }

    switch (proc->pump_level_display_type) {
        case PUMP_LEVEL_DISPLAY_LEFT_ADJUST:
            new_left_level_blink = 1;
            break;
        case PUMP_LEVEL_DISPLAY_RIGHT_ADJUST:
            new_right_level_blink = 1;
            break;
        case PUMP_LEVEL_DISPLAY_BOTH_ADJUST:
            new_left_level_blink = 1;
            new_right_level_blink = 1;
            break;
        default:
            break;
    }

    info->mode.mode_blink_state = new_mode_blink;
    info->time.time_blink_state = new_time_blink;
    info->level.left_blink_state = new_left_level_blink;
    info->level.right_blink_state = new_right_level_blink;
    info->animation.anim_play_state = (proc->pump_status == BREAST_PUMP_PAUSE) ? 1 : 0;
}

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

#if IS_SUPPORT_ROOT_PUMP_HOME_COMP_BG
static void pump_home_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    if (pump_info->background == NULL || !lv_obj_is_valid(pump_info->background)) {
        pump_info->background = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->background);
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
    #if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    #endif
    if (pump_info->background == NULL || !lv_obj_is_valid(pump_info->background)) {
        pump_info->background = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->background);
        lv_obj_align(pump_info->background, LV_ALIGN_CENTER, 0, 0);
        #if LV_USE_SNAPSHOT
        lv_image_set_src(pump_info->background, background_snapshot);
        #else
        lv_obj_add_flag(pump_info->background, LV_OBJ_FLAG_HIDDEN);
        #endif
    }
}
#endif

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
    pump_home_root_init();
    ble_ui_init(thiz);
    pump_info->ui = thiz;
    pump_info->force_update = true;
    pump_info->popup_ui_index = ID_POPUP_UI_NONE;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    pump_info->mode.index = pump_mode;
    pump_info->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
}

void pump_home_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && pump_info && pump_info->root);
    ble_ui_deinit(thiz);
    pump_home_base_mode_ui_remove(pump_info);
    pump_home_base_level_ui_remove(pump_info);
    pump_home_base_time_ui_remove(pump_info);
    pump_home_base_animation_ui_remove(pump_info);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(pump_info->root);
    u_free(pump_info);
    pump_info = NULL;
}

int pump_home_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && pump_info && pump_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump && proc->motor && proc->ble);
    const pump_result_info_t *result_info = NULL;
    READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    EXCEPTION_CHECK_INT(result_info);
    uint16_t hold_time = result_info->work_valid_time / 1000;
    uint16_t target_valid_time = result_info->target_valid_time / 1000;
    pump_setting_t setting;
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, setting);
    pump_info->level.left.new_value = setting.level[0];
    pump_info->level.right.new_value = setting.level[1];
    pump_info->mode.new_index = pump_mode;
    pump_info->mode.new_mode_type = proc->motor->get_runtime_mode_type(false);
    if (pump_mode == BREAST_PUMP_CUSTOM_MODE && pump_info->st) {
        int custom_mode_id = 0;
        pump_info->st->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO_ID, BREAST_PUMP_CUSTOM_MODE,
            0, &custom_mode_id);
        if (custom_mode_id != pump_info->mode.custom_mode_id) {
            pump_info->mode.custom_mode_id = custom_mode_id;
            pump_info->mode.index = BREAST_PUMP_MODE_INVALID;
        }
    }
    pump_home_base_blink_state_update(pump_info, proc);
    const bool blink_visible = proc->state_blink_visible != 0;
    pump_home_root_bg_update(thiz);
    pump_info->time.remain_time = target_valid_time > hold_time ? (target_valid_time - hold_time) : 0;
    pump_home_base_time_ui_update(pump_info, hold_time, blink_visible);
    pump_home_base_mode_ui_update(pump_info, blink_visible);
    pump_home_base_animation_ui_update(pump_info);
    pump_home_base_level_ui_update(pump_info, blink_visible);
    const ble_ui_blink_sync_t ble_sync = {
        .sync_enable = (pump_info->mode.mode_blink_state != 0 ||
            pump_info->time.time_blink_state != 0 ||
            pump_info->level.left_blink_state != 0 ||
            pump_info->level.right_blink_state != 0) ? 1 : 0,
        .blink_time = 0,
    };
    ble_ui_update(thiz, pump_info->root, proc->ble,
        BASE_PUMP_HOME_BLE_UI_X, BASE_PUMP_HOME_BLE_UI_Y, &ble_sync);
    ui_popup_update(thiz, proc->popup_ui_type,
        pump_info->root, &pump_info->popup_ui_index);
    pump_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump home fps: %d, mode:%d/%d", thiz->priv.fps_count, pump_mode, proc->popup_ui_type);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
