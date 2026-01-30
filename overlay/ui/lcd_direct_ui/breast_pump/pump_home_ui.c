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
#define IS_SUPPORT_HOME_COMP_BG                    (0)
#define IS_SUPPORT_HOME_DEBUG_OTHER_PAGE           (0)
#define IS_SUPPORT_HOME_PAGE_TEST                  (0)
#define IS_SUPPORT_AIR_LEAKAGE_TEST                (0)
#define HOME_MODE_MAX_COUNT                        (7)
#define HOME_SPEED_MAX_COUNT                       (3)
#define AIR_LEAKAGE_TEXT_COUNT                     (2)

#define PUMP_IMG_SEL_COLOR                         0xB3686D
#define PUMP_IMG_UNSEL_COLOR                       0xF7EFEF
#define PUMP_TEXT_SEL_COLOR                        0xB3686D
#define PUMP_TEXT_UNSEL_COLOR                      0xFFFFFF
#define PUMP_TEXT_BLACK_COLOR                      0x000000
#define PUMP_ARC_SEL_COLOR                         0xB3686D
#define PUMP_ARC_UNSEL_COLOR                       0xF7EFEF
#define PUMP_MODE_SEL_START_COLOR                  0xB3686D
#define PUMP_MODE_SEL_END_COLOR                    0xF1D9D6
#define AIR_LEAKAGE_BG_COLOR                       0x000000

#define STIMULATE_MODE_ICON_X                      (122 + 4)
#define STIMULATE_MODE_ICON_Y                      (283 + 15)
#define STIMULATE_MODE_LABEL_TEXT_X                (149 + 3)
#define STIMULATE_MODE_LABEL_TEXT_Y                (294 + 4)

#define LACTATION_MODE_ICON_X                      (STIMULATE_MODE_ICON_X + 2)
#define LACTATION_MODE_ICON_Y                      (STIMULATE_MODE_ICON_Y)
#define LACTATION_MODE_LABEL_TEXT_X                (STIMULATE_MODE_LABEL_TEXT_X + 2)
#define LACTATION_MODE_LABEL_TEXT_Y                (STIMULATE_MODE_LABEL_TEXT_Y)

#define MIX_MODE_ICON_X                            (STIMULATE_MODE_ICON_X+ 28)
#define MIX_MODE_ICON_Y                            (STIMULATE_MODE_ICON_Y)
#define MIX_MODE_LABEL_TEXT_X                      (STIMULATE_MODE_LABEL_TEXT_X + 28)
#define MIX_MODE_LABEL_TEXT_Y                      (STIMULATE_MODE_LABEL_TEXT_Y)

#define INITIAL_MODE_ICON_X                        (STIMULATE_MODE_ICON_X - 7)
#define INITIAL_MODE_ICON_Y                        (STIMULATE_MODE_ICON_Y)
#define INITIAL_MODE_LABEL_TEXT_X                  (STIMULATE_MODE_LABEL_TEXT_X - 11)
#define INITIAL_MODE_LABEL_TEXT_Y                  (STIMULATE_MODE_LABEL_TEXT_Y)

#define MILK_MODE_ICON_X                           (STIMULATE_MODE_ICON_X - 1)
#define MILK_MODE_ICON_Y                           (STIMULATE_MODE_ICON_Y)
#define MILK_MODE_LABEL_TEXT_X                     (STIMULATE_MODE_LABEL_TEXT_X - 5)
#define MILK_MODE_LABEL_TEXT_Y                     (STIMULATE_MODE_LABEL_TEXT_Y)

#define PP_MODE_ICON_X                             (STIMULATE_MODE_ICON_X + 37)
#define PP_MODE_ICON_Y                             (STIMULATE_MODE_ICON_Y)
#define PP_MODE_LABEL_TEXT_X                       (STIMULATE_MODE_LABEL_TEXT_X + 28)
#define PP_MODE_LABEL_TEXT_Y                       (STIMULATE_MODE_LABEL_TEXT_Y)

#define PER_MODE_ICON_X                            (STIMULATE_MODE_ICON_X - 9)
#define PER_MODE_ICON_Y                            (STIMULATE_MODE_ICON_Y)
#define PER_MODE_LABEL_TEXT_X                      (STIMULATE_MODE_LABEL_TEXT_X - 7)
#define PER_MODE_LABEL_TEXT_Y                      (STIMULATE_MODE_LABEL_TEXT_Y)

typedef struct {
    const char *name;
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
    uint16_t label_text_x;
    uint16_t label_text_y;
} home_mode_label_info_t;

