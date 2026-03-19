/*
* INCLUDE FILES
*****************************************************************************************
*/
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
#define WILL_FINISH_COUNTDOWN_STEP_TIME                           (75)      // 倒计时步长时间（30ms）
#define WILL_FINISH_COUNTDOWN_MAX_VALUE                           (3000/WILL_FINISH_COUNTDOWN_STEP_TIME)      // 倒计时最大值（3秒）每30ms递减1

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_WILL_FINISH_TIP_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * text;
    lv_obj_t * countdown_arc;            /// 圆弧进度条对象
    lv_obj_t * mask_layer;               /// 遮罩层对象
    uint16_t countdown_value;            /// 倒计时值
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

static will_finish_tip_display_info_t *will_finish_tip_info;


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
    if (will_finish_tip_info->root == NULL || !lv_obj_is_valid(will_finish_tip_info->root)) {
        will_finish_tip_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(will_finish_tip_info->root);
    }
    lv_obj_set_size(will_finish_tip_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(will_finish_tip_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(will_finish_tip_info->root, LV_OPA_TRANSP, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(will_finish_tip_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(will_finish_tip_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(will_finish_tip_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(will_finish_tip_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(will_finish_tip_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void will_finish_tip_arc_update(ui_manager_context_t *thiz)
{
    /* 1. 创建 / 校验圆弧对象 */
    if (will_finish_tip_info->countdown_arc == NULL ||
        !lv_obj_is_valid(will_finish_tip_info->countdown_arc)) {
        lv_obj_t *arc = lv_arc_create(will_finish_tip_info->root);
        EXCEPTION_CHECK_VOID(arc);
        will_finish_tip_info->countdown_arc = arc;

        /* 基础配置：范围、背景角度、尺寸和位置 */
        lv_arc_set_range(arc, 0, WILL_FINISH_COUNTDOWN_MAX_VALUE);      // 0 ~ 100 (3秒，每30ms递减1)
        lv_arc_set_bg_angles(arc, 0, 360);                  // 背景整圆
        lv_obj_set_size(arc, WILL_FINISH_ARC_SIZE, WILL_FINISH_ARC_SIZE);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
        /* 样式：隐藏背景弧和 knob，只显示指示弧 */
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);          // 背景弧透明
        lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);              // knob 透明

        lv_obj_set_style_arc_color(arc, lv_color_hex(WILL_FINISH_ARC_COLOR),
                                   LV_PART_INDICATOR);                       // 指示弧颜色
        lv_obj_set_style_arc_width(arc, WILL_FINISH_ARC_WIDTH,
                                   LV_PART_INDICATOR);                       // 指示弧宽度
    }

    /* 2. 计算进度值：progress = 100 - countdown_value */
    // progress: 0..WILL_FINISH_COUNTDOWN_MAX_VALUE (0..100)
    int16_t progress = (int16_t)WILL_FINISH_COUNTDOWN_MAX_VALUE -
                       (int16_t)will_finish_tip_info->countdown_value;
    if (progress < 0) {
        progress = 0;
    } else if (progress > (int16_t)WILL_FINISH_COUNTDOWN_MAX_VALUE) {
        progress = (int16_t)WILL_FINISH_COUNTDOWN_MAX_VALUE;
    }

    // 0~100 的值同步给 arc
    lv_arc_set_value(will_finish_tip_info->countdown_arc, progress);

    /* 3. 计算圆弧角度：从270°开始，按进度均匀绘制 */
    int16_t start, end;
    if (progress >= WILL_FINISH_COUNTDOWN_MAX_VALUE) {
      // 倒计时=0 → 满圆
      start = 0;
      end = 359; // 用 359° 而不是 360°，避免 wrap
    } else {
      // 按比例从 270° 开始画，顺时针绘制
      start = 270;
      int16_t sweep =
          (int16_t)((360 * progress) / WILL_FINISH_COUNTDOWN_MAX_VALUE); // 0..360
      end = start + sweep;
    }

    lv_arc_set_angles(will_finish_tip_info->countdown_arc, start, end);
}

static void will_finish_tip_animation_update(ui_manager_context_t *thiz)
{
    /// 更新子根内容
    if (will_finish_tip_info->text == NULL || !lv_obj_is_valid(will_finish_tip_info->text)) {
        will_finish_tip_info->text = lv_label_create(will_finish_tip_info->root);
        EXCEPTION_CHECK_VOID(will_finish_tip_info->text);
    }
    lv_label_set_text(will_finish_tip_info->text,"Ending");
    lv_obj_set_style_text_color(will_finish_tip_info->text, lv_color_hex(WILL_FINISH_TIP_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(will_finish_tip_info->text, &lv_font_oppo_sans_20_medium, 0);
    lv_obj_align(will_finish_tip_info->text, LV_ALIGN_CENTER, 0, 0);
    /// 更新圆弧进度条
    will_finish_tip_arc_update(thiz);
    /// 倒计时更新逻辑：每30ms递减1
    if (get_timestamp() - will_finish_tip_info->start_time >= WILL_FINISH_COUNTDOWN_STEP_TIME) {
        if (will_finish_tip_info->countdown_value > 0) {
            will_finish_tip_info->countdown_value--;
        }
        will_finish_tip_info->start_time = get_timestamp();
    }
}

static void will_finish_tip_mask_layer_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (will_finish_tip_info->mask_layer == NULL || !lv_obj_is_valid(will_finish_tip_info->mask_layer)) {
        will_finish_tip_info->mask_layer = lv_obj_create(will_finish_tip_info->root);
        EXCEPTION_CHECK_VOID(will_finish_tip_info->mask_layer);
        lv_obj_set_size(will_finish_tip_info->mask_layer, LV_PCT(100), LV_PCT(100));
        lv_obj_center(will_finish_tip_info->mask_layer);
        lv_obj_set_style_bg_color(will_finish_tip_info->mask_layer, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(will_finish_tip_info->mask_layer, 190, 0); /// 黑色背景透明度190
        lv_obj_set_style_border_width(will_finish_tip_info->mask_layer, 0, 0);
        lv_obj_set_style_pad_all(will_finish_tip_info->mask_layer, 0, 0);
        lv_obj_clear_flag(will_finish_tip_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
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
    if (will_finish_tip_info == NULL) {
        will_finish_tip_info = (will_finish_tip_display_info_t *)u_malloz(sizeof(will_finish_tip_display_info_t));
        EXCEPTION_CHECK_VOID(will_finish_tip_info);
    }
    will_finish_tip_info->force_update = true;
    will_finish_tip_info->countdown_value = WILL_FINISH_COUNTDOWN_MAX_VALUE;
    will_finish_tip_root_init(parent);
}

void ui_prompt_pump_will_finish_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (will_finish_tip_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(will_finish_tip_info->root);
    u_free(will_finish_tip_info);
    will_finish_tip_info = NULL;
}

void ui_prompt_pump_will_finish_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (will_finish_tip_info == NULL || will_finish_tip_info->root == NULL) {
        return;
    }
    will_finish_tip_mask_layer_update(thiz);
    will_finish_tip_animation_update(thiz);
    will_finish_tip_info->force_update = false;
    if (will_finish_tip_info->countdown_value == 0) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        ani->progress_cb(thiz, PUMP_POPUP_UI_PUMP_WILL_FINISH_TIP, ANI_POPUP_DISPLAY_FINISH);
    }
}