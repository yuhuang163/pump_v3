/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_SCREEN_SAVER_COMP_BG                (0)

#define HOME_SAVER_SCREEN_W                                 (360)

#define HOME_SAVER_HINT_Y                                   (90)
#define HOME_SAVER_HINT_TEXT                                "Press the knob to unlock"
#if IS_SUPPORT_HOME_SCREEN_SAVER_COMP_BG
#define HOME_SAVER_HINT_COLOR                               (0x00FF00)
#define HOME_SAVER_HINT_SHINE_COLOR                         (0x00FF00)
#else
#define HOME_SAVER_HINT_COLOR                               (0x999999)
#define HOME_SAVER_HINT_SHINE_COLOR                         (0xFFFFFF)
#endif

/** 提示扫光方式（三选一，与 home 主页取值一致） */
#define HOME_SAVER_HINT_SHINE_HARD                          (0) /// 硬边裁切亮色文案
#define HOME_SAVER_HINT_SHINE_SOFT                          (1) /// A8 软边（PNG alpha × 抛物线）
#define HOME_SAVER_HINT_SHINE_OVERLAY                       (2) /// 图片蒙层（直接用 TEXT_SHINE_MASK alpha）
#ifndef HOME_SAVER_HINT_SHINE_MODE
#define HOME_SAVER_HINT_SHINE_MODE                          HOME_SAVER_HINT_SHINE_OVERLAY
#endif
#define HOME_SAVER_HINT_SHINE_IS_HARD \
    (HOME_SAVER_HINT_SHINE_MODE == HOME_SAVER_HINT_SHINE_HARD)
#define HOME_SAVER_HINT_SHINE_IS_SOFT \
    (HOME_SAVER_HINT_SHINE_MODE == HOME_SAVER_HINT_SHINE_SOFT)
#define HOME_SAVER_HINT_SHINE_IS_OVERLAY \
    (HOME_SAVER_HINT_SHINE_MODE == HOME_SAVER_HINT_SHINE_OVERLAY)
/** SOFT/OVERLAY：使用 TEXT_SHINE_MASK 做 bitmap 蒙层 */
#define HOME_SAVER_HINT_SHINE_USE_IMG_MASK \
    (HOME_SAVER_HINT_SHINE_IS_SOFT || HOME_SAVER_HINT_SHINE_IS_OVERLAY)

/**
 * 扫光宽度（像素）。改此宏即可控制光带宽窄。
 * HARD 默认 80；SOFT/OVERLAY 默认 140（与 TEXT_SHINE_MASK 同宽，可小于资源图做居中裁切）。
 */
#ifndef HOME_SAVER_HINT_SHINE_W
#if HOME_SAVER_HINT_SHINE_IS_HARD
#define HOME_SAVER_HINT_SHINE_W                             (80)
#else
#define HOME_SAVER_HINT_SHINE_W                             (140)
#endif
#endif

#if HOME_SAVER_HINT_SHINE_IS_HARD
#define HOME_SAVER_MASK_W                                   (HOME_SAVER_HINT_SHINE_W)
#define HOME_SAVER_MASK_H                                   (40)
#define HOME_SAVER_MASK_Y                                   (HOME_SAVER_HINT_Y - 8)
#else
/** SOFT / OVERLAY：宽由 HOME_SAVER_HINT_SHINE_W 控制，高对齐 TEXT_SHINE_MASK */
#define HOME_SAVER_MASK_W                                   (HOME_SAVER_HINT_SHINE_W)
#define HOME_SAVER_MASK_H                                   (80)
#define HOME_SAVER_MASK_Y                                   (HOME_SAVER_HINT_Y - 28)
#endif
#define HOME_SAVER_MASK_X_MIN                               (-HOME_SAVER_MASK_W)
#define HOME_SAVER_MASK_X_MAX                               (HOME_SAVER_SCREEN_W)
#define HOME_SAVER_MASK_PERIOD_MS                           (3200)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_HOME_SCREEN_SAVER_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * black_ring;
    lv_obj_t * saver_icon;
    lv_obj_t * hint_text;                /// Press the knob to unlock
    lv_obj_t * hint_shine;               /// 扫光亮色文案（裁剪窗内）
    lv_obj_t * hint_mask;                /// 扫光裁剪窗
#if HOME_SAVER_HINT_SHINE_USE_IMG_MASK
    lv_draw_buf_t * hint_mask_a8;        /// TEXT_SHINE_MASK → A8 蒙层
