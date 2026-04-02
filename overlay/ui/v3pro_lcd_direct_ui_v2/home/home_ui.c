/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "resource_id_define.h"
#include "src/misc/lv_color.h"
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep
#include <stdint.h>

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_HOME_COMP_BG                (0)

#define IDLE_LABEL_AREA_ICON_X                      (44)
#define IDLE_LABEL_AREA_ICON_Y                      (82 + 2)
#define IDLE_BUTTON_ICON_X                          (80)
#define IDLE_BUTTON_ICON_Y                          (112 + 7)

#define IDLE_LABEL_TEXT_X                           (108 + 2)
#define IDLE_LABEL_TEXT_Y                           (118 - 4)
#define IDLE_TEXT_COLOR                             (0xB25962)

#define HOME_IDLE_ANIMATION_X                       (86 + 8)
#define HOME_IDLE_ANIMATION_Y                       (162)

#define HOME_LIGHT_ANIMATION_X                      (86 + 8)
#define HOME_LIGHT_ANIMATION_Y                      (162)

#define HOME_CLOCK_HOUR_X                           (44 - 1)
#define HOME_CLOCK_MINUTE_X                         (192 - 1)
#define HOME_CLOCK_TIME_Y                           (141 + 7)
#define HOME_CLOCK_COLON_X                          (174 - 4)
#define HOME_CLOCK_COLON_Y                          (141 + 20)

#define HOME_WITH_CLOCK_LABLE_Y_OFFSET              (29)
#define HOME_CLOCK_TIP_LABLE_Y_OFFSET               (7)
#define HOME_RECORD_TEXT_X_OFFSET                   (14)
#define HOME_RECORD_TEXT_Y_OFFSET                   (20)
#define HOME_LIGHT_TIP_LABLE_Y_OFFSET               (28)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_ROOT_HOME_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * text;
    lv_obj_t * time_text;
    lv_obj_t * label_icon;
    res_img_info_t label_info;
    lv_obj_t * clock_hour;
    lv_obj_t * clock_minute;
    lv_obj_t * clock_colon;
    res_img_info_t clock_colon_info;
    lv_obj_t * button_icon;
    res_img_info_t button_info;
    lv_obj_t * animation;
    res_img_info_t animation_info;
    home_ui_display_type_e home_ui_type;
    uint32_t timestamp_offset;
    uint16_t animation_index;
    uint8_t force_update;
    uint32_t hour;
    uint8_t minute;
} home_display_info_t;


