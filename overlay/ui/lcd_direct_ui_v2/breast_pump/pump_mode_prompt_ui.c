/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_MODE_PROMPT_BG                      (0)
#define MODE_PROMPT_ICON_MAX_COUNT                          (4)
#define MODE_PROMPT_TEXT_COUNT                              (4)

#define STIMULATION_PROMPT_ICON_X                           (148)
#define STIMULATION_PROMPT_ICON_Y                           (130)
#define EXPRESSION_PROMPT_ICON_X                            (148)
#define EXPRESSION_PROMPT_ICON_Y                            (130)
#define MIX_PROMPT_ICON_X                                   (148)
#define MIX_PROMPT_ICON_Y                                   (130)
#define RELAX_PROMPT_ICON_X                                 (148)
#define RELAX_PROMPT_ICON_Y                                 (130)
#define PROMPT_TEXT_COLOR                                   (0xE6DFDF)

#define MODE_PROMPT_DISPLAY_TIME                            (3000)

typedef struct {
    lv_obj_t * root;
    #if IS_SUPPORT_ROOT_MODE_PROMPT_BG
    lv_obj_t * backgroud;
    #endif
    lv_obj_t * text;
    lv_obj_t * mode_prompt_icon;
    res_img_info_t mode_img_info;
    uint8_t force_update;
    uint32_t start_time; /// 开始显示时间
} mode_switch_prompt_display_info_t;

typedef struct {
    const char *name;
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
} mode_prompt_icon_info_t;

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
    const lv_font_t *font;
    uint16_t line_space; /// 行间距（像素），用于换行
} mode_prompt_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static mode_switch_prompt_display_info_t *mode_switch_info;

const static mode_prompt_icon_info_t mode_prompt_icon_infos[MODE_PROMPT_ICON_MAX_COUNT] = {
    { "stimulation_prompt_icon", RESOURCE_ID_MODE_STIMULATION_PROMPT_ICON,
      STIMULATION_PROMPT_ICON_X,              STIMULATION_PROMPT_ICON_Y, 
    },
    { "expression_prompt_icon", RESOURCE_ID_MODE_EXPRESSION_PROMPT_ICON,
      EXPRESSION_PROMPT_ICON_X,              EXPRESSION_PROMPT_ICON_Y, 
    },
    { "mix_prompt_icon", RESOURCE_ID_MODE_MIX_PROMPT_ICON,
      MIX_PROMPT_ICON_X,              MIX_PROMPT_ICON_Y, 
    },
    { "relax_prompt_icon", RESOURCE_ID_MODE_RELAX_PROMPT_ICON,
      RELAX_PROMPT_ICON_X,              RELAX_PROMPT_ICON_Y, 
    },
};

