/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    uint8_t force_update;
    uint32_t start_time;
} ota_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ota_display_info_t *ota_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_ota_root_init(void)
{
    if (ota_info->root == NULL || !lv_obj_is_valid(ota_info->root)) {
        ota_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(ota_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(ota_info->root, lv_color_white(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(ota_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(ota_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(ota_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(ota_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ota_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(ota_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void ui_ota_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, ota_info->root);
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_ota_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ota_info == NULL) {
        ota_info = (ota_display_info_t *)u_malloz(sizeof(ota_display_info_t));
        EXCEPTION_CHECK_VOID(ota_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_ota_root_init(); /// 初始化 root 容器
    common_ui_bg_init(thiz);
    ota_info->force_update = true;
}

void ui_display_ota_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz); 
    EXCEPTION_CHECK_VOID(ota_info);
    EXCEPTION_CHECK_VOID(ota_info->root);
    LVGL_OBJ_DELETE(ota_info->root);
    u_free(ota_info);
    ota_info = NULL;
}

int ui_display_ota_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(ota_info);
    EXCEPTION_CHECK_INT(ota_info->root);
    ui_ota_bg_update(thiz);
    ota_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> ota fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