typedef struct {
    const char *name;
    uint16_t text_y;
} air_leakage_info_t;

typedef struct {    
    lv_obj_t * root;
    lv_obj_t * backgroud;
    lv_obj_t * time_colon;
    lv_obj_t * time_minute;
    lv_obj_t * time_second;
    lv_obj_t * speed_label;
    lv_obj_t * speed_label_text;
    lv_obj_t * left_level_arc;
    lv_obj_t * left_level_label;
    lv_obj_t * left_level_text;
    lv_obj_t * right_level_arc;
    lv_obj_t * right_level_label;
    lv_obj_t * right_level_text;
    lv_obj_t * mode_label;
    lv_obj_t * mode_icon;
    lv_obj_t * mode_text;
    res_img_info_t mode_icon_info;
    res_img_info_t background_info;
#if IS_SUPPORT_AIR_LEAKAGE_WARNING
    lv_obj_t * air_leakage_label;
    lv_obj_t * air_leakage_text[AIR_LEAKAGE_TEXT_COUNT];
    lv_obj_t * air_leakage_icon;
    res_img_info_t air_leakage_icon_info;
#endif
    uint8_t force_update;
    uint8_t mode;
    uint8_t speed;
    uint8_t left_level;
    uint8_t right_level;
    uint16_t hold_time;
    uint16_t animation_index;
#if IS_SUPPORT_AIR_LEAKAGE_WARNING
    uint8_t hide_air_leakage_popup_wm;
    algo_breast_pump_data_t algo_data;
#endif
} pump_home_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_COMP_BG
static PSRAM_DATA_LOC uint8_t home_bg_1_data[] = {
    #include "comparison_png2txt/home_comp_air_360x360.txt"
    // #include "comparison_png2txt/home_comp_360x360_1.txt"
};
#endif

const static home_mode_label_info_t home_mode_infos[HOME_MODE_MAX_COUNT] = {
    { "Stimulate", RESOURCE_ID_HOME_STIMULATE_ICON, 
      STIMULATE_MODE_ICON_X,       STIMULATE_MODE_ICON_Y, 
      STIMULATE_MODE_LABEL_TEXT_X, STIMULATE_MODE_LABEL_TEXT_Y
    },
    { "Lactation", RESOURCE_ID_HOME_LACTATION_ICON, 
      LACTATION_MODE_ICON_X,       LACTATION_MODE_ICON_Y,
      LACTATION_MODE_LABEL_TEXT_X, LACTATION_MODE_LABEL_TEXT_Y 
    },
    { "Mix",  RESOURCE_ID_HOME_MIX_ICON,     
      MIX_MODE_ICON_X,             MIX_MODE_ICON_Y, 
      MIX_MODE_LABEL_TEXT_X,       MIX_MODE_LABEL_TEXT_Y
    },
    { "Personalization", RESOURCE_ID_HOME_PERSONALIZATION_ICON,     
      PER_MODE_ICON_X,             PER_MODE_ICON_Y, 
      PER_MODE_LABEL_TEXT_X,       PER_MODE_LABEL_TEXT_Y
    },
    { "PP", RESOURCE_ID_HOME_PP_ICON,     
      PP_MODE_ICON_X,              PP_MODE_ICON_Y, 
      PP_MODE_LABEL_TEXT_X,        PP_MODE_LABEL_TEXT_Y
    },
    { "Milk supply...", RESOURCE_ID_HOME_MILK_ICON,     
      MILK_MODE_ICON_X,            MILK_MODE_ICON_Y, 
      MILK_MODE_LABEL_TEXT_X,      MILK_MODE_LABEL_TEXT_Y
    },
    { "Initiating lact...", RESOURCE_ID_HOME_INITIA_ICON,     
      INITIAL_MODE_ICON_X,         INITIAL_MODE_ICON_Y, 
      INITIAL_MODE_LABEL_TEXT_X,   INITIAL_MODE_LABEL_TEXT_Y
    },
};

const static air_leakage_info_t air_leakage_infos[AIR_LEAKAGE_TEXT_COUNT] = {
    { "Suction may not be optimal", 2},
    { "Please check the fit, position, and tubing\nconnection.", 39},
};

