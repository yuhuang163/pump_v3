/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pump_home_custom_ui_internal.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "ui_custom_mode_name.h" // IWYU pragma: keep
#include "ui_img_jpg_decode.h" // IWYU pragma: keep
#include "ui_img_png_decode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_HOME_MAIN_SCREEN_W                         (360)
#define PUMP_HOME_MAIN_SCALE_FULL                       (256)

#if IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_MODE_TITLE_COLOR                 (0x00FF00)
#else
#define PUMP_HOME_MAIN_MODE_TITLE_COLOR                 (0xFFFFFF)
#endif

/// 两段计时 MM:SS（hours==0）；坐标见 lang/*/layout.h
#define PUMP_HOME_MAIN_TIME_2SEG                        (2)
#define PUMP_HOME_MAIN_TIME_2SEG_WIDTH                  (88)

/// 三段计时 HH:MM:SS（hours>0，等宽）；坐标见 lang/*/layout.h
#define PUMP_HOME_MAIN_TIME_3SEG                        (3)
#define PUMP_HOME_MAIN_TIME_3SEG_WIDTH                  (69)

#define PUMP_HOME_MAIN_SPEED_GROUP_WIDTH                (47)
#if IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_SPEED_COLOR                      (0x00FF00)
#else
#define PUMP_HOME_MAIN_SPEED_COLOR                      (0xFFFFFF)
#endif
#define PUMP_HOME_MAIN_SPEED_ICON_VALUE_GAP             (6)

#define PUMP_HOME_MAIN_LEFT_LEVEL_W                     (58)
#define PUMP_HOME_MAIN_RIGHT_LEVEL_W                    (58)
#if IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_LEVEL_COLOR                      (0x00FF00)
#else
#define PUMP_HOME_MAIN_LEVEL_COLOR                      (0xFFFFFF)
#endif

#if IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_HINT_COLOR                       (0x00FF00)
#else
#define PUMP_HOME_MAIN_HINT_COLOR_STIMULATION           (PUMP_HOME_CUSTOM_HINT_COLOR_STIMULATION)
#define PUMP_HOME_MAIN_HINT_COLOR_LACTATION             (PUMP_HOME_CUSTOM_HINT_COLOR_EXPRESSION)
#define PUMP_HOME_MAIN_HINT_COLOR_PROGRAM               (PUMP_HOME_CUSTOM_HINT_COLOR_PROGRAM)
#endif
#define PUMP_HOME_MAIN_MASK_W                           (PUMP_HOME_BASE_MAIN_HINT_SHINE_W)
#define PUMP_HOME_MAIN_MASK_Y                           (PUMP_HOME_CUSTOM_MAIN_HINT_Y)

#define PUMP_HOME_MAIN_MODE_SWITCH_COLOR                (0xFFFFFF)
#define PUMP_HOME_MAIN_MODE_SWITCH_OPA                  (LV_OPA_60)
/// 漏气检测文案：与 home 首页 hint 字号/颜色/位置对齐
#define PUMP_HOME_MAIN_SEAL_CHECK_MASK_W                (PUMP_HOME_BASE_MAIN_HINT_SHINE_W)
#define PUMP_HOME_MAIN_SEAL_CHECK_MASK_Y                (PUMP_HOME_MAIN_SEAL_CHECK_Y)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t pump_home_main_bg_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return CUSTOM_RES_ID_PUMP_STIMULATION_BG;
        case BREAST_PUMP_LACTATION_MODE:
            return CUSTOM_RES_ID_PUMP_EXPRESSION_BG;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return CUSTOM_RES_ID_PUMP_PROGRAM_BG;
        default:
            return CUSTOM_RES_ID_PUMP_STIMULATION_BG;
    }
}

static void pump_home_main_mode_title_get(pump_home_base_ui_info_t *info, uint8_t mode,
    char *buf, size_t buf_len)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            snprintf(buf, buf_len, "%s", UI_STR_MODE_STIMULATION);
            break;
        case BREAST_PUMP_LACTATION_MODE:
            snprintf(buf, buf_len, "%s", UI_STR_MODE_EXPRESSION);
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            snprintf(buf, buf_len, "%s", UI_STR_MODE_MILK_BOOST);
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            snprintf(buf, buf_len, "%s", UI_STR_MODE_POWER_PUMPING);
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            ui_custom_mode_name_localize(info->mode.custom_id_str, buf, buf_len);
            break;
        default:
            snprintf(buf, buf_len, "");
            break;
    }
}

static uint32_t pump_home_main_hint_color_get(uint8_t mode)
{
#if IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG
    (void)mode;
    return PUMP_HOME_MAIN_HINT_COLOR;
#else
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return PUMP_HOME_MAIN_HINT_COLOR_STIMULATION;
        case BREAST_PUMP_LACTATION_MODE:
            return PUMP_HOME_MAIN_HINT_COLOR_LACTATION;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return PUMP_HOME_MAIN_HINT_COLOR_PROGRAM;
        default:
            return PUMP_HOME_MAIN_HINT_COLOR_STIMULATION;
    }
#endif
}

static bool pump_home_main_seal_check_showing(const ui_proc_pump_t *proc, bool paused)
{
    if (paused || proc == NULL) {
        return false;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK ||
        proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        return false;
    }
    return proc->seal_tip == PUMP_SEAL_TIP_CHECKING ||
        proc->seal_tip == PUMP_SEAL_TIP_RESULT ||
        proc->seal_tip == PUMP_SEAL_TIP_RESULT_FAIL;
}

