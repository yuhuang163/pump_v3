/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_GUIDE_COMP_BG                                0
#define GUIDE_CONTENT_COUNT                                     4
#define GUIDE_CONTENT_COLOR                                     0x3C3D40
#define GUIDE_QRCODE_BG_COLOR                                   0xFFFFFF
#define GUIDE_QRCODE_FG_COLOR                                   0x000000
#define GUIDE_QRCODE_BORDER_COLOR                               0xA6A6A6

typedef struct {
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
    uint16_t text_x;
    uint16_t text_y;
    const char *name;
} guide_content_info_t;

typedef struct {  
    uint8_t page_index;  
    lv_obj_t * root;
#if IS_SUPPORT_GUIDE_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * qrcode;
    lv_obj_t * qrcode_container;
    lv_obj_t * icon;
    lv_obj_t * text;
    res_img_info_t icon_info;
} guide_display_info_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_GUIDE_COMP_BG
static PSRAM_DATA_LOC uint8_t guide_bg_data[] = {
    // #include "comparison_png2txt/guide_phone_cmp_360x360.txt"
    // #include "comparison_png2txt/guide_chasing_milk_cmp_360x360.txt"
    // #include "comparison_png2txt/guide_adjust_comp_360x360.txt"
    // #include "comparison_png2txt/guide_milk_quantity_cmp_360x360.txt"
    #include "comparison_png2txt/guide_qrcode_cmp_360x360.txt"
};
#endif

const static char *bind_qr_str[] = {
    "https://lvgl.io",
};

static guide_display_info_t *guide_info = NULL;

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
void ui_display_guide_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_display_info_t *)u_malloz(sizeof(guide_display_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_guide_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        return;
    }
    // common_ui_img_resource_deinit(thiz, &guide_info->icon_info);
    LVGL_OBJ_DELETE(guide_info->root);  
    u_free(guide_info);
    guide_info = NULL;
}

int ui_display_guide_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(guide_info);
    EXCEPTION_CHECK_INT(guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->ble);
    EXCEPTION_CHECK_INT(proc->battery);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
