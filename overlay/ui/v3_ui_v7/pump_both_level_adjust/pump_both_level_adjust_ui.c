/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "pump_both_level_adjust_ui.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_BOTH_LEVEL_ADJUST_COMP_BG   (0) /// 是否启用对比背景图
#define IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_3SEG_TEST   (0)
/** 进入双边力度调节页：时间胶囊淡入 + 上移；默认开启 */
#ifndef IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_ENTER_ANIM
#define IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_ENTER_ANIM  (1)
#endif

/** 圆弧：直径 352，线宽 11，当前/剩余间距 8px≈3° */
#define PUMP_BOTH_LEVEL_ARC_SIZE                    (352)
#define PUMP_BOTH_LEVEL_ARC_WIDTH                   (11)

#if IS_SUPPORT_PUMP_BOTH_LEVEL_ADJUST_COMP_BG
#define PUMP_BOTH_LEVEL_ARC_REMAIN_COLOR            (0xFFFFFF)
#else
#define PUMP_BOTH_LEVEL_ARC_REMAIN_COLOR            (0x252525)
#endif

/** 段间距8 */
#define PUMP_BOTH_LEVEL_ARC_SEG_GAP_DEG             (8)
/** 左弧：底部起沿左侧向上（参考图量测） */
#define PUMP_BOTH_LEVEL_LEFT_START                  (136)
#define PUMP_BOTH_LEVEL_LEFT_TRACK                  (122)
/** 右弧：与左弧左右镜像；顶→底顺时针，进度自底向上（from_bottom） */
#define PUMP_BOTH_LEVEL_RIGHT_END                   (540 - PUMP_BOTH_LEVEL_LEFT_START) /* 406 */
#define PUMP_BOTH_LEVEL_RIGHT_TRACK                 (PUMP_BOTH_LEVEL_LEFT_TRACK)
#define PUMP_BOTH_LEVEL_RIGHT_START                 (PUMP_BOTH_LEVEL_RIGHT_END - PUMP_BOTH_LEVEL_RIGHT_TRACK) /* 282 */

#if IS_SUPPORT_PUMP_BOTH_LEVEL_ADJUST_COMP_BG
#define PUMP_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR       (0x00FF00)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_RHYTHM_COLOR     (0x00FF00)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_LACTATION_COLOR  (0x00FF00)
#else
#define PUMP_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR       (0xE2637E)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_RHYTHM_COLOR     (0x9C78E8)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_LACTATION_COLOR  (0x64A4F2)
#endif

#define PUMP_BOTH_LEVEL_VALUE_W                     (150)
#define PUMP_BOTH_LEVEL_VALUE_H                     (69)
#define PUMP_BOTH_LEVEL_VALUE_Y                     (122)
#define PUMP_BOTH_LEVEL_VALUE_LEFT_X                (29)
#define PUMP_BOTH_LEVEL_VALUE_RIGHT_X               (179)
#define PUMP_BOTH_LEVEL_VALUE_COLOR                 (0xFFFFFF)

#define PUMP_BOTH_LEVEL_DIR_W                       (150)
#define PUMP_BOTH_LEVEL_DIR_H                       (34)
#define PUMP_BOTH_LEVEL_DIR_Y                       (203)
#define PUMP_BOTH_LEVEL_DIR_LEFT_X                  (29)
#define PUMP_BOTH_LEVEL_DIR_RIGHT_X                 (179)
#define PUMP_BOTH_LEVEL_DIR_COLOR                   (0x999999)
#define PUMP_BOTH_LEVEL_DIR_LEFT_TEXT               "L"
#define PUMP_BOTH_LEVEL_DIR_RIGHT_TEXT              "R"

#define PUMP_BOTH_LEVEL_OVER_LIMIT_W                (220)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_H                (50)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_Y                (60)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE       (8)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_TEXT \
    "For safety,\nUp to 3 levels at a time."
#define PUMP_BOTH_LEVEL_MODE_SWITCH_W              (PANLE_DISPLAY_W)
#define PUMP_BOTH_LEVEL_MODE_SWITCH_H              (34)
#define PUMP_BOTH_LEVEL_MODE_SWITCH_Y              (78)
#define PUMP_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT        "Switch to %s"

/** 泵奶时间组件参数与 pump_speed_adjust_ui 一致 */
#define PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT                 (2)
#define PUMP_BOTH_LEVEL_TIME_COLON_COUNT                 (2)
#define PUMP_BOTH_LEVEL_TIME_LABEL_Y                     (293)
#define PUMP_BOTH_LEVEL_TIME_ICON_Y                      (311)
#define PUMP_BOTH_LEVEL_TIME_Y                           (308)
#define PUMP_BOTH_LEVEL_TIME_SEG_WIDTH                   (36)
#define PUMP_BOTH_LEVEL_TIME_COLON_GAP                   (2)
#define PUMP_BOTH_LEVEL_TIME_COLOR                       (0xFFFFFF)
#define PUMP_BOTH_LEVEL_TIME_2SEG                        (2)
#define PUMP_BOTH_LEVEL_TIME_3SEG                        (3)
#define PUMP_BOTH_LEVEL_TIME_2SEG_ICON_X                 (133)
#define PUMP_BOTH_LEVEL_TIME_2SEG_M_X                    (168)
#define PUMP_BOTH_LEVEL_TIME_COLON_W_FALLBACK            (8)
#if IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_ENTER_ANIM
#define PUMP_BOTH_LEVEL_TIME_ENTER_ANIM_MS               (280)
#define PUMP_BOTH_LEVEL_TIME_ENTER_Y_OFFSET              (24)
#define PUMP_BOTH_LEVEL_TIME_ENTER_PROGRESS_FULL         (256)
#define PUMP_BOTH_LEVEL_TIME_SCALE_FULL                  (256)
#endif

