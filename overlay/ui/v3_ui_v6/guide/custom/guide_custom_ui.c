/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_custom_ui_internal.h" // IWYU pragma: keep
#include "ui_img_baj_decode.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_GUIDE_COMP_BG                (0)
#define GUIDE_IP_X                                   (0)
#define GUIDE_IP_Y                                   (230)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_GUIDE_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t guide_bg_data[] = {
    // #include "comparison_png2txt/guide_custom_p1_360x360.txt"
    // #include "comparison_png2txt/guide_custom_p2_360x360.txt"
    // #include "comparison_png2txt/guide_custom_p3_1_360x360.txt"
    // #include "comparison_png2txt/guide_custom_p3_2_360x360.txt"
    #include "comparison_png2txt/guide_custom_p4_1_360x360.txt"
    // #include "comparison_png2txt/guide_custom_p4_2_360x360.txt"
};
#endif

static guide_custom_ui_info_t *guide_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_blink_state_update(guide_custom_ui_info_t *info, uint8_t page_index)
{
    uint8_t new_left_level_blink = 0;
    uint8_t new_right_level_blink = 0;
    guide_custom_level_display_type_e display_type = GUIDE_CUSTOM_LEVEL_DISPLAY_DEFAULT;

    EXCEPTION_CHECK_VOID(info);

    switch (page_index) {
        case GUIDE_PAGE_PUMP_LEVEL_DOUBLE_ADJUST:
            display_type = GUIDE_CUSTOM_LEVEL_DISPLAY_BOTH_ADJUST;
            break;
        case GUIDE_PAGE_PUMP_LEVEL_SINGLE_SELECT:
            display_type = GUIDE_CUSTOM_LEVEL_DISPLAY_DEFAULT;
            new_left_level_blink = 1;
            new_right_level_blink = 1;
            break;
        case GUIDE_PAGE_PUMP_LEVEL_LEFT_ADJUST:
            display_type = GUIDE_CUSTOM_LEVEL_DISPLAY_LEFT_ADJUST;
            new_left_level_blink = 1;
            break;
        case GUIDE_PAGE_PUMP_LEVEL_RIGHT_ADJUST:
            display_type = GUIDE_CUSTOM_LEVEL_DISPLAY_RIGHT_ADJUST;
            new_right_level_blink = 1;
            break;
        case GUIDE_PAGE_PUMP_MODE_SELECT:
        case GUIDE_PAGE_PUMP_MODE_UNSELECT:
        default:
            display_type = GUIDE_CUSTOM_LEVEL_DISPLAY_DEFAULT;
            break;
    }

    info->level.left_blink_state = new_left_level_blink;
    info->level.right_blink_state = new_right_level_blink;
    info->level_display_type = display_type;
}

static void guide_custom_root_init(void)
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

static void guide_custom_ip_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && guide_info && guide_info->root);
    if (guide_info->ip == NULL || !lv_obj_is_valid(guide_info->ip)) {
        guide_info->ip = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->ip);
        lv_obj_align(guide_info->ip, LV_ALIGN_TOP_MID, GUIDE_IP_X, GUIDE_IP_Y);
    }

    if (guide_info->force_update != 0) {
        ui_img_baj_release_decoded_buf(guide_info->ip);
        ui_img_baj_theme_set_src(thiz, guide_info->ip, CUSTOM_RES_ID_GUIDE_IP,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_clear_flag(guide_info->ip, LV_OBJ_FLAG_HIDDEN);
}

#if IS_SUPPORT_ROOT_GUIDE_COMP_BG
static void guide_custom_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (guide_info->background == NULL || !lv_obj_is_valid(guide_info->background)) {
        guide_info->background = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(guide_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, guide_bg_data,
            sizeof(guide_bg_data), "png");
        lv_image_set_src(guide_info->background, (const void *)&bg_img_path);
        lv_obj_align(guide_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
    (void)thiz;
}
#else
static void guide_custom_root_bg_update(ui_manager_context_t *thiz)
{
    #if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    #endif
    if (guide_info->background == NULL || !lv_obj_is_valid(guide_info->background)) {
        guide_info->background = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->background);
        lv_obj_align(guide_info->background, LV_ALIGN_CENTER, 0, 0);
        #if LV_USE_SNAPSHOT
        lv_image_set_src(guide_info->background, background_snapshot);
        #else
        lv_obj_add_flag(guide_info->background, LV_OBJ_FLAG_HIDDEN);
        #endif
    }
    (void)thiz;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
guide_custom_ui_info_t *guide_custom_info_get(void)
{
    return guide_info;
}

void guide_custom_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_custom_ui_info_t *)u_malloz(sizeof(guide_custom_ui_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    guide_custom_root_init();
    ble_ui_init(thiz);
    guide_info->ui = thiz;
    guide_info->force_update = true;
    guide_info->label.force_update = true;
    guide_info->label.text_id = GUIDE_CUSTOM_TEXT_MAX;
}

void guide_custom_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && guide_info && guide_info->root);
    ble_ui_deinit(thiz);
    ui_img_baj_release_decoded_buf(guide_info->ip);
    guide_custom_pump_level_ui_remove(guide_info);
    guide_custom_pump_label_ui_remove(guide_info);
    LVGL_OBJ_DELETE(guide_info->root);
    u_free(guide_info);
    guide_info = NULL;
}

int guide_custom_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && guide_info && guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble);
    guide_info->level.left.new_value = proc->setting.level[0];
    guide_info->level.right.new_value = proc->setting.level[1];
    guide_custom_root_bg_update(thiz);
    guide_custom_ip_update(thiz);
    guide_custom_blink_state_update(guide_info, proc->page_index);
    const bool blink_visible = proc->state_blink_visible != 0;
    guide_custom_pump_level_ui_update(guide_info, blink_visible);
    guide_custom_pump_label_ui_update(guide_info, proc->page_index);

    const ble_ui_blink_sync_t ble_sync = {
        .sync_enable = (guide_info->level.left_blink_state != 0 ||
            guide_info->level.right_blink_state != 0) ? 1 : 0,
        .blink_time = 0,
    };
    ble_ui_update(thiz, guide_info->root, proc->ble,
        CUSTOM_PUMP_HOME_BLE_UI_X, CUSTOM_PUMP_HOME_BLE_UI_Y, &ble_sync);
    guide_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
