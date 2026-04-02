/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_battery.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_LOW_POWER_COMP_BG                (0)

typedef struct {    
    lv_obj_t * root;
#if IS_SUPPORT_ROOT_LOW_POWER_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * low_power_icon;
    res_img_info_t low_power_icon_info;
    uint8_t force_update;
} battery_low_power_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static battery_low_power_display_info_t *low_power_info;

#if IS_SUPPORT_ROOT_LOW_POWER_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t low_power_bg_data[] = {
    #include "comparison_png2txt/home_low_power1_360x360.txt"
};
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void battery_low_power_ui_root_init(void)
{
    if (low_power_info->root == NULL || !lv_obj_is_valid(low_power_info->root)) {
        low_power_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(low_power_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(low_power_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(low_power_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(low_power_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(low_power_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(low_power_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void battery_low_power_ui_bg_ui_update(ui_manager_context_t *thiz)
{
    if (low_power_info->low_power_icon == NULL || !lv_obj_is_valid(low_power_info->low_power_icon)) {
        low_power_info->low_power_icon = lv_image_create(low_power_info->root);
    }
    common_ui_img_resource_init(thiz, RESOURCE_ID_LOW_POWER_PEOPLE_ICON, &low_power_info->low_power_icon_info);
    EXCEPTION_CHECK_VOID(low_power_info->low_power_icon_info.data);
    lv_fs_path_ex_t low_power_icon_img_path;
    lv_fs_make_path_from_buffer(&low_power_icon_img_path, LV_FS_MEMFS_LETTER,
        low_power_info->low_power_icon_info.data, low_power_info->low_power_icon_info.size, "baj");
    lv_image_set_src(low_power_info->low_power_icon, low_power_icon_img_path.path);
    lv_obj_align(low_power_info->low_power_icon, LV_ALIGN_TOP_LEFT, 12, 66);
}


#if IS_SUPPORT_ROOT_LOW_POWER_COMP_BG
static void low_power_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (low_power_info->backgroud == NULL || !lv_obj_is_valid(low_power_info->backgroud)) {
        low_power_info->backgroud = lv_image_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(low_power_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(low_power_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, low_power_bg_data,
            sizeof(low_power_bg_data), "png");
        lv_image_set_src(low_power_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(low_power_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void low_power_root_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, low_power_info->root);
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (low_power_info == NULL) {
        low_power_info = (battery_low_power_display_info_t *)u_malloz(sizeof(battery_low_power_display_info_t));
        EXCEPTION_CHECK_VOID(low_power_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    battery_low_power_ui_root_init();
    common_ui_bg_init(thiz);
    LOG_UI("battery low power ui prepare");
    low_power_info->force_update = true;
}

void ui_display_battery_low_power_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(low_power_info);
    common_ui_img_resource_deinit(thiz, &low_power_info->low_power_icon_info);
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(low_power_info->root);
    u_free(low_power_info);
    low_power_info = NULL;
}

int ui_display_battery_low_power_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(low_power_info);
    EXCEPTION_CHECK_INT(low_power_info->root);
    ui_proc_battery_t *proc = (ui_proc_battery_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    low_power_root_bg_update(thiz);
    battery_low_power_ui_bg_ui_update(thiz);
    low_power_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> battery low power fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}