/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "airplane_mode_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_AIRPLANE_MODE_COMP_BG                          (0)
#define AIRPLANE_MODE_TEXT_COUNT                                  (1)
#define AIRPLANE_MODE_TITLE_COLOR                                 0x1C1C20
#define AIRPLANE_MODE_CONTENT_COLOR                               0x3C3D40

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
} airplane_mode_text_info_t;

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * lock_icon;
    lv_obj_t * button_icon;
    lv_obj_t * texts[AIRPLANE_MODE_TEXT_COUNT];
    res_img_info_t lock_icon_info;
    res_img_info_t button_icon_info;
} airplane_mode_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t complete_tip_bg_data[] = {
    #include "comparison_png2txt/travel_lock_cmp_360x360.txt"
};
#endif

static airplane_mode_display_info_t *airplane_mode_info;

const static airplane_mode_text_info_t airplane_mode_text_infos[AIRPLANE_MODE_TEXT_COUNT] = {
    { "The breast pump is locked\nlong press\nfor 5 second to unlock", (66), (153) },
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
void ui_display_airplane_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (airplane_mode_info == NULL) {
        airplane_mode_info = (airplane_mode_display_info_t *)u_malloz(sizeof(airplane_mode_display_info_t));
        EXCEPTION_CHECK_VOID(airplane_mode_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_airplane_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (airplane_mode_info == NULL) {
        return;
    }
    // common_ui_img_resource_deinit(thiz, &airplane_mode_info->lock_icon_info);
    // common_ui_img_resource_deinit(thiz, &airplane_mode_info->button_icon_info);
    LVGL_OBJ_DELETE(airplane_mode_info->root); 
    u_free(airplane_mode_info);
    airplane_mode_info = NULL;
}

int ui_display_airplane_mode_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(airplane_mode_info);
    EXCEPTION_CHECK_INT(airplane_mode_info->root);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
        LOG_UI("===> airplane mode fps=%d", thiz->priv.fps_count);
    }
    return thiz->priv.fps_count;
}