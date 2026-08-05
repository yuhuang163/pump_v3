/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define AIRPLANE_MODE_ARC_DONE_COLOR                      (0xF3F4FF)

#define AIRPLANE_MODE_LOCK_ICON_Y                         (33)
#define AIRPLANE_MODE_TITLE_Y                             (139)
#define AIRPLANE_MODE_HINT_Y                              (181)
#define AIRPLANE_MODE_TITLE_COLOR                         (0xFFFFFF)
#define AIRPLANE_MODE_HINT_COLOR                          (0x999999)
#define AIRPLANE_MODE_HINT_LINE_SPACE                     (8)
/** 进度显示时，文案与其它 ICON 上移偏移 */
#define AIRPLANE_MODE_HOLDING_CONTENT_SHIFT_Y             (52)

#define AIRPLANE_MODE_TITLE_TEXT                          "Device locked"
#define AIRPLANE_MODE_HINT_TEXT                           "Press and hold for 5\nseconds to unlock"

#define AIRPLANE_MODE_STIM_KEY_ICON_X                     (99)
#define AIRPLANE_MODE_STIM_KEY_ICON_Y                     (267)
#define AIRPLANE_MODE_ADD_ICON_X                          (171)
#define AIRPLANE_MODE_ADD_ICON_Y                          (288)
#define AIRPLANE_MODE_EXPR_KEY_ICON_X                     (201)
#define AIRPLANE_MODE_EXPR_KEY_ICON_Y                     (267)

/** 长按解锁倒计时（文案为 5s/4s/3s/2s/1s） */
#define AIRPLANE_MODE_UNLOCK_DURATION_MS                  (5000)
#define AIRPLANE_MODE_UNLOCK_COUNTDOWN_MAX                (5)

typedef struct {
    lv_obj_t *root;
    ui_progress_arc_info_t arc;
    lv_obj_t *lock_icon;
    lv_obj_t *stim_key_icon;
    lv_obj_t *add_icon;
    lv_obj_t *expr_key_icon;
    lv_obj_t *title;
    lv_obj_t *hint;
    lv_draw_buf_t *lock_icon_buf;
    lv_draw_buf_t *stim_key_icon_buf;
    lv_draw_buf_t *add_icon_buf;
    lv_draw_buf_t *expr_key_icon_buf;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t holding;
    uint8_t completed;
    uint8_t rsv[9];
} airplane_mode_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static airplane_mode_display_info_t *airplane_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void airplane_mode_release_icon_buf(lv_obj_t *icon, lv_draw_buf_t **buf)
{
    if (buf != NULL && *buf != NULL) {
        if (icon != NULL && lv_obj_is_valid(icon)) {
            lv_image_set_src(icon, NULL);
        }
        lv_draw_buf_destroy(*buf);
        *buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(icon);
    }
}