static pump_home_display_info_t *pump_info;

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
        lv_obj_set_style_bg_color(pump_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
        lv_obj_set_style_bg_opa(pump_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
        lv_obj_set_style_border_width(pump_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
        lv_obj_set_style_pad_all(pump_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
        lv_obj_align(pump_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(pump_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
        lv_obj_clear_flag(pump_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
    }
}

static void pump_home_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (pump_info->backgroud == NULL || !lv_obj_is_valid(pump_info->backgroud)) {
        pump_info->backgroud = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(pump_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(pump_info->backgroud, LV_OBJ_FLAG_HIDDEN);
#if IS_SUPPORT_HOME_COMP_BG
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_1_data,
            sizeof(home_bg_1_data), "png");
#else
        common_ui_img_resource_init(thiz, RESOURCE_ID_PUMP_HOME_BG, &pump_info->background_info);
        EXCEPTION_CHECK_VOID(pump_info->background_info.data);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, 
            pump_info->background_info.data, pump_info->background_info.size, "baj");
#endif
        lv_image_set_src(pump_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(pump_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}

static void pump_home_time_update(ui_manager_context_t *thiz, breast_pump_manager_context_t *pump)
{
    EXCEPTION_CHECK_VOID(pump);
    const pump_result_info_t *result = pump->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    uint16_t hold_time = (result == NULL) ? 0 : (result->work_valid_time / 1000);
    if (pump_info->hold_time == hold_time && pump_info->force_update == false) {
        return;
    }

    uint8_t minutes = hold_time / 60, seconds = hold_time % 60;
    if (pump_info->time_colon == NULL || !lv_obj_is_valid(pump_info->time_colon)) {
        pump_info->time_colon = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->time_colon);
    }

    if (pump_info->time_minute == NULL || !lv_obj_is_valid(pump_info->time_minute)) {
        pump_info->time_minute = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->time_minute);
    }

    if (pump_info->time_second == NULL || !lv_obj_is_valid(pump_info->time_second)) {
        pump_info->time_second = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->time_second);
    }
    char minute_value[4] = {0};
    sprintf(minute_value, "%02d", minutes >= 60 ? 59 : minutes);
    lv_label_set_text(pump_info->time_minute, minute_value);
    lv_obj_set_style_text_color(pump_info->time_minute, lv_color_hex(PUMP_TEXT_BLACK_COLOR), 0);
    lv_obj_set_style_text_font(pump_info->time_minute, &lv_font_roboto_81_medium, 0);
    lv_obj_align(pump_info->time_minute, LV_ALIGN_TOP_LEFT, 80, 168);
    lv_label_set_text(pump_info->time_colon, ":");
    lv_obj_set_style_text_color(pump_info->time_colon, lv_color_hex(PUMP_TEXT_BLACK_COLOR), 0);
    lv_obj_set_style_text_font(pump_info->time_colon, &lv_font_roboto_68_medium, 0);
    lv_obj_align(pump_info->time_colon, LV_ALIGN_TOP_LEFT, 175, 179);
    char second_value[4] = {0};
    sprintf(second_value, "%02d", seconds >= 60 ? 59 : seconds);
    lv_label_set_text(pump_info->time_second, second_value);
    lv_obj_set_style_text_color(pump_info->time_second, lv_color_hex(PUMP_TEXT_BLACK_COLOR), 0);
    lv_obj_set_style_text_font(pump_info->time_second, &lv_font_roboto_81_medium, 0);
    lv_obj_align(pump_info->time_second, LV_ALIGN_TOP_LEFT, 196, 168);
    pump_info->hold_time = hold_time;
}

static int pump_home_left_level_label_update(ui_manager_context_t *thiz, uint8_t level, uint8_t is_selected)
{
    if (pump_info->left_level_label == NULL || !lv_obj_is_valid(pump_info->left_level_label)) {
        pump_info->left_level_label = lv_obj_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->left_level_label);
    }

    if (pump_info->left_level_text == NULL || !lv_obj_is_valid(pump_info->left_level_text)) {
        pump_info->left_level_text = lv_label_create(pump_info->left_level_label);
        EXCEPTION_CHECK_INT(pump_info->left_level_text);
    }
    lv_obj_clear_flag(pump_info->left_level_label, LV_OBJ_FLAG_SCROLLABLE);   /// 不要滚动条
    lv_obj_set_size(pump_info->left_level_label, 57, 38); /// 设置容器尺寸
    lv_obj_set_style_border_width(pump_info->left_level_label, 0, 0);         /// 无边框
    lv_obj_set_style_bg_opa(pump_info->left_level_label, LV_OPA_COVER, 0);    /// 背景不透明
    lv_obj_set_style_bg_color(pump_info->left_level_label, 
        lv_color_hex(is_selected ? PUMP_IMG_SEL_COLOR : PUMP_IMG_UNSEL_COLOR), 0);   /// 设置背景颜色
    lv_obj_set_style_radius(pump_info->left_level_label, 19, 0);  /// 高度 38 → 圆角设 19（或更大）
    char level_value[4] = {0};
    sprintf(level_value, "L%d", level);
    // LOG_UI("left_level_label: %d/%s, is_selected: %d", level, level_value, is_selected);
    lv_label_set_text(pump_info->left_level_text, level_value);  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(pump_info->left_level_text,
         lv_color_hex(is_selected ? PUMP_TEXT_UNSEL_COLOR : PUMP_TEXT_SEL_COLOR), 0);
    lv_obj_set_style_text_font(pump_info->left_level_text, &lv_font_roboto_21_medium, 0);  /// 设置字体
    lv_obj_center(pump_info->left_level_text); /// 文字居中
    lv_obj_align(pump_info->left_level_label, LV_ALIGN_TOP_LEFT, 66 + 4, 102);    /// 设置左档位标签位置
    return 0;
}

static int pump_home_left_level_arc_update(ui_manager_context_t *thiz, uint8_t level)
{
    if (pump_info->left_level_arc == NULL || !lv_obj_is_valid(pump_info->left_level_arc)) {
        pump_info->left_level_arc = lv_arc_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->left_level_arc);
    }
    lv_obj_set_size(pump_info->left_level_arc, 310, 310);
    lv_obj_center(pump_info->left_level_arc);
    
    lv_arc_set_bg_angles(pump_info->left_level_arc, 160, 212);
    uint16_t end_angle = 0;
    if (level > 15) {
        end_angle = 212;
    } else if (level == 0) {
        end_angle = 160;
    } else {
        end_angle = ((212 - 160) * level / 15) + 160;
    }
    lv_arc_set_angles(pump_info->left_level_arc, 160, end_angle); /// 设置圆弧角度
    lv_obj_set_style_arc_color(pump_info->left_level_arc,
        lv_color_hex(PUMP_ARC_UNSEL_COLOR), LV_PART_MAIN); /// 设置背景圆弧颜色
    lv_obj_set_style_arc_width(pump_info->left_level_arc, 10, LV_PART_MAIN); /// 设置背景圆弧宽度
    lv_obj_set_style_arc_color(pump_info->left_level_arc,
        lv_color_hex(PUMP_ARC_SEL_COLOR), LV_PART_INDICATOR); /// 设置深色前景圆弧颜色
    lv_obj_set_style_arc_width(pump_info->left_level_arc, 10, LV_PART_INDICATOR); /// 设置深色前景圆弧宽度
    lv_obj_remove_style(pump_info->left_level_arc, NULL, LV_PART_KNOB); /// 去掉旋钮

    return 0;
}

static void pump_home_left_level_update(ui_manager_context_t *thiz, uint8_t level, uint8_t is_selected)
{
    if (pump_info->left_level == level && pump_info->force_update == false) {
        return;
    }
    int ret = 0;
    ret = pump_home_left_level_arc_update(thiz, level);
    if (ret != 0) {
        return;
    }
    ret = pump_home_left_level_label_update(thiz, level, is_selected);
    if (ret != 0) {
        return;
    }
    pump_info->left_level = level;
}

static int pump_home_right_level_label_update(ui_manager_context_t *thiz, uint8_t level, uint8_t is_selected)
{
    if (pump_info->right_level_label == NULL || !lv_obj_is_valid(pump_info->right_level_label)) {
        pump_info->right_level_label = lv_obj_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->right_level_label);
    }

    if (pump_info->right_level_text == NULL || !lv_obj_is_valid(pump_info->right_level_text)) {
        pump_info->right_level_text = lv_label_create(pump_info->right_level_label);
        EXCEPTION_CHECK_INT(pump_info->right_level_text);

    }
    lv_obj_clear_flag(pump_info->right_level_label, LV_OBJ_FLAG_SCROLLABLE);   /// 不要滚动条
    lv_obj_set_size(pump_info->right_level_label, 57, 38); /// 设置容器尺寸
    lv_obj_set_style_border_width(pump_info->right_level_label, 0, 0);         /// 无边框
    lv_obj_set_style_bg_opa(pump_info->right_level_label, LV_OPA_COVER, 0);    /// 背景不透明
    lv_obj_set_style_bg_color(pump_info->right_level_label,
         lv_color_hex(is_selected ? PUMP_IMG_SEL_COLOR : PUMP_IMG_UNSEL_COLOR), 0);   /// 设置背景颜色
    lv_obj_set_style_radius(pump_info->right_level_label, 19, 0);   /// 高度 38 → 圆角设 19（或更大）
    char level_value[4] = {0};
    sprintf(level_value, "R%d", level);
    lv_label_set_text(pump_info->right_level_text, level_value);  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(pump_info->right_level_text,
         lv_color_hex(is_selected ? PUMP_TEXT_UNSEL_COLOR : PUMP_TEXT_SEL_COLOR), 0);
    lv_obj_set_style_text_font(pump_info->right_level_text, &lv_font_roboto_21_medium, 0);  /// 设置字体
    lv_obj_center(pump_info->right_level_text); /// 文字居中
    lv_obj_align(pump_info->right_level_label, LV_ALIGN_TOP_LEFT, 236 + 2, 102);    /// 设置右档位标签位置
    return 0;
}

