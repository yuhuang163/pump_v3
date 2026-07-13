/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_custom_ui_internal.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static guide_custom_ui_info_t *guide_info;

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
    ble_ui_init(thiz);
    guide_info->ui = thiz;
    guide_info->force_update = true;
    guide_info->shown_page_index = GUIDE_PAGE_INDEX_INVALID;
    guide_info->page_index = GUIDE_PAGE_MAX;
    guide_info->label.text_id = GUIDE_PAGE_MAX;
    guide_info->animation.shown_frame_res_id = BASE_RES_ID_GUIDE_ANIM_START - 1;
}

void guide_custom_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && guide_info && guide_info->root);
    ble_ui_deinit(thiz);
    guide_page_ui_remove(guide_info);
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
    guide_page_ui_update(guide_info, proc, thiz->priv.play_info.played_count);
    if (GUIDE_PAGE_SHOW_BLE(proc->page_index)) {
        const ble_ui_blink_sync_t ble_sync = {
            .sync_enable = (guide_info->level.left_blink_state != 0 ||
                guide_info->level.right_blink_state != 0) ? 1 : 0,
            .blink_time = 0,
        };
        ble_ui_update(thiz, guide_info->root, proc->ble,
            CUSTOM_PUMP_HOME_BLE_UI_X, CUSTOM_PUMP_HOME_BLE_UI_Y, &ble_sync);
    } else {
        ble_ui_hide();
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d, page: %u", thiz->priv.fps_count, proc->page_index);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
