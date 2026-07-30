/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "rhythm_confirm_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_RHYTHM_CONFIRM_COMP_BG               (0) /// 是否启用对比背景图

#define RHYTHM_CONFIRM_ICON_Y                           (89)
#define RHYTHM_CONFIRM_TITLE_Y                          (198)
#define RHYTHM_CONFIRM_SUBTITLE_Y                       (239)
#define RHYTHM_CONFIRM_TITLE_COLOR                      (0xFFFFFF)
#define RHYTHM_CONFIRM_SUBTITLE_COLOR                   (0x999999)
#define RHYTHM_CONFIRM_TITLE_TEXT                       "Switch to Program?"
#define RHYTHM_CONFIRM_SUBTITLE_TEXT                    "Timer will restart"

#define RHYTHM_CONFIRM_TIP_W                            (212)
#define RHYTHM_CONFIRM_TIP_H                            (50)
/** 底部提示图 Y：贴底留边（与 home 模式选择页一致） */
#define RHYTHM_CONFIRM_TIP_Y                            (PANLE_DISPLAY_H - RHYTHM_CONFIRM_TIP_H - 8)

/** 提示扫光方式（三选一，与 home 模式页取值一致） */
#define RHYTHM_CONFIRM_TIP_SHINE_HARD                   (0)
#define RHYTHM_CONFIRM_TIP_SHINE_SOFT                   (1)
#define RHYTHM_CONFIRM_TIP_SHINE_OVERLAY                (2)
#ifndef RHYTHM_CONFIRM_TIP_SHINE_MODE
#define RHYTHM_CONFIRM_TIP_SHINE_MODE                   RHYTHM_CONFIRM_TIP_SHINE_OVERLAY
#endif
#define RHYTHM_CONFIRM_TIP_SHINE_IS_HARD \
    (RHYTHM_CONFIRM_TIP_SHINE_MODE == RHYTHM_CONFIRM_TIP_SHINE_HARD)
#define RHYTHM_CONFIRM_TIP_SHINE_IS_SOFT \
    (RHYTHM_CONFIRM_TIP_SHINE_MODE == RHYTHM_CONFIRM_TIP_SHINE_SOFT)
#define RHYTHM_CONFIRM_TIP_SHINE_IS_OVERLAY \
    (RHYTHM_CONFIRM_TIP_SHINE_MODE == RHYTHM_CONFIRM_TIP_SHINE_OVERLAY)
#define RHYTHM_CONFIRM_TIP_SHINE_USE_IMG_MASK \
    (RHYTHM_CONFIRM_TIP_SHINE_IS_SOFT || RHYTHM_CONFIRM_TIP_SHINE_IS_OVERLAY)

#ifndef RHYTHM_CONFIRM_TIP_SHINE_W
#if RHYTHM_CONFIRM_TIP_SHINE_IS_HARD
#define RHYTHM_CONFIRM_TIP_SHINE_W                      (80)
#else
#define RHYTHM_CONFIRM_TIP_SHINE_W                      (80)
#endif
#endif

#if RHYTHM_CONFIRM_TIP_SHINE_IS_HARD
#define RHYTHM_CONFIRM_TIP_MASK_W                       (RHYTHM_CONFIRM_TIP_SHINE_W)
#define RHYTHM_CONFIRM_TIP_MASK_H                       (RHYTHM_CONFIRM_TIP_H)
#define RHYTHM_CONFIRM_TIP_MASK_Y                       (RHYTHM_CONFIRM_TIP_Y)
#define RHYTHM_CONFIRM_TIP_SHINE_RECOLOR_OPA            (LV_OPA_COVER)
#define RHYTHM_CONFIRM_TIP_BASE_OPA                     (LV_OPA_COVER)
#else
#define RHYTHM_CONFIRM_TIP_MASK_W                       (RHYTHM_CONFIRM_TIP_SHINE_W)
#define RHYTHM_CONFIRM_TIP_MASK_H                       (RHYTHM_CONFIRM_TIP_H)
#define RHYTHM_CONFIRM_TIP_MASK_Y                       (RHYTHM_CONFIRM_TIP_Y)
#define RHYTHM_CONFIRM_TIP_SHINE_RECOLOR_OPA            (LV_OPA_80)
#define RHYTHM_CONFIRM_TIP_BASE_OPA                     (LV_OPA_70)
#endif
#define RHYTHM_CONFIRM_TIP_MASK_X_MIN                   (-RHYTHM_CONFIRM_TIP_MASK_W)
#define RHYTHM_CONFIRM_TIP_MASK_X_MAX                   (PANLE_DISPLAY_W)
#define RHYTHM_CONFIRM_TIP_MASK_PERIOD_MS               (3200)
#define RHYTHM_CONFIRM_TIP_SHINE_COLOR                  (0xFFFFFF)

