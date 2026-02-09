/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {    
    lv_obj_t * root;
    lv_obj_t * text;
    lv_obj_t * countdown;
    uint8_t countdown_value;
    uint8_t force_update;
    uint32_t start_time;
} battery_low_power_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static battery_low_power_display_info_t *low_power_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void battery_low_power_root_init(void)
{
    if (low_power_info->root == NULL || !lv_obj_is_valid(low_power_info->root)) {
        low_power_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(low_power_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(low_power_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(low_power_info->root, LV_OPA_50, 0);              /// 设置背景透明度为10%
    lv_obj_set_style_border_width(low_power_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(low_power_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(low_power_info->root, LV_ALIGN_CENTER, 0, 0);                /// 设置容器居中
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static int battery_low_power_text_update(ui_manager_context_t *thiz)
{    
    if (low_power_info->force_update == false) {
        return 0;
    }

    if (low_power_info->text == NULL || !lv_obj_is_valid(low_power_info->root)) {
        low_power_info->text = lv_label_create(low_power_info->root);
        EXCEPTION_CHECK_INT(low_power_info->text);
    }
    lv_label_set_text(low_power_info->  text, "Battery level is too low\nabout to shut down");
    lv_obj_set_style_text_color(low_power_info->text, lv_color_black(), 0);
    lv_obj_set_style_text_font(low_power_info->text, &lv_font_roboto_19_regular, 0);
    lv_obj_set_style_text_line_space(low_power_info->text, 4, 0);
    lv_obj_set_style_text_align(low_power_info->text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(low_power_info->text, LV_ALIGN_TOP_LEFT, 100, 100);
    return 0;
}

static int battery_low_power_countdown_update(ui_manager_context_t *thiz)
{
    if (get_timestamp() - low_power_info->start_time < 1000) {
        return 0;
    }
    
    if (low_power_info->countdown == NULL || !lv_obj_is_valid(low_power_info->root)) {
        low_power_info->countdown = lv_label_create(low_power_info->root);
        EXCEPTION_CHECK_INT(low_power_info->countdown);
    }
    char countdown_value[4] = {0};
    sprintf(countdown_value, "%d", low_power_info->countdown_value);
    lv_label_set_text(low_power_info->countdown, countdown_value);
    lv_obj_set_style_text_color(low_power_info->countdown, lv_color_black(), 0);
    lv_obj_set_style_text_font(low_power_info->countdown, &lv_font_roboto_79_semibold, 0);
    lv_obj_set_style_text_line_space(low_power_info->countdown, 4, 0);
    lv_obj_set_style_text_align(low_power_info->countdown, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(low_power_info->countdown, LV_ALIGN_CENTER, 0, 0);
    if (low_power_info->countdown_value > 0) {
        low_power_info->countdown_value--;
    }
    low_power_info->start_time = get_timestamp();
    
    return 0;
}

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
    battery_low_power_root_init();  
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = 0;
    low_power_info->force_update = true;
    low_power_info->countdown_value = 3;
}

void ui_display_battery_low_power_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(low_power_info);
    LVGL_OBJ_DELETE(low_power_info->root);
    u_free(low_power_info);
    low_power_info = NULL;
}

int ui_display_battery_low_power_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(low_power_info);
    EXCEPTION_CHECK_INT(low_power_info->root);
    battery_low_power_text_update(thiz);
    battery_low_power_countdown_update(thiz);
    thiz->priv.fps_count++;
    low_power_info->force_update = false;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> battery low power fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}