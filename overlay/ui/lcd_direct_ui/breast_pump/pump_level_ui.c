/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_LEVEL_COMP_BG                        (0)

#define PUMP_LEVEL_ARC_SEL_COLOR                        0xB3686D
#define PUMP_LEVEL_ARC_UNSEL_COLOR                      0xF7EFEF
#define PUMP_LEVEL_LINE_COLOR                           0xD9D9D9
#define PUMP_LEVEL_TEXT_COLOR                           0x393535
#define PUMP_LEVEL_VALUE_COLOR                          0x1C1C20
#define PUMP_LEVEL_LABLE_UNSEL_COLOR                    0xF7EFEF
#define PUMP_LEVEL_LABLE_SEL_COLOR                      0xB3686D
#define PUMP_LEVEL_TEXT_SEL_COLOR                       0xB3686D
#define PUMP_LEVEL_TEXT_UNSEL_COLOR                     0xFFFFFF

typedef struct {
    lv_obj_t * root;
    lv_obj_t * arc;
    lv_obj_t * arc_progress;
    lv_obj_t * left_level_label;
    lv_obj_t * left_level_label_text;
    lv_obj_t * right_level_label;
    lv_obj_t * right_level_label_text;
    lv_obj_t * root_value;
    lv_obj_t * root_text;
    uint8_t force_update;
    uint8_t level;
    uint8_t control_type;
} level_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static level_display_info_t *level_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_level_root_init(void)
{
    if (level_info->root == NULL || !lv_obj_is_valid(level_info->root)) {
        level_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(level_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(level_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(level_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(level_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(level_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void pump_level_root_text_update(ui_manager_context_t *thiz)
{
    if (level_info->force_update == false) {
        return;
    }

    if (level_info->root_text == NULL || !lv_obj_is_valid(level_info->root_text)) {
        level_info->root_text = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->root_text);
    }
    lv_label_set_text(level_info->root_text, "Level");  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(level_info->root_text, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(level_info->root_text, &lv_font_roboto_24_regular, 0);  /// 设置字体
    lv_obj_align(level_info->root_text, LV_ALIGN_TOP_LEFT, 155, 113); /// 设置文字位置
}

static int pump_level_arc_update(ui_manager_context_t *thiz)
{
    if (level_info->force_update == false) {
        return 0;
    }

    if (level_info->arc == NULL || !lv_obj_is_valid(level_info->arc)) {
        level_info->arc = lv_scale_create(level_info->root);   
        EXCEPTION_CHECK_INT(level_info->arc);
        lv_scale_set_mode(level_info->arc, LV_SCALE_MODE_ROUND_INNER);
    }

    lv_obj_set_size(level_info->arc, 264, 264);                              /// 设置刻度盘控件的宽高（越大，同样 tick 数看起来越"稀疏"）
    lv_obj_center(level_info->arc);                                          /// 让刻度盘在父容器内居中
    lv_obj_set_style_bg_opa(level_info->arc, LV_OPA_TRANSP, 0);              /// 背景填充透明（不画背景色，避免遮挡底图）
    lv_obj_set_style_border_width(level_info->arc, 0, 0);                    /// 边框宽度 0（不画边框）
    lv_obj_set_style_pad_all(level_info->arc, 0, 0);                         /// 内边距 0（tick 计算用内容区域，padding 会影响半径/位置）
    lv_obj_set_style_arc_width(level_info->arc, 0, LV_PART_MAIN);            /// 主弧线宽度 0（隐藏 scale 的主弧/主线）
    lv_obj_set_style_arc_opa(level_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 主弧线透明（双保险：即使有宽度也不显示）
    lv_scale_set_rotation(level_info->arc, 154);                             /// 设置刻度盘起始角度（整体旋转；决定第 0 个 tick 从哪个角度开始画）
    lv_scale_set_angle_range(level_info->arc, 232);                          /// 设置刻度覆盖角度范围（扇区角度；越大 tick 越分散）
    lv_scale_set_range(level_info->arc, 0, 24);                              /// 设置刻度映射的数值范围（tick_idx 会线性映射到 [0,24]）

    lv_scale_set_total_tick_count(level_info->arc, 12);                      /// 总 tick 数（包含两端）；圆形模式下 tick 角度间隔 = angle_range/(total_tick_count-1)
                                                                       /// 觉得"刻度间隔太小/太密"：把 total_tick_count 调小，或把 angle_range 调大，或把控件尺寸调大
    lv_scale_set_major_tick_every(level_info->arc, 0);                       /// 设定"主刻度"频率：0=不使用主刻度；N>0 表示每 N 个 tick 一个主刻度（主刻度样式用 LV_PART_INDICATOR）
    lv_scale_set_label_show(level_info->arc, false);                         /// 是否显示主刻度的文字标签（false=只画刻度线，不画数字）
    lv_obj_set_style_line_width(level_info->arc, 2, LV_PART_ITEMS);          /// 次刻度线宽（像素；作用于 LV_PART_ITEMS）
    lv_obj_set_style_length(level_info->arc, 5, LV_PART_ITEMS);              /// 次刻度线长度（半径方向长度；作用于 LV_PART_ITEMS）
    lv_obj_set_style_line_color(level_info->arc, lv_color_hex(PUMP_LEVEL_LINE_COLOR), LV_PART_ITEMS); /// 次刻度线颜色
    
    return 0;
}

static int pump_level_arc_progress_update(ui_manager_context_t *thiz, uint8_t level)
{    
    if (level_info->arc_progress == NULL || !lv_obj_is_valid(level_info->arc_progress)) {
        level_info->arc_progress = lv_arc_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->arc_progress);
    }
    lv_obj_set_size(level_info->arc_progress, 308, 308);
    lv_obj_center(level_info->arc_progress);
    
    lv_arc_set_bg_angles(level_info->arc_progress, 152, 388);
    uint16_t end_angle = 0;
    if (level > 15) {
        end_angle = 388;
    } else if (level == 0) {
        end_angle = 152;
    } else {
        end_angle = ((388 - 152) * level / 15) + 152;
    }
    lv_arc_set_angles(level_info->arc_progress, 152, end_angle); /// 设置圆弧角度
    lv_obj_set_style_arc_color(level_info->arc_progress, 
        lv_color_hex(PUMP_LEVEL_ARC_UNSEL_COLOR), LV_PART_MAIN); /// 设置背景圆弧颜色
    lv_obj_set_style_arc_width(level_info->arc_progress, 17, LV_PART_MAIN); /// 设置背景圆弧宽度
    lv_obj_set_style_arc_color(level_info->arc_progress, 
        lv_color_hex(PUMP_LEVEL_ARC_SEL_COLOR), LV_PART_INDICATOR); /// 设置深色前景圆弧颜色
    lv_obj_set_style_arc_width(level_info->arc_progress, 17, LV_PART_INDICATOR);
    lv_obj_remove_style(level_info->arc_progress, NULL, LV_PART_KNOB); /// 去掉旋钮

    return 0;
}


static int pump_level_root_value_update(ui_manager_context_t *thiz, uint8_t level)
{
    if (level_info->root_value == NULL || !lv_obj_is_valid(level_info->root_value)) {
        level_info->root_value = lv_label_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->root_value);
    }
    char level_value[4] = {0};
    sprintf(level_value, "%d", level);
    lv_label_set_text(level_info->root_value, level_value);  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(level_info->root_value, lv_color_hex(PUMP_LEVEL_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(level_info->root_value, &lv_font_roboto_79_semibold, 0);  /// 设置字体
    if (level > 9) {
        lv_obj_align(level_info->root_value, LV_ALIGN_TOP_LEFT, 138, 150); /// 设置文字位置
    } else {
        lv_obj_align(level_info->root_value, LV_ALIGN_TOP_LEFT, 138 + 23, 150); /// 设置文字位置
    }

    return 0;
}

static int pump_left_level_lable_update(ui_manager_context_t *thiz, bool is_selected)
{
    if (level_info->left_level_label == NULL || !lv_obj_is_valid(level_info->left_level_label)) {
        level_info->left_level_label = lv_obj_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->left_level_label);
    }

    if (level_info->left_level_label_text == NULL || !lv_obj_is_valid(level_info->left_level_label_text)) {
        level_info->left_level_label_text = lv_label_create(level_info->left_level_label);
        EXCEPTION_CHECK_INT(level_info->left_level_label_text);
    }
    lv_obj_clear_flag(level_info->left_level_label, LV_OBJ_FLAG_SCROLLABLE);   /// 不要滚动条
    lv_obj_set_size(level_info->left_level_label, 98, 55); /// 设置容器尺寸
    lv_obj_set_style_border_width(level_info->left_level_label, 0, 0);         /// 无边框
    lv_obj_set_style_bg_opa(level_info->left_level_label, LV_OPA_COVER, 0);    /// 背景不透明
    lv_obj_set_style_bg_color(level_info->left_level_label, 
        lv_color_hex(is_selected ? PUMP_LEVEL_LABLE_SEL_COLOR : PUMP_LEVEL_LABLE_UNSEL_COLOR), 0);   /// 设置背景颜色
    lv_obj_set_style_radius(level_info->left_level_label, 28, 0);   /// 高度 55 → 圆角设 28（或更大）
    lv_label_set_text(level_info->left_level_label_text, "Left");  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(level_info->left_level_label_text, 
        lv_color_hex(is_selected ? PUMP_LEVEL_TEXT_UNSEL_COLOR : PUMP_LEVEL_TEXT_SEL_COLOR), 0);
    lv_obj_set_style_text_opa(level_info->left_level_label_text, 
        is_selected ? LV_OPA_COVER : LV_OPA_20, LV_PART_MAIN);  /// 文字不透明度=20%（LV_OPA_20=51/255≈20%），视觉效果约为 80% 透明
    lv_obj_set_style_text_font(level_info->left_level_label_text, &lv_font_roboto_27_regular, 0);  /// 设置字体
    lv_obj_center(level_info->left_level_label_text); /// 文字居中
    lv_obj_align(level_info->left_level_label, LV_ALIGN_TOP_LEFT, 75, 254);    /// 设置右档位标签位置

    return 0;
}

static int pump_right_level_lable_update(ui_manager_context_t *thiz, bool is_selected)
{
    if (level_info->right_level_label == NULL || !lv_obj_is_valid(level_info->right_level_label)) {
        level_info->right_level_label = lv_obj_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->right_level_label);
    }
    
    if (level_info->right_level_label_text == NULL || !lv_obj_is_valid(level_info->right_level_label_text)) {
        level_info->right_level_label_text = lv_label_create(level_info->right_level_label);
        EXCEPTION_CHECK_INT(level_info->right_level_label_text);
    }
    lv_obj_clear_flag(level_info->right_level_label, LV_OBJ_FLAG_SCROLLABLE);   /// 不要滚动条
    lv_obj_set_size(level_info->right_level_label, 98, 55); /// 设置容器尺寸
    lv_obj_set_style_border_width(level_info->right_level_label, 0, 0);         /// 无边框
    lv_obj_set_style_bg_opa(level_info->right_level_label, LV_OPA_COVER, 0);    /// 背景不透明
    lv_obj_set_style_bg_color(level_info->right_level_label, 
        lv_color_hex(is_selected ? PUMP_LEVEL_LABLE_SEL_COLOR : PUMP_LEVEL_LABLE_UNSEL_COLOR), 0);   /// 设置背景颜色
    lv_obj_set_style_radius(level_info->right_level_label, 28, 0);   /// 高度 55 → 圆角设 28（或更大）
    lv_label_set_text(level_info->right_level_label_text, "Right");  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(level_info->right_level_label_text, 
        lv_color_hex(is_selected ? PUMP_LEVEL_TEXT_UNSEL_COLOR : PUMP_LEVEL_TEXT_SEL_COLOR), 0);
    lv_obj_set_style_text_opa(level_info->right_level_label_text, 
        is_selected ? LV_OPA_COVER : LV_OPA_20, LV_PART_MAIN);  /// 文字不透明度=20%（LV_OPA_20=51/255≈20%），视觉效果约为 80% 透明
    lv_obj_set_style_text_font(level_info->right_level_label_text, &lv_font_roboto_27_regular, 0);  /// 设置字体
    lv_obj_center(level_info->right_level_label_text); /// 文字居中
    lv_obj_align(level_info->right_level_label, LV_ALIGN_TOP_LEFT, 195, 254);    /// 设置右档位标签位置

    return 0;
}

static void pump_level_ui_update(ui_manager_context_t *thiz, ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->motor);
    uint8_t control_type = proc->pump->get_breast_pump_control_type();
    pump_setting_t setting;
    uint8_t time_index = proc->motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    proc->pump->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    uint8_t level = control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? setting.level[0] : setting.level[1];
    if (level_info->force_update == false &&
        level_info->level == level &&
        level_info->control_type == control_type) {
        return;
    }
    LOG_UI("===> pump level ui update, level: %d, force_update: %d, level_info->level: %d, control_type: %d", 
        level, level_info->force_update, level_info->level, control_type);
    pump_level_arc_update(thiz);
    pump_level_arc_progress_update(thiz, level);
    pump_level_root_value_update(thiz, level);
    pump_left_level_lable_update(thiz, control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? true : false);
    pump_right_level_lable_update(thiz, control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT ? true : false);
    level_info->level = level;
    level_info->control_type = control_type;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_level_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (level_display_info_t *)u_malloz(sizeof(level_display_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_level_root_init();
    level_info->force_update = true;
}

void ui_display_pump_level_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int ui_display_pump_level_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(level_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    pump_level_root_text_update(thiz);
    pump_level_ui_update(thiz, proc);
    thiz->priv.fps_count++;
    level_info->force_update = false;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump left level fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