static const char *pump_home_main_seal_check_text_get(const ui_proc_pump_t *proc)
{
    if (proc == NULL) {
        return NULL;
    }
    if (proc->seal_tip == PUMP_SEAL_TIP_CHECKING) {
        return PUMP_HOME_MAIN_SEAL_CHECKING_TEXT;
    }
    if (proc->seal_tip == PUMP_SEAL_TIP_RESULT) {
        return PUMP_HOME_MAIN_SEAL_RESULT_TEXT;
    }
    if (proc->seal_tip == PUMP_SEAL_TIP_RESULT_FAIL) {
        return PUMP_HOME_MAIN_SEAL_FAIL_TEXT;
    }
    return NULL;
}

static void pump_home_main_obj_flag_set(lv_obj_t *obj, bool visible)
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

static lv_coord_t pump_home_main_time_y_offset_get(const pump_home_base_ui_info_t *info)
{
    return info->main.shown_mode_switch ? PUMP_HOME_MAIN_TIME_MODE_SWITCH_SHIFT_Y : 0;
}

static lv_coord_t pump_home_main_time_sep_y_get(const pump_home_base_ui_info_t *info,
    uint8_t digit_size)
{
    const lv_coord_t digit_y = (digit_size == PUMP_HOME_MAIN_TIME_2SEG) ?
        PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_Y : PUMP_HOME_CUSTOM_MAIN_TIME_3SEG_Y;
    const lv_coord_t sep_shift = (digit_size == PUMP_HOME_MAIN_TIME_2SEG) ?
        PUMP_HOME_CUSTOM_TIME_2SEG_SEP_SHIFT_Y : PUMP_HOME_CUSTOM_TIME_3SEG_SEP_SHIFT_Y;

    return digit_y + sep_shift + pump_home_main_time_y_offset_get(info);
}

static void pump_home_main_time_layout_invalidate(pump_home_base_ui_info_t *info)
{
    info->main.shown_hours = 0xFF;
    info->main.shown_minutes = 0xFF;
    info->main.shown_seconds = 0xFF;
    info->main.shown_digit_size = 0;
}

static void pump_home_main_obj_scale_set(lv_obj_t *obj, int32_t scale)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_update_layout(obj);
    lv_obj_set_style_transform_pivot_x(obj, lv_obj_get_width(obj) / 2, 0);
    lv_obj_set_style_transform_pivot_y(obj, lv_obj_get_height(obj) / 2, 0);
    lv_obj_set_style_transform_scale(obj, scale, 0);
}

static uint32_t pump_home_main_time_color_get(uint8_t mode)
{
#if IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG
    (void)mode;
    return 0x00FF00;
#else
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return PUMP_HOME_CUSTOM_TIME_COLOR_STIMULATION;
        case BREAST_PUMP_LACTATION_MODE:
            return PUMP_HOME_CUSTOM_TIME_COLOR_EXPRESSION;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return PUMP_HOME_CUSTOM_TIME_COLOR_PROGRAM;
        default:
            return PUMP_HOME_CUSTOM_TIME_COLOR_STIMULATION;
    }
#endif
}

static uint16_t pump_home_main_play_icon_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return CUSTOM_RES_ID_PLAY_STIMULATION_ICON;
        case BREAST_PUMP_LACTATION_MODE:
            return CUSTOM_RES_ID_PLAY_EXPRESSION_ICON;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return CUSTOM_RES_ID_PLAY_PROGRAM_ICON;
        default:
            return CUSTOM_RES_ID_PLAY_STIMULATION_ICON;
    }
}

static void pump_home_main_bg_detach(pump_home_base_ui_info_t *info)
{
    if (info->main.bg == NULL || !lv_obj_is_valid(info->main.bg)) {
        return;
    }

    /* JPG 预解码由页面持有，切图/离页时用户释放 */
    ui_img_jpg_release_decoded_buf(info->main.bg);
}

static void pump_home_main_bg_update(pump_home_base_ui_info_t *info, uint8_t mode)
{
    const uint16_t res_id = pump_home_main_bg_res_id_get(mode);

    if (info->main.bg == NULL || !lv_obj_is_valid(info->main.bg)) {
        info->main.bg = lv_image_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.bg);
        lv_obj_move_background(info->main.bg);
    }
    if (info->main.shown_bg_id == res_id && info->force_update == false) {
        return;
    }
    LOG_UI("pump_home_custom_bg_update: res_id=%d/%d", res_id, mode);

    pump_home_main_bg_detach(info);
    ui_img_jpg_theme_set_src(info->ui, info->main.bg, res_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_JPG_DECODE_RGB888);
    lv_obj_align(info->main.bg, LV_ALIGN_CENTER, 0, 0);
    info->main.shown_bg_id = res_id;
}

