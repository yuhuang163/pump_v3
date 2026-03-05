/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "src/misc/lv_color.h"
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

#define IS_SUPPORT_GUIDE_COMP_BG                                (0)
#define GUIDE_QRCODE_BG_COLOR                                   0xFFFFFF
#define GUIDE_QRCODE_FG_COLOR                                   0x000000
#define GUIDE_QRCODE_BORDER_COLOR                               (0xFFF2F3)  // 0xA6A6A6
#define GUIDE_IMG_NUM                                           (2)
#define GUIDE_TEXT_NUM                                          (3)
/// 箭头图标和文本框的偏移量
#define GUIDE_ARROW_ICON_X_OFFSET                               (168)
#define GUIDE_ARROW_ICON_Y_OFFSET                               (304)
#define GUIDE_ARROW_TEXT_X_OFFSET                               (122 + 1)
#define GUIDE_ARROW_TEXT_Y_OFFSET                               (328)
#define GUIDE_ARROW_TEXT_END_X_OFFSET                           (150 + 3)
#define GUIDE_ARROW_TEXT_END_Y_OFFSET                           (328)
#define GUIDE_TEXT_COLOR                                        (0xB25962)
#define GUIDE_BIND_TEXT_COLOR                                   (0x6A5A5B)

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    lv_obj_t * qrcode_container;
    lv_obj_t * qrcode;
    #if IS_SUPPORT_GUIDE_COMP_BG
    lv_obj_t * backgroud;
    #endif
    lv_obj_t * arrow; /// 箭头
    lv_obj_t * arrow_text; /// 箭头文本框
    lv_obj_t * img[GUIDE_IMG_NUM];
    lv_obj_t * text[GUIDE_TEXT_NUM];
    res_img_info_t arrow_info;
    res_img_info_t img_info[GUIDE_IMG_NUM];
    uint8_t force_update;
    uint8_t page_index;
} guide_display_info_t;

typedef struct {
    const char *text;
    const lv_font_t *font;
    uint16_t text_x;
    uint16_t text_y;
    lv_opa_t text_opa;
} ui_guide_text_info_t;

typedef struct {
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
} ui_guide_img_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static guide_display_info_t *guide_info;

const static char *bind_qr_str[] = {
    "https://lvgl.io",
};

#if IS_SUPPORT_GUIDE_COMP_BG
static PSRAM_DATA_LOC uint8_t guide_bg_data[] = {
    // #include "comparison_png2txt/guide_home_360x360.txt"
    // #include "comparison_png2txt/guide_start_pump_360x360.txt"
    // #include "comparison_png2txt/guide_setting_page_360x360.txt"
    // #include "comparison_png2txt/guide_stop_pump_360x360.txt"
    // #include "comparison_png2txt/guide_mode_select_360x360.txt"
    #include "comparison_png2txt/guide_bind_360x360.txt"
};
#endif

/// 首页
const static ui_guide_img_info_t guide_home_img_infos[] = {
    {RESOURCE_ID_GUIDE_KN_ABB_1, 57, 122}
};

const static ui_guide_text_info_t guide_home_text_infos[] = {
    {"Pumping  Trip", &lv_font_oppo_sans_20_medium, 120 + 1, 38 + 16, LV_OPA_COVER},
    {"Turn right to start", &lv_font_oppo_sans_14_medium, 118 + 8, 76 + 8, LV_OPA_70}
};

/// 开始泵奶
const static ui_guide_img_info_t guide_start_pump_img_infos[] = {
    {RESOURCE_ID_GUIDE_PUMPING_START_ABB_ICON, 87, 131}
};

const static ui_guide_text_info_t guide_start_pump_text_infos[] = {
    {"Regular  Pumping", &lv_font_oppo_sans_20_medium, 102, 48 + 5, LV_OPA_COVER},
    {"2 min Stimulation  +  20 min Expression for\n most moms.                               ", 
    &lv_font_oppo_sans_14_medium, 48 + 2, 80 + 4, LV_OPA_70},
    {"Press to start", &lv_font_oppo_sans_14_bold, 128 + 49, 88 + 13, LV_OPA_70}
};

