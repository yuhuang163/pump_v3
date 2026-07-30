/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "pump_level_adjust_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_LEVEL_ADJUST_COMP_BG        (0) /// 是否启用对比背景图
#define IS_SUPPORT_PUMP_LEVEL_TIME_3SEG_TEST        (0)

/** 圆弧参数与 HOME 单边力度调节一致 */
#define PUMP_LEVEL_ARC_SIZE                         (352)
#define PUMP_LEVEL_ARC_WIDTH                        (11)
#if IS_SUPPORT_PUMP_LEVEL_ADJUST_COMP_BG
#define PUMP_LEVEL_ARC_REMAIN_COLOR                 (0xFFFFFF)
#else
#define PUMP_LEVEL_ARC_REMAIN_COLOR                 (0x252525)
#endif
#define PUMP_LEVEL_ARC_BOTTOM_GAP_DEG               (100)
#define PUMP_LEVEL_ARC_SEG_GAP_DEG                  (12)
#define PUMP_LEVEL_ARC_TRACK_DEG \
    (360 - PUMP_LEVEL_ARC_BOTTOM_GAP_DEG)
#define PUMP_LEVEL_ARC_ROTATION \
    (90 + (PUMP_LEVEL_ARC_BOTTOM_GAP_DEG / 2))

#define PUMP_LEVEL_VALUE_Y                          (127)
#define PUMP_LEVEL_VALUE_COLOR                      (0xFFFFFF)

#define PUMP_LEVEL_DIR_Y                            (210)
#define PUMP_LEVEL_DIR_COLOR                        (0x999999)
#define PUMP_LEVEL_DIR_LEFT_TEXT                    "Left Level"
#define PUMP_LEVEL_DIR_RIGHT_TEXT                   "Right Level"
#define PUMP_LEVEL_OVER_LIMIT_TEXT                  "Max 3 Levels at a time"

/** 泵奶时间组件参数与 pump_home_base_mode_page_ui 一致 */
#define PUMP_LEVEL_TIME_DIGIT_COUNT                 (2)
#define PUMP_LEVEL_TIME_COLON_COUNT                 (2)
#define PUMP_LEVEL_TIME_LABEL_Y                     (293)
#define PUMP_LEVEL_TIME_ICON_Y                      (311)
#define PUMP_LEVEL_TIME_Y                           (308)
#define PUMP_LEVEL_TIME_SEG_WIDTH                   (36)
#define PUMP_LEVEL_TIME_COLON_GAP                   (2)
#define PUMP_LEVEL_TIME_COLOR                       (0xFFFFFF)
#define PUMP_LEVEL_TIME_2SEG                        (2)
#define PUMP_LEVEL_TIME_3SEG                        (3)
#define PUMP_LEVEL_TIME_2SEG_ICON_X                 (133)
#define PUMP_LEVEL_TIME_2SEG_M_X                    (168)
#define PUMP_LEVEL_TIME_COLON_W_FALLBACK            (8)

#define PUMP_LEVEL_ARC_ANIM_MS                      (220)

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_PUMP_LEVEL_ADJUST_COMP_BG
    lv_obj_t *background;