static void pump_home_main_mode_title_update(pump_home_base_ui_info_t *info, uint8_t mode)
{
    char title[PUMP_HOME_BASE_CUSTOM_MODE_NAME_LEN] = {0};

    pump_home_main_mode_title_get(info, mode, title, sizeof(title));
    if (info->main.mode_title == NULL || !lv_obj_is_valid(info->main.mode_title)) {
        info->main.mode_title = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.mode_title);
        lv_obj_set_style_text_color(info->main.mode_title, lv_color_hex(PUMP_HOME_MAIN_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->main.mode_title, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_align(info->main.mode_title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.mode_title, 0, 0);
    }

    if (strncmp(info->main.shown_title, title, sizeof(info->main.shown_title)) == 0 &&
        info->force_update == false) {
        return;
    }
    lv_label_set_text(info->main.mode_title, title);
    lv_obj_align(info->main.mode_title, LV_ALIGN_TOP_MID, 0, PUMP_HOME_CUSTOM_MAIN_MODE_TITLE_Y);
    strncpy(info->main.shown_title, title, sizeof(info->main.shown_title) - 1);
    info->main.shown_title[sizeof(info->main.shown_title) - 1] = '\0';
}

static void pump_home_main_time_pair_update(lv_obj_t *labels[PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, const lv_font_t *font, lv_coord_t seg_x, lv_coord_t seg_y,
    lv_coord_t seg_width, uint32_t color, bool hidden)
{
    const lv_coord_t cell_w = seg_width / PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_obj_set_style_text_font(labels[i], font, 0);
        lv_obj_set_style_text_color(labels[i], lv_color_hex(color), 0);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
        pump_home_main_obj_flag_set(labels[i], !hidden);
    }
}

static void pump_home_main_time_h_update(pump_home_base_ui_info_t *info, uint8_t hours,
    uint8_t digit_size, uint32_t color)
{
    const bool hidden = (digit_size == PUMP_HOME_MAIN_TIME_2SEG);
    const lv_coord_t y = PUMP_HOME_CUSTOM_MAIN_TIME_3SEG_Y + pump_home_main_time_y_offset_get(info);

    if (info->main.shown_hours == hours && info->main.shown_digit_size == digit_size &&
        info->force_update == false) {
        return;
    }

    pump_home_main_time_pair_update(info->main.time_h, pump_home_page_parent_get(info), hours,
        UI_FONT_NUM_MD, PUMP_HOME_MAIN_TIME_3SEG_H_X, y,
        PUMP_HOME_MAIN_TIME_3SEG_WIDTH, color, hidden);
    info->main.shown_hours = hours;
}

static void pump_home_main_time_m_update(pump_home_base_ui_info_t *info, uint8_t minutes,
    uint8_t digit_size, uint32_t color)
{
    const lv_coord_t y_offset = pump_home_main_time_y_offset_get(info);

    if (info->main.shown_minutes == minutes && info->main.shown_digit_size == digit_size &&
        info->force_update == false) {
        return;
    }

    if (digit_size == PUMP_HOME_MAIN_TIME_2SEG) {
        pump_home_main_time_pair_update(info->main.time_m, pump_home_page_parent_get(info), minutes,
            UI_FONT_NUM_LG, PUMP_HOME_MAIN_TIME_2SEG_M_X,
            PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_Y + y_offset, PUMP_HOME_MAIN_TIME_2SEG_WIDTH, color, false);
    } else {
        pump_home_main_time_pair_update(info->main.time_m, pump_home_page_parent_get(info), minutes,
            UI_FONT_NUM_MD, PUMP_HOME_MAIN_TIME_3SEG_M_X,
            PUMP_HOME_CUSTOM_MAIN_TIME_3SEG_Y + y_offset, PUMP_HOME_MAIN_TIME_3SEG_WIDTH, color, false);
    }
    info->main.shown_minutes = minutes;
}

static void pump_home_main_time_s_update(pump_home_base_ui_info_t *info, uint8_t seconds,
    uint8_t digit_size, uint32_t color)
{
    const lv_coord_t y_offset = pump_home_main_time_y_offset_get(info);

    if (info->main.shown_seconds == seconds && info->main.shown_digit_size == digit_size &&
        info->force_update == false) {
        return;
    }

    if (digit_size == PUMP_HOME_MAIN_TIME_2SEG) {
        pump_home_main_time_pair_update(info->main.time_s, pump_home_page_parent_get(info), seconds,
            UI_FONT_NUM_LG, PUMP_HOME_MAIN_TIME_2SEG_S_X,
            PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_Y + y_offset, PUMP_HOME_MAIN_TIME_2SEG_WIDTH, color, false);
    } else {
        pump_home_main_time_pair_update(info->main.time_s, pump_home_page_parent_get(info), seconds,
            UI_FONT_NUM_MD, PUMP_HOME_MAIN_TIME_3SEG_S_X,
            PUMP_HOME_CUSTOM_MAIN_TIME_3SEG_Y + y_offset, PUMP_HOME_MAIN_TIME_3SEG_WIDTH, color, false);
    }
    info->main.shown_seconds = seconds;
}

static void pump_home_main_time_sep_label_update(lv_obj_t **label, lv_obj_t *parent,
    const char *text, const lv_font_t *font, uint32_t color, lv_align_t align,
    lv_coord_t x, lv_coord_t y)
{
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
        lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(*label, 0, 0);
    }
    lv_obj_set_style_text_font(*label, font, 0);
    lv_obj_set_style_text_color(*label, lv_color_hex(color), 0);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, align, x, y);
    pump_home_main_obj_flag_set(*label, true);
}

static void pump_home_main_time_colon_release(pump_home_base_ui_info_t *info)
{
    (void)info;
}

static void pump_home_main_time_colon_3seg_update(pump_home_base_ui_info_t *info, uint32_t color)
{
    const lv_coord_t colon_x[PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT] = {
        PUMP_HOME_MAIN_TIME_COLON_HM_X,
        PUMP_HOME_MAIN_TIME_COLON_MS_X,
    };
    const char *texts[PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT] = {
        PUMP_HOME_CUSTOM_TIME_H_TEXT,
        PUMP_HOME_CUSTOM_TIME_M_TEXT,
    };
    const lv_coord_t colon_y = pump_home_main_time_sep_y_get(info, PUMP_HOME_MAIN_TIME_3SEG);
    lv_obj_t *parent = pump_home_page_parent_get(info);
    int i;

    for (i = 0; i < PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT; i++) {
        pump_home_main_time_sep_label_update(&info->main.time_colon[i], parent, texts[i],
            UI_FONT_NUM_MD, color, LV_ALIGN_TOP_LEFT, colon_x[i], colon_y);
    }
}

static void pump_home_main_time_colon_2seg_update(pump_home_base_ui_info_t *info, uint32_t color)
{
    lv_obj_t *parent = pump_home_page_parent_get(info);
    int i;

    pump_home_main_time_sep_label_update(&info->main.time_colon_2seg, parent,
        PUMP_HOME_CUSTOM_TIME_M_TEXT, UI_FONT_NUM_LG, color, LV_ALIGN_TOP_MID,
        PUMP_HOME_MAIN_TIME_2SEG_COLON_X,
        pump_home_main_time_sep_y_get(info, PUMP_HOME_MAIN_TIME_2SEG));

    for (i = 0; i < PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT; i++) {
        pump_home_main_obj_flag_set(info->main.time_colon[i], false);
    }
}

