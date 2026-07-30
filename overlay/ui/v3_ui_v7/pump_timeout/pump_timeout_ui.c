/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "pump_timeout_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_TIMEOUT_ICON_Y                             (93)
#define PUMP_TIMEOUT_TITLE_Y                            (202)
#define PUMP_TIMEOUT_TITLE_COLOR                        (0xFFFFFF)
#define PUMP_TIMEOUT_TITLE_LINE_SPACE                   (4)
#define PUMP_TIMEOUT_TITLE_TEXT                         "Pumping over 30 min.\nTake a short break."

#define PUMP_TIMEOUT_TIP_W                              (193)
#define PUMP_TIMEOUT_TIP_H                              (42)
/** 底部提示图 Y：贴底留边（与 home 模式选择页一致） */
#define PUMP_TIMEOUT_TIP_Y                              (PANLE_DISPLAY_H - PUMP_TIMEOUT_TIP_H - 8)

/** 提示扫光方式（三选一，与 home 模式页取值一致） */
#define PUMP_TIMEOUT_TIP_SHINE_HARD                     (0)
#define PUMP_TIMEOUT_TIP_SHINE_SOFT                     (1)
#define PUMP_TIMEOUT_TIP_SHINE_OVERLAY                  (2)
#ifndef PUMP_TIMEOUT_TIP_SHINE_MODE
#define PUMP_TIMEOUT_TIP_SHINE_MODE                     PUMP_TIMEOUT_TIP_SHINE_OVERLAY
#endif
#define PUMP_TIMEOUT_TIP_SHINE_IS_HARD \
    (PUMP_TIMEOUT_TIP_SHINE_MODE == PUMP_TIMEOUT_TIP_SHINE_HARD)
#define PUMP_TIMEOUT_TIP_SHINE_IS_SOFT \
    (PUMP_TIMEOUT_TIP_SHINE_MODE == PUMP_TIMEOUT_TIP_SHINE_SOFT)
#define PUMP_TIMEOUT_TIP_SHINE_IS_OVERLAY \
    (PUMP_TIMEOUT_TIP_SHINE_MODE == PUMP_TIMEOUT_TIP_SHINE_OVERLAY)
#define PUMP_TIMEOUT_TIP_SHINE_USE_IMG_MASK \
    (PUMP_TIMEOUT_TIP_SHINE_IS_SOFT || PUMP_TIMEOUT_TIP_SHINE_IS_OVERLAY)

#ifndef PUMP_TIMEOUT_TIP_SHINE_W
#if PUMP_TIMEOUT_TIP_SHINE_IS_HARD
#define PUMP_TIMEOUT_TIP_SHINE_W                        (80)
#else
#define PUMP_TIMEOUT_TIP_SHINE_W                        (80)
#endif
#endif

#if PUMP_TIMEOUT_TIP_SHINE_IS_HARD
#define PUMP_TIMEOUT_TIP_MASK_W                         (PUMP_TIMEOUT_TIP_SHINE_W)
#define PUMP_TIMEOUT_TIP_MASK_H                         (PUMP_TIMEOUT_TIP_H)
#define PUMP_TIMEOUT_TIP_MASK_Y                         (PUMP_TIMEOUT_TIP_Y)
#define PUMP_TIMEOUT_TIP_SHINE_RECOLOR_OPA              (LV_OPA_COVER)
#define PUMP_TIMEOUT_TIP_BASE_OPA                       (LV_OPA_COVER)
#else
#define PUMP_TIMEOUT_TIP_MASK_W                         (PUMP_TIMEOUT_TIP_SHINE_W)
#define PUMP_TIMEOUT_TIP_MASK_H                         (PUMP_TIMEOUT_TIP_H)
#define PUMP_TIMEOUT_TIP_MASK_Y                         (PUMP_TIMEOUT_TIP_Y)
#define PUMP_TIMEOUT_TIP_SHINE_RECOLOR_OPA              (LV_OPA_80)
#define PUMP_TIMEOUT_TIP_BASE_OPA                       (LV_OPA_70)
#endif
#define PUMP_TIMEOUT_TIP_MASK_X_MIN                     (-PUMP_TIMEOUT_TIP_MASK_W)
#define PUMP_TIMEOUT_TIP_MASK_X_MAX                     (PANLE_DISPLAY_W)
#define PUMP_TIMEOUT_TIP_MASK_PERIOD_MS                 (3200)
#define PUMP_TIMEOUT_TIP_SHINE_COLOR                    (0xFFFFFF)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *tip_img;
    lv_obj_t *tip_shine;
    lv_obj_t *tip_mask;
#if PUMP_TIMEOUT_TIP_SHINE_USE_IMG_MASK
    lv_draw_buf_t *tip_mask_a8;
