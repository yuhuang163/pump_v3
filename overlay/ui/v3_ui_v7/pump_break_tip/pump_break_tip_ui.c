/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_break_tip_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_BREAK_TIP_ICON_Y                             (113)
#define PUMP_BREAK_TIP_TITLE_Y                            (182)
#define PUMP_BREAK_TIP_TITLE_COLOR                        (0xFFFFFF)
#define PUMP_BREAK_TIP_TITLE_LINE_SPACE                   (4)
#define PUMP_BREAK_TIP_TITLE_TEXT                         "Pumping over 30 min.\nTake a short break."

#define PUMP_BREAK_TIP_COUNTDOWN_Y                        (324)
#define PUMP_BREAK_TIP_COUNTDOWN_COLOR                    (0x999999)
#define PUMP_BREAK_TIP_COUNTDOWN_MAX                      (3)
#define PUMP_BREAK_TIP_DURATION_MS                        (3000)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *countdown;
    lv_draw_buf_t *icon_buf;
    uint32_t start_time;
    uint8_t shown_remain_sec;
    uint8_t force_update;
    uint8_t rsv[6];
} pump_break_tip_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_break_tip_ui_info_t *break_tip_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_break_tip_root_init(void)
{
    if (break_tip_info->root == NULL || !lv_obj_is_valid(break_tip_info->root)) {
        break_tip_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(break_tip_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(break_tip_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(break_tip_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(break_tip_info->root, 0, 0);
    lv_obj_set_style_pad_all(break_tip_info->root, 0, 0);
    lv_obj_align(break_tip_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(break_tip_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(break_tip_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_break_tip_icon_update(ui_manager_context_t *thiz)
{
    if (break_tip_info->force_update == false) {
        return;
    }

    if (break_tip_info->icon == NULL || !lv_obj_is_valid(break_tip_info->icon)) {
        break_tip_info->icon = lv_image_create(break_tip_info->root);
        EXCEPTION_CHECK_VOID(break_tip_info->icon);
    }
    if (break_tip_info->icon_buf == NULL) {
        break_tip_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_RELAX_TIP_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (break_tip_info->icon_buf != NULL) {
        lv_image_set_src(break_tip_info->icon, break_tip_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, break_tip_info->icon, BASE_RES_ID_RELAX_TIP_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(break_tip_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_BREAK_TIP_ICON_Y);
}

static void pump_break_tip_title_update(void)
{
    if (break_tip_info->force_update == false) {
        return;
    }

    if (break_tip_info->title == NULL || !lv_obj_is_valid(break_tip_info->title)) {
        break_tip_info->title = lv_label_create(break_tip_info->root);
        EXCEPTION_CHECK_VOID(break_tip_info->title);
    }
    lv_label_set_text(break_tip_info->title, PUMP_BREAK_TIP_TITLE_TEXT);
    lv_obj_set_style_text_color(break_tip_info->title, lv_color_hex(PUMP_BREAK_TIP_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(break_tip_info->title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(break_tip_info->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(break_tip_info->title, PUMP_BREAK_TIP_TITLE_LINE_SPACE, 0);
    lv_obj_align(break_tip_info->title, LV_ALIGN_TOP_MID, 0, PUMP_BREAK_TIP_TITLE_Y);
}

static uint8_t pump_break_tip_remain_sec_get(uint32_t elapsed_ms)
{
    uint8_t remain;

    if (elapsed_ms >= PUMP_BREAK_TIP_DURATION_MS) {
        return 1;
    }
    remain = (uint8_t)(PUMP_BREAK_TIP_COUNTDOWN_MAX -
        ((elapsed_ms * PUMP_BREAK_TIP_COUNTDOWN_MAX) / PUMP_BREAK_TIP_DURATION_MS));
    if (remain < 1) {
        remain = 1;
    }
    if (remain > PUMP_BREAK_TIP_COUNTDOWN_MAX) {
        remain = PUMP_BREAK_TIP_COUNTDOWN_MAX;
    }
    return remain;
}

static void pump_break_tip_countdown_update(void)
{
    char num_buf[8] = {0};
    uint32_t elapsed;
    uint8_t remain_sec;

    if (break_tip_info->countdown == NULL || !lv_obj_is_valid(break_tip_info->countdown)) {
        break_tip_info->countdown = lv_label_create(break_tip_info->root);
        EXCEPTION_CHECK_VOID(break_tip_info->countdown);
        break_tip_info->shown_remain_sec = 0xFF;
    }

    elapsed = get_timestamp_ms(break_tip_info->start_time);
    remain_sec = pump_break_tip_remain_sec_get(elapsed);

    if (break_tip_info->force_update == false &&
        break_tip_info->shown_remain_sec == remain_sec) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), "%u s", remain_sec);
    lv_label_set_text(break_tip_info->countdown, num_buf);
    lv_obj_set_style_text_color(break_tip_info->countdown,
        lv_color_hex(PUMP_BREAK_TIP_COUNTDOWN_COLOR), 0);
    lv_obj_set_style_text_font(break_tip_info->countdown, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(break_tip_info->countdown, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(break_tip_info->countdown, LV_ALIGN_TOP_MID, 0, PUMP_BREAK_TIP_COUNTDOWN_Y);
    break_tip_info->shown_remain_sec = remain_sec;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_break_tip_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (break_tip_info == NULL) {
        break_tip_info = (pump_break_tip_ui_info_t *)u_malloz(sizeof(pump_break_tip_ui_info_t));
        EXCEPTION_CHECK_VOID(break_tip_info);
    }
    break_tip_info->force_update = true;
    break_tip_info->shown_remain_sec = 0xFF;
    break_tip_info->start_time = get_timestamp();
    pump_break_tip_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_break_tip_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && break_tip_info && break_tip_info->root);
    if (break_tip_info->icon_buf != NULL) {
        if (break_tip_info->icon != NULL && lv_obj_is_valid(break_tip_info->icon)) {
            lv_image_set_src(break_tip_info->icon, NULL);
        }
        lv_draw_buf_destroy(break_tip_info->icon_buf);
        break_tip_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(break_tip_info->icon);
    }
    LVGL_OBJ_DELETE(break_tip_info->root);
    u_free(break_tip_info);
    break_tip_info = NULL;
}

int ui_display_pump_break_tip_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && break_tip_info && break_tip_info->root);
    pump_break_tip_icon_update(thiz);
    pump_break_tip_title_update();
    pump_break_tip_countdown_update();
    break_tip_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump break tip fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
