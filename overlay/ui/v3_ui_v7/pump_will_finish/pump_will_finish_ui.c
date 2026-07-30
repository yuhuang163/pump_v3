/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_will_finish_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_WILL_FINISH_ARC_SIZE                   (352)
#define PUMP_WILL_FINISH_ARC_WIDTH                  (11)
#define PUMP_WILL_FINISH_ARC_DONE_COLOR             (0xEF4C4E)
#define PUMP_WILL_FINISH_ARC_REMAIN_COLOR           (0x252525)
/** 底部开口角度，使圆环不闭合（越大两端越高） */
#define PUMP_WILL_FINISH_ARC_BOTTOM_GAP_DEG         (100)
/** 已展示 / 剩余两段圆弧之间的角度偏差 */
#define PUMP_WILL_FINISH_ARC_SEG_GAP_DEG            (12)
/** 除去底部开口后的可绘制总角度 */
#define PUMP_WILL_FINISH_ARC_TRACK_DEG \
    (360 - PUMP_WILL_FINISH_ARC_BOTTOM_GAP_DEG)
/** 开口居中于底部：弧起点落在 gap 右侧 */
#define PUMP_WILL_FINISH_ARC_ROTATION \
    (90 + (PUMP_WILL_FINISH_ARC_BOTTOM_GAP_DEG / 2))

#define PUMP_WILL_FINISH_ICON_Y                     (94)
#define PUMP_WILL_FINISH_TITLE1_Y                   (202)
#define PUMP_WILL_FINISH_TITLE_GAP                  (10)
#define PUMP_WILL_FINISH_COUNTDOWN_Y                (334)
#define PUMP_WILL_FINISH_TITLE_COLOR                (0xFFFFFF)
#define PUMP_WILL_FINISH_COUNTDOWN_COLOR            (0x999999)
#define PUMP_WILL_FINISH_TITLE1_TEXT                "Pumping Timeout"
#define PUMP_WILL_FINISH_TITLE2_TEXT                "Ending automatically"

/** 页面展示倒计时（与 stop_hold 一致，文案为 3s/2s/1s） */
#define PUMP_WILL_FINISH_DURATION_MS                (2000)
#define PUMP_WILL_FINISH_COUNTDOWN_MAX              (3)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *done_arc;
    lv_obj_t *remain_arc;
    lv_obj_t *icon;
    lv_obj_t *title1;
    lv_obj_t *title2;
    lv_obj_t *countdown;
    lv_draw_buf_t *icon_buf;
    uint32_t start_time;
    uint8_t shown_remain_sec;
    uint8_t force_update;
    uint8_t completed;
    uint8_t rsv[9];
} pump_will_finish_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_will_finish_ui_info_t *finish_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_will_finish_root_init(void)
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