static int pump_home_right_level_arc_update(ui_manager_context_t *thiz, uint8_t level)
{
    if (pump_info->right_level_arc == NULL || !lv_obj_is_valid(pump_info->right_level_arc)) {
        pump_info->right_level_arc = lv_arc_create(pump_info->root);
        EXCEPTION_CHECK_INT(pump_info->right_level_arc);
    }
    lv_obj_set_size(pump_info->right_level_arc, 310, 310);
    lv_obj_center(pump_info->right_level_arc);
    lv_arc_set_bg_angles(pump_info->right_level_arc, 328, 380);
    uint16_t end_angle = 0;
    if (level > 15) {
        end_angle = 328;
    } else if (level == 0) {
        end_angle = 380;
    } else {
        end_angle = ((380 - 328) * (15 - level) / 15) + 328;
    }
    lv_arc_set_angles(pump_info->right_level_arc, end_angle, 380); /// 设置圆弧角度
    lv_obj_set_style_arc_color(pump_info->right_level_arc,
         lv_color_hex(PUMP_ARC_UNSEL_COLOR), LV_PART_MAIN); /// 设置背景圆弧颜色
    lv_obj_set_style_arc_width(pump_info->right_level_arc, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(pump_info->right_level_arc,
         lv_color_hex(PUMP_ARC_SEL_COLOR), LV_PART_INDICATOR); /// 设置深色前景圆弧颜色
    lv_obj_set_style_arc_width(pump_info->right_level_arc, 10, LV_PART_INDICATOR);
    lv_obj_remove_style(pump_info->right_level_arc, NULL, LV_PART_KNOB); /// 去掉旋钮

    return 0;
}

static void pump_home_right_level_update(ui_manager_context_t *thiz, uint8_t level, uint8_t is_selected)
{
    if (pump_info->right_level == level && pump_info->force_update == false) {
        return;
    }
    int ret = 0;
    ret = pump_home_right_level_arc_update(thiz, level);
    if (ret != 0) {
        return;
    }
    ret = pump_home_right_level_label_update(thiz, level, is_selected);
    if (ret != 0) {
        return;
    }
    pump_info->right_level = level;
}


static void pump_home_speed_label_update(ui_manager_context_t *thiz, uint8_t speed)
{
    if (pump_info->speed == speed && pump_info->force_update == false) {
        return;
    }

    if (pump_info->speed_label == NULL || !lv_obj_is_valid(pump_info->speed_label)) {
        pump_info->speed_label = lv_obj_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->speed_label);
    }

    if (pump_info->speed_label_text == NULL || !lv_obj_is_valid(pump_info->speed_label_text)) {
        pump_info->speed_label_text = lv_label_create(pump_info->speed_label);
        EXCEPTION_CHECK_VOID(pump_info->speed_label_text);
    }
    lv_obj_clear_flag(pump_info->speed_label, LV_OBJ_FLAG_SCROLLABLE);   /// 不要滚动条
    lv_obj_set_size(pump_info->speed_label, 98, 38); /// 设置容器尺寸
    lv_obj_set_style_border_width(pump_info->speed_label, 0, 0);         /// 无边框
    lv_obj_set_style_bg_opa(pump_info->speed_label, LV_OPA_COVER, 0);    /// 背景不透明
    lv_obj_set_style_bg_color(pump_info->speed_label, lv_color_hex(PUMP_IMG_UNSEL_COLOR), 0);   /// 设置背景颜色
    lv_obj_set_style_radius(pump_info->speed_label, 19, 0);   /// 高度 38 → 圆角设 19（或更大）
    if (speed == 0) {
        lv_label_set_text(pump_info->speed_label_text, "Comfort");  /// 设置 label 文字内容和样式
    } else if (speed == 1) {
        lv_label_set_text(pump_info->speed_label_text, "Regular");  /// 设置 label 文字内容和样式
    } else {
        lv_label_set_text(pump_info->speed_label_text, "Boost");  /// 设置 label 文字内容和样式
    }
    lv_obj_set_style_text_color(pump_info->speed_label_text, lv_color_hex(PUMP_TEXT_SEL_COLOR), 0);
    lv_obj_set_style_text_font(pump_info->speed_label_text, &lv_font_roboto_21_medium, 0);  /// 设置字体
    lv_obj_center(pump_info->speed_label_text); /// 文字居中
    lv_obj_align(pump_info->speed_label, LV_ALIGN_TOP_LEFT, 131 + 2, 102);    /// 设置右档位标签位置

    pump_info->speed = speed;
}

