/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "persist_storage.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_popup.h"   // IWYU pragma: keep
#include "windows_manager.h"

/*
* DEFINES
*****************************************************************************************
*/
#define BATTERY_LOW_POWER_TEXT_COLOR                                        0xC74B57
#define BATTERY_LOW_POWER_NUMBER_COLOR                                      0xFFFFFF
#define BATTERY_LOW_POWER_POP_ARC_SIZE                                      (360)    // 圆弧尺寸
#define BATTERY_LOW_POWER_POP_ARC_WIDTH                                     (12)      // 圆弧粗细
#define BATTERY_LOW_POWER_POP_ARC_COLOR                                     (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define BATTERY_LOW_POWER_POP_ARC_START_ANGLE                               (270)    
#define BATTERY_LOW_POWER_POP_ARC_FULL_SWEEP                                (360)    
#define BATTERY_LOW_POWER_POP_DISPLAY_TIME                                  (3000)  
#define BATTERY_LOW_POWER_POP_DISPLAY_STEP_TIME                             (100)
#define BATTERY_LOW_POWER_POP_DISPLAY_MAX_VALUE                             ((BATTERY_LOW_POWER_POP_DISPLAY_TIME / \
    BATTERY_LOW_POWER_POP_DISPLAY_STEP_TIME) - 2)
  