static void airplane_mode_set_predecoded_icon(ui_manager_context_t *thiz, lv_obj_t **icon,
    lv_draw_buf_t **buf, uint16_t res_id, lv_align_t align, int16_t x, int16_t y)
{
    if (*icon == NULL || !lv_obj_is_valid(*icon)) {
        *icon = lv_image_create(airplane_info->root);
        EXCEPTION_CHECK_VOID(*icon);
    }
    if (*buf == NULL) {
        *buf = ui_img_baj_theme_decode_draw_buf(thiz, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (*buf != NULL) {
        lv_image_set_src(*icon, *buf);
    } else {
        ui_img_baj_theme_set_src(thiz, *icon, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(*icon, align, x, y);
}

static void airplane_mode_root_init(void)
{
    if (airplane_info->root == NULL || !lv_obj_is_valid(airplane_info->root)) {
        airplane_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(airplane_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(airplane_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(airplane_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(airplane_info->root, 0, 0);
    lv_obj_set_style_pad_all(airplane_info->root, 0, 0);
    lv_obj_align(airplane_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(airplane_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(airplane_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void airplane_mode_holding_sync(ui_proc_airplane_mode_t *proc)
{
    uint8_t holding = (proc->popup_ui_type == ID_POPUP_UI_AIRPLANE_MODE_PROMPT) ? 1 : 0;

    if (holding && airplane_info->holding == false) {
        airplane_info->start_time = get_timestamp();
        airplane_info->completed = false;
        airplane_info->force_update = true;
        ui_progress_arc_reset(&airplane_info->arc);
    } else if (holding == false && airplane_info->holding) {
        airplane_info->completed = false;
        airplane_info->force_update = true;
        ui_progress_arc_reset(&airplane_info->arc);
    }
    airplane_info->holding = holding;
}

static void airplane_mode_arc_update(void)
{
    ui_progress_arc_style_t style;

    if (airplane_info->holding == false) {
        ui_progress_arc_hide(&airplane_info->arc);
        ui_progress_arc_countdown_hide(&airplane_info->arc);
        return;
    }

    ui_progress_arc_style_init(&style, AIRPLANE_MODE_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&airplane_info->arc, airplane_info->root, &style,
        airplane_info->start_time, AIRPLANE_MODE_UNLOCK_DURATION_MS,
        AIRPLANE_MODE_UNLOCK_COUNTDOWN_MAX, airplane_info->force_update);
}

static void airplane_mode_lock_icon_update(ui_manager_context_t *thiz)
{
    if (airplane_info->holding) {
        if (airplane_info->lock_icon != NULL && lv_obj_is_valid(airplane_info->lock_icon)) {
            lv_obj_add_flag(airplane_info->lock_icon, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    if (airplane_info->force_update == false &&
        airplane_info->lock_icon != NULL && lv_obj_is_valid(airplane_info->lock_icon)) {
        lv_obj_clear_flag(airplane_info->lock_icon, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    airplane_mode_set_predecoded_icon(thiz, &airplane_info->lock_icon,
        &airplane_info->lock_icon_buf, BASE_RES_ID_TRAVEL_LOCK_ICON,
        LV_ALIGN_TOP_MID, 0, AIRPLANE_MODE_LOCK_ICON_Y);
    if (airplane_info->lock_icon != NULL && lv_obj_is_valid(airplane_info->lock_icon)) {
        lv_obj_clear_flag(airplane_info->lock_icon, LV_OBJ_FLAG_HIDDEN);
    }
}

static int16_t airplane_mode_content_y(int16_t base_y)
{
    if (airplane_info->holding) {
        return (int16_t)(base_y - AIRPLANE_MODE_HOLDING_CONTENT_SHIFT_Y);
    }
    return base_y;
}

static void airplane_mode_key_icons_update(ui_manager_context_t *thiz)
{
    if (airplane_info->force_update == false) {
        return;
    }

    airplane_mode_set_predecoded_icon(thiz, &airplane_info->stim_key_icon,
        &airplane_info->stim_key_icon_buf, BASE_RES_ID_STIMULATION_KEY_ICON,
        LV_ALIGN_TOP_LEFT, AIRPLANE_MODE_STIM_KEY_ICON_X,
        airplane_mode_content_y(AIRPLANE_MODE_STIM_KEY_ICON_Y));
    airplane_mode_set_predecoded_icon(thiz, &airplane_info->add_icon,
        &airplane_info->add_icon_buf, BASE_RES_ID_ADD_ICON,
        LV_ALIGN_TOP_LEFT, AIRPLANE_MODE_ADD_ICON_X,
        airplane_mode_content_y(AIRPLANE_MODE_ADD_ICON_Y));
    airplane_mode_set_predecoded_icon(thiz, &airplane_info->expr_key_icon,
        &airplane_info->expr_key_icon_buf, BASE_RES_ID_GUIDE_RIGHT_KEY_ICON,
        LV_ALIGN_TOP_LEFT, AIRPLANE_MODE_EXPR_KEY_ICON_X,
        airplane_mode_content_y(AIRPLANE_MODE_EXPR_KEY_ICON_Y));
}

static void airplane_mode_texts_update(void)
{
    if (airplane_info->force_update == false) {
        return;
    }

    if (airplane_info->title == NULL || !lv_obj_is_valid(airplane_info->title)) {
        airplane_info->title = lv_label_create(airplane_info->root);
        EXCEPTION_CHECK_VOID(airplane_info->title);
    }
    lv_label_set_text(airplane_info->title, AIRPLANE_MODE_TITLE_TEXT);
    lv_obj_set_style_text_color(airplane_info->title,
        lv_color_hex(AIRPLANE_MODE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(airplane_info->title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(airplane_info->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(airplane_info->title, LV_ALIGN_TOP_MID, 0,
        airplane_mode_content_y(AIRPLANE_MODE_TITLE_Y));

    if (airplane_info->hint == NULL || !lv_obj_is_valid(airplane_info->hint)) {
        airplane_info->hint = lv_label_create(airplane_info->root);
        EXCEPTION_CHECK_VOID(airplane_info->hint);
    }
    lv_label_set_text(airplane_info->hint, AIRPLANE_MODE_HINT_TEXT);
    lv_obj_set_style_text_color(airplane_info->hint,
        lv_color_hex(AIRPLANE_MODE_HINT_COLOR), 0);
    lv_obj_set_style_text_font(airplane_info->hint, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(airplane_info->hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(airplane_info->hint, AIRPLANE_MODE_HINT_LINE_SPACE, 0);
    lv_obj_align(airplane_info->hint, LV_ALIGN_TOP_MID, 0,
        airplane_mode_content_y(AIRPLANE_MODE_HINT_Y));
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_airplane_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (airplane_info == NULL) {
        airplane_info = (airplane_mode_display_info_t *)u_malloz(sizeof(airplane_mode_display_info_t));
        EXCEPTION_CHECK_VOID(airplane_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    airplane_mode_root_init();
    airplane_info->force_update = true;
    airplane_info->holding = false;
    airplane_info->completed = false;
    ui_progress_arc_reset(&airplane_info->arc);
}

void ui_display_airplane_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && airplane_info);
    airplane_mode_release_icon_buf(airplane_info->lock_icon, &airplane_info->lock_icon_buf);
    airplane_mode_release_icon_buf(airplane_info->stim_key_icon, &airplane_info->stim_key_icon_buf);
    airplane_mode_release_icon_buf(airplane_info->add_icon, &airplane_info->add_icon_buf);
    airplane_mode_release_icon_buf(airplane_info->expr_key_icon, &airplane_info->expr_key_icon_buf);
    LVGL_OBJ_DELETE(airplane_info->root);
    u_free(airplane_info);
    airplane_info = NULL;
}

int ui_display_airplane_mode_process(ui_manager_context_t *thiz)
{
    ui_proc_airplane_mode_t *proc;

    EXCEPTION_CHECK_INT(thiz && airplane_info && airplane_info->root);
    proc = (ui_proc_airplane_mode_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);

    airplane_mode_holding_sync(proc);
    airplane_mode_arc_update();
    airplane_mode_lock_icon_update(thiz);
    airplane_mode_key_icons_update(thiz);
    airplane_mode_texts_update();
    airplane_info->force_update = false;

    if (airplane_info->holding && airplane_info->completed == false &&
        get_timestamp_ms(airplane_info->start_time) >= AIRPLANE_MODE_UNLOCK_DURATION_MS) {
        airplane_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> airplane mode fps=%d, holding=%d",
            thiz->priv.fps_count, airplane_info->holding);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    return thiz->priv.fps_count;
}
