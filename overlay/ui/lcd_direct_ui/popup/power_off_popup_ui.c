/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_popup.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define WILL_POWER_OFF_TEXT_COLOR                                        0xC74B57
#define WILL_POWER_OFF_NUMBER_COLOR                                      0xFFFFFF
#define WILL_POWER_OFF_ARC_SIZE                                          (360)    // 圆弧尺寸
#define WILL_POWER_OFF_ARC_WIDTH                                         (12)      // 圆弧粗细
#define WILL_POWER_OFF_ARC_COLOR                                         (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define WILL_POWER_OFF_ARC_START_ANGLE                                   (270)    
#define WILL_POWER_OFF_ARC_FULL_SWEEP                                    (360)    
#define WILL_POWER_OFF_DISPLAY_TIME                                      (3000)  
#define WILL_POWER_OFF_DISPLAY_STEP_TIME                                 (100)
#define WILL_POWER_OFF_DISPLAY_MAX_VALUE                                 ((WILL_POWER_OFF_DISPLAY_TIME / \
    WILL_POWER_OFF_DISPLAY_STEP_TIME) - 2)
  
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
} power_off_tip_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static power_off_tip_display_info_t *power_off_info;


/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void will_power_off_tip_root_init(void)
{
    if (power_off_info->root == NULL || !lv_obj_is_valid(power_off_info->root)) {
        power_off_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(power_off_info->root);
        lv_obj_set_size(power_off_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%
        lv_obj_set_style_bg_color(power_off_info->root, lv_color_black(), 0);     /// 黑色背景
        lv_obj_set_style_bg_opa(power_off_info->root, LV_OPA_TRANSP, 0);          /// 背景填充透明（不画背景色，避免遮挡底图）
        lv_obj_set_style_border_width(power_off_info->root, 0, 0);                /// 无边框
        lv_obj_set_style_pad_all(power_off_info->root, 0, 0);                     /// 无内边距
        lv_obj_align(power_off_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(power_off_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 不拦截点击事件
        lv_obj_clear_flag(power_off_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
    }
}

static void will_power_off_tip_text_update(ui_manager_context_t *thiz)
{
    uint8_t num = get_timestamp_ms(power_off_info->start_time) / 1000;
    uint8_t show_num = (num > 3 ? 0 : 3 - num);
    if (power_off_info->force_update == false && power_off_info->display_number == show_num) {
        return;
    }

    power_off_info->display_number = show_num;
    if (power_off_info->text == NULL || !lv_obj_is_valid(power_off_info->text)) {
        power_off_info->text = lv_label_create(power_off_info->root);
        EXCEPTION_CHECK_VOID(power_off_info->text);
        lv_label_set_text(power_off_info->text, "Shut down soon");
        lv_obj_set_style_text_color(power_off_info->text, lv_color_hex(WILL_POWER_OFF_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(power_off_info->text, &lv_font_oppo_sans_16_medium, 0);
        lv_obj_align(power_off_info->text, LV_ALIGN_TOP_MID, 0, 111);
    }

    if (power_off_info->number_text == NULL || !lv_obj_is_valid(power_off_info->number_text)) {
        power_off_info->number_text = lv_label_create(power_off_info->root);
        EXCEPTION_CHECK_VOID(power_off_info->number_text);
        lv_obj_set_style_text_color(power_off_info->number_text, lv_color_hex(WILL_POWER_OFF_NUMBER_COLOR), 0);
        lv_obj_set_style_text_font(power_off_info->number_text, &lv_font_oppo_sans_96_bold, 0);
        lv_obj_align(power_off_info->number_text, LV_ALIGN_CENTER, 0, 0);
    }
    char number_str[2] = {0};
    snprintf(number_str, sizeof(number_str), "%d", show_num);
    lv_label_set_text(power_off_info->number_text, number_str);
}

static void will_power_off_tip_arc_update(ui_manager_context_t *thiz)
{
    uint16_t ring_progress = get_timestamp_ms(power_off_info->start_time) / WILL_POWER_OFF_DISPLAY_STEP_TIME;
    if (power_off_info->force_update == false && power_off_info->ring_progress == ring_progress) {
        return;
    }
    power_off_info->ring_progress = ring_progress;
    if (power_off_info->arc == NULL || !lv_obj_is_valid(power_off_info->arc)) {
        power_off_info->arc = lv_arc_create(power_off_info->root);
        EXCEPTION_CHECK_VOID(power_off_info->arc);
        lv_obj_set_size(power_off_info->arc, WILL_POWER_OFF_ARC_SIZE, WILL_POWER_OFF_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(power_off_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(power_off_info->arc, 0, WILL_POWER_OFF_ARC_FULL_SWEEP);  /// 背景弧为完整闭环
        lv_arc_set_angles(power_off_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(power_off_info->arc, WILL_POWER_OFF_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(power_off_info->arc, lv_color_hex(WILL_POWER_OFF_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(power_off_info->arc, WILL_POWER_OFF_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(power_off_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(power_off_info->arc, lv_color_hex(WILL_POWER_OFF_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(power_off_info->arc, WILL_POWER_OFF_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(power_off_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(power_off_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (ring_progress >= WILL_POWER_OFF_DISPLAY_MAX_VALUE) {
        lv_obj_set_style_arc_opa(power_off_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(power_off_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(power_off_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(power_off_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(power_off_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (WILL_POWER_OFF_ARC_FULL_SWEEP * ring_progress) / WILL_POWER_OFF_DISPLAY_MAX_VALUE;
    }
    lv_arc_set_angles(power_off_info->arc, 0, end_angle);
}

static int will_power_off_tip_mask_layer_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    if (power_off_info->mask_layer == NULL || !lv_obj_is_valid(power_off_info->mask_layer)) {
        power_off_info->mask_layer = lv_obj_create(power_off_info->root);
        EXCEPTION_CHECK_INT(power_off_info->mask_layer);
        lv_obj_set_size(power_off_info->mask_layer, LV_PCT(100), LV_PCT(100));
        lv_obj_center(power_off_info->mask_layer);
        lv_obj_set_style_bg_color(power_off_info->mask_layer, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(power_off_info->mask_layer, 190, 0); /// 黑色背景透明度190
        lv_obj_set_style_border_width(power_off_info->mask_layer, 0, 0);
        lv_obj_set_style_pad_all(power_off_info->mask_layer, 0, 0);
        lv_obj_clear_flag(power_off_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
        /// 确保遮罩层在最底层（作为背景遮罩）
        lv_obj_move_background(power_off_info->mask_layer);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_power_off_popup_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (power_off_info == NULL) {
        power_off_info = (power_off_tip_display_info_t *)u_malloz(sizeof(power_off_tip_display_info_t));
        EXCEPTION_CHECK_VOID(power_off_info);
    }

    will_power_off_tip_root_init();
    power_off_info->force_update = true;
    power_off_info->display_number = 0;
    power_off_info->ring_progress = 0;
    power_off_info->start_time = get_timestamp();
}

void ui_display_power_off_popup_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (power_off_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(power_off_info->root);
    u_free(power_off_info);
    power_off_info = NULL;
}

int ui_display_power_off_popup_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    if (power_off_info == NULL || power_off_info->root == NULL) {
        return 0;
    }   
    will_power_off_tip_mask_layer_update(thiz);
    will_power_off_tip_arc_update(thiz);
    will_power_off_tip_text_update(thiz);
    power_off_info->force_update = false;
    if (get_timestamp_ms(power_off_info->start_time) >= WILL_POWER_OFF_DISPLAY_TIME) {
        ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
        EXCEPTION_CHECK_INT(ani);
        ani->progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
    }
    return 0;
}