static void pump_home_main_time_colon_update(pump_home_base_ui_info_t *info, uint8_t digit_size,
    uint32_t color)
{
    if (info->main.shown_digit_size == digit_size && info->force_update == false &&
        ((digit_size == PUMP_HOME_MAIN_TIME_2SEG &&
          info->main.time_colon_2seg != NULL && lv_obj_is_valid(info->main.time_colon_2seg)) ||
         (digit_size == PUMP_HOME_MAIN_TIME_3SEG &&
          info->main.time_colon[0] != NULL && lv_obj_is_valid(info->main.time_colon[0])))) {
        return;
    }

    if (digit_size == PUMP_HOME_MAIN_TIME_2SEG) {
        pump_home_main_time_colon_2seg_update(info, color);
    } else {
        pump_home_main_obj_flag_set(info->main.time_colon_2seg, false);
        pump_home_main_time_colon_3seg_update(info, color);
    }
}

static void pump_home_main_time_visibility_set(pump_home_base_ui_info_t *info, bool visible,
    uint8_t digit_size)
{
    int i;
    const bool show_3seg = visible && (digit_size == PUMP_HOME_MAIN_TIME_3SEG);
    const bool show_2seg = visible && (digit_size == PUMP_HOME_MAIN_TIME_2SEG);

    for (i = 0; i < PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT; i++) {
        pump_home_main_obj_flag_set(info->main.time_h[i], show_3seg);
        pump_home_main_obj_flag_set(info->main.time_m[i], visible);
        pump_home_main_obj_flag_set(info->main.time_s[i], visible);
    }
    for (i = 0; i < PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT; i++) {
        pump_home_main_obj_flag_set(info->main.time_colon[i], show_3seg);
    }
    pump_home_main_obj_flag_set(info->main.time_colon_2seg, show_2seg);
}

static void pump_home_main_time_update(pump_home_base_ui_info_t *info, uint8_t hours,
    uint8_t minutes, uint8_t seconds, bool paused)
{
    const uint8_t digit_size = (hours == 0) ? PUMP_HOME_MAIN_TIME_2SEG : PUMP_HOME_MAIN_TIME_3SEG;
    const bool time_visible = !paused && (info->main.shown_seal_check == 0);
    const uint32_t color = pump_home_main_time_color_get(info->mode.new_index);

    if (info->main.shown_time_color != color) {
        pump_home_main_time_layout_invalidate(info);
        info->main.shown_time_color = color;
    }

    if (!paused) {
        pump_home_main_time_h_update(info, hours, digit_size, color);
        pump_home_main_time_m_update(info, minutes, digit_size, color);
        pump_home_main_time_s_update(info, seconds, digit_size, color);
        pump_home_main_time_colon_update(info, digit_size, color);
        info->main.shown_digit_size = digit_size;
    }
    pump_home_main_time_visibility_set(info, time_visible, digit_size);
}

static void pump_home_main_play_icon_detach(pump_home_base_ui_info_t *info)
{
    if (info->main.play_icon == NULL || !lv_obj_is_valid(info->main.play_icon)) {
        return;
    }
    ui_img_png_release_decoded_buf(info->main.play_icon);
    info->main.shown_play_icon_id = 0;
}

static void pump_home_main_play_icon_update(pump_home_base_ui_info_t *info, bool paused)
{
    const uint16_t res_id = pump_home_main_play_icon_res_id_get(info->mode.new_index);

    if (!paused) {
        pump_home_main_play_icon_detach(info);
        pump_home_main_obj_flag_set(info->main.play_icon, false);
        return;
    }

    if (info->main.play_icon == NULL || !lv_obj_is_valid(info->main.play_icon)) {
        info->main.play_icon = lv_image_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.play_icon);
        info->main.shown_play_icon_id = 0;
    }
    if (info->main.shown_play_icon_id != res_id) {
        ui_img_png_theme_set_src(info->ui, info->main.play_icon, res_id,
            RES_THEME_TYPE_CUSTOM, UI_IMG_PNG_DECODE_RGBA8888);
        info->main.shown_play_icon_id = res_id;
    }
    lv_obj_align(info->main.play_icon, LV_ALIGN_TOP_MID, 0, PUMP_HOME_CUSTOM_MAIN_PLAY_ICON_Y);
    pump_home_main_obj_flag_set(info->main.play_icon, true);
}

static void pump_home_main_hint_style_get(ui_shine_style_t *style)
{
    ui_shine_style_init(style);
    style->mask_w = PUMP_HOME_MAIN_MASK_W;
    style->mask_y = PUMP_HOME_MAIN_MASK_Y;
    style->screen_w = PUMP_HOME_MAIN_SCREEN_W;
}

static void pump_home_main_hint_text_update(pump_home_base_ui_info_t *info, uint8_t mode,
    bool paused)
{
    const uint32_t color = pump_home_main_hint_color_get(mode);

    if (!paused) {
        pump_home_main_obj_flag_set(info->main.hint_text, false);
        return;
    }

    if (info->main.hint_text != NULL && lv_obj_is_valid(info->main.hint_text) &&
        info->main.shown_hint_color == color && info->force_update == false &&
        info->main.shown_paused != 0) {
        pump_home_main_obj_flag_set(info->main.hint_text, true);
        return;
    }

    if (info->main.hint_text == NULL || !lv_obj_is_valid(info->main.hint_text)) {
        info->main.hint_text = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.hint_text);
        lv_obj_set_style_text_font(info->main.hint_text, UI_FONT_AEONIK_19, 0);
        lv_obj_set_style_text_align(info->main.hint_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.hint_text, 0, 0);
    }
    lv_obj_set_style_text_color(info->main.hint_text, lv_color_hex(color), 0);
    lv_label_set_text(info->main.hint_text, PUMP_HOME_MAIN_HINT_TEXT);
    lv_obj_align(info->main.hint_text, LV_ALIGN_TOP_MID, 0, PUMP_HOME_CUSTOM_MAIN_HINT_Y);
    pump_home_main_obj_flag_set(info->main.hint_text, true);
    info->main.shown_hint_color = color;
}

