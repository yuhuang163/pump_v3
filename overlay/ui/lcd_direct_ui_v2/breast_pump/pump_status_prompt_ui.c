/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_popup.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_STATUS_COMP_BG          (0)
#define PUMP_STATUS_TEXT_COUNT                  (2)

#define PUMP_START_ICON_X                       (148)
#define PUMP_START_ICON_Y                       (118)
#define PUMP_PAUSE_ICON_X                       (148)
#define PUMP_PAUSE_ICON_Y                       (118)

#define PROMPT_TEXT_COLOR                              (0xFFFFFF)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_PUMP_STATUS_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * pump_status_icon;
    res_img_info_t status_img_info;
    lv_obj_t * status_text[2];
    uint8_t force_update;
    // uint8_t status;                 // 当前泵状态
    // uint32_t start_time; /// 开始显示时间
} pump_status_display_info_t;

typedef struct {
    const char *name;
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
} pump_status_icon_info_t;

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
    const lv_font_t *font;
    uint16_t text_opa; /// 文本不透明度
} pump_status_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_STATUS_COMP_BG
static PSRAM_DATA_LOC uint8_t pump_status_prompt_bg_data[] = {
    #include "comparison_png2txt/pump_status_prompt_bg_360x360.txt"
};
#endif

static pump_status_display_info_t *pump_status_info;

const static pump_status_text_info_t pump_status_text_infos[PUMP_STATUS_TEXT_COUNT] = {
    { "Pumping paused", 126, 198, &lv_font_oppo_sans_16_medium, LV_OPA_100},
    { "Auto shutoff after 15 minutes of inactivity", 60, 222, &lv_font_oppo_sans_14_medium, LV_OPA_70},
};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_status_prompt_init(lv_obj_t *parent)
{
    if (pump_status_info->root == NULL || !lv_obj_is_valid(pump_status_info->root)) {
        pump_status_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(pump_status_info->root);
    }
    lv_obj_set_size(pump_status_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(pump_status_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(pump_status_info->root, LV_OPA_70, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(pump_status_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(pump_status_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(pump_status_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(pump_status_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(pump_status_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void pump_status_prompt_root_text_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_status_info->force_update == false) {
        return;
    }
    /// 设置文本区域icon
    if (pump_status_info->pump_status_icon == NULL || !lv_obj_is_valid(pump_status_info->pump_status_icon)) {
        pump_status_info->pump_status_icon = lv_image_create(pump_status_info->root);
        EXCEPTION_CHECK_VOID(pump_status_info->pump_status_icon);
    }
    
    common_ui_img_resource_init(thiz, RESOURCE_ID_PUMP_PAUSE_PROMPT_ICON, &pump_status_info->status_img_info);
    EXCEPTION_CHECK_VOID(pump_status_info->status_img_info.data);
    lv_fs_path_ex_t status_img_path;
    lv_fs_make_path_from_buffer(&status_img_path, LV_FS_MEMFS_LETTER, 
        pump_status_info->status_img_info.data, pump_status_info->status_img_info.size, "baj");
    lv_image_set_src(pump_status_info->pump_status_icon, (const void *)&status_img_path);
    lv_obj_align(pump_status_info->pump_status_icon, LV_ALIGN_TOP_LEFT, 
        PUMP_PAUSE_ICON_X, PUMP_PAUSE_ICON_Y); /// 设置icon位置

    /// 设置文本label UI第一行
    for (uint8_t i = 0; i < PUMP_STATUS_TEXT_COUNT; i++) {
        if (pump_status_info->status_text[i] == NULL || !lv_obj_is_valid(pump_status_info->status_text[i])) {
            pump_status_info->status_text[i] = lv_label_create(pump_status_info->root);
            EXCEPTION_CHECK_VOID(pump_status_info->status_text[i]);
        }
        lv_label_set_text(pump_status_info->status_text[i], pump_status_text_infos[i].name);  /// 设置 label 文字内容和样式
        lv_obj_set_style_text_color(pump_status_info->status_text[i], lv_color_hex(PROMPT_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(pump_status_info->status_text[i], pump_status_text_infos[i].font, 0);  /// 设置字体
        lv_obj_set_style_text_opa(pump_status_info->status_text[i], pump_status_text_infos[i].text_opa, 0);  /// 设置文本不透明度
        lv_obj_set_style_text_align(pump_status_info->status_text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(pump_status_info->status_text[i], LV_ALIGN_TOP_LEFT, pump_status_text_infos[i].text_x, pump_status_text_infos[i].text_y); /// 设置文字位置
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_prompt_pump_status_prepare(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (pump_status_info == NULL) {
        pump_status_info = (pump_status_display_info_t *)u_malloz(sizeof(pump_status_display_info_t));
        EXCEPTION_CHECK_VOID(pump_status_info);
    }
    pump_status_info->force_update = true;
    pump_status_prompt_init(parent);
}

void ui_prompt_pump_status_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_status_info == NULL) {
        return;
    }
    common_ui_img_resource_deinit(thiz, &pump_status_info->status_img_info);
    LVGL_OBJ_DELETE(pump_status_info->root);
    u_free(pump_status_info);
    pump_status_info = NULL;
}

void ui_prompt_pump_status_process(ui_manager_context_t *thiz, uint8_t status)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_status_info == NULL || pump_status_info->root == NULL) {
        return;
    }
    pump_status_prompt_root_text_update(thiz);
    pump_status_info->force_update = false;
}