static void pump_will_finish_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, PUMP_WILL_FINISH_ARC_SIZE, PUMP_WILL_FINISH_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, PUMP_WILL_FINISH_ARC_ROTATION);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, PUMP_WILL_FINISH_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, PUMP_WILL_FINISH_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static void pump_will_finish_arc_update(void)
{
    uint32_t elapsed;
    uint16_t done_sweep;
    uint16_t remain_sweep;
    uint16_t remain_start;

    if (finish_info->done_arc == NULL || !lv_obj_is_valid(finish_info->done_arc)) {
        finish_info->done_arc = lv_arc_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->done_arc);
        pump_will_finish_arc_style_init(finish_info->done_arc, PUMP_WILL_FINISH_ARC_DONE_COLOR);
    }
    if (finish_info->remain_arc == NULL || !lv_obj_is_valid(finish_info->remain_arc)) {
        finish_info->remain_arc = lv_arc_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->remain_arc);
        pump_will_finish_arc_style_init(finish_info->remain_arc, PUMP_WILL_FINISH_ARC_REMAIN_COLOR);
    }

    elapsed = get_timestamp_ms(finish_info->start_time);
    if (elapsed >= PUMP_WILL_FINISH_DURATION_MS) {
        done_sweep = PUMP_WILL_FINISH_ARC_TRACK_DEG;
        remain_sweep = 0;
    } else {
        uint16_t split_track = PUMP_WILL_FINISH_ARC_TRACK_DEG - PUMP_WILL_FINISH_ARC_SEG_GAP_DEG;
        done_sweep = (uint16_t)((split_track * elapsed) / PUMP_WILL_FINISH_DURATION_MS);
        remain_sweep = split_track - done_sweep;
    }

    if (done_sweep > 0) {
        lv_arc_set_angles(finish_info->done_arc, 0, done_sweep);
        lv_obj_clear_flag(finish_info->done_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(finish_info->done_arc, 0, 0);
        lv_obj_add_flag(finish_info->done_arc, LV_OBJ_FLAG_HIDDEN);
    }

    if (remain_sweep > 0) {
        if (done_sweep > 0) {
            remain_start = done_sweep + PUMP_WILL_FINISH_ARC_SEG_GAP_DEG;
        } else {
            remain_start = 0;
            remain_sweep = PUMP_WILL_FINISH_ARC_TRACK_DEG;
        }
        lv_arc_set_angles(finish_info->remain_arc, remain_start, remain_start + remain_sweep);
        lv_obj_clear_flag(finish_info->remain_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(finish_info->remain_arc, 0, 0);
        lv_obj_add_flag(finish_info->remain_arc, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_invalidate(finish_info->done_arc);
    lv_obj_invalidate(finish_info->remain_arc);
}

static void pump_will_finish_icon_update(ui_manager_context_t *thiz)
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
    lv_obj_align(finish_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_WILL_FINISH_ICON_Y);
}

static void pump_will_finish_title_update(void)
{
    if (finish_info->force_update == false) {
        return;
    }

    if (finish_info->title1 == NULL || !lv_obj_is_valid(finish_info->title1)) {
        finish_info->title1 = lv_label_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->title1);
    }
    lv_label_set_text(finish_info->title1, PUMP_WILL_FINISH_TITLE1_TEXT);
    lv_obj_set_style_text_color(finish_info->title1, lv_color_hex(PUMP_WILL_FINISH_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(finish_info->title1, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(finish_info->title1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(finish_info->title1, LV_ALIGN_TOP_MID, 0, PUMP_WILL_FINISH_TITLE1_Y);

    if (finish_info->title2 == NULL || !lv_obj_is_valid(finish_info->title2)) {
        finish_info->title2 = lv_label_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->title2);
    }
    lv_label_set_text(finish_info->title2, PUMP_WILL_FINISH_TITLE2_TEXT);
    lv_obj_set_style_text_color(finish_info->title2, lv_color_hex(PUMP_WILL_FINISH_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(finish_info->title2, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(finish_info->title2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(finish_info->title2, finish_info->title1, LV_ALIGN_OUT_BOTTOM_MID,
        0, PUMP_WILL_FINISH_TITLE_GAP);
}

static uint8_t pump_will_finish_remain_sec_get(uint32_t elapsed_ms)
{
    uint8_t remain;

    if (elapsed_ms >= PUMP_WILL_FINISH_DURATION_MS) {
        return 1;
    }
    remain = (uint8_t)(PUMP_WILL_FINISH_COUNTDOWN_MAX -
        ((elapsed_ms * PUMP_WILL_FINISH_COUNTDOWN_MAX) / PUMP_WILL_FINISH_DURATION_MS));
    if (remain < 1) {
        remain = 1;
    }
    if (remain > PUMP_WILL_FINISH_COUNTDOWN_MAX) {
        remain = PUMP_WILL_FINISH_COUNTDOWN_MAX;
    }
    return remain;
}

static void pump_will_finish_countdown_update(void)
{
    char num_buf[8] = {0};
    uint32_t elapsed = get_timestamp_ms(finish_info->start_time);
    uint8_t remain = pump_will_finish_remain_sec_get(elapsed);

    if (finish_info->countdown == NULL || !lv_obj_is_valid(finish_info->countdown)) {
        finish_info->countdown = lv_label_create(finish_info->root);
        EXCEPTION_CHECK_VOID(finish_info->countdown);
        finish_info->shown_remain_sec = 0xFF;
    }
    if (finish_info->force_update == false && finish_info->shown_remain_sec == remain) {
        return;
    }
    snprintf(num_buf, sizeof(num_buf), "%u s", remain);
    lv_label_set_text(finish_info->countdown, num_buf);
    lv_obj_set_style_text_color(finish_info->countdown,
        lv_color_hex(PUMP_WILL_FINISH_COUNTDOWN_COLOR), 0);
    lv_obj_set_style_text_font(finish_info->countdown, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(finish_info->countdown, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(finish_info->countdown, LV_ALIGN_TOP_MID, 0, PUMP_WILL_FINISH_COUNTDOWN_Y);
    finish_info->shown_remain_sec = remain;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_will_finish_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (finish_info == NULL) {
        finish_info = (pump_will_finish_ui_info_t *)u_malloz(sizeof(pump_will_finish_ui_info_t));
        EXCEPTION_CHECK_VOID(finish_info);
    }
    finish_info->force_update = true;
    finish_info->completed = false;
    finish_info->shown_remain_sec = 0xFF;
    finish_info->start_time = get_timestamp();
    pump_will_finish_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_will_finish_remove(ui_manager_context_t *thiz)
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

int ui_display_pump_will_finish_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && finish_info && finish_info->root);
    pump_will_finish_arc_update();
    pump_will_finish_icon_update(thiz);
    pump_will_finish_title_update();
    pump_will_finish_countdown_update();
    finish_info->force_update = false;

    if (finish_info->completed == false &&
        get_timestamp_ms(finish_info->start_time) >= PUMP_WILL_FINISH_DURATION_MS) {
        finish_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump will finish fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