/// 设置泵奶
const static ui_guide_img_info_t guide_setting_pump_img_infos[] = {
    {RESOURCE_ID_GUIDE_PUMPING_ABB_ICON, 100, 32 + 1},
    {RESOURCE_ID_GUIDE_LRM_ICON, 116, 248}
};

const static ui_guide_text_info_t guide_setting_pump_text_infos[] = {
    {"Adjust suction and speed per side by\n pressing buttons or knobs, or switch modes",
    &lv_font_oppo_sans_14_medium, 40 + 2, 204 + 4, LV_OPA_70},
};

/// 停止泵奶
const static ui_guide_img_info_t guide_stop_pump_img_infos[] = {
    {RESOURCE_ID_GUIDE_PUMPING_ABB_ICON, 100, 32},
    {RESOURCE_ID_GUIDE_PLAYPAUSE_ICON, 164, 248}
};

const static ui_guide_text_info_t guide_stop_pump_text_infos[] = {
    {"Press and hold for 3  seconds to stop\n pumping", 
    &lv_font_oppo_sans_14_medium, 60 + 6, 204 + 4, LV_OPA_70},
};

/// 泵奶模式选择
const static ui_guide_img_info_t guide_pump_mode_select_img_infos[] = {
    {RESOURCE_ID_GUIDE_KN_ABB_0, 74, 47},
    {RESOURCE_ID_GUIDE_MODESTAR_ICON, 140, 248}
};

const static ui_guide_text_info_t guide_pump_mode_select_text_infos[] = {
    {"More pumping options:3 modes, 6\nprograms.                                             ", 
    &lv_font_oppo_sans_14_medium, 66 + 4, 204 + 4, LV_OPA_70},
    {"Turn right to switch", &lv_font_oppo_sans_14_bold, 101 + 50, 224 + 2, LV_OPA_70}
};

