/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "pump_speed_adjust_ui.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_SPEED_ADJUST_COMP_BG        (0) /// 是否启用对比背景图
#define IS_SUPPORT_PUMP_SPEED_TIME_3SEG_TEST        (0)

/** 速度档位圆弧：与 HOME 速度调节页一致 */
#define PUMP_SPEED_ARC_SIZE                         (352)
#define PUMP_SPEED_ARC_WIDTH                        (11)
#define PUMP_SPEED_ARC_SEG_COUNT                    (5)
#define PUMP_SPEED_ARC_GAP_DEG                      (8)
#define PUMP_SPEED_ARC_SEG_DEG                      (26)
#if IS_SUPPORT_PUMP_SPEED_ADJUST_COMP_BG
#define PUMP_SPEED_ARC_REMAIN_COLOR                 (0xFFFFFF)
#else
#define PUMP_SPEED_ARC_REMAIN_COLOR                 (0x252525)
#endif
#define PUMP_SPEED_ARC_TRACK_DEG \
    (PUMP_SPEED_ARC_SEG_COUNT * PUMP_SPEED_ARC_SEG_DEG + \
     (PUMP_SPEED_ARC_SEG_COUNT - 1) * PUMP_SPEED_ARC_GAP_DEG)
#define PUMP_SPEED_ARC_BOTTOM_GAP_DEG \
    (360 - PUMP_SPEED_ARC_TRACK_DEG)
#define PUMP_SPEED_ARC_ROTATION \
    (90 + (PUMP_SPEED_ARC_BOTTOM_GAP_DEG / 2))

#define PUMP_SPEED_VALUE_Y                          (127)
#define PUMP_SPEED_VALUE_COLOR                      (0xFFFFFF)

#define PUMP_SPEED_LABEL_Y                          (210)
#define PUMP_SPEED_LABEL_COLOR                      (0x999999)
#define PUMP_SPEED_LABEL_TEXT                       "Speed"

/** 泵奶时间组件参数与 pump_level_adjust_ui 一致 */
#define PUMP_SPEED_TIME_DIGIT_COUNT                 (2)
#define PUMP_SPEED_TIME_COLON_COUNT                 (2)
#define PUMP_SPEED_TIME_LABEL_Y                     (293)
#define PUMP_SPEED_TIME_ICON_Y                      (311)
#define PUMP_SPEED_TIME_Y                           (308)
#define PUMP_SPEED_TIME_SEG_WIDTH                   (36)
#define PUMP_SPEED_TIME_COLON_GAP                   (2)
#define PUMP_SPEED_TIME_COLOR                       (0xFFFFFF)
#define PUMP_SPEED_TIME_2SEG                        (2)
#define PUMP_SPEED_TIME_3SEG                        (3)
#define PUMP_SPEED_TIME_2SEG_ICON_X                 (133)
#define PUMP_SPEED_TIME_2SEG_M_X                    (168)
#define PUMP_SPEED_TIME_COLON_W_FALLBACK            (8)

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_PUMP_SPEED_ADJUST_COMP_BG
    lv_obj_t *background;
