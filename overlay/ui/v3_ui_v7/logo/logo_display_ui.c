/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_logo.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_LOGO_COMP_BG                (0)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * logo_icon;
} logo_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_LOGO_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t logo_bg_data[] = {
    #include "comparison_png2txt/home_ip3_360x360.txt"
};
#endif

static logo_display_info_t *logo_info = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void logo_root_init(void)
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

#if IS_SUPPORT_ROOT_LOGO_COMP_BG
static void logo_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (logo_info->background == NULL || !lv_obj_is_valid(logo_info->background)) {
        logo_info->background = lv_image_create(logo_info->root);
        EXCEPTION_CHECK_VOID(logo_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(logo_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(logo_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, logo_bg_data,
            sizeof(logo_bg_data), "png");
        lv_image_set_src(logo_info->background, (const void *)&bg_img_path);
        lv_obj_align(logo_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void logo_root_bg_update(ui_manager_context_t *thiz)
{

}
#endif

static void logo_icon_update(ui_manager_context_t *thiz)
{
    if (logo_info->logo_icon == NULL || !lv_obj_is_valid(logo_info->logo_icon)) {
        logo_info->logo_icon = lv_image_create(logo_info->root);
        EXCEPTION_CHECK_VOID(logo_info->logo_icon);
        ui_img_baj_theme_set_src(thiz, logo_info->logo_icon, BASE_RES_ID_START_LOGO_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(logo_info->logo_icon, LV_ALIGN_CENTER, 0, 0);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void logo_display_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (logo_info == NULL) {
        logo_info = (logo_display_info_t *)u_malloz(sizeof(logo_display_info_t));
        EXCEPTION_CHECK_VOID(logo_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    logo_root_init();
}

void logo_display_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && logo_info && logo_info->root);
    ui_img_baj_release_decoded_buf(logo_info->logo_icon);
    LVGL_OBJ_DELETE(logo_info->root);
    u_free(logo_info);
    logo_info = NULL;
}

int logo_display_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && logo_info && logo_info->root);
    logo_root_bg_update(thiz);
    logo_icon_update(thiz);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> logo fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
