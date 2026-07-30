/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "pump_break_tip_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_BREAK_TIP_ICON_Y                             (93)
#define PUMP_BREAK_TIP_TITLE_Y                            (202)
#define PUMP_BREAK_TIP_TITLE_COLOR                        (0xFFFFFF)
#define PUMP_BREAK_TIP_TITLE_LINE_SPACE                   (4)
#define PUMP_BREAK_TIP_TITLE_TEXT                         "Pumping over 30 min.\nTake a short break."

#define PUMP_BREAK_TIP_W                              (220)
#define PUMP_BREAK_TIP_H                              (63)
/** 底部提示图 Y：贴底留边（与 home 模式选择页一致） */
#define PUMP_BREAK_TIP_Y                              (PANLE_DISPLAY_H - PUMP_BREAK_TIP_H - 8)

#define PUMP_BREAK_TIP_BASE_OPA                       (LV_OPA_70)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t force_update;
    uint8_t rsv[7];
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
    ui_img_baj_theme_set_src(thiz, break_tip_info->icon, BASE_RES_ID_TIMEOUT_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
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

static lv_draw_buf_t *pump_break_tip_predecode(void)
{
    const ui_img_data_t *tip;

    if (break_tip_info->tip_buf != NULL) {
        return break_tip_info->tip_buf;
    }
    tip = ui_font_img_pump_break_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    break_tip_info->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return break_tip_info->tip_buf;
}

static void pump_break_tip_rebuild(void)
{
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    if (break_tip_info->tip_img != NULL && lv_obj_is_valid(break_tip_info->tip_img) &&
        break_tip_info->tip_shine.mask != NULL && lv_obj_is_valid(break_tip_info->tip_shine.mask) &&
        break_tip_info->tip_shine.shine != NULL && lv_obj_is_valid(break_tip_info->tip_shine.shine)) {
        return; /* anim handles sync */
    }

    predecoded = pump_break_tip_predecode();

    if (break_tip_info->tip_img == NULL || !lv_obj_is_valid(break_tip_info->tip_img)) {
        break_tip_info->tip_img = lv_image_create(break_tip_info->root);
        EXCEPTION_CHECK_VOID(break_tip_info->tip_img);
        lv_obj_clear_flag(break_tip_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(break_tip_info->tip_img, predecoded);
        }
        lv_obj_set_style_image_opa(break_tip_info->tip_img, PUMP_BREAK_TIP_BASE_OPA, 0);
        lv_obj_align(break_tip_info->tip_img, LV_ALIGN_TOP_MID, 0, PUMP_BREAK_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = PUMP_BREAK_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;

    ui_shine_prepare_from_tip(&break_tip_info->tip_shine, &style, predecoded);
    ui_shine_update(&break_tip_info->tip_shine, break_tip_info->root, &style, break_tip_info->tip_img, 1);
    lv_obj_move_foreground(break_tip_info->tip_img);
    ui_shine_show(&break_tip_info->tip_shine);
}

static void pump_break_tip_update(void)
{
    ui_shine_style_t style;

    if (break_tip_info->force_update) {
        pump_break_tip_rebuild();
    }
    ui_shine_style_init(&style);
    style.mask_y = PUMP_BREAK_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&break_tip_info->tip_shine, &style, break_tip_info->tip_img, 0);
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
    pump_break_tip_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_break_tip_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && break_tip_info && break_tip_info->root);
    ui_img_baj_release_decoded_buf(break_tip_info->icon);
    ui_shine_remove(&break_tip_info->tip_shine);
    if (break_tip_info->tip_img != NULL && lv_obj_is_valid(break_tip_info->tip_img)) {
        lv_image_set_src(break_tip_info->tip_img, NULL);
    }
    if (break_tip_info->tip_buf != NULL) {
        lv_draw_buf_destroy(break_tip_info->tip_buf);
        break_tip_info->tip_buf = NULL;
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
    pump_break_tip_update();
    break_tip_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump break tip fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
