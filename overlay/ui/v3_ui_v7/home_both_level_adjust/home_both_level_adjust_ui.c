/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "home_both_level_adjust_ui.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_BOTH_LEVEL_ADJUST_COMP_BG   (0) /// 是否启用对比背景图

/** 圆弧：直径 352，线宽 11，当前/剩余间距 8px≈3° */
#define HOME_BOTH_LEVEL_ARC_SIZE                    (352)
#define HOME_BOTH_LEVEL_ARC_WIDTH                   (11)

#if IS_SUPPORT_HOME_BOTH_LEVEL_ADJUST_COMP_BG
#define HOME_BOTH_LEVEL_ARC_REMAIN_COLOR            (0xFFFFFF)
#else
#define HOME_BOTH_LEVEL_ARC_REMAIN_COLOR            (0x252525)
#endif

/** 段间距8 */
#define HOME_BOTH_LEVEL_ARC_SEG_GAP_DEG             (8)
/** 左弧：底部起沿左侧向上（参考图量测） */
#define HOME_BOTH_LEVEL_LEFT_START                  (136)
#define HOME_BOTH_LEVEL_LEFT_TRACK                  (122)
#define HOME_BOTH_LEVEL_LEFT_END                    (HOME_BOTH_LEVEL_LEFT_START + HOME_BOTH_LEVEL_LEFT_TRACK)
/** 右弧：与左弧左右镜像；顶→底顺时针，进度自底向上（from_bottom） */
#define HOME_BOTH_LEVEL_RIGHT_END                   (540 - HOME_BOTH_LEVEL_LEFT_START) /* 406 */
#define HOME_BOTH_LEVEL_RIGHT_TRACK                 (HOME_BOTH_LEVEL_LEFT_TRACK)
#define HOME_BOTH_LEVEL_RIGHT_START                 (HOME_BOTH_LEVEL_RIGHT_END - HOME_BOTH_LEVEL_RIGHT_TRACK) /* 282 */

#if IS_SUPPORT_HOME_BOTH_LEVEL_ADJUST_COMP_BG
#define HOME_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR       (0x00FF00)
#define HOME_BOTH_LEVEL_OVER_LIMIT_RHYTHM_COLOR     (0x00FF00)
#define HOME_BOTH_LEVEL_OVER_LIMIT_LACTATION_COLOR  (0x00FF00)
#else
#define HOME_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR       (0xE2637E)
#define HOME_BOTH_LEVEL_OVER_LIMIT_RHYTHM_COLOR     (0x9C78E8)
#define HOME_BOTH_LEVEL_OVER_LIMIT_LACTATION_COLOR  (0x64A4F2)
#endif

#define HOME_BOTH_LEVEL_VALUE_W                     (150)
#define HOME_BOTH_LEVEL_VALUE_H                     (69)
#define HOME_BOTH_LEVEL_VALUE_Y                     (122)
#define HOME_BOTH_LEVEL_VALUE_LEFT_X                (29)
#define HOME_BOTH_LEVEL_VALUE_RIGHT_X               (179)
#define HOME_BOTH_LEVEL_VALUE_COLOR                 (0xFFFFFF)

#define HOME_BOTH_LEVEL_DIR_W                       (150)
#define HOME_BOTH_LEVEL_DIR_H                       (34)
#define HOME_BOTH_LEVEL_DIR_Y                       (203)
#define HOME_BOTH_LEVEL_DIR_LEFT_X                  (29)
#define HOME_BOTH_LEVEL_DIR_RIGHT_X                 (179)
#define HOME_BOTH_LEVEL_DIR_COLOR                   (0x999999)
#define HOME_BOTH_LEVEL_DIR_LEFT_TEXT               "L"
#define HOME_BOTH_LEVEL_DIR_RIGHT_TEXT              "R"

#define HOME_BOTH_LEVEL_OVER_LIMIT_W                (220)
#define HOME_BOTH_LEVEL_OVER_LIMIT_H                (50)
#define HOME_BOTH_LEVEL_OVER_LIMIT_Y                (68)
#define HOME_BOTH_LEVEL_OVER_LIMIT_TEXT \
    "Max 3 levels at a time\nfor your safety"