/// 绑定引导
const static ui_guide_text_info_t guide_bind_guide_text_infos[] = {
    {"Scan to connect for tracking,  remote\ncontrol, custom modes... ",
    &lv_font_oppo_sans_14_medium, 66, 220 + 8, LV_OPA_COVER}
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_guide_root_init(void)
{
    if (guide_info->root == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(guide_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(guide_info->root, lv_color_white(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(guide_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(guide_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(guide_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(guide_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

#if IS_SUPPORT_GUIDE_COMP_BG
static void ui_guide_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (guide_info->backgroud == NULL || !lv_obj_is_valid(guide_info->backgroud)) {
        guide_info->backgroud = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(guide_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, guide_bg_data,
            sizeof(guide_bg_data), "png");
        lv_image_set_src(guide_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(guide_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else

static void ui_guide_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, guide_info->root);
}
#endif

static void guide_common_img_update(ui_manager_context_t *thiz)
{
    if (guide_info->arrow == NULL || !lv_obj_is_valid(guide_info->arrow)) {
        guide_info->arrow = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->arrow);
    }
    common_ui_img_resource_init(thiz, RESOURCE_ID_GUIDE_ARROW_ICON, &guide_info->arrow_info);
    EXCEPTION_CHECK_VOID(guide_info->arrow_info.data);
    lv_fs_path_ex_t arrow_path;
    lv_fs_make_path_from_buffer(&arrow_path,
                                LV_FS_MEMFS_LETTER,
                                guide_info->arrow_info.data,
                                guide_info->arrow_info.size,
                                "baj");
    lv_image_set_src(guide_info->arrow, (const void *)&arrow_path);
    lv_obj_align(guide_info->arrow, LV_ALIGN_TOP_LEFT, GUIDE_ARROW_ICON_X_OFFSET, GUIDE_ARROW_ICON_Y_OFFSET);
}

static void guide_common_text_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (guide_info->arrow_text == NULL || !lv_obj_is_valid(guide_info->arrow_text)) {
        guide_info->arrow_text = lv_label_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->arrow_text);
    }
    char *text_str = NULL;
    lv_opa_t text_opa = LV_OPA_COVER;
    uint16_t text_x = 0, text_y = 0;
    if (page_index == GUIDE_PAGE_BIND_GUIDE) {
        text_str = "End  Trip";
        text_x = GUIDE_ARROW_TEXT_END_X_OFFSET;
        text_y = GUIDE_ARROW_TEXT_END_Y_OFFSET;
    } else {
        text_str = "Press down to exit";
        text_x = GUIDE_ARROW_TEXT_X_OFFSET;
        text_y = GUIDE_ARROW_TEXT_Y_OFFSET;
        text_opa = LV_OPA_70;
    }
    lv_label_set_text(guide_info->arrow_text, text_str);
    lv_obj_set_style_text_color(guide_info->arrow_text, lv_color_hex(GUIDE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(guide_info->arrow_text, &lv_font_oppo_sans_14_medium, 0);  /// 设置字体
    lv_obj_set_style_text_line_space(guide_info->arrow_text, 7, 0);  /// 设置行间距（像素）
    lv_obj_set_style_text_align(guide_info->arrow_text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_set_style_text_opa(guide_info->arrow_text, text_opa, 0);  /// 设置文本透明度
    lv_obj_align(guide_info->arrow_text, LV_ALIGN_TOP_LEFT, text_x, text_y);
}

static void guide_ui_home_update(ui_manager_context_t *thiz)
{
    for (int i = 0; i < LENGTH_OF_ARRAY(guide_home_img_infos); i++) {
        if (guide_info->img[i] == NULL || !lv_obj_is_valid(guide_info->img[i])) {
            guide_info->img[i] = lv_image_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->img[i]);
        } else {
            lv_obj_clear_flag(guide_info->img[i], LV_OBJ_FLAG_HIDDEN);
        }
        common_ui_img_resource_init(thiz, guide_home_img_infos[i].img_id, &guide_info->img_info[i]);
        EXCEPTION_CHECK_VOID(guide_info->img_info[i].data);
        lv_fs_path_ex_t img_path;
        lv_fs_make_path_from_buffer(&img_path,
                                    LV_FS_MEMFS_LETTER,
                                    guide_info->img_info[i].data,   
                                    guide_info->img_info[i].size,
                                    "baj");
        lv_image_set_src(guide_info->img[i], (const void *)&img_path);
        lv_obj_align(guide_info->img[i], LV_ALIGN_TOP_LEFT, guide_home_img_infos[i].icon_x, guide_home_img_infos[i].icon_y);
    }

    for (int i = 0; i < LENGTH_OF_ARRAY(guide_home_text_infos); i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        } else {
            lv_obj_clear_flag(guide_info->text[i], LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text(guide_info->text[i], guide_home_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(GUIDE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_home_text_infos[i].font, 0);
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_opa(guide_info->text[i], guide_home_text_infos[i].text_opa, 0);
        lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_home_text_infos[i].text_x, guide_home_text_infos[i].text_y);
    }
}

static void guide_ui_start_pump_update(ui_manager_context_t *thiz)
{
    for (int i = 0; i < LENGTH_OF_ARRAY(guide_start_pump_img_infos); i++) {
        if (guide_info->img[i] == NULL || !lv_obj_is_valid(guide_info->img[i])) {
            guide_info->img[i] = lv_image_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->img[i]);
        } else {
            lv_obj_clear_flag(guide_info->img[i], LV_OBJ_FLAG_HIDDEN);
        }
        common_ui_img_resource_init(thiz, guide_start_pump_img_infos[i].img_id,
            &guide_info->img_info[i]);
        EXCEPTION_CHECK_VOID(guide_info->img_info[i].data);
        lv_fs_path_ex_t img_path;
        lv_fs_make_path_from_buffer(&img_path,
                                    LV_FS_MEMFS_LETTER,
                                    guide_info->img_info[i].data,   
                                    guide_info->img_info[i].size,
                                    "baj");
        lv_image_set_src(guide_info->img[i], (const void *)&img_path);
        lv_obj_align(guide_info->img[i], LV_ALIGN_TOP_LEFT, guide_start_pump_img_infos[i].icon_x,
            guide_start_pump_img_infos[i].icon_y);
    }

    for (int i = 0; i < LENGTH_OF_ARRAY(guide_start_pump_text_infos); i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        } else {
            lv_obj_clear_flag(guide_info->text[i], LV_OBJ_FLAG_HIDDEN);
        }   
        lv_label_set_text(guide_info->text[i], guide_start_pump_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(GUIDE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_start_pump_text_infos[i].font, 0);
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0); /// 设置行间距（像素）
        // lv_obj_set_style_text_letter_space(guide_info->text[i], 1, 0); /// 设置字间距（像素）
        lv_obj_set_style_text_opa(guide_info->text[i], guide_start_pump_text_infos[i].text_opa, 0);
        lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_start_pump_text_infos[i].text_x,
            guide_start_pump_text_infos[i].text_y);
    }
}

static void guide_ui_setting_pump_update(ui_manager_context_t *thiz)
{
    for (int i = 0; i < LENGTH_OF_ARRAY(guide_setting_pump_img_infos); i++) {
        if (guide_info->img[i] == NULL || !lv_obj_is_valid(guide_info->img[i])) {
            guide_info->img[i] = lv_image_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->img[i]);
        } else {
            lv_obj_clear_flag(guide_info->img[i], LV_OBJ_FLAG_HIDDEN);
        }
        common_ui_img_resource_init(thiz, guide_setting_pump_img_infos[i].img_id,
            &guide_info->img_info[i]);
        EXCEPTION_CHECK_VOID(guide_info->img_info[i].data);
        lv_fs_path_ex_t img_path;
        lv_fs_make_path_from_buffer(&img_path,
                                    LV_FS_MEMFS_LETTER,
                                    guide_info->img_info[i].data,   
                                    guide_info->img_info[i].size,
                                    "baj");
        lv_image_set_src(guide_info->img[i], (const void *)&img_path);
        lv_obj_align(guide_info->img[i], LV_ALIGN_TOP_LEFT, guide_setting_pump_img_infos[i].icon_x,
            guide_setting_pump_img_infos[i].icon_y);
    }

    for (int i = 0; i < LENGTH_OF_ARRAY(guide_setting_pump_text_infos); i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        } else {
            lv_obj_clear_flag(guide_info->text[i], LV_OBJ_FLAG_HIDDEN);
        }   
        lv_label_set_text(guide_info->text[i], guide_setting_pump_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(GUIDE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_setting_pump_text_infos[i].font, 0);
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0); /// 设置行间距（像素）
        // lv_obj_set_style_text_letter_space(guide_info->text[i], 1, 0); /// 设置字间距（像素）
        lv_obj_set_style_text_opa(guide_info->text[i], guide_setting_pump_text_infos[i].text_opa, 0);
        lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_setting_pump_text_infos[i].text_x,
            guide_setting_pump_text_infos[i].text_y);
    }
}

static void guide_ui_stop_pump_update(ui_manager_context_t *thiz)
{
    for (int i = 0; i < LENGTH_OF_ARRAY(guide_stop_pump_img_infos); i++) {
        if (guide_info->img[i] == NULL || !lv_obj_is_valid(guide_info->img[i])) {
            guide_info->img[i] = lv_image_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->img[i]);
        } else {
            lv_obj_clear_flag(guide_info->img[i], LV_OBJ_FLAG_HIDDEN);
        }
        common_ui_img_resource_init(thiz, guide_stop_pump_img_infos[i].img_id,
            &guide_info->img_info[i]);
        EXCEPTION_CHECK_VOID(guide_info->img_info[i].data);
        lv_fs_path_ex_t img_path;
        lv_fs_make_path_from_buffer(&img_path,
                                    LV_FS_MEMFS_LETTER,
                                    guide_info->img_info[i].data,   
                                    guide_info->img_info[i].size,
                                    "baj");
        lv_image_set_src(guide_info->img[i], (const void *)&img_path);
        lv_obj_align(guide_info->img[i], LV_ALIGN_TOP_LEFT, guide_stop_pump_img_infos[i].icon_x,
            guide_stop_pump_img_infos[i].icon_y);
    }

    for (int i = 0; i < LENGTH_OF_ARRAY(guide_stop_pump_text_infos); i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        } else {
            lv_obj_clear_flag(guide_info->text[i], LV_OBJ_FLAG_HIDDEN);
        }   
        lv_label_set_text(guide_info->text[i], guide_stop_pump_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(GUIDE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_stop_pump_text_infos[i].font, 0);
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0); /// 设置行间距（像素）
        // lv_obj_set_style_text_letter_space(guide_info->text[i], 1, 0); /// 设置字间距（像素）
        lv_obj_set_style_text_opa(guide_info->text[i], guide_stop_pump_text_infos[i].text_opa, 0);
        lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_stop_pump_text_infos[i].text_x,
            guide_stop_pump_text_infos[i].text_y);
    }
}

static void guide_ui_pump_mode_select_update(ui_manager_context_t *thiz)
{
    for (int i = 0; i < LENGTH_OF_ARRAY(guide_pump_mode_select_img_infos); i++) {
        if (guide_info->img[i] == NULL || !lv_obj_is_valid(guide_info->img[i])) {
            guide_info->img[i] = lv_image_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->img[i]);
        } else {
            lv_obj_clear_flag(guide_info->img[i], LV_OBJ_FLAG_HIDDEN);
        }
        common_ui_img_resource_init(thiz, guide_pump_mode_select_img_infos[i].img_id,
            &guide_info->img_info[i]);
        EXCEPTION_CHECK_VOID(guide_info->img_info[i].data);
        lv_fs_path_ex_t img_path;
        lv_fs_make_path_from_buffer(&img_path,
                                    LV_FS_MEMFS_LETTER,
                                    guide_info->img_info[i].data,   
                                    guide_info->img_info[i].size,
                                    "baj");
        lv_image_set_src(guide_info->img[i], (const void *)&img_path);
        lv_obj_align(guide_info->img[i], LV_ALIGN_TOP_LEFT, guide_pump_mode_select_img_infos[i].icon_x,
            guide_pump_mode_select_img_infos[i].icon_y);
    }

    for (int i = 0; i < LENGTH_OF_ARRAY(guide_pump_mode_select_text_infos); i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        } else {
            lv_obj_clear_flag(guide_info->text[i], LV_OBJ_FLAG_HIDDEN);
        }   
        lv_label_set_text(guide_info->text[i], guide_pump_mode_select_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(GUIDE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_pump_mode_select_text_infos[i].font, 0);
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0); /// 设置行间距（像素）
        // lv_obj_set_style_text_letter_space(guide_info->text[i], 1, 0); /// 设置字间距（像素）
        lv_obj_set_style_text_opa(guide_info->text[i], guide_pump_mode_select_text_infos[i].text_opa, 0);
        lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_pump_mode_select_text_infos[i].text_x,
            guide_pump_mode_select_text_infos[i].text_y);
    }
}

