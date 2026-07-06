/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_HOME_BASE_TIME_BLINK_ENABLE             (0) /// 0: 关闭时间闪烁, 1: 开启时间闪烁.
#define PUMP_TIME_COLOR                              (0xBD4060)
#define PUMP_TIME_2SEG                               (72)  /// 两段计时 MM:SS
#define PUMP_TIME_3SEG                               (60)  /// 三段计时 HH:MM:SS

#define PUMP_TIME_3SEG_H_X                           (63)
#define PUMP_TIME_3SEG_H_Y                           (185)
#define PUMP_TIME_3SEG_M_X                           (147)
#define PUMP_TIME_3SEG_M_Y                           (PUMP_TIME_3SEG_H_Y)
#define PUMP_TIME_3SEG_S_X                           (231)
#define PUMP_TIME_3SEG_S_Y                           (PUMP_TIME_3SEG_M_Y)
#define PUMP_TIME_3SEG_COL_H_X                       (133 - 5)
#define PUMP_TIME_3SEG_COL_H_Y                       (180)
#define PUMP_TIME_3SEG_COL_M_X                       (217 - 5)
#define PUMP_TIME_3SEG_COL_M_Y                       (PUMP_TIME_3SEG_COL_H_Y)
#define PUMP_TIME_3SEG_WIDTH                         (65)

#define PUMP_TIME_CENTER_OFFSET                      (13)
#define PUMP_TIME_2SEG_WIDTH                         (86)
#define PUMP_TIME_2SEG_M_X                           (180 - PUMP_TIME_2SEG_WIDTH - PUMP_TIME_CENTER_OFFSET)
#define PUMP_TIME_2SEG_M_Y                           (192)
#define PUMP_TIME_2SEG_S_X                           (180 + PUMP_TIME_CENTER_OFFSET)
#define PUMP_TIME_2SEG_S_Y                           (PUMP_TIME_2SEG_M_Y)
#define PUMP_TIME_2SEG_COL_X                         (0)
#define PUMP_TIME_2SEG_COL_Y                         (186)


#define PUMP_TIME_REMAIN_Y                           (155)
#define PUMP_TIME_REMAIN_MINUS_X                     (275)
#define PUMP_TIME_REMAIN_MINUS_M_GAP                 (2)
#define PUMP_TIME_REMAIN_DIGIT_WIDTH                 (24)

#define PUMP_TIME_Y_OFFSET_OTHER                     (0)   /// 常规模式（当前坐标基准）
#define PUMP_TIME_Y_OFFSET_REMAIN                    (14)  /// 有剩余时长模式

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool pump_home_time_need_remain_text(uint8_t mode_index)
{
    return mode_index == BREAST_PUMP_CUSTOM_MODE ||
        mode_index == BREAST_PUMP_MILK_BOOST_MODE ||
        mode_index == BREAST_PUMP_PP_MILK_TRACK_MODE;
}

static int8_t pump_home_time_y_offset_get(uint8_t mode_index)
{
    return pump_home_time_need_remain_text(mode_index) ?
        PUMP_TIME_Y_OFFSET_REMAIN : PUMP_TIME_Y_OFFSET_OTHER;
}

