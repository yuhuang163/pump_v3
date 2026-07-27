/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "ui_img_png_decode.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define UI_PREDECODE_BG_COUNT  3

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static lv_draw_buf_t *lv_snap_shot[UI_SNAP_SHOT_ID_MAX] = {NULL};  /// 背景快照，避免重复解码
const static ui_snap_shot_info_t snap_shot_infos[UI_SNAP_SHOT_ID_MAX] = {
    {UI_IMG_RES_ID_NONE, false},
};

/// 模式背景预解码（NATIVE），页面按资源 ID 直接取用
static const uint16_t predecode_bg_ids[UI_PREDECODE_BG_COUNT] = {
    BASE_RES_ID_SIMULATION_MODE_BG,
    BASE_RES_ID_EXPRESSION_MODE_BG,
    BASE_RES_ID_PROGRAM_MODE_BG,
};

static lv_draw_buf_t *lv_predecode_bg[UI_PREDECODE_BG_COUNT] = {NULL};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_predecode_bg_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);

    for (uint8_t i = 0; i < UI_PREDECODE_BG_COUNT; i++) {
        if (lv_predecode_bg[i] != NULL) {
            continue;
        }
        lv_predecode_bg[i] = ui_img_png_theme_decode_draw_buf(thiz, predecode_bg_ids[i],
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_NATIVE);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
lv_draw_buf_t *get_lvgl_snap_shot(ui_snap_shot_id_e snapshot_id)
{
    if (snapshot_id >= UI_SNAP_SHOT_ID_MAX) {
        return NULL;
    }
    return lv_snap_shot[snapshot_id];
}

lv_draw_buf_t *get_predecoded_bg(uint16_t res_id)
{
    for (uint8_t i = 0; i < UI_PREDECODE_BG_COUNT; i++) {
        if (predecode_bg_ids[i] == res_id) {
            return lv_predecode_bg[i];
        }
    }
    return NULL;
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
    for (uint8_t i = 0; i < UI_SNAP_SHOT_ID_MAX; i++) {
        ui_theme_img_resource_init(thiz, snap_shot_infos[i].img_id,
            RES_THEME_TYPE_BASE, &snap_shot_info);
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
    ui_img_resource_deinit(thiz, &snap_shot_info);
    LVGL_OBJ_DELETE(snap_shot_obj);

    /// 三张模式背景：直接 PNG 预解码为 NATIVE，不走 snapshot
    ui_predecode_bg_prepare(thiz);
}