static void guide_qrcode_update(ui_manager_context_t *thiz)
{  
    ///  创建容器包裹二维码（用于添加边框和内边距效果）
    if (guide_info->qrcode_container == NULL || !lv_obj_is_valid(guide_info->qrcode_container)) {
        guide_info->qrcode_container = lv_obj_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->qrcode_container);
        ///  设置容器尺寸
        lv_obj_set_size(guide_info->qrcode_container, 138, 138);
        ///  设置容器背景颜色为白色
        lv_obj_set_style_bg_color(guide_info->qrcode_container, lv_color_hex(GUIDE_QRCODE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(guide_info->qrcode_container, LV_OPA_0, 0); /// 设置容器背景透明
        ///  设置容器边框样式（灰色边框，宽度2像素）
        lv_obj_set_style_border_color(guide_info->qrcode_container, lv_color_hex(GUIDE_QRCODE_BORDER_COLOR), 0);
        lv_obj_set_style_border_width(guide_info->qrcode_container, 2, 0);
        ///  设置内边距为4像素，让边框和二维码内容之间有白色间距
        lv_obj_set_style_pad_all(guide_info->qrcode_container, 10, 0);
        ///  禁用容器的滚动功能
        lv_obj_clear_flag(guide_info->qrcode_container, LV_OBJ_FLAG_SCROLLABLE);
    } else {
        lv_obj_clear_flag(guide_info->qrcode_container, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (guide_info->qrcode == NULL || !lv_obj_is_valid(guide_info->qrcode)) {
        guide_info->qrcode = lv_qrcode_create(guide_info->qrcode_container);
        EXCEPTION_CHECK_VOID(guide_info->qrcode);
    } else {
        lv_obj_clear_flag(guide_info->qrcode, LV_OBJ_FLAG_HIDDEN);
    }
    lv_qrcode_set_size(guide_info->qrcode, 124);
    lv_qrcode_set_dark_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_FG_COLOR));
    lv_qrcode_set_light_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_BG_COLOR));
    lv_qrcode_update(guide_info->qrcode, bind_qr_str[0], strlen(bind_qr_str[0]));
    lv_obj_center(guide_info->qrcode);
    lv_obj_align(guide_info->qrcode_container, LV_ALIGN_TOP_LEFT, 111, 71);
}