static void pump_home_mode_label_update(ui_manager_context_t *thiz, uint8_t mode_index)
{
    if (mode_index >= HOME_MODE_MAX_COUNT) {
        mode_index = HOME_MODE_MAX_COUNT - 1;
    }

    if (pump_info->mode == mode_index && pump_info->force_update == false) {
        return;
    }

    if (pump_info->mode_label == NULL || !lv_obj_is_valid(pump_info->mode_label)) {
        pump_info->mode_label = lv_obj_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->mode_label);
    }
    lv_obj_clear_flag(pump_info->mode_label, LV_OBJ_FLAG_SCROLLABLE);    /// 不要滚动条
    lv_obj_set_size(pump_info->mode_label, 158, 44); /// 设置容器尺寸
    lv_obj_set_style_border_width(pump_info->mode_label, 0, 0);          /// 无边框
    lv_obj_set_style_bg_opa(pump_info->mode_label, LV_OPA_COVER, 0);    /// 背景不透明
    lv_obj_set_style_bg_color(pump_info->mode_label, lv_color_hex(PUMP_MODE_SEL_START_COLOR), 0);
    lv_obj_set_style_bg_grad_color(pump_info->mode_label, lv_color_hex(PUMP_MODE_SEL_END_COLOR), 0); 
    lv_obj_set_style_bg_grad_dir(pump_info->mode_label, LV_GRAD_DIR_HOR, 0); /// 横向渐变
    lv_obj_set_style_radius(pump_info->mode_label, 22, 0);   /// 高度 44 → 圆角设 22（或更大）
    lv_obj_align(pump_info->mode_label, LV_ALIGN_TOP_LEFT, 101 + 4, 283);    /// 设置模式标签位置

    if (pump_info->mode_text == NULL || !lv_obj_is_valid(pump_info->mode_text)) {
        pump_info->mode_text = lv_label_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->mode_text);
    }
    lv_label_set_text(pump_info->mode_text, home_mode_infos[mode_index].name);  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(pump_info->mode_text, lv_color_hex(PUMP_TEXT_UNSEL_COLOR), 0);
    if (mode_index < 3) {
        lv_obj_set_style_text_font(pump_info->mode_text, &lv_font_roboto_21_medium, 0);  /// 设置字体
    } else {
        lv_obj_set_style_text_font(pump_info->mode_text, &lv_font_roboto_16_medium, 0);  /// 设置字体
    }
    lv_obj_align(pump_info->mode_text, LV_ALIGN_TOP_LEFT, 
        home_mode_infos[mode_index].label_text_x, home_mode_infos[mode_index].label_text_y);    /// 设置label文字位置

    common_ui_img_resource_init(thiz, home_mode_infos[mode_index].img_id, &pump_info->mode_icon_info);
    EXCEPTION_CHECK_VOID(pump_info->mode_icon_info.data);
    if (pump_info->mode_icon == NULL || !lv_obj_is_valid(pump_info->mode_icon)) {
        pump_info->mode_icon = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->mode_icon);
    }
    lv_fs_path_ex_t mode_icon_img_path;
    lv_fs_make_path_from_buffer(&mode_icon_img_path, LV_FS_MEMFS_LETTER, pump_info->mode_icon_info.data,
        pump_info->mode_icon_info.size, "baj");
    lv_image_set_src(pump_info->mode_icon, (const void *)&mode_icon_img_path);
    lv_obj_align(pump_info->mode_icon, LV_ALIGN_TOP_LEFT, 
        home_mode_infos[mode_index].icon_x, home_mode_infos[mode_index].icon_y);    /// 设置icon位置

    pump_info->mode = mode_index;
}

