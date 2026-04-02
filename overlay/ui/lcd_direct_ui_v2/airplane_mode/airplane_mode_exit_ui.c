/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "airplane_mode_ui.h" // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/
#define AIRPLANE_MODE_EXIT_ARC_SIZE                       (360)   
#define AIRPLANE_MODE_EXIT_ARC_WIDTH                      (12)  
#define AIRPLANE_MODE_EXIT_ARC_COLOR                      (0xB25962) 
#define AIRPLANE_MODE_EXIT_MAX_VALUE                      (5)     
#define AIRPLANE_MODE_EXIT_ARC_START_ANGLE                (270)    
#define AIRPLANE_MODE_EXIT_ARC_FULL_SWEEP                 (360)  
#define AIRPLANE_MODE_EXIT_DISPLAY_TIME                   (5000)  
#define AIRPLANE_MODE_EXIT_DISPLAY_STEP_TIME              (100)
#define AIRPLANE_MODE_EXIT_DISPLAY_MAX_VALUE              ((AIRPLANE_MODE_EXIT_DISPLAY_TIME / AIRPLANE_MODE_EXIT_DISPLAY_STEP_TIME) - 1)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * number_text;
    lv_obj_t * arc;
    lv_obj_t * mask_layer; /// 遮罩层
    uint8_t force_update;
    uint8_t display_number; /// 显示倒计时数字
    uint16_t ring_progress; /// 圆弧进度
    uint32_t start_time; /// 开始显示时间
} airplane_mode_exit_display_info_t;
/*
* TYPEDEFS
*****************************************************************************************
*/
static airplane_mode_exit_display_info_t *display_info;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void airplane_mode_exit_root_init(lv_obj_t *parent)
{
    if (display_info->root == NULL || !lv_obj_is_valid(display_info->root)) {
        display_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(display_info->root);
        lv_obj_set_size(display_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%
        lv_obj_set_style_bg_color(display_info->root, lv_color_black(), 0);     /// 黑色背景
        lv_obj_set_style_bg_opa(display_info->root, LV_OPA_TRANSP, 0);          /// 背景填充透明（不画背景色，避免遮挡底图）
        lv_obj_set_style_border_width(display_info->root, 0, 0);                /// 无边框
        lv_obj_set_style_pad_all(display_info->root, 0, 0);                     /// 无内边距
        lv_obj_align(display_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(display_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 不拦截点击事件
        lv_obj_clear_flag(display_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
    }
}

static void airplane_mode_exit_progress_arc_update(ui_manager_context_t *thiz)
{
    uint16_t ring_progress = get_timestamp_ms(display_info->start_time) / AIRPLANE_MODE_EXIT_DISPLAY_STEP_TIME;
    if (display_info->force_update == false && display_info->ring_progress == ring_progress) {
        return;
    }
    display_info->ring_progress = ring_progress;
    if (display_info->arc == NULL || !lv_obj_is_valid(display_info->arc)) {
        display_info->arc = lv_arc_create(display_info->root);
        EXCEPTION_CHECK_VOID(display_info->arc);
        lv_obj_set_size(display_info->arc, AIRPLANE_MODE_EXIT_ARC_SIZE, AIRPLANE_MODE_EXIT_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(display_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(display_info->arc, 0, AIRPLANE_MODE_EXIT_ARC_FULL_SWEEP);  /// 背景弧为完整闭环
        lv_arc_set_angles(display_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(display_info->arc, AIRPLANE_MODE_EXIT_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(display_info->arc, lv_color_hex(AIRPLANE_MODE_EXIT_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(display_info->arc, AIRPLANE_MODE_EXIT_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(display_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(display_info->arc, lv_color_hex(AIRPLANE_MODE_EXIT_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(display_info->arc, AIRPLANE_MODE_EXIT_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(display_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(display_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (ring_progress > AIRPLANE_MODE_EXIT_DISPLAY_MAX_VALUE) {
        ring_progress = AIRPLANE_MODE_EXIT_DISPLAY_MAX_VALUE;
    }

    if (ring_progress >= AIRPLANE_MODE_EXIT_DISPLAY_MAX_VALUE) {
        lv_obj_set_style_arc_opa(display_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(display_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(display_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(display_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(display_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (AIRPLANE_MODE_EXIT_ARC_FULL_SWEEP * ring_progress) / AIRPLANE_MODE_EXIT_DISPLAY_MAX_VALUE;
    }
    lv_arc_set_angles(display_info->arc, 0, end_angle);
}

static void airplane_mode_exit_countdown_num_update(ui_manager_context_t *thiz)
{
    if (display_info->number_text == NULL || !lv_obj_is_valid(display_info->number_text)) {
        display_info->number_text = lv_label_create(display_info->root);
        EXCEPTION_CHECK_VOID(display_info->number_text);
    }
    char num_buf[8] = {0};
    uint8_t num = get_timestamp_ms(display_info->start_time) / 1000;
    uint8_t show_num = (num > 5 ? 0 : 5 - num);
    if (display_info->force_update == false &&
        display_info->display_number == show_num) {
        return;
    }
    snprintf(num_buf, sizeof(num_buf), "%d", show_num);
    lv_label_set_text(display_info->number_text, num_buf);
    lv_obj_set_style_text_color(display_info->number_text, lv_color_white(), 0);
    lv_obj_set_style_text_font(display_info->number_text, &lv_font_oppo_sans_96_bold, 0);
    lv_obj_set_style_text_align(display_info->number_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(display_info->number_text, LV_ALIGN_CENTER, 0, 0);
    display_info->display_number = show_num;
}

static void airplane_mode_exit_mask_layer_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (display_info->mask_layer == NULL || !lv_obj_is_valid(display_info->mask_layer)) {
        display_info->mask_layer = lv_obj_create(display_info->root);
        EXCEPTION_CHECK_VOID(display_info->mask_layer);
        lv_obj_set_size(display_info->mask_layer, LV_PCT(100), LV_PCT(100));
        lv_obj_center(display_info->mask_layer);
        lv_obj_set_style_bg_color(display_info->mask_layer, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(display_info->mask_layer, 190, 0); /// 黑色背景透明度190
        lv_obj_set_style_border_width(display_info->mask_layer, 0, 0);
        lv_obj_set_style_pad_all(display_info->mask_layer, 0, 0);
        lv_obj_clear_flag(display_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_airplane_mode_exit_prepare(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (display_info == NULL) {
        display_info = (airplane_mode_exit_display_info_t *)u_malloz(sizeof(airplane_mode_exit_display_info_t));
        EXCEPTION_CHECK_VOID(display_info);
    }
    display_info->force_update = true;
    display_info->ring_progress = 0;
    display_info->display_number = 0;
    display_info->start_time = get_timestamp();
    airplane_mode_exit_root_init(parent);
    LOG_UI("airplane mode exit prepare");
}

void ui_display_airplane_mode_exit_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (display_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(display_info->root);
    u_free(display_info);
    display_info = NULL;
}

void ui_display_airplane_mode_exit_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (display_info == NULL || display_info->root == NULL) {
        return;
    }
    airplane_mode_exit_mask_layer_update(thiz);
    airplane_mode_exit_progress_arc_update(thiz);
    airplane_mode_exit_countdown_num_update(thiz);
    display_info->force_update = false;
    if (get_timestamp_ms(display_info->start_time) > AIRPLANE_MODE_EXIT_DISPLAY_TIME + 200) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        LOG_UI("display completed");
        ani->progress_cb(thiz, AIRPLANE_MODE_POPUP_UI_EXIT, ANI_POPUP_DISPLAY_FINISH);
    }
}