static void pump_home_main_hint_mask_update(pump_home_base_ui_info_t *info, bool paused)
{
    lv_obj_t *parent = pump_home_page_parent_get(info);
    ui_shine_style_t style;

    if (!paused) {
        ui_shine_hide(&info->main.hint_shine);
        return;
    }

    if (info->main.hint_shine.mask != NULL && lv_obj_is_valid(info->main.hint_shine.mask) &&
        info->main.hint_shine.shine != NULL && lv_obj_is_valid(info->main.hint_shine.shine) &&
        info->force_update == false && info->main.shown_paused != 0) {
        ui_shine_show(&info->main.hint_shine);
        return;
    }

    pump_home_main_hint_style_get(&style);
    if (info->main.shown_paused != 1) {
        info->main.hint_shine.mask_x = (int16_t)(-(int16_t)style.mask_w);
        info->main.hint_shine.mask_tick_ms = get_timestamp();
    }
    ui_shine_prepare_from_text(&info->main.hint_shine, &style,
        PUMP_HOME_MAIN_HINT_TEXT, UI_FONT_AEONIK_19, 0, 0);
    ui_shine_update(&info->main.hint_shine, parent, &style, info->main.hint_text, 1);
    ui_shine_show(&info->main.hint_shine);
}

static void pump_home_main_hint_update(pump_home_base_ui_info_t *info, uint8_t mode, bool paused)
{
    pump_home_main_hint_text_update(info, mode, paused);
    pump_home_main_hint_mask_update(info, paused);
}

static void pump_home_main_hint_shine_anim_update(pump_home_base_ui_info_t *info, bool paused)
{
    ui_shine_style_t style;
    uint8_t anim_paused;

    if (!paused) {
        return;
    }

    pump_home_main_hint_style_get(&style);
    anim_paused = info->animation.paused_by_transition ? 1 : 0;
    ui_shine_anim_update(&info->main.hint_shine, &style, info->main.hint_text, anim_paused);
}

static void pump_home_main_mode_switch_update(pump_home_base_ui_info_t *info, uint8_t mode_type,
    bool is_show_mode_switch)
{
    char tip_buf[64] = {0};
    const uint8_t pump_mode = info->mode.new_index;
    uint8_t mode_switch = info->main.shown_mode_switch;
    bool state_changed;
    bool type_changed;
    bool pump_mode_changed;

    type_changed = (info->main.tracked_mode_type != 0xFF &&
        info->main.tracked_mode_type != mode_type);
    pump_mode_changed = (info->main.tracked_pump_mode != 0xFF &&
        info->main.tracked_pump_mode != pump_mode);

    /*
     * 仅韵律模式下、同一工作模式内的子模式切换才提示。
     * 非韵律刺激↔吸乳、韵律↔刺激/吸乳（工作模式切换）均不显示。
     */
    if (type_changed && !pump_mode_changed && !PUMP_IS_COMMON_MODE(pump_mode)) {
        mode_switch = 1;
        info->main.mode_switch_time = get_timestamp();
    } else if (pump_mode_changed || PUMP_IS_COMMON_MODE(pump_mode)) {
        mode_switch = 0;
    }
    
    /// 如果不需要显示模式切换提示，则不显示
    if (is_show_mode_switch == false) {
        mode_switch = 0;
    }

    info->main.tracked_mode_type = mode_type;
    info->main.tracked_pump_mode = pump_mode;

    if (mode_switch &&
        get_timestamp_ms(info->main.mode_switch_time) >= PUMP_SUB_MODE_SWITCH_TIMEOUT_MS) {
        mode_switch = 0;
    }

    state_changed = (info->main.shown_mode_switch != mode_switch) ||
        (mode_switch && info->main.tip_mode_type != mode_type) ||
        (info->force_update != 0);

    if (info->main.mode_switch_label == NULL || !lv_obj_is_valid(info->main.mode_switch_label)) {
        info->main.mode_switch_label = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.mode_switch_label);
        lv_obj_set_style_text_color(info->main.mode_switch_label,
            lv_color_hex(PUMP_HOME_MAIN_MODE_SWITCH_COLOR), 0);
        lv_obj_set_style_text_opa(info->main.mode_switch_label, PUMP_HOME_MAIN_MODE_SWITCH_OPA, 0);
        lv_obj_set_style_text_font(info->main.mode_switch_label, UI_FONT_HINT, 0);
        lv_obj_set_style_text_align(info->main.mode_switch_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_line_space(info->main.mode_switch_label,
            PUMP_HOME_MAIN_MODE_SWITCH_LINE_SPACE, 0);
        lv_obj_set_style_text_letter_space(info->main.mode_switch_label, 0, 0);
        state_changed = true;
    }

    if (state_changed == false) {
        return;
    }

    if (mode_switch) {
        info->main.tip_mode_type = mode_type;
        snprintf(tip_buf, sizeof(tip_buf), PUMP_HOME_MAIN_MODE_SWITCH_TEXT_FMT,
            home_level_adjust_mode_type_name_get(mode_type));
        lv_label_set_text(info->main.mode_switch_label, tip_buf);
        lv_obj_align(info->main.mode_switch_label, LV_ALIGN_TOP_MID, 0, PUMP_HOME_MAIN_MODE_SWITCH_Y);
        pump_home_main_obj_flag_set(info->main.mode_switch_label, true);
        info->animation.hidden_by_mode_switch = 1;
    } else {
        pump_home_main_obj_flag_set(info->main.mode_switch_label, false);
        info->animation.hidden_by_mode_switch = 0;
    }

    if (info->main.shown_mode_switch != mode_switch) {
        pump_home_main_time_layout_invalidate(info);
    }
    info->main.shown_mode_switch = mode_switch;
}