#define PUMP_BOTH_LEVEL_ARC_ANIM_MS                 (220)

typedef struct {
    lv_obj_t *done_arc;
    lv_obj_t *remain_arc;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint16_t anim_visual_sweep;
    uint32_t anim_start_time;
    uint8_t anim_to_level;
} pump_both_level_side_arc_t;

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_PUMP_BOTH_LEVEL_ADJUST_COMP_BG
    lv_obj_t *background;
#endif
    pump_both_level_side_arc_t side[2];
    lv_obj_t *value_box[2];
    lv_obj_t *value_label[2];
    lv_obj_t *dir_box[2];
    lv_obj_t *dir_label[2];
    lv_obj_t *mode_switch_box;
    lv_obj_t *mode_switch_label;
    lv_obj_t *over_limit_box;
    lv_obj_t *over_limit_label;
    ui_manager_context_t *ui;
    lv_obj_t *time_label;
    lv_obj_t *status_icon;
    lv_obj_t *time_h[PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[PUMP_BOTH_LEVEL_TIME_COLON_COUNT];
#if IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_3SEG_TEST
    uint8_t test_hours;
#endif
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;
    uint8_t shown_paused;
    uint16_t shown_status_icon_id;
    uint8_t shown_level[2];
    uint8_t shown_over_limit;
    uint8_t shown_mode_switch;
    uint8_t shown_mode_type;
    uint8_t pump_mode;
    uint8_t mode_type;
    uint8_t level_max;
    uint8_t force_update;
    uint32_t accent_color;
    uint32_t over_limit_color;
    uint32_t shown_accent_color;
    uint32_t shown_over_limit_color;
} pump_both_level_adjust_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_BOTH_LEVEL_ADJUST_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t pump_both_level_adjust_bg_data[] = {
    #include "comparison_png2txt/LR_level_adjust12_352x352.txt"
};
#endif

