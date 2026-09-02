/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_finish_charge_error_ui.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_FINISH_CHARGE_ERROR_ARC_DONE_COLOR        (0xEF4C4E)

#define PUMP_FINISH_CHARGE_ERROR_TITLE1_COLOR          (0xFFFFFF)
#define PUMP_FINISH_CHARGE_ERROR_TITLE2_COLOR          (0x999999)
#define PUMP_FINISH_CHARGE_ERROR_COUNTDOWN_Y           (332)

/** 页面展示倒计时（文案为 3s/2s/1s） */
#define PUMP_FINISH_CHARGE_ERROR_DURATION_MS           (3000)
#define PUMP_FINISH_CHARGE_ERROR_COUNTDOWN_MAX         (3)

typedef struct {
    lv_obj_t *root;
    ui_progress_arc_info_t arc;
    lv_obj_t *icon;
    lv_obj_t *title1;
    lv_obj_t *title2;
    lv_draw_buf_t *icon_buf;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t completed;
    uint8_t rsv[10];
} pump_finish_charge_error_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_finish_charge_error_ui_info_t *charge_error_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_finish_charge_error_root_init(void)
{
    if (charge_error_info->root == NULL || !lv_obj_is_valid(charge_error_info->root)) {
        charge_error_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(charge_error_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(charge_error_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(charge_error_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(charge_error_info->root, 0, 0);
    lv_obj_set_style_pad_all(charge_error_info->root, 0, 0);
    lv_obj_align(charge_error_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(charge_error_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(charge_error_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_finish_charge_error_arc_update(void)
{
    ui_progress_arc_style_t style;

    ui_progress_arc_style_init(&style, PUMP_FINISH_CHARGE_ERROR_ARC_DONE_COLOR);
    style.show_countdown = true;
    style.countdown_y = PUMP_FINISH_CHARGE_ERROR_COUNTDOWN_Y;
    ui_progress_arc_time_update(&charge_error_info->arc, charge_error_info->root, &style,
        charge_error_info->start_time, PUMP_FINISH_CHARGE_ERROR_DURATION_MS,
        PUMP_FINISH_CHARGE_ERROR_COUNTDOWN_MAX, charge_error_info->force_update);
}

static void pump_finish_charge_error_icon_update(ui_manager_context_t *thiz)
{
    if (charge_error_info->force_update == false) {
        return;
    }

    if (charge_error_info->icon == NULL || !lv_obj_is_valid(charge_error_info->icon)) {
        charge_error_info->icon = lv_image_create(charge_error_info->root);
        EXCEPTION_CHECK_VOID(charge_error_info->icon);
    }
    if (charge_error_info->icon_buf == NULL) {
        charge_error_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_POWER_KEY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (charge_error_info->icon_buf != NULL) {
        lv_image_set_src(charge_error_info->icon, charge_error_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, charge_error_info->icon, BASE_RES_ID_POWER_KEY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(charge_error_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_FINISH_CHARGE_ERROR_ICON_Y);
}

static void pump_finish_charge_error_title_update(void)
{
    if (charge_error_info->force_update == false) {
        return;
    }

    if (charge_error_info->title1 == NULL || !lv_obj_is_valid(charge_error_info->title1)) {
        charge_error_info->title1 = lv_label_create(charge_error_info->root);
        EXCEPTION_CHECK_VOID(charge_error_info->title1);
    }
    lv_label_set_text(charge_error_info->title1, PUMP_FINISH_CHARGE_ERROR_TITLE1_TEXT);
    lv_obj_set_style_text_color(charge_error_info->title1,
        lv_color_hex(PUMP_FINISH_CHARGE_ERROR_TITLE1_COLOR), 0);
    lv_obj_set_style_text_font(charge_error_info->title1, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(charge_error_info->title1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(charge_error_info->title1, LV_ALIGN_TOP_MID, 0,
        PUMP_FINISH_CHARGE_ERROR_TITLE1_Y);

    if (charge_error_info->title2 == NULL || !lv_obj_is_valid(charge_error_info->title2)) {
        charge_error_info->title2 = lv_label_create(charge_error_info->root);
        EXCEPTION_CHECK_VOID(charge_error_info->title2);
    }
    lv_label_set_text(charge_error_info->title2, PUMP_FINISH_CHARGE_ERROR_TITLE2_TEXT);
    lv_obj_set_style_text_color(charge_error_info->title2,
        lv_color_hex(PUMP_FINISH_CHARGE_ERROR_TITLE2_COLOR), 0);
    lv_obj_set_style_text_font(charge_error_info->title2, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(charge_error_info->title2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(charge_error_info->title2, LV_ALIGN_TOP_MID, 0,
        PUMP_FINISH_CHARGE_ERROR_TITLE2_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_finish_charge_error_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (charge_error_info == NULL) {
        charge_error_info = (pump_finish_charge_error_ui_info_t *)u_malloz(
            sizeof(pump_finish_charge_error_ui_info_t));
        EXCEPTION_CHECK_VOID(charge_error_info);
    }
    charge_error_info->force_update = true;
    charge_error_info->completed = false;
    charge_error_info->start_time = get_timestamp();
    ui_progress_arc_reset(&charge_error_info->arc);
    pump_finish_charge_error_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_finish_charge_error_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && charge_error_info && charge_error_info->root);
    if (charge_error_info->icon_buf != NULL) {
        if (charge_error_info->icon != NULL && lv_obj_is_valid(charge_error_info->icon)) {
            lv_image_set_src(charge_error_info->icon, NULL);
        }
        lv_draw_buf_destroy(charge_error_info->icon_buf);
        charge_error_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(charge_error_info->icon);
    }
    LVGL_OBJ_DELETE(charge_error_info->root);
    u_free(charge_error_info);
    charge_error_info = NULL;
}

int ui_display_pump_finish_charge_error_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && charge_error_info && charge_error_info->root);
    pump_finish_charge_error_arc_update();
    pump_finish_charge_error_icon_update(thiz);
    pump_finish_charge_error_title_update();
    charge_error_info->force_update = false;

    if (charge_error_info->completed == false &&
        get_timestamp_ms(charge_error_info->start_time) >= PUMP_FINISH_CHARGE_ERROR_DURATION_MS) {
        charge_error_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump finish charge error fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
