/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_COMP_BG                                0
#define HOME_CONTENT_COUNT                                     4
#define HOME_CONTENT_COLOR                                     0x3C3D40
#define HOME_QRCODE_BG_COLOR                                   0xFFFFFF
#define HOME_QRCODE_FG_COLOR                                   0x000000
#define HOME_QRCODE_BORDER_COLOR                               0xA6A6A6

typedef struct {
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
    uint16_t text_x;
    uint16_t text_y;
    const char *name;
} home_content_info_t;

typedef struct {  
    uint8_t page_index;  
    lv_obj_t * root;
#if IS_SUPPORT_HOME_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * qrcode;
    lv_obj_t * qrcode_container;
    lv_obj_t * icon;
    lv_obj_t * text;
    res_img_info_t icon_info;
} home_display_info_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_COMP_BG
static PSRAM_DATA_LOC uint8_t home_bg_data[] = {
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

static home_display_info_t *home_info = NULL;

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
void ui_display_home_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_info == NULL) {
        home_info = (home_display_info_t *)u_malloz(sizeof(home_display_info_t));
        EXCEPTION_CHECK_VOID(home_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_home_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_info == NULL) {
        return;
    }
    // common_ui_img_resource_deinit(thiz, &home_info->icon_info);
    LVGL_OBJ_DELETE(home_info->root);  
    u_free(home_info);
    home_info = NULL;
}

int ui_display_home_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(home_info);
    EXCEPTION_CHECK_INT(home_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->ble);
    EXCEPTION_CHECK_INT(proc->battery);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
