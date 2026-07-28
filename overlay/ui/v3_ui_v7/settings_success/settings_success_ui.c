/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "settings_success_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define SETTINGS_SUCCESS_ICON_Y                         (104)
#define SETTINGS_SUCCESS_TITLE_Y                        (212)
#define SETTINGS_SUCCESS_TITLE_COLOR                    (0xFFFFFF)
#define SETTINGS_SUCCESS_TITLE_TEXT                     "Success"

typedef struct {
    lv_obj_t *root;
    lv_obj_t *icon;
    lv_obj_t *title;
    uint8_t force_update;
    uint8_t rsv[15];
} settings_success_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static settings_success_ui_info_t *success_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void settings_success_root_init(void)
{
    if (success_info->root == NULL || !lv_obj_is_valid(success_info->root)) {
        success_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(success_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(success_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(success_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(success_info->root, 0, 0);
    lv_obj_set_style_pad_all(success_info->root, 0, 0);
    lv_obj_align(success_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(success_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(success_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void settings_success_icon_update(ui_manager_context_t *thiz)
{
    if (success_info->force_update == false) {
        return;
    }

    if (success_info->icon == NULL || !lv_obj_is_valid(success_info->icon)) {
        success_info->icon = lv_image_create(success_info->root);
        EXCEPTION_CHECK_VOID(success_info->icon);
    }
    ui_img_baj_theme_set_src(thiz, success_info->icon, BASE_RES_ID_SUCCESS_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(success_info->icon, LV_ALIGN_TOP_MID, 0, SETTINGS_SUCCESS_ICON_Y);
}

static void settings_success_title_update(void)
{
    if (success_info->force_update == false) {
        return;
    }

    if (success_info->title == NULL || !lv_obj_is_valid(success_info->title)) {
        success_info->title = lv_label_create(success_info->root);
        EXCEPTION_CHECK_VOID(success_info->title);
    }
    lv_label_set_text(success_info->title, SETTINGS_SUCCESS_TITLE_TEXT);
    lv_obj_set_style_text_color(success_info->title, lv_color_hex(SETTINGS_SUCCESS_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(success_info->title, &lv_font_aeonik_soft_36_medium, 0);
    lv_obj_set_style_text_align(success_info->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(success_info->title, LV_ALIGN_TOP_MID, 0, SETTINGS_SUCCESS_TITLE_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_settings_success_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (success_info == NULL) {
        success_info = (settings_success_ui_info_t *)u_malloz(sizeof(settings_success_ui_info_t));
        EXCEPTION_CHECK_VOID(success_info);
    }
    success_info->force_update = true;
    settings_success_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_settings_success_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && success_info && success_info->root);
    ui_img_baj_release_decoded_buf(success_info->icon);
    LVGL_OBJ_DELETE(success_info->root);
    u_free(success_info);
    success_info = NULL;
}

int ui_display_settings_success_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && success_info && success_info->root);
    settings_success_icon_update(thiz);
    settings_success_title_update();
    success_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> settings success fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
