/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_logo.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    lv_obj_t * logo_icon; 
    res_img_info_t img_info;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t rsv[15];
} logo_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static logo_display_info_t *logo_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_logo_root_init(void)
{
    if (logo_info->root == NULL || !lv_obj_is_valid(logo_info->root)) {
        logo_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(logo_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(logo_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(logo_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(logo_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(logo_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(logo_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(logo_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(logo_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void ui_logo_bg_update(ui_manager_context_t *thiz)
{
    ui_bg_update(thiz, logo_info->root);
}

static void sys_logo_label_update(ui_manager_context_t *thiz)
{
    if (logo_info->force_update == false) {
        return;
    }
    ui_theme_img_resource_init(thiz, BASE_RES_ID_START_LOGO_ICON, RES_THEME_TYPE_BASE,
        &logo_info->img_info);
    EXCEPTION_CHECK_VOID(logo_info->img_info.data);
    if (logo_info->logo_icon == NULL || !lv_obj_is_valid(logo_info->logo_icon)) {
        logo_info->logo_icon = lv_image_create(logo_info->root);
        EXCEPTION_CHECK_VOID(logo_info->logo_icon);
    }
    lv_fs_path_ex_t logo_img_path;
    lv_fs_make_path_from_buffer(&logo_img_path, LV_FS_MEMFS_LETTER, logo_info->img_info.data,
        logo_info->img_info.size, "baj");
    lv_image_set_src(logo_info->logo_icon, (const void *)&logo_img_path);
    lv_obj_align(logo_info->logo_icon, LV_ALIGN_CENTER, 0, 0);    /// 设置logo图标位置
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void logo_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (logo_info == NULL) {
        logo_info = (logo_display_info_t *)u_malloz(sizeof(logo_display_info_t));
        EXCEPTION_CHECK_VOID(logo_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_logo_root_init(); /// 初始化 root 容器
    ui_bg_init(thiz);
    logo_info->force_update = true;
}

void logo_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && logo_info && logo_info->root);
    ui_img_resource_deinit(thiz, &logo_info->img_info);
    ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(logo_info->root);
    u_free(logo_info);
    logo_info = NULL;
}

int logo_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && logo_info && logo_info->root);
    ui_logo_bg_update(thiz);
    sys_logo_label_update(thiz);   
    logo_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> logo fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
