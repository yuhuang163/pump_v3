/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_LEVEL_COMP_BG                         (0)

#define PUMP_ARC_COLOR                                        (0xBF646E)
#define PUMP_LEVEL_CIRCLE_COLOR                               (0xFFFFFF) /// (0xFFEDEF)
#define PUMP_TITLE_TEXT_COLOR                                 (0xFFFFFF)
#define PUMP_LEVEL_TEXT_COLOR                                 (0xC06670)
#define PUMP_LEVEL_OFFSET_TEXT_COLOR                          (0x533129)
#define PUMP_LEVEL_RING_SIZE                                  (314)
#define PUMP_LEVEL_RING_R                                     (PUMP_LEVEL_RING_SIZE / 2)
#define PUMP_LEFT_ARC_START_ANGLE                             (120)
#define PUMP_LEFT_ARC_END_ANGLE                               (241 - 10)
#define PUMP_RIGHT_ARC_START_ANGLE                            (299 + 10)
#define PUMP_RIGHT_ARC_END_ANGLE                              (420)
#define PUMP_LEVEL_ARC_WIDTH                                  (20)
#define PUMP_LEVEL_CIRCLE_SIZE                                (40)
#define PUMP_LEVEL_MAX                                        (15)

#define PUMP_LEVEL_LEFT_CIRCLE_X                              (80 - 23)
#define PUMP_LEVEL_LEFT_CIRCLE_Y                              (13)
#define PUMP_LEVEL_RIGHT_CIRCLE_X                             (240 - 23)
#define PUMP_LEVEL_RIGHT_CIRCLE_Y                             PUMP_LEVEL_LEFT_CIRCLE_Y

#define PUMP_LEVEL_TITLE_TEXT_X                               (156)
#define PUMP_LEVEL_TITLE_TEXT_Y                               (28 + 4)

#define PUMP_LEVEL_SYMBOL_X                                   (99)
#define PUMP_LEVEL_SYMBOL_Y                                   (167)

/// 符号状态
typedef enum {
    PUMP_BOTH_LEVEL_SYMBOL_NONE = 0,  
    PUMP_BOTH_LEVEL_SYMBOL_ADD,   /// 加
    PUMP_BOTH_LEVEL_SYMBOL_SUB,   /// 减
} pump_both_level_symbol_status_e;

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * title_text; /// 档位文字
    lv_obj_t * symbol; /// 符号
    res_img_info_t symbol_info;
    lv_obj_t * level_circle[2]; /// 圆圈
    lv_obj_t * level_arc[2];    /// 圆弧
    lv_obj_t * level_text[2];   /// 等级文字
    lv_obj_t * level_offset_text; /// 等级偏移文字
    uint8_t force_update;
    uint8_t popup_ui_index;
    int8_t left_level_change;
    int8_t right_level_change;
    int8_t level_change;
    uint8_t symbol_status; /// 符号状态
    uint8_t rsv[10];
} level_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_LEVEL_COMP_BG
static PSRAM_DATA_LOC uint8_t setting_bg_data[] = {
    #include "comparison_png2txt/LR_level_adjust2_360x360.txt"
};
#endif

static level_display_info_t *level_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void pump_both_level_root_init(void)
{
    if (level_info->root == NULL || !lv_obj_is_valid(level_info->root)) {
        level_info->root = lv_obj_create(lv_screen_active());
        lv_obj_set_size(level_info->root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(level_info->root, lv_color_black(), 0);
        // lv_obj_set_style_bg_color(level_info->root, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(level_info->root, 0, 0);
        lv_obj_set_style_pad_all(level_info->root, 0, 0);
        lv_obj_align(level_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }
}

#if IS_SUPPORT_PUMP_LEVEL_COMP_BG
static void pump_both_level_root_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);
    
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (level_info->background == NULL || !lv_obj_is_valid(level_info->background)) {
        level_info->background = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->background);
    }
    
    if (status == 0) {
        lv_obj_add_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_path_ex_t bg_img_path;
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, setting_bg_data,
            sizeof(setting_bg_data), "png");
        lv_image_set_src(level_info->background, (const void *)&bg_img_path);
        lv_obj_align(level_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_both_level_root_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && level_info && level_info->root);
    
    if (level_info->force_update == false) {
        return;
    }

#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_PUMP_BOTH_LEVEL_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
#endif

    lv_fs_path_ex_t bg_img_path;
    if (level_info->background == NULL || !lv_obj_is_valid(level_info->background)) {
        level_info->background = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->background);
        lv_obj_align(level_info->background, LV_ALIGN_CENTER, 0, 0);
#if LV_USE_SNAPSHOT
        lv_image_set_src(level_info->background, background_snapshot);
