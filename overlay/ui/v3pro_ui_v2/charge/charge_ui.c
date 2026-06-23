/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    lv_obj_t * charge_icon; 
    res_img_info_t charge_icon_info;
    uint8_t force_update;
} charge_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static charge_display_info_t *charge_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void charge_ui_root_init(void)
{
    if (charge_info->root == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(charge_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(charge_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(charge_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(charge_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(charge_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(charge_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void charge_bg_ui_update(ui_manager_context_t *thiz)
{
    if (charge_info->charge_icon == NULL || !lv_obj_is_valid(charge_info->charge_icon)) {
        charge_info->charge_icon = lv_image_create(charge_info->root);
        EXCEPTION_CHECK_VOID(charge_info->charge_icon);
        common_ui_img_resource_init_with_theme(thiz, BASE_RES_ID_CHARGE_PAGE_IP, &charge_info->charge_icon_info);
        EXCEPTION_CHECK_VOID(charge_info->charge_icon_info.data);
        lv_fs_path_ex_t charge_icon_img_path;
        lv_fs_make_path_from_buffer(&charge_icon_img_path, LV_FS_MEMFS_LETTER,
            charge_info->charge_icon_info.data, charge_info->charge_icon_info.size, "baj");
        lv_image_set_src(charge_info->charge_icon, charge_icon_img_path.path);
        lv_obj_align(charge_info->charge_icon, LV_ALIGN_CENTER, 0, 0);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_charge_screen_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (charge_info == NULL) {
        charge_info = (charge_display_info_t *)u_malloz(sizeof(charge_display_info_t));
        EXCEPTION_CHECK_VOID(charge_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    charge_ui_root_init();
    battery_ui_init(thiz);
    ble_ui_init(thiz);
    charge_info->force_update = true;
}

void ui_display_charge_screen_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && charge_info && charge_info->root);
    common_ui_img_resource_deinit(thiz, &charge_info->charge_icon_info);
    battery_ui_deinit(thiz);
    ble_ui_deinit(thiz);
    LVGL_OBJ_DELETE(charge_info->root);
    u_free(charge_info);
    charge_info = NULL;
}

int ui_display_charge_screen_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(charge_info);
    EXCEPTION_CHECK_INT(charge_info->root);
    ui_proc_charge_t *proc = (ui_proc_charge_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->battery);
    charge_bg_ui_update(thiz);
    battery_ui_update(thiz, charge_info->root, proc->battery, true);
    ble_ui_update(thiz, charge_info->root, proc->ble, COMMON_BLE_UI_X, COMMON_BLE_UI_Y);
    charge_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> charge fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}