/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pump_finish_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_FINISH_COMP_BG                         (0)

#define PUMP_FINISH_ICON_Y                                     (50)
#define PUMP_FINISH_TITLE_X                                    (0)
#define PUMP_FINISH_TITLE_Y                                    (222)
#define PUMP_FINISH_CONTENT_X                                  (0)
#define PUMP_FINISH_CONTENT_Y                                  (260)
#if IS_SUPPORT_PUMP_FINISH_COMP_BG
#define FINISH_TIP_TITLE_COLOR                                 (0x00FF00)
#define FINISH_TIP_CONTENT_COLOR                               (0x00FF00)
#else
#define FINISH_TIP_TITLE_COLOR                                 (0xA0183B)
#define FINISH_TIP_CONTENT_COLOR                               (0xBD4060)
#endif
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_finish_custom_ui_info_t *finish_info;

const static pump_finish_custom_text_info_t complete_tip_text_infos[PUMP_FINISH_CUSTOM_TIP_TEXT_COUNT] = {
    { "Finish", PUMP_FINISH_TITLE_X, PUMP_FINISH_TITLE_Y, &lv_font_aeonik_soft_19_medium, FINISH_TIP_TITLE_COLOR},
    { "Open the Momcozy app\nto track your milk output", PUMP_FINISH_CONTENT_X, PUMP_FINISH_CONTENT_Y,
        &lv_font_aeonik_soft_19_medium, FINISH_TIP_CONTENT_COLOR},
};

#if IS_SUPPORT_PUMP_FINISH_COMP_BG
static PSRAM_DATA_LOC uint8_t pump_finish_bg_data[] = {
    #include "comparison_png2txt/pump_finish11_360x360.txt"
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
    lv_obj_set_size(finish_info->root, LV_PCT(100), LV_PCT(100));
    // lv_obj_set_style_bg_color(finish_info->root, lv_color_hex(0xEDE3E5), 0);
    lv_obj_set_style_bg_color(finish_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(finish_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(finish_info->root, 0, 0);
    lv_obj_set_style_pad_all(finish_info->root, 0, 0);
    lv_obj_align(finish_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_complete_icon_update(ui_manager_context_t *thiz)
{
    if (finish_info->icon == NULL || !lv_obj_is_valid(finish_info->icon)) {
        finish_info->icon = lv_image_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->icon);
    }
    ui_img_baj_theme_set_src(thiz, finish_info->icon, CUSTOM_RES_ID_PUMP_FINISH_IP,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(finish_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_FINISH_ICON_Y);
}

static void pump_complete_tip_texts_update(ui_manager_context_t *thiz, uint8_t index)
{
    if (finish_info->texts[index] == NULL || !lv_obj_is_valid(finish_info->texts[index])) {
        finish_info->texts[index] = lv_label_create(finish_info->root);
    }
    lv_label_set_text(finish_info->texts[index], complete_tip_text_infos[index].name);
    lv_obj_set_style_text_color(finish_info->texts[index], lv_color_hex(complete_tip_text_infos[index].color), 0);
    lv_obj_set_style_text_font(finish_info->texts[index], complete_tip_text_infos[index].font, 0);
    lv_obj_set_style_text_line_space(finish_info->texts[index], 4, 0);
    lv_obj_set_style_text_align(finish_info->texts[index], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(finish_info->texts[index], LV_ALIGN_TOP_MID,
        complete_tip_text_infos[index].text_x, complete_tip_text_infos[index].text_y);
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
        ui_img_png_set_src(finish_info->background, pump_finish_bg_data,
            sizeof(pump_finish_bg_data), UI_IMG_PNG_DECODE_RGBA8888);
        lv_obj_align(finish_info->background, LV_ALIGN_CENTER, 0, 0);
    }
    if (status == 0) {
        lv_obj_add_flag(finish_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(finish_info->background, LV_OBJ_FLAG_HIDDEN);
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
        finish_info = (pump_finish_custom_ui_info_t *)u_malloz(sizeof(pump_finish_custom_ui_info_t));
        EXCEPTION_CHECK_VOID(finish_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_complete_tip_root_init();
    finish_info->force_update = true;
}

void pump_finish_custom_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && finish_info && finish_info->root);
    ui_img_baj_release_decoded_buf(finish_info->icon);
#if IS_SUPPORT_PUMP_FINISH_COMP_BG
    ui_img_png_release_decoded_buf(finish_info->background);
#endif
    LVGL_OBJ_DELETE(finish_info->root);
    u_free(finish_info);
    finish_info = NULL;
}

int pump_finish_custom_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && finish_info && finish_info->root);
    pump_finish_root_bg_update(thiz);
    if (finish_info->force_update == true) {
        pump_complete_icon_update(thiz);
        for (uint8_t i = 0; i < PUMP_FINISH_CUSTOM_TIP_TEXT_COUNT; i++) {
            pump_complete_tip_texts_update(thiz, i);
        }
        finish_info->force_update = false;
    }
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump complete fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
