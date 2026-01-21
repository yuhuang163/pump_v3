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
#define IS_SUPPORT_CHARGE_COMP_BG                    (0)
#define CHARGE_TEXT_COLOR                            0xFFFFFF

typedef struct {  
    lv_obj_t * root;
#if IS_SUPPORT_CHARGE_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * charge_icon;
    lv_obj_t * charge_animation;
    lv_obj_t * charge_battery;
    lv_obj_t * text;
    res_img_info_t charge_icon_info;
    res_img_info_t charge_animation_info;
    res_img_info_t charge_battery_info;
    uint8_t force_update;
    uint8_t percent;
    uint16_t animation_index; /// 动画索引.
} charge_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_CHARGE_COMP_BG
static PSRAM_DATA_LOC uint8_t charge_bg_data[] = {
    #include "comparison_png2txt/charge_page_comp_360x360.txt"
};
#endif

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
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

#if IS_SUPPORT_CHARGE_COMP_BG
static void charge_bg_update(ui_manager_context_t *thiz)
{
    static uint8_t status = 0;
    static uint32_t start_time = 0;
    if (charge_info->background == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->background = lv_image_create(charge_info->root);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(charge_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(charge_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, charge_bg_data,
            sizeof(charge_bg_data), "png");
        lv_image_set_src(charge_info->background, (const void *)&bg_img_path);
        lv_obj_align(charge_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

static uint8_t get_battery_img_id(uint8_t battery_percent)
{
    if (battery_percent > 95) {
        return RESOURCE_ID_CHARGE_PAGE_BATT_100;
    } else if (battery_percent > 70) {
        return RESOURCE_ID_CHARGE_PAGE_BATT_75;
    } else if (battery_percent > 35) {
        return RESOURCE_ID_CHARGE_PAGE_BATT_50;
    } else if (battery_percent > 5) {
        return RESOURCE_ID_CHARGE_PAGE_BATT_25;
    } else {
        return RESOURCE_ID_CHARGE_PAGE_BATT_0;
    }
}

static int charge_battery_update(ui_manager_context_t *thiz, ui_proc_charge_t *proc)
{
    EXCEPTION_CHECK_INT(proc);
    uint8_t percent = proc->battery->get_battery_percent();
    // if (charge_info->percent == percent && charge_info->force_update == false) {
    //     return 0;
    // }

    if (charge_info->charge_battery == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->charge_battery = lv_image_create(charge_info->root);
        EXCEPTION_CHECK_INT(charge_info->charge_battery);
    }
    uint16_t battery_img_id = get_battery_img_id(percent);
    common_ui_img_resource_init(thiz, battery_img_id, &charge_info->charge_battery_info);
    EXCEPTION_CHECK_INT(charge_info->charge_battery_info.data);
    lv_fs_path_ex_t charge_battery_img_path;
    lv_fs_make_path_from_buffer(&charge_battery_img_path, LV_FS_MEMFS_LETTER,
        charge_info->charge_battery_info.data, charge_info->charge_battery_info.size, "baj");
    lv_image_set_src(charge_info->charge_battery, charge_battery_img_path.path);
    lv_obj_align(charge_info->charge_battery, LV_ALIGN_TOP_LEFT, 141 + 3, 148);

    charge_info->percent = percent;
    return 0;
}

static int charge_animation_update(ui_manager_context_t *thiz, uint16_t animation_index)
{
    if (charge_info->force_update == false) {
        return 0;
    }
    
    if (charge_info->charge_animation == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->charge_animation = lv_image_create(charge_info->root);
        EXCEPTION_CHECK_INT(charge_info->charge_animation);
    }
    common_ui_img_resource_init(thiz, RESOURCE_ID_CHARGE_ANIMATION_START + animation_index,
         &charge_info->charge_animation_info);
    EXCEPTION_CHECK_INT(charge_info->charge_animation_info.data);
    lv_fs_path_ex_t charge_animation_img_path;
    lv_fs_make_path_from_buffer(&charge_animation_img_path, LV_FS_MEMFS_LETTER,
        charge_info->charge_animation_info.data, charge_info->charge_animation_info.size, "baj");
    lv_image_set_src(charge_info->charge_animation, charge_animation_img_path.path);
    lv_obj_align(charge_info->charge_animation, LV_ALIGN_CENTER, 0, 0);
    
    return 0;
}

static int charge_static_object_update(ui_manager_context_t *thiz)
{
    if (charge_info->force_update == false) {
        return 0;
    }
    
    /// 更新文本.
    if (charge_info->text == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->text = lv_label_create(charge_info->root);
        EXCEPTION_CHECK_INT(charge_info->text);
    }
    lv_label_set_text(charge_info->text, "Super charging");
    lv_obj_set_style_text_color(charge_info->text, lv_color_hex(CHARGE_TEXT_COLOR), 0);
    lv_obj_set_style_text_opa(charge_info->text, LV_OPA_70, 0);  /// 设置文本不透明度为60%
    lv_obj_set_style_text_font(charge_info->text, &lv_font_roboto_19_regular, 0);  /// 设置字体
    lv_obj_set_style_text_line_space(charge_info->text, 4, 0);  /// 设置行间距（像素）
    lv_obj_set_style_text_align(charge_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_align(charge_info->text, LV_ALIGN_TOP_LEFT, 116 + 4, 200 + 3);

    /// 更新充电图标.
    if (charge_info->charge_icon == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->charge_icon = lv_image_create(charge_info->root);
        EXCEPTION_CHECK_INT(charge_info->charge_icon);
    }
    common_ui_img_resource_init(thiz, RESOURCE_ID_CHARGE_PAGE_ICON, &charge_info->charge_icon_info);
    EXCEPTION_CHECK_INT(charge_info->charge_icon_info.data);
    lv_fs_path_ex_t charge_icon_img_path;
    lv_fs_make_path_from_buffer(&charge_icon_img_path, LV_FS_MEMFS_LETTER,
        charge_info->charge_icon_info.data, charge_info->charge_icon_info.size, "baj");
    lv_image_set_src(charge_info->charge_icon, charge_icon_img_path.path);
    lv_obj_align(charge_info->charge_icon, LV_ALIGN_TOP_LEFT, 137 + 4, 308);
    
    return 0;
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
    charge_ui_root_init();  
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    charge_info->force_update = true;
}

void ui_display_charge_screen_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(charge_info);
    common_ui_img_resource_deinit(thiz, &charge_info->charge_icon_info);
    common_ui_img_resource_deinit(thiz, &charge_info->charge_animation_info);
    common_ui_img_resource_deinit(thiz, &charge_info->charge_battery_info);
    LVGL_OBJ_DELETE(charge_info->root);
    u_free(charge_info);
    charge_info = NULL;
}

int ui_display_charge_screen(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(charge_info);
    EXCEPTION_CHECK_INT(charge_info->root);
    ui_proc_charge_t *proc = (ui_proc_charge_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    // charge_bg_update(thiz);
    charge_animation_update(thiz, 0);
    charge_battery_update(thiz, proc);
    charge_static_object_update(thiz);
    thiz->priv.fps_count++;
    charge_info->force_update = false; 
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> charge fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
        
    }
    return 0;
}