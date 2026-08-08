/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_pause_timeout_ui.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_PAUSE_TIMEOUT_ARC_DONE_COLOR             (0xEF4C4E)

#define PUMP_PAUSE_TIMEOUT_ICON_Y                     (77)
#define PUMP_PAUSE_TIMEOUT_TITLE1_Y                   (183 + 2)
#define PUMP_PAUSE_TIMEOUT_TITLE_GAP                  (10 + 2)
#define PUMP_PAUSE_TIMEOUT_TITLE_COLOR                (0xFFFFFF)
#define PUMP_PAUSE_TIMEOUT_TITLE1_TEXT                "Pumping Ended"
#define PUMP_PAUSE_TIMEOUT_TITLE2_TEXT                "Automatically\nending..."

/** 页面展示倒计时（与 finish_timeout 一致，文案为 3s/2s/1s） */
#define PUMP_PAUSE_TIMEOUT_DURATION_MS                (2000)
#define PUMP_PAUSE_TIMEOUT_COUNTDOWN_MAX              (3)

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
} pump_pause_timeout_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_pause_timeout_ui_info_t *pause_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_pause_timeout_root_init(void)
{
    if (pause_info->root == NULL || !lv_obj_is_valid(pause_info->root)) {
        pause_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(pause_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(pause_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(pause_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pause_info->root, 0, 0);
    lv_obj_set_style_pad_all(pause_info->root, 0, 0);
    lv_obj_align(pause_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(pause_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(pause_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_pause_timeout_arc_update(void)
{
    ui_progress_arc_style_t style;

    ui_progress_arc_style_init(&style, PUMP_PAUSE_TIMEOUT_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&pause_info->arc, pause_info->root, &style,
        pause_info->start_time, PUMP_PAUSE_TIMEOUT_DURATION_MS,
        PUMP_PAUSE_TIMEOUT_COUNTDOWN_MAX, pause_info->force_update);
}

static void pump_pause_timeout_icon_update(ui_manager_context_t *thiz)
{
    if (pause_info->force_update == false) {
        return;
    }

    if (pause_info->icon == NULL || !lv_obj_is_valid(pause_info->icon)) {
        pause_info->icon = lv_image_create(pause_info->root);
        EXCEPTION_CHECK_VOID(pause_info->icon);
    }
    if (pause_info->icon_buf == NULL) {
        pause_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_POWER_KEY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (pause_info->icon_buf != NULL) {
        lv_image_set_src(pause_info->icon, pause_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, pause_info->icon, BASE_RES_ID_POWER_KEY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(pause_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_PAUSE_TIMEOUT_ICON_Y);
}

static void pump_pause_timeout_title_update(void)
{
    if (pause_info->force_update == false) {
        return;
    }

    if (pause_info->title1 == NULL || !lv_obj_is_valid(pause_info->title1)) {
        pause_info->title1 = lv_label_create(pause_info->root);
        EXCEPTION_CHECK_VOID(pause_info->title1);
    }
    lv_label_set_text(pause_info->title1, PUMP_PAUSE_TIMEOUT_TITLE1_TEXT);
    lv_obj_set_style_text_color(pause_info->title1, lv_color_hex(PUMP_PAUSE_TIMEOUT_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(pause_info->title1, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(pause_info->title1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(pause_info->title1, LV_ALIGN_TOP_MID, 0, PUMP_PAUSE_TIMEOUT_TITLE1_Y);
    lv_obj_set_style_text_line_space(pause_info->title1, 8, 0);

    if (pause_info->title2 == NULL || !lv_obj_is_valid(pause_info->title2)) {
        pause_info->title2 = lv_label_create(pause_info->root);
        EXCEPTION_CHECK_VOID(pause_info->title2);
    }
    lv_label_set_text(pause_info->title2, PUMP_PAUSE_TIMEOUT_TITLE2_TEXT);
    lv_obj_set_style_text_color(pause_info->title2, lv_color_hex(PUMP_PAUSE_TIMEOUT_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(pause_info->title2, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(pause_info->title2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(pause_info->title2, pause_info->title1, LV_ALIGN_OUT_BOTTOM_MID,
        0, PUMP_PAUSE_TIMEOUT_TITLE_GAP);
    lv_obj_set_style_text_line_space(pause_info->title2, 8, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_pause_timeout_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pause_info == NULL) {
        pause_info = (pump_pause_timeout_ui_info_t *)u_malloz(sizeof(pump_pause_timeout_ui_info_t));
        EXCEPTION_CHECK_VOID(pause_info);
    }
    pause_info->force_update = true;
    pause_info->completed = false;
    pause_info->start_time = get_timestamp();
    ui_progress_arc_reset(&pause_info->arc);
    pump_pause_timeout_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_pause_timeout_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && pause_info && pause_info->root);
    if (pause_info->icon_buf != NULL) {
        if (pause_info->icon != NULL && lv_obj_is_valid(pause_info->icon)) {
            lv_image_set_src(pause_info->icon, NULL);
        }
        lv_draw_buf_destroy(pause_info->icon_buf);
        pause_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(pause_info->icon);
    }
    LVGL_OBJ_DELETE(pause_info->root);
    u_free(pause_info);
    pause_info = NULL;
}

int ui_display_pump_pause_timeout_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && pause_info && pause_info->root);
    pump_pause_timeout_arc_update();
    pump_pause_timeout_icon_update(thiz);
    pump_pause_timeout_title_update();
    pause_info->force_update = false;

    if (pause_info->completed == false &&
        get_timestamp_ms(pause_info->start_time) >= PUMP_PAUSE_TIMEOUT_DURATION_MS) {
        pause_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump pause timeout fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
