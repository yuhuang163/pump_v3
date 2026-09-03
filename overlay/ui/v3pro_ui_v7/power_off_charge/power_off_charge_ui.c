/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_power_off.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_OFF_CHARGE_ARC_DONE_COLOR           (0x29B572)
#define POWER_OFF_CHARGE_ICON_Y                   (111)
#define POWER_OFF_CHARGE_TEXT_COLOR               (0xFFFFFF)
/** 进入页时电量弧从 0 扫到目标电量的时长 */
#define POWER_OFF_CHARGE_ARC_ENTER_ANIM_MS        (700)

typedef struct {
    lv_obj_t *root;
    ui_progress_arc_info_t arc;
    lv_obj_t *charge_icon;
    lv_obj_t *text;
    lv_draw_buf_t *icon_buf;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint32_t anim_start_time;
    uint8_t force_update;
    uint8_t percent;
    uint8_t enter_anim_pending;
    uint8_t rsv[13];
} power_off_charge_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static power_off_charge_display_info_t *charge_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void power_off_charge_ui_root_init(void)
{
    if (charge_info->root == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(charge_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(charge_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(charge_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(charge_info->root, 0, 0);
    lv_obj_set_style_pad_all(charge_info->root, 0, 0);
    lv_obj_align(charge_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static uint16_t power_off_charge_arc_visual_sweep_get(void)
{
    uint32_t elapsed;
    uint16_t from = charge_info->anim_from_sweep;
    uint16_t to = charge_info->anim_to_sweep;

    if (from == to) {
        return to;
    }

    elapsed = get_timestamp_ms(charge_info->anim_start_time);
    if (elapsed >= POWER_OFF_CHARGE_ARC_ENTER_ANIM_MS) {
        charge_info->anim_from_sweep = to;
        return to;
    }

    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) /
            POWER_OFF_CHARGE_ARC_ENTER_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) /
        POWER_OFF_CHARGE_ARC_ENTER_ANIM_MS);
}

static void power_off_charge_arc_anim_sync(uint8_t percent)
{
    uint16_t target_sweep = ui_progress_arc_sweep_from_progress(percent, 100);

    if (charge_info->enter_anim_pending) {
        charge_info->anim_from_sweep = 0;
        charge_info->anim_to_sweep = target_sweep;
        charge_info->anim_start_time = get_timestamp();
        charge_info->percent = percent;
        charge_info->enter_anim_pending = 0;
        return;
    }

    if (charge_info->percent == percent && charge_info->anim_to_sweep == target_sweep) {
        return;
    }

    if (charge_info->anim_from_sweep != charge_info->anim_to_sweep) {
        /* 入场动画未结束时只改目标，避免从头重扫 */
        charge_info->anim_to_sweep = target_sweep;
        charge_info->percent = percent;
        return;
    }

    charge_info->anim_from_sweep = target_sweep;
    charge_info->anim_to_sweep = target_sweep;
    charge_info->percent = percent;
}

static void power_off_charge_arc_update(uint8_t percent)
{
    ui_progress_arc_style_t style;
    uint16_t done_sweep;

    if (percent > 100) {
        percent = 100;
    }

    power_off_charge_arc_anim_sync(percent);
    done_sweep = power_off_charge_arc_visual_sweep_get();

    ui_progress_arc_style_init(&style, POWER_OFF_CHARGE_ARC_DONE_COLOR);
    style.show_countdown = false;
    ui_progress_arc_update(&charge_info->arc, charge_info->root, &style, done_sweep,
        charge_info->force_update);
}

static void power_off_charge_icon_update(ui_manager_context_t *thiz)
{
    if (charge_info->force_update == false) {
        return;
    }

    if (charge_info->charge_icon == NULL || !lv_obj_is_valid(charge_info->charge_icon)) {
        charge_info->charge_icon = lv_image_create(charge_info->root);
        EXCEPTION_CHECK_VOID(charge_info->charge_icon);
    }
    if (charge_info->icon_buf == NULL) {
        charge_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_CHARGE_PAGE_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (charge_info->icon_buf != NULL) {
        lv_image_set_src(charge_info->charge_icon, charge_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, charge_info->charge_icon, BASE_RES_ID_CHARGE_PAGE_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(charge_info->charge_icon, LV_ALIGN_TOP_MID, 0, POWER_OFF_CHARGE_ICON_Y);
}

static void power_off_charge_text_update(void)
{
    if (charge_info->force_update == false) {
        return;
    }

    if (charge_info->text == NULL || !lv_obj_is_valid(charge_info->text)) {
        charge_info->text = lv_label_create(charge_info->root);
        EXCEPTION_CHECK_VOID(charge_info->text);
    }
    lv_label_set_text(charge_info->text, POWER_OFF_CHARGE_TEXT);
    lv_obj_set_style_text_color(charge_info->text, lv_color_hex(POWER_OFF_CHARGE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(charge_info->text, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(charge_info->text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(charge_info->text, LV_ALIGN_TOP_MID, 0, POWER_OFF_CHARGE_TEXT_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_power_off_charge_screen_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (charge_info == NULL) {
        charge_info = (power_off_charge_display_info_t *)u_malloz(sizeof(power_off_charge_display_info_t));
        EXCEPTION_CHECK_VOID(charge_info);
    }
    power_off_charge_ui_root_init();
    ui_progress_arc_reset(&charge_info->arc);
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    charge_info->force_update = true;
    charge_info->percent = 0xFF;
    charge_info->anim_from_sweep = 0;
    charge_info->anim_to_sweep = 0;
    charge_info->anim_start_time = 0;
    charge_info->enter_anim_pending = 1;
}

void ui_display_power_off_charge_screen_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && charge_info);
    if (charge_info->icon_buf != NULL) {
        if (charge_info->charge_icon != NULL && lv_obj_is_valid(charge_info->charge_icon)) {
            lv_image_set_src(charge_info->charge_icon, NULL);
        }
        lv_draw_buf_destroy(charge_info->icon_buf);
        charge_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(charge_info->charge_icon);
    }
    LVGL_OBJ_DELETE(charge_info->root);
    u_free(charge_info);
    charge_info = NULL;
}

int ui_display_power_off_charge_screen_process(ui_manager_context_t *thiz)
{
    ui_proc_power_off_t *proc;
    uint8_t percent;

    EXCEPTION_CHECK_INT(thiz && charge_info && charge_info->root);
    proc = (ui_proc_power_off_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->battery);
    percent = proc->battery->get_battery_percent();

    power_off_charge_arc_update(percent);
    power_off_charge_icon_update(thiz);
    power_off_charge_text_update();
    charge_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> power off charge fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
