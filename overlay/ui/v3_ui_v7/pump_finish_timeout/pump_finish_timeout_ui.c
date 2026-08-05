/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_finish_timeout_ui.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_FINISH_TIMEOUT_ARC_DONE_COLOR             (0xEF4C4E)

#define PUMP_FINISH_TIMEOUT_ICON_Y                     (77)
#define PUMP_FINISH_TIMEOUT_TITLE1_Y                   (183 + 2)
#define PUMP_FINISH_TIMEOUT_TITLE_GAP                  (10 + 2)
#define PUMP_FINISH_TIMEOUT_TITLE_COLOR                (0xFFFFFF)
#define PUMP_FINISH_TIMEOUT_TITLE1_TEXT                "Pumping Ended"
#define PUMP_FINISH_TIMEOUT_TITLE2_TEXT                "Automatically\nending..."

/** 页面展示倒计时（与 stop_hold 一致，文案为 3s/2s/1s） */
#define PUMP_FINISH_TIMEOUT_DURATION_MS                (2000)
#define PUMP_FINISH_TIMEOUT_COUNTDOWN_MAX              (3)

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
} pump_finish_timeout_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_finish_timeout_ui_info_t *finish_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_finish_timeout_root_init(void)
{
    if (finish_info->root == NULL || !lv_obj_is_valid(finish_info->root)) {
        finish_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(finish_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(finish_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(finish_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(finish_info->root, 0, 0);
    lv_obj_set_style_pad_all(finish_info->root, 0, 0);
    lv_obj_align(finish_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(finish_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_finish_timeout_arc_update(void)
{
    ui_progress_arc_style_t style;

    ui_progress_arc_style_init(&style, PUMP_FINISH_TIMEOUT_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&finish_info->arc, finish_info->root, &style,
        finish_info->start_time, PUMP_FINISH_TIMEOUT_DURATION_MS,
        PUMP_FINISH_TIMEOUT_COUNTDOWN_MAX, finish_info->force_update);
}

static void pump_finish_timeout_icon_update(ui_manager_context_t *thiz)
{
    if (finish_info->force_update == false) {
        return;
    }

    if (finish_info->icon == NULL || !lv_obj_is_valid(finish_info->icon)) {
        finish_info->icon = lv_image_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->icon);
    }
    if (finish_info->icon_buf == NULL) {
        finish_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_POWER_KEY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (finish_info->icon_buf != NULL) {
        lv_image_set_src(finish_info->icon, finish_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, finish_info->icon, BASE_RES_ID_POWER_KEY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(finish_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_FINISH_TIMEOUT_ICON_Y);
}

static void pump_finish_timeout_title_update(void)
{
    if (finish_info->force_update == false) {
        return;
    }

    if (finish_info->title1 == NULL || !lv_obj_is_valid(finish_info->title1)) {
        finish_info->title1 = lv_label_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->title1);
    }
    lv_label_set_text(finish_info->title1, PUMP_FINISH_TIMEOUT_TITLE1_TEXT);
    lv_obj_set_style_text_color(finish_info->title1, lv_color_hex(PUMP_FINISH_TIMEOUT_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(finish_info->title1, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(finish_info->title1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(finish_info->title1, LV_ALIGN_TOP_MID, 0, PUMP_FINISH_TIMEOUT_TITLE1_Y);
    lv_obj_set_style_text_line_space(finish_info->title1, 8, 0);

    if (finish_info->title2 == NULL || !lv_obj_is_valid(finish_info->title2)) {
        finish_info->title2 = lv_label_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->title2);
    }
    lv_label_set_text(finish_info->title2, PUMP_FINISH_TIMEOUT_TITLE2_TEXT);
    lv_obj_set_style_text_color(finish_info->title2, lv_color_hex(PUMP_FINISH_TIMEOUT_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(finish_info->title2, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(finish_info->title2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(finish_info->title2, finish_info->title1, LV_ALIGN_OUT_BOTTOM_MID,
        0, PUMP_FINISH_TIMEOUT_TITLE_GAP);
    lv_obj_set_style_text_line_space(finish_info->title2, 8, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_finish_timeout_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (finish_info == NULL) {
        finish_info = (pump_finish_timeout_ui_info_t *)u_malloz(sizeof(pump_finish_timeout_ui_info_t));
        EXCEPTION_CHECK_VOID(finish_info);
    }
    finish_info->force_update = true;
    finish_info->completed = false;
    finish_info->start_time = get_timestamp();
    ui_progress_arc_reset(&finish_info->arc);
    pump_finish_timeout_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_finish_timeout_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && finish_info && finish_info->root);
    if (finish_info->icon_buf != NULL) {
        if (finish_info->icon != NULL && lv_obj_is_valid(finish_info->icon)) {
            lv_image_set_src(finish_info->icon, NULL);
        }
        lv_draw_buf_destroy(finish_info->icon_buf);
        finish_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(finish_info->icon);
    }
    LVGL_OBJ_DELETE(finish_info->root);
    u_free(finish_info);
    finish_info = NULL;
}

int ui_display_pump_finish_timeout_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && finish_info && finish_info->root);
    pump_finish_timeout_arc_update();
    pump_finish_timeout_icon_update(thiz);
    pump_finish_timeout_title_update();
    finish_info->force_update = false;

    if (finish_info->completed == false &&
        get_timestamp_ms(finish_info->start_time) >= PUMP_FINISH_TIMEOUT_DURATION_MS) {
        finish_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump finish timeout fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
