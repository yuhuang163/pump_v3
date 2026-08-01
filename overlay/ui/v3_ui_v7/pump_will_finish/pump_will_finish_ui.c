/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "pump_will_finish_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_WILL_FINISH_ICON_Y                       (94)
#define PUMP_WILL_FINISH_TITLE1_Y                     (198)
#define PUMP_WILL_FINISH_TITLE_GAP                    (10)
#define PUMP_WILL_FINISH_TITLE_COLOR                  (0xFFFFFF)
#define PUMP_WILL_FINISH_TITLE2_COLOR                 (0x999999)
#define PUMP_WILL_FINISH_TITLE1_TEXT                  "Program ending soon"
#define PUMP_WILL_FINISH_TITLE2_TEXT                  "Continue pumping?"

/** 底部提示图：与 home_speed_adjust 一致 */
#define PUMP_WILL_FINISH_TIP_H                        (49)
#define PUMP_WILL_FINISH_TIP_Y                        (PANLE_DISPLAY_H - PUMP_WILL_FINISH_TIP_H - 8)

#define PUMP_WILL_FINISH_TIP_BASE_OPA                 (LV_OPA_70)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *icon;
    lv_obj_t *title1;
    lv_obj_t *title2;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *icon_buf;
    lv_draw_buf_t *tip_buf;
    uint8_t force_update;
    uint8_t rsv[7];
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
            BASE_RES_ID_QUANDARY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
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
    lv_obj_set_style_text_font(will_finish_info->title1, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(will_finish_info->title1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(will_finish_info->title1, LV_ALIGN_TOP_MID, 0, PUMP_WILL_FINISH_TITLE1_Y);

    if (will_finish_info->title2 == NULL || !lv_obj_is_valid(will_finish_info->title2)) {
        will_finish_info->title2 = lv_label_create(will_finish_info->root);
        EXCEPTION_CHECK_VOID(will_finish_info->title2);
    }
    lv_label_set_text(will_finish_info->title2, PUMP_WILL_FINISH_TITLE2_TEXT);
    lv_obj_set_style_text_color(will_finish_info->title2,
        lv_color_hex(PUMP_WILL_FINISH_TITLE2_COLOR), 0);
    lv_obj_set_style_text_font(will_finish_info->title2, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(will_finish_info->title2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(will_finish_info->title2, will_finish_info->title1,
        LV_ALIGN_OUT_BOTTOM_MID, 0, PUMP_WILL_FINISH_TITLE_GAP);
}

static lv_draw_buf_t *pump_will_finish_tip_predecode(void)
{
    const ui_img_data_t *tip;

    if (will_finish_info->tip_buf != NULL) {
        return will_finish_info->tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    will_finish_info->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return will_finish_info->tip_buf;
}

static void pump_will_finish_tip_rebuild(void)
{
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    if (will_finish_info->tip_img != NULL && lv_obj_is_valid(will_finish_info->tip_img) &&
        will_finish_info->tip_shine.mask != NULL && lv_obj_is_valid(will_finish_info->tip_shine.mask) &&
        will_finish_info->tip_shine.shine != NULL && lv_obj_is_valid(will_finish_info->tip_shine.shine)) {
        return; /* anim handles sync */
    }

    predecoded = pump_will_finish_tip_predecode();

    if (will_finish_info->tip_img == NULL || !lv_obj_is_valid(will_finish_info->tip_img)) {
        will_finish_info->tip_img = lv_image_create(will_finish_info->root);
        EXCEPTION_CHECK_VOID(will_finish_info->tip_img);
        lv_obj_clear_flag(will_finish_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(will_finish_info->tip_img, predecoded);
        }
        lv_obj_set_style_image_opa(will_finish_info->tip_img, PUMP_WILL_FINISH_TIP_BASE_OPA, 0);
        lv_obj_align(will_finish_info->tip_img, LV_ALIGN_TOP_MID, 0, PUMP_WILL_FINISH_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = PUMP_WILL_FINISH_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;

    ui_shine_prepare_from_tip(&will_finish_info->tip_shine, &style, predecoded);
    ui_shine_update(&will_finish_info->tip_shine, will_finish_info->root, &style,
        will_finish_info->tip_img, 1);
    lv_obj_move_foreground(will_finish_info->tip_img);
    ui_shine_show(&will_finish_info->tip_shine);
}

static void pump_will_finish_tip_update(void)
{
    ui_shine_style_t style;

    if (will_finish_info->force_update) {
        pump_will_finish_tip_rebuild();
    }
    ui_shine_style_init(&style);
    style.mask_y = PUMP_WILL_FINISH_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&will_finish_info->tip_shine, &style, will_finish_info->tip_img, 0);
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
    ui_shine_remove(&will_finish_info->tip_shine);
    if (will_finish_info->tip_img != NULL && lv_obj_is_valid(will_finish_info->tip_img)) {
        lv_image_set_src(will_finish_info->tip_img, NULL);
    }
    if (will_finish_info->tip_buf != NULL) {
        lv_draw_buf_destroy(will_finish_info->tip_buf);
        will_finish_info->tip_buf = NULL;
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
    pump_will_finish_tip_update();
    will_finish_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump will finish fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