static int32_t pump_home_main_seal_check_line_space_get(const ui_proc_pump_t *proc)
{
    if (proc != NULL && proc->seal_tip == PUMP_SEAL_TIP_RESULT_FAIL) {
        return PUMP_HOME_MAIN_SEAL_FAIL_LINE_SPACE;
    }
    return 0;
}

static void pump_home_main_seal_check_style_get(ui_shine_style_t *style, lv_obj_t *label)
{
    ui_shine_style_init(style);
    style->mask_w = PUMP_HOME_MAIN_SEAL_CHECK_MASK_W;
    style->mask_y = PUMP_HOME_MAIN_SEAL_CHECK_MASK_Y;
    style->screen_w = PUMP_HOME_MAIN_SCREEN_W;
    if (label != NULL && lv_obj_is_valid(label)) {
        lv_obj_update_layout(label);
        style->mask_y = (int16_t)lv_obj_get_y(label);
    }
}

static void pump_home_main_seal_check_mask_update(pump_home_base_ui_info_t *info, bool show,
    const char *text, int32_t line_space)
{
    lv_obj_t *parent = pump_home_page_parent_get(info);
    ui_shine_style_t style;

    if (!show || text == NULL) {
        ui_shine_hide(&info->main.seal_check_shine);
        return;
    }

    if (info->main.seal_check_shine.mask != NULL && lv_obj_is_valid(info->main.seal_check_shine.mask) &&
        info->main.seal_check_shine.shine != NULL && lv_obj_is_valid(info->main.seal_check_shine.shine) &&
        info->force_update == false && info->main.shown_seal_check != 0) {
        ui_shine_show(&info->main.seal_check_shine);
        return;
    }

    pump_home_main_seal_check_style_get(&style, info->main.seal_check_label);
    info->main.seal_check_shine.mask_x = (int16_t)(-(int16_t)style.mask_w);
    info->main.seal_check_shine.mask_tick_ms = get_timestamp();
    ui_shine_prepare_from_text(&info->main.seal_check_shine, &style,
        text, UI_FONT_HINT, 0, line_space);
    ui_shine_update(&info->main.seal_check_shine, parent, &style, info->main.seal_check_label, 1);
    ui_shine_show(&info->main.seal_check_shine);
}

static void pump_home_main_seal_check_shine_anim_update(pump_home_base_ui_info_t *info, bool show)
{
    ui_shine_style_t style;
    uint8_t anim_paused;

    if (!show) {
        return;
    }

    pump_home_main_seal_check_style_get(&style, info->main.seal_check_label);
    anim_paused = info->animation.paused_by_transition ? 1 : 0;
    ui_shine_anim_update(&info->main.seal_check_shine, &style, info->main.seal_check_label,
        anim_paused);
}

static void pump_home_main_seal_check_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc,
    uint8_t mode, bool paused)
{
    const bool show = pump_home_main_seal_check_showing(proc, paused);
    const char *text = show ? pump_home_main_seal_check_text_get(proc) : NULL;
    const uint32_t color = pump_home_main_hint_color_get(mode);
    const int32_t line_space = pump_home_main_seal_check_line_space_get(proc);
    bool state_changed;
    bool text_changed = false;

    if (info->main.seal_check_label == NULL || !lv_obj_is_valid(info->main.seal_check_label)) {
        info->main.seal_check_label = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.seal_check_label);
        lv_obj_set_style_text_font(info->main.seal_check_label, UI_FONT_HINT, 0);
        lv_obj_set_style_text_align(info->main.seal_check_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.seal_check_label, 0, 0);
        info->main.shown_seal_check = 0xFF;
        info->main.shown_seal_color = 0xFFFFFFFFU;
        info->main.shown_seal_text[0] = '\0';
    }

    if (show && text != NULL) {
        text_changed = (strncmp(info->main.shown_seal_text, text,
            sizeof(info->main.shown_seal_text)) != 0);
    }

    state_changed = (info->main.shown_seal_check != (show ? 1 : 0)) ||
        (show && info->main.shown_seal_color != color) ||
        text_changed ||
        (info->force_update != 0);

    if (state_changed) {
        if (show && text != NULL) {
            lv_obj_set_style_text_color(info->main.seal_check_label, lv_color_hex(color), 0);
            lv_obj_set_style_text_line_space(info->main.seal_check_label, line_space, 0);
            lv_label_set_text(info->main.seal_check_label, text);
            lv_obj_align(info->main.seal_check_label, LV_ALIGN_TOP_MID, 0, PUMP_HOME_MAIN_SEAL_CHECK_Y);
            lv_obj_update_layout(info->main.seal_check_label);
            pump_home_main_obj_flag_set(info->main.seal_check_label, true);
            strncpy(info->main.shown_seal_text, text, sizeof(info->main.shown_seal_text) - 1);
            info->main.shown_seal_text[sizeof(info->main.shown_seal_text) - 1] = '\0';
            info->main.shown_seal_color = color;
        } else {
            pump_home_main_obj_flag_set(info->main.seal_check_label, false);
            info->main.shown_seal_text[0] = '\0';
        }
        if (text_changed) {
            ui_shine_remove(&info->main.seal_check_shine);
        }
    }

    info->animation.seal_tip = show ? proc->seal_tip : PUMP_SEAL_TIP_NONE;
    pump_home_main_seal_check_mask_update(info, show, text, line_space);
    info->main.shown_seal_check = show ? 1 : 0;
}

