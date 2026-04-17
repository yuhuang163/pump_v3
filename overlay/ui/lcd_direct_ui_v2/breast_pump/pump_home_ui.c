/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_COMP_BG                    (0)
#define HOME_MODE_MAX_COUNT                        (4)
#define HOME_BF_MODE_COUNT                         (6)
#define HOME_SPEED_MAX_COUNT                       (3)
#define HOME_TEXT_COUNT                            (3)
#define HOME_ANIMATION_MAX_COUNT                   (RESOURCE_ID_PUMP_MODE_ANIMATION_END - RESOURCE_ID_PUMP_MODE_ANIMATION_START + 1)
#define PUMP_ARC_COLOR                             (0xB3636D)
#define PUMP_LEVEL_CIRCLE_COLOR                    (0xFFFFFF) /// (0xFFEDEF)
#define PUMP_TIME_TEXT_COLOR                       (0xB25962)
#define MODE_TEXT_SEL_COLOR                        (0xFFFFFF)
#define MODE_TEXT_UNSEL_COLOR                      (0xFFE5E7)
#define PUMP_LEVEL_MAX                             (15)
#define PUMP_LEVEL_RING_SIZE                       (306)
#define PUMP_LEVEL_RING_R                          (PUMP_LEVEL_RING_SIZE / 2)
#define PUMP_LEFT_ARC_START_ANGLE                  (136 + 8)
#define PUMP_LEFT_ARC_END_ANGLE                    (224 + 12)
#define PUMP_RIGHT_ARC_START_ANGLE                 (316 - 12)
#define PUMP_RIGHT_ARC_END_ANGLE                   (404 - 8)
#define PUMP_LEVEL_ARC_WIDTH                       (6)
#define PUMP_LEVEL_CIRCLE_SIZE                     (9)
#define PUMP_SPEED_ICON_X                          (186)
#define PUMP_SPEED_ICON_Y                          (24)
#define HOME_CUSTOM_MODE_TEXT_Y                    (91 + 9)
#define HOME_CUSTOM_LEFT_TEXT_CENTER_X             (136 - 4)
#define HOME_CUSTOM_RIGHT_TEXT_CENTER_X            (224 + 2)
#define NEXT_RING_SIZE                             (309)
#define NEXT_RING_R                                (NEXT_RING_SIZE / 2 - 43)
#define NEXT_TEXT_CENTER_Y_OFFSET                  (60)

typedef struct {    
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * mode_text[HOME_TEXT_COUNT];
    lv_obj_t * speed_icon;
    lv_obj_t * time_minute;
    lv_obj_t * time_second;
    lv_obj_t * left_level_arc;
    lv_obj_t * left_level_circle; /// 圆圈
    lv_obj_t * left_progress_text; /// 进度文字
    lv_obj_t * right_level_arc;
    lv_obj_t * right_level_circle; /// 圆圈
    lv_obj_t * right_progress_text; /// 进度文字
    lv_obj_t * animation;
    res_img_info_t speed_icon_info;
    res_img_info_t animation_info;
    res_img_info_t background_info;
    uint8_t force_update;
    uint8_t popup_ui_index; /// 当前弹窗索引.
    uint8_t percent;
    uint8_t is_charging;
    uint8_t mode;
    uint8_t sub_mode;
    uint8_t next_sub_mode;
    uint8_t speed;
    uint8_t left_level;
    uint8_t right_level;
    uint16_t hold_time;
    uint16_t animation_index;
    uint8_t animation_play_complete; /// 动画播放完成标志
    uint8_t rsv[5];
} pump_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_COMP_BG
static PSRAM_DATA_LOC uint8_t home_bg_data[] = {
    #include "comparison_png2txt/pump_home_2_360x360.txt"
};
#endif

const static char *home_mode_names[HOME_MODE_MAX_COUNT] = {
    "Stimulate",
    "Expression",
    "Mixed",
    "Relax",
};

static const char *bf_mode_name[HOME_BF_MODE_COUNT] = {
    "Cozy Flow", /// 自然韵律模式
    "Milk Boost", /// 2+8奶量提升模式
    "Power Pumping", /// PP追奶模式
    "Milk Start", /// 开奶模式
    "Deep Emptying", /// 追加排空模式
    "Customize", /// 已使用过的个性化模式
};

