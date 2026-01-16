/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_COMPLETE_TIP_COMP_BG                        (0)
#define COMPLETE_TIP_TEXT_COUNT                                (2)

#define FINISH_TIP_TITLE_COLOR                                 0x1C1C20
#define FINISH_TIP_CONTENT_COLOR                               0x3C3D40

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
} complete_tip_text_info_t;

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_COMPLETE_TIP_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * icon;
    lv_obj_t * texts[COMPLETE_TIP_TEXT_COUNT];
    res_img_info_t icon_info;
} complete_tip_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_COMPLETE_TIP_COMP_BG
static PSRAM_DATA_LOC uint8_t complete_tip_bg_data[] = {
    #include "comparison_png2txt/pump_finish_comp_360x360.txt"
};
#endif

static complete_tip_display_info_t *finish_info;

const static complete_tip_text_info_t complete_tip_text_infos[COMPLETE_TIP_TEXT_COUNT] = {
    { "Finish", (138 + 3), (151 + 7) },
    { "Please go to MOMCOZY\napp and record the\namount of milk.", (61 + 12), (199 + 3)},
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
void ui_display_pump_complete_tip_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (finish_info == NULL) {
        finish_info = (complete_tip_display_info_t *)u_malloz(sizeof(complete_tip_display_info_t));
        EXCEPTION_CHECK_VOID(finish_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_complete_tip_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (finish_info == NULL) {
        return;
    }
    // common_ui_img_resource_deinit(thiz, &finish_info->icon_info);
    LVGL_OBJ_DELETE(finish_info->root); 
    u_free(finish_info);
    finish_info = NULL;
}

int ui_display_pump_complete_tip(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(finish_info);
    EXCEPTION_CHECK_INT(finish_info->root);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump complete tip fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}