#endif
    ui_manager_context_t *ui;
    lv_obj_t *arcs[PUMP_SPEED_ARC_SEG_COUNT];
    lv_obj_t *value_label;
    lv_obj_t *speed_label;
    lv_obj_t *time_label;
    lv_obj_t *status_icon;
    lv_obj_t *time_h[PUMP_SPEED_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[PUMP_SPEED_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_SPEED_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[PUMP_SPEED_TIME_COLON_COUNT];
#if IS_SUPPORT_PUMP_SPEED_TIME_3SEG_TEST
    uint8_t test_hours;
#endif
    uint8_t shown_speed;
    uint8_t shown_selected;
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;
    uint8_t shown_paused;
    uint8_t force_update;
    uint8_t pump_mode;
    uint16_t shown_status_icon_id;
    uint32_t accent_color;
    uint32_t shown_accent_color;
} pump_speed_adjust_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_SPEED_ADJUST_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t pump_speed_adjust_bg_data[] = {
    #include "comparison_png2txt/speed_adjust12_352x352.txt"
};
#endif

static pump_speed_adjust_ui_info_t *speed_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_speed_adjust_root_init(void)
{
    if (speed_info->root == NULL || !lv_obj_is_valid(speed_info->root)) {
        speed_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(speed_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(speed_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(speed_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(speed_info->root, 0, 0);
    lv_obj_set_style_pad_all(speed_info->root, 0, 0);
    lv_obj_align(speed_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_PUMP_SPEED_ADJUST_COMP_BG
static void pump_speed_adjust_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (speed_info->background == NULL || !lv_obj_is_valid(speed_info->background)) {
        speed_info->background = lv_image_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->background);
        lv_obj_move_background(speed_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(speed_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(speed_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER,
            pump_speed_adjust_bg_data, sizeof(pump_speed_adjust_bg_data), "png");
        lv_image_set_src(speed_info->background, (const void *)&bg_img_path);
        lv_obj_align(speed_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_speed_adjust_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static void pump_speed_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, PUMP_SPEED_ARC_SIZE, PUMP_SPEED_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, PUMP_SPEED_ARC_ROTATION);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, PUMP_SPEED_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, PUMP_SPEED_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

static uint8_t pump_speed_selected_count_get(uint8_t speed)
{
    uint8_t selected = speed + 1;

    if (selected > PUMP_SPEED_ARC_SEG_COUNT) {
        selected = PUMP_SPEED_ARC_SEG_COUNT;
    }
    if (selected > BREAST_PUMP_SPEED_MAX) {
        selected = BREAST_PUMP_SPEED_MAX;
    }
    return selected;
}

static void pump_speed_arc_update(ui_proc_pump_t *proc, uint32_t accent_color)
{
    uint8_t i;
    uint8_t selected;
    bool need_update;

    selected = pump_speed_selected_count_get(proc->setting.speed);
    need_update = (speed_info->force_update ||
        speed_info->shown_accent_color != accent_color ||
        speed_info->shown_selected != selected);

    for (i = 0; i < PUMP_SPEED_ARC_SEG_COUNT; i++) {
        uint16_t start = (uint16_t)(i * (PUMP_SPEED_ARC_SEG_DEG + PUMP_SPEED_ARC_GAP_DEG));
        uint16_t end = (uint16_t)(start + PUMP_SPEED_ARC_SEG_DEG);
        uint32_t color = ((i + 1) == selected) ? accent_color : PUMP_SPEED_ARC_REMAIN_COLOR;
        bool created = false;

        if (speed_info->arcs[i] == NULL || !lv_obj_is_valid(speed_info->arcs[i])) {
            speed_info->arcs[i] = lv_arc_create(speed_info->root);
            EXCEPTION_CHECK_VOID(speed_info->arcs[i]);
            pump_speed_arc_style_init(speed_info->arcs[i], color);
            created = true;
        }

        if (created || need_update) {
            lv_obj_set_style_arc_color(speed_info->arcs[i], lv_color_hex(color),
                LV_PART_INDICATOR);
            lv_arc_set_angles(speed_info->arcs[i], start, end);
            lv_obj_invalidate(speed_info->arcs[i]);
        }
    }

    speed_info->shown_selected = selected;
    speed_info->shown_accent_color = accent_color;
}

static void pump_speed_value_update(ui_proc_pump_t *proc)
{
    char num_buf[8] = {0};
    uint8_t display_speed = proc->setting.speed + 1;

    if (speed_info->value_label == NULL || !lv_obj_is_valid(speed_info->value_label)) {
        speed_info->value_label = lv_label_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->value_label);
        speed_info->shown_speed = 0xFF;
    }
    if (speed_info->force_update == false && speed_info->shown_speed == proc->setting.speed) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), "%u", display_speed);
    lv_label_set_text(speed_info->value_label, num_buf);
    lv_obj_set_style_text_color(speed_info->value_label,
        lv_color_hex(PUMP_SPEED_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(speed_info->value_label, &lv_font_aeonik_soft_69_semibold, 0);
    lv_obj_set_style_text_align(speed_info->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(speed_info->value_label, LV_ALIGN_TOP_MID, 0, PUMP_SPEED_VALUE_Y);
    speed_info->shown_speed = proc->setting.speed;
}

static void pump_speed_label_update(void)
{
    if (speed_info->speed_label != NULL && lv_obj_is_valid(speed_info->speed_label) &&
        speed_info->force_update == false) {
        return;
    }

    if (speed_info->speed_label == NULL || !lv_obj_is_valid(speed_info->speed_label)) {
        speed_info->speed_label = lv_label_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->speed_label);
    }

    lv_label_set_text(speed_info->speed_label, PUMP_SPEED_LABEL_TEXT);
    lv_obj_set_style_text_color(speed_info->speed_label,
        lv_color_hex(PUMP_SPEED_LABEL_COLOR), 0);
    lv_obj_set_style_text_font(speed_info->speed_label, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(speed_info->speed_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(speed_info->speed_label, LV_ALIGN_TOP_MID, 0, PUMP_SPEED_LABEL_Y);
}

static void pump_speed_obj_flag_set(lv_obj_t *obj, bool visible)
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

static void pump_speed_time_pair_update(lv_obj_t *labels[PUMP_SPEED_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x, lv_coord_t seg_y, bool hidden)
{
    const lv_coord_t cell_w = PUMP_SPEED_TIME_SEG_WIDTH / PUMP_SPEED_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < PUMP_SPEED_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(PUMP_SPEED_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
        pump_speed_obj_flag_set(labels[i], !hidden);
    }
}

static lv_coord_t pump_speed_time_colon_w_get(void)
{
    lv_coord_t colon_w = PUMP_SPEED_TIME_COLON_W_FALLBACK;
    lv_obj_t *colon = speed_info->time_colon[1];

    if (colon == NULL || !lv_obj_is_valid(colon)) {
        colon = speed_info->time_colon[0];
    }
    if (colon != NULL && lv_obj_is_valid(colon)) {
        lv_obj_update_layout(colon);
        colon_w = lv_obj_get_width(colon);
        if (colon_w <= 0) {
            colon_w = PUMP_SPEED_TIME_COLON_W_FALLBACK;
        }
    }
    return colon_w;
}

static lv_coord_t pump_speed_time_unit_span_get(void)
{
    return PUMP_SPEED_TIME_SEG_WIDTH + PUMP_SPEED_TIME_COLON_GAP +
        pump_speed_time_colon_w_get() + PUMP_SPEED_TIME_COLON_GAP;
}

static void pump_speed_time_colon_ensure(void)
{
    int i;

    for (i = 0; i < PUMP_SPEED_TIME_COLON_COUNT; i++) {
        if (speed_info->time_colon[i] == NULL || !lv_obj_is_valid(speed_info->time_colon[i])) {
            speed_info->time_colon[i] = lv_label_create(speed_info->root);
            EXCEPTION_CHECK_VOID(speed_info->time_colon[i]);
            lv_obj_set_style_text_color(speed_info->time_colon[i],
                lv_color_hex(PUMP_SPEED_TIME_COLOR), 0);
            lv_obj_set_style_text_font(speed_info->time_colon[i], &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_align(speed_info->time_colon[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_text(speed_info->time_colon[i], ":");
        }
    }
}

static void pump_speed_time_layout_apply(uint8_t hours, uint8_t minutes, uint8_t seconds,
    uint8_t digit_size, bool force)
{
    const lv_coord_t unit_span = pump_speed_time_unit_span_get();
    const lv_coord_t colon_w = pump_speed_time_colon_w_get();
    const lv_coord_t m_x = PUMP_SPEED_TIME_2SEG_M_X;
    const lv_coord_t colon_ms_x = m_x + PUMP_SPEED_TIME_SEG_WIDTH + PUMP_SPEED_TIME_COLON_GAP;
    const lv_coord_t s_x = colon_ms_x + colon_w + PUMP_SPEED_TIME_COLON_GAP;
    lv_coord_t icon_x = PUMP_SPEED_TIME_2SEG_ICON_X;
    lv_coord_t h_x = m_x;
    lv_coord_t colon_hm_x = colon_ms_x;
    const bool is_3seg = (digit_size == PUMP_SPEED_TIME_3SEG);
    int i;

    if (is_3seg) {
        h_x = m_x - unit_span;
        colon_hm_x = h_x + PUMP_SPEED_TIME_SEG_WIDTH + PUMP_SPEED_TIME_COLON_GAP;
        icon_x = PUMP_SPEED_TIME_2SEG_ICON_X - unit_span;
    }

    if (speed_info->status_icon != NULL && lv_obj_is_valid(speed_info->status_icon)) {
        lv_obj_align(speed_info->status_icon, LV_ALIGN_TOP_LEFT, icon_x, PUMP_SPEED_TIME_ICON_Y);
    }

    if (force || speed_info->shown_hours != hours || speed_info->shown_digit_size != digit_size) {
        pump_speed_time_pair_update(speed_info->time_h, speed_info->root, hours, h_x,
            PUMP_SPEED_TIME_Y, !is_3seg);
        speed_info->shown_hours = hours;
    }
    if (force || speed_info->shown_minutes != minutes || speed_info->shown_digit_size != digit_size) {
        pump_speed_time_pair_update(speed_info->time_m, speed_info->root, minutes, m_x,
            PUMP_SPEED_TIME_Y, false);
        speed_info->shown_minutes = minutes;
    }
    if (force || speed_info->shown_seconds != seconds || speed_info->shown_digit_size != digit_size) {
        pump_speed_time_pair_update(speed_info->time_s, speed_info->root, seconds, s_x,
            PUMP_SPEED_TIME_Y, false);
        speed_info->shown_seconds = seconds;
    }

    if (speed_info->time_colon[0] != NULL && lv_obj_is_valid(speed_info->time_colon[0])) {
        lv_obj_align(speed_info->time_colon[0], LV_ALIGN_TOP_LEFT, colon_hm_x, PUMP_SPEED_TIME_Y);
        pump_speed_obj_flag_set(speed_info->time_colon[0], is_3seg);
    }
    if (speed_info->time_colon[1] != NULL && lv_obj_is_valid(speed_info->time_colon[1])) {
        lv_obj_align(speed_info->time_colon[1], LV_ALIGN_TOP_LEFT, colon_ms_x, PUMP_SPEED_TIME_Y);
        pump_speed_obj_flag_set(speed_info->time_colon[1], true);
    }
    speed_info->shown_digit_size = digit_size;

    if (speed_info->time_label != NULL && lv_obj_is_valid(speed_info->time_label)) {
        lv_obj_move_foreground(speed_info->time_label);
    }
    if (speed_info->status_icon != NULL && lv_obj_is_valid(speed_info->status_icon)) {
        lv_obj_move_foreground(speed_info->status_icon);
    }
    for (i = 0; i < PUMP_SPEED_TIME_COLON_COUNT; i++) {
        if (speed_info->time_colon[i] != NULL && lv_obj_is_valid(speed_info->time_colon[i])) {
            lv_obj_move_foreground(speed_info->time_colon[i]);
        }
    }
    for (i = 0; i < PUMP_SPEED_TIME_DIGIT_COUNT; i++) {
        if (speed_info->time_h[i] != NULL && lv_obj_is_valid(speed_info->time_h[i])) {
            lv_obj_move_foreground(speed_info->time_h[i]);
        }
        if (speed_info->time_m[i] != NULL && lv_obj_is_valid(speed_info->time_m[i])) {
            lv_obj_move_foreground(speed_info->time_m[i]);
        }
        if (speed_info->time_s[i] != NULL && lv_obj_is_valid(speed_info->time_s[i])) {
            lv_obj_move_foreground(speed_info->time_s[i]);
        }
    }
}

static void pump_speed_time_rebuild(void)
{
    EXCEPTION_CHECK_VOID(speed_info->root && speed_info->ui);

    if (speed_info->time_label == NULL || !lv_obj_is_valid(speed_info->time_label)) {
        speed_info->time_label = lv_image_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->time_label);
        lv_obj_clear_flag(speed_info->time_label, LV_OBJ_FLAG_CLICKABLE);
        ui_img_baj_theme_set_src(speed_info->ui, speed_info->time_label,
            BASE_RES_ID_PUMP_TIME_LABEL, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(speed_info->time_label, LV_ALIGN_TOP_MID, 0, PUMP_SPEED_TIME_LABEL_Y);
    }

    if (speed_info->status_icon == NULL || !lv_obj_is_valid(speed_info->status_icon)) {
        speed_info->status_icon = lv_image_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->status_icon);
        lv_obj_clear_flag(speed_info->status_icon, LV_OBJ_FLAG_CLICKABLE);
        speed_info->shown_status_icon_id = 0;
        speed_info->shown_paused = 0xFF;
    }

    pump_speed_time_colon_ensure();
    speed_info->shown_hours = 0xFF;
    speed_info->shown_minutes = 0xFF;
    speed_info->shown_seconds = 0xFF;
    speed_info->shown_digit_size = 0;
    pump_speed_time_layout_apply(0, 0, 0, PUMP_SPEED_TIME_2SEG, true);
}

static void pump_speed_time_content_update(ui_proc_pump_t *proc)
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
#if IS_SUPPORT_PUMP_SPEED_TIME_3SEG_TEST
    hours = speed_info->test_hours;
    minutes = speed_info->test_hours;
    seconds = speed_info->test_hours;
#else
    hours = hold_time / 3600;
    minutes = (hold_time / 60) % 60;
    seconds = hold_time % 60;
#endif
    digit_size = (hours == 0) ? PUMP_SPEED_TIME_2SEG : PUMP_SPEED_TIME_3SEG;
    paused = (proc->pump_status == BREAST_PUMP_PAUSE);
    icon_id = paused ? BASE_RES_ID_PLAY_MIN_ICON : BASE_RES_ID_PAUSE_MIN_ICON;

    if (speed_info->status_icon != NULL && lv_obj_is_valid(speed_info->status_icon) &&
        (speed_info->shown_status_icon_id != icon_id || speed_info->force_update ||
         speed_info->shown_paused != (paused ? 1 : 0))) {
        ui_img_baj_theme_set_src(speed_info->ui, speed_info->status_icon, icon_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        speed_info->shown_status_icon_id = icon_id;
        speed_info->shown_paused = paused ? 1 : 0;
    }

    pump_speed_time_layout_apply(hours, minutes, seconds, digit_size, speed_info->force_update != 0);
}

static void pump_speed_time_update(ui_proc_pump_t *proc)
{
    if (speed_info->force_update) {
        pump_speed_time_rebuild();
    }
    pump_speed_time_content_update(proc);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_speed_adjust_prepare(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(thiz);
    if (speed_info == NULL) {
        speed_info = (pump_speed_adjust_ui_info_t *)u_malloz(sizeof(pump_speed_adjust_ui_info_t));
        EXCEPTION_CHECK_VOID(speed_info);
    }
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    speed_info->pump_mode = (uint8_t)pump_mode;
    speed_info->accent_color = home_level_adjust_accent_color_get(speed_info->pump_mode);
    LOG_UI("pump speed adjust prepare mode=%d color=0x%06X speed=%d",
        speed_info->pump_mode, speed_info->accent_color, proc->setting.speed);

    speed_info->ui = thiz;
    speed_info->force_update = true;
    speed_info->shown_speed = 0xFF;
    speed_info->shown_selected = 0xFF;
    speed_info->shown_accent_color = 0;
    speed_info->shown_hours = 0xFF;
    speed_info->shown_minutes = 0xFF;
    speed_info->shown_seconds = 0xFF;
    speed_info->shown_digit_size = 0;
    speed_info->shown_paused = 0xFF;
    speed_info->shown_status_icon_id = 0;
    pump_speed_adjust_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_speed_adjust_remove(ui_manager_context_t *thiz)
{
    int i;

    EXCEPTION_CHECK_VOID(thiz && speed_info && speed_info->root);
    ui_img_baj_release_decoded_buf(speed_info->time_label);
    LVGL_OBJ_DELETE(speed_info->time_label);
    speed_info->time_label = NULL;
    ui_img_baj_release_decoded_buf(speed_info->status_icon);
    LVGL_OBJ_DELETE(speed_info->status_icon);
    speed_info->status_icon = NULL;
    for (i = 0; i < PUMP_SPEED_TIME_COLON_COUNT; i++) {
        LVGL_OBJ_DELETE(speed_info->time_colon[i]);
        speed_info->time_colon[i] = NULL;
    }
    for (i = 0; i < PUMP_SPEED_TIME_DIGIT_COUNT; i++) {
        LVGL_OBJ_DELETE(speed_info->time_h[i]);
        speed_info->time_h[i] = NULL;
        LVGL_OBJ_DELETE(speed_info->time_m[i]);
        speed_info->time_m[i] = NULL;
        LVGL_OBJ_DELETE(speed_info->time_s[i]);
        speed_info->time_s[i] = NULL;
    }
    LVGL_OBJ_DELETE(speed_info->root);
    u_free(speed_info);
    speed_info = NULL;
}

int ui_display_pump_speed_adjust_process(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_INT(thiz && speed_info && speed_info->root);
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump);

    pump_speed_arc_update(proc, speed_info->accent_color);
    pump_speed_value_update(proc);
    pump_speed_label_update();
    pump_speed_time_update(proc);
    pump_speed_adjust_bg_update(thiz);
    speed_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump speed adjust fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
#if IS_SUPPORT_PUMP_SPEED_TIME_3SEG_TEST
        speed_info->test_hours += 1;
        if (speed_info->test_hours >= 59) {
            speed_info->test_hours = 0;
        }
#endif
    }
    return 0;
}
