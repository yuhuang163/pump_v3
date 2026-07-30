/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_HOME_COMP_BG                (0)

#define IDLE_LABEL_AREA_ICON_X                      (0)
#define IDLE_LABEL_AREA_ICON_Y                      (112)
#define IDLE_BUTTON_ICON_X                          (96)
#define IDLE_BUTTON_ICON_Y                          (24)

#define IDLE_LABEL_TEXT_X                           (0)
#define IDLE_LABEL_TEXT_Y                           (27)
#define IDLE_TEXT_COLOR                             (0xB25962)

#define MODE_DESC_LABEL_AREA_ICON_X                 (0)
#define MODE_DESC_LABEL_AREA_ICON_Y                 (106)

#define MODE_DESC_LABEL_TEXT_X                      (0)
#define MODE_DESC_LABEL_TEXT_Y                      (30)

#define HOME_IDLE_ANIMATION_X                       (0)
#define HOME_IDLE_ANIMATION_Y                       (166)

#define HOME_LIGHT_ANIMATION_X                      (0)
#define HOME_LIGHT_ANIMATION_Y                      (0)

#define IDLE_RECORD_AREA_LABEL_X                    (0)
#define IDLE_RECORD_AREA_LABEL_Y                    (106)
#define IDLE_RECORD_TIME_TEXT_X                     (0)
#define IDLE_RECORD_TIME_TEXT_Y                     (IDLE_RECORD_LABEL_Y + 10)
#define IDLE_RECORD_LABEL_TEXT_X                    (0)
#define IDLE_RECORD_LABEL_TEXT_Y                    (30)

#define HOME_MODE_LABEL_COLOR                      (0xB25962)
#define HOME_MODE_MAX_COUNT                        (7)
#define HOME_SPEED_MAX_COUNT                       (3)
#define HOME_TEXT_COUNT                            (3)
#define HOME_TIME_COLON_COUNT                      (2)
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
#define PUMP_SPEED_ICON_Y                          (22)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * label_icon;
    lv_obj_t * label_text;
    res_img_info_t label_info;
    lv_obj_t * button_icon;
    res_img_info_t button_info;
    lv_obj_t * animation;
    res_img_info_t animation_info;
    lv_obj_t * left_level_arc;
    lv_obj_t * left_level_circle; /// 圆圈
    lv_obj_t * left_progress_text; /// 进度文字
    lv_obj_t * right_level_arc;
    lv_obj_t * right_level_circle; /// 圆圈
    lv_obj_t * right_progress_text; /// 进度文字
    lv_obj_t * speed_icon;
    res_img_info_t speed_icon_info;
    lv_obj_t * mode_text;
    lv_obj_t * mode_label;
    lv_obj_t * mode_icon;
    res_img_info_t mode_icon_info;
    storage_manager_context_t *st;
    uint32_t start_time;
    uint16_t animation_index;
    uint8_t popup_ui_index;
    uint8_t force_update;
    uint8_t sub_ui_status;
    uint8_t speed;
    uint8_t left_level;
    uint8_t right_level;
    uint8_t mode;
    uint8_t label_ui_type;
    uint8_t label_force_update;
#if IS_USE_SPECTRA_SUCTION_CURVE
    uint8_t level_max;
    uint8_t left_level_render_max;
    uint8_t right_level_render_max;
#endif
    char custom_name[24]; /// 个性化模式名称
    uint32_t custom_mode_id;
    uint8_t rsv[10];
} home_display_info_t;

typedef struct {
    const char *name;
    uint16_t img_id;
    uint16_t x_offset; 
    uint16_t y_offset; 
    bool align_center; 
    const lv_font_t *font;
} home_text_info_t;

typedef enum {
    HOME_TEXT_IDLE = 0,          /// 默认文本
    HOME_TEXT_RECORD,            /// 显示记录文本
    HOME_TEXT_SIMULATE_MODE_DESC, /// 显示刺激模式描述文本
    HOME_TEXT_EXPRESSION_MODE_DESC, /// 显示表达模式描述文本
    HOME_TEXT_MIXED_MODE_DESC, /// 显示混合模式描述文本
    HOME_TEXT_MILK_BOOST_DESC, /// 显示2+8奶量提升模式描述文本
    HOME_TEXT_POWER_PUMPING_DESC, /// 显示PP追奶模式描述文本
    HOME_TEXT_CUSTOMIZE_DESC, /// 显示个性化模式描述文本
    HOME_TEXT_ID_MAX
} home_text_id_e;

