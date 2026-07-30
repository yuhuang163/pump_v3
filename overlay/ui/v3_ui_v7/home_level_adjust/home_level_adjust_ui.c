/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG        (0)

/** 圆弧参数与泵奶停止长按页一致 */
#define HOME_LEVEL_ARC_SIZE                         (352)
#define HOME_LEVEL_ARC_WIDTH                        (11)
#define HOME_LEVEL_ARC_REMAIN_COLOR                 (0x252525)
/** 刺激模式力度颜色 */
#define HOME_STIMULATION_LEVEL_COLOR                (0xE2637E)
/** 吸乳模式力度颜色 */
#define HOME_LACTATION_LEVEL_COLOR                  (0x64A4F2)
/** 韵律模式力度颜色 */
#define HOME_RHYTHM_LEVEL_COLOR                     (0x9C78E8)
/** 底部开口角度，使圆环不闭合（越大两端越高） */
#define HOME_LEVEL_ARC_BOTTOM_GAP_DEG               (100)
/** 已展示 / 剩余两段圆弧之间的角度偏差 */
#define HOME_LEVEL_ARC_SEG_GAP_DEG                  (12)
/** 除去底部开口后的可绘制总角度 */
#define HOME_LEVEL_ARC_TRACK_DEG \
    (360 - HOME_LEVEL_ARC_BOTTOM_GAP_DEG)
/** 开口居中于底部：弧起点落在 gap 右侧 */
#define HOME_LEVEL_ARC_ROTATION \
    (90 + (HOME_LEVEL_ARC_BOTTOM_GAP_DEG / 2))

#define HOME_LEVEL_VALUE_Y                          (127)
#define HOME_LEVEL_VALUE_COLOR                      (0xFFFFFF)

#define HOME_LEVEL_DIR_Y                            (210)
#define HOME_LEVEL_DIR_COLOR                        (0x999999)
#define HOME_LEVEL_DIR_LEFT_TEXT                    "Left Level"
#define HOME_LEVEL_DIR_RIGHT_TEXT                   "Right Level"
#define HOME_LEVEL_OVER_LIMIT_TEXT                  "Max 3 Levels at a time"

#define HOME_LEVEL_TIP_W                            (216)
#define HOME_LEVEL_TIP_H                            (51)
/** 底部提示图 Y：贴底留边（与 home 模式选择页一致） */
#define HOME_LEVEL_TIP_Y                            (PANLE_DISPLAY_H - HOME_LEVEL_TIP_H - 8)

/** 提示扫光方式（三选一，与 home 模式页取值一致） */
#define HOME_LEVEL_TIP_SHINE_HARD                   (0)
#define HOME_LEVEL_TIP_SHINE_SOFT                   (1)
#define HOME_LEVEL_TIP_SHINE_OVERLAY                (2)
#ifndef HOME_LEVEL_TIP_SHINE_MODE
#define HOME_LEVEL_TIP_SHINE_MODE                   HOME_LEVEL_TIP_SHINE_OVERLAY
#endif
#define HOME_LEVEL_TIP_SHINE_IS_HARD \
    (HOME_LEVEL_TIP_SHINE_MODE == HOME_LEVEL_TIP_SHINE_HARD)
#define HOME_LEVEL_TIP_SHINE_IS_SOFT \
    (HOME_LEVEL_TIP_SHINE_MODE == HOME_LEVEL_TIP_SHINE_SOFT)
#define HOME_LEVEL_TIP_SHINE_IS_OVERLAY \
    (HOME_LEVEL_TIP_SHINE_MODE == HOME_LEVEL_TIP_SHINE_OVERLAY)
#define HOME_LEVEL_TIP_SHINE_USE_IMG_MASK \
    (HOME_LEVEL_TIP_SHINE_IS_SOFT || HOME_LEVEL_TIP_SHINE_IS_OVERLAY)

#ifndef HOME_LEVEL_TIP_SHINE_W
#if HOME_LEVEL_TIP_SHINE_IS_HARD
#define HOME_LEVEL_TIP_SHINE_W                      (80)
#else
#define HOME_LEVEL_TIP_SHINE_W                      (80)
#endif
#endif