static void pump_home_main_speed_update(pump_home_base_ui_info_t *info, uint8_t speed)
{
    char speed_text[8] = {0};
    lv_coord_t icon_w = 0;
    lv_coord_t value_w = 0;
    lv_coord_t total_w;
    lv_coord_t start_x;

    if (info->main.speed_icon == NULL || !lv_obj_is_valid(info->main.speed_icon)) {
        info->main.speed_icon = lv_image_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.speed_icon);
        ui_img_baj_theme_set_src(info->ui, info->main.speed_icon, CUSTOM_RES_ID_SPEED_ICON,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->main.speed_value == NULL || !lv_obj_is_valid(info->main.speed_value)) {
        info->main.speed_value = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.speed_value);
        lv_obj_set_style_text_color(info->main.speed_value, lv_color_hex(PUMP_HOME_MAIN_SPEED_COLOR), 0);
        lv_obj_set_style_text_font(info->main.speed_value, UI_FONT_AEONIK, 0);
    }

    if (info->main.shown_speed == speed && info->force_update == false) {
        return;
    }

    /// 存储为 0 基，界面按 1 基展示
    snprintf(speed_text, sizeof(speed_text), "%u", (unsigned)(speed + 1));
    lv_label_set_text(info->main.speed_value, speed_text);
    lv_obj_update_layout(info->main.speed_icon);
    lv_obj_update_layout(info->main.speed_value);
    icon_w = lv_obj_get_width(info->main.speed_icon);
    value_w = lv_obj_get_width(info->main.speed_value);
    total_w = icon_w + PUMP_HOME_MAIN_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > PUMP_HOME_MAIN_SPEED_GROUP_WIDTH) {
        total_w = PUMP_HOME_MAIN_SPEED_GROUP_WIDTH;
    }
    start_x = (PUMP_HOME_MAIN_SCREEN_W - total_w) / 2;
    lv_obj_align(info->main.speed_icon, LV_ALIGN_TOP_LEFT, start_x, PUMP_HOME_CUSTOM_MAIN_SPEED_ICON_Y);
    lv_obj_align(info->main.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + PUMP_HOME_MAIN_SPEED_ICON_VALUE_GAP, PUMP_HOME_CUSTOM_MAIN_SPEED_VALUE_Y);
    info->main.shown_speed = speed;
}

static void pump_home_main_level_label_update(lv_obj_t **label, lv_obj_t *parent, const char *prefix,
    uint8_t level, lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_text_align_t align,
    uint8_t *shown_level, bool force)
{
    char text[8] = {0};

    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
        lv_label_set_long_mode(*label, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(*label, width);
        lv_obj_set_style_text_color(*label, lv_color_hex(PUMP_HOME_MAIN_LEVEL_COLOR), 0);
        lv_obj_set_style_text_font(*label, UI_FONT_AEONIK, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 3, 0);
    }
    if (*shown_level == level && force == false) {
        return;
    }
    snprintf(text, sizeof(text), "%s%u", prefix, (unsigned)level);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
    *shown_level = level;
}