static pump_both_level_adjust_ui_info_t *pump_both_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_both_level_adjust_root_init(void)
{
    if (pump_both_info->root == NULL || !lv_obj_is_valid(pump_both_info->root)) {
        pump_both_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(pump_both_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(pump_both_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(pump_both_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pump_both_info->root, 0, 0);
    lv_obj_set_style_pad_all(pump_both_info->root, 0, 0);
    lv_obj_align(pump_both_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(pump_both_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(pump_both_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_PUMP_BOTH_LEVEL_ADJUST_COMP_BG
static void pump_both_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (pump_both_info->background == NULL || !lv_obj_is_valid(pump_both_info->background)) {
        pump_both_info->background = lv_image_create(pump_both_info->root);
        EXCEPTION_CHECK_VOID(pump_both_info->background);
        lv_obj_move_background(pump_both_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(pump_both_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(pump_both_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER,
            pump_both_level_adjust_bg_data, sizeof(pump_both_level_adjust_bg_data), "png");
        lv_image_set_src(pump_both_info->background, (const void *)&bg_img_path);
        lv_obj_align(pump_both_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_both_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static uint16_t pump_both_level_arc_sweep_from_level(uint8_t level, uint8_t level_max,
    uint16_t track_deg)
{
    uint16_t split_track;

    if (level_max == 0 || level == 0) {
        return 0;
    }
    if (level >= level_max) {
        return track_deg;
    }
    split_track = track_deg - PUMP_BOTH_LEVEL_ARC_SEG_GAP_DEG;
    return (uint16_t)((split_track * level) / level_max);
}

static uint16_t pump_both_level_arc_visual_sweep_get(pump_both_level_side_arc_t *side)
{
    uint32_t elapsed;
    uint16_t from = side->anim_from_sweep;
    uint16_t to = side->anim_to_sweep;

    if (from == to) {
        return to;
    }

    elapsed = get_timestamp_ms(side->anim_start_time);
    if (elapsed >= PUMP_BOTH_LEVEL_ARC_ANIM_MS) {
        side->anim_from_sweep = to;
        return to;
    }

    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) / PUMP_BOTH_LEVEL_ARC_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) / PUMP_BOTH_LEVEL_ARC_ANIM_MS);
}

static void pump_both_level_arc_anim_sync(pump_both_level_side_arc_t *side,
    uint8_t target_level, uint8_t level_max, uint16_t track_deg, bool snap)
{
    uint16_t target_sweep = pump_both_level_arc_sweep_from_level(target_level, level_max, track_deg);

    /* 仅首次进入硬切；模式切换与调档一律从当前视觉位置插值 */
    if (snap || side->anim_to_level == 0xFF) {
        side->anim_from_sweep = target_sweep;
        side->anim_to_sweep = target_sweep;
        side->anim_visual_sweep = target_sweep;
        side->anim_to_level = target_level;
        side->anim_start_time = get_timestamp();
        return;
    }

    if (side->anim_to_level == target_level && side->anim_to_sweep == target_sweep) {
        return;
    }

    side->anim_from_sweep = pump_both_level_arc_visual_sweep_get(side);
    side->anim_to_sweep = target_sweep;
    side->anim_to_level = target_level;
    side->anim_start_time = get_timestamp();
}

static void pump_both_level_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, PUMP_BOTH_LEVEL_ARC_SIZE, PUMP_BOTH_LEVEL_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, PUMP_BOTH_LEVEL_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, PUMP_BOTH_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static void pump_both_level_side_arc_apply(pump_both_level_side_arc_t *side,
    uint16_t track_start, uint16_t track_deg, bool from_bottom, uint16_t done_sweep)
{
    uint16_t remain_sweep;
    uint16_t done_start;
    uint16_t done_end;
    uint16_t remain_start;
    uint16_t remain_end;

    if (done_sweep >= track_deg) {
        done_sweep = track_deg;
        remain_sweep = 0;
    } else if (done_sweep == 0) {
        remain_sweep = track_deg;
    } else {
        uint16_t split_track = track_deg - PUMP_BOTH_LEVEL_ARC_SEG_GAP_DEG;
        if (done_sweep > split_track) {
            done_sweep = split_track;
        }
        remain_sweep = split_track - done_sweep;
    }

    if (from_bottom) {
        /* 右弧：进度自底向上，done 贴底端 */
        done_end = track_start + track_deg;
        done_start = done_end - done_sweep;
        if (remain_sweep > 0 && done_sweep > 0) {
            remain_end = done_start - PUMP_BOTH_LEVEL_ARC_SEG_GAP_DEG;
            remain_start = track_start;
            remain_sweep = remain_end - remain_start;
        } else if (remain_sweep > 0) {
            remain_start = track_start;
            remain_end = track_start + track_deg;
        } else {
            remain_start = 0;
            remain_end = 0;
        }
    } else {
        /* 左弧：进度自底向上，done 贴起点 */
        done_start = track_start;
        done_end = track_start + done_sweep;
        if (remain_sweep > 0 && done_sweep > 0) {
            remain_start = done_end + PUMP_BOTH_LEVEL_ARC_SEG_GAP_DEG;
            remain_end = track_start + track_deg;
            remain_sweep = remain_end - remain_start;
        } else if (remain_sweep > 0) {
            remain_start = track_start;
            remain_end = track_start + track_deg;
        } else {
            remain_start = 0;
            remain_end = 0;
        }
    }

    if (done_sweep > 0) {
        lv_arc_set_angles(side->done_arc, done_start, done_end);
        lv_obj_clear_flag(side->done_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(side->done_arc, 0, 0);
        lv_obj_add_flag(side->done_arc, LV_OBJ_FLAG_HIDDEN);
    }

    if (remain_sweep > 0) {
        lv_arc_set_angles(side->remain_arc, remain_start, remain_end);
        lv_obj_clear_flag(side->remain_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_arc_set_angles(side->remain_arc, 0, 0);
        lv_obj_add_flag(side->remain_arc, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_invalidate(side->done_arc);
    lv_obj_invalidate(side->remain_arc);
}

static void pump_both_level_arc_update(ui_proc_pump_t *proc, uint8_t level_max,
    uint32_t accent_color)
{
    uint8_t i;
    bool color_changed = (pump_both_info->shown_accent_color != accent_color);

    for (i = 0; i < 2; i++) {
        pump_both_level_side_arc_t *side = &pump_both_info->side[i];
        uint16_t track_deg = (i == 0) ? PUMP_BOTH_LEVEL_LEFT_TRACK : PUMP_BOTH_LEVEL_RIGHT_TRACK;
        uint16_t track_start = (i == 0) ? PUMP_BOTH_LEVEL_LEFT_START : PUMP_BOTH_LEVEL_RIGHT_START;
        uint16_t done_sweep;
        bool need_draw;

        if (side->done_arc == NULL || !lv_obj_is_valid(side->done_arc)) {
            side->done_arc = lv_arc_create(pump_both_info->root);
            EXCEPTION_CHECK_VOID(side->done_arc);
            pump_both_level_arc_style_init(side->done_arc, accent_color);
        }
        if (side->remain_arc == NULL || !lv_obj_is_valid(side->remain_arc)) {
            side->remain_arc = lv_arc_create(pump_both_info->root);
            EXCEPTION_CHECK_VOID(side->remain_arc);
            pump_both_level_arc_style_init(side->remain_arc, PUMP_BOTH_LEVEL_ARC_REMAIN_COLOR);
        }

        if (color_changed || pump_both_info->force_update) {
            lv_obj_set_style_arc_color(side->done_arc, lv_color_hex(accent_color),
                LV_PART_INDICATOR);
        }

        pump_both_level_arc_anim_sync(side, proc->both_level_arc[i], level_max, track_deg, false);
        done_sweep = pump_both_level_arc_visual_sweep_get(side);
        need_draw = (pump_both_info->force_update || color_changed ||
            side->anim_visual_sweep != done_sweep);
        if (!need_draw) {
            continue;
        }
        side->anim_visual_sweep = done_sweep;
        pump_both_level_side_arc_apply(side, track_start, track_deg, (i == 1), done_sweep);
    }

    pump_both_info->shown_accent_color = accent_color;
}

static lv_obj_t *pump_both_level_box_create(lv_coord_t x, lv_coord_t y,
    lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *box = lv_obj_create(pump_both_info->root);
    EXCEPTION_CHECK_POINT(box);
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, w, h);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_CLICKABLE);
    return box;
}

static void pump_both_level_value_update(ui_proc_pump_t *proc)
{
    uint8_t i;
    char num_buf[8] = {0};
    static const lv_coord_t value_x[2] = {
        PUMP_BOTH_LEVEL_VALUE_LEFT_X, PUMP_BOTH_LEVEL_VALUE_RIGHT_X
    };

    for (i = 0; i < 2; i++) {
        if (pump_both_info->value_box[i] == NULL || !lv_obj_is_valid(pump_both_info->value_box[i])) {
            pump_both_info->value_box[i] = pump_both_level_box_create(value_x[i],
                PUMP_BOTH_LEVEL_VALUE_Y, PUMP_BOTH_LEVEL_VALUE_W, PUMP_BOTH_LEVEL_VALUE_H);
            EXCEPTION_CHECK_VOID(pump_both_info->value_box[i]);
            pump_both_info->shown_level[i] = 0xFF;
        }
        if (pump_both_info->value_label[i] == NULL || !lv_obj_is_valid(pump_both_info->value_label[i])) {
            pump_both_info->value_label[i] = lv_label_create(pump_both_info->value_box[i]);
            EXCEPTION_CHECK_VOID(pump_both_info->value_label[i]);
        }
        if (pump_both_info->force_update == false &&
            pump_both_info->shown_level[i] == proc->both_level_value[i]) {
            continue;
        }

        snprintf(num_buf, sizeof(num_buf), "%u", proc->both_level_value[i]);
        lv_label_set_text(pump_both_info->value_label[i], num_buf);
        lv_obj_set_style_text_color(pump_both_info->value_label[i],
            lv_color_hex(PUMP_BOTH_LEVEL_VALUE_COLOR), 0);
        lv_obj_set_style_text_font(pump_both_info->value_label[i],
            &lv_font_aeonik_soft_69_semibold, 0);
        lv_obj_set_style_text_align(pump_both_info->value_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(pump_both_info->value_label[i]);
        pump_both_info->shown_level[i] = proc->both_level_value[i];
    }
}

static void pump_both_level_dir_update(void)
{
    uint8_t i;
    bool need_style = (pump_both_info->force_update != 0);
    static const lv_coord_t dir_x[2] = {
        PUMP_BOTH_LEVEL_DIR_LEFT_X, PUMP_BOTH_LEVEL_DIR_RIGHT_X
    };
    static const char *dir_text[2] = {
        PUMP_BOTH_LEVEL_DIR_LEFT_TEXT, PUMP_BOTH_LEVEL_DIR_RIGHT_TEXT
    };

    for (i = 0; i < 2; i++) {
        if (pump_both_info->dir_box[i] == NULL || !lv_obj_is_valid(pump_both_info->dir_box[i])) {
            pump_both_info->dir_box[i] = pump_both_level_box_create(dir_x[i],
                PUMP_BOTH_LEVEL_DIR_Y, PUMP_BOTH_LEVEL_DIR_W, PUMP_BOTH_LEVEL_DIR_H);
            EXCEPTION_CHECK_VOID(pump_both_info->dir_box[i]);
            need_style = true;
        }
        if (pump_both_info->dir_label[i] == NULL || !lv_obj_is_valid(pump_both_info->dir_label[i])) {
            pump_both_info->dir_label[i] = lv_label_create(pump_both_info->dir_box[i]);
            EXCEPTION_CHECK_VOID(pump_both_info->dir_label[i]);
            need_style = true;
        }
    }

    if (need_style == false) {
        return;
    }

    for (i = 0; i < 2; i++) {
        lv_obj_set_size(pump_both_info->dir_box[i],
            PUMP_BOTH_LEVEL_DIR_W, PUMP_BOTH_LEVEL_DIR_H);
        lv_obj_set_align(pump_both_info->dir_box[i], LV_ALIGN_TOP_LEFT);
        lv_obj_set_pos(pump_both_info->dir_box[i], dir_x[i], PUMP_BOTH_LEVEL_DIR_Y);
        lv_obj_clear_flag(pump_both_info->dir_box[i], LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(pump_both_info->dir_label[i], dir_text[i]);
        lv_obj_set_style_text_color(pump_both_info->dir_label[i],
            lv_color_hex(PUMP_BOTH_LEVEL_DIR_COLOR), 0);
        lv_obj_set_style_text_font(pump_both_info->dir_label[i],
            &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(pump_both_info->dir_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(pump_both_info->dir_label[i]);
    }
}

static void pump_both_level_mode_switch_update(ui_proc_pump_t *proc, uint32_t accent_color)
{
    char switch_buf[32] = {0};
    uint8_t mode_switch = proc->sub_mode_switch;

    if (pump_both_info->mode_switch_box == NULL ||
        !lv_obj_is_valid(pump_both_info->mode_switch_box)) {
        pump_both_info->mode_switch_box = pump_both_level_box_create(0,
            PUMP_BOTH_LEVEL_MODE_SWITCH_Y,
            PUMP_BOTH_LEVEL_MODE_SWITCH_W, PUMP_BOTH_LEVEL_MODE_SWITCH_H);
        EXCEPTION_CHECK_VOID(pump_both_info->mode_switch_box);
        pump_both_info->shown_mode_switch = 0xFF;
        pump_both_info->shown_mode_type = 0xFF;
    }
    if (pump_both_info->mode_switch_label == NULL ||
        !lv_obj_is_valid(pump_both_info->mode_switch_label)) {
        pump_both_info->mode_switch_label = lv_label_create(pump_both_info->mode_switch_box);
        EXCEPTION_CHECK_VOID(pump_both_info->mode_switch_label);
        lv_obj_set_style_text_font(pump_both_info->mode_switch_label,
            &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(pump_both_info->mode_switch_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(pump_both_info->mode_switch_label, PUMP_BOTH_LEVEL_MODE_SWITCH_W);
        lv_obj_center(pump_both_info->mode_switch_label);
    }

    if (pump_both_info->force_update == false &&
        pump_both_info->shown_mode_switch == mode_switch &&
        pump_both_info->shown_mode_type == proc->sub_mode_type) {
        return;
    }

    if (mode_switch) {
        snprintf(switch_buf, sizeof(switch_buf), PUMP_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT,
            home_level_adjust_mode_type_name_get(proc->sub_mode_type));
        lv_label_set_text(pump_both_info->mode_switch_label, switch_buf);
        lv_obj_set_style_text_color(pump_both_info->mode_switch_label,
            lv_color_hex(accent_color), 0);
        lv_obj_clear_flag(pump_both_info->mode_switch_box, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(pump_both_info->mode_switch_box, LV_OBJ_FLAG_HIDDEN);
    }
    pump_both_info->shown_mode_switch = mode_switch;
    pump_both_info->shown_mode_type = proc->sub_mode_type;
}

static void pump_both_level_over_limit_update(ui_proc_pump_t *proc, uint32_t color)
{
    const char *text = PUMP_BOTH_LEVEL_OVER_LIMIT_TEXT;

    if (pump_both_info->over_limit_box == NULL || !lv_obj_is_valid(pump_both_info->over_limit_box)) {
        pump_both_info->over_limit_box = pump_both_level_box_create(
            (PANLE_DISPLAY_W - PUMP_BOTH_LEVEL_OVER_LIMIT_W) / 2,
            PUMP_BOTH_LEVEL_OVER_LIMIT_Y,
            PUMP_BOTH_LEVEL_OVER_LIMIT_W, PUMP_BOTH_LEVEL_OVER_LIMIT_H);
        EXCEPTION_CHECK_VOID(pump_both_info->over_limit_box);
        pump_both_info->shown_over_limit = 0xFF;
    }
    if (pump_both_info->over_limit_label == NULL || !lv_obj_is_valid(pump_both_info->over_limit_label)) {
        pump_both_info->over_limit_label = lv_label_create(pump_both_info->over_limit_box);
        EXCEPTION_CHECK_VOID(pump_both_info->over_limit_label);
        lv_obj_set_style_text_font(pump_both_info->over_limit_label,
            &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(pump_both_info->over_limit_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(pump_both_info->over_limit_label, PUMP_BOTH_LEVEL_OVER_LIMIT_W);
        lv_obj_center(pump_both_info->over_limit_label);
        lv_obj_set_style_text_line_space(pump_both_info->over_limit_label, PUMP_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE, 0);
    }

    if (pump_both_info->force_update == false &&
        pump_both_info->shown_over_limit == proc->both_level_over_limit &&
        pump_both_info->shown_over_limit_color == color) {
        return;
    }

    lv_label_set_text(pump_both_info->over_limit_label, text);
    lv_obj_set_style_text_color(pump_both_info->over_limit_label, lv_color_hex(color), 0);
    if (proc->both_level_over_limit) {
        lv_obj_clear_flag(pump_both_info->over_limit_box, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(pump_both_info->over_limit_box, LV_OBJ_FLAG_HIDDEN);
    }
    pump_both_info->shown_over_limit = proc->both_level_over_limit;
    pump_both_info->shown_over_limit_color = color;
}

static void pump_both_level_obj_flag_set(lv_obj_t *obj, bool visible)
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

static void pump_both_level_time_pair_update(lv_obj_t *labels[PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x, lv_coord_t seg_y, bool hidden)
{
    const lv_coord_t cell_w = PUMP_BOTH_LEVEL_TIME_SEG_WIDTH / PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(PUMP_BOTH_LEVEL_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
        pump_both_level_obj_flag_set(labels[i], !hidden);
    }
}

static lv_coord_t pump_both_level_time_colon_w_get(void)
{
    lv_coord_t colon_w = PUMP_BOTH_LEVEL_TIME_COLON_W_FALLBACK;
    lv_obj_t *colon = pump_both_info->time_colon[1];

    if (colon == NULL || !lv_obj_is_valid(colon)) {
        colon = pump_both_info->time_colon[0];
    }
    if (colon != NULL && lv_obj_is_valid(colon)) {
        lv_obj_update_layout(colon);
        colon_w = lv_obj_get_width(colon);
        if (colon_w <= 0) {
            colon_w = PUMP_BOTH_LEVEL_TIME_COLON_W_FALLBACK;
        }
    }
    return colon_w;
}

static lv_coord_t pump_both_level_time_unit_span_get(void)
{
    return PUMP_BOTH_LEVEL_TIME_SEG_WIDTH + PUMP_BOTH_LEVEL_TIME_COLON_GAP +
        pump_both_level_time_colon_w_get() + PUMP_BOTH_LEVEL_TIME_COLON_GAP;
}

static void pump_both_level_time_colon_ensure(void)
{
    int i;

    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_COLON_COUNT; i++) {
        if (pump_both_info->time_colon[i] == NULL ||
            !lv_obj_is_valid(pump_both_info->time_colon[i])) {
            pump_both_info->time_colon[i] = lv_label_create(pump_both_info->root);
            EXCEPTION_CHECK_VOID(pump_both_info->time_colon[i]);
            lv_obj_set_style_text_color(pump_both_info->time_colon[i],
                lv_color_hex(PUMP_BOTH_LEVEL_TIME_COLOR), 0);
            lv_obj_set_style_text_font(pump_both_info->time_colon[i],
                &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_align(pump_both_info->time_colon[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_text(pump_both_info->time_colon[i], ":");
        }
    }
}

static void pump_both_level_time_layout_apply(uint8_t hours, uint8_t minutes, uint8_t seconds,
    uint8_t digit_size, bool force)
{
    const lv_coord_t unit_span = pump_both_level_time_unit_span_get();
    const lv_coord_t colon_w = pump_both_level_time_colon_w_get();
    const lv_coord_t m_x = PUMP_BOTH_LEVEL_TIME_2SEG_M_X;
    const lv_coord_t colon_ms_x = m_x + PUMP_BOTH_LEVEL_TIME_SEG_WIDTH +
        PUMP_BOTH_LEVEL_TIME_COLON_GAP;
    const lv_coord_t s_x = colon_ms_x + colon_w + PUMP_BOTH_LEVEL_TIME_COLON_GAP;
    lv_coord_t icon_x = PUMP_BOTH_LEVEL_TIME_2SEG_ICON_X;
    lv_coord_t h_x = m_x;
    lv_coord_t colon_hm_x = colon_ms_x;
    const bool is_3seg = (digit_size == PUMP_BOTH_LEVEL_TIME_3SEG);
    int i;

    if (is_3seg) {
        h_x = m_x - unit_span;
        colon_hm_x = h_x + PUMP_BOTH_LEVEL_TIME_SEG_WIDTH + PUMP_BOTH_LEVEL_TIME_COLON_GAP;
        icon_x = PUMP_BOTH_LEVEL_TIME_2SEG_ICON_X - unit_span;
    }

    if (pump_both_info->status_icon != NULL && lv_obj_is_valid(pump_both_info->status_icon)) {
        lv_obj_align(pump_both_info->status_icon, LV_ALIGN_TOP_LEFT, icon_x,
            PUMP_BOTH_LEVEL_TIME_ICON_Y);
    }

    if (force || pump_both_info->shown_hours != hours ||
        pump_both_info->shown_digit_size != digit_size) {
        pump_both_level_time_pair_update(pump_both_info->time_h, pump_both_info->root, hours, h_x,
            PUMP_BOTH_LEVEL_TIME_Y, !is_3seg);
        pump_both_info->shown_hours = hours;
    }
    if (force || pump_both_info->shown_minutes != minutes ||
        pump_both_info->shown_digit_size != digit_size) {
        pump_both_level_time_pair_update(pump_both_info->time_m, pump_both_info->root, minutes, m_x,
            PUMP_BOTH_LEVEL_TIME_Y, false);
        pump_both_info->shown_minutes = minutes;
    }
    if (force || pump_both_info->shown_seconds != seconds ||
        pump_both_info->shown_digit_size != digit_size) {
        pump_both_level_time_pair_update(pump_both_info->time_s, pump_both_info->root, seconds, s_x,
            PUMP_BOTH_LEVEL_TIME_Y, false);
        pump_both_info->shown_seconds = seconds;
    }

    if (pump_both_info->time_colon[0] != NULL && lv_obj_is_valid(pump_both_info->time_colon[0])) {
        lv_obj_align(pump_both_info->time_colon[0], LV_ALIGN_TOP_LEFT, colon_hm_x,
            PUMP_BOTH_LEVEL_TIME_Y);
        pump_both_level_obj_flag_set(pump_both_info->time_colon[0], is_3seg);
    }
    if (pump_both_info->time_colon[1] != NULL && lv_obj_is_valid(pump_both_info->time_colon[1])) {
        lv_obj_align(pump_both_info->time_colon[1], LV_ALIGN_TOP_LEFT, colon_ms_x,
            PUMP_BOTH_LEVEL_TIME_Y);
        pump_both_level_obj_flag_set(pump_both_info->time_colon[1], true);
    }
    pump_both_info->shown_digit_size = digit_size;

    if (pump_both_info->time_label != NULL && lv_obj_is_valid(pump_both_info->time_label)) {
        lv_obj_move_foreground(pump_both_info->time_label);
    }
    if (pump_both_info->status_icon != NULL && lv_obj_is_valid(pump_both_info->status_icon)) {
        lv_obj_move_foreground(pump_both_info->status_icon);
    }
    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_COLON_COUNT; i++) {
        if (pump_both_info->time_colon[i] != NULL &&
            lv_obj_is_valid(pump_both_info->time_colon[i])) {
            lv_obj_move_foreground(pump_both_info->time_colon[i]);
        }
    }
    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT; i++) {
        if (pump_both_info->time_h[i] != NULL && lv_obj_is_valid(pump_both_info->time_h[i])) {
            lv_obj_move_foreground(pump_both_info->time_h[i]);
        }
        if (pump_both_info->time_m[i] != NULL && lv_obj_is_valid(pump_both_info->time_m[i])) {
            lv_obj_move_foreground(pump_both_info->time_m[i]);
        }
        if (pump_both_info->time_s[i] != NULL && lv_obj_is_valid(pump_both_info->time_s[i])) {
            lv_obj_move_foreground(pump_both_info->time_s[i]);
        }
    }
}

static void pump_both_level_time_rebuild(void)
{
    EXCEPTION_CHECK_VOID(pump_both_info->root && pump_both_info->ui);

    if (pump_both_info->time_label == NULL || !lv_obj_is_valid(pump_both_info->time_label)) {
        pump_both_info->time_label = lv_image_create(pump_both_info->root);
        EXCEPTION_CHECK_VOID(pump_both_info->time_label);
        lv_obj_clear_flag(pump_both_info->time_label, LV_OBJ_FLAG_CLICKABLE);
        ui_img_baj_theme_set_src(pump_both_info->ui, pump_both_info->time_label,
            BASE_RES_ID_PUMP_TIME_LABEL, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(pump_both_info->time_label, LV_ALIGN_TOP_MID, 0, PUMP_BOTH_LEVEL_TIME_LABEL_Y);
    }

    if (pump_both_info->status_icon == NULL || !lv_obj_is_valid(pump_both_info->status_icon)) {
        pump_both_info->status_icon = lv_image_create(pump_both_info->root);
        EXCEPTION_CHECK_VOID(pump_both_info->status_icon);
        lv_obj_clear_flag(pump_both_info->status_icon, LV_OBJ_FLAG_CLICKABLE);
        pump_both_info->shown_status_icon_id = 0;
        pump_both_info->shown_paused = 0xFF;
    }

    pump_both_level_time_colon_ensure();
    pump_both_info->shown_hours = 0xFF;
    pump_both_info->shown_minutes = 0xFF;
    pump_both_info->shown_seconds = 0xFF;
    pump_both_info->shown_digit_size = 0;
    pump_both_level_time_layout_apply(0, 0, 0, PUMP_BOTH_LEVEL_TIME_2SEG, true);
}

static void pump_both_level_time_content_update(ui_proc_pump_t *proc)
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
#if IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_3SEG_TEST
    hours = pump_both_info->test_hours;
    minutes = pump_both_info->test_hours;
    seconds = pump_both_info->test_hours;
#else
    hours = hold_time / 3600;
    minutes = (hold_time / 60) % 60;
    seconds = hold_time % 60;
#endif
    digit_size = (hours == 0) ? PUMP_BOTH_LEVEL_TIME_2SEG : PUMP_BOTH_LEVEL_TIME_3SEG;
    paused = (proc->pump_status == BREAST_PUMP_PAUSE);
    icon_id = paused ? BASE_RES_ID_PLAY_MIN_ICON : BASE_RES_ID_PAUSE_MIN_ICON;

    if (pump_both_info->status_icon != NULL && lv_obj_is_valid(pump_both_info->status_icon) &&
        (pump_both_info->shown_status_icon_id != icon_id || pump_both_info->force_update ||
         pump_both_info->shown_paused != (paused ? 1 : 0))) {
        ui_img_baj_theme_set_src(pump_both_info->ui, pump_both_info->status_icon, icon_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        pump_both_info->shown_status_icon_id = icon_id;
        pump_both_info->shown_paused = paused ? 1 : 0;
    }

    pump_both_level_time_layout_apply(hours, minutes, seconds, digit_size,
        pump_both_info->force_update != 0);
}

#if IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_ENTER_ANIM
static void pump_both_level_time_obj_style_apply(lv_obj_t *obj, lv_opa_t opa, lv_coord_t y_off)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_set_style_opa(obj, opa, 0);
    lv_obj_set_style_translate_y(obj, y_off, 0);
    lv_obj_set_style_transform_scale(obj, PUMP_BOTH_LEVEL_TIME_SCALE_FULL, 0);
}

static void pump_both_level_time_visual_apply(lv_opa_t opa, lv_coord_t y_off)
{
    uint8_t i;

    if (pump_both_info == NULL) {
        return;
    }
    pump_both_level_time_obj_style_apply(pump_both_info->time_label, opa, y_off);
    pump_both_level_time_obj_style_apply(pump_both_info->status_icon, opa, y_off);
    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_COLON_COUNT; i++) {
        pump_both_level_time_obj_style_apply(pump_both_info->time_colon[i], opa, y_off);
    }
    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT; i++) {
        pump_both_level_time_obj_style_apply(pump_both_info->time_h[i], opa, y_off);
        pump_both_level_time_obj_style_apply(pump_both_info->time_m[i], opa, y_off);
        pump_both_level_time_obj_style_apply(pump_both_info->time_s[i], opa, y_off);
    }
}

static void pump_both_level_time_enter_anim_exec(void *var, int32_t progress)
{
    lv_opa_t opa;
    lv_coord_t y_off;

    (void)var;
    if (pump_both_info == NULL) {
        return;
    }
    if (progress < 0) {
        progress = 0;
    } else if (progress > PUMP_BOTH_LEVEL_TIME_ENTER_PROGRESS_FULL) {
        progress = PUMP_BOTH_LEVEL_TIME_ENTER_PROGRESS_FULL;
    }

    opa = (lv_opa_t)((progress * LV_OPA_COVER) / PUMP_BOTH_LEVEL_TIME_ENTER_PROGRESS_FULL);
    y_off = (lv_coord_t)((PUMP_BOTH_LEVEL_TIME_ENTER_Y_OFFSET *
        (PUMP_BOTH_LEVEL_TIME_ENTER_PROGRESS_FULL - progress)) /
        PUMP_BOTH_LEVEL_TIME_ENTER_PROGRESS_FULL);
    pump_both_level_time_visual_apply(opa, y_off);
}

static void pump_both_level_time_enter_anim_ready(lv_anim_t *anim)
{
    (void)anim;
    if (pump_both_info == NULL) {
        return;
    }
    pump_both_level_time_visual_apply(LV_OPA_COVER, 0);
}

static void pump_both_level_time_enter_anim_stop(void)
{
    if (pump_both_info == NULL) {
        return;
    }
    lv_anim_delete(pump_both_info, pump_both_level_time_enter_anim_exec);
    pump_both_level_time_visual_apply(LV_OPA_COVER, 0);
}

static void pump_both_level_time_enter_anim_start(void)
{
    lv_anim_t anim;

    EXCEPTION_CHECK_VOID(pump_both_info);
    lv_anim_delete(pump_both_info, pump_both_level_time_enter_anim_exec);
    pump_both_level_time_visual_apply(LV_OPA_TRANSP, PUMP_BOTH_LEVEL_TIME_ENTER_Y_OFFSET);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, pump_both_info);
    lv_anim_set_time(&anim, PUMP_BOTH_LEVEL_TIME_ENTER_ANIM_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&anim, pump_both_level_time_enter_anim_exec);
    lv_anim_set_completed_cb(&anim, pump_both_level_time_enter_anim_ready);
    lv_anim_set_values(&anim, 0, PUMP_BOTH_LEVEL_TIME_ENTER_PROGRESS_FULL);
    lv_anim_start(&anim);
}
#endif /* IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_ENTER_ANIM */

static void pump_both_level_time_update(ui_proc_pump_t *proc)
{
    if (pump_both_info->force_update) {
        pump_both_level_time_rebuild();
    }
    pump_both_level_time_content_update(proc);
#if IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_ENTER_ANIM
    if (pump_both_info->force_update) {
        pump_both_level_time_enter_anim_start();
    }
#endif
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
uint32_t pump_both_level_adjust_over_limit_color_get(uint8_t pump_mode)
{
    switch (pump_mode) {
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return PUMP_BOTH_LEVEL_OVER_LIMIT_RHYTHM_COLOR;
        case BREAST_PUMP_STIMULATION_MODE:
            return PUMP_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR;
        case BREAST_PUMP_LACTATION_MODE:
            return PUMP_BOTH_LEVEL_OVER_LIMIT_LACTATION_COLOR;
        default:
            return PUMP_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR;
    }
}

void ui_display_pump_both_level_adjust_prepare(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(thiz);
    if (pump_both_info == NULL) {
        pump_both_info = (pump_both_level_adjust_ui_info_t *)u_malloz(
            sizeof(pump_both_level_adjust_ui_info_t));
        EXCEPTION_CHECK_VOID(pump_both_info);
    }
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    pump_both_info->pump_mode = (uint8_t)pump_mode;
    pump_both_info->mode_type = proc->sub_mode_type;
    pump_both_info->level_max = home_level_adjust_max_get(pump_both_info->mode_type);
    pump_both_info->accent_color = home_level_adjust_accent_color_get(pump_both_info->pump_mode);
    pump_both_info->over_limit_color =
        pump_both_level_adjust_over_limit_color_get(pump_both_info->pump_mode);
    LOG_UI("pump both level adjust prepare mode=%d type=%d max=%d color=0x%06X",
        pump_both_info->pump_mode, pump_both_info->mode_type, pump_both_info->level_max,
        pump_both_info->accent_color);

    pump_both_info->ui = thiz;
    pump_both_info->force_update = true;
    pump_both_info->shown_level[0] = 0xFF;
    pump_both_info->shown_level[1] = 0xFF;
    pump_both_info->shown_over_limit = 0xFF;
    pump_both_info->shown_mode_switch = 0xFF;
    pump_both_info->shown_mode_type = 0xFF;
    pump_both_info->side[0].anim_to_level = 0xFF;
    pump_both_info->side[1].anim_to_level = 0xFF;
    pump_both_info->shown_accent_color = 0;
    pump_both_info->shown_over_limit_color = 0;
    pump_both_info->shown_hours = 0xFF;
    pump_both_info->shown_minutes = 0xFF;
    pump_both_info->shown_seconds = 0xFF;
    pump_both_info->shown_digit_size = 0;
    pump_both_info->shown_paused = 0xFF;
    pump_both_info->shown_status_icon_id = 0;
    pump_both_level_adjust_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_both_level_adjust_remove(ui_manager_context_t *thiz)
{
    int i;

    EXCEPTION_CHECK_VOID(thiz && pump_both_info && pump_both_info->root);
#if IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_ENTER_ANIM
    pump_both_level_time_enter_anim_stop();
#endif
    ui_img_baj_release_decoded_buf(pump_both_info->time_label);
    LVGL_OBJ_DELETE(pump_both_info->time_label);
    pump_both_info->time_label = NULL;
    ui_img_baj_release_decoded_buf(pump_both_info->status_icon);
    LVGL_OBJ_DELETE(pump_both_info->status_icon);
    pump_both_info->status_icon = NULL;
    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_COLON_COUNT; i++) {
        LVGL_OBJ_DELETE(pump_both_info->time_colon[i]);
        pump_both_info->time_colon[i] = NULL;
    }
    for (i = 0; i < PUMP_BOTH_LEVEL_TIME_DIGIT_COUNT; i++) {
        LVGL_OBJ_DELETE(pump_both_info->time_h[i]);
        pump_both_info->time_h[i] = NULL;
        LVGL_OBJ_DELETE(pump_both_info->time_m[i]);
        pump_both_info->time_m[i] = NULL;
        LVGL_OBJ_DELETE(pump_both_info->time_s[i]);
        pump_both_info->time_s[i] = NULL;
    }
    LVGL_OBJ_DELETE(pump_both_info->root);
    u_free(pump_both_info);
    pump_both_info = NULL;
}

int ui_display_pump_both_level_adjust_process(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;
    uint8_t level_max;

    EXCEPTION_CHECK_INT(thiz && pump_both_info && pump_both_info->root);
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump);

    level_max = home_level_adjust_max_get(proc->sub_mode_type);
    if (pump_both_info->mode_type != proc->sub_mode_type ||
        pump_both_info->level_max != level_max) {
        /* 模式切换：数字可跳变，进度条从当前扫角渐进到新目标，不硬切 */
        pump_both_info->mode_type = proc->sub_mode_type;
        pump_both_info->level_max = level_max;
    }

    pump_both_level_adjust_bg_update(thiz);
    pump_both_level_arc_update(proc, pump_both_info->level_max, pump_both_info->accent_color);
    pump_both_level_value_update(proc);
    pump_both_level_dir_update();
    pump_both_level_mode_switch_update(proc, pump_both_info->accent_color);
    pump_both_level_over_limit_update(proc, pump_both_info->over_limit_color);
    pump_both_level_time_update(proc);
    pump_both_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump both level adjust fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
#if IS_SUPPORT_PUMP_BOTH_LEVEL_TIME_3SEG_TEST
        pump_both_info->test_hours += 1;
        if (pump_both_info->test_hours >= 59) {
            pump_both_info->test_hours = 0;
        }
#endif
    }
    return 0;
}