#endif
    lv_draw_buf_t *tip_buf;
    int16_t tip_mask_x;
    uint32_t tip_mask_tick_ms;
    uint8_t force_update;
    uint8_t rsv[7];
} pump_timeout_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_timeout_ui_info_t *timeout_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_timeout_root_init(void)
{
    if (timeout_info->root == NULL || !lv_obj_is_valid(timeout_info->root)) {
        timeout_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(timeout_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(timeout_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(timeout_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(timeout_info->root, 0, 0);
    lv_obj_set_style_pad_all(timeout_info->root, 0, 0);
    lv_obj_align(timeout_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(timeout_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(timeout_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_timeout_icon_update(ui_manager_context_t *thiz)
{
    if (timeout_info->force_update == false) {
        return;
    }

    if (timeout_info->icon == NULL || !lv_obj_is_valid(timeout_info->icon)) {
        timeout_info->icon = lv_image_create(timeout_info->root);
        EXCEPTION_CHECK_VOID(timeout_info->icon);
    }
    ui_img_baj_theme_set_src(thiz, timeout_info->icon, BASE_RES_ID_TIMEOUT_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(timeout_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_TIMEOUT_ICON_Y);
}

static void pump_timeout_title_update(void)
{
    if (timeout_info->force_update == false) {
        return;
    }

    if (timeout_info->title == NULL || !lv_obj_is_valid(timeout_info->title)) {
        timeout_info->title = lv_label_create(timeout_info->root);
        EXCEPTION_CHECK_VOID(timeout_info->title);
    }
    lv_label_set_text(timeout_info->title, PUMP_TIMEOUT_TITLE_TEXT);
    lv_obj_set_style_text_color(timeout_info->title, lv_color_hex(PUMP_TIMEOUT_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(timeout_info->title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(timeout_info->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(timeout_info->title, PUMP_TIMEOUT_TITLE_LINE_SPACE, 0);
    lv_obj_align(timeout_info->title, LV_ALIGN_TOP_MID, 0, PUMP_TIMEOUT_TITLE_Y);
}

static lv_draw_buf_t *pump_timeout_tip_predecode(void)
{
    const ui_img_data_t *tip;

    if (timeout_info->tip_buf != NULL) {
        return timeout_info->tip_buf;
    }
    tip = ui_font_img_pump_timeout_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    timeout_info->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return timeout_info->tip_buf;
}

static void pump_timeout_tip_shine_sync(void)
{
    lv_coord_t tip_x;
    lv_coord_t tip_y;

    if (timeout_info->tip_img == NULL || !lv_obj_is_valid(timeout_info->tip_img) ||
        timeout_info->tip_shine == NULL || !lv_obj_is_valid(timeout_info->tip_shine) ||
        timeout_info->tip_mask == NULL || !lv_obj_is_valid(timeout_info->tip_mask)) {
        return;
    }

    lv_obj_update_layout(timeout_info->tip_img);
    tip_x = lv_obj_get_x(timeout_info->tip_img);
    tip_y = lv_obj_get_y(timeout_info->tip_img);
    lv_obj_set_pos(timeout_info->tip_shine,
        tip_x - timeout_info->tip_mask_x, tip_y - PUMP_TIMEOUT_TIP_MASK_Y);
}

#if PUMP_TIMEOUT_TIP_SHINE_USE_IMG_MASK
/** 从预解码 TEXT_SHINE_MASK 生成 A8；高度不足时垂直居中裁切 */
static lv_draw_buf_t *pump_timeout_tip_mask_a8_from_rgba(const lv_draw_buf_t *rgba,
    uint32_t dst_w, uint32_t dst_h)
{
    lv_draw_buf_t *a8;
    uint32_t src_w;
    uint32_t src_h;
    uint32_t y;
    uint32_t x;
    uint32_t y0;
    uint32_t x0;

    if (rgba == NULL || rgba->data == NULL || dst_w == 0U || dst_h == 0U) {
        return NULL;
    }
    if (rgba->header.cf != LV_COLOR_FORMAT_ARGB8888 &&
        rgba->header.cf != LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED) {
        return NULL;
    }

    src_w = rgba->header.w;
    src_h = rgba->header.h;
    if (src_w == 0U || src_h == 0U) {
        return NULL;
    }

    a8 = lv_draw_buf_create(dst_w, dst_h, LV_COLOR_FORMAT_A8, LV_STRIDE_AUTO);
    if (a8 == NULL || a8->data == NULL) {
        if (a8 != NULL) {
            lv_draw_buf_destroy(a8);
        }
        return NULL;
    }

    x0 = (src_w > dst_w) ? ((src_w - dst_w) / 2U) : 0U;
    y0 = (src_h > dst_h) ? ((src_h - dst_h) / 2U) : 0U;

    for (y = 0; y < dst_h; y++) {
        uint32_t sy = (y0 + y < src_h) ? (y0 + y) : (src_h - 1U);
        const lv_color32_t *src =
            (const lv_color32_t *)(rgba->data + sy * rgba->header.stride);
        uint8_t *dst = a8->data + y * a8->header.stride;
        for (x = 0; x < dst_w; x++) {
            uint32_t sx = (x0 + x < src_w) ? (x0 + x) : (src_w - 1U);
            uint32_t a = src[sx].alpha;
#if PUMP_TIMEOUT_TIP_SHINE_IS_SOFT
            uint32_t edge = (x * (dst_w - 1U - x) * 4U * 255U) / (dst_w * dst_w);
            if (edge > 255U) {
                edge = 255U;
            }
            a = (a * edge) / 255U;
            a = (a * a) / 255U;
#endif
            dst[x] = (uint8_t)a;
        }
    }
    return a8;
}

static void pump_timeout_tip_mask_a8_release(void)
{
    if (timeout_info == NULL || timeout_info->tip_mask_a8 == NULL) {
        return;
    }
    if (timeout_info->tip_mask != NULL && lv_obj_is_valid(timeout_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(timeout_info->tip_mask, NULL, 0);
    }
    lv_draw_buf_destroy(timeout_info->tip_mask_a8);
    timeout_info->tip_mask_a8 = NULL;
}

static void pump_timeout_tip_mask_a8_prepare(void)
{
    const lv_draw_buf_t *rgba;

    if (timeout_info->tip_mask_a8 != NULL) {
        if (timeout_info->tip_mask != NULL && lv_obj_is_valid(timeout_info->tip_mask)) {
            lv_obj_set_style_bitmap_mask_src(timeout_info->tip_mask, timeout_info->tip_mask_a8, 0);
        }
        return;
    }

    rgba = get_predecoded_text_shine_mask();
    timeout_info->tip_mask_a8 = pump_timeout_tip_mask_a8_from_rgba(rgba,
        PUMP_TIMEOUT_TIP_MASK_W, PUMP_TIMEOUT_TIP_MASK_H);
    if (timeout_info->tip_mask_a8 != NULL && timeout_info->tip_mask != NULL &&
        lv_obj_is_valid(timeout_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(timeout_info->tip_mask, timeout_info->tip_mask_a8, 0);
    }
}
#endif /* PUMP_TIMEOUT_TIP_SHINE_USE_IMG_MASK */

static void pump_timeout_tip_rebuild(void)
{
    lv_draw_buf_t *predecoded;

    if (timeout_info->tip_img != NULL && lv_obj_is_valid(timeout_info->tip_img) &&
        timeout_info->tip_mask != NULL && lv_obj_is_valid(timeout_info->tip_mask) &&
        timeout_info->tip_shine != NULL && lv_obj_is_valid(timeout_info->tip_shine)) {
        pump_timeout_tip_shine_sync();
        return;
    }

    predecoded = pump_timeout_tip_predecode();

    if (timeout_info->tip_img == NULL || !lv_obj_is_valid(timeout_info->tip_img)) {
        timeout_info->tip_img = lv_image_create(timeout_info->root);
        EXCEPTION_CHECK_VOID(timeout_info->tip_img);
        lv_obj_clear_flag(timeout_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(timeout_info->tip_img, predecoded);
        }
        lv_obj_set_style_image_opa(timeout_info->tip_img, PUMP_TIMEOUT_TIP_BASE_OPA, 0);
        lv_obj_align(timeout_info->tip_img, LV_ALIGN_TOP_MID, 0, PUMP_TIMEOUT_TIP_Y);
    }

    if (timeout_info->tip_mask == NULL || !lv_obj_is_valid(timeout_info->tip_mask)) {
        timeout_info->tip_mask = lv_obj_create(timeout_info->root);
        EXCEPTION_CHECK_VOID(timeout_info->tip_mask);
        lv_obj_remove_style_all(timeout_info->tip_mask);
        lv_obj_set_size(timeout_info->tip_mask, PUMP_TIMEOUT_TIP_MASK_W, PUMP_TIMEOUT_TIP_MASK_H);
        lv_obj_set_style_bg_opa(timeout_info->tip_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(timeout_info->tip_mask, 0, 0);
        lv_obj_set_style_pad_all(timeout_info->tip_mask, 0, 0);
        lv_obj_clear_flag(timeout_info->tip_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(timeout_info->tip_mask, LV_OBJ_FLAG_CLICKABLE);
        timeout_info->tip_mask_x = PUMP_TIMEOUT_TIP_MASK_X_MIN;
        timeout_info->tip_mask_tick_ms = get_timestamp();
    }

    if (timeout_info->tip_shine == NULL || !lv_obj_is_valid(timeout_info->tip_shine)) {
        timeout_info->tip_shine = lv_image_create(timeout_info->tip_mask);
        EXCEPTION_CHECK_VOID(timeout_info->tip_shine);
        lv_obj_clear_flag(timeout_info->tip_shine, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(timeout_info->tip_shine, predecoded);
        }
        lv_obj_set_style_image_recolor(timeout_info->tip_shine,
            lv_color_hex(PUMP_TIMEOUT_TIP_SHINE_COLOR), 0);
        lv_obj_set_style_image_recolor_opa(timeout_info->tip_shine,
            PUMP_TIMEOUT_TIP_SHINE_RECOLOR_OPA, 0);
        lv_obj_set_style_blend_mode(timeout_info->tip_shine, LV_BLEND_MODE_NORMAL, 0);
    }

#if PUMP_TIMEOUT_TIP_SHINE_USE_IMG_MASK
    pump_timeout_tip_mask_a8_prepare();
#endif
    lv_obj_align(timeout_info->tip_mask, LV_ALIGN_TOP_LEFT,
        timeout_info->tip_mask_x, PUMP_TIMEOUT_TIP_MASK_Y);
    lv_obj_move_foreground(timeout_info->tip_img);
    lv_obj_move_foreground(timeout_info->tip_mask);
    pump_timeout_tip_shine_sync();
}

static void pump_timeout_tip_mask_anim_update(void)
{
    uint32_t elapsed;
    int32_t x;

    if (timeout_info->tip_mask == NULL || !lv_obj_is_valid(timeout_info->tip_mask)) {
        return;
    }

    elapsed = get_timestamp() - timeout_info->tip_mask_tick_ms;
    if (elapsed >= PUMP_TIMEOUT_TIP_MASK_PERIOD_MS) {
        timeout_info->tip_mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = PUMP_TIMEOUT_TIP_MASK_X_MIN +
        (int32_t)((PUMP_TIMEOUT_TIP_MASK_X_MAX - PUMP_TIMEOUT_TIP_MASK_X_MIN) * elapsed) /
        PUMP_TIMEOUT_TIP_MASK_PERIOD_MS;
    if (x != timeout_info->tip_mask_x) {
        timeout_info->tip_mask_x = (int16_t)x;
        lv_obj_set_x(timeout_info->tip_mask, timeout_info->tip_mask_x);
        pump_timeout_tip_shine_sync();
    }
}

static void pump_timeout_tip_update(void)
{
    if (timeout_info->force_update) {
        pump_timeout_tip_rebuild();
    }
    pump_timeout_tip_mask_anim_update();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_timeout_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (timeout_info == NULL) {
        timeout_info = (pump_timeout_ui_info_t *)u_malloz(sizeof(pump_timeout_ui_info_t));
        EXCEPTION_CHECK_VOID(timeout_info);
    }
    timeout_info->force_update = true;
    pump_timeout_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_timeout_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && timeout_info && timeout_info->root);
    ui_img_baj_release_decoded_buf(timeout_info->icon);
    if (timeout_info->tip_shine != NULL && lv_obj_is_valid(timeout_info->tip_shine)) {
        lv_image_set_src(timeout_info->tip_shine, NULL);
        LVGL_OBJ_DELETE(timeout_info->tip_shine);
        timeout_info->tip_shine = NULL;
    }
#if PUMP_TIMEOUT_TIP_SHINE_USE_IMG_MASK
    pump_timeout_tip_mask_a8_release();
#endif
    if (timeout_info->tip_mask != NULL && lv_obj_is_valid(timeout_info->tip_mask)) {
        LVGL_OBJ_DELETE(timeout_info->tip_mask);
        timeout_info->tip_mask = NULL;
    }
    if (timeout_info->tip_img != NULL && lv_obj_is_valid(timeout_info->tip_img)) {
        lv_image_set_src(timeout_info->tip_img, NULL);
    }
    if (timeout_info->tip_buf != NULL) {
        lv_draw_buf_destroy(timeout_info->tip_buf);
        timeout_info->tip_buf = NULL;
    }
    LVGL_OBJ_DELETE(timeout_info->root);
    u_free(timeout_info);
    timeout_info = NULL;
}

int ui_display_pump_timeout_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && timeout_info && timeout_info->root);
    pump_timeout_icon_update(thiz);
    pump_timeout_title_update();
    pump_timeout_tip_update();
    timeout_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump timeout fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