typedef struct {
    const char *name;
    uint16_t img_id; 
    uint16_t width; 
    uint16_t height;
} home_mode_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_HOME_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_bg_data[] = {
    // #include "comparison_png2txt/home_ip1_360x360.txt"
    // #include "comparison_png2txt/home_ip2_360x360.txt"
    // #include "comparison_png2txt/home_ip3_360x360.txt"
    #include "comparison_png2txt/home_idle2_360x360.txt"
    // #include "comparison_png2txt/home_ip5_360x360.txt"
    // #include "comparison_png2txt/home_ip6_360x360.txt"
};
#endif

static home_display_info_t *home_info;

const static home_text_info_t home_text_infos[HOME_TEXT_ID_MAX] = {
    {"Click         to start pumping", BASE_RES_ID_HOME_NARROW_LABLE, 
        IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y, false, &lv_font_oppo_sans_16_medium},
    {"since last pumping session", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, IDLE_RECORD_LABEL_TEXT_Y, false, &lv_font_oppo_sans_16_medium},
    {"Fast, gentle suction to\ntrigger let-down", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Slow, deep suction for\nefficient milk expression", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Efficiently promote let-\ndown and better emptying", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Most suitable for daily use\nafter successful lactation", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Increase milk supply, no\nmore than once a day", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Edit and sync anytime via\nthe Momcozy App", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},   
};

const static uint16_t home_speed_icon_ids[HOME_SPEED_MAX_COUNT] = {
    BASE_RES_ID_PUMP_SPEED_SLOW_ICON,
    BASE_RES_ID_PUMP_SPEED_REGULAR_ICON,
    BASE_RES_ID_PUMP_SPEED_FAST_ICON,
};

const static home_mode_info_t home_mode_infos[HOME_MODE_MAX_COUNT] = {
    {"Stimulate", BASE_RES_ID_PUMP_SIMULATION_ICON, 122, 40},
    {"Expression", BASE_RES_ID_PUMP_EXPRESSION_ICON, 134, 40},
    {"Mixed", BASE_RES_ID_PUMP_MIXED_ICON, 98, 40},
    {"Relax", BASE_RES_ID_PUMP_MIXED_ICON, 98, 40},
    {"Milk Boost", BASE_RES_ID_PUMP_MILK_BOOST_ICON, 130, 40},
    {"Power Pumping", BASE_RES_ID_PUMP_MILK_BOOST_ICON, 168, 40},
    {"Customize", BASE_RES_ID_PUMP_CUSTOMIZE_ICON, 168, 40},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_root_init(void)
{
    if (home_info->root == NULL || !lv_obj_is_valid(home_info->root)) {
        home_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(home_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(home_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(home_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(home_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(home_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(home_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

#if IS_SUPPORT_ROOT_HOME_COMP_BG
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (home_info->background == NULL || !lv_obj_is_valid(home_info->background)) {
        home_info->background = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(home_info->background, (const void *)&bg_img_path);
        lv_obj_align(home_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    if (home_info->force_update == false) {
        return;
    }

    if (home_info->background == NULL || !lv_obj_is_valid(home_info->background)) {
        home_info->background = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->background);
        lv_obj_align(home_info->background, LV_ALIGN_CENTER, 0, 0);
    }

#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_REGULAR_PUMPING_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    lv_image_set_src(home_info->background, background_snapshot);
#else
    lv_obj_add_flag(home_info->background, LV_OBJ_FLAG_HIDDEN); /// 隐藏
#endif
}
#endif

static void home_label_ui_update(ui_manager_context_t *thiz, home_text_id_e text_id, uint16_t x_offset, uint16_t y_offset, bool is_show)
{
    if (home_info->label_force_update == false) {
        return;
    }

    if (home_info->label_icon == NULL || !lv_obj_is_valid(home_info->label_icon)) {
        home_info->label_icon = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->label_icon);
        lv_obj_add_flag(home_info->label_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (home_info->label_text == NULL || !lv_obj_is_valid(home_info->label_text)) {
        home_info->label_text = lv_label_create(home_info->label_icon);
        EXCEPTION_CHECK_VOID(home_info->label_text);
        lv_obj_add_flag(home_info->label_text, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_line_space(home_info->label_text, 3, 0); /// 设置行间距
    }

    if (home_info->button_icon == NULL || !lv_obj_is_valid(home_info->button_icon)) {
        home_info->button_icon = lv_image_create(home_info->label_icon);
        EXCEPTION_CHECK_VOID(home_info->button_icon);
        lv_obj_add_flag(home_info->button_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (text_id >= HOME_TEXT_ID_MAX) {
        text_id = HOME_TEXT_IDLE;
    }

    uint8_t button_show = 0;
    uint16_t button_x_offset = 0;
    uint16_t button_y_offset = 0;
    if (text_id == HOME_TEXT_IDLE) {
        button_show = 1;
        button_x_offset = IDLE_BUTTON_ICON_X;
        button_y_offset = IDLE_BUTTON_ICON_Y;
    }
    
    const home_text_info_t *info = &home_text_infos[text_id];
    if (is_show) {
        /// 更新图标
        lv_obj_clear_flag(home_info->label_icon, LV_OBJ_FLAG_HIDDEN);
        common_ui_img_resource_init(thiz, info->img_id, &home_info->label_info);
        EXCEPTION_CHECK_VOID(home_info->label_info.data);
        lv_fs_path_ex_t label_img_path;
        lv_fs_make_path_from_buffer(&label_img_path, LV_FS_MEMFS_LETTER, 
            home_info->label_info.data, home_info->label_info.size, "baj");
        lv_image_set_src(home_info->label_icon, (const void *)&label_img_path);
        lv_obj_align(home_info->label_icon, x_offset == 0 ? LV_ALIGN_TOP_MID : LV_ALIGN_TOP_LEFT,
            x_offset, y_offset); /// 设置icon位置
        /// 更新文本
        lv_obj_clear_flag(home_info->label_text, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(home_info->label_text, home_text_infos[text_id].name);  /// 设置 label 文字内容和样式
        lv_obj_set_style_text_color(home_info->label_text, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->label_text, info->font, 0);  /// 设置字体
        if (info->x_offset == 0) {
            lv_obj_align(home_info->label_text, LV_ALIGN_TOP_MID, 0, info->y_offset); /// 设置文字位置
        } else {
            lv_obj_align(home_info->label_text, LV_ALIGN_TOP_LEFT, info->x_offset, info->y_offset); /// 设置文字位置
        }
        lv_obj_set_style_text_align(home_info->label_text, info->align_center ? 
            LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);  /// 设置文本居中对齐
        if (button_show == false) {
            lv_obj_add_flag(home_info->button_icon, LV_OBJ_FLAG_HIDDEN);
            return;
        }
        /// 更新按钮图标
        lv_obj_clear_flag(home_info->button_icon, LV_OBJ_FLAG_HIDDEN);
        common_ui_img_resource_init(thiz, BASE_RES_ID_HOME_START_BUTTON_ICON,
            &home_info->button_info);
        EXCEPTION_CHECK_VOID(home_info->button_info.data);
        lv_fs_path_ex_t button_img_path;
        lv_fs_make_path_from_buffer(&button_img_path, LV_FS_MEMFS_LETTER,
                                    home_info->button_info.data,
                                    home_info->button_info.size, "baj");
        lv_image_set_src(home_info->button_icon, (const void *)&button_img_path);
        lv_obj_align(home_info->button_icon, LV_ALIGN_TOP_LEFT, button_x_offset, button_y_offset); /// 设置icon位置
        return;
    }
    lv_obj_add_flag(home_info->label_icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(home_info->label_text, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(home_info->button_icon, LV_OBJ_FLAG_HIDDEN);
}

static void home_root_animation_update(ui_manager_context_t *thiz, 
    uint16_t start_resource_id, uint16_t animation_size, uint16_t x_offset, uint16_t y_offset)
{  
    if (home_info->animation == NULL || !lv_obj_is_valid(home_info->animation)) {
        home_info->animation = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->animation);
    }  
    
    if (home_info->animation_index >= animation_size) {
        home_info->animation_index = 0;
    }

    lv_obj_align(home_info->animation, LV_ALIGN_TOP_MID, x_offset, y_offset); 
    if (home_info->animation_info.data != NULL) {
        common_ui_img_resource_deinit(thiz, &home_info->animation_info);
        memset(&home_info->animation_info, 0, sizeof(home_info->animation_info));
    }
    common_ui_img_resource_init_with_theme(thiz, 
        start_resource_id + home_info->animation_index, &home_info->animation_info);
    EXCEPTION_CHECK_VOID(home_info->animation_info.data);
    lv_fs_path_ex_t animation_img_path;
    lv_fs_make_path_from_buffer(&animation_img_path, LV_FS_MEMFS_LETTER, 
        home_info->animation_info.data, home_info->animation_info.size, "baj");
    lv_image_set_src(home_info->animation, (const void *)&animation_img_path);
    home_info->animation_index++;
}

static void home_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
    if (home_info->label_ui_type != proc->home_display_type) {
        home_info->label_ui_type = proc->home_display_type;
        home_info->label_force_update = true;
        LOG_UI("home_display_type: %d", proc->home_display_type);
    }
    
    switch (home_info->label_ui_type) {
        default:
        case HOME_UI_DISPLAY_DEFAULT:
            home_label_ui_update(thiz, HOME_TEXT_IDLE,
                IDLE_LABEL_AREA_ICON_X, IDLE_LABEL_AREA_ICON_Y, true);
            home_root_animation_update(thiz, BASE_RES_ID_HD_ANIM_IP_START, 
                BASE_RES_ID_HD_ANIM_IP_END - BASE_RES_ID_HD_ANIM_IP_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_SIMULATE_MODE_DESC:
            home_label_ui_update(thiz, HOME_TEXT_SIMULATE_MODE_DESC,
                MODE_DESC_LABEL_AREA_ICON_X, MODE_DESC_LABEL_AREA_ICON_Y, true);
            home_root_animation_update(thiz, BASE_RES_ID_HD_ANIM_IP_START,
                BASE_RES_ID_HD_ANIM_IP_END - BASE_RES_ID_HD_ANIM_IP_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_EXPRESSION_MODE_DESC:
            home_label_ui_update(thiz, HOME_TEXT_EXPRESSION_MODE_DESC,
                MODE_DESC_LABEL_AREA_ICON_X, MODE_DESC_LABEL_AREA_ICON_Y, true);
            home_root_animation_update(thiz, BASE_RES_ID_HD_ANIM_IP_START,
                BASE_RES_ID_HD_ANIM_IP_END - BASE_RES_ID_HD_ANIM_IP_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_MIXED_MODE_DESC:
            home_label_ui_update(thiz, HOME_TEXT_MIXED_MODE_DESC,
                MODE_DESC_LABEL_AREA_ICON_X, MODE_DESC_LABEL_AREA_ICON_Y, true);
            home_root_animation_update(thiz, BASE_RES_ID_HD_ANIM_IP_START,
                BASE_RES_ID_HD_ANIM_IP_END - BASE_RES_ID_HD_ANIM_IP_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_MILK_BOOST_DESC:
            home_label_ui_update(thiz, HOME_TEXT_MILK_BOOST_DESC,
                MODE_DESC_LABEL_AREA_ICON_X, MODE_DESC_LABEL_AREA_ICON_Y, true);
            home_root_animation_update(thiz, BASE_RES_ID_HD_ANIM_IP_START,
                BASE_RES_ID_HD_ANIM_IP_END - BASE_RES_ID_HD_ANIM_IP_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_POWER_PUMPING_DESC:
            home_label_ui_update(thiz, HOME_TEXT_POWER_PUMPING_DESC,
                MODE_DESC_LABEL_AREA_ICON_X, MODE_DESC_LABEL_AREA_ICON_Y, true);
            home_root_animation_update(thiz, BASE_RES_ID_HD_ANIM_IP_START,
                BASE_RES_ID_HD_ANIM_IP_END - BASE_RES_ID_HD_ANIM_IP_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_CUSTOMIZE_DESC:
            home_label_ui_update(thiz, HOME_TEXT_CUSTOMIZE_DESC,
                MODE_DESC_LABEL_AREA_ICON_X, MODE_DESC_LABEL_AREA_ICON_Y, true);
            home_root_animation_update(thiz, BASE_RES_ID_HD_ANIM_IP_START,
                BASE_RES_ID_HD_ANIM_IP_END - BASE_RES_ID_HD_ANIM_IP_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
    }
    home_info->label_force_update = false;
}

static int home_pump_left_level_text_update(ui_manager_context_t *thiz, uint8_t level, uint16_t end_angle)
{
    char level_text[4] = {0};
    if (home_info->left_progress_text == NULL || !lv_obj_is_valid(home_info->left_progress_text)) {
        home_info->left_progress_text = lv_label_create(home_info->root);
        EXCEPTION_CHECK_INT(home_info->left_progress_text);
        lv_obj_set_style_text_color(home_info->left_progress_text, lv_color_hex(PUMP_TIME_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->left_progress_text, &lv_font_oppo_sans_16_medium, 0);
    }
    snprintf(level_text, sizeof(level_text), "%d", level);
    lv_label_set_text(home_info->left_progress_text, level_text);
    common_ui_text_place_in_circle(home_info->left_progress_text, 
        end_angle, PUMP_LEVEL_RING_R + 15, false); /// 设置文字位置
    return 0;
}

static int home_pump_left_level_arc_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (home_info->left_level_arc == NULL || !lv_obj_is_valid(home_info->left_level_arc)) {
        home_info->left_level_arc = lv_arc_create(home_info->root);
        EXCEPTION_CHECK_INT(home_info->left_level_arc);
        lv_obj_set_size(home_info->left_level_arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE); /// 设置弧线大小
        lv_obj_center(home_info->left_level_arc); /// 设置弧线居中
        lv_arc_set_bg_angles(home_info->left_level_arc, 
            PUMP_LEFT_ARC_START_ANGLE, PUMP_LEFT_ARC_END_ANGLE);  /// 设置背景弧角度
        lv_arc_set_angles(home_info->left_level_arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);  /// 设置前景弧角度
        lv_obj_set_style_arc_color(home_info->left_level_arc,
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(home_info->left_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(home_info->left_level_arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧不透明度38%（97/255≈38%）
        lv_obj_set_style_arc_color(home_info->left_level_arc,
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(home_info->left_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(home_info->left_level_arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧完全不透明
        lv_obj_remove_style(home_info->left_level_arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    } else {
        lv_arc_set_angles(home_info->left_level_arc, PUMP_LEFT_ARC_START_ANGLE, end_angle);  /// 设置前景弧角度
    }

    return 0;
}

static int home_pump_left_level_circle_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (home_info->left_level_circle == NULL || !lv_obj_is_valid(home_info->left_level_circle)) {
        home_info->left_level_circle = lv_obj_create(home_info->left_level_arc); // 关键：挂到 arc 下
        lv_obj_remove_style_all(home_info->left_level_circle);
        lv_obj_set_size(home_info->left_level_circle, PUMP_LEVEL_CIRCLE_SIZE, PUMP_LEVEL_CIRCLE_SIZE); /// 设置白点大小
        lv_obj_set_style_radius(home_info->left_level_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(home_info->left_level_circle, lv_color_hex(PUMP_LEVEL_CIRCLE_COLOR), 0);
        lv_obj_set_style_bg_opa(home_info->left_level_circle, LV_OPA_COVER, 0);
    }
    common_ui_place_dot_on_arc(home_info->left_level_arc, home_info->left_level_circle, end_angle,
                               PUMP_LEVEL_ARC_WIDTH,   /// 弧线宽度
                               PUMP_LEVEL_CIRCLE_SIZE,   /// 白点直径
                               0,   /// 额外内边距: 需要更靠里就填 1~2
                               false);  /// 左侧弧线做镜像方向修正
    return 0;
}

static void home_pump_left_level_update(ui_manager_context_t *thiz, uint8_t level)
{
#if IS_USE_SPECTRA_SUCTION_CURVE
    if (home_info->left_level == level && home_info->force_update == false &&
        home_info->left_level_render_max == home_info->level_max) {
        return;
    }
#else
    if (home_info->left_level == level && home_info->force_update == false) {
        return;
    }
#endif

    int ret = 0;
    uint16_t end_angle = 0;
#if IS_USE_SPECTRA_SUCTION_CURVE
    if (level > home_info->level_max) {
        end_angle = PUMP_LEFT_ARC_END_ANGLE;
        level = home_info->level_max;
    } else if (level == 0) {
        end_angle = PUMP_LEFT_ARC_START_ANGLE;
    } else {
        end_angle = ((PUMP_LEFT_ARC_END_ANGLE - PUMP_LEFT_ARC_START_ANGLE) * level / home_info->level_max) +
                PUMP_LEFT_ARC_START_ANGLE;
    }
#else
    if (level > PUMP_LEVEL_MAX) {
        end_angle = PUMP_LEFT_ARC_END_ANGLE;
        level = PUMP_LEVEL_MAX;
    } else if (level == 0) {
        end_angle = PUMP_LEFT_ARC_START_ANGLE;
    } else {
        end_angle = ((PUMP_LEFT_ARC_END_ANGLE - PUMP_LEFT_ARC_START_ANGLE) * level / PUMP_LEVEL_MAX) +
                PUMP_LEFT_ARC_START_ANGLE;
    }
#endif
    
    ret = home_pump_left_level_text_update(thiz, level, end_angle);
    if (ret != 0) {
        return;
    }

    ret = home_pump_left_level_arc_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }

    ret = home_pump_left_level_circle_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }
    home_info->left_level = level;
#if IS_USE_SPECTRA_SUCTION_CURVE
    home_info->left_level_render_max = home_info->level_max;
#endif
}


static int home_pump_right_level_text_update(ui_manager_context_t *thiz, uint8_t level, uint16_t end_angle)
{
    char level_text[4] = {0};
    if (home_info->right_progress_text == NULL || !lv_obj_is_valid(home_info->right_progress_text)) {
        home_info->right_progress_text = lv_label_create(home_info->root);
        EXCEPTION_CHECK_INT(home_info->right_progress_text);
        lv_obj_set_style_text_color(home_info->right_progress_text, lv_color_hex(PUMP_TIME_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->right_progress_text, &lv_font_oppo_sans_16_medium, 0);
    } 
    snprintf(level_text, sizeof(level_text), "%d", level);
    lv_label_set_text(home_info->right_progress_text, level_text);
    common_ui_text_place_in_circle(home_info->right_progress_text, 
        end_angle, PUMP_LEVEL_RING_R + 15, false);
    return 0;
}

static int home_pump_right_level_arc_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (home_info->right_level_arc == NULL || !lv_obj_is_valid(home_info->right_level_arc)) {
        home_info->right_level_arc = lv_arc_create(home_info->root);
        EXCEPTION_CHECK_INT(home_info->right_level_arc);
        lv_obj_set_size(home_info->right_level_arc, PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE);
        lv_obj_center(home_info->right_level_arc);
        lv_arc_set_bg_angles(home_info->right_level_arc, 
            PUMP_RIGHT_ARC_START_ANGLE, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置背景弧角度
        lv_arc_set_angles(home_info->right_level_arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置前景弧角度
        lv_obj_set_style_arc_color(home_info->right_level_arc, 
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(home_info->right_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN); /// 背景弧宽度
        lv_obj_set_style_arc_opa(home_info->right_level_arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧不透明度38%（97/255≈38%）
        lv_obj_set_style_arc_color(home_info->right_level_arc, 
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(home_info->right_level_arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR); /// 前景弧宽度
        lv_obj_set_style_arc_opa(home_info->right_level_arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧完全不透明
        lv_obj_remove_style(home_info->right_level_arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    } else {
        lv_arc_set_angles(home_info->right_level_arc, end_angle, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置前景弧角度
    }
    return 0;
}

static int home_pump_right_level_circle_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (home_info->right_level_circle == NULL || !lv_obj_is_valid(home_info->right_level_circle)) {
        home_info->right_level_circle = lv_obj_create(home_info->right_level_arc); // 关键：挂到 arc 下
        lv_obj_remove_style_all(home_info->right_level_circle);
        lv_obj_set_size(home_info->right_level_circle, PUMP_LEVEL_CIRCLE_SIZE, PUMP_LEVEL_CIRCLE_SIZE); /// 设置白点大小
        lv_obj_set_style_radius(home_info->right_level_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(home_info->right_level_circle, lv_color_hex(PUMP_LEVEL_CIRCLE_COLOR), 0);
        lv_obj_set_style_bg_opa(home_info->right_level_circle, LV_OPA_COVER, 0);
    }
    common_ui_place_dot_on_arc(home_info->right_level_arc, home_info->right_level_circle, end_angle,
                               PUMP_LEVEL_ARC_WIDTH,      /// 弧线宽度
                               PUMP_LEVEL_CIRCLE_SIZE,      /// 白点直径
                               -0.5,     /// 额外内边距: 需要更靠里就填 1~2
                               false); /// 右侧跨 360 度时也保持绝对角度
    return 0;
}

static void home_pump_right_level_update(ui_manager_context_t *thiz, uint8_t level)
{
#if IS_USE_SPECTRA_SUCTION_CURVE
    if (home_info->right_level == level && home_info->force_update == false &&
        home_info->right_level_render_max == home_info->level_max) {
        return;
    }
#else
    if (home_info->right_level == level && home_info->force_update == false) {
        return;
    }
#endif
    int ret = 0;
    uint16_t end_angle = 0;
#if IS_USE_SPECTRA_SUCTION_CURVE
    if (level > home_info->level_max) {
        end_angle = PUMP_RIGHT_ARC_START_ANGLE;
        level = home_info->level_max;
    } else if (level == 0) {
        end_angle = PUMP_RIGHT_ARC_END_ANGLE;
    } else {
        end_angle = ((PUMP_RIGHT_ARC_END_ANGLE - PUMP_RIGHT_ARC_START_ANGLE) *
            (home_info->level_max - level) / home_info->level_max) + PUMP_RIGHT_ARC_START_ANGLE;
    }
#else
    if (level > PUMP_LEVEL_MAX) {
        end_angle = PUMP_RIGHT_ARC_START_ANGLE;
        level = PUMP_LEVEL_MAX;
    } else if (level == 0) {
        end_angle = PUMP_RIGHT_ARC_END_ANGLE;
    } else {
        end_angle = ((PUMP_RIGHT_ARC_END_ANGLE - PUMP_RIGHT_ARC_START_ANGLE) *
            (PUMP_LEVEL_MAX - level) / PUMP_LEVEL_MAX) + PUMP_RIGHT_ARC_START_ANGLE;
    }
#endif

    ret = home_pump_right_level_text_update(thiz, level, end_angle);
    if (ret != 0) {
        return;
    }

    ret = home_pump_right_level_arc_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }

    ret = home_pump_right_level_circle_update(thiz, end_angle);
    if (ret != 0) {
        return;
    }
    home_info->right_level = level;
#if IS_USE_SPECTRA_SUCTION_CURVE
    home_info->right_level_render_max = home_info->level_max;
#endif
}


static void home_pump_speed_label_update(ui_manager_context_t *thiz, uint8_t speed)
{
    if (speed >= HOME_SPEED_MAX_COUNT) {
        return;
    }

    if (home_info->speed == speed && home_info->force_update == false) {
        return;
    } 
    
    lv_fs_path_ex_t speed_icon_path;
    if (home_info->speed_icon == NULL || !lv_obj_is_valid(home_info->speed_icon)) {
        home_info->speed_icon = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->speed_icon);
    } 
    common_ui_img_resource_init(thiz, home_speed_icon_ids[speed], &home_info->speed_icon_info);
    EXCEPTION_CHECK_VOID(home_info->speed_icon_info.data);
    lv_fs_make_path_from_buffer(&speed_icon_path, LV_FS_MEMFS_LETTER, 
        home_info->speed_icon_info.data, home_info->speed_icon_info.size, "baj");
    lv_image_set_src(home_info->speed_icon, (const void *)&speed_icon_path);
    lv_obj_align(home_info->speed_icon, LV_ALIGN_TOP_LEFT, PUMP_SPEED_ICON_X, PUMP_SPEED_ICON_Y);
    home_info->speed = speed; 
}

static void home_pump_mode_update(ui_manager_context_t *thiz, uint8_t mode_index)
{
    if (mode_index >= HOME_MODE_MAX_COUNT) {
        mode_index = HOME_MODE_MAX_COUNT - 1;
    }

    if (home_info->mode == mode_index && home_info->force_update == false) {
        return;
    }

    /// 设置label
    if (home_info->mode_label == NULL || !lv_obj_is_valid(home_info->mode_label)) {
        home_info->mode_label = lv_obj_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->mode_label);
        lv_obj_remove_style_all(home_info->mode_label);
        lv_obj_set_style_radius(home_info->mode_label, LV_RADIUS_CIRCLE, 0); /// 设置label圆角
        lv_obj_set_style_bg_color(home_info->mode_label, lv_color_hex(HOME_MODE_LABEL_COLOR), 0); /// 设置label背景颜色
        lv_obj_set_style_bg_grad_color(home_info->mode_label, lv_color_hex(HOME_MODE_LABEL_COLOR), 0); /// 设置label背景颜色
        lv_obj_set_style_bg_opa(home_info->mode_label, LV_OPA_COVER, 0); /// 设置label背景不透明度
        lv_obj_align(home_info->mode_label, LV_ALIGN_TOP_MID, 0, 70); /// 设置label文字位置
    } 

    /// 设置label文字
    if (home_info->mode_text == NULL || !lv_obj_is_valid(home_info->mode_text)) {
        home_info->mode_text = lv_label_create(home_info->mode_label);
        EXCEPTION_CHECK_VOID(home_info->mode_text);
        lv_obj_set_style_text_color(home_info->mode_text, lv_color_hex(MODE_TEXT_SEL_COLOR), 0);
        lv_obj_set_style_text_font(home_info->mode_text, &lv_font_oppo_sans_16_medium, 0);
        lv_obj_align(home_info->mode_text, LV_ALIGN_LEFT_MID, 40, 0); /// 设置label文字位置
    } 
    
    if (mode_index == BREAST_PUMP_CUSTOM_MODE) {
        lv_label_set_text(home_info->mode_text, home_info->custom_name);  /// 设置 label 文字内容和样式
    } else {
        lv_label_set_text(home_info->mode_text, home_mode_infos[mode_index].name);  /// 设置 label 文字内容和样式
    }
    lv_obj_update_layout(home_info->mode_text);
    lv_coord_t text_width = lv_obj_get_width(home_info->mode_text);
    lv_obj_set_size(home_info->mode_label, text_width + 40 + 12,
        home_mode_infos[mode_index].height); /// 设置label大小

    /// 设置icon
    if (home_info->mode_icon == NULL || !lv_obj_is_valid(home_info->mode_icon)) {
        home_info->mode_icon = lv_image_create(home_info->mode_label);
        EXCEPTION_CHECK_VOID(home_info->mode_icon);
        lv_obj_align(home_info->mode_icon, LV_ALIGN_LEFT_MID, 12, 0); /// 设置icon位置
    }
    common_ui_img_resource_init(thiz, home_mode_infos[mode_index].img_id, &home_info->mode_icon_info);
    EXCEPTION_CHECK_VOID(home_info->mode_icon_info.data);
    lv_fs_path_ex_t mode_icon_path;
    lv_fs_make_path_from_buffer(&mode_icon_path, LV_FS_MEMFS_LETTER, 
        home_info->mode_icon_info.data, home_info->mode_icon_info.size, "baj");
    lv_image_set_src(home_info->mode_icon, (const void *)&mode_icon_path);

    home_info->mode = mode_index;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_info == NULL) {
        home_info = (home_display_info_t *)u_malloz(sizeof(home_display_info_t));
        EXCEPTION_CHECK_VOID(home_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_root_init();
    ble_ui_init(thiz);
    home_info->force_update = true;
    home_info->label_force_update = true;
    home_info->animation_index = 0;
    home_info->popup_ui_index = ID_POPUP_UI_NONE;
    home_info->sub_ui_status = 1;
    home_info->start_time = 0;
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    home_info->mode = pump_mode;
#if IS_USE_SPECTRA_SUCTION_CURVE
    home_info->level_max = pump_mode == BREAST_PUMP_STIMULATION_MODE ? BREAST_PUMP_LEVEL_5 : BREAST_PUMP_LEVEL_15;
#endif
    memset(home_info->custom_name, 0, sizeof(home_info->custom_name));
    sprintf(home_info->custom_name, "Customize");
    home_info->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
}

void ui_display_home_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(home_info);
    common_ui_img_resource_deinit(thiz, &home_info->label_info);
    common_ui_img_resource_deinit(thiz, &home_info->button_info);
    common_ui_img_resource_deinit(thiz, &home_info->animation_info);
    common_ui_img_resource_deinit(thiz, &home_info->speed_icon_info);
    common_ui_img_resource_deinit(thiz, &home_info->mode_icon_info);
    ble_ui_deinit(thiz);
    ui_common_operation_tip_remove(thiz);
    LVGL_OBJ_DELETE(home_info->root);
    u_free(home_info);
    home_info = NULL;
}

int ui_display_home_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && home_info && home_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble && proc->pump);
    pump_setting_t setting;
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, setting);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    if (pump_mode == BREAST_PUMP_CUSTOM_MODE && home_info->st) {
        uint32_t custom_mode_id = 0;
        home_info->st->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO_ID, BREAST_PUMP_CUSTOM_MODE,
            0, &custom_mode_id);
        if (custom_mode_id != home_info->custom_mode_id) {
            home_info->custom_mode_id = custom_mode_id;
            home_info->st->get_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE,
                sizeof(home_info->custom_name), home_info->custom_name);
        }
    }
#if IS_USE_SPECTRA_SUCTION_CURVE
    uint8_t level_max = pump_mode == BREAST_PUMP_STIMULATION_MODE ?
        BREAST_PUMP_LEVEL_5 : BREAST_PUMP_LEVEL_15;
    if (home_info->level_max != level_max) {
        home_info->level_max = level_max;
        home_info->force_update = true;
    }
#endif
    home_root_bg_update(thiz);
    ble_ui_update(thiz, home_info->root, proc->ble, PUMP_HOME_BLE_UI_X, PUMP_HOME_BLE_UI_Y);
    home_pump_left_level_update(thiz, setting.level[0]);
    home_pump_right_level_update(thiz, setting.level[1]);
    home_pump_speed_label_update(thiz, setting.speed);
    home_pump_mode_update(thiz, pump_mode);
    home_ui_update(thiz, proc);
    ui_common_operation_tip_update(thiz, proc->popup_ui_type, home_info->root,
        &home_info->popup_ui_index);
    home_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