#if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
static void pump_home_main_wear_result_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc)
{
    pump_home_base_main_page_info_t *main = &info->main;
    char wear_result_text[32];

    EXCEPTION_CHECK_VOID(info && proc);
    if (proc->wear_result[0] == main->wear_result[0] &&
        proc->wear_result[1] == main->wear_result[1] &&
        proc->seal_result    == main->seal_result) {
        return;
    }

    if (main->wear_result_text == NULL || !lv_obj_is_valid(main->wear_result_text)) {
        main->wear_result_text = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(main->wear_result_text);
        lv_obj_set_style_text_color(main->wear_result_text, lv_color_white(), 0);
        lv_obj_set_style_text_font(main->wear_result_text, UI_FONT_TITLE, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(main->wear_result_text, 2, 0);  /// 设置行间距（像素）
        lv_obj_set_style_text_align(main->wear_result_text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(main->wear_result_text, LV_ALIGN_TOP_MID, 0, 325);
    }
    snprintf(wear_result_text, sizeof(wear_result_text), "%d-%d-%d",
        proc->seal_result, proc->wear_result[0], proc->wear_result[1]);
    lv_label_set_text(main->wear_result_text, wear_result_text);
    memcpy(main->wear_result, proc->wear_result, sizeof(main->wear_result));
    main->seal_result = proc->seal_result;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_custom_main_page_ui_build(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_shine_remove(&info->main.hint_shine);
    ui_shine_remove(&info->main.seal_check_shine);
    pump_home_main_time_colon_release(info);
    memset(&info->main, 0, sizeof(info->main));
    info->main.shown_bg_id = 0;
    info->main.shown_hint_color = 0xFFFFFFFFU;
    info->main.shown_time_color = 0xFFFFFFFFU;
    info->main.shown_play_icon_id = 0;
    info->main.shown_speed = 0xFF;
    info->main.shown_left_level = 0xFF;
    info->main.shown_right_level = 0xFF;
    info->main.shown_hours = 0xFF;
    info->main.shown_minutes = 0xFF;
    info->main.shown_seconds = 0xFF;
    info->main.shown_digit_size = 0;
    info->main.shown_paused = 0xFF;
    info->main.shown_mode_switch = 0;
    info->main.shown_seal_check = 0;
    info->main.shown_seal_color = 0xFFFFFFFFU;
    info->main.shown_seal_text[0] = '\0';
    info->main.tracked_mode_type = 0xFF;
    info->main.tracked_pump_mode = 0xFF;
    info->main.tip_mode_type = 0xFF;
    info->main.mode_switch_time = 0;
    info->animation.paused_by_transition = 0;
    info->animation.hidden_by_mode_switch = 0;
    info->animation.seal_tip = PUMP_SEAL_TIP_NONE;
    #if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
    info->main.wear_result[0] = 0xFF;
    info->main.wear_result[1] = 0xFF;
    info->main.seal_result = 0xFF;
    #endif
}

void pump_home_custom_main_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc)
{
    uint8_t mode;
    bool paused;
#if IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG
    const uint8_t left_level = 9;
    const uint8_t right_level = 13;
#else
    uint8_t left_level;
    uint8_t right_level;
#endif
    uint8_t speed = 0;
    pump_setting_t setting;
    int time_index = 0;
    const pump_result_info_t *result_info = NULL;
    uint16_t hold_time;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    EXCEPTION_CHECK_VOID(info && proc && proc->pump);
    mode = info->mode.new_index;
    paused = (proc->pump_status == BREAST_PUMP_PAUSE);
#if (IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG == 0)
    left_level = info->level.left;
    right_level = info->level.right;
#endif
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, setting);
    speed = setting.speed;
    READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    EXCEPTION_CHECK_VOID(result_info);
    hold_time = result_info->work_valid_time / 1000;
#if IS_SUPPORT_BASE_TIME_3SEG_TEST
    hours = info->test_hours;
    minutes = info->test_hours;
    seconds = info->test_hours;
#else
    hours = hold_time / 3600;
    minutes = (hold_time / 60) % 60;
    seconds = hold_time % 60;
#endif
#if (IS_SUPPORT_CUSTOM_PUMP_HOME_COMP_BG == 0)
    pump_home_main_bg_update(info, mode);
#endif
    pump_home_main_mode_title_update(info, mode);
    bool is_show_mode_switch = (hold_time > 5 && paused == false) ? true : false;
    if (is_show_mode_switch && pump_home_main_seal_check_showing(proc, paused)) {  
        is_show_mode_switch = false; /// 如果需要显示模式切换提示，但漏气文案展示，则不显示
    }
    pump_home_main_mode_switch_update(info, info->mode.new_mode_type, is_show_mode_switch);
    pump_home_main_seal_check_update(info, proc, mode, paused);
    pump_home_main_seal_check_shine_anim_update(info, pump_home_main_seal_check_showing(proc, paused));
    pump_home_main_time_update(info, hours, minutes, seconds, paused);
    pump_home_base_animation_ui_update(info);
    pump_home_main_play_icon_update(info, paused);
    pump_home_main_hint_update(info, mode, paused);
    pump_home_main_hint_shine_anim_update(info, paused);
    pump_home_main_speed_update(info, speed);
    pump_home_main_level_label_update(&info->main.left_level, pump_home_page_parent_get(info), "L",
        left_level, PUMP_HOME_MAIN_LEFT_LEVEL_X, PUMP_HOME_CUSTOM_MAIN_LEFT_LEVEL_Y,
        PUMP_HOME_MAIN_LEFT_LEVEL_W, LV_TEXT_ALIGN_RIGHT, &info->main.shown_left_level,
        info->force_update != 0);
    pump_home_main_level_label_update(&info->main.right_level, pump_home_page_parent_get(info), "R",
        right_level, PUMP_HOME_MAIN_RIGHT_LEVEL_X, PUMP_HOME_CUSTOM_MAIN_RIGHT_LEVEL_Y,
        PUMP_HOME_MAIN_RIGHT_LEVEL_W, LV_TEXT_ALIGN_LEFT, &info->main.shown_right_level,
        info->force_update != 0);
#if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
    pump_home_main_wear_result_update(info, proc);
#endif
    info->main.shown_paused = paused ? 1 : 0;
    /* 缩放复位只在切页过渡里 apply；稳态每帧设 FULL 会反复 update_layout */
}

void pump_home_custom_main_page_ui_remove(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    pump_home_main_bg_detach(info);
    pump_home_main_time_colon_release(info);
    pump_home_main_play_icon_detach(info);
    ui_img_baj_release_decoded_buf(info->main.speed_icon);
    ui_shine_remove(&info->main.hint_shine);
    ui_shine_remove(&info->main.seal_check_shine);
    memset(&info->main, 0, sizeof(info->main));
}

void pump_home_custom_main_page_transition_apply(pump_home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning)
{
    EXCEPTION_CHECK_VOID(info);
    (void)hero_scale;
    (void)bg_scale;
    (void)transitioning;

    /* page0 不缩放，仅随 page 容器位移；保持原背景与控件 */
    if (info->main.bg != NULL && lv_obj_is_valid(info->main.bg)) {
        lv_obj_clear_flag(info->main.bg, LV_OBJ_FLAG_HIDDEN);
        pump_home_main_obj_scale_set(info->main.bg, PUMP_HOME_MAIN_SCALE_FULL);
    }
    pump_home_main_obj_scale_set(info->main.mode_title, PUMP_HOME_MAIN_SCALE_FULL);
    {
        uint8_t i;
        for (i = 0; i < PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT; i++) {
            pump_home_main_obj_scale_set(info->main.time_h[i], PUMP_HOME_MAIN_SCALE_FULL);
            pump_home_main_obj_scale_set(info->main.time_m[i], PUMP_HOME_MAIN_SCALE_FULL);
            pump_home_main_obj_scale_set(info->main.time_s[i], PUMP_HOME_MAIN_SCALE_FULL);
        }
        for (i = 0; i < PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT; i++) {
            pump_home_main_obj_scale_set(info->main.time_colon[i], PUMP_HOME_MAIN_SCALE_FULL);
        }
        pump_home_main_obj_scale_set(info->main.time_colon_2seg, PUMP_HOME_MAIN_SCALE_FULL);
    }
}

const pump_home_main_page_ops_t pump_home_custom_main_page_ops = {
    .build = pump_home_custom_main_page_ui_build,
    .update = pump_home_custom_main_page_ui_update,
    .remove = pump_home_custom_main_page_ui_remove,
    .transition_apply = pump_home_custom_main_page_transition_apply,
};
