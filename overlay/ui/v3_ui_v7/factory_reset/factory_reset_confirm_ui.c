/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_factory_reset.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define FACTORY_RESET_CONFIRM_ARC_DONE_COLOR          (0xF3F4FF)

#define FACTORY_RESET_CONFIRM_ICON_Y                  (73)
#define FACTORY_RESET_CONFIRM_TITLE_Y                 (179)
#define FACTORY_RESET_CONFIRM_HINT_Y                  (221)
#define FACTORY_RESET_CONFIRM_TITLE_COLOR             (0xFFFFFF)
#define FACTORY_RESET_CONFIRM_HINT_COLOR              (0x999999)
#define FACTORY_RESET_CONFIRM_TITLE_TEXT              "Reset settings"
#define FACTORY_RESET_CONFIRM_HINT_TEXT               "Press and hold knob\nfor 3s,"

/** 长按倒计时（文案为 3s/2s/1s） */
#define FACTORY_RESET_CONFIRM_DURATION_MS             (2000)
#define FACTORY_RESET_CONFIRM_COUNTDOWN_MAX           (3)

typedef struct {
    lv_obj_t *root;
    ui_progress_arc_info_t arc;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *hint;
    lv_draw_buf_t *icon_buf;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t holding;
    uint8_t completed;
    uint8_t rsv[9];
} ui_factory_reset_confirm_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_factory_reset_confirm_info_t *ui_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_factory_reset_confirm_root_init(void)
{
    if (ui_info->root == NULL || !lv_obj_is_valid(ui_info->root)) {
        ui_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(ui_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(ui_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(ui_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ui_info->root, 0, 0);
    lv_obj_set_style_pad_all(ui_info->root, 0, 0);
    lv_obj_align(ui_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void ui_factory_reset_confirm_arc_update(void)
{
    ui_progress_arc_style_t style;

    if (ui_info->holding == false) {
        ui_progress_arc_hide(&ui_info->arc);
        ui_progress_arc_countdown_hide(&ui_info->arc);
        return;
    }

    ui_progress_arc_style_init(&style, FACTORY_RESET_CONFIRM_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&ui_info->arc, ui_info->root, &style,
        ui_info->start_time, FACTORY_RESET_CONFIRM_DURATION_MS,
        FACTORY_RESET_CONFIRM_COUNTDOWN_MAX, ui_info->force_update);
}

static void ui_factory_reset_confirm_icon_update(ui_manager_context_t *thiz)
{
    if (ui_info->force_update == false) {
        return;
    }

    if (ui_info->icon == NULL || !lv_obj_is_valid(ui_info->icon)) {
        ui_info->icon = lv_image_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->icon);
    }
    if (ui_info->icon_buf == NULL) {
        ui_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_ROTARY_KEY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (ui_info->icon_buf != NULL) {
        lv_image_set_src(ui_info->icon, ui_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, ui_info->icon, BASE_RES_ID_ROTARY_KEY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(ui_info->icon, LV_ALIGN_TOP_MID, 0, FACTORY_RESET_CONFIRM_ICON_Y);
}

static void ui_factory_reset_confirm_text_update(void)
{
    if (ui_info->force_update == false) {
        return;
    }

    if (ui_info->title == NULL || !lv_obj_is_valid(ui_info->title)) {
        ui_info->title = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->title);
    }
    lv_label_set_text(ui_info->title, FACTORY_RESET_CONFIRM_TITLE_TEXT);
    lv_obj_set_style_text_color(ui_info->title,
        lv_color_hex(FACTORY_RESET_CONFIRM_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(ui_info->title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(ui_info->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(ui_info->title, LV_ALIGN_TOP_MID, 0, FACTORY_RESET_CONFIRM_TITLE_Y);

    if (ui_info->hint == NULL || !lv_obj_is_valid(ui_info->hint)) {
        ui_info->hint = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->hint);
    }
    lv_label_set_text(ui_info->hint, FACTORY_RESET_CONFIRM_HINT_TEXT);
    lv_obj_set_style_text_color(ui_info->hint,
        lv_color_hex(FACTORY_RESET_CONFIRM_HINT_COLOR), 0);
    lv_obj_set_style_text_font(ui_info->hint, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(ui_info->hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(ui_info->hint, 4, 0);
    lv_obj_align(ui_info->hint, LV_ALIGN_TOP_MID, 0, FACTORY_RESET_CONFIRM_HINT_Y);
}

static void ui_factory_reset_confirm_holding_sync(ui_proc_factory_reset_t *proc)
{
    uint8_t holding = (proc->popup_ui_type == ID_POPUP_UI_FACTORY_RESET_CONFIRM) ? 1 : 0;

    if (holding && ui_info->holding == false) {
        ui_info->start_time = get_timestamp();
        ui_info->completed = false;
        ui_info->force_update = true;
        ui_progress_arc_reset(&ui_info->arc);
    } else if (holding == false && ui_info->holding) {
        ui_info->completed = false;
        ui_progress_arc_reset(&ui_info->arc);
    }
    ui_info->holding = holding;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_factory_reset_confirm_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ui_info == NULL) {
        ui_info = (ui_factory_reset_confirm_info_t *)u_malloz(sizeof(ui_factory_reset_confirm_info_t));
        EXCEPTION_CHECK_VOID(ui_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_factory_reset_confirm_root_init();
    ui_info->force_update = true;
    ui_info->holding = false;
    ui_info->completed = false;
    ui_progress_arc_reset(&ui_info->arc);
}

void ui_display_factory_reset_confirm_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && ui_info);
    if (ui_info->icon_buf != NULL) {
        if (ui_info->icon != NULL && lv_obj_is_valid(ui_info->icon)) {
            lv_image_set_src(ui_info->icon, NULL);
        }
        lv_draw_buf_destroy(ui_info->icon_buf);
        ui_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(ui_info->icon);
    }
    LVGL_OBJ_DELETE(ui_info->root);
    u_free(ui_info);
    ui_info = NULL;
}

int ui_display_factory_reset_confirm_process(ui_manager_context_t *thiz, bool blink_visible)
{
    ui_proc_factory_reset_t *proc;

    (void)blink_visible;
    EXCEPTION_CHECK_INT(thiz && ui_info && ui_info->root);
    proc = (ui_proc_factory_reset_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);

    ui_factory_reset_confirm_holding_sync(proc);
    ui_factory_reset_confirm_arc_update();
    ui_factory_reset_confirm_icon_update(thiz);
    ui_factory_reset_confirm_text_update();
    ui_info->force_update = false;

    if (ui_info->holding && ui_info->completed == false &&
        get_timestamp_ms(ui_info->start_time) >= FACTORY_RESET_CONFIRM_DURATION_MS) {
        ui_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> factory reset confirm fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