/** 底部提示图：与速度调节一致 */
#define HOME_BOTH_LEVEL_TIP_W                       (212)
#define HOME_BOTH_LEVEL_TIP_H                       (50)
#define HOME_BOTH_LEVEL_TIP_Y                       (PANLE_DISPLAY_H - HOME_BOTH_LEVEL_TIP_H - 8)

#define HOME_BOTH_LEVEL_TIP_SHINE_HARD              (0)
#define HOME_BOTH_LEVEL_TIP_SHINE_SOFT              (1)
#define HOME_BOTH_LEVEL_TIP_SHINE_OVERLAY           (2)
#ifndef HOME_BOTH_LEVEL_TIP_SHINE_MODE
#define HOME_BOTH_LEVEL_TIP_SHINE_MODE              HOME_BOTH_LEVEL_TIP_SHINE_OVERLAY
#endif
#define HOME_BOTH_LEVEL_TIP_SHINE_IS_HARD \
    (HOME_BOTH_LEVEL_TIP_SHINE_MODE == HOME_BOTH_LEVEL_TIP_SHINE_HARD)
#define HOME_BOTH_LEVEL_TIP_SHINE_IS_SOFT \
    (HOME_BOTH_LEVEL_TIP_SHINE_MODE == HOME_BOTH_LEVEL_TIP_SHINE_SOFT)
#define HOME_BOTH_LEVEL_TIP_SHINE_IS_OVERLAY \
    (HOME_BOTH_LEVEL_TIP_SHINE_MODE == HOME_BOTH_LEVEL_TIP_SHINE_OVERLAY)
#define HOME_BOTH_LEVEL_TIP_SHINE_USE_IMG_MASK \
    (HOME_BOTH_LEVEL_TIP_SHINE_IS_SOFT || HOME_BOTH_LEVEL_TIP_SHINE_IS_OVERLAY)

#ifndef HOME_BOTH_LEVEL_TIP_SHINE_W
#define HOME_BOTH_LEVEL_TIP_SHINE_W                 (80)
#endif

#if HOME_BOTH_LEVEL_TIP_SHINE_IS_HARD
#define HOME_BOTH_LEVEL_TIP_MASK_W                  (HOME_BOTH_LEVEL_TIP_SHINE_W)
#define HOME_BOTH_LEVEL_TIP_MASK_H                  (HOME_BOTH_LEVEL_TIP_H)
#define HOME_BOTH_LEVEL_TIP_MASK_Y                  (HOME_BOTH_LEVEL_TIP_Y)
#define HOME_BOTH_LEVEL_TIP_SHINE_RECOLOR_OPA       (LV_OPA_COVER)
#define HOME_BOTH_LEVEL_TIP_BASE_OPA                (LV_OPA_COVER)
#else
#define HOME_BOTH_LEVEL_TIP_MASK_W                  (HOME_BOTH_LEVEL_TIP_SHINE_W)
#define HOME_BOTH_LEVEL_TIP_MASK_H                  (HOME_BOTH_LEVEL_TIP_H)
#define HOME_BOTH_LEVEL_TIP_MASK_Y                  (HOME_BOTH_LEVEL_TIP_Y)
#define HOME_BOTH_LEVEL_TIP_SHINE_RECOLOR_OPA       (LV_OPA_80)
#define HOME_BOTH_LEVEL_TIP_BASE_OPA                (LV_OPA_70)
#endif
#define HOME_BOTH_LEVEL_TIP_MASK_X_MIN              (-HOME_BOTH_LEVEL_TIP_MASK_W)
#define HOME_BOTH_LEVEL_TIP_MASK_X_MAX              (PANLE_DISPLAY_W)
#define HOME_BOTH_LEVEL_TIP_MASK_PERIOD_MS          (3200)
#define HOME_BOTH_LEVEL_TIP_SHINE_COLOR             (0xFFFFFF)

