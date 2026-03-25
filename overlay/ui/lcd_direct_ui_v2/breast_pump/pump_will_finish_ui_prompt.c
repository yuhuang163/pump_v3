/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h"   // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_WILL_FINISH_TIP_COMP_BG                        (0)
#define WILL_FINISH_TIP_TEXT_COLOR                                0xFFFFFF
#define WILL_FINISH_ARC_SIZE                                      (360)    // 圆弧尺寸
#define WILL_FINISH_ARC_WIDTH                                     (12)      // 圆弧粗细
#define WILL_FINISH_ARC_COLOR                                     (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define WILL_FINISH_ARC_START_ANGLE                               (270)    
#define WILL_FINISH_ARC_FULL_SWEEP                                (360)    
#define WILL_FINISH_DISPLAY_TIME                                  (3000)  
#define WILL_FINISH_DISPLAY_STEP_TIME                             (100)
#define WILL_FINISH_DISPLAY_MAX_VALUE                             ((WILL_FINISH_DISPLAY_TIME / WILL_FINISH_DISPLAY_STEP_TIME) - 2)
  
typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_WILL_FINISH_TIP_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * text;
    lv_obj_t * arc;            /// 圆弧进度条对象
    lv_obj_t * mask_layer;               /// 遮罩层对象
    uint8_t countdown_value;            /// 倒计时值
    uint8_t display_number;               /// 显示倒计时数字
    uint32_t start_time;                 /// 倒计时起始时间
    uint8_t force_update;
} will_finish_tip_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_COMPLETE_TIP_COMP_BG
static PSRAM_DATA_LOC uint8_t will_finish_tip_bg_data[] = {
    #include "comparison_png2txt/pump_finish_comp_360x360.txt"
};
#endif

static will_finish_tip_display_info_t *tip_info;


/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void will_finish_tip_root_init(lv_obj_t *parent)
{
    if (tip_info->root == NULL || !lv_obj_is_valid(tip_info->root)) {
        tip_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(tip_info->root);
    }
    lv_obj_set_size(tip_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(tip_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(tip_info->root, LV_OPA_TRANSP, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(tip_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(tip_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(tip_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(tip_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(tip_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void will_finish_tip_arc_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (tip_info->force_update == false &&
        tip_info->display_number == tip_info->countdown_value) {
        return;
    }

    tip_info->display_number = tip_info->countdown_value;
    if (tip_info->arc == NULL || !lv_obj_is_valid(tip_info->arc)) {
        tip_info->arc = lv_arc_create(tip_info->root);
        EXCEPTION_CHECK_VOID(tip_info->arc);
        lv_obj_set_size(tip_info->arc, WILL_FINISH_ARC_SIZE, WILL_FINISH_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(tip_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(tip_info->arc, 0, WILL_FINISH_ARC_FULL_SWEEP);  /// 背景弧为完整闭环
        lv_arc_set_angles(tip_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(tip_info->arc, WILL_FINISH_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(tip_info->arc, lv_color_hex(WILL_FINISH_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(tip_info->arc, WILL_FINISH_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(tip_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(tip_info->arc, lv_color_hex(WILL_FINISH_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(tip_info->arc, WILL_FINISH_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(tip_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(tip_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (tip_info->countdown_value > WILL_FINISH_DISPLAY_MAX_VALUE) {
        tip_info->countdown_value = WILL_FINISH_DISPLAY_MAX_VALUE;
    }

    if (tip_info->countdown_value >= WILL_FINISH_DISPLAY_MAX_VALUE) {
        lv_obj_set_style_arc_opa(tip_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(tip_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(tip_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(tip_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(tip_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (tip_info->countdown_value > 0) {
        end_angle = (WILL_FINISH_ARC_FULL_SWEEP * tip_info->countdown_value) / WILL_FINISH_DISPLAY_MAX_VALUE;
    }
    lv_arc_set_angles(tip_info->arc, 0, end_angle);
}

static void will_finish_tip_animation_update(ui_manager_context_t *thiz)
{
    if (tip_info->text == NULL || !lv_obj_is_valid(tip_info->text)) {
        tip_info->text = lv_label_create(tip_info->root);
        EXCEPTION_CHECK_VOID(tip_info->text);
        lv_label_set_text(tip_info->text,"Ending");
        lv_obj_set_style_text_color(tip_info->text, lv_color_hex(WILL_FINISH_TIP_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(tip_info->text, &lv_font_oppo_sans_20_medium, 0);
        lv_obj_align(tip_info->text, LV_ALIGN_CENTER, 0, 0);
    }
}

static void will_finish_tip_mask_layer_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (tip_info->mask_layer == NULL || !lv_obj_is_valid(tip_info->mask_layer)) {
        tip_info->mask_layer = lv_obj_create(tip_info->root);
        EXCEPTION_CHECK_VOID(tip_info->mask_layer);
        lv_obj_set_size(tip_info->mask_layer, LV_PCT(100), LV_PCT(100));
        lv_obj_center(tip_info->mask_layer);
        lv_obj_set_style_bg_color(tip_info->mask_layer, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(tip_info->mask_layer, 190, 0); /// 黑色背景透明度190
        lv_obj_set_style_border_width(tip_info->mask_layer, 0, 0);
        lv_obj_set_style_pad_all(tip_info->mask_layer, 0, 0);
        lv_obj_clear_flag(tip_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_prompt_pump_will_finish_prepare(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (tip_info == NULL) {
        tip_info = (will_finish_tip_display_info_t *)u_malloz(sizeof(will_finish_tip_display_info_t));
        EXCEPTION_CHECK_VOID(tip_info);
    }
    tip_info->force_update = true;
    will_finish_tip_root_init(parent);
    tip_info->countdown_value = 0;
    tip_info->start_time = get_timestamp();
    LOG_UI("pump will finish prepare");
}

void ui_prompt_pump_will_finish_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (tip_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(tip_info->root);
    u_free(tip_info);
    tip_info = NULL;
}

void ui_prompt_pump_will_finish_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (tip_info == NULL || tip_info->root == NULL) {
        return;
    }
    will_finish_tip_mask_layer_update(thiz);
    will_finish_tip_animation_update(thiz);
    will_finish_tip_arc_update(thiz);
    tip_info->force_update = false;
    if (get_timestamp_ms(tip_info->start_time) >= WILL_FINISH_DISPLAY_TIME) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        LOG_UI("pump will finish process completed");
        ani->progress_cb(thiz, PUMP_POPUP_UI_PUMP_WILL_FINISH_TIP, ANI_POPUP_DISPLAY_FINISH);
    } else {
        tip_info->countdown_value = get_timestamp_ms(tip_info->start_time) / WILL_FINISH_DISPLAY_STEP_TIME;
    }
}