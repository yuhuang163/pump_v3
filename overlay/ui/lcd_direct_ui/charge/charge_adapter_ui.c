/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "resource_id_define.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_CHARGE_ADAPTER_COMP_BG            (1)
#define CHARGE_ADAPTER_TEXT_COLOR                    0x3C3D40

typedef struct {  
    lv_obj_t * root;
#if IS_SUPPORT_CHARGE_ADAPTER_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * charge_adapter_icon;
    lv_obj_t * text;
    res_img_info_t charge_adapter_icon_info;
    uint8_t force_update;
} charge_adapter_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_CHARGE_ADAPTER_COMP_BG
static PSRAM_DATA_LOC uint8_t adapter_bg_data[] = {
    #include "comparison_png2txt/charge_page_comp_360x360.txt"
};
#endif

static charge_adapter_display_info_t *adapter_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void charge_adapter_ui_root_init(ui_manager_context_t *thiz)
{
    if (adapter_info->root == NULL || !lv_obj_is_valid(adapter_info->root)) {
        adapter_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(adapter_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(adapter_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(adapter_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(adapter_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(adapter_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(adapter_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(adapter_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(adapter_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

#if IS_SUPPORT_CHARGE_ADAPTER_COMP_BG
static void charge_adapter_bg_update(ui_manager_context_t *thiz)
{
    static uint8_t status = 0;
    static uint32_t start_time = 0;
    if (adapter_info->background == NULL || !lv_obj_is_valid(adapter_info->root)) {
        adapter_info->background = lv_image_create(adapter_info->root);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(adapter_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(adapter_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, adapter_bg_data,
            sizeof(adapter_bg_data), "png");
        lv_image_set_src(adapter_info->background, (const void *)&bg_img_path);
        lv_obj_align(adapter_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

static int charge_adapter_object_update(ui_manager_context_t *thiz)
{
    if (adapter_info->force_update == false) {
        return 0;
    }
    
    /// 更新文本.
    if (adapter_info->text == NULL || !lv_obj_is_valid(adapter_info->root)) {
        adapter_info->text = lv_label_create(adapter_info->root);
        EXCEPTION_CHECK_INT(adapter_info->text);
    }
    lv_label_set_text(adapter_info->text, 
        "The adapter is incompatible.\nPlease confirm the adapter.");
    lv_obj_set_style_text_color(adapter_info->text, lv_color_hex(CHARGE_ADAPTER_TEXT_COLOR), 0);
    // lv_obj_set_style_text_opa(adapter_info->text, LV_OPA_70, 0);  /// 设置文本不透明度为60%
    lv_obj_set_style_text_font(adapter_info->text, &lv_font_roboto_17_regular, 0);  /// 设置字体
    lv_obj_set_style_text_line_space(adapter_info->text, 4, 0);  /// 设置行间距（像素）
    lv_obj_set_style_text_align(adapter_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_align(adapter_info->text, LV_ALIGN_TOP_LEFT, 116 + 4, 200 + 3);

    /// 更新充电图标.
    if (adapter_info->charge_adapter_icon == NULL || !lv_obj_is_valid(adapter_info->root)) {
        adapter_info->charge_adapter_icon = lv_image_create(adapter_info->root);
        EXCEPTION_CHECK_INT(adapter_info->charge_adapter_icon);
    }
    common_ui_img_resource_init(thiz, RESOURCE_ID_CHARGE_ADAPTER_ICON, &adapter_info->charge_adapter_icon_info);
    EXCEPTION_CHECK_INT(adapter_info->charge_adapter_icon_info.data);
    lv_fs_path_ex_t charge_adapter_icon_img_path;
    lv_fs_make_path_from_buffer(&charge_adapter_icon_img_path, LV_FS_MEMFS_LETTER,
        adapter_info->charge_adapter_icon_info.data, adapter_info->charge_adapter_icon_info.size, "baj");
    lv_image_set_src(adapter_info->charge_adapter_icon, charge_adapter_icon_img_path.path);
    lv_obj_align(adapter_info->charge_adapter_icon, LV_ALIGN_TOP_LEFT, 100 + 3, 60);

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_charge_adapter_screen_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (adapter_info == NULL) {
        adapter_info = (charge_adapter_display_info_t *)u_malloz(sizeof(charge_adapter_display_info_t));
        EXCEPTION_CHECK_VOID(adapter_info);
    }
    charge_adapter_ui_root_init(thiz);  
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    adapter_info->force_update = true;
}

void ui_display_charge_adapter_screen_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(adapter_info);
    common_ui_img_resource_deinit(thiz, &adapter_info->charge_adapter_icon_info);
    LVGL_OBJ_DELETE(adapter_info->root);
    u_free(adapter_info);
    adapter_info = NULL;
}

int ui_display_charge_adapter_screen(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(adapter_info);
    EXCEPTION_CHECK_INT(adapter_info->root);
    ui_proc_charge_t *proc = (ui_proc_charge_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    charge_adapter_bg_update(thiz);
    charge_adapter_object_update(thiz);
    thiz->priv.fps_count++;
    adapter_info->force_update = false; 
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> charge adapter fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
        
    }
    return 0;
}