/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_finish_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_FINISH_COMP_BG                         (0)
#define COMPLETE_TIP_TEXT_COUNT                                (2)

#define PUMP_FINISH_IP_X                                       (0)
#define PUMP_FINISH_IP_Y                                       (32)
#define PUMP_FINISH_TITLE_X                                    (0)
#define PUMP_FINISH_TITLE_Y                                    (232 + 4)
#define PUMP_FINISH_CONTENT_X                                  (0)
#define PUMP_FINISH_CONTENT_Y                                  (264 + 4)
#define FINISH_TIP_TITLE_COLOR                                 0xB25962
#define FINISH_TIP_CONTENT_COLOR                               0xC57D80

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
    const lv_font_t *font;
    uint32_t color;
} complete_tip_text_info_t;

typedef struct {
    lv_obj_t * root;
    #if IS_SUPPORT_PUMP_FINISH_COMP_BG
    lv_obj_t * backgroud;
    #endif
    lv_obj_t * icon;
    lv_obj_t * texts[COMPLETE_TIP_TEXT_COUNT];
    res_img_info_t icon_info;
    uint8_t force_update;
    uint8_t rsv[15];
} complete_tip_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

static complete_tip_display_info_t *finish_info;

const static complete_tip_text_info_t complete_tip_text_infos[COMPLETE_TIP_TEXT_COUNT] = {
    { "Finish", PUMP_FINISH_TITLE_X, PUMP_FINISH_TITLE_Y, &lv_font_aeonik_soft_20_medium, FINISH_TIP_TITLE_COLOR},
    { "Open the Momcozy app to track your\nmilk output", PUMP_FINISH_CONTENT_X, PUMP_FINISH_CONTENT_Y, \
        &lv_font_aeonik_soft_16_medium, FINISH_TIP_CONTENT_COLOR},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_FINISH_COMP_BG
static PSRAM_DATA_LOC uint8_t pump_finish_bg_data[] = {
    // #include "comparison_png2txt/bg_360x360.txt"
    #include "comparison_png2txt/pump_finish_360x360.txt"
};
#endif
/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_complete_tip_root_init(void)
{
    if (finish_info->root == NULL || !lv_obj_is_valid(finish_info->root)) {
        finish_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(finish_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(finish_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(finish_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(finish_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(finish_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(finish_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void pump_complete_tip_label_update(ui_manager_context_t *thiz)
{
    if (finish_info->icon == NULL || !lv_obj_is_valid(finish_info->icon)) {
        finish_info->icon = lv_image_create(finish_info->root);
    }
    ui_theme_img_resource_init(thiz, BASE_RES_ID_PUMP_FINISH_ICON, RES_THEME_TYPE_CUSTOM,
        &finish_info->icon_info);
    EXCEPTION_CHECK_VOID(finish_info->icon_info.data);
    lv_fs_path_ex_t complete_tip_img_path;
    lv_fs_make_path_from_buffer(&complete_tip_img_path, LV_FS_MEMFS_LETTER,
        finish_info->icon_info.data, finish_info->icon_info.size, "baj");
    lv_image_set_src(finish_info->icon, complete_tip_img_path.path);
    lv_obj_align(finish_info->icon, LV_ALIGN_TOP_MID, PUMP_FINISH_IP_X, PUMP_FINISH_IP_Y);
}

static void pump_complete_tip_texts_update(ui_manager_context_t *thiz, uint8_t index)
{
    if (finish_info->texts[index] == NULL || !lv_obj_is_valid(finish_info->texts[index])) {
        finish_info->texts[index] = lv_label_create(finish_info->root);
    }
    lv_label_set_text(finish_info->texts[index], complete_tip_text_infos[index].name);
    lv_obj_set_style_text_color(finish_info->texts[index], lv_color_hex(complete_tip_text_infos[index].color), 0);
    lv_obj_set_style_text_font(finish_info->texts[index], complete_tip_text_infos[index].font, 0);  /// 设置字体
    lv_obj_set_style_text_line_space(finish_info->texts[index], 2, 0);  /// 设置行间距（像素）
    lv_obj_set_style_text_align(finish_info->texts[index], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_align(finish_info->texts[index], LV_ALIGN_TOP_MID,
        complete_tip_text_infos[index].text_x, complete_tip_text_infos[index].text_y);
}

#if IS_SUPPORT_PUMP_FINISH_COMP_BG
static void pump_finish_root_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(finish_info);
    EXCEPTION_CHECK_VOID(finish_info->root);
    
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (finish_info->backgroud == NULL || !lv_obj_is_valid(finish_info->backgroud)) {
        finish_info->backgroud = lv_image_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(finish_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(finish_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, pump_finish_bg_data,
            sizeof(pump_finish_bg_data), "png");
        lv_image_set_src(finish_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(finish_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_finish_root_bg_update(ui_manager_context_t *thiz)
{ 

}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_finish_custom_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (finish_info == NULL) {
        finish_info = (complete_tip_display_info_t *)u_malloz(sizeof(complete_tip_display_info_t));
        EXCEPTION_CHECK_VOID(finish_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_complete_tip_root_init();
    finish_info->force_update = true;
}

void pump_finish_custom_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(finish_info);
    ui_img_resource_deinit(thiz, &finish_info->icon_info);
    LVGL_OBJ_DELETE(finish_info->root); 
    u_free(finish_info);
    finish_info = NULL;
}

int pump_finish_custom_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(finish_info);
    EXCEPTION_CHECK_INT(finish_info->root);
    pump_finish_root_bg_update(thiz);
    if (finish_info->force_update == true) {
        pump_complete_tip_label_update(thiz);
        for (uint8_t i = 0; i < COMPLETE_TIP_TEXT_COUNT; i++) {
            pump_complete_tip_texts_update(thiz, i);
        }
        finish_info->force_update = false;
    }
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump complete tip fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}