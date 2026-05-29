/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static lv_draw_buf_t *lv_snap_shot[UI_SNAP_SHOT_ID_MAX] = {NULL};  /// 背景快照，避免重复解码
const static ui_snap_shot_info_t snap_shot_infos[UI_SNAP_SHOT_ID_MAX] = {
    {RESOURCE_ID_COMMON_BG, false},
    {RESOURCE_ID_PUMP_LEVEL_BG, false},
    {RESOURCE_ID_REGULAR_PUMPING_BG, false},
    {RESOURCE_ID_EXPERT_PUMPING_BG, false},
    {RESOURCE_ID_PUMP_LEVEL_BOTH_SIDES_BG, false},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
lv_draw_buf_t *get_lvgl_snap_shot(ui_snap_shot_id_e snapshot_id)
{
    if(snapshot_id >= UI_SNAP_SHOT_ID_MAX) {
        return NULL;
    }
    return lv_snap_shot[snapshot_id];
}

void ui_snap_shot_perpare(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    lv_obj_t *snap_shot_obj = NULL;
    res_img_info_t snap_shot_info = {0};
    snap_shot_obj = lv_image_create(lv_screen_active());
    EXCEPTION_CHECK_VOID(snap_shot_obj);
    lv_obj_align(snap_shot_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_image_opa(snap_shot_obj, LV_OPA_COVER, 0);
    lv_obj_add_flag(snap_shot_obj, LV_OBJ_FLAG_HIDDEN); /// 隐藏快照对象
    for(uint8_t i = 0; i < UI_SNAP_SHOT_ID_MAX; i++) {
        common_ui_img_resource_init(thiz, snap_shot_infos[i].img_id, &snap_shot_info);
        if (snap_shot_info.data == NULL) {
            continue;
        }
        lv_fs_path_ex_t background_path;
        lv_fs_make_path_from_buffer(&background_path, LV_FS_MEMFS_LETTER, snap_shot_info.data,
                snap_shot_info.size, "png");
        lv_image_set_src(snap_shot_obj, (const void *)&background_path);
        lv_refr_now(NULL); /// 等待渲染完成后再创建快照
        lv_snap_shot[i] = lv_snapshot_take(snap_shot_obj, 
            snap_shot_infos[i].is_need_alpha ? LV_COLOR_FORMAT_NATIVE_WITH_ALPHA : LV_COLOR_FORMAT_NATIVE);
    }
    common_ui_img_resource_deinit(thiz, &snap_shot_info);
    LVGL_OBJ_DELETE(snap_shot_obj);
}