typedef struct {    
    lv_obj_t * root;
    lv_obj_t * text;
    lv_obj_t * arc;                   /// 圆弧进度条对象
    lv_obj_t * number_text;           /// 数字文本对象
    lv_obj_t * mask_layer;            /// 遮罩层对象
    uint8_t force_update;
    uint8_t display_number;           /// 显示倒计时数字
    uint16_t ring_progress;           /// 圆弧进度
    uint32_t start_time;              /// 倒计时起始时间
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
static void battery_low_power_pop_ui_root_init(void)
{
    if (low_power_info->root == NULL || !lv_obj_is_valid(low_power_info->root)) {
        low_power_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(low_power_info->root);
    }
    lv_obj_set_size(low_power_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(low_power_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(low_power_info->root, LV_OPA_TRANSP, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(low_power_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(low_power_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(low_power_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}


static void battery_low_power_pop_ui_arc_update(ui_manager_context_t *thiz)
{
    uint16_t ring_progress = get_timestamp_ms(low_power_info->start_time) / BATTERY_LOW_POWER_POP_DISPLAY_STEP_TIME;
    if (low_power_info->force_update == false && low_power_info->ring_progress == ring_progress) {
        return;
    }
    low_power_info->ring_progress = ring_progress;
    if (low_power_info->arc == NULL || !lv_obj_is_valid(low_power_info->arc)) {
        low_power_info->arc = lv_arc_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->arc);
        lv_obj_set_size(low_power_info->arc, BATTERY_LOW_POWER_POP_ARC_SIZE, BATTERY_LOW_POWER_POP_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(low_power_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(low_power_info->arc, 0, BATTERY_LOW_POWER_POP_ARC_FULL_SWEEP);  /// 背景弧为完整闭环
        lv_arc_set_angles(low_power_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(low_power_info->arc, BATTERY_LOW_POWER_POP_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(low_power_info->arc, lv_color_hex(BATTERY_LOW_POWER_POP_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(low_power_info->arc, BATTERY_LOW_POWER_POP_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(low_power_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(low_power_info->arc, lv_color_hex(BATTERY_LOW_POWER_POP_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(low_power_info->arc, BATTERY_LOW_POWER_POP_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(low_power_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(low_power_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (ring_progress >= BATTERY_LOW_POWER_POP_DISPLAY_MAX_VALUE) {
        lv_obj_set_style_arc_opa(low_power_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(low_power_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(low_power_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(low_power_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(low_power_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (BATTERY_LOW_POWER_POP_ARC_FULL_SWEEP * ring_progress) / BATTERY_LOW_POWER_POP_DISPLAY_MAX_VALUE;
    }
    lv_arc_set_angles(low_power_info->arc, 0, end_angle);
}

static void battery_low_power_pop_ui_text_update(ui_manager_context_t *thiz)
{ 
    uint8_t num = get_timestamp_ms(low_power_info->start_time) / 1000;
    uint8_t show_num = (num > 3 ? 0 : 3 - num);
    if (low_power_info->force_update == false && low_power_info->display_number == show_num) {
        return;
    }

    low_power_info->display_number = show_num;
    if (low_power_info->text == NULL || !lv_obj_is_valid(low_power_info->text)) {
        low_power_info->text = lv_label_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->text);
        lv_label_set_text(low_power_info->text, "Powering off in");
        lv_obj_set_style_text_color(low_power_info->text, lv_color_hex(BATTERY_LOW_POWER_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(low_power_info->text, &lv_font_oppo_sans_16_medium, 0);
        lv_obj_align(low_power_info->text, LV_ALIGN_TOP_MID, 0, 111);
    }

    if (low_power_info->number_text == NULL || !lv_obj_is_valid(low_power_info->number_text)) {
        low_power_info->number_text = lv_label_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->number_text);
        lv_obj_set_style_text_color(low_power_info->number_text, lv_color_hex(BATTERY_LOW_POWER_NUMBER_COLOR), 0);
        lv_obj_set_style_text_font(low_power_info->number_text, &lv_font_oppo_sans_96_bold, 0);
        lv_obj_align(low_power_info->number_text, LV_ALIGN_CENTER, 0, 0);
    }
    char number_str[2] = {0};
    snprintf(number_str, sizeof(number_str), "%d", show_num);
    lv_label_set_text(low_power_info->number_text, number_str);
}

static void battery_low_power_pop_ui_mask_layer_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (low_power_info->mask_layer == NULL || !lv_obj_is_valid(low_power_info->mask_layer)) {
        low_power_info->mask_layer = lv_obj_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->mask_layer);
        lv_obj_set_size(low_power_info->mask_layer, LV_PCT(100), LV_PCT(100));
        lv_obj_center(low_power_info->mask_layer);
        lv_obj_set_style_bg_color(low_power_info->mask_layer, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(low_power_info->mask_layer, 190, 0); /// 黑色背景透明度190
        lv_obj_set_style_border_width(low_power_info->mask_layer, 0, 0);
        lv_obj_set_style_pad_all(low_power_info->mask_layer, 0, 0);
        lv_obj_clear_flag(low_power_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_low_power_popup_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (low_power_info == NULL) {
        low_power_info = (battery_low_power_display_info_t *)u_malloz(sizeof(battery_low_power_display_info_t));
        EXCEPTION_CHECK_VOID(low_power_info);
    }

    battery_low_power_pop_ui_root_init();
    low_power_info->force_update = true;
    low_power_info->display_number = 0;
    low_power_info->ring_progress = 0;
    low_power_info->start_time = get_timestamp();
}

void ui_display_low_power_popup_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(low_power_info);
    LVGL_OBJ_DELETE(low_power_info->root);
    u_free(low_power_info);
    low_power_info = NULL;
}

int ui_display_low_power_popup_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(low_power_info);
    EXCEPTION_CHECK_INT(low_power_info->root);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    EXCEPTION_CHECK_INT(battery);
    battery_low_power_pop_ui_mask_layer_update(thiz);
    battery_low_power_pop_ui_arc_update(thiz);
    battery_low_power_pop_ui_text_update(thiz);
    low_power_info->force_update = false;
    bool is_low_battery = battery->is_low_battery();
    if (get_timestamp_ms(low_power_info->start_time) >= BATTERY_LOW_POWER_POP_DISPLAY_TIME || is_low_battery == 0) {
        ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
        EXCEPTION_CHECK_INT(ani);
        LOG_UI("%s", is_low_battery == 0 ? "battery is not low power, ignore." : "battery low power pop process completed");
        ani->progress_cb(thiz, is_low_battery, ANI_POPUP_DISPLAY_FINISH);
    }
    return 0;
}