#if HOME_LEVEL_TIP_SHINE_IS_HARD
#define HOME_LEVEL_TIP_MASK_W                       (HOME_LEVEL_TIP_SHINE_W)
#define HOME_LEVEL_TIP_MASK_H                       (HOME_LEVEL_TIP_H)
#define HOME_LEVEL_TIP_MASK_Y                       (HOME_LEVEL_TIP_Y)
#define HOME_LEVEL_TIP_SHINE_RECOLOR_OPA            (LV_OPA_COVER)
#define HOME_LEVEL_TIP_BASE_OPA                     (LV_OPA_COVER)
#else
#define HOME_LEVEL_TIP_MASK_W                       (HOME_LEVEL_TIP_SHINE_W)
#define HOME_LEVEL_TIP_MASK_H                       (HOME_LEVEL_TIP_H)
#define HOME_LEVEL_TIP_MASK_Y                       (HOME_LEVEL_TIP_Y)
#define HOME_LEVEL_TIP_SHINE_RECOLOR_OPA            (LV_OPA_80)
#define HOME_LEVEL_TIP_BASE_OPA                     (LV_OPA_70)
#endif
#define HOME_LEVEL_TIP_MASK_X_MIN                   (-HOME_LEVEL_TIP_MASK_W)
#define HOME_LEVEL_TIP_MASK_X_MAX                   (PANLE_DISPLAY_W)
#define HOME_LEVEL_TIP_MASK_PERIOD_MS               (3200)
#define HOME_LEVEL_TIP_SHINE_COLOR                  (0xFFFFFF)

/** 相邻档位圆弧过渡时长 */
#define HOME_LEVEL_ARC_ANIM_MS                      (220)

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG
    lv_obj_t *background;
#endif
    lv_obj_t *done_arc;
    lv_obj_t *remain_arc;
    lv_obj_t *value_label;
    lv_obj_t *dir_label;
    lv_obj_t *tip_img;
    lv_obj_t *tip_shine;
    lv_obj_t *tip_mask;
#if HOME_LEVEL_TIP_SHINE_USE_IMG_MASK
    lv_draw_buf_t *tip_mask_a8;
#endif
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    int16_t tip_mask_x;
    uint32_t tip_mask_tick_ms;
    uint8_t shown_level;
    uint8_t shown_side;
    uint8_t shown_over_limit;
    uint8_t anim_to_level;
    uint8_t pump_mode; /// 进入页时的模式（页内不切换）
    uint8_t level_max; /// 当前模式力度上限
    uint8_t force_update;
    uint8_t rsv;
    uint32_t accent_color; /// 圆弧/过大提示强调色
    uint32_t shown_accent_color;
    uint32_t shown_dir_color;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint16_t anim_visual_sweep;
    uint32_t anim_start_time;
} home_level_adjust_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_level_adjust_bg_data[] = {
    #include "comparison_png2txt/left_level_adjust11_360x360.txt"
};
#endif