#define HOME_BOTH_LEVEL_ARC_ANIM_MS                 (220)

typedef struct {
    lv_obj_t *done_arc;
    lv_obj_t *remain_arc;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint16_t anim_visual_sweep;
    uint32_t anim_start_time;
    uint8_t anim_to_level;
} home_both_level_side_arc_t;

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_HOME_BOTH_LEVEL_ADJUST_COMP_BG
    lv_obj_t *background;
#endif
    home_both_level_side_arc_t side[2];
    lv_obj_t *value_box[2];
    lv_obj_t *value_label[2];
    lv_obj_t *dir_box[2];
    lv_obj_t *dir_label[2];
    lv_obj_t *over_limit_box;
    lv_obj_t *over_limit_label;
    lv_obj_t *tip_img;
    lv_obj_t *tip_shine;
    lv_obj_t *tip_mask;
#if HOME_BOTH_LEVEL_TIP_SHINE_USE_IMG_MASK
    lv_draw_buf_t *tip_mask_a8;
#endif
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    int16_t tip_mask_x;
    uint32_t tip_mask_tick_ms;
    uint8_t shown_level[2];
    uint8_t shown_over_limit;
    uint8_t pump_mode;
    uint8_t level_max;
    uint8_t force_update;
    uint8_t rsv;
    uint32_t accent_color;
    uint32_t over_limit_color;
    uint32_t shown_accent_color;
    uint32_t shown_over_limit_color;
} home_both_level_adjust_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_BOTH_LEVEL_ADJUST_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_both_level_adjust_bg_data[] = {
    #include "comparison_png2txt/LR_level_adjust12_352x352.txt"
};
#endif