/** 提示文案：1=内嵌图片代码段（默认），0=资源包 BASE_RES_ID_MODE_SELECT_TIP */
#ifndef RHYTHM_CONFIRM_TIP_FROM_CODE
#define RHYTHM_CONFIRM_TIP_FROM_CODE                    (1)
#endif

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_RHYTHM_CONFIRM_COMP_BG
    lv_obj_t *background;
#endif
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *tip_img;
    lv_obj_t *tip_shine;
    lv_obj_t *tip_mask;
#if RHYTHM_CONFIRM_TIP_SHINE_USE_IMG_MASK
    lv_draw_buf_t *tip_mask_a8;
#endif
    uint8_t tip_predecoded;
    int16_t tip_mask_x;
    uint32_t tip_mask_tick_ms;
    uint8_t force_update;
    uint8_t rsv[7];
} rhythm_confirm_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_RHYTHM_CONFIRM_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t rhythm_confirm_bg_data[] = {
    #include "comparison_png2txt/pump_home_22_352x352.txt"
};
#endif

static rhythm_confirm_ui_info_t *confirm_info;

/** 提示文案预解码缓存（本模块持有，页面侧勿 destroy） */
static lv_draw_buf_t *s_rhythm_confirm_tip_predecoded = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void rhythm_confirm_root_init(void)
{
    if (confirm_info->root == NULL || !lv_obj_is_valid(confirm_info->root)) {
        confirm_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(confirm_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(confirm_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(confirm_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(confirm_info->root, 0, 0);
    lv_obj_set_style_pad_all(confirm_info->root, 0, 0);
    lv_obj_align(confirm_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(confirm_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(confirm_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_RHYTHM_CONFIRM_COMP_BG
static void rhythm_confirm_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (confirm_info->background == NULL || !lv_obj_is_valid(confirm_info->background)) {
        confirm_info->background = lv_image_create(confirm_info->root);
        EXCEPTION_CHECK_VOID(confirm_info->background);
        lv_obj_move_background(confirm_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(confirm_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(confirm_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, rhythm_confirm_bg_data,
            sizeof(rhythm_confirm_bg_data), "png");
        lv_image_set_src(confirm_info->background, (const void *)&bg_img_path);
        lv_obj_align(confirm_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void rhythm_confirm_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static void rhythm_confirm_icon_update(ui_manager_context_t *thiz)
{
    if (confirm_info->force_update == false) {
        return;
    }

    if (confirm_info->icon == NULL || !lv_obj_is_valid(confirm_info->icon)) {
        confirm_info->icon = lv_image_create(confirm_info->root);
        EXCEPTION_CHECK_VOID(confirm_info->icon);
    }
    ui_img_baj_theme_set_src(thiz, confirm_info->icon, BASE_RES_ID_QUANDARY_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(confirm_info->icon, LV_ALIGN_TOP_MID, 0, RHYTHM_CONFIRM_ICON_Y);
}

static void rhythm_confirm_title_update(void)
{
    if (confirm_info->force_update == false) {
        return;
    }

    if (confirm_info->title == NULL || !lv_obj_is_valid(confirm_info->title)) {
        confirm_info->title = lv_label_create(confirm_info->root);
        EXCEPTION_CHECK_VOID(confirm_info->title);
    }
    lv_label_set_text(confirm_info->title, RHYTHM_CONFIRM_TITLE_TEXT);
    lv_obj_set_style_text_color(confirm_info->title, lv_color_hex(RHYTHM_CONFIRM_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(confirm_info->title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(confirm_info->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(confirm_info->title, LV_ALIGN_TOP_MID, 0, RHYTHM_CONFIRM_TITLE_Y);
}

static void rhythm_confirm_subtitle_update(void)
{
    if (confirm_info->force_update == false) {
        return;
    }

    if (confirm_info->subtitle == NULL || !lv_obj_is_valid(confirm_info->subtitle)) {
        confirm_info->subtitle = lv_label_create(confirm_info->root);
        EXCEPTION_CHECK_VOID(confirm_info->subtitle);
    }
    lv_label_set_text(confirm_info->subtitle, RHYTHM_CONFIRM_SUBTITLE_TEXT);
    lv_obj_set_style_text_color(confirm_info->subtitle,
        lv_color_hex(RHYTHM_CONFIRM_SUBTITLE_COLOR), 0);
    lv_obj_set_style_text_font(confirm_info->subtitle, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(confirm_info->subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(confirm_info->subtitle, LV_ALIGN_TOP_MID, 0, RHYTHM_CONFIRM_SUBTITLE_Y);
}

static lv_draw_buf_t *rhythm_confirm_tip_predecode(ui_manager_context_t *ui)
{
    if (s_rhythm_confirm_tip_predecoded != NULL) {
        return s_rhythm_confirm_tip_predecoded;
    }
#if RHYTHM_CONFIRM_TIP_FROM_CODE
    (void)ui;
    {
        const ui_img_data_t *tip = ui_font_img_mode_select_tip_get();
        if (tip == NULL || tip->data == NULL || tip->size == 0U) {
            return NULL;
        }
        s_rhythm_confirm_tip_predecoded = ui_img_png_decode_draw_buf(tip->data, tip->size,
            UI_IMG_PNG_DECODE_RGBA8888);
    }
#else
    if (ui == NULL) {
        return NULL;
    }
    s_rhythm_confirm_tip_predecoded = ui_img_baj_theme_decode_draw_buf(ui,
        BASE_RES_ID_MODE_SELECT_TIP, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
#endif
    return s_rhythm_confirm_tip_predecoded;
}

static void rhythm_confirm_tip_shine_sync(void)
{
    lv_coord_t tip_x;
    lv_coord_t tip_y;

    if (confirm_info->tip_img == NULL || !lv_obj_is_valid(confirm_info->tip_img) ||
        confirm_info->tip_shine == NULL || !lv_obj_is_valid(confirm_info->tip_shine) ||
        confirm_info->tip_mask == NULL || !lv_obj_is_valid(confirm_info->tip_mask)) {
        return;
    }

    lv_obj_update_layout(confirm_info->tip_img);
    tip_x = lv_obj_get_x(confirm_info->tip_img);
    tip_y = lv_obj_get_y(confirm_info->tip_img);
    lv_obj_set_pos(confirm_info->tip_shine,
        tip_x - confirm_info->tip_mask_x, tip_y - RHYTHM_CONFIRM_TIP_MASK_Y);
}

#if RHYTHM_CONFIRM_TIP_SHINE_USE_IMG_MASK
/** 从预解码 TEXT_SHINE_MASK 生成 A8；高度不足时垂直居中裁切 */
static lv_draw_buf_t *rhythm_confirm_tip_mask_a8_from_rgba(const lv_draw_buf_t *rgba,
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
#if RHYTHM_CONFIRM_TIP_SHINE_IS_SOFT
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

static void rhythm_confirm_tip_mask_a8_release(void)
{
    if (confirm_info == NULL || confirm_info->tip_mask_a8 == NULL) {
        return;
    }
    if (confirm_info->tip_mask != NULL && lv_obj_is_valid(confirm_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(confirm_info->tip_mask, NULL, 0);
    }
    lv_draw_buf_destroy(confirm_info->tip_mask_a8);
    confirm_info->tip_mask_a8 = NULL;
}

static void rhythm_confirm_tip_mask_a8_prepare(void)
{
    const lv_draw_buf_t *rgba;

    if (confirm_info->tip_mask_a8 != NULL) {
        if (confirm_info->tip_mask != NULL && lv_obj_is_valid(confirm_info->tip_mask)) {
            lv_obj_set_style_bitmap_mask_src(confirm_info->tip_mask, confirm_info->tip_mask_a8, 0);
        }
        return;
    }

    rgba = get_predecoded_text_shine_mask();
    confirm_info->tip_mask_a8 = rhythm_confirm_tip_mask_a8_from_rgba(rgba,
        RHYTHM_CONFIRM_TIP_MASK_W, RHYTHM_CONFIRM_TIP_MASK_H);
    if (confirm_info->tip_mask_a8 != NULL && confirm_info->tip_mask != NULL &&
        lv_obj_is_valid(confirm_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(confirm_info->tip_mask, confirm_info->tip_mask_a8, 0);
    }
}
#endif /* RHYTHM_CONFIRM_TIP_SHINE_USE_IMG_MASK */

static void rhythm_confirm_tip_rebuild(ui_manager_context_t *thiz)
{
    lv_draw_buf_t *predecoded;

    if (confirm_info->tip_img != NULL && lv_obj_is_valid(confirm_info->tip_img) &&
        confirm_info->tip_mask != NULL && lv_obj_is_valid(confirm_info->tip_mask) &&
        confirm_info->tip_shine != NULL && lv_obj_is_valid(confirm_info->tip_shine)) {
        rhythm_confirm_tip_shine_sync();
        return;
    }

    predecoded = rhythm_confirm_tip_predecode(thiz);

    if (confirm_info->tip_img == NULL || !lv_obj_is_valid(confirm_info->tip_img)) {
        confirm_info->tip_img = lv_image_create(confirm_info->root);
        EXCEPTION_CHECK_VOID(confirm_info->tip_img);
        lv_obj_clear_flag(confirm_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(confirm_info->tip_img, predecoded);
            confirm_info->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(confirm_info->tip_img, RHYTHM_CONFIRM_TIP_BASE_OPA, 0);
        lv_obj_align(confirm_info->tip_img, LV_ALIGN_TOP_MID, 0, RHYTHM_CONFIRM_TIP_Y);
    }

    if (confirm_info->tip_mask == NULL || !lv_obj_is_valid(confirm_info->tip_mask)) {
        confirm_info->tip_mask = lv_obj_create(confirm_info->root);
        EXCEPTION_CHECK_VOID(confirm_info->tip_mask);
        lv_obj_remove_style_all(confirm_info->tip_mask);
        lv_obj_set_size(confirm_info->tip_mask, RHYTHM_CONFIRM_TIP_MASK_W, RHYTHM_CONFIRM_TIP_MASK_H);
        lv_obj_set_style_bg_opa(confirm_info->tip_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(confirm_info->tip_mask, 0, 0);
        lv_obj_set_style_pad_all(confirm_info->tip_mask, 0, 0);
        lv_obj_clear_flag(confirm_info->tip_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(confirm_info->tip_mask, LV_OBJ_FLAG_CLICKABLE);
        confirm_info->tip_mask_x = RHYTHM_CONFIRM_TIP_MASK_X_MIN;
        confirm_info->tip_mask_tick_ms = get_timestamp();
    }

    if (confirm_info->tip_shine == NULL || !lv_obj_is_valid(confirm_info->tip_shine)) {
        confirm_info->tip_shine = lv_image_create(confirm_info->tip_mask);
        EXCEPTION_CHECK_VOID(confirm_info->tip_shine);
        lv_obj_clear_flag(confirm_info->tip_shine, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(confirm_info->tip_shine, predecoded);
        }
        lv_obj_set_style_image_recolor(confirm_info->tip_shine,
            lv_color_hex(RHYTHM_CONFIRM_TIP_SHINE_COLOR), 0);
        lv_obj_set_style_image_recolor_opa(confirm_info->tip_shine,
            RHYTHM_CONFIRM_TIP_SHINE_RECOLOR_OPA, 0);
        lv_obj_set_style_blend_mode(confirm_info->tip_shine, LV_BLEND_MODE_NORMAL, 0);
    }

#if RHYTHM_CONFIRM_TIP_SHINE_USE_IMG_MASK
    rhythm_confirm_tip_mask_a8_prepare();
#endif
    lv_obj_align(confirm_info->tip_mask, LV_ALIGN_TOP_LEFT,
        confirm_info->tip_mask_x, RHYTHM_CONFIRM_TIP_MASK_Y);
    lv_obj_move_foreground(confirm_info->tip_img);
    lv_obj_move_foreground(confirm_info->tip_mask);
    rhythm_confirm_tip_shine_sync();
}

static void rhythm_confirm_tip_mask_anim_update(void)
{
    uint32_t elapsed;
    int32_t x;

    if (confirm_info->tip_mask == NULL || !lv_obj_is_valid(confirm_info->tip_mask)) {
        return;
    }

    elapsed = get_timestamp() - confirm_info->tip_mask_tick_ms;
    if (elapsed >= RHYTHM_CONFIRM_TIP_MASK_PERIOD_MS) {
        confirm_info->tip_mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = RHYTHM_CONFIRM_TIP_MASK_X_MIN +
        (int32_t)((RHYTHM_CONFIRM_TIP_MASK_X_MAX - RHYTHM_CONFIRM_TIP_MASK_X_MIN) * elapsed) /
        RHYTHM_CONFIRM_TIP_MASK_PERIOD_MS;
    if (x != confirm_info->tip_mask_x) {
        confirm_info->tip_mask_x = (int16_t)x;
        lv_obj_set_x(confirm_info->tip_mask, confirm_info->tip_mask_x);
        rhythm_confirm_tip_shine_sync();
    }
}

static void rhythm_confirm_tip_update(ui_manager_context_t *thiz)
{
    if (confirm_info->force_update) {
        rhythm_confirm_tip_rebuild(thiz);
    }
    rhythm_confirm_tip_mask_anim_update();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_rhythm_confirm_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (confirm_info == NULL) {
        confirm_info = (rhythm_confirm_ui_info_t *)u_malloz(sizeof(rhythm_confirm_ui_info_t));
        EXCEPTION_CHECK_VOID(confirm_info);
    }
    confirm_info->force_update = true;
    rhythm_confirm_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_rhythm_confirm_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && confirm_info && confirm_info->root);
    ui_img_baj_release_decoded_buf(confirm_info->icon);
    if (confirm_info->tip_shine != NULL && lv_obj_is_valid(confirm_info->tip_shine)) {
        lv_image_set_src(confirm_info->tip_shine, NULL);
        LVGL_OBJ_DELETE(confirm_info->tip_shine);
        confirm_info->tip_shine = NULL;
    }
#if RHYTHM_CONFIRM_TIP_SHINE_USE_IMG_MASK
    rhythm_confirm_tip_mask_a8_release();
#endif
    if (confirm_info->tip_mask != NULL && lv_obj_is_valid(confirm_info->tip_mask)) {
        LVGL_OBJ_DELETE(confirm_info->tip_mask);
        confirm_info->tip_mask = NULL;
    }
    if (confirm_info->tip_img != NULL && lv_obj_is_valid(confirm_info->tip_img)) {
        if (confirm_info->tip_predecoded) {
            lv_image_set_src(confirm_info->tip_img, NULL);
        } else {
            ui_img_baj_release_decoded_buf(confirm_info->tip_img);
        }
        LVGL_OBJ_DELETE(confirm_info->tip_img);
        confirm_info->tip_img = NULL;
    }
    LVGL_OBJ_DELETE(confirm_info->root);
    u_free(confirm_info);
    confirm_info = NULL;
}

int ui_display_rhythm_confirm_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && confirm_info && confirm_info->root);
    rhythm_confirm_bg_update(thiz);
    rhythm_confirm_icon_update(thiz);
    rhythm_confirm_title_update();
    rhythm_confirm_subtitle_update();
    rhythm_confirm_tip_update(thiz);
    confirm_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> rhythm confirm fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