const static mode_prompt_text_info_t mode_prompt_text_infos[MODE_PROMPT_TEXT_COUNT] = {
    { "Switching to Stimulation Mode", (72), (210), &lv_font_oppo_sans_16_medium ,0},
    { "Switching to Expression Mode", (78), (210), &lv_font_oppo_sans_16_medium ,0},
    { "Switching to Mix Mode", (106), (210), &lv_font_oppo_sans_16_medium ,0},
    { "Switching to Relax Mode", (98), (210), &lv_font_oppo_sans_16_medium ,0},
};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_MODE_PROMPT_BG
static PSRAM_DATA_LOC uint8_t mode_prompt_bg_data[] = {
    // #include "comparison_png2txt/bg_360x360.txt"
    #include "png2txt/mode_prompt_bg_360x360.txt"
};
#endif
/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void mode_switch_prompt_root_init(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    if (mode_switch_info->root == NULL || !lv_obj_is_valid(mode_switch_info->root)) {
        mode_switch_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(mode_switch_info->root);
    }
    lv_obj_set_size(mode_switch_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(mode_switch_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(mode_switch_info->root, LV_OPA_50, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(mode_switch_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(mode_switch_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(mode_switch_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(mode_switch_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(mode_switch_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void mode_switch_prompt_update(ui_manager_context_t *thiz, uint8_t mode_index)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_index >= MODE_PROMPT_ICON_MAX_COUNT) {
        mode_index = 0;
    }
    
    if (mode_switch_info->force_update == false) {
        return;
    }

    /// 设置文本区域icon
    if (mode_switch_info->mode_prompt_icon == NULL || !lv_obj_is_valid(mode_switch_info->mode_prompt_icon)) {
        mode_switch_info->mode_prompt_icon = lv_image_create(mode_switch_info->root);
        EXCEPTION_CHECK_VOID(mode_switch_info->mode_prompt_icon);
    }
    
    common_ui_img_resource_init(thiz, mode_prompt_icon_infos[mode_index].img_id,  &mode_switch_info->mode_img_info);
    EXCEPTION_CHECK_VOID(mode_switch_info->mode_img_info.data);
    lv_fs_path_ex_t mode_prompt_icon_img_path;
    lv_fs_make_path_from_buffer(&mode_prompt_icon_img_path, LV_FS_MEMFS_LETTER, 
        mode_switch_info->mode_img_info.data, mode_switch_info->mode_img_info.size, "baj");
    lv_image_set_src(mode_switch_info->mode_prompt_icon, (const void *)&mode_prompt_icon_img_path);
    lv_obj_align(mode_switch_info->mode_prompt_icon, LV_ALIGN_TOP_LEFT, 
        mode_prompt_icon_infos[mode_index].icon_x, mode_prompt_icon_infos[mode_index].icon_y); /// 设置icon位置

    /// 设置文本label
    if (mode_switch_info->text == NULL || !lv_obj_is_valid(mode_switch_info->text)) {
        mode_switch_info->text = lv_label_create(mode_switch_info->root);
        EXCEPTION_CHECK_VOID(mode_switch_info->text);
    }
    lv_label_set_text(mode_switch_info->text, mode_prompt_text_infos[mode_index].name);  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(mode_switch_info->text, lv_color_hex(PROMPT_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(mode_switch_info->text, mode_prompt_text_infos[mode_index].font, 0);  /// 设置字体
    lv_obj_set_style_text_line_space(mode_switch_info->text, mode_prompt_text_infos[mode_index].line_space, 0);  /// 设置行间距（像素）
    lv_obj_align(mode_switch_info->text, LV_ALIGN_TOP_LEFT, mode_prompt_text_infos[mode_index].text_x, 
        mode_prompt_text_infos[mode_index].text_y); /// 设置文字位置
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_prompt_mode_switch_prepare(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (mode_switch_info == NULL) {
        mode_switch_info = (mode_switch_prompt_display_info_t *)u_malloz(sizeof(mode_switch_prompt_display_info_t));
        EXCEPTION_CHECK_VOID(mode_switch_info);
    }
    mode_switch_info->force_update = true;
    mode_switch_info->start_time = get_timestamp();
    mode_switch_prompt_root_init(thiz, parent);
}

void ui_prompt_mode_switch_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_switch_info == NULL) {
        return;
    }
    common_ui_img_resource_deinit(thiz, &mode_switch_info->mode_img_info);
    LVGL_OBJ_DELETE(mode_switch_info->root);
    u_free(mode_switch_info);
    mode_switch_info = NULL;
}

void ui_prompt_mode_switch_process(ui_manager_context_t *thiz, uint8_t mode)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_switch_info == NULL || mode_switch_info->root == NULL) {
        return;
    }
    mode_switch_prompt_update(thiz, mode);
    mode_switch_info->force_update = false;
    if (get_timestamp() - mode_switch_info->start_time >= MODE_PROMPT_DISPLAY_TIME) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        ani->progress_cb(thiz, PUMP_POPUP_UI_MODE_PROMPT, ANI_POPUP_DISPLAY_FINISH);
    }
}