#if IS_SUPPORT_AIR_LEAKAGE_WARNING
static void pump_home_air_leakage_label_update(ui_manager_context_t *thiz)
{
    /// 更新空气泄漏标签
    if (pump_info->air_leakage_label == NULL || !lv_obj_is_valid(pump_info->air_leakage_label)) {
        pump_info->air_leakage_label = lv_obj_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->air_leakage_label);
    }
    lv_obj_clear_flag(pump_info->air_leakage_label, LV_OBJ_FLAG_SCROLLABLE);    /// 不要滚动条
    lv_obj_set_size(pump_info->air_leakage_label, 327, 132); /// 设置容器尺寸
    lv_obj_set_style_border_width(pump_info->air_leakage_label, 0, 0);          /// 无边框
    lv_obj_set_style_bg_opa(pump_info->air_leakage_label, LV_OPA_80, 0);    /// 背景透明80%
    lv_obj_set_style_bg_color(pump_info->air_leakage_label, lv_color_hex(AIR_LEAKAGE_BG_COLOR), 0);
    lv_obj_set_style_radius(pump_info->air_leakage_label, 66, 0);   /// 高度 44 → 圆角设 22（或更大）
    lv_obj_align(pump_info->air_leakage_label, LV_ALIGN_TOP_LEFT, 17, 114);    /// 设置模式标签位置

    /// 更新空气泄漏文本
    for (int i = 0; i < AIR_LEAKAGE_TEXT_COUNT; i++) {
        if (pump_info->air_leakage_text[i] == NULL || !lv_obj_is_valid(pump_info->air_leakage_text[i])) {
            pump_info->air_leakage_text[i] = lv_label_create(pump_info->air_leakage_label);
            EXCEPTION_CHECK_VOID(pump_info->air_leakage_text[i]);
        }
        lv_label_set_text(pump_info->air_leakage_text[i], air_leakage_infos[i].name);  /// 设置 label 文字内容和样式
        lv_obj_set_style_text_color(pump_info->air_leakage_text[i], lv_color_hex(PUMP_TEXT_UNSEL_COLOR), 0);
        lv_obj_set_style_text_font(pump_info->air_leakage_text[i], 
            i == 0 ? &lv_font_roboto_20_medium : &lv_font_roboto_16_regular, 0);  /// 设置字体
        lv_obj_set_style_text_align(pump_info->air_leakage_text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(pump_info->air_leakage_text[i], LV_ALIGN_TOP_MID, 0, air_leakage_infos[i].text_y);    /// 设置模式标签位置
    }
}

