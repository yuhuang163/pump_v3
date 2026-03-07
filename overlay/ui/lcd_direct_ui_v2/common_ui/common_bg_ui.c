/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static lv_obj_t * backgroud;
static res_img_info_t background_info;
static lv_draw_buf_t *background_snapshot;  /// 背景快照，避免重复解码
static uint8_t s_force_update = 0;

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
void common_ui_bg_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    s_force_update = true;
}

void common_ui_bg_update(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (s_force_update == false) {
        return;
    }

    /* Avoid reusing a global static image object across different parent roots. */
    if (backgroud != NULL && lv_obj_is_valid(backgroud)) {
        lv_obj_t *bg_parent = lv_obj_get_parent(backgroud);
        if (bg_parent != parent) {
            backgroud = NULL;
        }
    }

    if (backgroud == NULL || !lv_obj_is_valid(backgroud)) {
        backgroud = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(backgroud);
#if LV_USE_SNAPSHOT
        if (background_snapshot != NULL) {
            lv_draw_buf_destroy(background_snapshot);
            background_snapshot = NULL;
        }
#endif
        common_ui_img_resource_init(thiz, RESOURCE_ID_PUMP_HOME_BG, &background_info);
        EXCEPTION_CHECK_VOID(background_info.data);
        lv_fs_path_ex_t background_path;
        lv_fs_make_path_from_buffer(&background_path,
                                    LV_FS_MEMFS_LETTER,
                                    background_info.data,
                                    background_info.size,
                                    "png");
        lv_image_set_src(backgroud, (const void *)&background_path);
        lv_obj_align(backgroud, LV_ALIGN_CENTER, 0, 0);
#if LV_USE_SNAPSHOT
        lv_refr_now(NULL); /// 等待渲染完成后再创建快照
        background_snapshot = lv_snapshot_take(backgroud, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA);
#endif
    }

#if LV_USE_SNAPSHOT
    if (background_snapshot != NULL) {
        lv_image_set_src(backgroud, background_snapshot);
    }
#endif
    s_force_update = false;
    // lv_obj_move_background(guide_info->backgroud);
    // lv_obj_move_to_index(guide_info->backgroud, 0);
}

void common_ui_bg_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (background_snapshot != NULL) {
        lv_draw_buf_destroy(background_snapshot);
        background_snapshot = NULL;
    }
    if (backgroud != NULL) {
        common_ui_img_resource_deinit(thiz, &background_info);
    }
    backgroud = NULL;
    s_force_update = false;
}