#endif
    int16_t mask_x;
    uint32_t mask_tick_ms;
    uint8_t force_update;
    uint8_t rsv[7];
} home_screen_saver_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static home_screen_saver_display_info_t *saver_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG
static PSRAM_DATA_LOC uint8_t home_screen_saver_bg_data[] = {
    #include "comparison_png2txt/home_idle_360x360.txt"
};
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_screen_saver_root_init(void)
{
    if (saver_info->root == NULL || !lv_obj_is_valid(saver_info->root)) {
        saver_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(saver_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(saver_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(saver_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(saver_info->root, 0, 0);
    lv_obj_set_style_pad_all(saver_info->root, 0, 0);
    lv_obj_align(saver_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(saver_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(saver_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG
static void home_screen_saver_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (saver_info->backgroud == NULL || !lv_obj_is_valid(saver_info->backgroud)) {
        saver_info->backgroud = lv_image_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(saver_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(saver_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_screen_saver_bg_data,
            sizeof(home_screen_saver_bg_data), "png");
        lv_image_set_src(saver_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(saver_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_screen_saver_root_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static void home_screen_saver_icon_prepare(ui_manager_context_t *thiz)
{
    if (saver_info->saver_icon == NULL || !lv_obj_is_valid(saver_info->saver_icon)) {
        saver_info->saver_icon = lv_image_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->saver_icon);
    }
    ui_img_png_theme_set_src(thiz, saver_info->saver_icon, BASE_RES_ID_HOME_IDLE_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(saver_info->saver_icon, LV_ALIGN_CENTER, 0, 0);
}

static void home_screen_saver_hint_shine_sync(void)
{
    lv_coord_t text_x;
    lv_coord_t text_y;

    if (saver_info->hint_text == NULL || !lv_obj_is_valid(saver_info->hint_text) ||
        saver_info->hint_shine == NULL || !lv_obj_is_valid(saver_info->hint_shine) ||
        saver_info->hint_mask == NULL || !lv_obj_is_valid(saver_info->hint_mask)) {
        return;
    }

    lv_obj_update_layout(saver_info->hint_text);
    text_x = lv_obj_get_x(saver_info->hint_text);
    text_y = lv_obj_get_y(saver_info->hint_text);
    lv_obj_set_pos(saver_info->hint_shine,
        text_x - saver_info->mask_x, text_y - HOME_SAVER_MASK_Y);
}

#if HOME_SAVER_HINT_SHINE_USE_IMG_MASK
/** 从预解码 TEXT_SHINE_MASK 生成 A8；宽高不足时居中裁切 */
static lv_draw_buf_t *home_screen_saver_hint_mask_a8_from_rgba(const lv_draw_buf_t *rgba,
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
#if HOME_SAVER_HINT_SHINE_IS_SOFT
            /* SOFT：再叠加抛物线，两端更透 */
            uint32_t edge = (x * (dst_w - 1U - x) * 4U * 255U) / (dst_w * dst_w);
            if (edge > 255U) {
                edge = 255U;
            }
            a = (a * edge) / 255U;
            a = (a * a) / 255U;
#endif
            /* OVERLAY：直接使用 TEXT_SHINE_MASK 的 alpha */
            dst[x] = (uint8_t)a;
        }
    }
    return a8;
}

static void home_screen_saver_hint_mask_a8_release(void)
{
    if (saver_info == NULL || saver_info->hint_mask_a8 == NULL) {
        return;
    }
    if (saver_info->hint_mask != NULL && lv_obj_is_valid(saver_info->hint_mask)) {
        lv_obj_set_style_bitmap_mask_src(saver_info->hint_mask, NULL, 0);
    }
    lv_draw_buf_destroy(saver_info->hint_mask_a8);
    saver_info->hint_mask_a8 = NULL;
}

static void home_screen_saver_hint_mask_a8_prepare(void)
{
    const lv_draw_buf_t *rgba;

    if (saver_info->hint_mask_a8 != NULL) {
        if (saver_info->hint_mask != NULL && lv_obj_is_valid(saver_info->hint_mask)) {
            lv_obj_set_style_bitmap_mask_src(saver_info->hint_mask, saver_info->hint_mask_a8, 0);
        }
        return;
    }

    rgba = get_predecoded_text_shine_mask();
    saver_info->hint_mask_a8 = home_screen_saver_hint_mask_a8_from_rgba(rgba,
        HOME_SAVER_MASK_W, HOME_SAVER_MASK_H);
    if (saver_info->hint_mask_a8 != NULL && saver_info->hint_mask != NULL &&
        lv_obj_is_valid(saver_info->hint_mask)) {
        lv_obj_set_style_bitmap_mask_src(saver_info->hint_mask, saver_info->hint_mask_a8, 0);
    }
}
#endif /* HOME_SAVER_HINT_SHINE_USE_IMG_MASK */

static void home_screen_saver_hint_text_update(void)
{
    if (saver_info->hint_text != NULL && lv_obj_is_valid(saver_info->hint_text) &&
        saver_info->force_update == false) {
        return;
    }

    if (saver_info->hint_text == NULL || !lv_obj_is_valid(saver_info->hint_text)) {
        saver_info->hint_text = lv_label_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->hint_text);
        lv_obj_set_style_text_font(saver_info->hint_text, &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(saver_info->hint_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(saver_info->hint_text, 0, 0);
    }
    lv_obj_set_style_text_color(saver_info->hint_text, lv_color_hex(HOME_SAVER_HINT_COLOR), 0);
    lv_label_set_text(saver_info->hint_text, HOME_SAVER_HINT_TEXT);
    lv_obj_align(saver_info->hint_text, LV_ALIGN_TOP_MID, 0, HOME_SAVER_HINT_Y);
}

/** 裁剪窗内亮色文案扫光；SOFT/OVERLAY 再用 TEXT_SHINE_MASK 做 A8 蒙层 */
static void home_screen_saver_hint_mask_update(void)
{
    if (saver_info->hint_mask != NULL && lv_obj_is_valid(saver_info->hint_mask) &&
        saver_info->hint_shine != NULL && lv_obj_is_valid(saver_info->hint_shine) &&
        saver_info->force_update == false) {
        home_screen_saver_hint_shine_sync();
        return;
    }

    if (saver_info->hint_mask == NULL || !lv_obj_is_valid(saver_info->hint_mask)) {
        saver_info->hint_mask = lv_obj_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->hint_mask);
        lv_obj_remove_style_all(saver_info->hint_mask);
        lv_obj_set_size(saver_info->hint_mask, HOME_SAVER_MASK_W, HOME_SAVER_MASK_H);
        lv_obj_set_style_bg_opa(saver_info->hint_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(saver_info->hint_mask, 0, 0);
        lv_obj_set_style_pad_all(saver_info->hint_mask, 0, 0);
        lv_obj_clear_flag(saver_info->hint_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(saver_info->hint_mask, LV_OBJ_FLAG_CLICKABLE);
        saver_info->mask_x = HOME_SAVER_MASK_X_MIN;
        saver_info->mask_tick_ms = get_timestamp();
    }

    if (saver_info->hint_shine == NULL || !lv_obj_is_valid(saver_info->hint_shine)) {
        saver_info->hint_shine = lv_label_create(saver_info->hint_mask);
        EXCEPTION_CHECK_VOID(saver_info->hint_shine);
        lv_obj_set_style_text_color(saver_info->hint_shine,
            lv_color_hex(HOME_SAVER_HINT_SHINE_COLOR), 0);
        lv_obj_set_style_text_font(saver_info->hint_shine, &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(saver_info->hint_shine, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(saver_info->hint_shine, 0, 0);
    }
    lv_label_set_text(saver_info->hint_shine, HOME_SAVER_HINT_TEXT);

#if HOME_SAVER_HINT_SHINE_USE_IMG_MASK
    home_screen_saver_hint_mask_a8_prepare();
#endif

    lv_obj_align(saver_info->hint_mask, LV_ALIGN_TOP_LEFT, saver_info->mask_x, HOME_SAVER_MASK_Y);
    lv_obj_move_foreground(saver_info->hint_mask);
    home_screen_saver_hint_shine_sync();
}

static void home_screen_saver_hint_update(void)
{
    home_screen_saver_hint_text_update();
    home_screen_saver_hint_mask_update();
}

static void home_screen_saver_mask_anim_update(void)
{
    uint32_t elapsed;
    int32_t x;

    if (saver_info->hint_mask == NULL || !lv_obj_is_valid(saver_info->hint_mask)) {
        return;
    }

    elapsed = get_timestamp() - saver_info->mask_tick_ms;
    if (elapsed >= HOME_SAVER_MASK_PERIOD_MS) {
        saver_info->mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = HOME_SAVER_MASK_X_MIN +
        (int32_t)((HOME_SAVER_MASK_X_MAX - HOME_SAVER_MASK_X_MIN) * elapsed) /
        HOME_SAVER_MASK_PERIOD_MS;
    if (x != saver_info->mask_x) {
        saver_info->mask_x = (int16_t)x;
        lv_obj_set_x(saver_info->hint_mask, saver_info->mask_x);
        home_screen_saver_hint_shine_sync();
    }
}

static void home_screen_saver_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
    (void)proc;
    if (saver_info->force_update == false) {
        return;
    }
    home_screen_saver_icon_prepare(thiz);
    home_screen_saver_hint_update();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_screen_saver_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (saver_info == NULL) {
        saver_info = (home_screen_saver_display_info_t *)u_malloz(sizeof(home_screen_saver_display_info_t));
        EXCEPTION_CHECK_VOID(saver_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_screen_saver_root_init();
    ble_ui_init(thiz);
    saver_info->force_update = true;
    home_screen_saver_icon_prepare(thiz);
    home_screen_saver_hint_update();
    saver_info->force_update = false;
}

void home_screen_saver_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && saver_info);
    ui_img_png_release_decoded_buf(saver_info->saver_icon);
#if HOME_SAVER_HINT_SHINE_USE_IMG_MASK
    home_screen_saver_hint_mask_a8_release();
#endif
    ble_ui_deinit(thiz);
    LVGL_OBJ_DELETE(saver_info->root);
    u_free(saver_info);
    saver_info = NULL;
}

int home_screen_saver_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && saver_info && saver_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble);
    home_screen_saver_root_bg_update(thiz);
    ble_ui_update(thiz, saver_info->root, proc->ble,
        BASE_PUMP_HOME_BLE_UI_X, BASE_PUMP_HOME_BLE_UI_Y, NULL);
    home_screen_saver_ui_update(thiz, proc);
    home_screen_saver_mask_anim_update();
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home screen saver fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
