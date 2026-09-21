/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_custom_mode_name.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_MAIN_SCREEN_W                              (360)
#define HOME_MAIN_SCALE_FULL                            (256)

#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_MODE_TITLE_COLOR                      (0x00FF00)
#else
#define HOME_MAIN_MODE_TITLE_COLOR                      (0xFFFFFF)
#endif

/// 两段计时 MM:SS（hours==0）；坐标见 lang/*/layout.h
#define HOME_MAIN_TIME_2SEG                             (2)
#define HOME_MAIN_TIME_2SEG_WIDTH                       (88)

/// 三段计时 HH:MM:SS（hours>0，等宽，参考 V6）；坐标见 lang/*/layout.h
#define HOME_MAIN_TIME_3SEG                             (3)
#define HOME_MAIN_TIME_3SEG_WIDTH                       (69)
#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_TIME_COLOR                            (0x00FF00)
#else
#define HOME_MAIN_TIME_COLOR                            (0xFFFFFF)
#endif

#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_HINT_COLOR                            (0x00FF00)
#else
#define HOME_MAIN_HINT_COLOR_STIMULATION                (0xAD7C89)
#define HOME_MAIN_HINT_COLOR_LACTATION                  (0xB9D9FF)
#define HOME_MAIN_HINT_COLOR_PROGRAM                    (0xB195DF)
#endif

#define HOME_MAIN_MASK_W                                (HOME_MAIN_HINT_SHINE_W)
#define HOME_MAIN_MASK_Y                                (HOME_MAIN_HINT_Y)

#define HOME_MAIN_SPEED_GROUP_WIDTH                     (47)
#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_SPEED_COLOR                           (0x00FF00)
#else
#define HOME_MAIN_SPEED_COLOR                           (0xFFFFFF)
#endif
#define HOME_MAIN_SPEED_ICON_VALUE_GAP                  (6)

#define HOME_MAIN_LEFT_LEVEL_W                          (58)
#define HOME_MAIN_RIGHT_LEVEL_W                         (58)
#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_LEVEL_COLOR                           (0x00FF00)
#else
#define HOME_MAIN_LEVEL_COLOR                           (0xFFFFFF)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static lv_coord_t home_main_hint_y_get(const home_base_ui_info_t *info)
{
    (void)info;
    return HOME_MAIN_HINT_Y;
}

static uint16_t home_main_bg_res_id_get(const home_base_ui_info_t *info, uint8_t mode)
{
    (void)info;
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return BASE_RES_ID_STIMULATION_MODE_BG;
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_MODE_BG;

        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return BASE_RES_ID_PROGRAM_MODE_BG;
        default:
            return BASE_RES_ID_STIMULATION_MODE_BG;
    }
}

static uint32_t home_main_hint_color_get(uint8_t mode)
{
#if IS_SUPPORT_BASE_HOME_COMP_BG
    (void)mode;
    return HOME_MAIN_HINT_COLOR;
#else
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return HOME_MAIN_HINT_COLOR_STIMULATION;
        case BREAST_PUMP_LACTATION_MODE:
            return HOME_MAIN_HINT_COLOR_LACTATION;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return HOME_MAIN_HINT_COLOR_PROGRAM;
        default:
            return HOME_MAIN_HINT_COLOR_STIMULATION;
    }
#endif
}

static void home_main_obj_scale_set(lv_obj_t *obj, int32_t scale)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_update_layout(obj);
    lv_obj_set_style_transform_pivot_x(obj, lv_obj_get_width(obj) / 2, 0);
    lv_obj_set_style_transform_pivot_y(obj, lv_obj_get_height(obj) / 2, 0);
    lv_obj_set_style_transform_scale(obj, scale, 0);
}

static void home_main_mode_title_get(home_base_ui_info_t *info, uint8_t mode, char *buf, size_t buf_len)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            snprintf(buf, buf_len, "%s", UI_STR_MODE_STIMULATION);
            break;
        case BREAST_PUMP_LACTATION_MODE:
            snprintf(buf, buf_len, "%s", UI_STR_MODE_EXPRESSION);
            break;
        case BREAST_PUMP_MIX_MODE:
            snprintf(buf, buf_len, "%s", UI_STR_MODE_MIXED);
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