static void pump_home_time_symbol_label_update(lv_obj_t **label, lv_obj_t *root, const char *text,
    const lv_font_t *font, lv_coord_t x, lv_coord_t y, bool hidden)
{
    if (hidden) {
        if (*label != NULL && lv_obj_is_valid(*label)) {
            lv_obj_add_flag(*label, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(root);
        EXCEPTION_CHECK_VOID(*label);
        lv_label_set_long_mode(*label, LV_LABEL_LONG_CLIP);
    }

    lv_obj_set_width(*label, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_letter_space(*label, 0, 0);
    lv_obj_set_style_text_color(*label, lv_color_hex(PUMP_TIME_COLOR), 0);
    lv_obj_set_style_text_font(*label, font, 0);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_clear_flag(*label, LV_OBJ_FLAG_HIDDEN);
}

static void pump_home_time_cell_labels_set_hidden(lv_obj_t *labels[], int count, bool hidden)
{
    for (int i = 0; i < count; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            continue;
        }
        if (hidden) {
            lv_obj_add_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void pump_home_time_pair_labels_set_hidden(lv_obj_t *labels[PUMP_HOME_TIME_DIGIT_COUNT], bool hidden)
{
    pump_home_time_cell_labels_set_hidden(labels, PUMP_HOME_TIME_DIGIT_COUNT, hidden);
}

static void pump_home_time_symbol_label_set_hidden(lv_obj_t **label, bool hidden)
{
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        return;
    }
    if (hidden) {
        lv_obj_add_flag(*label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(*label, LV_OBJ_FLAG_HIDDEN);
    }
}

static void pump_home_time_pair_labels_update(lv_obj_t *labels[PUMP_HOME_TIME_DIGIT_COUNT], lv_obj_t *root,
    uint8_t value, const lv_font_t *font, lv_coord_t seg_x, lv_coord_t seg_y, lv_coord_t seg_width,
    int8_t y_offset, bool hidden)
{
    if (hidden) {
        pump_home_time_pair_labels_set_hidden(labels, true);
        return;
    }

    lv_coord_t cell_width = seg_width / PUMP_HOME_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};

    for (int i = 0; i < PUMP_HOME_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(root);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
        }

        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_width);
        lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        lv_obj_set_style_text_color(labels[i], lv_color_hex(PUMP_TIME_COLOR), 0);
        lv_obj_set_style_text_font(labels[i], font, 0);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_width, seg_y + y_offset);
        lv_obj_clear_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void pump_home_time_remain_set_hidden(pump_home_base_time_t *time, bool hidden)
{
    pump_home_time_symbol_label_set_hidden(&time->remain_minus, hidden);
    pump_home_time_pair_labels_set_hidden(time->remain_m, hidden);
    pump_home_time_symbol_label_set_hidden(&time->remain_colon, hidden);
    pump_home_time_pair_labels_set_hidden(time->remain_s, hidden);
}

static void pump_home_time_remain_update(pump_home_base_ui_info_t *info, uint8_t mode_index)
{
    pump_home_base_time_t *time = &info->time;
    bool hidden = !pump_home_time_need_remain_text(mode_index);

    if (hidden) {
        pump_home_time_remain_set_hidden(time, true);
        time->remain_time_shown = UINT32_MAX;
        return;
    }

    if (time->remain_time_shown == time->remain_time && info->force_update == false) {
        return;
    }

    uint32_t remain_seconds = time->remain_time;
    uint8_t minutes = remain_seconds / 60;
    uint8_t seconds = remain_seconds % 60;
    const lv_font_t *font = &lv_font_aeonik_soft_18_medium;
    lv_coord_t x = PUMP_TIME_REMAIN_MINUS_X;

    pump_home_time_symbol_label_update(&time->remain_minus, info->root, "-", font,
        x, PUMP_TIME_REMAIN_Y + info->mode.select_remain_y_offset, false);
    lv_obj_update_layout(time->remain_minus);
    x += lv_obj_get_width(time->remain_minus) + PUMP_TIME_REMAIN_MINUS_M_GAP;

    pump_home_time_pair_labels_update(time->remain_m, info->root, minutes, font,
        x, PUMP_TIME_REMAIN_Y + info->mode.select_remain_y_offset, PUMP_TIME_REMAIN_DIGIT_WIDTH, 0, false);
    x += PUMP_TIME_REMAIN_DIGIT_WIDTH + 1;

    pump_home_time_symbol_label_update(&time->remain_colon, info->root, ":", font,
        x, PUMP_TIME_REMAIN_Y + info->mode.select_remain_y_offset, false);
    lv_obj_update_layout(time->remain_colon);
    x += lv_obj_get_width(time->remain_colon) + 1;

    pump_home_time_pair_labels_update(time->remain_s, info->root, seconds, font,
        x, PUMP_TIME_REMAIN_Y + info->mode.select_remain_y_offset, PUMP_TIME_REMAIN_DIGIT_WIDTH, 0, false);
    time->remain_time_shown = time->remain_time;
}

static void pump_home_time_colon_update(pump_home_base_ui_info_t *info, uint8_t digit_size, int8_t y_offset)
{
    pump_home_base_time_t *time = &info->time;

    if (time->digit_size == digit_size && time->y_offset == y_offset &&
        info->force_update == false) {
        return;
    }

    char colon_value[4] = {0};
    for (int i = 0; i < HOME_TIME_COLON_COUNT; i++) {
        if (time->colon[i] == NULL || !lv_obj_is_valid(time->colon[i])) {
            time->colon[i] = lv_label_create(info->root);
            EXCEPTION_CHECK_VOID(time->colon[i]);
            lv_obj_set_style_text_align(time->colon[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(time->colon[i], lv_color_hex(PUMP_TIME_COLOR), 0);
            snprintf(colon_value, sizeof(colon_value), "%s", ":");
            lv_label_set_text(time->colon[i], colon_value);
        }

        if (digit_size == PUMP_TIME_2SEG) {
            if (i == 0) {
                lv_obj_add_flag(time->colon[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_set_style_text_font(time->colon[i], &lv_font_quicksand_72_bold, 0);
                lv_obj_align(time->colon[i], LV_ALIGN_TOP_MID, 0, PUMP_TIME_2SEG_COL_Y + y_offset);
            }
        } else {
            if (i == 0) {
                lv_obj_clear_flag(time->colon[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(time->colon[i], &lv_font_quicksand_60_bold, 0);
                lv_obj_align(time->colon[i], LV_ALIGN_TOP_LEFT,
                    PUMP_TIME_3SEG_COL_H_X, PUMP_TIME_3SEG_COL_H_Y + y_offset);
            } else {
                lv_obj_set_style_text_font(time->colon[i], &lv_font_quicksand_60_bold, 0);
                lv_obj_align(time->colon[i], LV_ALIGN_TOP_LEFT,
                    PUMP_TIME_3SEG_COL_M_X, PUMP_TIME_3SEG_COL_M_Y + y_offset);
            }
        }
    }
}

static void pump_home_time_h_update(pump_home_base_ui_info_t *info, uint8_t hours, uint8_t digit_size, int8_t y_offset)
{
    pump_home_base_time_t *time = &info->time;

    if (time->hours == hours && info->force_update == false &&
        time->digit_size == digit_size && time->y_offset == y_offset) {
        return;
    }

    pump_home_time_pair_labels_update(time->h, info->root, hours, &lv_font_quicksand_60_bold,
        PUMP_TIME_3SEG_H_X, PUMP_TIME_3SEG_H_Y, PUMP_TIME_3SEG_WIDTH, y_offset,
        digit_size == PUMP_TIME_2SEG);
    time->hours = hours;
}

static void pump_home_time_m_update(pump_home_base_ui_info_t *info, uint8_t minutes,
    uint8_t digit_size, int8_t y_offset)
{
    pump_home_base_time_t *time = &info->time;

    if (time->minutes == minutes && info->force_update == false &&
        time->digit_size == digit_size && time->y_offset == y_offset) {
        return;
    }

    if (digit_size == PUMP_TIME_2SEG) {
        pump_home_time_pair_labels_update(time->m, info->root, minutes, &lv_font_quicksand_72_bold,
            PUMP_TIME_2SEG_M_X, PUMP_TIME_2SEG_M_Y, PUMP_TIME_2SEG_WIDTH, y_offset, false);
    } else {
        pump_home_time_pair_labels_update(time->m, info->root, minutes, &lv_font_quicksand_60_bold,
            PUMP_TIME_3SEG_M_X, PUMP_TIME_3SEG_M_Y, PUMP_TIME_3SEG_WIDTH, y_offset, false);
    }
    time->minutes = minutes;
}

static void pump_home_time_s_update(pump_home_base_ui_info_t *info, uint8_t seconds,
    uint8_t digit_size, int8_t y_offset)
{
    pump_home_base_time_t *time = &info->time;

    if (time->seconds == seconds && info->force_update == false &&
        time->digit_size == digit_size && time->y_offset == y_offset) {
        return;
    }

    if (digit_size == PUMP_TIME_2SEG) {
        pump_home_time_pair_labels_update(time->s, info->root, seconds, &lv_font_quicksand_72_bold,
            PUMP_TIME_2SEG_S_X, PUMP_TIME_2SEG_S_Y, PUMP_TIME_2SEG_WIDTH, y_offset, false);
    } else {
        pump_home_time_pair_labels_update(time->s, info->root, seconds, &lv_font_quicksand_60_bold,
            PUMP_TIME_3SEG_S_X, PUMP_TIME_3SEG_S_Y, PUMP_TIME_3SEG_WIDTH, y_offset, false);
    }
    time->seconds = seconds;
}

#if PUMP_HOME_BASE_TIME_BLINK_ENABLE
static void pump_home_time_blink_apply(pump_home_base_ui_info_t *info, bool blink_visible)
{
    pump_home_base_time_t *time = &info->time;
    const bool blink_hidden = (time->time_blink_state != 0) && !blink_visible;

    if (time->digit_size == PUMP_TIME_3SEG) {
        pump_home_time_pair_labels_set_hidden(time->h, blink_hidden);
    }
    pump_home_time_pair_labels_set_hidden(time->m, blink_hidden);
    pump_home_time_pair_labels_set_hidden(time->s, blink_hidden);

    for (int i = 0; i < HOME_TIME_COLON_COUNT; i++) {
        if (blink_hidden) {
            pump_home_time_symbol_label_set_hidden(&time->colon[i], true);
            continue;
        }
        const bool colon_hidden = (time->digit_size == PUMP_TIME_2SEG && i == 0);
        pump_home_time_symbol_label_set_hidden(&time->colon[i], colon_hidden);
    }
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_time_apply_select_offset(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    info->time.force_realign = 1;
}

void pump_home_base_time_ui_remove(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
}

void pump_home_base_time_ui_update(pump_home_base_ui_info_t *info, uint16_t hold_time, bool blink_visible)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->time.force_realign) {
        info->time.hours = UINT8_MAX;
        info->time.minutes = UINT8_MAX;
        info->time.seconds = UINT8_MAX;
        info->time.digit_size = 0;
        info->time.y_offset = INT8_MAX;
        info->time.remain_time_shown = UINT32_MAX;
        info->time.force_realign = 0;
    }
    int8_t y_offset = pump_home_time_y_offset_get(info->mode.new_index) +
        info->mode.select_clock_y_offset;
    uint8_t hours = hold_time / 3600;
    uint8_t minutes = (hold_time / 60) % 60;
    uint8_t seconds = hold_time % 60;
    // hours = minutes = seconds;
    uint8_t digit_size = (hours == 0) ? PUMP_TIME_2SEG : PUMP_TIME_3SEG;
    pump_home_time_h_update(info, hours, digit_size, y_offset);
    pump_home_time_m_update(info, minutes, digit_size, y_offset);
    pump_home_time_s_update(info, seconds, digit_size, y_offset);
    pump_home_time_colon_update(info, digit_size, y_offset);
    info->time.digit_size = digit_size;
    info->time.y_offset = y_offset;
    pump_home_time_remain_update(info, info->mode.new_index);
#if PUMP_HOME_BASE_TIME_BLINK_ENABLE
    pump_home_time_blink_apply(info, blink_visible);
#endif
}
