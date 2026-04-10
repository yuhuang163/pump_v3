/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_popup.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define COUNT_DOWN_TEXT_COLOR                                        0xC74B57
#define COUNT_DOWN_NUMBER_COLOR                                      0xFFFFFF
#define COUNT_DOWN_ARC_SIZE                                          (360)    // 圆弧尺寸
#define COUNT_DOWN_ARC_WIDTH                                         (12)      // 圆弧粗细
#define COUNT_DOWN_ARC_COLOR                                         (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define COUNT_DOWN_ARC_START_ANGLE                                   (270)    
#define COUNT_DOWN_ARC_FULL_SWEEP                                    (360)    
#define COUNT_DOWN_DISPLAY_STEP_TIME                                 (100)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * arc;                   /// 圆弧进度条对象
    lv_obj_t * number_text;           /// 数字文本对象
    lv_obj_t * mask_layer;            /// 遮罩层对象
    uint32_t start_time;              /// 倒计时起始时间
    uint16_t ring_progress;           /// 圆弧进度
    uint16_t ring_progress_max;       /// 圆弧进度最大值
    uint16_t target_time;             /// 倒计时目标时间 单位：秒
    uint8_t force_update;
    uint8_t display_number;           /// 显示倒计时数字
    uint8_t rsv[14];
} count_down_tip_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static count_down_tip_display_info_t *count_down_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void count_down_tip_root_init(void)
{
    if (count_down_info->root == NULL || !lv_obj_is_valid(count_down_info->root)) {
        count_down_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(count_down_info->root);
    }
    lv_obj_set_size(count_down_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%
    lv_obj_set_style_bg_color(count_down_info->root, lv_color_black(), 0);     /// 黑色背景
    lv_obj_set_style_bg_opa(count_down_info->root, LV_OPA_TRANSP, 0);          /// 背景填充透明（不画背景色，避免遮挡底图）
    lv_obj_set_style_border_width(count_down_info->root, 0, 0);                /// 无边框
    lv_obj_set_style_pad_all(count_down_info->root, 0, 0);                     /// 无内边距
    lv_obj_align(count_down_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(count_down_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 不拦截点击事件
    lv_obj_clear_flag(count_down_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void count_down_tip_text_update(ui_manager_context_t *thiz)
{
    uint8_t num = get_timestamp_ms(count_down_info->start_time) / 1000;
    uint8_t show_num = (num > 3 ? 0 : 3 - num);
    if (count_down_info->force_update == false && count_down_info->display_number == show_num) {
        return;
    }
    count_down_info->display_number = show_num;
    if (count_down_info->number_text == NULL || !lv_obj_is_valid(count_down_info->number_text)) {
        count_down_info->number_text = lv_label_create(count_down_info->root);
        EXCEPTION_CHECK_VOID(count_down_info->number_text);
        lv_obj_set_style_text_color(count_down_info->number_text, lv_color_hex(COUNT_DOWN_NUMBER_COLOR), 0);
        lv_obj_set_style_text_font(count_down_info->number_text, &lv_font_oppo_sans_96_bold, 0);
        lv_obj_align(count_down_info->number_text, LV_ALIGN_CENTER, 0, 0);
    }
    char number_str[2] = {0};
    snprintf(number_str, sizeof(number_str), "%d", show_num);
    lv_label_set_text(count_down_info->number_text, number_str);
}

static void count_down_tip_arc_update(ui_manager_context_t *thiz)
{
    uint16_t ring_progress = get_timestamp_ms(count_down_info->start_time) / COUNT_DOWN_DISPLAY_STEP_TIME;
    if (count_down_info->force_update == false && count_down_info->ring_progress == ring_progress) {
        return;
    }
    count_down_info->ring_progress = ring_progress;
    if (count_down_info->arc == NULL || !lv_obj_is_valid(count_down_info->arc)) {
        count_down_info->arc = lv_arc_create(count_down_info->root);
        EXCEPTION_CHECK_VOID(count_down_info->arc);
        lv_obj_set_size(count_down_info->arc, COUNT_DOWN_ARC_SIZE, COUNT_DOWN_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(count_down_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(count_down_info->arc, 0, COUNT_DOWN_ARC_FULL_SWEEP);  /// 背景弧为完整闭环
        lv_arc_set_angles(count_down_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(count_down_info->arc, COUNT_DOWN_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(count_down_info->arc, lv_color_hex(COUNT_DOWN_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(count_down_info->arc, COUNT_DOWN_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(count_down_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(count_down_info->arc, lv_color_hex(COUNT_DOWN_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(count_down_info->arc, COUNT_DOWN_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(count_down_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(count_down_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (ring_progress >= count_down_info->ring_progress_max) {
        lv_obj_set_style_arc_opa(count_down_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(count_down_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(count_down_info->arc, 0, 0);

        return;
    }
    lv_obj_set_style_arc_opa(count_down_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(count_down_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (COUNT_DOWN_ARC_FULL_SWEEP * ring_progress) / count_down_info->ring_progress_max;
    }
    lv_arc_set_angles(count_down_info->arc, 0, end_angle);
}

static int count_down_tip_mask_layer_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    if (count_down_info->mask_layer == NULL || !lv_obj_is_valid(count_down_info->mask_layer)) {
        count_down_info->mask_layer = lv_obj_create(count_down_info->root);
        EXCEPTION_CHECK_INT(count_down_info->mask_layer);
        lv_obj_set_size(count_down_info->mask_layer, LV_PCT(100), LV_PCT(100));
        lv_obj_center(count_down_info->mask_layer);
        lv_obj_set_style_bg_color(count_down_info->mask_layer, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(count_down_info->mask_layer, 190, 0); /// 黑色背景透明度190
        lv_obj_set_style_border_width(count_down_info->mask_layer, 0, 0);
        lv_obj_set_style_pad_all(count_down_info->mask_layer, 0, 0);
        lv_obj_clear_flag(count_down_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_move_background(count_down_info->mask_layer); /// 确保遮罩层在最底层（作为背景遮罩）
    }
    return 0;
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_count_down_popup_prepare(ui_manager_context_t *thiz, uint16_t time)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (count_down_info == NULL) {
        count_down_info = (count_down_tip_display_info_t *)u_malloz(sizeof(count_down_tip_display_info_t));
        EXCEPTION_CHECK_VOID(count_down_info);
    }
    count_down_tip_root_init();
    count_down_info->force_update = true;
    count_down_info->display_number = 0;
    count_down_info->ring_progress = 0;
    count_down_info->target_time = time;
    count_down_info->ring_progress_max = (time / COUNT_DOWN_DISPLAY_STEP_TIME) - 2;
    count_down_info->start_time = get_timestamp();
    // LOG_UI("ui_display_count_down_popup_prepare: time=%d", time);
}

void ui_display_count_down_popup_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && count_down_info);
    LVGL_OBJ_DELETE(count_down_info->root);
    u_free(count_down_info);
    count_down_info = NULL;
}

int ui_display_count_down_popup_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && count_down_info && count_down_info->root);
    count_down_tip_mask_layer_update(thiz);
    count_down_tip_arc_update(thiz);
    count_down_tip_text_update(thiz);
    count_down_info->force_update = false;
    if (get_timestamp_ms(count_down_info->start_time) >= count_down_info->target_time) {
        ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
        EXCEPTION_CHECK_INT(ani);
        ani->progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
    }
    LOG_UI("ui_display_count_down_popup_process: time=%d", count_down_info->display_number);
    return 0;
}