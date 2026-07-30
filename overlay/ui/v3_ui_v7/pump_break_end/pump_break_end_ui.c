/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_break_end_ui.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_BREAK_END_ARC_DONE_COLOR             (0xEF4C4E)

#define PUMP_BREAK_END_ICON_Y                     (94)
#define PUMP_BREAK_END_TITLE1_Y                   (202)
#define PUMP_BREAK_END_TITLE_GAP                  (10)
#define PUMP_BREAK_END_TITLE_COLOR                (0xFFFFFF)
#define PUMP_BREAK_END_TITLE1_TEXT                "Stop pumping"
#define PUMP_BREAK_END_TITLE2_TEXT                "Ending automatically"

/** 页面展示倒计时（与 stop_hold 一致，文案为 3s/2s/1s） */
#define PUMP_BREAK_END_DURATION_MS                (2000)
#define PUMP_BREAK_END_COUNTDOWN_MAX              (3)

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
} pump_break_end_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_break_end_ui_info_t *break_end_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_break_end_root_init(void)
{
    if (break_end_info->root == NULL || !lv_obj_is_valid(break_end_info->root)) {
        break_end_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(break_end_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(break_end_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(break_end_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(break_end_info->root, 0, 0);
    lv_obj_set_style_pad_all(break_end_info->root, 0, 0);
    lv_obj_align(break_end_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(break_end_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(break_end_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_break_end_arc_update(void)
{
    ui_progress_arc_style_t style;

    ui_progress_arc_style_init(&style, PUMP_BREAK_END_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&break_end_info->arc, break_end_info->root, &style,
        break_end_info->start_time, PUMP_BREAK_END_DURATION_MS,
        PUMP_BREAK_END_COUNTDOWN_MAX, break_end_info->force_update);
}

static void pump_break_end_icon_update(ui_manager_context_t *thiz)
{
    if (break_end_info->force_update == false) {
        return;
    }

    if (break_end_info->icon == NULL || !lv_obj_is_valid(break_end_info->icon)) {
        break_end_info->icon = lv_image_create(break_end_info->root);
        EXCEPTION_CHECK_VOID(break_end_info->icon);
    }
    if (break_end_info->icon_buf == NULL) {
        break_end_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_POWER_KEY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (break_end_info->icon_buf != NULL) {
        lv_image_set_src(break_end_info->icon, break_end_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, break_end_info->icon, BASE_RES_ID_POWER_KEY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(break_end_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_BREAK_END_ICON_Y);
}

static void pump_break_end_title_update(void)
{
    if (break_end_info->force_update == false) {
        return;
    }

    if (break_end_info->title1 == NULL || !lv_obj_is_valid(break_end_info->title1)) {
        break_end_info->title1 = lv_label_create(break_end_info->root);
        EXCEPTION_CHECK_VOID(break_end_info->title1);
    }
    lv_label_set_text(break_end_info->title1, PUMP_BREAK_END_TITLE1_TEXT);
    lv_obj_set_style_text_color(break_end_info->title1, lv_color_hex(PUMP_BREAK_END_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(break_end_info->title1, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(break_end_info->title1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(break_end_info->title1, LV_ALIGN_TOP_MID, 0, PUMP_BREAK_END_TITLE1_Y);

    if (break_end_info->title2 == NULL || !lv_obj_is_valid(break_end_info->title2)) {
        break_end_info->title2 = lv_label_create(break_end_info->root);
        EXCEPTION_CHECK_VOID(break_end_info->title2);
    }
    lv_label_set_text(break_end_info->title2, PUMP_BREAK_END_TITLE2_TEXT);
    lv_obj_set_style_text_color(break_end_info->title2, lv_color_hex(PUMP_BREAK_END_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(break_end_info->title2, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(break_end_info->title2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(break_end_info->title2, break_end_info->title1, LV_ALIGN_OUT_BOTTOM_MID,
        0, PUMP_BREAK_END_TITLE_GAP);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_break_end_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (break_end_info == NULL) {
        break_end_info = (pump_break_end_ui_info_t *)u_malloz(sizeof(pump_break_end_ui_info_t));
        EXCEPTION_CHECK_VOID(break_end_info);
    }
    break_end_info->force_update = true;
    break_end_info->completed = false;
    break_end_info->start_time = get_timestamp();
    ui_progress_arc_reset(&break_end_info->arc);
    pump_break_end_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_break_end_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && break_end_info && break_end_info->root);
    if (break_end_info->icon_buf != NULL) {
        if (break_end_info->icon != NULL && lv_obj_is_valid(break_end_info->icon)) {
            lv_image_set_src(break_end_info->icon, NULL);
        }
        lv_draw_buf_destroy(break_end_info->icon_buf);
        break_end_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(break_end_info->icon);
    }
    LVGL_OBJ_DELETE(break_end_info->root);
    u_free(break_end_info);
    break_end_info = NULL;
}

int ui_display_pump_break_end_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && break_end_info && break_end_info->root);
    pump_break_end_arc_update();
    pump_break_end_icon_update(thiz);
    pump_break_end_title_update();
    break_end_info->force_update = false;

    if (break_end_info->completed == false &&
        get_timestamp_ms(break_end_info->start_time) >= PUMP_BREAK_END_DURATION_MS) {
        break_end_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump break end fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
