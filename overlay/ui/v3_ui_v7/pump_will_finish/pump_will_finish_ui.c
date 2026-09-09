/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep
#include "pump_will_finish_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_WILL_FINISH_TITLE_COLOR                  (0xFFFFFF)
#define PUMP_WILL_FINISH_TITLE2_COLOR                 (0x999999)
#define PUMP_WILL_FINISH_COUNTDOWN_COLOR             (0x999999)

#define PUMP_WILL_FINISH_DURATION_MS                (10000)
#define PUMP_WILL_FINISH_COUNTDOWN_MAX              (10)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *icon;
    lv_obj_t *title1;
    lv_obj_t *title2;
    lv_obj_t *countdown;
    lv_draw_buf_t *icon_buf;
    uint32_t start_time;
    uint8_t shown_remain_sec;
    uint8_t force_update;
    uint8_t rsv[6];
} pump_will_finish_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_will_finish_ui_info_t *will_finish_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_will_finish_root_init(void)
{
    if (will_finish_info->root == NULL || !lv_obj_is_valid(will_finish_info->root)) {
        will_finish_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(will_finish_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(will_finish_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(will_finish_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(will_finish_info->root, 0, 0);
    lv_obj_set_style_pad_all(will_finish_info->root, 0, 0);
    lv_obj_align(will_finish_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(will_finish_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(will_finish_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_will_finish_icon_update(ui_manager_context_t *thiz)
{
    if (will_finish_info->force_update == false) {
        return;
    }

    if (will_finish_info->icon == NULL || !lv_obj_is_valid(will_finish_info->icon)) {
        will_finish_info->icon = lv_image_create(will_finish_info->root);
        EXCEPTION_CHECK_VOID(will_finish_info->icon);
    }
    if (will_finish_info->icon_buf == NULL) {
        will_finish_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_PUMP_WILL_FINISH_TIP_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (will_finish_info->icon_buf != NULL) {
        lv_image_set_src(will_finish_info->icon, will_finish_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, will_finish_info->icon, BASE_RES_ID_QUANDARY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(will_finish_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_WILL_FINISH_ICON_Y);
}

static void pump_will_finish_title_update(void)
{
    if (will_finish_info->force_update == false) {
        return;
    }

    if (will_finish_info->title1 == NULL || !lv_obj_is_valid(will_finish_info->title1)) {
        will_finish_info->title1 = lv_label_create(will_finish_info->root);
        EXCEPTION_CHECK_VOID(will_finish_info->title1);
    }
    lv_label_set_text(will_finish_info->title1, PUMP_WILL_FINISH_TITLE1_TEXT);
    lv_obj_set_style_text_color(will_finish_info->title1,
        lv_color_hex(PUMP_WILL_FINISH_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(will_finish_info->title1, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(will_finish_info->title1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(will_finish_info->title1, 8, 0);
    lv_obj_align(will_finish_info->title1, LV_ALIGN_TOP_MID, 0, PUMP_WILL_FINISH_TITLE1_Y);

    if (will_finish_info->title2 == NULL || !lv_obj_is_valid(will_finish_info->title2)) {
        will_finish_info->title2 = lv_label_create(will_finish_info->root);
        EXCEPTION_CHECK_VOID(will_finish_info->title2);
    }
    lv_label_set_text(will_finish_info->title2, PUMP_WILL_FINISH_TITLE2_TEXT);
    lv_obj_set_style_text_color(will_finish_info->title2,
        lv_color_hex(PUMP_WILL_FINISH_TITLE2_COLOR), 0);
    lv_obj_set_style_text_font(will_finish_info->title2, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(will_finish_info->title2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(will_finish_info->title2, will_finish_info->title1,
        LV_ALIGN_OUT_BOTTOM_MID, 0, PUMP_WILL_FINISH_TITLE_GAP);
    lv_obj_set_style_text_line_space(will_finish_info->title2, 8, 0);
}

static void pump_will_finish_countdown_update(void)
{
    char num_buf[8] = {0};
    uint32_t elapsed;
    uint8_t remain_sec;

    elapsed = get_timestamp_ms(will_finish_info->start_time);
    remain_sec = ui_progress_arc_remain_sec_get(elapsed, PUMP_WILL_FINISH_DURATION_MS,
        PUMP_WILL_FINISH_COUNTDOWN_MAX);

    if (will_finish_info->countdown == NULL || !lv_obj_is_valid(will_finish_info->countdown)) {
        will_finish_info->countdown = lv_label_create(will_finish_info->root);
        EXCEPTION_CHECK_VOID(will_finish_info->countdown);
        will_finish_info->shown_remain_sec = 0xFF;
    }

    if (will_finish_info->force_update == false &&
        will_finish_info->shown_remain_sec == remain_sec) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), UI_STR_COUNTDOWN_SEC_FMT, remain_sec);
    lv_label_set_text(will_finish_info->countdown, num_buf);
    lv_obj_set_style_text_color(will_finish_info->countdown,
        lv_color_hex(PUMP_WILL_FINISH_COUNTDOWN_COLOR), 0);
    lv_obj_set_style_text_font(will_finish_info->countdown, UI_FONT_HINT, 0);
    lv_obj_set_style_text_align(will_finish_info->countdown, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(will_finish_info->countdown, LV_ALIGN_TOP_MID, 0,
        UI_PROGRESS_ARC_COUNTDOWN_Y_DEFAULT);
    will_finish_info->shown_remain_sec = remain_sec;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_will_finish_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (will_finish_info == NULL) {
        will_finish_info = (pump_will_finish_ui_info_t *)u_malloz(sizeof(pump_will_finish_ui_info_t));
        EXCEPTION_CHECK_VOID(will_finish_info);
    }
    will_finish_info->force_update = true;
    will_finish_info->start_time = get_timestamp();
    will_finish_info->shown_remain_sec = 0xFF;
    pump_will_finish_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_will_finish_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && will_finish_info && will_finish_info->root);
    if (will_finish_info->icon_buf != NULL) {
        if (will_finish_info->icon != NULL && lv_obj_is_valid(will_finish_info->icon)) {
            lv_image_set_src(will_finish_info->icon, NULL);
        }
        lv_draw_buf_destroy(will_finish_info->icon_buf);
        will_finish_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(will_finish_info->icon);
    }
    LVGL_OBJ_DELETE(will_finish_info->root);
    u_free(will_finish_info);
    will_finish_info = NULL;
}

int ui_display_pump_will_finish_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && will_finish_info && will_finish_info->root);
    pump_will_finish_icon_update(thiz);
    pump_will_finish_title_update();
    pump_will_finish_countdown_update();
    will_finish_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump will finish fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