static home_level_adjust_ui_info_t *level_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_level_adjust_root_init(void)
{
    if (level_info->root == NULL || !lv_obj_is_valid(level_info->root)) {
        level_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(level_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(level_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(level_info->root, 0, 0);
    lv_obj_set_style_pad_all(level_info->root, 0, 0);
    lv_obj_align(level_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG
static void home_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (level_info->background == NULL || !lv_obj_is_valid(level_info->background)) {
        level_info->background = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->background);
        lv_obj_move_background(level_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_level_adjust_bg_data,
            sizeof(home_level_adjust_bg_data), "png");
        lv_image_set_src(level_info->background, (const void *)&bg_img_path);
        lv_obj_align(level_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static uint16_t home_level_arc_sweep_from_level(uint8_t level, uint8_t level_max)
{
    uint16_t split_track;

    if (level_max == 0 || level == 0) {
        return 0;
    }
    if (level >= level_max) {
        return HOME_LEVEL_ARC_TRACK_DEG;
    }
    split_track = HOME_LEVEL_ARC_TRACK_DEG - HOME_LEVEL_ARC_SEG_GAP_DEG;
    return (uint16_t)((split_track * level) / level_max);
}

static uint16_t home_level_arc_visual_sweep_get(void)
{
    uint32_t elapsed;
    uint16_t from = level_info->anim_from_sweep;
    uint16_t to = level_info->anim_to_sweep;

    if (from == to) {
        return to;
    }

    elapsed = get_timestamp_ms(level_info->anim_start_time);
    if (elapsed >= HOME_LEVEL_ARC_ANIM_MS) {
        level_info->anim_from_sweep = to;
        return to;
    }

    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) / HOME_LEVEL_ARC_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) / HOME_LEVEL_ARC_ANIM_MS);
}

static void home_level_arc_anim_sync(uint8_t target_level, uint8_t level_max, bool snap)
{
    uint16_t target_sweep = home_level_arc_sweep_from_level(target_level, level_max);

    if (level_info->force_update || snap) {
        level_info->anim_from_sweep = target_sweep;
        level_info->anim_to_sweep = target_sweep;
        level_info->anim_visual_sweep = target_sweep;
        level_info->anim_to_level = target_level;
        level_info->anim_start_time = get_timestamp();
        return;
    }

    if (level_info->anim_to_level == target_level &&
        level_info->anim_to_sweep == target_sweep) {
        return;
    }

    level_info->anim_from_sweep = home_level_arc_visual_sweep_get();
    level_info->anim_to_sweep = target_sweep;
    level_info->anim_to_level = target_level;
    level_info->anim_start_time = get_timestamp();
}

static void home_level_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, HOME_LEVEL_ARC_SIZE, HOME_LEVEL_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, HOME_LEVEL_ARC_ROTATION);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, HOME_LEVEL_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, HOME_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static void home_level_arc_update(ui_proc_home_t *proc, uint8_t level_max, uint32_t accent_color)
{
    uint16_t done_sweep;
    uint16_t remain_sweep;
    uint16_t remain_start;
    bool side_changed;
    bool color_changed;

    if (level_info->done_arc == NULL || !lv_obj_is_valid(level_info->done_arc)) {
        level_info->done_arc = lv_arc_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->done_arc);
        home_level_arc_style_init(level_info->done_arc, accent_color);
        level_info->shown_accent_color = accent_color;
    }
    if (level_info->remain_arc == NULL || !lv_obj_is_valid(level_info->remain_arc)) {
        level_info->remain_arc = lv_arc_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->remain_arc);
        home_level_arc_style_init(level_info->remain_arc, HOME_LEVEL_ARC_REMAIN_COLOR);
    }

    color_changed = (level_info->shown_accent_color != accent_color);
    if (color_changed || level_info->force_update) {
        lv_obj_set_style_arc_color(level_info->done_arc, lv_color_hex(accent_color), LV_PART_INDICATOR);
        level_info->shown_accent_color = accent_color;
    }

    side_changed = (level_info->shown_side != 0xFF &&
        level_info->shown_side != proc->level_adjust_side);
    home_level_arc_anim_sync(proc->level_arc_level, level_max, side_changed);
    done_sweep = home_level_arc_visual_sweep_get();
    if (level_info->force_update == false &&
        color_changed == false &&
        level_info->anim_visual_sweep == done_sweep) {
        return;
    }
    level_info->anim_visual_sweep = done_sweep;

    if (done_sweep >= HOME_LEVEL_ARC_TRACK_DEG) {
        done_sweep = HOME_LEVEL_ARC_TRACK_DEG;
        remain_sweep = 0;
    } else if (done_sweep == 0) {
        remain_sweep = HOME_LEVEL_ARC_TRACK_DEG;
    } else {
        uint16_t split_track = HOME_LEVEL_ARC_TRACK_DEG - HOME_LEVEL_ARC_SEG_GAP_DEG;
        if (done_sweep > split_track) {
            done_sweep = split_track;
        }
        remain_sweep = split_track - done_sweep;
    }

    if (done_sweep > 0) {
        lv_arc_set_angles(level_info->done_arc, 0, done_sweep);
        lv_obj_clear_flag(level_info->done_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(level_info->done_arc, 0, 0);
        lv_obj_add_flag(level_info->done_arc, LV_OBJ_FLAG_HIDDEN);
    }

    if (remain_sweep > 0) {
        /* 两段同时存在时保持角度偏差；仅剩余时铺满 track */
        if (done_sweep > 0) {
            remain_start = done_sweep + HOME_LEVEL_ARC_SEG_GAP_DEG;
        } else {
            remain_start = 0;
            remain_sweep = HOME_LEVEL_ARC_TRACK_DEG;
        }
        lv_arc_set_angles(level_info->remain_arc, remain_start, remain_start + remain_sweep);
        lv_obj_clear_flag(level_info->remain_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(level_info->remain_arc, 0, 0);
        lv_obj_add_flag(level_info->remain_arc, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_invalidate(level_info->done_arc);
    lv_obj_invalidate(level_info->remain_arc);
}

static void home_level_value_update(ui_proc_home_t *proc)
{
    char num_buf[8] = {0};

    if (level_info->value_label == NULL || !lv_obj_is_valid(level_info->value_label)) {
        level_info->value_label = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->value_label);
        level_info->shown_level = 0xFF;
    }
    if (level_info->force_update == false && level_info->shown_level == proc->level_value) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), "%u", proc->level_value);
    lv_label_set_text(level_info->value_label, num_buf);
    lv_obj_set_style_text_color(level_info->value_label,
        lv_color_hex(HOME_LEVEL_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(level_info->value_label, &lv_font_aeonik_soft_69_semibold, 0);
    lv_obj_set_style_text_align(level_info->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(level_info->value_label, LV_ALIGN_TOP_MID, 0, HOME_LEVEL_VALUE_Y);
    level_info->shown_level = proc->level_value;
}

static void home_level_dir_update(ui_proc_home_t *proc, uint32_t accent_color)
{
    const char *text;
    uint32_t color;

    if (level_info->dir_label == NULL || !lv_obj_is_valid(level_info->dir_label)) {
        level_info->dir_label = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->dir_label);
        level_info->shown_side = 0xFF;
        level_info->shown_over_limit = 0xFF;
    }
    if (level_info->force_update == false &&
        level_info->shown_side == proc->level_adjust_side &&
        level_info->shown_over_limit == proc->level_over_limit &&
        level_info->shown_dir_color ==
            (proc->level_over_limit ? accent_color : HOME_LEVEL_DIR_COLOR)) {
        return;
    }

    if (proc->level_over_limit) {
        text = HOME_LEVEL_OVER_LIMIT_TEXT;
        color = accent_color;
    } else if (proc->level_adjust_side == HOME_LEVEL_ADJUST_SIDE_RIGHT) {
        text = HOME_LEVEL_DIR_RIGHT_TEXT;
        color = HOME_LEVEL_DIR_COLOR;
    } else {
        text = HOME_LEVEL_DIR_LEFT_TEXT;
        color = HOME_LEVEL_DIR_COLOR;
    }

    lv_label_set_text(level_info->dir_label, text);
    lv_obj_set_style_text_color(level_info->dir_label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(level_info->dir_label, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(level_info->dir_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(level_info->dir_label, LV_ALIGN_TOP_MID, 0, HOME_LEVEL_DIR_Y);
    level_info->shown_side = proc->level_adjust_side;
    level_info->shown_over_limit = proc->level_over_limit;
    level_info->shown_dir_color = color;
}

static void home_level_tip_shine_sync(void)
{
    lv_coord_t tip_x;
    lv_coord_t tip_y;

    if (level_info->tip_img == NULL || !lv_obj_is_valid(level_info->tip_img) ||
        level_info->tip_shine == NULL || !lv_obj_is_valid(level_info->tip_shine) ||
        level_info->tip_mask == NULL || !lv_obj_is_valid(level_info->tip_mask)) {
        return;
    }

    lv_obj_update_layout(level_info->tip_img);
    tip_x = lv_obj_get_x(level_info->tip_img);
    tip_y = lv_obj_get_y(level_info->tip_img);
    lv_obj_set_pos(level_info->tip_shine,
        tip_x - level_info->tip_mask_x, tip_y - HOME_LEVEL_TIP_MASK_Y);
}

#if HOME_LEVEL_TIP_SHINE_USE_IMG_MASK
/** 从预解码 TEXT_SHINE_MASK 生成 A8；高度不足时垂直居中裁切 */
static lv_draw_buf_t *home_level_tip_mask_a8_from_rgba(const lv_draw_buf_t *rgba,
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
#if HOME_LEVEL_TIP_SHINE_IS_SOFT
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

static void home_level_tip_mask_a8_release(void)
{
    if (level_info == NULL || level_info->tip_mask_a8 == NULL) {
        return;
    }
    if (level_info->tip_mask != NULL && lv_obj_is_valid(level_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(level_info->tip_mask, NULL, 0);
    }
    lv_draw_buf_destroy(level_info->tip_mask_a8);
    level_info->tip_mask_a8 = NULL;
}

static void home_level_tip_mask_a8_prepare(void)
{
    const lv_draw_buf_t *rgba;

    if (level_info->tip_mask_a8 != NULL) {
        if (level_info->tip_mask != NULL && lv_obj_is_valid(level_info->tip_mask)) {
            lv_obj_set_style_bitmap_mask_src(level_info->tip_mask, level_info->tip_mask_a8, 0);
        }
        return;
    }

    rgba = get_predecoded_text_shine_mask();
    level_info->tip_mask_a8 = home_level_tip_mask_a8_from_rgba(rgba,
        HOME_LEVEL_TIP_MASK_W, HOME_LEVEL_TIP_MASK_H);
    if (level_info->tip_mask_a8 != NULL && level_info->tip_mask != NULL &&
        lv_obj_is_valid(level_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(level_info->tip_mask, level_info->tip_mask_a8, 0);
    }
}
#endif /* HOME_LEVEL_TIP_SHINE_USE_IMG_MASK */

static lv_draw_buf_t *home_level_tip_predecode(void)
{
    const ui_img_data_t *tip;

    if (level_info->tip_buf != NULL) {
        return level_info->tip_buf;
    }
    tip = ui_font_img_level_adjust_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    level_info->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return level_info->tip_buf;
}

static void home_level_tip_rebuild(void)
{
    lv_draw_buf_t *predecoded;

    if (level_info->tip_img != NULL && lv_obj_is_valid(level_info->tip_img) &&
        level_info->tip_mask != NULL && lv_obj_is_valid(level_info->tip_mask) &&
        level_info->tip_shine != NULL && lv_obj_is_valid(level_info->tip_shine)) {
        home_level_tip_shine_sync();
        return;
    }

    predecoded = home_level_tip_predecode();

    if (level_info->tip_img == NULL || !lv_obj_is_valid(level_info->tip_img)) {
        level_info->tip_img = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->tip_img);
        lv_obj_clear_flag(level_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(level_info->tip_img, predecoded);
            level_info->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(level_info->tip_img, HOME_LEVEL_TIP_BASE_OPA, 0);
        lv_obj_align(level_info->tip_img, LV_ALIGN_TOP_MID, 0, HOME_LEVEL_TIP_Y);
    }

    if (level_info->tip_mask == NULL || !lv_obj_is_valid(level_info->tip_mask)) {
        level_info->tip_mask = lv_obj_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->tip_mask);
        lv_obj_remove_style_all(level_info->tip_mask);
        lv_obj_set_size(level_info->tip_mask, HOME_LEVEL_TIP_MASK_W, HOME_LEVEL_TIP_MASK_H);
        lv_obj_set_style_bg_opa(level_info->tip_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(level_info->tip_mask, 0, 0);
        lv_obj_set_style_pad_all(level_info->tip_mask, 0, 0);
        lv_obj_clear_flag(level_info->tip_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(level_info->tip_mask, LV_OBJ_FLAG_CLICKABLE);
        level_info->tip_mask_x = HOME_LEVEL_TIP_MASK_X_MIN;
        level_info->tip_mask_tick_ms = get_timestamp();
    }

    if (level_info->tip_shine == NULL || !lv_obj_is_valid(level_info->tip_shine)) {
        level_info->tip_shine = lv_image_create(level_info->tip_mask);
        EXCEPTION_CHECK_VOID(level_info->tip_shine);
        lv_obj_clear_flag(level_info->tip_shine, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(level_info->tip_shine, predecoded);
        }
        lv_obj_set_style_image_recolor(level_info->tip_shine,
            lv_color_hex(HOME_LEVEL_TIP_SHINE_COLOR), 0);
        lv_obj_set_style_image_recolor_opa(level_info->tip_shine,
            HOME_LEVEL_TIP_SHINE_RECOLOR_OPA, 0);
        lv_obj_set_style_blend_mode(level_info->tip_shine, LV_BLEND_MODE_NORMAL, 0);
    }

#if HOME_LEVEL_TIP_SHINE_USE_IMG_MASK
    home_level_tip_mask_a8_prepare();
#endif
    lv_obj_align(level_info->tip_mask, LV_ALIGN_TOP_LEFT,
        level_info->tip_mask_x, HOME_LEVEL_TIP_MASK_Y);
    lv_obj_move_foreground(level_info->tip_img);
    lv_obj_move_foreground(level_info->tip_mask);
    home_level_tip_shine_sync();
}

static void home_level_tip_mask_anim_update(void)
{
    uint32_t elapsed;
    int32_t x;

    if (level_info->tip_mask == NULL || !lv_obj_is_valid(level_info->tip_mask)) {
        return;
    }

    elapsed = get_timestamp() - level_info->tip_mask_tick_ms;
    if (elapsed >= HOME_LEVEL_TIP_MASK_PERIOD_MS) {
        level_info->tip_mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = HOME_LEVEL_TIP_MASK_X_MIN +
        (int32_t)((HOME_LEVEL_TIP_MASK_X_MAX - HOME_LEVEL_TIP_MASK_X_MIN) * elapsed) /
        HOME_LEVEL_TIP_MASK_PERIOD_MS;
    if (x != level_info->tip_mask_x) {
        level_info->tip_mask_x = (int16_t)x;
        lv_obj_set_x(level_info->tip_mask, level_info->tip_mask_x);
        home_level_tip_shine_sync();
    }
}

static void home_level_tip_update(void)
{
    if (level_info->force_update) {
        home_level_tip_rebuild();
    }
    home_level_tip_mask_anim_update();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
uint8_t home_level_adjust_max_get(uint8_t pump_mode)
{
    switch (pump_mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return BREAST_PUMP_LEVEL_5;
        case BREAST_PUMP_LACTATION_MODE:
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        default:
            return BREAST_PUMP_LEVEL_15;
    }
}

uint32_t home_level_adjust_accent_color_get(uint8_t pump_mode)
{
    switch (pump_mode) {
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return HOME_RHYTHM_LEVEL_COLOR;
        case BREAST_PUMP_STIMULATION_MODE:
            return HOME_STIMULATION_LEVEL_COLOR;
        case BREAST_PUMP_LACTATION_MODE:
            return HOME_LACTATION_LEVEL_COLOR;
        default:
            return HOME_STIMULATION_LEVEL_COLOR;
    }
}

uint8_t home_level_adjust_ui_level_max_get(void)
{
    if (level_info == NULL) {
        return BREAST_PUMP_LEVEL_15;
    }
    return level_info->level_max;
}

void ui_display_home_level_adjust_prepare(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (home_level_adjust_ui_info_t *)u_malloz(sizeof(home_level_adjust_ui_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    /* 页内不切换模式：进入时用 int 读一次并缓存在 UI（READ_PUMP_WORK_MODE 要求 sizeof(int)） */
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    level_info->pump_mode = (uint8_t)pump_mode;
    level_info->level_max = home_level_adjust_max_get(level_info->pump_mode);
    level_info->accent_color = home_level_adjust_accent_color_get(level_info->pump_mode);
    LOG_UI("level adjust prepare mode=%d max=%d color=0x%06X",
        level_info->pump_mode, level_info->level_max, level_info->accent_color);

    level_info->force_update = true;
    level_info->shown_level = 0xFF;
    level_info->shown_side = 0xFF;
    level_info->shown_over_limit = 0xFF;
    level_info->shown_accent_color = 0;
    level_info->shown_dir_color = 0;
    home_level_adjust_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_home_level_adjust_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && level_info && level_info->root);
    if (level_info->tip_shine != NULL && lv_obj_is_valid(level_info->tip_shine)) {
        lv_image_set_src(level_info->tip_shine, NULL);
        LVGL_OBJ_DELETE(level_info->tip_shine);
        level_info->tip_shine = NULL;
    }
#if HOME_LEVEL_TIP_SHINE_USE_IMG_MASK
    home_level_tip_mask_a8_release();
#endif
    if (level_info->tip_mask != NULL && lv_obj_is_valid(level_info->tip_mask)) {
        LVGL_OBJ_DELETE(level_info->tip_mask);
        level_info->tip_mask = NULL;
    }
    if (level_info->tip_img != NULL && lv_obj_is_valid(level_info->tip_img)) {
        lv_image_set_src(level_info->tip_img, NULL);
    }
    if (level_info->tip_buf != NULL) {
        lv_draw_buf_destroy(level_info->tip_buf);
        level_info->tip_buf = NULL;
    }
    level_info->tip_predecoded = 0;
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int ui_display_home_level_adjust_process(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;

    EXCEPTION_CHECK_INT(thiz && level_info && level_info->root);
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump);

    home_level_adjust_bg_update(thiz);
    home_level_arc_update(proc, level_info->level_max, level_info->accent_color);
    home_level_value_update(proc);
    home_level_dir_update(proc, level_info->accent_color);
    home_level_tip_update();
    level_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home level adjust fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