static home_both_level_adjust_ui_info_t *both_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_both_level_adjust_root_init(void)
{
    if (both_info->root == NULL || !lv_obj_is_valid(both_info->root)) {
        both_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(both_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(both_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(both_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(both_info->root, 0, 0);
    lv_obj_set_style_pad_all(both_info->root, 0, 0);
    lv_obj_align(both_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(both_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(both_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_HOME_BOTH_LEVEL_ADJUST_COMP_BG
static void home_both_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (both_info->background == NULL || !lv_obj_is_valid(both_info->background)) {
        both_info->background = lv_image_create(both_info->root);
        EXCEPTION_CHECK_VOID(both_info->background);
        lv_obj_move_background(both_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(both_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(both_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER,
            home_both_level_adjust_bg_data, sizeof(home_both_level_adjust_bg_data), "png");
        lv_image_set_src(both_info->background, (const void *)&bg_img_path);
        lv_obj_align(both_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_both_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static uint16_t home_both_level_arc_sweep_from_level(uint8_t level, uint8_t level_max,
    uint16_t track_deg)
{
    uint16_t split_track;

    if (level_max == 0 || level == 0) {
        return 0;
    }
    if (level >= level_max) {
        return track_deg;
    }
    split_track = track_deg - HOME_BOTH_LEVEL_ARC_SEG_GAP_DEG;
    return (uint16_t)((split_track * level) / level_max);
}

static uint16_t home_both_level_arc_visual_sweep_get(home_both_level_side_arc_t *side)
{
    uint32_t elapsed;
    uint16_t from = side->anim_from_sweep;
    uint16_t to = side->anim_to_sweep;

    if (from == to) {
        return to;
    }

    elapsed = get_timestamp_ms(side->anim_start_time);
    if (elapsed >= HOME_BOTH_LEVEL_ARC_ANIM_MS) {
        side->anim_from_sweep = to;
        return to;
    }

    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) / HOME_BOTH_LEVEL_ARC_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) / HOME_BOTH_LEVEL_ARC_ANIM_MS);
}

static void home_both_level_arc_anim_sync(home_both_level_side_arc_t *side,
    uint8_t target_level, uint8_t level_max, uint16_t track_deg, bool snap)
{
    uint16_t target_sweep = home_both_level_arc_sweep_from_level(target_level, level_max, track_deg);

    if (both_info->force_update || snap) {
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

    side->anim_from_sweep = home_both_level_arc_visual_sweep_get(side);
    side->anim_to_sweep = target_sweep;
    side->anim_to_level = target_level;
    side->anim_start_time = get_timestamp();
}

static void home_both_level_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, HOME_BOTH_LEVEL_ARC_SIZE, HOME_BOTH_LEVEL_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, HOME_BOTH_LEVEL_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, HOME_BOTH_LEVEL_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static void home_both_level_side_arc_apply(home_both_level_side_arc_t *side,
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
        uint16_t split_track = track_deg - HOME_BOTH_LEVEL_ARC_SEG_GAP_DEG;
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
            remain_end = done_start - HOME_BOTH_LEVEL_ARC_SEG_GAP_DEG;
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
            remain_start = done_end + HOME_BOTH_LEVEL_ARC_SEG_GAP_DEG;
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

static void home_both_level_arc_update(ui_proc_home_t *proc, uint8_t level_max,
    uint32_t accent_color)
{
    uint8_t i;
    bool color_changed = (both_info->shown_accent_color != accent_color);

    for (i = 0; i < 2; i++) {
        home_both_level_side_arc_t *side = &both_info->side[i];
        uint16_t track_deg = (i == 0) ? HOME_BOTH_LEVEL_LEFT_TRACK : HOME_BOTH_LEVEL_RIGHT_TRACK;
        uint16_t track_start = (i == 0) ? HOME_BOTH_LEVEL_LEFT_START : HOME_BOTH_LEVEL_RIGHT_START;
        uint16_t done_sweep;
        bool need_draw;

        if (side->done_arc == NULL || !lv_obj_is_valid(side->done_arc)) {
            side->done_arc = lv_arc_create(both_info->root);
            EXCEPTION_CHECK_VOID(side->done_arc);
            home_both_level_arc_style_init(side->done_arc, accent_color);
        }
        if (side->remain_arc == NULL || !lv_obj_is_valid(side->remain_arc)) {
            side->remain_arc = lv_arc_create(both_info->root);
            EXCEPTION_CHECK_VOID(side->remain_arc);
            home_both_level_arc_style_init(side->remain_arc, HOME_BOTH_LEVEL_ARC_REMAIN_COLOR);
        }

        if (color_changed || both_info->force_update) {
            lv_obj_set_style_arc_color(side->done_arc, lv_color_hex(accent_color),
                LV_PART_INDICATOR);
        }

        home_both_level_arc_anim_sync(side, proc->both_level_arc[i], level_max, track_deg, false);
        done_sweep = home_both_level_arc_visual_sweep_get(side);
        need_draw = (both_info->force_update || color_changed ||
            side->anim_visual_sweep != done_sweep);
        if (!need_draw) {
            continue;
        }
        side->anim_visual_sweep = done_sweep;
        home_both_level_side_arc_apply(side, track_start, track_deg, (i == 1), done_sweep);
    }

    both_info->shown_accent_color = accent_color;
}

static lv_obj_t *home_both_level_box_create(lv_coord_t x, lv_coord_t y,
    lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *box = lv_obj_create(both_info->root);
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

static void home_both_level_value_update(ui_proc_home_t *proc)
{
    uint8_t i;
    char num_buf[8] = {0};
    static const lv_coord_t value_x[2] = {
        HOME_BOTH_LEVEL_VALUE_LEFT_X, HOME_BOTH_LEVEL_VALUE_RIGHT_X
    };

    for (i = 0; i < 2; i++) {
        if (both_info->value_box[i] == NULL || !lv_obj_is_valid(both_info->value_box[i])) {
            both_info->value_box[i] = home_both_level_box_create(value_x[i],
                HOME_BOTH_LEVEL_VALUE_Y, HOME_BOTH_LEVEL_VALUE_W, HOME_BOTH_LEVEL_VALUE_H);
            EXCEPTION_CHECK_VOID(both_info->value_box[i]);
            both_info->shown_level[i] = 0xFF;
        }
        if (both_info->value_label[i] == NULL || !lv_obj_is_valid(both_info->value_label[i])) {
            both_info->value_label[i] = lv_label_create(both_info->value_box[i]);
            EXCEPTION_CHECK_VOID(both_info->value_label[i]);
        }
        if (both_info->force_update == false &&
            both_info->shown_level[i] == proc->both_level_value[i]) {
            continue;
        }

        snprintf(num_buf, sizeof(num_buf), "%u", proc->both_level_value[i]);
        lv_label_set_text(both_info->value_label[i], num_buf);
        lv_obj_set_style_text_color(both_info->value_label[i],
            lv_color_hex(HOME_BOTH_LEVEL_VALUE_COLOR), 0);
        lv_obj_set_style_text_font(both_info->value_label[i],
            &lv_font_aeonik_soft_69_semibold, 0);
        lv_obj_set_style_text_align(both_info->value_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(both_info->value_label[i]);
        both_info->shown_level[i] = proc->both_level_value[i];
    }
}

static void home_both_level_dir_update(void)
{
    uint8_t i;
    static const lv_coord_t dir_x[2] = {
        HOME_BOTH_LEVEL_DIR_LEFT_X, HOME_BOTH_LEVEL_DIR_RIGHT_X
    };
    static const char *dir_text[2] = {
        HOME_BOTH_LEVEL_DIR_LEFT_TEXT, HOME_BOTH_LEVEL_DIR_RIGHT_TEXT
    };

    for (i = 0; i < 2; i++) {
        if (both_info->dir_box[i] != NULL && lv_obj_is_valid(both_info->dir_box[i]) &&
            both_info->force_update == false) {
            continue;
        }
        if (both_info->dir_box[i] == NULL || !lv_obj_is_valid(both_info->dir_box[i])) {
            both_info->dir_box[i] = home_both_level_box_create(dir_x[i],
                HOME_BOTH_LEVEL_DIR_Y, HOME_BOTH_LEVEL_DIR_W, HOME_BOTH_LEVEL_DIR_H);
            EXCEPTION_CHECK_VOID(both_info->dir_box[i]);
        }
        if (both_info->dir_label[i] == NULL || !lv_obj_is_valid(both_info->dir_label[i])) {
            both_info->dir_label[i] = lv_label_create(both_info->dir_box[i]);
            EXCEPTION_CHECK_VOID(both_info->dir_label[i]);
        }
        lv_label_set_text(both_info->dir_label[i], dir_text[i]);
        lv_obj_set_style_text_color(both_info->dir_label[i],
            lv_color_hex(HOME_BOTH_LEVEL_DIR_COLOR), 0);
        lv_obj_set_style_text_font(both_info->dir_label[i],
            &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(both_info->dir_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(both_info->dir_label[i]);
    }
}

static void home_both_level_over_limit_update(ui_proc_home_t *proc, uint32_t color)
{
    if (both_info->over_limit_box == NULL || !lv_obj_is_valid(both_info->over_limit_box)) {
        both_info->over_limit_box = home_both_level_box_create(
            (PANLE_DISPLAY_W - HOME_BOTH_LEVEL_OVER_LIMIT_W) / 2,
            HOME_BOTH_LEVEL_OVER_LIMIT_Y,
            HOME_BOTH_LEVEL_OVER_LIMIT_W, HOME_BOTH_LEVEL_OVER_LIMIT_H);
        EXCEPTION_CHECK_VOID(both_info->over_limit_box);
        both_info->shown_over_limit = 0xFF;
    }
    if (both_info->over_limit_label == NULL || !lv_obj_is_valid(both_info->over_limit_label)) {
        both_info->over_limit_label = lv_label_create(both_info->over_limit_box);
        EXCEPTION_CHECK_VOID(both_info->over_limit_label);
        lv_label_set_text(both_info->over_limit_label, HOME_BOTH_LEVEL_OVER_LIMIT_TEXT);
        lv_obj_set_style_text_font(both_info->over_limit_label,
            &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(both_info->over_limit_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(both_info->over_limit_label, HOME_BOTH_LEVEL_OVER_LIMIT_W);
        lv_obj_center(both_info->over_limit_label);
    }

    if (both_info->force_update == false &&
        both_info->shown_over_limit == proc->both_level_over_limit &&
        both_info->shown_over_limit_color == color) {
        return;
    }

    lv_obj_set_style_text_color(both_info->over_limit_label, lv_color_hex(color), 0);
    if (proc->both_level_over_limit) {
        lv_obj_clear_flag(both_info->over_limit_box, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(both_info->over_limit_box, LV_OBJ_FLAG_HIDDEN);
    }
    both_info->shown_over_limit = proc->both_level_over_limit;
    both_info->shown_over_limit_color = color;
}

static void home_both_level_tip_shine_sync(void)
{
    lv_coord_t tip_x;
    lv_coord_t tip_y;

    if (both_info->tip_img == NULL || !lv_obj_is_valid(both_info->tip_img) ||
        both_info->tip_shine == NULL || !lv_obj_is_valid(both_info->tip_shine) ||
        both_info->tip_mask == NULL || !lv_obj_is_valid(both_info->tip_mask)) {
        return;
    }

    lv_obj_update_layout(both_info->tip_img);
    tip_x = lv_obj_get_x(both_info->tip_img);
    tip_y = lv_obj_get_y(both_info->tip_img);
    lv_obj_set_pos(both_info->tip_shine,
        tip_x - both_info->tip_mask_x, tip_y - HOME_BOTH_LEVEL_TIP_MASK_Y);
}

#if HOME_BOTH_LEVEL_TIP_SHINE_USE_IMG_MASK
static lv_draw_buf_t *home_both_level_tip_mask_a8_from_rgba(const lv_draw_buf_t *rgba,
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
#if HOME_BOTH_LEVEL_TIP_SHINE_IS_SOFT
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

static void home_both_level_tip_mask_a8_release(void)
{
    if (both_info == NULL || both_info->tip_mask_a8 == NULL) {
        return;
    }
    if (both_info->tip_mask != NULL && lv_obj_is_valid(both_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(both_info->tip_mask, NULL, 0);
    }
    lv_draw_buf_destroy(both_info->tip_mask_a8);
    both_info->tip_mask_a8 = NULL;
}

static void home_both_level_tip_mask_a8_prepare(void)
{
    const lv_draw_buf_t *rgba;

    if (both_info->tip_mask_a8 != NULL) {
        if (both_info->tip_mask != NULL && lv_obj_is_valid(both_info->tip_mask)) {
            lv_obj_set_style_bitmap_mask_src(both_info->tip_mask, both_info->tip_mask_a8, 0);
        }
        return;
    }

    rgba = get_predecoded_text_shine_mask();
    both_info->tip_mask_a8 = home_both_level_tip_mask_a8_from_rgba(rgba,
        HOME_BOTH_LEVEL_TIP_MASK_W, HOME_BOTH_LEVEL_TIP_MASK_H);
    if (both_info->tip_mask_a8 != NULL && both_info->tip_mask != NULL &&
        lv_obj_is_valid(both_info->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(both_info->tip_mask, both_info->tip_mask_a8, 0);
    }
}
#endif /* HOME_BOTH_LEVEL_TIP_SHINE_USE_IMG_MASK */

static lv_draw_buf_t *home_both_level_tip_predecode(void)
{
    const ui_img_data_t *tip;

    if (both_info->tip_buf != NULL) {
        return both_info->tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    both_info->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return both_info->tip_buf;
}

static void home_both_level_tip_rebuild(void)
{
    lv_draw_buf_t *predecoded;

    if (both_info->tip_img != NULL && lv_obj_is_valid(both_info->tip_img) &&
        both_info->tip_mask != NULL && lv_obj_is_valid(both_info->tip_mask) &&
        both_info->tip_shine != NULL && lv_obj_is_valid(both_info->tip_shine)) {
        home_both_level_tip_shine_sync();
        return;
    }

    predecoded = home_both_level_tip_predecode();

    if (both_info->tip_img == NULL || !lv_obj_is_valid(both_info->tip_img)) {
        both_info->tip_img = lv_image_create(both_info->root);
        EXCEPTION_CHECK_VOID(both_info->tip_img);
        lv_obj_clear_flag(both_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(both_info->tip_img, predecoded);
            both_info->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(both_info->tip_img, HOME_BOTH_LEVEL_TIP_BASE_OPA, 0);
        lv_obj_align(both_info->tip_img, LV_ALIGN_TOP_MID, 0, HOME_BOTH_LEVEL_TIP_Y);
    }

    if (both_info->tip_mask == NULL || !lv_obj_is_valid(both_info->tip_mask)) {
        both_info->tip_mask = lv_obj_create(both_info->root);
        EXCEPTION_CHECK_VOID(both_info->tip_mask);
        lv_obj_remove_style_all(both_info->tip_mask);
        lv_obj_set_size(both_info->tip_mask, HOME_BOTH_LEVEL_TIP_MASK_W, HOME_BOTH_LEVEL_TIP_MASK_H);
        lv_obj_set_style_bg_opa(both_info->tip_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(both_info->tip_mask, 0, 0);
        lv_obj_set_style_pad_all(both_info->tip_mask, 0, 0);
        lv_obj_clear_flag(both_info->tip_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(both_info->tip_mask, LV_OBJ_FLAG_CLICKABLE);
        both_info->tip_mask_x = HOME_BOTH_LEVEL_TIP_MASK_X_MIN;
        both_info->tip_mask_tick_ms = get_timestamp();
    }

    if (both_info->tip_shine == NULL || !lv_obj_is_valid(both_info->tip_shine)) {
        both_info->tip_shine = lv_image_create(both_info->tip_mask);
        EXCEPTION_CHECK_VOID(both_info->tip_shine);
        lv_obj_clear_flag(both_info->tip_shine, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(both_info->tip_shine, predecoded);
        }
        lv_obj_set_style_image_recolor(both_info->tip_shine,
            lv_color_hex(HOME_BOTH_LEVEL_TIP_SHINE_COLOR), 0);
        lv_obj_set_style_image_recolor_opa(both_info->tip_shine,
            HOME_BOTH_LEVEL_TIP_SHINE_RECOLOR_OPA, 0);
        lv_obj_set_style_blend_mode(both_info->tip_shine, LV_BLEND_MODE_NORMAL, 0);
    }

#if HOME_BOTH_LEVEL_TIP_SHINE_USE_IMG_MASK
    home_both_level_tip_mask_a8_prepare();
#endif
    lv_obj_align(both_info->tip_mask, LV_ALIGN_TOP_LEFT,
        both_info->tip_mask_x, HOME_BOTH_LEVEL_TIP_MASK_Y);
    lv_obj_move_foreground(both_info->tip_img);
    lv_obj_move_foreground(both_info->tip_mask);
    home_both_level_tip_shine_sync();
}

static void home_both_level_tip_mask_anim_update(void)
{
    uint32_t elapsed;
    int32_t x;

    if (both_info->tip_mask == NULL || !lv_obj_is_valid(both_info->tip_mask)) {
        return;
    }

    elapsed = get_timestamp() - both_info->tip_mask_tick_ms;
    if (elapsed >= HOME_BOTH_LEVEL_TIP_MASK_PERIOD_MS) {
        both_info->tip_mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = HOME_BOTH_LEVEL_TIP_MASK_X_MIN +
        (int32_t)((HOME_BOTH_LEVEL_TIP_MASK_X_MAX - HOME_BOTH_LEVEL_TIP_MASK_X_MIN) * elapsed) /
        HOME_BOTH_LEVEL_TIP_MASK_PERIOD_MS;
    if (x != both_info->tip_mask_x) {
        both_info->tip_mask_x = (int16_t)x;
        lv_obj_set_x(both_info->tip_mask, both_info->tip_mask_x);
        home_both_level_tip_shine_sync();
    }
}

static void home_both_level_tip_update(void)
{
    if (both_info->force_update) {
        home_both_level_tip_rebuild();
    }
    home_both_level_tip_mask_anim_update();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
uint8_t home_both_level_adjust_ui_level_max_get(void)
{
    if (both_info == NULL) {
        return BREAST_PUMP_LEVEL_15;
    }
    return both_info->level_max;
}

uint32_t home_both_level_adjust_over_limit_color_get(uint8_t pump_mode)
{
    switch (pump_mode) {
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return HOME_BOTH_LEVEL_OVER_LIMIT_RHYTHM_COLOR;
        case BREAST_PUMP_STIMULATION_MODE:
            return HOME_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR;
        case BREAST_PUMP_LACTATION_MODE:
            return HOME_BOTH_LEVEL_OVER_LIMIT_LACTATION_COLOR;
        default:
            return HOME_BOTH_LEVEL_OVER_LIMIT_STIM_COLOR;
    }
}

void ui_display_home_both_level_adjust_prepare(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(thiz);
    if (both_info == NULL) {
        both_info = (home_both_level_adjust_ui_info_t *)u_malloz(
            sizeof(home_both_level_adjust_ui_info_t));
        EXCEPTION_CHECK_VOID(both_info);
    }
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    both_info->pump_mode = (uint8_t)pump_mode;
    both_info->level_max = home_level_adjust_max_get(both_info->pump_mode);
    both_info->accent_color = home_level_adjust_accent_color_get(both_info->pump_mode);
    both_info->over_limit_color =
        home_both_level_adjust_over_limit_color_get(both_info->pump_mode);
    LOG_UI("both level adjust prepare mode=%d max=%d color=0x%06X",
        both_info->pump_mode, both_info->level_max, both_info->accent_color);

    both_info->force_update = true;
    both_info->shown_level[0] = 0xFF;
    both_info->shown_level[1] = 0xFF;
    both_info->shown_over_limit = 0xFF;
    both_info->shown_accent_color = 0;
    both_info->shown_over_limit_color = 0;
    home_both_level_adjust_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_home_both_level_adjust_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && both_info && both_info->root);
    if (both_info->tip_shine != NULL && lv_obj_is_valid(both_info->tip_shine)) {
        lv_image_set_src(both_info->tip_shine, NULL);
        LVGL_OBJ_DELETE(both_info->tip_shine);
        both_info->tip_shine = NULL;
    }
#if HOME_BOTH_LEVEL_TIP_SHINE_USE_IMG_MASK
    home_both_level_tip_mask_a8_release();
#endif
    if (both_info->tip_mask != NULL && lv_obj_is_valid(both_info->tip_mask)) {
        LVGL_OBJ_DELETE(both_info->tip_mask);
        both_info->tip_mask = NULL;
    }
    if (both_info->tip_img != NULL && lv_obj_is_valid(both_info->tip_img)) {
        lv_image_set_src(both_info->tip_img, NULL);
    }
    if (both_info->tip_buf != NULL) {
        lv_draw_buf_destroy(both_info->tip_buf);
        both_info->tip_buf = NULL;
    }
    both_info->tip_predecoded = 0;
    LVGL_OBJ_DELETE(both_info->root);
    u_free(both_info);
    both_info = NULL;
}

int ui_display_home_both_level_adjust_process(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;

    EXCEPTION_CHECK_INT(thiz && both_info && both_info->root);
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump);

    home_both_level_adjust_bg_update(thiz);
    home_both_level_arc_update(proc, both_info->level_max, both_info->accent_color);
    home_both_level_value_update(proc);
    home_both_level_dir_update();
    home_both_level_over_limit_update(proc, both_info->over_limit_color);
    home_both_level_tip_update();
    both_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home both level adjust fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
