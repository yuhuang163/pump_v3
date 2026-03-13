/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "best1306p.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_home.h"   // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_WILL_POWER_OFF_TIP_COMP_BG                        (0)
#define WILL_POWER_OFF_TIP_TEXT_COLOR                                0xFFFFFF
#define WILL_POWER_OFF_ARC_SIZE                                      (360)    // 圆弧尺寸
#define WILL_POWER_OFF_ARC_WIDTH                                     (12)      // 圆弧粗细
#define WILL_POWER_OFF_ARC_COLOR                                     (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define WILL_POWER_OFF_COUNTDOWN_STEP_TIME                           (75)      // 倒计时步长时间（75ms）
#define WILL_POWER_OFF_COUNTDOWN_MAX_VALUE                           (3000/WILL_POWER_OFF_COUNTDOWN_STEP_TIME)      // 倒计时最大值（3秒）每75ms递减1
#define WILL_POWER_OFF_CALLBACK_DELAY_TIME                           (100)     // 倒计时到0后延迟调用回调的时间（100ms）
#define WILL_POWER_OFF_COUNTDOWN_SECOND_2_THRESHOLD                  ((WILL_POWER_OFF_COUNTDOWN_MAX_VALUE * 2) / 3)  // 显示"2"的阈值
#define WILL_POWER_OFF_COUNTDOWN_SECOND_1_THRESHOLD                 (WILL_POWER_OFF_COUNTDOWN_MAX_VALUE / 3)        // 显示"1"的阈值

#define WILL_POWER_OFF_TIP_TITLE_TEXT_COLOR                           0xC74B57 
#define WILL_POWER_OFF_TIP_NUMBER_TEXT_COLOR                          0xFFFFFF

#define WILL_POWER_OFF_TIP_TITLE_TEXT_X                              (126)
#define WILL_POWER_OFF_TIP_TITLE_TEXT_Y                              (108)
#define WILL_POWER_OFF_TIP_NUMBER_TEXT_X                             (154)
#define WILL_POWER_OFF_TIP_NUMBER_TEXT_Y                             (144)


typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_WILL_POWER_OFF_TIP_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * title_text;
    lv_obj_t * number_text;
    lv_obj_t * countdown_arc;            /// 圆弧进度条对象
    uint16_t countdown_value;            /// 倒计时值
    uint32_t start_time;                 /// 倒计时起始时间
    uint32_t countdown_zero_time;        /// 倒计时到0的时间戳
    uint8_t force_update;
    uint8_t current_display_number;      /// 当前显示的数字，避免重复设置
    uint8_t callback_triggered;          /// 回调是否已触发
} will_power_off_tip_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_WILL_POWER_OFF_TIP_COMP_BG
static PSRAM_DATA_LOC uint8_t will_power_off_tip_bg_data[] = {
    #include "comparison_png2txt/pump_finish_comp_360x360.txt"
};
#endif

static will_power_off_tip_display_info_t *will_power_off_tip_info;