static void pump_home_air_leakage_icon_update(ui_manager_context_t *thiz)
{
    if (pump_info->air_leakage_icon == NULL || !lv_obj_is_valid(pump_info->air_leakage_icon)) {
        pump_info->air_leakage_icon = lv_image_create(pump_info->root);
        EXCEPTION_CHECK_VOID(pump_info->air_leakage_icon);
    }
    common_ui_img_resource_init(thiz, RESOURCE_ID_AIR_LEAKAGE_ICON, &pump_info->air_leakage_icon_info);
    EXCEPTION_CHECK_VOID(pump_info->air_leakage_icon_info.data);
    lv_fs_path_ex_t air_leakage_icon_img_path;
    lv_fs_make_path_from_buffer(&air_leakage_icon_img_path, LV_FS_MEMFS_LETTER, pump_info->air_leakage_icon_info.data,
        pump_info->air_leakage_icon_info.size, "baj");
    lv_image_set_src(pump_info->air_leakage_icon, (const void *)&air_leakage_icon_img_path);
    lv_obj_align(pump_info->air_leakage_icon, LV_ALIGN_TOP_LEFT, 98 + 5, 40);    /// 设置icon位置
}

static void pump_home_air_leakage_update(ui_manager_context_t *thiz, ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc->algo);
    proc->algo->get_algo_data(&pump_info->algo_data);
#if IS_SUPPORT_AIR_LEAKAGE_TEST
    static uint16_t log_time = 0 , status = 0;
    log_time++;
    if (log_time > 200) {
        log_time = 0;
        status = (status == 0) ? 1 : 0;
        LOG_UI("air leakage result: %d, %d; show: %d, hide: %d, hide_wm: %d", 
            pump_info->algo_data.wear_detection.result[0], pump_info->algo_data.wear_detection.result[1],
            proc->is_air_leakage_popup_show, proc->hide_air_leakage_popup_wm, pump_info->hide_air_leakage_popup_wm);
    }

    if (status == 1) {
        pump_info->algo_data.wear_detection.result[0] = 1;
        pump_info->algo_data.wear_detection.result[1] = 1;
    } else {
        pump_info->algo_data.wear_detection.result[0] = 0;
        pump_info->algo_data.wear_detection.result[1] = 0;
    }