static void home_main_bg_detach(home_base_ui_info_t *info)
{
    if (info->main.bg == NULL || !lv_obj_is_valid(info->main.bg)) {
        return;
    }

    const void *src = lv_image_get_src(info->main.bg);
    if (src == NULL) {
        return;
    }

    /* 系统主题预解码背景由 snapshot 模块持有，页面只解绑，不 destroy */
    if (get_predecoded_bg(info->main.shown_bg_id) == src) {
        lv_image_set_src(info->main.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->main.bg);
}

static void home_main_bg_update(home_base_ui_info_t *info, uint8_t mode)
{
    const uint16_t res_id = home_main_bg_res_id_get(info, mode);

    if (info->main.bg == NULL || !lv_obj_is_valid(info->main.bg)) {
        info->main.bg = lv_image_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.bg);
        lv_obj_move_background(info->main.bg);
    }
    if (info->main.shown_bg_id == res_id && info->force_update == false) {
        return;
    }

    home_main_bg_detach(info);
    {
        lv_draw_buf_t *predecoded = get_predecoded_bg(res_id);
        if (predecoded != NULL) {
            lv_image_set_src(info->main.bg, predecoded);
        } else {
            ui_img_png_theme_set_src(info->ui, info->main.bg, res_id,
                RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
        }
    }
    lv_obj_align(info->main.bg, LV_ALIGN_CENTER, 0, 0);
    info->main.shown_bg_id = res_id;
}

static void home_main_mode_title_update(home_base_ui_info_t *info, uint8_t mode)
{
    char title[HOME_MODE_NAME_LEN] = {0};

    home_main_mode_title_get(info, mode, title, sizeof(title));
    if (info->main.mode_title == NULL || !lv_obj_is_valid(info->main.mode_title)) {
        info->main.mode_title = lv_label_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.mode_title);
        lv_obj_set_style_text_color(info->main.mode_title, lv_color_hex(HOME_MAIN_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->main.mode_title, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_align(info->main.mode_title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.mode_title, 0, 0); // Figma 2%: 26 * 0.02 ≈ 0.52 → 1px
    }

    if (strncmp(info->main.shown_title, title, sizeof(info->main.shown_title)) == 0 &&
        info->force_update == false) {
        return;
    }
    lv_label_set_text(info->main.mode_title, title);
    lv_obj_align(info->main.mode_title, LV_ALIGN_TOP_MID, 0, HOME_MAIN_MODE_TITLE_Y);
    strncpy(info->main.shown_title, title, sizeof(info->main.shown_title) - 1);
    info->main.shown_title[sizeof(info->main.shown_title) - 1] = '\0';
}

static void home_main_obj_flag_set(lv_obj_t *obj, bool visible)
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

static void home_main_time_pair_update(lv_obj_t *labels[HOME_MAIN_TIME_DIGIT_COUNT], lv_obj_t *parent,
    uint8_t value, const lv_font_t *font, lv_coord_t seg_x, lv_coord_t seg_y, lv_coord_t seg_width,
    bool hidden)
{
    const lv_coord_t cell_w = seg_width / HOME_MAIN_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < HOME_MAIN_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(HOME_MAIN_TIME_COLOR), 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_obj_set_style_text_font(labels[i], font, 0);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
        home_main_obj_flag_set(labels[i], !hidden);
    }
}

static void home_main_time_h_update(home_base_ui_info_t *info, uint8_t hours, uint8_t digit_size)
{
    const bool hidden = (digit_size == HOME_MAIN_TIME_2SEG);

    if (info->main.shown_hours == hours && info->main.shown_digit_size == digit_size &&
        info->force_update == false) {
        return;
    }

    home_main_time_pair_update(info->main.time_h, home_page_parent_get(info), hours,
        UI_FONT_NUM_MD, HOME_MAIN_TIME_3SEG_H_X, HOME_MAIN_TIME_3SEG_Y,
        HOME_MAIN_TIME_3SEG_WIDTH, hidden);
    info->main.shown_hours = hours;
}

static void home_main_time_m_update(home_base_ui_info_t *info, uint8_t minutes, uint8_t digit_size)
{
    if (info->main.shown_minutes == minutes && info->main.shown_digit_size == digit_size &&
        info->force_update == false) {
        return;
    }

    if (digit_size == HOME_MAIN_TIME_2SEG) {
        home_main_time_pair_update(info->main.time_m, home_page_parent_get(info), minutes,
            UI_FONT_NUM_LG, HOME_MAIN_TIME_2SEG_M_X, HOME_MAIN_TIME_2SEG_Y,
            HOME_MAIN_TIME_2SEG_WIDTH, false);
    } else {
        home_main_time_pair_update(info->main.time_m, home_page_parent_get(info), minutes,
            UI_FONT_NUM_MD, HOME_MAIN_TIME_3SEG_M_X, HOME_MAIN_TIME_3SEG_Y,
            HOME_MAIN_TIME_3SEG_WIDTH, false);
    }
    info->main.shown_minutes = minutes;
}

static void home_main_time_s_update(home_base_ui_info_t *info, uint8_t seconds, uint8_t digit_size)
{
    if (info->main.shown_seconds == seconds && info->main.shown_digit_size == digit_size &&
        info->force_update == false) {
        return;
    }

    if (digit_size == HOME_MAIN_TIME_2SEG) {
        home_main_time_pair_update(info->main.time_s, home_page_parent_get(info), seconds,
            UI_FONT_NUM_LG, HOME_MAIN_TIME_2SEG_S_X, HOME_MAIN_TIME_2SEG_Y,
            HOME_MAIN_TIME_2SEG_WIDTH, false);
    } else {
        home_main_time_pair_update(info->main.time_s, home_page_parent_get(info), seconds,
            UI_FONT_NUM_MD, HOME_MAIN_TIME_3SEG_S_X, HOME_MAIN_TIME_3SEG_Y,
            HOME_MAIN_TIME_3SEG_WIDTH, false);
    }
    info->main.shown_seconds = seconds;
}

static void home_main_time_colon_release(home_base_ui_info_t *info)
{
    int i;

    for (i = 0; i < HOME_MAIN_TIME_COLON_COUNT; i++) {
        if (info->main.time_colon[i] != NULL && lv_obj_is_valid(info->main.time_colon[i])) {
            lv_image_set_src(info->main.time_colon[i], NULL);
        }
    }
    if (info->main.time_colon_buf != NULL) {
        lv_draw_buf_destroy(info->main.time_colon_buf);
        info->main.time_colon_buf = NULL;
    }
    ui_img_baj_release_decoded_buf(info->main.time_colon_2seg);
}

static void home_main_time_colon_3seg_update(home_base_ui_info_t *info)
{
    const lv_coord_t colon_x[HOME_MAIN_TIME_COLON_COUNT] = {
        HOME_MAIN_TIME_COLON_HM_X,
        HOME_MAIN_TIME_COLON_MS_X,
    };
    lv_obj_t *parent = home_page_parent_get(info);
    int i;

    for (i = 0; i < HOME_MAIN_TIME_COLON_COUNT; i++) {
        if (info->main.time_colon[i] == NULL || !lv_obj_is_valid(info->main.time_colon[i])) {
            info->main.time_colon[i] = lv_image_create(parent);
            EXCEPTION_CHECK_VOID(info->main.time_colon[i]);
        }
    }

    if (info->main.time_colon_buf == NULL) {
        info->main.time_colon_buf = ui_img_baj_theme_decode_draw_buf(info->ui,
            BASE_RES_ID_HOUR_COLON_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    EXCEPTION_CHECK_VOID(info->main.time_colon_buf);

    for (i = 0; i < HOME_MAIN_TIME_COLON_COUNT; i++) {
        lv_image_set_src(info->main.time_colon[i], info->main.time_colon_buf);
        lv_obj_align(info->main.time_colon[i], LV_ALIGN_TOP_LEFT,
            colon_x[i], HOME_MAIN_TIME_COLON_3SEG_Y);
        home_main_obj_flag_set(info->main.time_colon[i], true);
    }
}

static void home_main_time_colon_2seg_update(home_base_ui_info_t *info)
{
    lv_obj_t *parent = home_page_parent_get(info);
    int i;

    if (info->main.time_colon_2seg == NULL || !lv_obj_is_valid(info->main.time_colon_2seg)) {
        info->main.time_colon_2seg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->main.time_colon_2seg);
        ui_img_baj_theme_set_src(info->ui, info->main.time_colon_2seg, BASE_RES_ID_CLOCK_COLON_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->main.time_colon_2seg, LV_ALIGN_TOP_MID,
        HOME_MAIN_TIME_2SEG_COLON_X, HOME_MAIN_TIME_2SEG_COLON_Y);
    home_main_obj_flag_set(info->main.time_colon_2seg, true);

    for (i = 0; i < HOME_MAIN_TIME_COLON_COUNT; i++) {
        home_main_obj_flag_set(info->main.time_colon[i], false);
    }
}

static void home_main_time_colon_update(home_base_ui_info_t *info, uint8_t digit_size)
{
    if (info->main.shown_digit_size == digit_size && info->force_update == false &&
        ((digit_size == HOME_MAIN_TIME_2SEG &&
          info->main.time_colon_2seg != NULL && lv_obj_is_valid(info->main.time_colon_2seg)) ||
         (digit_size == HOME_MAIN_TIME_3SEG &&
          info->main.time_colon_buf != NULL))) {
        return;
    }

    if (digit_size == HOME_MAIN_TIME_2SEG) {
        home_main_time_colon_2seg_update(info);
    } else {
        home_main_obj_flag_set(info->main.time_colon_2seg, false);
        home_main_time_colon_3seg_update(info);
    }
}

static void home_main_time_update(home_base_ui_info_t *info, uint8_t hours, uint8_t minutes,
    uint8_t seconds)
{
    const uint8_t digit_size = (hours == 0) ? HOME_MAIN_TIME_2SEG : HOME_MAIN_TIME_3SEG;

    home_main_time_h_update(info, hours, digit_size);
    home_main_time_m_update(info, minutes, digit_size);
    home_main_time_s_update(info, seconds, digit_size);
    home_main_time_colon_update(info, digit_size);
    info->main.shown_digit_size = digit_size;
}

static void home_main_hint_style_get(const home_base_ui_info_t *info, ui_shine_style_t *style)
{
    ui_shine_style_init(style);
    style->mask_w = HOME_MAIN_MASK_W;
    style->mask_y = home_main_hint_y_get(info);
    style->screen_w = HOME_MAIN_SCREEN_W;
}

static uint8_t home_main_hint_type_get(const ui_proc_home_t *proc)
{
    if (proc == NULL) {
        return HOME_HINT_TYPE_NORMAL;
    }
    return proc->hint_low_bat;
}

static const char *home_main_hint_text_get(uint8_t hint_type)
{
    if (hint_type == HOME_HINT_TYPE_FORBID_START) {
        return HOME_MAIN_HINT_TEXT_FORBID_START;
    }
    if (hint_type == HOME_HINT_TYPE_LOW_BAT) {
        return HOME_MAIN_HINT_TEXT_LOW_BAT;
    }
    return HOME_MAIN_HINT_TEXT;
}

static uint8_t home_main_hint_line_space_get(uint8_t hint_type)
{
    return (hint_type == HOME_HINT_TYPE_LOW_BAT) ? HOME_MAIN_HINT_TEXT_LOW_BAT_LINE_SPACE : 0;
}

static void home_main_hint_text_update(home_base_ui_info_t *info, uint8_t mode,
    const ui_proc_home_t *proc)
{
    const uint32_t color = home_main_hint_color_get(mode);
    const uint8_t hint_type = home_main_hint_type_get(proc);
    const char *text = home_main_hint_text_get(hint_type);

    if (info->main.hint_text != NULL && lv_obj_is_valid(info->main.hint_text) &&
        info->main.shown_hint_color == color && info->main.shown_hint_low_bat == hint_type &&
        info->force_update == false) {
        return;
    }

    if (info->main.hint_text == NULL || !lv_obj_is_valid(info->main.hint_text)) {
        info->main.hint_text = lv_label_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.hint_text);
        lv_obj_set_style_text_font(info->main.hint_text, UI_FONT_HINT, 0);
        lv_obj_set_style_text_align(info->main.hint_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.hint_text, 0, 0); // Figma 2%: 19 * 0.02 ≈ 0.38 → 1px
    }
    lv_obj_set_style_text_line_space(info->main.hint_text,
        home_main_hint_line_space_get(hint_type), 0);
    lv_obj_set_style_text_color(info->main.hint_text, lv_color_hex(color), 0);
    lv_label_set_text(info->main.hint_text, text);
    lv_obj_align(info->main.hint_text, LV_ALIGN_TOP_MID, 0, home_main_hint_y_get(info));
    info->main.shown_hint_color = color;
    info->main.shown_hint_low_bat = hint_type;
}

/** 裁剪窗内亮色扫光：字库栅格 + TEXT_SHINE_MASK 剖面烘焙软边 */
static void home_main_hint_mask_update(home_base_ui_info_t *info)
{
    lv_obj_t *parent = home_page_parent_get(info);
    ui_shine_style_t style;
    const char *text = home_main_hint_text_get(info->main.shown_hint_low_bat);

    if (info->main.hint_shine.mask != NULL && lv_obj_is_valid(info->main.hint_shine.mask) &&
        info->main.hint_shine.shine != NULL && lv_obj_is_valid(info->main.hint_shine.shine) &&
        info->force_update == false) {
        return;
    }

    home_main_hint_style_get(info, &style);
    ui_shine_prepare_from_text(&info->main.hint_shine, &style,
        text, UI_FONT_HINT, 0,
        home_main_hint_line_space_get(info->main.shown_hint_low_bat));
    ui_shine_update(&info->main.hint_shine, parent, &style, info->main.hint_text, 1);
}

static void home_main_hint_update(home_base_ui_info_t *info, uint8_t mode,
    const ui_proc_home_t *proc)
{
    const uint8_t prev_low_bat = info->main.shown_hint_low_bat;

    home_main_hint_text_update(info, mode, proc);
    if (info->main.shown_hint_low_bat != prev_low_bat) {
        ui_shine_remove(&info->main.hint_shine);
    }
    home_main_hint_mask_update(info);
}

static void home_main_hint_shine_anim_update(home_base_ui_info_t *info)
{
    ui_shine_style_t style;
    uint8_t paused;

    home_main_hint_style_get(info, &style);
    paused = info->animation.paused_by_transition ? 1 : 0;
    ui_shine_anim_update(&info->main.hint_shine, &style, info->main.hint_text, paused);
}

static void home_main_speed_update(home_base_ui_info_t *info, uint8_t speed)
{
    char speed_text[8] = {0};
    lv_coord_t icon_w = 0;
    lv_coord_t value_w = 0;
    lv_coord_t total_w;
    lv_coord_t start_x;

    if (info->main.speed_icon == NULL || !lv_obj_is_valid(info->main.speed_icon)) {
        info->main.speed_icon = lv_image_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.speed_icon);
        ui_img_baj_theme_set_src(info->ui, info->main.speed_icon,
            BASE_RES_ID_SPEED_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->main.speed_value == NULL || !lv_obj_is_valid(info->main.speed_value)) {
        info->main.speed_value = lv_label_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.speed_value);
        lv_obj_set_style_text_color(info->main.speed_value, lv_color_hex(HOME_MAIN_SPEED_COLOR), 0);
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
    total_w = icon_w + HOME_MAIN_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > HOME_MAIN_SPEED_GROUP_WIDTH) {
        total_w = HOME_MAIN_SPEED_GROUP_WIDTH;
    }
    start_x = (HOME_MAIN_SCREEN_W - total_w) / 2;
    lv_obj_align(info->main.speed_icon, LV_ALIGN_TOP_LEFT, start_x,
        HOME_MAIN_SPEED_ICON_Y);
    lv_obj_align(info->main.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + HOME_MAIN_SPEED_ICON_VALUE_GAP,
        HOME_MAIN_SPEED_VALUE_Y);
    info->main.shown_speed = speed;
}

static void home_main_level_label_update(lv_obj_t **label, lv_obj_t *parent, const char *prefix,
    uint8_t level, lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_text_align_t align,
    uint8_t *shown_level, bool force)
{
    char text[8] = {0};

    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
        lv_label_set_long_mode(*label, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(*label, width);
        lv_obj_set_style_text_color(*label, lv_color_hex(HOME_MAIN_LEVEL_COLOR), 0);
        lv_obj_set_style_text_font(*label, UI_FONT_AEONIK, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 3, 0); // Figma 16%: 26 * 0.16 ≈ 4.16 → 4px
    }
    if (*shown_level == level && force == false) {
        return;
    }
    snprintf(text, sizeof(text), "%s%u", prefix, (unsigned)level);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
    *shown_level = level;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_main_page_ui_build(home_base_ui_info_t *info)
{
    lv_obj_t *parent;
    const uint8_t mode = info->mode.new_index;
#if IS_SUPPORT_BASE_HOME_COMP_BG
    const uint8_t left_level = 9;
    const uint8_t right_level = 13;
#else
    const uint8_t left_level = info->level.left;
    const uint8_t right_level = info->level.right;
#endif

    EXCEPTION_CHECK_VOID(info);
    ui_shine_remove(&info->main.hint_shine);
    home_main_time_colon_release(info);
    memset(&info->main, 0, sizeof(info->main));
    info->main.shown_bg_id = 0;
    info->main.shown_hint_color = 0xFFFFFFFFU;
    info->main.shown_hint_low_bat = 0xFF;
    info->main.shown_speed = 0xFF;
    info->main.shown_left_level = 0xFF;
    info->main.shown_right_level = 0xFF;
    info->main.shown_hours = 0xFF;
    info->main.shown_minutes = 0xFF;
    info->main.shown_seconds = 0xFF;
    info->main.shown_digit_size = 0;
    info->animation.paused_by_transition = 0;

    parent = home_page_parent_get(info);
    EXCEPTION_CHECK_VOID(parent);
    /* peer 入场页需一次铺齐主页控件，避免动画中只见背景/标题 */
#if (IS_SUPPORT_BASE_HOME_COMP_BG == 0)
    home_main_bg_update(info, mode);
#endif
    home_main_mode_title_update(info, mode);
    home_main_time_update(info, 0, 0, 0);
    {
        ui_proc_home_t *proc = NULL;

        if (info->ui != NULL) {
            proc = (ui_proc_home_t *)info->ui->priv.play_info.user_data;
        }
        home_main_hint_update(info, mode, proc);
    }
    home_main_speed_update(info, 0);
    home_main_level_label_update(&info->main.left_level, parent, "L",
        left_level, HOME_MAIN_LEFT_LEVEL_X,
        HOME_MAIN_LEFT_LEVEL_Y, HOME_MAIN_LEFT_LEVEL_W,
        LV_TEXT_ALIGN_RIGHT, &info->main.shown_left_level, true);
    home_main_level_label_update(&info->main.right_level, parent, "R",
        right_level, HOME_MAIN_RIGHT_LEVEL_X,
        HOME_MAIN_RIGHT_LEVEL_Y, HOME_MAIN_RIGHT_LEVEL_W,
        LV_TEXT_ALIGN_LEFT, &info->main.shown_right_level, true);
}

void home_main_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc)
{
    const uint8_t mode = info->mode.new_index;
    #if IS_SUPPORT_BASE_HOME_COMP_BG
    const uint8_t left_level = 9;//info->level.left;
    const uint8_t right_level = 13;//info->level.right;
    #else
    const uint8_t left_level = info->level.left;
    const uint8_t right_level = info->level.right;
    #endif
    uint8_t speed = 0;
    EXCEPTION_CHECK_VOID(info && proc && proc->pump);
    pump_setting_t setting;
    READ_PUMP_SETTING_INFO(proc->pump, 0, setting);
    speed = setting.speed;

    #if (IS_SUPPORT_BASE_HOME_COMP_BG == 0)
    home_main_bg_update(info, mode);
    #endif
    home_main_mode_title_update(info, mode);
    home_main_time_update(info, 0, 0, 0);
    home_main_hint_update(info, mode, proc);
    home_main_hint_shine_anim_update(info);
    home_main_speed_update(info, speed);
    home_main_level_label_update(&info->main.left_level, home_page_parent_get(info), "L",
        left_level, HOME_MAIN_LEFT_LEVEL_X,
        HOME_MAIN_LEFT_LEVEL_Y, HOME_MAIN_LEFT_LEVEL_W,
        LV_TEXT_ALIGN_RIGHT, &info->main.shown_left_level, info->force_update != 0);
    home_main_level_label_update(&info->main.right_level, home_page_parent_get(info), "R",
        right_level, HOME_MAIN_RIGHT_LEVEL_X,
        HOME_MAIN_RIGHT_LEVEL_Y, HOME_MAIN_RIGHT_LEVEL_W,
        LV_TEXT_ALIGN_LEFT, &info->main.shown_right_level, info->force_update != 0);
    /* 缩放复位只在切页过渡里 apply；稳态每帧设 FULL 会反复 update_layout，拖帧率 */
}

void home_main_page_ui_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    home_main_bg_detach(info);
    home_main_time_colon_release(info);
    ui_img_baj_release_decoded_buf(info->main.speed_icon);
    ui_shine_remove(&info->main.hint_shine);
    memset(&info->main, 0, sizeof(info->main));
}