static void guide_ui_bind_guide_update(ui_manager_context_t *thiz)
{
    guide_qrcode_update(thiz);
    for (int i = 0; i < LENGTH_OF_ARRAY(guide_bind_guide_text_infos); i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        } else {
            lv_obj_clear_flag(guide_info->text[i], LV_OBJ_FLAG_HIDDEN);
        }   
        lv_label_set_text(guide_info->text[i], guide_bind_guide_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(GUIDE_BIND_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_bind_guide_text_infos[i].font, 0);
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_opa(guide_info->text[i], guide_bind_guide_text_infos[i].text_opa, 0);
        lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_bind_guide_text_infos[i].text_x,
            guide_bind_guide_text_infos[i].text_y);
    }
}

static void guide_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (guide_info->force_update == false && guide_info->page_index == page_index) {
        return;
    }
    /// 如果二维码容器和二维码对象存在，则隐藏它们
    if (guide_info->qrcode_container && guide_info->qrcode) {
        lv_obj_add_flag(guide_info->qrcode_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guide_info->qrcode, LV_OBJ_FLAG_HIDDEN);
    }
    /// 如果图片对象存在，则隐藏它们
    for (int i = 0; i < GUIDE_IMG_NUM; i++) {
        if (guide_info->img[i] == NULL) {
            continue;
        }
        lv_obj_add_flag(guide_info->img[i], LV_OBJ_FLAG_HIDDEN);
    }
    /// 如果文本对象存在，则隐藏它们
    for (int i = 0; i < GUIDE_TEXT_NUM; i++) {
        if (guide_info->text[i] == NULL) {
            continue;
        }
        lv_obj_add_flag(guide_info->text[i], LV_OBJ_FLAG_HIDDEN);
    }

    switch(page_index) {
        case GUIDE_PAGE_HOME:
            guide_ui_home_update(thiz);
            break;
        case GUIDE_PAGE_START_PUMP:
            guide_ui_start_pump_update(thiz);
            break;
        case GUIDE_PAGE_SETTING_PUMP:
            guide_ui_setting_pump_update(thiz);
            break;
        case GUIDE_PAGE_STOP_PUMP:
            guide_ui_stop_pump_update(thiz);
            break;
        case GUIDE_PAGE_PUMP_MODE_SELECT:
            guide_ui_pump_mode_select_update(thiz);
            break;
        case GUIDE_PAGE_BIND_GUIDE:
            guide_ui_bind_guide_update(thiz);
            break;
        default:
            return;
    }
    guide_common_img_update(thiz);
    guide_common_text_update(thiz, page_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_guide_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_display_info_t *)u_malloz(sizeof(guide_display_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_guide_root_init(); /// 初始化 root 容器
    common_ui_bg_init(thiz);
    guide_info->force_update = true;
}

void ui_display_guide_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(guide_info);
    EXCEPTION_CHECK_VOID(guide_info->root);
    common_ui_img_resource_deinit(thiz, &guide_info->arrow_info);
    for (int i = 0; i < GUIDE_IMG_NUM; i++) {
        common_ui_img_resource_deinit(thiz, &guide_info->img_info[i]);
    }
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(guide_info->root);
    u_free(guide_info);
    guide_info = NULL;
}

int ui_display_guide_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(guide_info);
    EXCEPTION_CHECK_INT(guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    ui_guide_bg_update(thiz);
    guide_ui_update(thiz, proc->page_index);
    guide_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