#endif
    ui_manager_context_t *ui;
    lv_obj_t *done_arc;
    lv_obj_t *remain_arc;
    lv_obj_t *value_label;
    lv_obj_t *dir_label;
    lv_obj_t *time_label;
    lv_obj_t *status_icon;
    lv_obj_t *time_h[PUMP_LEVEL_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[PUMP_LEVEL_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_LEVEL_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[PUMP_LEVEL_TIME_COLON_COUNT];
#if IS_SUPPORT_PUMP_LEVEL_TIME_3SEG_TEST
    uint8_t test_hours;
#endif
    uint8_t shown_level;
    uint8_t shown_side;
    uint8_t shown_over_limit;
    uint8_t anim_to_level;
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;
    uint8_t shown_paused;
    uint8_t force_update;
    uint8_t pump_mode; /// 进入页时的模式（页内不切换）
    uint8_t level_max; /// 当前模式力度上限
    uint16_t shown_status_icon_id;
    uint32_t accent_color; /// 圆弧/过大提示强调色
    uint32_t shown_accent_color;
    uint32_t shown_dir_color;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint16_t anim_visual_sweep;
    uint32_t anim_start_time;
} pump_level_adjust_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_LEVEL_ADJUST_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t pump_level_adjust_bg_data[] = {
    #include "comparison_png2txt/left_level_adjust12_352x352.txt"
};
#endif

static pump_level_adjust_ui_info_t *level_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_level_adjust_root_init(void)
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

#if IS_SUPPORT_PUMP_LEVEL_ADJUST_COMP_BG
static void pump_level_adjust_bg_update(ui_manager_context_t *thiz)
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
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER,
            pump_level_adjust_bg_data, sizeof(pump_level_adjust_bg_data), "png");
        lv_image_set_src(level_info->background, (const void *)&bg_img_path);
        lv_obj_align(level_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static uint16_t pump_level_arc_sweep_from_level(uint8_t level, uint8_t level_max)
{
    uint16_t split_track;

    if (level_max == 0 || level == 0) {
        return 0;
    }
    if (level >= level_max) {
        return PUMP_LEVEL_ARC_TRACK_DEG;
    }
    split_track = PUMP_LEVEL_ARC_TRACK_DEG - PUMP_LEVEL_ARC_SEG_GAP_DEG;
    return (uint16_t)((split_track * level) / level_max);
}

static uint16_t pump_level_arc_visual_sweep_get(void)
{
    uint32_t elapsed;
    uint16_t from = level_info->anim_from_sweep;
    uint16_t to = level_info->anim_to_sweep;

    if (from == to) {
        return to;
    }

    elapsed = get_timestamp_ms(level_info->anim_start_time);
    if (elapsed >= PUMP_LEVEL_ARC_ANIM_MS) {
        level_info->anim_from_sweep = to;
        return to;
    }

    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) / PUMP_LEVEL_ARC_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) / PUMP_LEVEL_ARC_ANIM_MS);
}

static void pump_level_arc_anim_sync(uint8_t target_level, uint8_t level_max, bool snap)
{
    uint16_t target_sweep = pump_level_arc_sweep_from_level(target_level, level_max);

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

    level_info->anim_from_sweep = pump_level_arc_visual_sweep_get();
    level_info->anim_to_sweep = target_sweep;
    level_info->anim_to_level = target_level;
    level_info->anim_start_time = get_timestamp();
}