/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static home_display_info_t *home_info;
#if IS_SUPPORT_ROOT_HOME_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_bg_data[] = {
    #include "comparison_png2txt/Custom5_360x360.txt"
};
#endif

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
    lv_obj_set_style_bg_color(home_info->root, lv_color_white(), 0);     /// 设置背景颜色为黑色
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
    if (home_info->backgroud == NULL || !lv_obj_is_valid(home_info->backgroud)) {
        home_info->backgroud = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(home_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(home_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(home_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, home_info->root);
}
#endif

static void home_button_ui_update(ui_manager_context_t *thiz, uint16_t x_offset, uint16_t y_offset, bool is_show)
{
    if (home_info->button_icon == NULL || !lv_obj_is_valid(home_info->button_icon)) {
        home_info->button_icon = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->button_icon);
        lv_obj_add_flag(home_info->button_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (is_show) {
        if (home_info->force_update == false) {
            return;
        }
        lv_obj_clear_flag(home_info->button_icon, LV_OBJ_FLAG_HIDDEN);
        common_ui_img_resource_init(thiz, RESOURCE_ID_HOME_IDLE_BUTTOM_ICON,
            &home_info->button_info);
        EXCEPTION_CHECK_VOID(home_info->button_info.data);
        lv_fs_path_ex_t button_img_path;
        lv_fs_make_path_from_buffer(&button_img_path, LV_FS_MEMFS_LETTER,
                                    home_info->button_info.data,
                                    home_info->button_info.size, "baj");
        lv_image_set_src(home_info->button_icon, (const void *)&button_img_path);
        lv_obj_align(home_info->button_icon, LV_ALIGN_TOP_LEFT, x_offset, y_offset); /// 设置icon位置
    } else {
        lv_obj_add_flag(home_info->button_icon, LV_OBJ_FLAG_HIDDEN);
    }
}

static void home_label_text_ui_update(ui_manager_context_t *thiz, char *content, 
        uint16_t x_offset, uint16_t y_offset, bool is_show, bool align_center, const lv_font_t *font)
{

    if (home_info->text == NULL || !lv_obj_is_valid(home_info->text)) {
        home_info->text = lv_label_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->text);
        lv_obj_add_flag(home_info->text, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (is_show) {
        if (home_info->force_update == false) {
            return;
        }
        lv_obj_clear_flag(home_info->text, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(home_info->text, content);  /// 设置 label 文字内容和样式
        lv_obj_set_style_text_color(home_info->text, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->text, font, 0);  /// 设置字体
        lv_obj_align(home_info->text, LV_ALIGN_TOP_LEFT, x_offset, y_offset); /// 设置文字位置
        lv_obj_set_style_text_line_space(home_info->text, 3, 0); /// 设置行间距
        if (align_center) {
            lv_obj_set_style_text_align(home_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        } else {
            lv_obj_set_style_text_align(home_info->text, LV_TEXT_ALIGN_LEFT, 0);  /// 设置文本左对齐
        }
    } else {
        lv_obj_add_flag(home_info->text, LV_OBJ_FLAG_HIDDEN);
    }
}

static void home_record_time_ui_update(ui_manager_context_t *thiz, uint32_t timestamp_offset,
        uint16_t x_offset, uint16_t y_offset, bool is_show) {
    if (home_info->time_text == NULL || !lv_obj_is_valid(home_info->time_text)) {
        home_info->time_text = lv_label_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->time_text);
        lv_obj_add_flag(home_info->time_text, LV_OBJ_FLAG_HIDDEN);
    }

    if (is_show) {
        uint32_t total_minutes = timestamp_offset / 1000 / 60;
        uint32_t hour = total_minutes / 60;
        uint32_t minute = total_minutes % 60;
        if (home_info->force_update == false && home_info->hour == hour && home_info->minute == minute) {
            return;
        }
        char time_text[32] = {0};
        sprintf(time_text, "%d%s%d%s", hour, " hr ", minute, " min");
        lv_obj_clear_flag(home_info->time_text, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(home_info->time_text, time_text);  /// 设置 label 文字内容和样式
        lv_obj_set_style_text_color(home_info->time_text, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->time_text, &lv_font_oppo_sans_14_bold, 0);  /// 设置字体
        lv_obj_update_layout(home_info->time_text);
        lv_coord_t text_width = lv_obj_get_width(home_info->time_text);
        lv_obj_align(home_info->time_text, LV_ALIGN_TOP_LEFT, x_offset - (text_width / 2), y_offset); /// 设置文字位置
        home_info->hour = hour;
        home_info->minute = minute;
    } else {
        lv_obj_add_flag(home_info->time_text, LV_OBJ_FLAG_HIDDEN);
    }
}

static void home_label_ui_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t x_offset, uint16_t y_offset, bool is_show)
{
    if (home_info->label_icon == NULL || !lv_obj_is_valid(home_info->label_icon)) {
        home_info->label_icon = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->label_icon);
        lv_obj_add_flag(home_info->label_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (is_show) {
        if (home_info->force_update == false) {
            return;
        }
        lv_obj_clear_flag(home_info->label_icon, LV_OBJ_FLAG_HIDDEN);
        common_ui_img_resource_init(thiz, img_id, &home_info->label_info);
        EXCEPTION_CHECK_VOID(home_info->label_info.data);
        lv_fs_path_ex_t label_img_path;
        lv_fs_make_path_from_buffer(&label_img_path, LV_FS_MEMFS_LETTER, 
            home_info->label_info.data, home_info->label_info.size, "baj");
        lv_image_set_src(home_info->label_icon, (const void *)&label_img_path);
        lv_obj_align(home_info->label_icon, LV_ALIGN_TOP_LEFT, x_offset, y_offset); /// 设置icon位置
    } else {
        lv_obj_add_flag(home_info->label_icon, LV_OBJ_FLAG_HIDDEN);
    }
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

    lv_obj_align(home_info->animation, LV_ALIGN_TOP_LEFT, x_offset, y_offset); 
    if (home_info->animation_info.data != NULL) {
        common_ui_img_resource_deinit(thiz, &home_info->animation_info);
        memset(&home_info->animation_info, 0, sizeof(home_info->animation_info));
    }
    common_ui_img_resource_init(thiz, 
        start_resource_id + home_info->animation_index, &home_info->animation_info);
    EXCEPTION_CHECK_VOID(home_info->animation_info.data);
    lv_fs_path_ex_t animation_img_path;
    lv_fs_make_path_from_buffer(&animation_img_path, LV_FS_MEMFS_LETTER, 
        home_info->animation_info.data, home_info->animation_info.size, "baj");
    lv_image_set_src(home_info->animation, (const void *)&animation_img_path);
    home_info->animation_index++;
}

static void home_clock_ui_update(ui_manager_context_t *thiz, int16_t y_offset, bool is_show)
{
    /// 更新时钟小时
    if (home_info->clock_hour == NULL || !lv_obj_is_valid(home_info->clock_hour)) {
        home_info->clock_hour = lv_label_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->clock_hour);
        lv_obj_set_width(home_info->clock_hour, 120);
        lv_label_set_long_mode(home_info->clock_hour, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(home_info->clock_hour, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_text_color(home_info->clock_hour, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->clock_hour, &lv_font_quicksand_96_bold, 0);
        lv_obj_add_flag(home_info->clock_hour, LV_OBJ_FLAG_HIDDEN);
    }
    
    /// 更新时钟分钟
    if (home_info->clock_minute == NULL || !lv_obj_is_valid(home_info->clock_minute)) {
        home_info->clock_minute = lv_label_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->clock_minute);
        lv_obj_set_width(home_info->clock_minute, 120); 
        lv_label_set_long_mode(home_info->clock_minute, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(home_info->clock_minute, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_text_color(home_info->clock_minute, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->clock_minute, &lv_font_quicksand_96_bold, 0);
        lv_obj_add_flag(home_info->clock_minute, LV_OBJ_FLAG_HIDDEN);
    }

    /// 更新时钟冒号
    if (home_info->clock_colon == NULL || !lv_obj_is_valid(home_info->clock_colon)) {
        home_info->clock_colon = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->clock_colon);
        common_ui_img_resource_init(thiz, RESOURCE_ID_CLOCK_COLON_ICON, &home_info->clock_colon_info);
        EXCEPTION_CHECK_VOID(home_info->clock_colon_info.data);
        lv_fs_path_ex_t clock_colon_img_path;
        lv_fs_make_path_from_buffer(&clock_colon_img_path, LV_FS_MEMFS_LETTER, 
            home_info->clock_colon_info.data, home_info->clock_colon_info.size, "baj");
        LOG_UI("clock_colon_img_path: %s", clock_colon_img_path.path);
        lv_image_set_src(home_info->clock_colon, (const void *)&clock_colon_img_path);
        lv_obj_add_flag(home_info->clock_colon, LV_OBJ_FLAG_HIDDEN);
    }

    if (is_show) {
        utc_time_t utc_time = {0};
        uint32_t timestamp = platform_get_utc_time();
        timestamp_to_utc_time(timestamp, &utc_time);
        if (home_info->hour == utc_time.hour && home_info->minute == utc_time.minute
            && home_info->force_update == false) {
            return;
        }
        char time_text[16] = {0};
        lv_obj_clear_flag(home_info->clock_hour, LV_OBJ_FLAG_HIDDEN);
        sprintf(time_text, "%02d", utc_time.hour);
        lv_label_set_text(home_info->clock_hour, time_text);
        lv_obj_align(home_info->clock_hour, LV_ALIGN_TOP_LEFT, 
            HOME_CLOCK_HOUR_X, HOME_CLOCK_TIME_Y + y_offset);
        lv_obj_clear_flag(home_info->clock_minute, LV_OBJ_FLAG_HIDDEN);
        sprintf(time_text, "%02d", utc_time.minute);
        lv_label_set_text(home_info->clock_minute, time_text);
        lv_obj_align(home_info->clock_minute, LV_ALIGN_TOP_LEFT, 
            HOME_CLOCK_MINUTE_X, HOME_CLOCK_TIME_Y + y_offset);
        lv_obj_clear_flag(home_info->clock_colon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(home_info->clock_colon, LV_ALIGN_TOP_LEFT, 
            HOME_CLOCK_COLON_X, HOME_CLOCK_COLON_Y + y_offset);
        home_info->hour = utc_time.hour;
        home_info->minute = utc_time.minute;
    } else {
        lv_obj_add_flag(home_info->clock_hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(home_info->clock_minute, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(home_info->clock_colon, LV_OBJ_FLAG_HIDDEN);
    }
}

static home_ui_display_type_e home_ui_display_type_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
#if 0
    static home_ui_display_type_e home_ui_type_test = HOME_UI_DISPLAY_RECORD; /// TODO: 测试用，后续删除
    static uint32_t count = 0; /// TODO: 测试用，后续删除
    count++;
    if (count % 100 == 0) {
        count = 0;
        home_ui_type_test++;
        if (home_ui_type_test >= HOME_UI_DISPLAY_MAX) {
            home_ui_type_test = HOME_UI_DISPLAY_DEFAULT;
        }
    }
    home_info->timestamp_offset = ((120 * 60 * 60 * 1000) + (9 * 60 * 1000)); /// TODO: 测试用，后续删除
    return home_ui_type_test;
#else
    return HOME_UI_DISPLAY_DEFAULT;
#endif
}

static void home_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
    home_ui_display_type_e home_ui_type = home_ui_display_type_update(thiz, proc);
    if (home_ui_type != home_info->home_ui_type) {
        home_info->home_ui_type = home_ui_type;
        home_info->force_update = true;
        home_info->hour = 0;
        home_info->minute = 0;
    }
    switch (home_ui_type) {
        default:
        case HOME_UI_DISPLAY_DEFAULT:
            home_label_ui_update(thiz, RESOURCE_ID_HOME_IDLE_LABLE_ICON,
                IDLE_LABEL_AREA_ICON_X, IDLE_LABEL_AREA_ICON_Y, true);
            home_button_ui_update(thiz, IDLE_BUTTON_ICON_X, IDLE_BUTTON_ICON_Y, true);
            home_label_text_ui_update(thiz, "Select a mode to start\npumping.",
                IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y, true, false, &lv_font_oppo_sans_14_medium);
            home_record_time_ui_update(thiz, 
                home_info->timestamp_offset, IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y, false);
            home_clock_ui_update(thiz, 0, false);
            home_root_animation_update(thiz, RESOURCE_ID_HOME_DEFAULT_ANIMATION_START,
                 RESOURCE_ID_HOME_DEFAULT_ANIMATION_END - RESOURCE_ID_HOME_DEFAULT_ANIMATION_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_CLOCK_AND_RECORD:
            home_label_ui_update(thiz, RESOURCE_ID_HOME_IDLE_LABLE_ICON,
                IDLE_LABEL_AREA_ICON_X, IDLE_LABEL_AREA_ICON_Y - HOME_WITH_CLOCK_LABLE_Y_OFFSET, true);
            home_button_ui_update(thiz, IDLE_BUTTON_ICON_X, IDLE_BUTTON_ICON_Y, false);
            home_label_text_ui_update(thiz, "since last pumping session",
                IDLE_LABEL_TEXT_X - HOME_RECORD_TEXT_X_OFFSET, 
                (IDLE_LABEL_TEXT_Y - HOME_WITH_CLOCK_LABLE_Y_OFFSET) + HOME_RECORD_TEXT_Y_OFFSET, 
                true, false, &lv_font_oppo_sans_14_medium);
            home_record_time_ui_update(thiz, home_info->timestamp_offset, 
                    180, IDLE_LABEL_TEXT_Y - HOME_WITH_CLOCK_LABLE_Y_OFFSET, true);
            home_clock_ui_update(thiz, 0, true);
            home_root_animation_update(thiz, RESOURCE_ID_HOME_DEFAULT_ANIMATION_START,
                RESOURCE_ID_HOME_DEFAULT_ANIMATION_END - RESOURCE_ID_HOME_DEFAULT_ANIMATION_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_CLOCK:
            home_label_ui_update(thiz, RESOURCE_ID_HOME_IDLE_LABLE_ICON,
                IDLE_LABEL_AREA_ICON_X, IDLE_LABEL_AREA_ICON_Y - HOME_WITH_CLOCK_LABLE_Y_OFFSET, true);
            home_button_ui_update(thiz, IDLE_BUTTON_ICON_X, IDLE_BUTTON_ICON_Y - HOME_WITH_CLOCK_LABLE_Y_OFFSET, true);
            home_label_text_ui_update(thiz, "Select a mode to start\npumping.",
                IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y - HOME_WITH_CLOCK_LABLE_Y_OFFSET, true, false, &lv_font_oppo_sans_14_medium);
            home_record_time_ui_update(thiz, home_info->timestamp_offset, 
                IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y - HOME_WITH_CLOCK_LABLE_Y_OFFSET, false);
            home_clock_ui_update(thiz, 0, true);
            home_root_animation_update(thiz, RESOURCE_ID_HOME_DEFAULT_ANIMATION_START,
                 RESOURCE_ID_HOME_DEFAULT_ANIMATION_END - RESOURCE_ID_HOME_DEFAULT_ANIMATION_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_RECORD:
            home_label_ui_update(thiz, RESOURCE_ID_HOME_IDLE_LABLE_ICON,
                IDLE_LABEL_AREA_ICON_X, IDLE_LABEL_AREA_ICON_Y, true);
            home_button_ui_update(thiz, IDLE_BUTTON_ICON_X, IDLE_BUTTON_ICON_Y, false);
            home_label_text_ui_update(thiz, "since last pumping session",
                IDLE_LABEL_TEXT_X - HOME_RECORD_TEXT_X_OFFSET, 
                IDLE_LABEL_TEXT_Y + HOME_RECORD_TEXT_Y_OFFSET, true, false, &lv_font_oppo_sans_14_medium);
            home_record_time_ui_update(thiz, home_info->timestamp_offset,
                180, IDLE_LABEL_TEXT_Y, true);
            home_clock_ui_update(thiz, 0, false);
            home_root_animation_update(thiz, RESOURCE_ID_HOME_DEFAULT_ANIMATION_START,
                RESOURCE_ID_HOME_DEFAULT_ANIMATION_END - RESOURCE_ID_HOME_DEFAULT_ANIMATION_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_CLOCK_ADJUST_TIP:
            home_label_ui_update(thiz, RESOURCE_ID_HOME_IDLE_LABLE_ICON,
                IDLE_LABEL_AREA_ICON_X, IDLE_LABEL_AREA_ICON_Y - HOME_CLOCK_TIP_LABLE_Y_OFFSET, true);
            home_button_ui_update(thiz, IDLE_BUTTON_ICON_X, IDLE_BUTTON_ICON_Y, false);
            home_label_text_ui_update(thiz, "Time not updated. Connect\nthe device to sync",
                IDLE_LABEL_TEXT_X - 12, IDLE_LABEL_TEXT_Y - HOME_CLOCK_TIP_LABLE_Y_OFFSET, true, true, &lv_font_oppo_sans_14_medium);
            home_record_time_ui_update(thiz, home_info->timestamp_offset,
                 IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y - HOME_CLOCK_TIP_LABLE_Y_OFFSET, false);
            home_clock_ui_update(thiz, 0, false);
            home_root_animation_update(thiz, RESOURCE_ID_HOME_DEFAULT_ANIMATION_START,
                RESOURCE_ID_HOME_DEFAULT_ANIMATION_END - RESOURCE_ID_HOME_DEFAULT_ANIMATION_START + 1,
                HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
        case HOME_UI_DISPLAY_CLOCK_LIGHT_TIP:
            home_label_ui_update(thiz, RESOURCE_ID_HOME_LIGHT_LABLE_ICON,
                IDLE_LABEL_AREA_ICON_X, IDLE_LABEL_AREA_ICON_Y - HOME_LIGHT_TIP_LABLE_Y_OFFSET, true);
            home_button_ui_update(thiz, IDLE_BUTTON_ICON_X, IDLE_BUTTON_ICON_Y, false);
            home_label_text_ui_update(thiz, "Press to turn on the light",
                IDLE_LABEL_TEXT_X - 18, 
                (IDLE_LABEL_TEXT_Y - 2) - HOME_LIGHT_TIP_LABLE_Y_OFFSET,
                true, false, &lv_font_oppo_sans_16_medium);
            home_record_time_ui_update(thiz, home_info->timestamp_offset,
                 IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y - HOME_LIGHT_TIP_LABLE_Y_OFFSET, false);
            home_clock_ui_update(thiz, -18, true);
            // home_root_animation_update(thiz, RESOURCE_ID_HOME_LIGHT_ANIMATION_START,
            //     RESOURCE_ID_HOME_LIGHT_ANIMATION_END - RESOURCE_ID_HOME_LIGHT_ANIMATION_START + 1,
            //     HOME_IDLE_ANIMATION_X, HOME_IDLE_ANIMATION_Y);
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_prepare(ui_manager_context_t *thiz)
{
#if 0
    ui_display_home_screen_saver_prepare(thiz);
#else
    EXCEPTION_CHECK_VOID(thiz);
    if (home_info == NULL) {
        home_info = (home_display_info_t *)u_malloz(sizeof(home_display_info_t));
        EXCEPTION_CHECK_VOID(home_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_root_init();
    battery_ui_init(thiz);
    ble_ui_init(thiz);
    common_ui_bg_init(thiz);
    home_info->force_update = true;
    home_info->animation_index = 0;
#endif
}

void ui_display_home_remove(ui_manager_context_t *thiz)
{
#if 0
    ui_display_home_screen_saver_remove(thiz);
#else
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(home_info);
    common_ui_img_resource_deinit(thiz, &home_info->label_info);
    common_ui_img_resource_deinit(thiz, &home_info->button_info);
    common_ui_img_resource_deinit(thiz, &home_info->animation_info);
    common_ui_img_resource_deinit(thiz, &home_info->clock_colon_info);
    battery_ui_deinit(thiz);
    ble_ui_deinit(thiz);
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(home_info->root);
    u_free(home_info);
    home_info = NULL;
#endif
}

int ui_display_home_process(ui_manager_context_t *thiz)
{
#if 0
    ui_display_home_screen_saver_process(thiz);
#else
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(home_info);
    EXCEPTION_CHECK_INT(home_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->ble);
    home_root_bg_update(thiz);
    battery_ui_update(thiz, home_info->root, proc->battery, true);
    ble_ui_update(thiz, home_info->root, proc->ble, COMMON_BLE_UI_X, COMMON_BLE_UI_Y);
    home_ui_update(thiz, proc);
    home_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home fps: %d, home_ui_type: %d", thiz->priv.fps_count, home_info->home_ui_type);
        thiz->priv.fps_count = 0;
    }
#endif
    return 0;
}