void home_main_page_transition_apply(home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning)
{
    EXCEPTION_CHECK_VOID(info);
    (void)hero_scale;
    (void)bg_scale;
    (void)transitioning;

    /* page0 不缩放，仅随 page 容器位移；保持原背景与控件 */
    if (info->main.bg != NULL && lv_obj_is_valid(info->main.bg)) {
        lv_obj_clear_flag(info->main.bg, LV_OBJ_FLAG_HIDDEN);
        home_main_obj_scale_set(info->main.bg, HOME_MAIN_SCALE_FULL);
    }
    home_main_obj_scale_set(info->main.mode_title, HOME_MAIN_SCALE_FULL);
    {
        uint8_t i;
        for (i = 0; i < HOME_MAIN_TIME_DIGIT_COUNT; i++) {
            home_main_obj_scale_set(info->main.time_h[i], HOME_MAIN_SCALE_FULL);
            home_main_obj_scale_set(info->main.time_m[i], HOME_MAIN_SCALE_FULL);
            home_main_obj_scale_set(info->main.time_s[i], HOME_MAIN_SCALE_FULL);
        }
        for (i = 0; i < HOME_MAIN_TIME_COLON_COUNT; i++) {
            home_main_obj_scale_set(info->main.time_colon[i], HOME_MAIN_SCALE_FULL);
        }
        home_main_obj_scale_set(info->main.time_colon_2seg, HOME_MAIN_SCALE_FULL);
    }
}

const home_main_page_ops_t home_base_main_page_ops = {
    .build = home_main_page_ui_build,
    .update = home_main_page_ui_update,
    .remove = home_main_page_ui_remove,
    .transition_apply = home_main_page_transition_apply,
};