static void pump_level_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, PUMP_LEVEL_ARC_SIZE, PUMP_LEVEL_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, PUMP_LEVEL_ARC_ROTATION);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, PUMP_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static void pump_level_arc_update(ui_proc_pump_t *proc, uint8_t level_max, uint32_t accent_color)
{
    uint16_t done_sweep;
    uint16_t remain_sweep;
    uint16_t remain_start;
    bool side_changed;
    bool color_changed;

    if (level_info->done_arc == NULL || !lv_obj_is_valid(level_info->done_arc)) {
        level_info->done_arc = lv_arc_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->done_arc);
        pump_level_arc_style_init(level_info->done_arc, accent_color);
        level_info->shown_accent_color = accent_color;
    }
    if (level_info->remain_arc == NULL || !lv_obj_is_valid(level_info->remain_arc)) {
        level_info->remain_arc = lv_arc_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->remain_arc);
        pump_level_arc_style_init(level_info->remain_arc, PUMP_LEVEL_ARC_REMAIN_COLOR);
    }

    color_changed = (level_info->shown_accent_color != accent_color);
    if (color_changed || level_info->force_update) {
        lv_obj_set_style_arc_color(level_info->done_arc, lv_color_hex(accent_color), LV_PART_INDICATOR);
        level_info->shown_accent_color = accent_color;
    }

    side_changed = (level_info->shown_side != 0xFF &&
        level_info->shown_side != proc->level_adjust_side);
    pump_level_arc_anim_sync(proc->level_arc_level, level_max, side_changed);
    done_sweep = pump_level_arc_visual_sweep_get();
    if (level_info->force_update == false &&
        color_changed == false &&
        level_info->anim_visual_sweep == done_sweep) {
        return;
    }
    level_info->anim_visual_sweep = done_sweep;

    if (done_sweep >= PUMP_LEVEL_ARC_TRACK_DEG) {
        done_sweep = PUMP_LEVEL_ARC_TRACK_DEG;
        remain_sweep = 0;
    } else if (done_sweep == 0) {
        remain_sweep = PUMP_LEVEL_ARC_TRACK_DEG;
    } else {
        uint16_t split_track = PUMP_LEVEL_ARC_TRACK_DEG - PUMP_LEVEL_ARC_SEG_GAP_DEG;
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
        if (done_sweep > 0) {
            remain_start = done_sweep + PUMP_LEVEL_ARC_SEG_GAP_DEG;
        } else {
            remain_start = 0;
            remain_sweep = PUMP_LEVEL_ARC_TRACK_DEG;
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

static void pump_level_value_update(ui_proc_pump_t *proc)
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
        lv_color_hex(PUMP_LEVEL_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(level_info->value_label, &lv_font_aeonik_soft_69_semibold, 0);
    lv_obj_set_style_text_align(level_info->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(level_info->value_label, LV_ALIGN_TOP_MID, 0, PUMP_LEVEL_VALUE_Y);
    level_info->shown_level = proc->level_value;
}

static void pump_level_dir_update(ui_proc_pump_t *proc, uint32_t accent_color)
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
            (proc->level_over_limit ? accent_color : PUMP_LEVEL_DIR_COLOR)) {
        return;
    }

    if (proc->level_over_limit) {
        text = PUMP_LEVEL_OVER_LIMIT_TEXT;
        color = accent_color;
    } else if (proc->level_adjust_side == PUMP_LEVEL_ADJUST_SIDE_RIGHT) {
        text = PUMP_LEVEL_DIR_RIGHT_TEXT;
        color = PUMP_LEVEL_DIR_COLOR;
    } else {
        text = PUMP_LEVEL_DIR_LEFT_TEXT;
        color = PUMP_LEVEL_DIR_COLOR;
    }

    lv_label_set_text(level_info->dir_label, text);
    lv_obj_set_style_text_color(level_info->dir_label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(level_info->dir_label, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(level_info->dir_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(level_info->dir_label, LV_ALIGN_TOP_MID, 0, PUMP_LEVEL_DIR_Y);
    level_info->shown_side = proc->level_adjust_side;
    level_info->shown_over_limit = proc->level_over_limit;
    level_info->shown_dir_color = color;
}

static void pump_level_obj_flag_set(lv_obj_t *obj, bool visible)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    if (visible) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void pump_level_time_pair_update(lv_obj_t *labels[PUMP_LEVEL_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x, lv_coord_t seg_y, bool hidden)
{
    const lv_coord_t cell_w = PUMP_LEVEL_TIME_SEG_WIDTH / PUMP_LEVEL_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < PUMP_LEVEL_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(PUMP_LEVEL_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
        pump_level_obj_flag_set(labels[i], !hidden);
    }
}

static lv_coord_t pump_level_time_colon_w_get(void)
{
    lv_coord_t colon_w = PUMP_LEVEL_TIME_COLON_W_FALLBACK;
    lv_obj_t *colon = level_info->time_colon[1];

    if (colon == NULL || !lv_obj_is_valid(colon)) {
        colon = level_info->time_colon[0];
    }
    if (colon != NULL && lv_obj_is_valid(colon)) {
        lv_obj_update_layout(colon);
        colon_w = lv_obj_get_width(colon);
        if (colon_w <= 0) {
            colon_w = PUMP_LEVEL_TIME_COLON_W_FALLBACK;
        }
    }
    return colon_w;
}

static lv_coord_t pump_level_time_unit_span_get(void)
{
    return PUMP_LEVEL_TIME_SEG_WIDTH + PUMP_LEVEL_TIME_COLON_GAP +
        pump_level_time_colon_w_get() + PUMP_LEVEL_TIME_COLON_GAP;
}

static void pump_level_time_colon_ensure(void)
{
    int i;

    for (i = 0; i < PUMP_LEVEL_TIME_COLON_COUNT; i++) {
        if (level_info->time_colon[i] == NULL || !lv_obj_is_valid(level_info->time_colon[i])) {
            level_info->time_colon[i] = lv_label_create(level_info->root);
            EXCEPTION_CHECK_VOID(level_info->time_colon[i]);
            lv_obj_set_style_text_color(level_info->time_colon[i],
                lv_color_hex(PUMP_LEVEL_TIME_COLOR), 0);
            lv_obj_set_style_text_font(level_info->time_colon[i], &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_align(level_info->time_colon[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_text(level_info->time_colon[i], ":");
        }
    }
}

static void pump_level_time_layout_apply(uint8_t hours, uint8_t minutes, uint8_t seconds,
    uint8_t digit_size, bool force)
{
    const lv_coord_t unit_span = pump_level_time_unit_span_get();
    const lv_coord_t colon_w = pump_level_time_colon_w_get();
    const lv_coord_t m_x = PUMP_LEVEL_TIME_2SEG_M_X;
    const lv_coord_t colon_ms_x = m_x + PUMP_LEVEL_TIME_SEG_WIDTH + PUMP_LEVEL_TIME_COLON_GAP;
    const lv_coord_t s_x = colon_ms_x + colon_w + PUMP_LEVEL_TIME_COLON_GAP;
    lv_coord_t icon_x = PUMP_LEVEL_TIME_2SEG_ICON_X;
    lv_coord_t h_x = m_x;
    lv_coord_t colon_hm_x = colon_ms_x;
    const bool is_3seg = (digit_size == PUMP_LEVEL_TIME_3SEG);
    int i;

    if (is_3seg) {
        h_x = m_x - unit_span;
        colon_hm_x = h_x + PUMP_LEVEL_TIME_SEG_WIDTH + PUMP_LEVEL_TIME_COLON_GAP;
        icon_x = PUMP_LEVEL_TIME_2SEG_ICON_X - unit_span;
    }

    if (level_info->status_icon != NULL && lv_obj_is_valid(level_info->status_icon)) {
        lv_obj_align(level_info->status_icon, LV_ALIGN_TOP_LEFT, icon_x, PUMP_LEVEL_TIME_ICON_Y);
    }

    if (force || level_info->shown_hours != hours || level_info->shown_digit_size != digit_size) {
        pump_level_time_pair_update(level_info->time_h, level_info->root, hours, h_x,
            PUMP_LEVEL_TIME_Y, !is_3seg);
        level_info->shown_hours = hours;
    }
    if (force || level_info->shown_minutes != minutes || level_info->shown_digit_size != digit_size) {
        pump_level_time_pair_update(level_info->time_m, level_info->root, minutes, m_x,
            PUMP_LEVEL_TIME_Y, false);
        level_info->shown_minutes = minutes;
    }
    if (force || level_info->shown_seconds != seconds || level_info->shown_digit_size != digit_size) {
        pump_level_time_pair_update(level_info->time_s, level_info->root, seconds, s_x,
            PUMP_LEVEL_TIME_Y, false);
        level_info->shown_seconds = seconds;
    }

    if (level_info->time_colon[0] != NULL && lv_obj_is_valid(level_info->time_colon[0])) {
        lv_obj_align(level_info->time_colon[0], LV_ALIGN_TOP_LEFT, colon_hm_x, PUMP_LEVEL_TIME_Y);
        pump_level_obj_flag_set(level_info->time_colon[0], is_3seg);
    }
    if (level_info->time_colon[1] != NULL && lv_obj_is_valid(level_info->time_colon[1])) {
        lv_obj_align(level_info->time_colon[1], LV_ALIGN_TOP_LEFT, colon_ms_x, PUMP_LEVEL_TIME_Y);
        pump_level_obj_flag_set(level_info->time_colon[1], true);
    }
    level_info->shown_digit_size = digit_size;

    if (level_info->time_label != NULL && lv_obj_is_valid(level_info->time_label)) {
        lv_obj_move_foreground(level_info->time_label);
    }
    if (level_info->status_icon != NULL && lv_obj_is_valid(level_info->status_icon)) {
        lv_obj_move_foreground(level_info->status_icon);
    }
    for (i = 0; i < PUMP_LEVEL_TIME_COLON_COUNT; i++) {
        if (level_info->time_colon[i] != NULL && lv_obj_is_valid(level_info->time_colon[i])) {
            lv_obj_move_foreground(level_info->time_colon[i]);
        }
    }
    for (i = 0; i < PUMP_LEVEL_TIME_DIGIT_COUNT; i++) {
        if (level_info->time_h[i] != NULL && lv_obj_is_valid(level_info->time_h[i])) {
            lv_obj_move_foreground(level_info->time_h[i]);
        }
        if (level_info->time_m[i] != NULL && lv_obj_is_valid(level_info->time_m[i])) {
            lv_obj_move_foreground(level_info->time_m[i]);
        }
        if (level_info->time_s[i] != NULL && lv_obj_is_valid(level_info->time_s[i])) {
            lv_obj_move_foreground(level_info->time_s[i]);
        }
    }
}

static void pump_level_time_rebuild(void)
{
    EXCEPTION_CHECK_VOID(level_info->root && level_info->ui);

    if (level_info->time_label == NULL || !lv_obj_is_valid(level_info->time_label)) {
        level_info->time_label = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->time_label);
        lv_obj_clear_flag(level_info->time_label, LV_OBJ_FLAG_CLICKABLE);
        ui_img_baj_theme_set_src(level_info->ui, level_info->time_label,
            BASE_RES_ID_PUMP_TIME_LABEL, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(level_info->time_label, LV_ALIGN_TOP_MID, 0, PUMP_LEVEL_TIME_LABEL_Y);
    }

    if (level_info->status_icon == NULL || !lv_obj_is_valid(level_info->status_icon)) {
        level_info->status_icon = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->status_icon);
        lv_obj_clear_flag(level_info->status_icon, LV_OBJ_FLAG_CLICKABLE);
        level_info->shown_status_icon_id = 0;
        level_info->shown_paused = 0xFF;
    }

    pump_level_time_colon_ensure();
    level_info->shown_hours = 0xFF;
    level_info->shown_minutes = 0xFF;
    level_info->shown_seconds = 0xFF;
    level_info->shown_digit_size = 0;
    pump_level_time_layout_apply(0, 0, 0, PUMP_LEVEL_TIME_2SEG, true);
}

static void pump_level_time_content_update(ui_proc_pump_t *proc)
{
    const pump_result_info_t *result_info = NULL;
    uint16_t hold_time;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t digit_size;
    bool paused;
    uint16_t icon_id;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    EXCEPTION_CHECK_VOID(result_info);
    hold_time = result_info->work_valid_time / 1000;
#if IS_SUPPORT_PUMP_LEVEL_TIME_3SEG_TEST
    hours = level_info->test_hours;
    minutes = level_info->test_hours;
    seconds = level_info->test_hours;
#else
    hours = hold_time / 3600;
    minutes = (hold_time / 60) % 60;
    seconds = hold_time % 60;
#endif
    digit_size = (hours == 0) ? PUMP_LEVEL_TIME_2SEG : PUMP_LEVEL_TIME_3SEG;
    paused = (proc->pump_status == BREAST_PUMP_PAUSE);
    icon_id = paused ? BASE_RES_ID_PLAY_MIN_ICON : BASE_RES_ID_PAUSE_MIN_ICON;

    if (level_info->status_icon != NULL && lv_obj_is_valid(level_info->status_icon) &&
        (level_info->shown_status_icon_id != icon_id || level_info->force_update ||
         level_info->shown_paused != (paused ? 1 : 0))) {
        ui_img_baj_theme_set_src(level_info->ui, level_info->status_icon, icon_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        level_info->shown_status_icon_id = icon_id;
        level_info->shown_paused = paused ? 1 : 0;
    }

    pump_level_time_layout_apply(hours, minutes, seconds, digit_size, level_info->force_update != 0);
}

static void pump_level_time_update(ui_proc_pump_t *proc)
{
    if (level_info->force_update) {
        pump_level_time_rebuild();
    }
    pump_level_time_content_update(proc);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
uint8_t pump_level_adjust_ui_level_max_get(void)
{
    if (level_info == NULL) {
        return BREAST_PUMP_LEVEL_15;
    }
    return level_info->level_max;
}

void ui_display_pump_level_adjust_prepare(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (pump_level_adjust_ui_info_t *)u_malloz(sizeof(pump_level_adjust_ui_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    /* 页内不切换模式：进入时用 int 读一次并缓存在 UI（READ_PUMP_WORK_MODE 要求 sizeof(int)） */
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    level_info->pump_mode = (uint8_t)pump_mode;
    level_info->level_max = home_level_adjust_max_get(level_info->pump_mode);
    level_info->accent_color = home_level_adjust_accent_color_get(level_info->pump_mode);
    LOG_UI("pump level adjust prepare mode=%d max=%d color=0x%06X",
        level_info->pump_mode, level_info->level_max, level_info->accent_color);

    level_info->ui = thiz;
    level_info->force_update = true;
    level_info->shown_level = 0xFF;
    level_info->shown_side = 0xFF;
    level_info->shown_over_limit = 0xFF;
    level_info->shown_accent_color = 0;
    level_info->shown_dir_color = 0;
    level_info->shown_hours = 0xFF;
    level_info->shown_minutes = 0xFF;
    level_info->shown_seconds = 0xFF;
    level_info->shown_digit_size = 0;
    level_info->shown_paused = 0xFF;
    level_info->shown_status_icon_id = 0;
    pump_level_adjust_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_level_adjust_remove(ui_manager_context_t *thiz)
{
    int i;

    EXCEPTION_CHECK_VOID(thiz && level_info && level_info->root);
    ui_img_baj_release_decoded_buf(level_info->time_label);
    LVGL_OBJ_DELETE(level_info->time_label);
    level_info->time_label = NULL;
    ui_img_baj_release_decoded_buf(level_info->status_icon);
    LVGL_OBJ_DELETE(level_info->status_icon);
    level_info->status_icon = NULL;
    for (i = 0; i < PUMP_LEVEL_TIME_COLON_COUNT; i++) {
        LVGL_OBJ_DELETE(level_info->time_colon[i]);
        level_info->time_colon[i] = NULL;
    }
    for (i = 0; i < PUMP_LEVEL_TIME_DIGIT_COUNT; i++) {
        LVGL_OBJ_DELETE(level_info->time_h[i]);
        level_info->time_h[i] = NULL;
        LVGL_OBJ_DELETE(level_info->time_m[i]);
        level_info->time_m[i] = NULL;
        LVGL_OBJ_DELETE(level_info->time_s[i]);
        level_info->time_s[i] = NULL;
    }
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int ui_display_pump_level_adjust_process(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_INT(thiz && level_info && level_info->root);
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump);

    pump_level_arc_update(proc, level_info->level_max, level_info->accent_color);
    pump_level_value_update(proc);
    pump_level_dir_update(proc, level_info->accent_color);
    pump_level_time_update(proc);
    pump_level_adjust_bg_update(thiz);
    level_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump level adjust fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
#if IS_SUPPORT_PUMP_LEVEL_TIME_3SEG_TEST
        level_info->test_hours += 1;
        if (level_info->test_hours >= 59) {
            level_info->test_hours = 0;
        }
#endif
    }
    return 0;
}
