/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_base.h" // IWYU pragma: keep

#if IS_SUPPORT_POWER_ON_GUIDE_UI

#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_power_on_guide.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep
#include "power_on_hold_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_ON_HOLD_ARC_DONE_COLOR              (0xF3F4FF)

#define POWER_ON_HOLD_TEXT_COLOR                  (0xFFFFFF)

/** 长按倒计时（文案为 2s/1s） */
#define POWER_ON_HOLD_DURATION_MS                 (2000)
#define POWER_ON_HOLD_COUNTDOWN_MAX               (2)

typedef struct {
    lv_obj_t *root;
    ui_progress_arc_info_t arc;
    lv_obj_t *icon;
    lv_obj_t *line1;
    lv_obj_t *line2;
    lv_draw_buf_t *icon_buf;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t holding;
    uint8_t completed;
    uint8_t rsv[9];
} ui_power_on_hold_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_power_on_hold_info_t *ui_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_power_on_hold_root_init(void)
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

static void ui_power_on_hold_arc_update(void)
{
    ui_progress_arc_style_t style;

    if (ui_info->holding == false) {
        ui_progress_arc_hide(&ui_info->arc);
        ui_progress_arc_countdown_hide(&ui_info->arc);
        return;
    }

    ui_progress_arc_style_init(&style, POWER_ON_HOLD_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&ui_info->arc, ui_info->root, &style,
        ui_info->start_time, POWER_ON_HOLD_DURATION_MS,
        POWER_ON_HOLD_COUNTDOWN_MAX, ui_info->force_update);
}

static void ui_power_on_hold_icon_update(ui_manager_context_t *thiz)
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
            BASE_RES_ID_GUIDE_POWER_KEY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (ui_info->icon_buf != NULL) {
        lv_image_set_src(ui_info->icon, ui_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, ui_info->icon, BASE_RES_ID_GUIDE_POWER_KEY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(ui_info->icon, LV_ALIGN_TOP_MID, 0, POWER_ON_HOLD_ICON_Y);
}

static void ui_power_on_hold_text_update(void)
{
    if (ui_info->force_update == false) {
        return;
    }

    if (ui_info->line1 == NULL || !lv_obj_is_valid(ui_info->line1)) {
        ui_info->line1 = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->line1);
    }
    lv_label_set_text(ui_info->line1, POWER_ON_HOLD_LINE1_TEXT);
    lv_obj_set_style_text_color(ui_info->line1, lv_color_hex(POWER_ON_HOLD_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(ui_info->line1, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(ui_info->line1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(ui_info->line1, LV_ALIGN_TOP_MID, 0, POWER_ON_HOLD_LINE1_Y);

    if (ui_info->line2 == NULL || !lv_obj_is_valid(ui_info->line2)) {
        ui_info->line2 = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->line2);
    }
    lv_label_set_recolor(ui_info->line2, true);
    lv_label_set_text(ui_info->line2, POWER_ON_HOLD_LINE2_TEXT);
    lv_obj_set_style_text_color(ui_info->line2, lv_color_hex(POWER_ON_HOLD_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(ui_info->line2, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(ui_info->line2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(ui_info->line2, ui_info->line1, LV_ALIGN_OUT_BOTTOM_MID, 0,
        POWER_ON_HOLD_LINE_GAP);
}

static void ui_power_on_hold_holding_sync(ui_proc_power_on_guide_t *proc)
{
    uint8_t holding = proc->power_on_hold_holding ? 1 : 0;

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
void ui_display_power_on_hold_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ui_info == NULL) {
        ui_info = (ui_power_on_hold_info_t *)u_malloz(sizeof(ui_power_on_hold_info_t));
        EXCEPTION_CHECK_VOID(ui_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_power_on_hold_root_init();
    ui_info->force_update = true;
    ui_info->holding = false;
    ui_info->completed = false;
    ui_progress_arc_reset(&ui_info->arc);
}

void ui_display_power_on_hold_remove(ui_manager_context_t *thiz)
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

int ui_display_power_on_hold_process(ui_manager_context_t *thiz)
{
    ui_proc_power_on_guide_t *proc;

    EXCEPTION_CHECK_INT(thiz && ui_info && ui_info->root);
    proc = (ui_proc_power_on_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);

    ui_power_on_hold_holding_sync(proc);
    ui_power_on_hold_arc_update();
    ui_power_on_hold_icon_update(thiz);
    ui_power_on_hold_text_update();
    ui_info->force_update = false;

    if (ui_info->holding && ui_info->completed == false &&
        get_timestamp_ms(ui_info->start_time) >= POWER_ON_HOLD_DURATION_MS) {
        ui_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> power on hold fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}

#endif /* IS_SUPPORT_POWER_ON_GUIDE_UI */
