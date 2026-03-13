/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "ui_img_define.h"
#include "ui_page_define.h"
#include "ui_manager.h"
#include "window_screen_pump.h"
#include "breast_pump_ui.h"

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_LEVEL_COMP_BG                        (0)
#define IS_SUPPORT_PUMP_LEVEL_COMP_BG                   (0)

//颜色定义
#define PUMP_LEVEL_CONTROL_TYPE_LABEL_COLOR                   0xB25962
#define PUMP_LEVEL_ROOT_BG_COLOR                              0xFFE0E5
#define PUMP_LEVEL_TITLE_TEXT_COLOR                           0x533129
#define PUMP_LEVEL_CONTROL_TYPE_TEXT_COLOR                     0xB25962
#define PUMP_LEVEL_TEXT_LEVEL_LABLE_COLOR                      0x533129


#define PUMP_LEVEL_TITLE_TEXT_X                           117  + 36
#define PUMP_LEVEL_TITLE_TEXT_Y                           96

#define PUMP_LEVEL_CONTROL_TYPE_LABLE1_X_L_R                         150
#define PUMP_LEVEL_CONTROL_TYPE_LABLE1_Y_L_R                         224
#define PUMP_LEVEL_CONTROL_TYPE_LABLE2_X_B                           186
#define PUMP_LEVEL_CONTROL_TYPE_LABLE2_Y_B                           224
#define PUMP_LEVEL_CONTROL_TYPE_LABLE1_X_B                           96
#define PUMP_LEVEL_CONTROL_TYPE_LABLE1_Y_B                           224
#define PUMP_LEVEL_TEXT_LEVEL_LABLE_X                                130+8
#define PUMP_LEVEL_TEXT_LEVEL_LABLE_Y                                136+8
#define PUMP_LEVEL_DIGIT_WIDTH                                       43      // 单个数字字符固定宽度
#define PUMP_LEVEL_DIGIT_SPACING                                      0       // 两个数字之间的间距



// 等级指示器配置
#define INDICATOR_SIZE                                  308
#define TOTAL_LEVEL                                     15      // 总等级


typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * level_image;                // 等级图片（1-15级）
    res_img_info_t level_image_info;        // 等级图片资源信息
    lv_obj_t * title_text;                        // 档位文字
    lv_obj_t * control_type_lable1;                // 控制类型标签
    lv_obj_t * control_type_lable_text1;            // 控制类型标签文字
    lv_obj_t * control_type_lable2;                // 控制类型标签
    lv_obj_t * control_type_lable_text2;            // 控制类型标签文字
    lv_obj_t * text_level_lable2;                    // 等级文字标签
    lv_obj_t * text_level_digit1;                    // 等级数字标签（十位）
    lv_obj_t * text_level_digit2;                    // 等级数字标签（个位）
    lv_obj_t * add_label;                    // 加号文字标签    
    res_img_info_t add_label_info;            // 加号文字标签资源信息
    uint8_t force_update;
    uint8_t left_level;
    uint8_t right_level;
    int8_t level_change;
    uint8_t control_type;
    uint8_t last_display_level;             // 上次显示的等级，用于避免重复刷新
} level_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LEVEL_COMP_BG
static PSRAM_DATA_LOC uint8_t setting_bg_data[] = {
    #include "png2txt/pump_level_bg_300x300.txt"
};
#endif