const static uint16_t home_speed_icon_ids[HOME_SPEED_MAX_COUNT] = {
    RESOURCE_ID_PUMP_SPEED_SLOW_ICON,
    RESOURCE_ID_PUMP_SPEED_REGULAR_ICON,
    RESOURCE_ID_PUMP_SPEED_BOOST_ICON,
};

static pump_display_info_t *pump_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_home_init(void)
{
    if (pump_info->root == NULL || !lv_obj_is_valid(pump_info->root)) {
        pump_info->root = lv_obj_create(lv_screen_active());
        lv_obj_set_size(pump_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
        lv_obj_set_style_bg_color(pump_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
        lv_obj_set_style_bg_opa(pump_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
        lv_obj_set_style_border_width(pump_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
        lv_obj_set_style_pad_all(pump_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
        lv_obj_align(pump_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(pump_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
        lv_obj_clear_flag(pump_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
    }
}

#if IS_SUPPORT_HOME_COMP_BG
static void pump_home_bg_update(ui_manager_context_t *thiz, uint8_t is_regular_mode)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (pump_info->background == NULL || !lv_obj_is_valid(pump_info->background)) {
        pump_info->background = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(pump_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(pump_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(pump_info->background, (const void *)&bg_img_path);
        lv_obj_align(pump_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_home_bg_update(ui_manager_context_t *thiz, uint8_t is_regular_mode)
{
    if (pump_info->background == NULL || !lv_obj_is_valid(pump_info->background)) {
        pump_info->background = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->background);
#if LV_USE_SNAPSHOT
        lv_draw_buf_t *background_snapshot = thiz->get_lvgl_snap_shot(is_regular_mode ? \
             UI_SNAP_SHOT_REGULAR_PUMPING_BG : UI_SNAP_SHOT_EXPERT_PUMPING_BG);
        EXCEPTION_CHECK_VOID(background_snapshot);
        lv_image_set_src(pump_info->background, background_snapshot);
#else
        lv_obj_add_flag(pump_info->background, LV_OBJ_FLAG_HIDDEN); /// 隐藏
#endif
        lv_obj_align(pump_info->background, LV_ALIGN_CENTER, 0, 0);
    }
}
#endif

static void pump_home_time_update(ui_manager_context_t *thiz, uint16_t hold_time, uint16_t y_offset)
{
    if (pump_info->hold_time == hold_time && pump_info->force_update == false) {
        return;
    }

    uint8_t minutes = hold_time / 60, seconds = hold_time % 60;
    if (pump_info->time_minute == NULL || !lv_obj_is_valid(pump_info->time_minute)) {
        pump_info->time_minute = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->time_minute);
        lv_obj_set_width(pump_info->time_minute, 120);
        lv_label_set_long_mode(pump_info->time_minute, LV_LABEL_LONG_CLIP); /// 截断过长文本
        lv_obj_set_style_text_align(pump_info->time_minute, LV_TEXT_ALIGN_RIGHT, 0); /// 右对齐
        lv_obj_set_style_text_color(pump_info->time_minute, lv_color_hex(PUMP_TIME_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(pump_info->time_minute, &lv_font_quicksand_96_bold, 0);
        lv_obj_align(pump_info->time_minute, LV_ALIGN_TOP_LEFT, 46, 108 + 15 + y_offset);
    }
    char minute_value[4] = {0};
    snprintf(minute_value, sizeof(minute_value), "%02d", minutes >= 60 ? 59 : minutes);
    lv_label_set_text(pump_info->time_minute, minute_value);

    if (pump_info->time_second == NULL || !lv_obj_is_valid(pump_info->time_second)) {
        pump_info->time_second = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->time_second);
        lv_obj_set_width(pump_info->time_second, 120); 
        lv_label_set_long_mode(pump_info->time_second, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(pump_info->time_second, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_text_color(pump_info->time_second, lv_color_hex(PUMP_TIME_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(pump_info->time_second, &lv_font_quicksand_96_bold, 0);
        lv_obj_align(pump_info->time_second, LV_ALIGN_TOP_LEFT, 194, 108 + 15 + y_offset);
    }
    char second_value[4] = {0};
    snprintf(second_value, sizeof(second_value), "%02d", seconds >= 60 ? 59 : seconds);
    lv_label_set_text(pump_info->time_second, second_value);
    pump_info->hold_time = hold_time;
}

static int pump_home_left_level_text_update(ui_manager_context_t *thiz, uint8_t level, uint16_t end_angle)
{
    char level_text[4] = {0};
    if (pump_info->left_progress_text == NULL || !lv_obj_is_valid(pump_info->left_progress_text)) {
        pump_info->left_progress_text = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->left_progress_text);
        lv_obj_set_style_text_color(pump_info->left_progress_text, lv_color_hex(PUMP_TIME_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(pump_info->left_progress_text, &lv_font_oppo_sans_16_medium, 0);
    }
    snprintf(level_text, sizeof(level_text), "%d", level);
    lv_label_set_text(pump_info->left_progress_text, level_text);
    common_ui_text_place_in_circle(pump_info->left_progress_text, 
        end_angle, PUMP_LEVEL_RING_R + 15, false); /// 设置文字位置
    return 0;
}

static int pump_home_left_level_arc_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (pump_info->left_level_arc == NULL || !lv_obj_is_valid(pump_info->left_level_arc)) {
        pump_info->left_level_arc = lv_arc_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->left_level_arc);
        lv_obj_set_size(pump_info->left_level_arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE); /// 设置弧线大小
        lv_obj_center(pump_info->left_level_arc); /// 设置弧线居中
        lv_arc_set_bg_angles(pump_info->left_level_arc, 
            PUMP_LEFT_ARC_START_ANGLE, PUMP_LEFT_ARC_END_ANGLE);  /// 设置背景弧角度
        lv_arc_set_angles(pump_info->left_level_arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);  /// 设置前景弧角度
        lv_obj_set_style_arc_color(pump_info->left_level_arc,
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(pump_info->left_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(pump_info->left_level_arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧不透明度38%（97/255≈38%）
        lv_obj_set_style_arc_color(pump_info->left_level_arc,
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(pump_info->left_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(pump_info->left_level_arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧完全不透明
        lv_obj_remove_style(pump_info->left_level_arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    } else {
        lv_arc_set_angles(pump_info->left_level_arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);  /// 设置前景弧角度
    }

    return 0;
}

static int pump_home_left_level_circle_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (pump_info->left_level_circle == NULL || !lv_obj_is_valid(pump_info->left_level_circle)) {
        pump_info->left_level_circle = lv_obj_create(pump_info->left_level_arc); // 关键：挂到 arc 下
        lv_obj_remove_style_all(pump_info->left_level_circle);
        lv_obj_set_size(pump_info->left_level_circle, PUMP_LEVEL_CIRCLE_SIZE, PUMP_LEVEL_CIRCLE_SIZE); /// 设置白点大小
        lv_obj_set_style_radius(pump_info->left_level_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(pump_info->left_level_circle, lv_color_hex(PUMP_LEVEL_CIRCLE_COLOR), 0);
        lv_obj_set_style_bg_opa(pump_info->left_level_circle, LV_OPA_COVER, 0);
    }
    common_ui_place_dot_on_arc(pump_info->left_level_arc, pump_info->left_level_circle, end_angle,
                               PUMP_LEVEL_ARC_WIDTH,   /// 弧线宽度
                               PUMP_LEVEL_CIRCLE_SIZE,   /// 白点直径
                               0,   /// 额外内边距: 需要更靠里就填 1~2
                               false);  /// 左侧弧线做镜像方向修正
    return 0;
}

static void pump_home_left_level_update(ui_manager_context_t *thiz, uint8_t level)
{
    if (pump_info->left_level == level && pump_info->force_update == false) {
        return;
    }

    int ret = 0;
    uint16_t end_angle = 0;
    if (level > PUMP_LEVEL_MAX) {
        end_angle = PUMP_LEFT_ARC_END_ANGLE;
        level = PUMP_LEVEL_MAX;
    } else if (level == 0) {
        end_angle = PUMP_LEFT_ARC_START_ANGLE;
    } else {
        end_angle = ((PUMP_LEFT_ARC_END_ANGLE - PUMP_LEFT_ARC_START_ANGLE) * level / PUMP_LEVEL_MAX) +
                PUMP_LEFT_ARC_START_ANGLE;
    }
    
    ret = pump_home_left_level_text_update(thiz, level, end_angle);
    if (ret != 0) {
        return;
    }

    ret = pump_home_left_level_arc_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }

    ret = pump_home_left_level_circle_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }
    pump_info->left_level = level;
}

static int pump_home_right_level_text_update(ui_manager_context_t *thiz, uint8_t level, uint16_t end_angle)
{
    char level_text[4] = {0};
    if (pump_info->right_progress_text == NULL || !lv_obj_is_valid(pump_info->right_progress_text)) {
        pump_info->right_progress_text = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->right_progress_text);
        lv_obj_set_style_text_color(pump_info->right_progress_text, lv_color_hex(PUMP_TIME_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(pump_info->right_progress_text, &lv_font_oppo_sans_16_medium, 0);
    } 
    snprintf(level_text, sizeof(level_text), "%d", level);
    lv_label_set_text(pump_info->right_progress_text, level_text);
    common_ui_text_place_in_circle(pump_info->right_progress_text, 
        end_angle, PUMP_LEVEL_RING_R + 15, false);
    return 0;
}

static int pump_home_right_level_arc_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (pump_info->right_level_arc == NULL || !lv_obj_is_valid(pump_info->right_level_arc)) {
        pump_info->right_level_arc = lv_arc_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->right_level_arc);
        lv_obj_set_size(pump_info->right_level_arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE);
        lv_obj_center(pump_info->right_level_arc);
        lv_arc_set_bg_angles(pump_info->right_level_arc, 
            PUMP_RIGHT_ARC_START_ANGLE, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置背景弧角度
        lv_arc_set_angles(pump_info->right_level_arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置前景弧角度
        lv_obj_set_style_arc_color(pump_info->right_level_arc, 
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(pump_info->right_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN); /// 背景弧宽度
        lv_obj_set_style_arc_opa(pump_info->right_level_arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧不透明度38%（97/255≈38%）
        lv_obj_set_style_arc_color(pump_info->right_level_arc, 
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(pump_info->right_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR); /// 前景弧宽度
        lv_obj_set_style_arc_opa(pump_info->right_level_arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧完全不透明
        lv_obj_remove_style(pump_info->right_level_arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    } else {
        lv_arc_set_angles(pump_info->right_level_arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置前景弧角度
    }
    return 0;
}

static int pump_home_right_level_circle_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (pump_info->right_level_circle == NULL || !lv_obj_is_valid(pump_info->right_level_circle)) {
        pump_info->right_level_circle = lv_obj_create(pump_info->right_level_arc); // 关键：挂到 arc 下
        lv_obj_remove_style_all(pump_info->right_level_circle);
        lv_obj_set_size(pump_info->right_level_circle, PUMP_LEVEL_CIRCLE_SIZE, PUMP_LEVEL_CIRCLE_SIZE); /// 设置白点大小
        lv_obj_set_style_radius(pump_info->right_level_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(pump_info->right_level_circle, lv_color_hex(PUMP_LEVEL_CIRCLE_COLOR), 0);
        lv_obj_set_style_bg_opa(pump_info->right_level_circle, LV_OPA_COVER, 0);
    }
    common_ui_place_dot_on_arc(pump_info->right_level_arc, pump_info->right_level_circle, end_angle,
                               PUMP_LEVEL_ARC_WIDTH,      /// 弧线宽度
                               PUMP_LEVEL_CIRCLE_SIZE,      /// 白点直径
                               -0.5,     /// 额外内边距: 需要更靠里就填 1~2
                               false); /// 右侧跨 360 度时也保持绝对角度
    return 0;
}

static void pump_home_right_level_update(ui_manager_context_t *thiz, uint8_t level)
{
    if (pump_info->right_level == level && pump_info->force_update == false) {
        return;
    }
    int ret = 0;
    uint16_t end_angle = 0;
    if (level > PUMP_LEVEL_MAX) {
        end_angle = PUMP_RIGHT_ARC_START_ANGLE;
        level = PUMP_LEVEL_MAX;
    } else if (level == 0) {
        end_angle = PUMP_RIGHT_ARC_END_ANGLE;
    } else {
        end_angle = ((PUMP_RIGHT_ARC_END_ANGLE - PUMP_RIGHT_ARC_START_ANGLE) *
            (PUMP_LEVEL_MAX - level) / PUMP_LEVEL_MAX) + PUMP_RIGHT_ARC_START_ANGLE;
    }

    ret = pump_home_right_level_text_update(thiz, level, end_angle);
    if (ret != 0) {
        return;
    }

    ret = pump_home_right_level_arc_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }

    ret = pump_home_right_level_circle_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }
    pump_info->right_level = level;
}

static void pump_home_speed_label_update(ui_manager_context_t *thiz, uint8_t speed)
{
    if (speed >= HOME_SPEED_MAX_COUNT) {
        return;
    }

    if (pump_info->speed == speed && pump_info->force_update == false) {
        return;
    } 
    
    lv_fs_path_ex_t speed_icon_path;
    if (pump_info->speed_icon == NULL || !lv_obj_is_valid(pump_info->speed_icon)) {
        pump_info->speed_icon = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->speed_icon);
    } 
    common_ui_img_resource_init(thiz, home_speed_icon_ids[speed], &pump_info->speed_icon_info);
    EXCEPTION_CHECK_VOID(pump_info->speed_icon_info.data);
    lv_fs_make_path_from_buffer(&speed_icon_path, LV_FS_MEMFS_LETTER, 
        pump_info->speed_icon_info.data, pump_info->speed_icon_info.size, "baj");
    lv_image_set_src(pump_info->speed_icon, (const void *)&speed_icon_path);
    lv_obj_align(pump_info->speed_icon, LV_ALIGN_TOP_LEFT, PUMP_SPEED_ICON_X, PUMP_SPEED_ICON_Y);
    pump_info->speed = speed; 
}

static void pump_home_common_mode_update(ui_manager_context_t *thiz, uint8_t mode_index)
{
    if (pump_info->mode == mode_index && pump_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < HOME_TEXT_COUNT; i++) {
        if (pump_info->mode_text[i] == NULL || !lv_obj_is_valid(pump_info->mode_text[i])) {
            pump_info->mode_text[i] = lv_label_create(pump_info->root);
            EXCEPTION_CHECK_VOID(pump_info->mode_text[i]);
            lv_obj_set_style_text_color(pump_info->mode_text[i], lv_color_hex(MODE_TEXT_SEL_COLOR), 0);
            lv_obj_set_style_text_font(pump_info->mode_text[i], &lv_font_oppo_sans_12_medium, 0);
        } 

        if (i != 0) {
            lv_obj_add_flag(pump_info->mode_text[i], LV_OBJ_FLAG_HIDDEN); /// 隐藏
            return;
        }
        lv_obj_clear_flag(pump_info->mode_text[i], LV_OBJ_FLAG_HIDDEN); /// 显示
        lv_label_set_text(pump_info->mode_text[i], home_mode_names[mode_index]);  /// 设置 label 文字内容和样式
        lv_obj_align(pump_info->mode_text[i], LV_ALIGN_TOP_MID, 0, 81); /// 设置label文字位置
        pump_info->mode = mode_index;
    }
}

static void pump_home_custom_mode_update(ui_manager_context_t *thiz, uint8_t mode_index, 
    uint8_t sub_mode, uint8_t next_sub_mode)
{    
    uint8_t mode_name_index = 0;
    if (pump_info->mode == mode_index && pump_info->sub_mode == sub_mode &&
        pump_info->next_sub_mode == next_sub_mode && pump_info->force_update == false) {
        return;
    }
    uint8_t index = mode_index - BREAST_PUMP_REGULAR_PUMPING_MODE;
    if (index > HOME_BF_MODE_COUNT) {
        index = 0;
    }
    for (uint8_t i = 0; i < HOME_TEXT_COUNT; i++) {
        if (pump_info->mode_text[i] == NULL || !lv_obj_is_valid(pump_info->mode_text[i])) {
            pump_info->mode_text[i] = lv_label_create(pump_info->root);
            EXCEPTION_CHECK_VOID(pump_info->mode_text[i]);
        } 
        lv_obj_clear_flag(pump_info->mode_text[i], LV_OBJ_FLAG_HIDDEN); /// 显示
        if (i == 0) {
            if (sub_mode >= HOME_MODE_MAX_COUNT) {
                sub_mode = HOME_MODE_MAX_COUNT - 1;
            }
            lv_obj_set_style_text_font(pump_info->mode_text[i], &lv_font_oppo_sans_12_medium, 0);
            lv_label_set_text(pump_info->mode_text[i], home_mode_names[sub_mode]);  /// 设置 label 文字内容和样式
            lv_obj_set_style_text_color(pump_info->mode_text[i], lv_color_hex(MODE_TEXT_SEL_COLOR), 0);
            lv_obj_update_layout(pump_info->mode_text[i]);
            lv_coord_t text_width = lv_obj_get_width(pump_info->mode_text[i]);
            lv_obj_align(pump_info->mode_text[i], LV_ALIGN_TOP_LEFT,
                HOME_CUSTOM_LEFT_TEXT_CENTER_X - text_width / 2, HOME_CUSTOM_MODE_TEXT_Y); /// 根据文字宽度动态居中
        } else if (i == 1) {
            if (next_sub_mode >= HOME_MODE_MAX_COUNT) {
                next_sub_mode = HOME_MODE_MAX_COUNT - 1;
            }
            lv_obj_set_style_text_font(pump_info->mode_text[i], &lv_font_oppo_sans_12_medium, 0);
            lv_label_set_text(pump_info->mode_text[i], home_mode_names[next_sub_mode]);  /// 设置 label 文字内容和样式
            lv_obj_set_style_text_color(pump_info->mode_text[i], lv_color_hex(MODE_TEXT_UNSEL_COLOR), 0);
            lv_obj_update_layout(pump_info->mode_text[i]);
            lv_coord_t text_width = lv_obj_get_width(pump_info->mode_text[i]);
            lv_obj_align(pump_info->mode_text[i], LV_ALIGN_TOP_LEFT,
                HOME_CUSTOM_RIGHT_TEXT_CENTER_X - text_width / 2, HOME_CUSTOM_MODE_TEXT_Y); /// 根据文字宽度动态居中
        } else {
            lv_obj_set_style_text_font(pump_info->mode_text[i], &lv_font_oppo_sans_12_medium, 0);
            lv_label_set_text(pump_info->mode_text[i], bf_mode_name[index]);  /// 设置 label 文字内容和样式
            lv_obj_set_style_text_color(pump_info->mode_text[i], lv_color_hex(0xB25962), 0);
            lv_obj_update_layout(pump_info->mode_text[i]);
            lv_obj_align(pump_info->mode_text[i], LV_ALIGN_TOP_MID, 0, 66); /// 根据文字宽度动态居中
        }
    }
    LOG_UI("%s mode, mode_text: %s, next_mode_text: %s", bf_mode_name[index], home_mode_names[sub_mode], home_mode_names[next_sub_mode]);
    pump_info->mode = mode_index;
    pump_info->sub_mode = sub_mode;
    pump_info->next_sub_mode = next_sub_mode;
}

static void pump_home_animation_update(ui_manager_context_t *thiz, uint16_t start_resource_id, uint16_t animation_size,
    uint16_t x_offset, uint16_t y_offset)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(pump_info);
    if (pump_info->animation_play_complete) {
        return;
    }

    if (pump_info->animation == NULL || !lv_obj_is_valid(pump_info->animation)) {
        pump_info->animation = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->animation);
    }  
    
    if (pump_info->animation_index >= animation_size) {
        pump_info->animation_index = 0;
        pump_info->animation_play_complete = true;
    }  
    
    lv_obj_align(pump_info->animation, LV_ALIGN_TOP_LEFT, x_offset, y_offset); 
    if (pump_info->animation_info.data != NULL) {
        common_ui_img_resource_deinit(thiz, &pump_info->animation_info);
        memset(&pump_info->animation_info, 0, sizeof(pump_info->animation_info));
    }
    common_ui_img_resource_init_with_theme(thiz, 
        start_resource_id + pump_info->animation_index, &pump_info->animation_info);
    EXCEPTION_CHECK_VOID(pump_info->animation_info.data);
    lv_fs_path_ex_t animation_img_path;
    lv_fs_make_path_from_buffer(&animation_img_path, LV_FS_MEMFS_LETTER, 
        pump_info->animation_info.data, pump_info->animation_info.size, "baj");
    lv_image_set_src(pump_info->animation, (const void *)&animation_img_path);
    pump_info->animation_index++;
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_info == NULL) {
        pump_info = (pump_display_info_t *)u_malloz(sizeof(pump_display_info_t));
        EXCEPTION_CHECK_VOID(pump_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_home_init();  
    ble_ui_init(thiz);
    pump_info->animation_index = 0;
    pump_info->animation_play_complete = false;
    pump_info->force_update = true;
    pump_info->popup_ui_index = ID_POPUP_UI_NONE;
}

void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(pump_info);
    common_ui_img_resource_deinit(thiz, &pump_info->speed_icon_info);
    common_ui_img_resource_deinit(thiz, &pump_info->animation_info);
    common_ui_img_resource_deinit(thiz, &pump_info->background_info);
    ble_ui_deinit(thiz);
    ui_common_operation_tip_remove(thiz);
    LVGL_OBJ_DELETE(pump_info->root);
    u_free(pump_info);
    pump_info = NULL;
}

int ui_display_pump_home_screen_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(pump_info && pump_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump && proc->motor && proc->ble);
    const pump_result_info_t *result_info = NULL;
    READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    EXCEPTION_CHECK_INT(result_info);
    uint16_t hold_time = result_info->work_total_time / 1000;
    pump_setting_t setting;
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, setting);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    uint8_t is_regular_mode = PUMP_IS_COMMON_MODE(pump_mode);
    pump_home_bg_update(thiz, is_regular_mode);
    ble_ui_update(thiz, pump_info->root, proc->ble, PUMP_HOME_BLE_UI_X, PUMP_HOME_BLE_UI_Y);
    pump_home_time_update(thiz, hold_time, is_regular_mode ? 0 : 19);
    if (is_regular_mode) {
        pump_home_animation_update(thiz, RESOURCE_ID_NM_ANIM_IP_START, 
            RESOURCE_ID_NM_ANIM_IP_END - RESOURCE_ID_NM_ANIM_IP_START + 1,
            85, 168);
        pump_home_common_mode_update(thiz, pump_mode);  
    } else {
        uint8_t sub_mode = 0;
        uint8_t next_sub_mode = 0;
        if (proc->motor->get_runtime_mode_type) {
            sub_mode = proc->motor->get_runtime_mode_type(false);
            next_sub_mode = proc->motor->get_runtime_mode_type(true);
        }
        pump_home_animation_update(thiz, RESOURCE_ID_PM_ANIM_IP_START, 
            RESOURCE_ID_PM_ANIM_IP_END - RESOURCE_ID_PM_ANIM_IP_START + 1,
            0, 108);
        pump_home_custom_mode_update(thiz, pump_mode, sub_mode, next_sub_mode);
    }
    pump_home_left_level_update(thiz, setting.level[0]);
    pump_home_right_level_update(thiz, setting.level[1]);
    pump_home_speed_label_update(thiz, setting.speed);
    ui_common_operation_tip_update(thiz, proc->popup_ui_type, pump_info->root,
        &pump_info->popup_ui_index);
    pump_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump home fps: %d, mode:%d/%d", thiz->priv.fps_count, pump_mode, proc->popup_ui_type);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
