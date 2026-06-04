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
static lv_obj_t * backgroud = NULL;
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
    EXCEPTION_CHECK_VOID(thiz && parent);
    if (s_force_update == false) {
        return;
    }

#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
#endif

    if (backgroud == NULL || !lv_obj_is_valid(backgroud)) {
        backgroud = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(backgroud);
        lv_obj_align(backgroud, LV_ALIGN_CENTER, 0, 0);
#if LV_USE_SNAPSHOT
        lv_image_set_src(backgroud, background_snapshot);
#else
        lv_obj_add_flag(backgroud, LV_OBJ_FLAG_HIDDEN); /// 隐藏
#endif
    }
    s_force_update = false;
}

void common_ui_bg_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    backgroud = NULL;
    s_force_update = false;
}