static level_display_info_t *level_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void pump_level_root_init(void)
{
    if (level_info->root == NULL || !lv_obj_is_valid(level_info->root)) {
        level_info->root = lv_obj_create(lv_screen_active());
        lv_obj_set_size(level_info->root, LV_PCT(100), LV_PCT(100));
        //设置前景色为浅粉，防止闪烁
        lv_obj_set_style_bg_color(level_info->root, lv_color_hex(PUMP_LEVEL_ROOT_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(level_info->root, 0, 0);
        lv_obj_set_style_pad_all(level_info->root, 0, 0);
        lv_obj_align(level_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }
}

#if IS_SUPPORT_PUMP_LEVEL_COMP_BG
static void pump_level_root_bg_update(ui_manager_context_t *thiz)
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
static void pump_level_root_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);
    
    if (level_info->force_update == false) {
        return;
    }

#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = thiz->get_lvgl_snap_shot(UI_SNAP_SHOT_PUMP_LEVEL_BG);
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

/**
 * @brief 创建等级指示器
 */
static void pump_level_indicator_create(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(parent);

    // 创建等级图片对象
    if (level_info->level_image == NULL || !lv_obj_is_valid(level_info->level_image)) {
        level_info->level_image = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(level_info->level_image);
        lv_obj_center(level_info->level_image);
    }
}

/**
 * @brief 更新等级指示器进度（参照pump_home_animation_update，使用图片代替圆弧）
 */
static void pump_level_arc_progress_update(ui_manager_context_t *thiz, int8_t level_change)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);
    
    // 创建指示器
    if (level_info->level_image == NULL || !lv_obj_is_valid(level_info->level_image)) {
        pump_level_indicator_create(thiz, level_info->root);
    }
    
    EXCEPTION_CHECK_VOID(level_info->level_image);
    /// 取绝对值
    uint8_t level = abs(level_change);
    // 边界保护：等级范围是0-15
    uint8_t display_level = (level > TOTAL_LEVEL) ? TOTAL_LEVEL : level;
    
    // 如果等级没有变化，不需要刷新
    if (level_info->last_display_level == display_level && !level_info->force_update) {
        return;
    }
    // 0级时隐藏图片
    if (display_level == 0) {
        //前景色恢复为白色，防止闪烁
        if (level_info->root != NULL && lv_obj_is_valid(level_info->root)) {
            lv_obj_set_style_bg_color(level_info->root, lv_color_hex(PUMP_LEVEL_ROOT_BG_COLOR), 0);
            lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);
        }
        //打开背景图片，防止闪烁
        if (level_info->background != NULL && lv_obj_is_valid(level_info->background)) {
            lv_obj_clear_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
        }
        if (level_info->level_image != NULL && lv_obj_is_valid(level_info->level_image)) {
            lv_obj_add_flag(level_info->level_image, LV_OBJ_FLAG_HIDDEN);
        }
        level_info->last_display_level = display_level;
        return;
    }
    
    // 显示图片（如果之前被隐藏）
    if (level_info->level_image != NULL && lv_obj_is_valid(level_info->level_image)) {
        lv_obj_clear_flag(level_info->level_image, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 计算资源ID：RESOURCE_ID_PUMP_LEVEL_START + (display_level - 1)
    // 例如：level 1 -> RESOURCE_ID_PUMP_LEVEL_START, level 15 -> RESOURCE_ID_PUMP_LEVEL_START + 14
    uint16_t resource_id = RESOURCE_ID_PUMP_LEVEL_START + (display_level - 1);
    if (resource_id > RESOURCE_ID_PUMP_LEVEL_END) {
        resource_id = RESOURCE_ID_PUMP_LEVEL_END;
    }
    
    // 初始化图片资源（参照pump_home_animation_update）
    common_ui_img_resource_init(thiz, resource_id, &level_info->level_image_info);
    EXCEPTION_CHECK_VOID(level_info->level_image_info.data);

    //前景色恢复为白色，防止闪烁
    if (level_info->root != NULL && lv_obj_is_valid(level_info->root)) {
        lv_obj_set_style_bg_color(level_info->root, lv_color_hex(PUMP_LEVEL_ROOT_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);
    }
    //打开背景图片，防止闪烁
    if (level_info->background != NULL && lv_obj_is_valid(level_info->background)) {
        lv_obj_clear_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
    }
   
    // 设置图片路径并显示
    lv_fs_path_ex_t level_img_path;
    lv_fs_make_path_from_buffer(&level_img_path, LV_FS_MEMFS_LETTER, 
        level_info->level_image_info.data, level_info->level_image_info.size, "baj");
    lv_image_set_src(level_info->level_image, (const void *)&level_img_path);
    
    // 记录当前显示的等级
    level_info->last_display_level = display_level;
}

static void pump_level_title_text_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);
    
    if (level_info->title_text == NULL || !lv_obj_is_valid(level_info->title_text)) {
        level_info->title_text = lv_label_create(level_info->root);
    }
    lv_label_set_text(level_info->title_text, "Level");
    lv_obj_set_style_text_color(level_info->title_text, lv_color_hex(PUMP_LEVEL_TITLE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(level_info->title_text, &lv_font_oppo_sans_24_medium, 0);
    lv_obj_align(level_info->title_text, LV_ALIGN_TOP_LEFT, PUMP_LEVEL_TITLE_TEXT_X, PUMP_LEVEL_TITLE_TEXT_Y);
}

static void pump_level_control_type_lable_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);

    /* 2. 创建胶囊状（圆角矩形）背景 (替换之前的圆形) */
    if (level_info->control_type_lable1 == NULL || !lv_obj_is_valid(level_info->control_type_lable1)) {
        level_info->control_type_lable1 = lv_obj_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->control_type_lable1);
    }

    if (level_info->control_type_lable_text1 == NULL || !lv_obj_is_valid(level_info->control_type_lable_text1)) {
        level_info->control_type_lable_text1 = lv_label_create(level_info->control_type_lable1);
        EXCEPTION_CHECK_VOID(level_info->control_type_lable_text1);
    }

    if (level_info->control_type_lable2 == NULL || !lv_obj_is_valid(level_info->control_type_lable2)) {
        level_info->control_type_lable2 = lv_obj_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->control_type_lable2);
        
    }

    if (level_info->control_type_lable_text2 == NULL || !lv_obj_is_valid(level_info->control_type_lable_text2)) {
        level_info->control_type_lable_text2 = lv_label_create(level_info->control_type_lable2);
        EXCEPTION_CHECK_VOID(level_info->control_type_lable_text2);
    }

    if (level_info->control_type == BREAST_PUMP_CONTROL_TYPE_BILATERAL) {
        /// 双侧模式下，两个标签都显示
        // 胶囊状：宽度大于高度，圆角半径为高度的一半
        uint16_t capsule_width = 78;   // 胶囊宽度
        uint16_t capsule_height = 40; // 胶囊高度
        uint16_t capsule_radius = capsule_height / 2;  // 圆角半径 = 高度的一半
        
        // 设置标签1（左侧，显示"L"）
        lv_obj_set_size(level_info->control_type_lable1, capsule_width, capsule_height);
        lv_obj_align(level_info->control_type_lable1, LV_ALIGN_TOP_LEFT, 
            PUMP_LEVEL_CONTROL_TYPE_LABLE1_X_B, PUMP_LEVEL_CONTROL_TYPE_LABLE1_Y_B);
        lv_obj_clear_flag(level_info->control_type_lable1, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(level_info->control_type_lable1, lv_color_hex(PUMP_LEVEL_CONTROL_TYPE_LABEL_COLOR), 0);
        lv_obj_set_style_bg_opa(level_info->control_type_lable1, LV_OPA_20, 0);//透明20%
        lv_obj_set_style_radius(level_info->control_type_lable1, capsule_radius, 0);
        lv_obj_set_style_border_opa(level_info->control_type_lable1, LV_OPA_TRANSP, 0);
        lv_obj_clear_flag(level_info->control_type_lable1, LV_OBJ_FLAG_HIDDEN);
        
        // 设置标签1文本：L + 左侧等级（格式化为两位数）
        char label1_text[8];
        char level_str[4];
        if (level_info->left_level < 10) {  
            snprintf(level_str, sizeof(level_str), "0%d", level_info->left_level);
        } else {
            snprintf(level_str, sizeof(level_str), "%d", level_info->left_level);
        }
        snprintf(label1_text, sizeof(label1_text), "L %s", level_str);
        lv_label_set_text(level_info->control_type_lable_text1, label1_text);
        // 设置固定宽度，避免等级变化时位置跳动
        lv_obj_set_width(level_info->control_type_lable_text1, 78);  // 固定宽度，确保"L 00"到"L 15"都能完整显示
        lv_label_set_long_mode(level_info->control_type_lable_text1, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(level_info->control_type_lable_text1, LV_TEXT_ALIGN_CENTER, 0);  // 文本居中对齐
        lv_obj_set_style_bg_opa(level_info->control_type_lable_text1, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(level_info->control_type_lable_text1, LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_opa(level_info->control_type_lable_text1, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(level_info->control_type_lable_text1, 0, 0);
        lv_obj_center(level_info->control_type_lable_text1);
        lv_obj_set_style_text_color(level_info->control_type_lable_text1, lv_color_hex(PUMP_LEVEL_CONTROL_TYPE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->control_type_lable_text1, &lv_font_oppo_sans_24_medium, 0);
        lv_obj_align(level_info->control_type_lable_text1, LV_ALIGN_CENTER, 0, 3);
        lv_obj_clear_flag(level_info->control_type_lable_text1, LV_OBJ_FLAG_HIDDEN);
        
        // 设置标签2（右侧，显示"R"）
        lv_obj_set_size(level_info->control_type_lable2, capsule_width, capsule_height);
        lv_obj_align(level_info->control_type_lable2, LV_ALIGN_TOP_LEFT, 
            PUMP_LEVEL_CONTROL_TYPE_LABLE2_X_B, PUMP_LEVEL_CONTROL_TYPE_LABLE2_Y_B);
        lv_obj_clear_flag(level_info->control_type_lable2, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(level_info->control_type_lable2, lv_color_hex(PUMP_LEVEL_CONTROL_TYPE_LABEL_COLOR), 0);
        lv_obj_set_style_bg_opa(level_info->control_type_lable2, LV_OPA_20, 0);//透明20%
        lv_obj_set_style_radius(level_info->control_type_lable2, capsule_radius, 0);
        lv_obj_set_style_border_opa(level_info->control_type_lable2, LV_OPA_TRANSP, 0);
        lv_obj_clear_flag(level_info->control_type_lable2, LV_OBJ_FLAG_HIDDEN);
        
        // 设置标签2文本：R + 右侧等级（格式化为两位数）
        char label2_text[8];
        char level_str2[4];
        if (level_info->right_level < 10) {
            snprintf(level_str2, sizeof(level_str2), "0%d", level_info->right_level);
        } else {
            snprintf(level_str2, sizeof(level_str2), "%d", level_info->right_level);
        }
        snprintf(label2_text, sizeof(label2_text), "R %s", level_str2);
        lv_label_set_text(level_info->control_type_lable_text2, label2_text);
        // 设置固定宽度，避免等级变化时位置跳动
        lv_obj_set_width(level_info->control_type_lable_text2, 78);  // 固定宽度，确保"R 00"到"R 15"都能完整显示
        lv_label_set_long_mode(level_info->control_type_lable_text2, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(level_info->control_type_lable_text2, LV_TEXT_ALIGN_CENTER, 0);  // 文本居中对齐
        lv_obj_set_style_bg_opa(level_info->control_type_lable_text2, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(level_info->control_type_lable_text2, LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_opa(level_info->control_type_lable_text2, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(level_info->control_type_lable_text2, 0, 0);
        lv_obj_center(level_info->control_type_lable_text2);
        lv_obj_set_style_text_color(level_info->control_type_lable_text2, lv_color_hex(PUMP_LEVEL_CONTROL_TYPE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->control_type_lable_text2, &lv_font_oppo_sans_24_medium, 0);
        lv_obj_align(level_info->control_type_lable_text2, LV_ALIGN_CENTER, 0, 3);
        lv_obj_clear_flag(level_info->control_type_lable_text2, LV_OBJ_FLAG_HIDDEN);
        
    } else {
        /// 单侧模式下，只显示一个标签
        // 胶囊状：宽度大于高度，圆角半径为高度的一半
        uint16_t capsule_width = 60;   // 胶囊宽度
        uint16_t capsule_height = 40; // 胶囊高度
        uint16_t capsule_radius = capsule_height / 2;  // 圆角半径 = 高度的一半
        
        lv_obj_set_size(level_info->control_type_lable1, capsule_width, capsule_height);
        /// 设置坐标
        lv_obj_align(level_info->control_type_lable1, LV_ALIGN_TOP_LEFT, PUMP_LEVEL_CONTROL_TYPE_LABLE1_X_L_R, PUMP_LEVEL_CONTROL_TYPE_LABLE1_Y_L_R);
        
        lv_obj_clear_flag(level_info->control_type_lable1, LV_OBJ_FLAG_SCROLLABLE);   /// 不要滚动条
        // 设置浅粉色背景 (RGB: 255, 224, 229 精准匹配浅粉)
        lv_obj_set_style_bg_color(level_info->control_type_lable1, lv_color_hex(PUMP_LEVEL_CONTROL_TYPE_LABEL_COLOR), 0);
        lv_obj_set_style_bg_opa(level_info->control_type_lable1, LV_OPA_20, 0);//透明20%
        // 设置圆角半径为高度的一半，形成胶囊/椭圆效果
        lv_obj_set_style_radius(level_info->control_type_lable1, capsule_radius, 0);
        lv_obj_set_style_border_opa(level_info->control_type_lable1, LV_OPA_TRANSP, 0);  // 无边框

        // 设置标签文本：L/R 文本
        if (level_info->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
            lv_label_set_text(level_info->control_type_lable_text1, "L");
        } else if (level_info->control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
            lv_label_set_text(level_info->control_type_lable_text1, "R");
        } else {
            LOGE("invalid control type: %d", level_info->control_type);
        }
        
        // 设置文本标签样式：背景透明，无边框，无阴影
        lv_obj_set_style_bg_opa(level_info->control_type_lable_text1, LV_OPA_TRANSP, 0);  // 背景透明
        lv_obj_set_style_border_opa(level_info->control_type_lable_text1, LV_OPA_TRANSP, 0);  // 无边框
        lv_obj_set_style_shadow_opa(level_info->control_type_lable_text1, LV_OPA_TRANSP, 0);  // 无阴影
        lv_obj_set_style_pad_all(level_info->control_type_lable_text1, 0, 0);  // 无内边距
        
        lv_obj_center(level_info->control_type_lable_text1);           // 居中显示
        lv_obj_set_style_text_color(level_info->control_type_lable_text1, lv_color_hex(PUMP_LEVEL_CONTROL_TYPE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->control_type_lable_text1, &lv_font_oppo_sans_24_medium, 0);
        lv_obj_align(level_info->control_type_lable_text1, LV_ALIGN_CENTER, 0, 3);
        //关闭第二个标签 隐藏
        lv_obj_add_flag(level_info->control_type_lable2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(level_info->control_type_lable_text2, LV_OBJ_FLAG_HIDDEN);
    }

}

/**
 * @brief 设置固定宽度的数字标签样式
 */
static void pump_level_digit_label_setup(lv_obj_t *label, uint16_t x, uint16_t y)
{
    lv_obj_set_size(label, PUMP_LEVEL_DIGIT_WIDTH, 72);  // 固定宽度，高度匹配字体
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);  // 居中对齐，确保字符居中
    lv_obj_set_style_text_color(label, lv_color_hex(PUMP_LEVEL_TEXT_LEVEL_LABLE_COLOR), 0);
    lv_obj_set_style_text_font(label, &lv_font_oppo_sans_72_bold, 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, x, y);
}

static void pump_level_text_level_lable_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);

    if (level_info->text_level_lable2 == NULL || !lv_obj_is_valid(level_info->text_level_lable2)) {
        level_info->text_level_lable2 = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->text_level_lable2);
    }
    // 创建两个独立的数字标签（十位和个位）
    if (level_info->text_level_digit1 == NULL || !lv_obj_is_valid(level_info->text_level_digit1)) {
        level_info->text_level_digit1 = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->text_level_digit1);
    }
    if (level_info->text_level_digit2 == NULL || !lv_obj_is_valid(level_info->text_level_digit2)) {
        level_info->text_level_digit2 = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->text_level_digit2);
    }
    if (level_info->add_label == NULL || !lv_obj_is_valid(level_info->add_label)) {
        level_info->add_label = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->add_label);
    }

    if (level_info->control_type == BREAST_PUMP_CONTROL_TYPE_BILATERAL) {
        // 双侧模式：使用两个独立标签显示level_change值（十位和个位）
        uint8_t level_value = abs(level_info->level_change);
        uint8_t tens_digit = level_value / 10;      // 十位数字
        uint8_t ones_digit = level_value % 10;      // 个位数字
        
        // 显示十位数字（固定宽度）
        char tens_char[2] = { '0' + tens_digit, '\0' };
        lv_label_set_text(level_info->text_level_digit1, tens_char);
        pump_level_digit_label_setup(level_info->text_level_digit1, 
            PUMP_LEVEL_TEXT_LEVEL_LABLE_X, PUMP_LEVEL_TEXT_LEVEL_LABLE_Y);
        lv_obj_clear_flag(level_info->text_level_digit1, LV_OBJ_FLAG_HIDDEN);
        
        // 显示个位数字（固定宽度）
        char ones_char[2] = { '0' + ones_digit, '\0' };
        lv_label_set_text(level_info->text_level_digit2, ones_char);
        pump_level_digit_label_setup(level_info->text_level_digit2, 
            PUMP_LEVEL_TEXT_LEVEL_LABLE_X + PUMP_LEVEL_DIGIT_WIDTH + PUMP_LEVEL_DIGIT_SPACING, 
            PUMP_LEVEL_TEXT_LEVEL_LABLE_Y);
        lv_obj_clear_flag(level_info->text_level_digit2, LV_OBJ_FLAG_HIDDEN);
        

        ///lable2 显示 +文本 隐藏
        //lv_label_set_text(level_info->text_level_lable2, "+");
        //lv_obj_set_style_text_color(level_info->text_level_lable2, lv_color_hex(PUMP_LEVEL_TEXT_LEVEL_LABLE_COLOR), 0);
        //lv_obj_set_style_text_font(level_info->text_level_lable2, &lv_font_oppo_sans_24_medium, 0);
        //lv_obj_align(level_info->text_level_lable2, LV_ALIGN_TOP_LEFT, 110, 168);
        lv_obj_add_flag(level_info->text_level_lable2, LV_OBJ_FLAG_HIDDEN);

        ///add_label 显示加号图标（参照背景图标代码）
        //LOG_UI("level_change: %d", level_info->level_change);
        common_ui_img_resource_init(thiz, level_info->level_change >= 0 ? RESOURCE_ID_PUMP_LEVEL_ADD_LABEL_ICON : RESOURCE_ID_PUMP_LEVEL_SUB_LABEL_ICON, &level_info->add_label_info);
        EXCEPTION_CHECK_VOID(level_info->add_label_info.data);
        
        lv_fs_path_ex_t add_img_path;
        lv_fs_make_path_from_buffer(&add_img_path, LV_FS_MEMFS_LETTER, 
            level_info->add_label_info.data, level_info->add_label_info.size, "baj");
        lv_image_set_src(level_info->add_label, (const void *)&add_img_path);
        lv_obj_align(level_info->add_label, LV_ALIGN_TOP_LEFT, 110, 162);
        lv_obj_clear_flag(level_info->add_label, LV_OBJ_FLAG_HIDDEN);


    } else {

        ///add_label图片标签   隐藏
        lv_obj_add_flag(level_info->add_label, LV_OBJ_FLAG_HIDDEN);
        // 单侧模式：使用两个独立标签显示等级值（十位和个位）
        uint8_t display_level = (level_info->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) ? 
                                level_info->left_level : level_info->right_level;
        uint8_t tens_digit = display_level / 10;      // 十位数字
        uint8_t ones_digit = display_level % 10;      // 个位数字
        
        // 显示十位数字（固定宽度）
        char tens_char[2] = { '0' + tens_digit, '\0' };
        lv_label_set_text(level_info->text_level_digit1, tens_char);
        pump_level_digit_label_setup(level_info->text_level_digit1, 
            PUMP_LEVEL_TEXT_LEVEL_LABLE_X, PUMP_LEVEL_TEXT_LEVEL_LABLE_Y);
        lv_obj_clear_flag(level_info->text_level_digit1, LV_OBJ_FLAG_HIDDEN);
        
        // 显示个位数字（固定宽度）
        char ones_char[2] = { '0' + ones_digit, '\0' };
        lv_label_set_text(level_info->text_level_digit2, ones_char);
        pump_level_digit_label_setup(level_info->text_level_digit2, 
            PUMP_LEVEL_TEXT_LEVEL_LABLE_X + PUMP_LEVEL_DIGIT_WIDTH + PUMP_LEVEL_DIGIT_SPACING, 
            PUMP_LEVEL_TEXT_LEVEL_LABLE_Y);
        lv_obj_clear_flag(level_info->text_level_digit2, LV_OBJ_FLAG_HIDDEN);
             
        // 第二个标签显示/15 文本
        lv_label_set_text(level_info->text_level_lable2, "/15");
        lv_obj_set_style_text_color(level_info->text_level_lable2, lv_color_hex(PUMP_LEVEL_TEXT_LEVEL_LABLE_COLOR), 0);
        lv_obj_set_style_text_font(level_info->text_level_lable2, &lv_font_oppo_sans_20_medium, 0);
        lv_obj_align(level_info->text_level_lable2, LV_ALIGN_TOP_LEFT, PUMP_LEVEL_TEXT_LEVEL_LABLE_X + 86, PUMP_LEVEL_TEXT_LEVEL_LABLE_Y + 38);
         //修改字体透明度为50%
        lv_obj_set_style_text_opa(level_info->text_level_lable2, LV_OPA_50, LV_PART_MAIN);
        ///text_level_lable2 显示lable
        lv_obj_clear_flag(level_info->text_level_lable2, LV_OBJ_FLAG_HIDDEN);

    }

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
        // 初始化所有指针为NULL
        memset(level_info, 0, sizeof(level_display_info_t));
        level_info->last_display_level = 0xff; //初始化为无效值
        level_info->level_change = 0; //初始化为0
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
    common_ui_img_resource_deinit(thiz, &level_info->level_image_info);
    common_ui_img_resource_deinit(thiz, &level_info->add_label_info);
    /// 安全删除所有LVGL对象
    LVGL_OBJ_DELETE(level_info->background);
    LVGL_OBJ_DELETE(level_info->level_image);
    LVGL_OBJ_DELETE(level_info->add_label);
    LVGL_OBJ_DELETE(level_info->text_level_digit1);
    LVGL_OBJ_DELETE(level_info->text_level_digit2);
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int ui_display_pump_level_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(level_info);
    EXCEPTION_CHECK_INT(level_info->root);
    
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    EXCEPTION_CHECK_INT(proc->motor);

    level_info->left_level = proc->setting.level[0];
    level_info->right_level = proc->setting.level[1];
    level_info->control_type = proc->info.control_type;
    level_info->level_change = proc->level_change;  // 测试值，实际应从业务逻辑获取
    
    // 根据控制类型计算当前显示等级
    int8_t display_level;
    if (level_info->control_type == BREAST_PUMP_CONTROL_TYPE_BILATERAL) {
        display_level = level_info->level_change;
    } else {
        display_level = (level_info->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) ? 
                        level_info->left_level : level_info->right_level;
    }
    
    // 更新UI
    pump_level_root_bg_update(thiz);
    pump_level_arc_progress_update(thiz, display_level);  // 使用实际计算的等级
    pump_level_title_text_update(thiz);
    pump_level_control_type_lable_update(thiz);
    pump_level_text_level_lable_update(thiz);
    level_info->force_update = false;
    
    // FPS统计
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> pump level fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    
    return 0;
}