#endif
    if (pump_info->algo_data.wear_detection.result[0] == 0 && pump_info->algo_data.wear_detection.result[1] == 0) {
        if (proc->is_air_leakage_popup_show) {
            proc->is_air_leakage_popup_show = false;
            pump_info->hide_air_leakage_popup_wm = false;
            proc->hide_air_leakage_popup_wm = false;
            if (pump_info->air_leakage_label) {
                lv_obj_add_flag(pump_info->air_leakage_label, LV_OBJ_FLAG_HIDDEN);
            }
            if (pump_info->air_leakage_icon) {
                lv_obj_add_flag(pump_info->air_leakage_icon, LV_OBJ_FLAG_HIDDEN);
            }
            LOG_UI("release air leakage popup"); /// 解除空气泄漏弹窗提示.
        }
        return;
    }

    if (proc->hide_air_leakage_popup_wm == true && 
        pump_info->hide_air_leakage_popup_wm == false) {
        pump_info->hide_air_leakage_popup_wm = true;
        if (pump_info->air_leakage_label) {
            lv_obj_add_flag(pump_info->air_leakage_label, LV_OBJ_FLAG_HIDDEN);
            LOG_UI("hide air leakage popup wm"); /// 隐藏空气泄漏弹窗提示.
        }
        return;
    }

    if (proc->is_air_leakage_popup_show == true) {
        return;
    }
    proc->is_air_leakage_popup_show = true;
    if (pump_info->air_leakage_label) {
        lv_obj_clear_flag(pump_info->air_leakage_label, LV_OBJ_FLAG_HIDDEN);
    }
    pump_home_air_leakage_label_update(thiz);
    if (pump_info->air_leakage_icon) {
        lv_obj_clear_flag(pump_info->air_leakage_icon, LV_OBJ_FLAG_HIDDEN);
    }
    pump_home_air_leakage_icon_update(thiz);
    LOG_UI("show air leakage popup"); /// 显示空气泄漏弹窗提示.
}
#endif

static void pump_setting_ui_update(ui_manager_context_t *thiz, ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->motor);
    uint8_t control_type = proc->pump->get_breast_pump_control_type();
    pump_setting_t setting;
    uint8_t time_index = proc->motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    proc->pump->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    uint8_t mode = proc->pump->get_breast_pump_mode();
    uint8_t is_selected = control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? true : false;
    pump_home_left_level_update(thiz, setting.level[0], is_selected); /// 更新左档位
    is_selected = control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT ? true : false;
    pump_home_right_level_update(thiz, setting.level[1], is_selected); /// 更新右档位
    pump_home_speed_label_update(thiz, setting.speed); /// 更新速度
    pump_home_mode_label_update(thiz, mode); /// 更新模式
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_info == NULL) {
        pump_info = (pump_home_display_info_t *)u_malloz(sizeof(pump_home_display_info_t));
        EXCEPTION_CHECK_VOID(pump_info);
    }
    pump_home_init();  
    battery_icon_init(thiz);
    ble_ui_init(thiz);
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_info->force_update = true;
}

void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(pump_info);
    ble_ui_deinit(thiz);
    battery_icon_deinit(thiz);
    common_ui_img_resource_deinit(thiz, &pump_info->background_info);
    common_ui_img_resource_deinit(thiz, &pump_info->mode_icon_info);
    LVGL_OBJ_DELETE(pump_info->root);
    u_free(pump_info);
    pump_info = NULL;
}

int ui_display_pump_home_screen_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(pump_info);
    EXCEPTION_CHECK_INT(pump_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    ble_ui_update(thiz, pump_info->root, proc->ble);
    battery_icon_update(thiz, pump_info->root, proc->battery);
    pump_home_time_update(thiz, proc->pump);
    pump_setting_ui_update(thiz, proc);
#if IS_SUPPORT_AIR_LEAKAGE_WARNING
    pump_home_air_leakage_update(thiz, proc);
#endif
    thiz->priv.fps_count++;
    pump_info->force_update = false; 
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump home fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
        
    }
    return 0;
}