/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void will_power_off_tip_sub_root_init(lv_obj_t *parent)
{
    if (will_power_off_tip_info->root == NULL || !lv_obj_is_valid(will_power_off_tip_info->root)) {
        will_power_off_tip_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(will_power_off_tip_info->root);
    }
    lv_obj_set_size(will_power_off_tip_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(will_power_off_tip_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(will_power_off_tip_info->root, LV_OPA_50, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(will_power_off_tip_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(will_power_off_tip_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(will_power_off_tip_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(will_power_off_tip_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(will_power_off_tip_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void will_power_off_tip_arc_update(ui_manager_context_t *thiz)
{
    /* 1. 创建 / 校验圆弧对象 */
    if (will_power_off_tip_info->countdown_arc == NULL ||
        !lv_obj_is_valid(will_power_off_tip_info->countdown_arc)) {
        lv_obj_t *arc = lv_arc_create(will_power_off_tip_info->root);
        EXCEPTION_CHECK_VOID(arc);
        will_power_off_tip_info->countdown_arc = arc;

        /* 基础配置：范围、背景角度、尺寸和位置 */
        lv_arc_set_range(arc, 0, WILL_POWER_OFF_COUNTDOWN_MAX_VALUE);      // 0 ~ 40 (3秒，每75ms递减1)
        lv_arc_set_bg_angles(arc, 0, 360);                  // 背景整圆
        lv_obj_set_size(arc, WILL_POWER_OFF_ARC_SIZE, WILL_POWER_OFF_ARC_SIZE);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
        /* 样式：隐藏背景弧和 knob，只显示指示弧 */
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);          // 背景弧透明
        lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);              // knob 透明

        lv_obj_set_style_arc_color(arc, lv_color_hex(WILL_POWER_OFF_ARC_COLOR),
                                   LV_PART_INDICATOR);                       // 指示弧颜色
        lv_obj_set_style_arc_width(arc, WILL_POWER_OFF_ARC_WIDTH,
                                   LV_PART_INDICATOR);                       // 指示弧宽度
    }

    /* 2. 计算进度值：progress = max - countdown_value */
    // progress: 0..WILL_POWER_OFF_COUNTDOWN_MAX_VALUE (0..40)
    int16_t progress = (int16_t)WILL_POWER_OFF_COUNTDOWN_MAX_VALUE -
                       (int16_t)will_power_off_tip_info->countdown_value;
    if (progress < 0) {
        progress = 0;
    } else if (progress > (int16_t)WILL_POWER_OFF_COUNTDOWN_MAX_VALUE) {
        progress = (int16_t)WILL_POWER_OFF_COUNTDOWN_MAX_VALUE;
    }

    // 0~max 的值同步给 arc
    lv_arc_set_value(will_power_off_tip_info->countdown_arc, progress);

    /* 3. 计算圆弧角度：从270°开始，按进度均匀绘制 */
    int16_t start, end;
    // 当倒计时为0或进度达到最大值时，画完整圆
    if (will_power_off_tip_info->countdown_value == 0 || progress >= WILL_POWER_OFF_COUNTDOWN_MAX_VALUE) {
      // 倒计时=0 → 满圆，从270°开始画完整360度
      // 使用 start = 270, end = 270 + 359 = 629，LVGL会自动处理为 end = 269
      // 这样从270°顺时针到269°（经过360°），正好是完整圆
      start = 270;
      end = 270 + 359;
    } else {
      // 按比例从 270° 开始画，顺时针绘制
      start = 270;
      int16_t sweep =
          (int16_t)((360 * progress) / WILL_POWER_OFF_COUNTDOWN_MAX_VALUE); // 0..360
      end = start + sweep;
      // 确保end角度在有效范围内
      if (end >= 360) {
          end = end - 360;
      }
    }

    lv_arc_set_angles(will_power_off_tip_info->countdown_arc, start, end);
}

static void will_power_off_tip_animation_update(ui_manager_context_t *thiz)
{
    /// 更新子根内容
    if (will_power_off_tip_info->title_text == NULL || !lv_obj_is_valid(will_power_off_tip_info->title_text)) {
        will_power_off_tip_info->title_text = lv_label_create(will_power_off_tip_info->root);
        EXCEPTION_CHECK_VOID(will_power_off_tip_info->title_text);
        lv_label_set_text(will_power_off_tip_info->title_text,"Powering off in");
        lv_obj_set_style_text_color(will_power_off_tip_info->title_text, lv_color_hex(WILL_POWER_OFF_TIP_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(will_power_off_tip_info->title_text, &lv_font_oppo_sans_16_medium, 0);
        lv_obj_align(will_power_off_tip_info->title_text, LV_ALIGN_TOP_LEFT, WILL_POWER_OFF_TIP_TITLE_TEXT_X, WILL_POWER_OFF_TIP_TITLE_TEXT_Y);
    }

    if (will_power_off_tip_info->number_text == NULL || !lv_obj_is_valid(will_power_off_tip_info->number_text)) {
        will_power_off_tip_info->number_text = lv_label_create(will_power_off_tip_info->root);
        EXCEPTION_CHECK_VOID(will_power_off_tip_info->number_text);
        lv_label_set_text(will_power_off_tip_info->number_text,"3");
        lv_obj_set_style_text_color(will_power_off_tip_info->number_text, lv_color_hex(WILL_POWER_OFF_TIP_NUMBER_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(will_power_off_tip_info->number_text, &lv_font_oppo_sans_96_bold, 0);
        lv_obj_align(will_power_off_tip_info->number_text, LV_ALIGN_TOP_LEFT, WILL_POWER_OFF_TIP_NUMBER_TEXT_X, WILL_POWER_OFF_TIP_NUMBER_TEXT_Y);
        will_power_off_tip_info->current_display_number = 3;  /// 初始化显示数字
    }

    /// 更新圆弧进度条
    will_power_off_tip_arc_update(thiz);
    /// 倒计时更新逻辑：每75ms递减1
    if (get_timestamp() - will_power_off_tip_info->start_time >= WILL_POWER_OFF_COUNTDOWN_STEP_TIME) {
        if (will_power_off_tip_info->countdown_value > 0) {
            will_power_off_tip_info->countdown_value--;
        }
        will_power_off_tip_info->start_time = get_timestamp();
    }
    /// 更新倒计时数字（只在数字改变时更新，避免闪烁）
    uint8_t target_number = 3;
    if (will_power_off_tip_info->countdown_value == 0) {
        target_number = 0;
    } else if (will_power_off_tip_info->countdown_value <= WILL_POWER_OFF_COUNTDOWN_SECOND_1_THRESHOLD) {
        target_number = 1;
    } else if (will_power_off_tip_info->countdown_value <= WILL_POWER_OFF_COUNTDOWN_SECOND_2_THRESHOLD) {
        target_number = 2;
    }
    
    /// 只在数字改变时才更新文本，避免重复设置导致闪烁
    if (will_power_off_tip_info->current_display_number != target_number) {
        char number_str[2] = {0};
        number_str[0] = '0' + target_number;
        lv_label_set_text(will_power_off_tip_info->number_text, number_str);
        will_power_off_tip_info->current_display_number = target_number;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_prompt_home_will_power_off_prepare(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (will_power_off_tip_info == NULL) {
        will_power_off_tip_info = (will_power_off_tip_display_info_t *)u_malloz(sizeof(will_power_off_tip_display_info_t));
        EXCEPTION_CHECK_VOID(will_power_off_tip_info);
    }
    will_power_off_tip_info->force_update = true;
    will_power_off_tip_info->countdown_value = WILL_POWER_OFF_COUNTDOWN_MAX_VALUE;
    will_power_off_tip_info->start_time = get_timestamp();
    will_power_off_tip_info->countdown_zero_time = 0;  /// 初始化倒计时到0的时间戳
    will_power_off_tip_info->current_display_number = 3;  /// 初始化显示数字
    will_power_off_tip_info->callback_triggered = 0;  /// 初始化回调触发标志
    will_power_off_tip_sub_root_init(parent);
}

void ui_prompt_home_will_power_off_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (will_power_off_tip_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(will_power_off_tip_info->root);
    u_free(will_power_off_tip_info);
    will_power_off_tip_info = NULL;
}

void ui_prompt_home_will_power_off_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (will_power_off_tip_info == NULL || will_power_off_tip_info->root == NULL) {
        return;
    }
    will_power_off_tip_animation_update(thiz);
    will_power_off_tip_info->force_update = false;
    
    /// 当倒计时到0时，记录时间戳
    if (will_power_off_tip_info->countdown_value == 0 && 
        will_power_off_tip_info->countdown_zero_time == 0) {
        will_power_off_tip_info->countdown_zero_time = get_timestamp();
    }
    
    /// 延迟调用回调，确保用户能看到完整的闭合圆和"0"的显示
    if (will_power_off_tip_info->countdown_value == 0 && 
        !will_power_off_tip_info->callback_triggered &&
        will_power_off_tip_info->countdown_zero_time > 0 &&
        (get_timestamp() - will_power_off_tip_info->countdown_zero_time >= WILL_POWER_OFF_CALLBACK_DELAY_TIME)) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        will_power_off_tip_info->callback_triggered = 1;
        ani->progress_cb(thiz, HOME_UI_TYPE_HOME_WILL_POWER_OFF, ANI_POPUP_DISPLAY_FINISH);
    }
}