#else
        lv_obj_add_flag(level_info->background, LV_OBJ_FLAG_HIDDEN); /// 隐藏
#endif
    }
}
#endif

static void pump_both_level_text_update(ui_manager_context_t *thiz)
{    
    if (level_info->force_update == false) {
        return;
    }

    if (level_info->title_text == NULL || !lv_obj_is_valid(level_info->title_text)) {
        level_info->title_text = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->title_text);
        lv_label_set_text(level_info->title_text, "Level");
        lv_obj_set_style_text_color(level_info->title_text, lv_color_hex(PUMP_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->title_text, &lv_font_oppo_sans_20_medium, 0);
        lv_obj_align(level_info->title_text, LV_ALIGN_TOP_LEFT, PUMP_LEVEL_TITLE_TEXT_X, PUMP_LEVEL_TITLE_TEXT_Y);
    }
}

static void pump_both_level_symbol_update(ui_manager_context_t *thiz, int8_t level_change)
{   
    uint8_t symbol_status = PUMP_BOTH_LEVEL_SYMBOL_NONE;
    if (level_change > 0) {
        symbol_status = PUMP_BOTH_LEVEL_SYMBOL_ADD;
    } else if (level_change < 0) {
        symbol_status = PUMP_BOTH_LEVEL_SYMBOL_SUB;
    } else {
        symbol_status = PUMP_BOTH_LEVEL_SYMBOL_NONE;
    }

    if (level_info->symbol_status == symbol_status && level_info->force_update == false) {
        return;
    }

    if (level_info->symbol == NULL || !lv_obj_is_valid(level_info->symbol)) {
        level_info->symbol = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->symbol);
        lv_obj_align(level_info->symbol, LV_ALIGN_TOP_LEFT, PUMP_LEVEL_SYMBOL_X, PUMP_LEVEL_SYMBOL_Y);
    }

    if (symbol_status == PUMP_BOTH_LEVEL_SYMBOL_ADD) {
        common_ui_img_resource_init(thiz, BASE_RES_ID_PUMP_LEVEL_ADD_ICON, &level_info->symbol_info);
        EXCEPTION_CHECK_VOID(level_info->symbol_info.data);
    } else if (symbol_status == PUMP_BOTH_LEVEL_SYMBOL_SUB) {
        common_ui_img_resource_init(thiz, BASE_RES_ID_PUMP_LEVEL_SUB_ICON, &level_info->symbol_info);
        EXCEPTION_CHECK_VOID(level_info->symbol_info.data);
    } else {
        level_info->symbol_status = symbol_status;    
        lv_obj_add_flag(level_info->symbol, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(level_info->symbol, LV_OBJ_FLAG_HIDDEN);
    lv_fs_path_ex_t symbol_path;
    lv_fs_make_path_from_buffer(&symbol_path, LV_FS_MEMFS_LETTER, 
        level_info->symbol_info.data, level_info->symbol_info.size, "baj");
    lv_image_set_src(level_info->symbol, (const void *)&symbol_path);
    level_info->symbol_status = symbol_status;
}

static int pump_left_level_circle_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (level_info->level_circle[0] == NULL || !lv_obj_is_valid(level_info->level_circle[0])) {
        level_info->level_circle[0] = lv_obj_create(level_info->level_arc[0]);
        lv_obj_remove_style_all(level_info->level_circle[0]);
        lv_obj_set_size(level_info->level_circle[0], PUMP_LEVEL_CIRCLE_SIZE, PUMP_LEVEL_CIRCLE_SIZE); /// 设置白点大小
        lv_obj_set_style_radius(level_info->level_circle[0], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(level_info->level_circle[0], lv_color_hex(PUMP_LEVEL_CIRCLE_COLOR), 0);
        lv_obj_set_style_bg_opa(level_info->level_circle[0], LV_OPA_COVER, 0);
        lv_obj_align(level_info->level_circle[0], LV_ALIGN_TOP_LEFT, PUMP_LEVEL_LEFT_CIRCLE_X, PUMP_LEVEL_LEFT_CIRCLE_Y);
    }

    return 0;
}

static int pump_left_level_text_update(ui_manager_context_t *thiz, uint8_t level, uint16_t end_angle)
{
    char level_text[4] = {0};
    if (level_info->level_text[0] == NULL || !lv_obj_is_valid(level_info->level_text[0])) {
        EXCEPTION_CHECK_INT(level_info->level_circle[0]);
        level_info->level_text[0] = lv_label_create(level_info->level_circle[0]);
        EXCEPTION_CHECK_INT(level_info->level_text[0]);
        lv_obj_set_style_text_color(level_info->level_text[0], lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->level_text[0], &lv_font_oppo_sans_24_medium, 0);
        lv_obj_align(level_info->level_text[0], LV_ALIGN_CENTER, 0, 0);
    }
    snprintf(level_text, sizeof(level_text), "%d", level);
    lv_label_set_text(level_info->level_text[0], level_text);

    return 0;
}

static int pump_left_level_arc_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (level_info->level_arc[0] == NULL || !lv_obj_is_valid(level_info->level_arc[0])) {
        level_info->level_arc[0] = lv_arc_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->level_arc[0]);
        lv_obj_set_size(level_info->level_arc[0], PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE); /// 设置弧线大小
        lv_obj_center(level_info->level_arc[0]); /// 设置弧线居中
        lv_arc_set_bg_angles(level_info->level_arc[0], 
            PUMP_LEFT_ARC_START_ANGLE, PUMP_LEFT_ARC_END_ANGLE);  /// 设置背景弧角度
        lv_arc_set_angles(level_info->level_arc[0], PUMP_LEFT_ARC_START_ANGLE, end_angle);  /// 设置前景弧角度
        lv_obj_set_style_arc_color(level_info->level_arc[0],
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(level_info->level_arc[0], PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(level_info->level_arc[0], LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧不透明度38%（97/255≈38%）
        lv_obj_set_style_arc_color(level_info->level_arc[0],
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(level_info->level_arc[0], PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(level_info->level_arc[0], LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧完全不透明
        lv_obj_remove_style(level_info->level_arc[0], NULL, LV_PART_KNOB); /// 去掉旋钮
    } else {
        lv_arc_set_angles(level_info->level_arc[0], PUMP_LEFT_ARC_START_ANGLE, end_angle);  /// 设置前景弧角度
    }
    return 0;
}

static void pump_left_level_update(ui_manager_context_t *thiz, uint8_t level, int8_t level_change)
{
    if (level_info->left_level_change == level_change && level_info->force_update == false) {
        return;
    }
    if (level_change >= 0) {
        level = level + level_change;
        if (level > PUMP_LEVEL_MAX) {
            level = PUMP_LEVEL_MAX;
        }
    } else {
        uint8_t abs_level_change = abs(level_change);
        level = level < abs_level_change ? 0 : level - abs_level_change;
    }
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

    if (pump_left_level_arc_update(thiz, end_angle) != 0) {
        return;
    }

    if (pump_left_level_circle_update(thiz, end_angle) != 0) {
        return;
    }

    if (pump_left_level_text_update(thiz, level, end_angle) != 0) {
        return;
    }
    level_info->left_level_change = level_change;
}

static int pump_right_level_circle_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (level_info->level_circle[1] == NULL || !lv_obj_is_valid(level_info->level_circle[1])) {
        level_info->level_circle[1] = lv_obj_create(level_info->level_arc[1]); // 关键：挂到 arc 下
        lv_obj_remove_style_all(level_info->level_circle[1]);
        lv_obj_set_size(level_info->level_circle[1], PUMP_LEVEL_CIRCLE_SIZE, PUMP_LEVEL_CIRCLE_SIZE); /// 设置白点大小
        lv_obj_set_style_radius(level_info->level_circle[1], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(level_info->level_circle[1], lv_color_hex(PUMP_LEVEL_CIRCLE_COLOR), 0);
        lv_obj_set_style_bg_opa(level_info->level_circle[1], LV_OPA_COVER, 0);
        lv_obj_align(level_info->level_circle[1], LV_ALIGN_TOP_LEFT, PUMP_LEVEL_RIGHT_CIRCLE_X, PUMP_LEVEL_RIGHT_CIRCLE_Y);
    }

    return 0;
}

static int pump_right_level_text_update(ui_manager_context_t *thiz, uint8_t level, uint16_t end_angle)
{
    char level_text[4] = {0};
    if (level_info->level_text[1] == NULL || !lv_obj_is_valid(level_info->level_text[1])) {
        EXCEPTION_CHECK_INT(level_info->level_circle[1]);
        level_info->level_text[1] = lv_label_create(level_info->level_circle[1]);
        EXCEPTION_CHECK_INT(level_info->level_text[1]);
        lv_obj_set_style_text_color(level_info->level_text[1], lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->level_text[1], &lv_font_oppo_sans_24_medium, 0);
        lv_obj_align(level_info->level_text[1], LV_ALIGN_CENTER, 0, 0);
    }
    snprintf(level_text, sizeof(level_text), "%d", level);
    lv_label_set_text(level_info->level_text[1], level_text);

    return 0;
}

static int pump_right_level_arc_update(ui_manager_context_t *thiz, uint16_t end_angle)
{
    if (level_info->level_arc[1] == NULL || !lv_obj_is_valid(level_info->level_arc[1])) {
        level_info->level_arc[1] = lv_arc_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->level_arc[1]);
        lv_obj_set_size(level_info->level_arc[1], PUMP_LEVEL_RING_SIZE, PUMP_LEVEL_RING_SIZE);
        lv_obj_center(level_info->level_arc[1]);
        lv_arc_set_bg_angles(level_info->level_arc[1], 
            PUMP_RIGHT_ARC_START_ANGLE, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置背景弧角度
        lv_arc_set_angles(level_info->level_arc[1], end_angle, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置前景弧角度
        lv_obj_set_style_arc_color(level_info->level_arc[1], 
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(level_info->level_arc[1], PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN); /// 背景弧宽度
        lv_obj_set_style_arc_opa(level_info->level_arc[1], LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧不透明度38%（97/255≈38%）
        lv_obj_set_style_arc_color(level_info->level_arc[1], 
            lv_color_hex(PUMP_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(level_info->level_arc[1], PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR); /// 前景弧宽度
        lv_obj_set_style_arc_opa(level_info->level_arc[1], LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧完全不透明
        lv_obj_remove_style(level_info->level_arc[1], NULL, LV_PART_KNOB); /// 去掉旋钮
    } else {
        lv_arc_set_angles(level_info->level_arc[1], end_angle, PUMP_RIGHT_ARC_END_ANGLE);  /// 设置前景弧角度
    }
    return 0;
}

static void pump_right_level_update(ui_manager_context_t *thiz, uint8_t level, int8_t level_change)
{
    if (level_info->right_level_change == level_change && level_info->force_update == false) {
        return;
    }

    if (level_change >= 0) {
        level = level + level_change;
        if (level > PUMP_LEVEL_MAX) {
            level = PUMP_LEVEL_MAX;
        }
    } else {
        uint8_t abs_level_change = abs(level_change);
        level = level < abs_level_change ? 0 : level - abs_level_change;
    }
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

    if (pump_right_level_arc_update(thiz, end_angle) != 0) {
        return;
    }

    if (pump_right_level_circle_update(thiz, end_angle) != 0) {
        return;
    }

    if (pump_right_level_text_update(thiz, level, end_angle) != 0) {
        return;
    }

    level_info->right_level_change = level_change;
}

static void pump_both_level_offset_text_update(ui_manager_context_t *thiz, int8_t level_change)
{
    if (level_info->level_change == level_change && level_info->force_update == false) {
        return;
    }

    if (level_info->level_offset_text == NULL || !lv_obj_is_valid(level_info->level_offset_text)) {
        level_info->level_offset_text = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->level_offset_text);
        lv_obj_set_style_text_color(level_info->level_offset_text, lv_color_hex(PUMP_LEVEL_OFFSET_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->level_offset_text, &lv_font_oppo_sans_72_bold, 0);
        lv_obj_set_width(level_info->level_offset_text, 120);
        lv_label_set_long_mode(level_info->level_offset_text, LV_LABEL_LONG_CLIP); /// 截断过长文本
        lv_obj_set_style_text_align(level_info->level_offset_text, LV_TEXT_ALIGN_CENTER, 0); /// 居中对齐
        lv_obj_align(level_info->level_offset_text, LV_ALIGN_CENTER, 0, 0);
    }
    char level_change_text[4] = {0};
    snprintf(level_change_text, sizeof(level_change_text), "%02d", abs(level_change));
    lv_label_set_text(level_info->level_offset_text, level_change_text);
    level_info->level_change = level_change;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void common_pump_both_level_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (level_display_info_t *)u_malloz(sizeof(level_display_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    pump_both_level_root_init();
    level_info->force_update = true;
    level_info->popup_ui_index = ID_POPUP_UI_NONE;
}

void common_pump_both_level_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    common_ui_img_resource_deinit(thiz, &level_info->symbol_info);
    ui_common_operation_tip_remove(thiz);
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int common_pump_both_level_ui_process(ui_manager_context_t *thiz, uint8_t *level, 
    int8_t level_change, uint16_t popup_ui_type)
{
    EXCEPTION_CHECK_INT(thiz && level_info && level_info->root && level);
    pump_both_level_root_bg_update(thiz);
    pump_both_level_text_update(thiz);
    pump_both_level_symbol_update(thiz, level_change);
    pump_both_level_offset_text_update(thiz, level_change);
    pump_left_level_update(thiz, level[0], level_change);
    pump_right_level_update(thiz, level[1], level_change);
    ui_common_operation_tip_update(thiz, popup_ui_type, level_info->root, &level_info->popup_ui_index);
    level_info->force_update = false;

    return 0;
}