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

#define PUMP_FINISH_ICON_Y                                     (55)
#define PUMP_FINISH_TITLE_Y                                    (165)
#define PUMP_FINISH_CONTENT_Y                                  (207)
#define FINISH_TIP_TITLE_COLOR                                 (0xFFFFFF)
#define FINISH_TIP_CONTENT_COLOR                               (0x999999)

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint16_t text_line_space;
    uint32_t color;
} complete_tip_text_info_t;

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_PUMP_FINISH_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * icon;
    lv_obj_t * texts[COMPLETE_TIP_TEXT_COUNT];
    uint8_t force_update;
    uint8_t rsv[15];
} complete_tip_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

static complete_tip_display_info_t *finish_info;

const static complete_tip_text_info_t complete_tip_text_infos[COMPLETE_TIP_TEXT_COUNT] = {
    { "Pump finished", PUMP_FINISH_TITLE_Y, &lv_font_aeonik_soft_26_medium, 0, FINISH_TIP_TITLE_COLOR},
    { "Open the Momcozy\napp to track your milk\noutput", PUMP_FINISH_CONTENT_Y,
        &lv_font_aeonik_soft_26_medium, 10, FINISH_TIP_CONTENT_COLOR},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_FINISH_COMP_BG
static PSRAM_DATA_LOC uint8_t pump_finish_bg_data[] = {
    #include "comparison_png2txt/pump_finish12_352x352.txt"
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
    lv_obj_set_style_bg_color(finish_info->root, lv_color_black(), 0);     /// 设置背景颜色
    lv_obj_set_style_bg_opa(finish_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(finish_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(finish_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(finish_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void pump_complete_icon_update(ui_manager_context_t *thiz)
{
    if (finish_info->force_update == false) {
        return;
    }

    if (finish_info->icon == NULL || !lv_obj_is_valid(finish_info->icon)) {
        finish_info->icon = lv_image_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->icon);
    }
    ui_img_baj_theme_set_src(thiz, finish_info->icon, BASE_RES_ID_SUCCESS_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(finish_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_FINISH_ICON_Y);
}

static void pump_complete_tip_texts_update(void)
{
    if (finish_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < COMPLETE_TIP_TEXT_COUNT; i++) {
        if (finish_info->texts[i] == NULL || !lv_obj_is_valid(finish_info->texts[i])) {
            finish_info->texts[i] = lv_label_create(finish_info->root);
            EXCEPTION_CHECK_VOID(finish_info->texts[i]);
        }
        
        lv_label_set_text(finish_info->texts[i], complete_tip_text_infos[i].name);
        lv_obj_set_style_text_color(finish_info->texts[i],
            lv_color_hex(complete_tip_text_infos[i].color), 0);
        lv_obj_set_style_text_font(finish_info->texts[i], complete_tip_text_infos[i].font, 0);
        lv_obj_set_style_text_line_space(finish_info->texts[i], complete_tip_text_infos[i].text_line_space, 0);
        lv_obj_set_style_text_align(finish_info->texts[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(finish_info->texts[i], LV_ALIGN_TOP_MID, 0, complete_tip_text_infos[i].text_y);
    }
}

#if IS_SUPPORT_PUMP_FINISH_COMP_BG
static void pump_finish_root_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && finish_info && finish_info->root);
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (finish_info->background == NULL || !lv_obj_is_valid(finish_info->background)) {
        finish_info->background = lv_image_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->background);
        lv_obj_move_background(finish_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(finish_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(finish_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, pump_finish_bg_data,
            sizeof(pump_finish_bg_data), "png");
        lv_image_set_src(finish_info->background, (const void *)&bg_img_path);
        lv_obj_align(finish_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_finish_root_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_finish_base_ui_prepare(ui_manager_context_t *thiz)
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

void pump_finish_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && finish_info && finish_info->root);
    ui_img_baj_release_decoded_buf(finish_info->icon);
    LVGL_OBJ_DELETE(finish_info->root);
    u_free(finish_info);
    finish_info = NULL;
}

int pump_finish_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && finish_info && finish_info->root);
    pump_finish_root_bg_update(thiz);
    pump_complete_icon_update(thiz);
    pump_complete_tip_texts_update();
    finish_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump complete fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
