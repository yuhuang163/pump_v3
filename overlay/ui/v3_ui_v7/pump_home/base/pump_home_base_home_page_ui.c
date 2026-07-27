/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_HOME_MAIN_SCREEN_W                         (360)

#define PUMP_HOME_MAIN_MODE_TITLE_Y                     (64 + 4)
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_MODE_TITLE_COLOR                 (0x00FF00)
#else
#define PUMP_HOME_MAIN_MODE_TITLE_COLOR                 (0xFFFFFF)
#endif

/// 相对 home 主页时间上移 14
#define PUMP_HOME_MAIN_TIME_SEG_WIDTH                   (88)
#define PUMP_HOME_MAIN_TIME_M_X                         (75 + 4)
#define PUMP_HOME_MAIN_TIME_M_Y                         (124 + 4)
#define PUMP_HOME_MAIN_TIME_S_X                         (189 + 4)
#define PUMP_HOME_MAIN_TIME_S_Y                         (PUMP_HOME_MAIN_TIME_M_Y)
#define PUMP_HOME_MAIN_TIME_COLON_X                     (0)
#define PUMP_HOME_MAIN_TIME_COLON_Y                     (120 + 4)
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_TIME_COLOR                       (0x00FF00)
#else
#define PUMP_HOME_MAIN_TIME_COLOR                       (0xFFFFFF)
#endif

#define PUMP_HOME_MAIN_SPEED_GROUP_WIDTH                (47)
#define PUMP_HOME_MAIN_SPEED_ICON_Y                     (265 + 4)
#define PUMP_HOME_MAIN_SPEED_VALUE_Y                    (267 + 4)
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_SPEED_COLOR                      (0x00FF00)
#else
#define PUMP_HOME_MAIN_SPEED_COLOR                      (0xFFFFFF)
#endif
#define PUMP_HOME_MAIN_SPEED_ICON_VALUE_GAP             (6)

#define PUMP_HOME_MAIN_LEFT_LEVEL_X                     (72 + 4)
#define PUMP_HOME_MAIN_LEFT_LEVEL_Y                     (267 + 4)
#define PUMP_HOME_MAIN_LEFT_LEVEL_W                     (58)
#define PUMP_HOME_MAIN_RIGHT_LEVEL_X                    (222 + 4)
#define PUMP_HOME_MAIN_RIGHT_LEVEL_Y                    (PUMP_HOME_MAIN_LEFT_LEVEL_Y)
#define PUMP_HOME_MAIN_RIGHT_LEVEL_W                    (58)
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_LEVEL_COLOR                      (0x00FF00)
#else
#define PUMP_HOME_MAIN_LEVEL_COLOR                      (0xFFFFFF)
#endif

#define PUMP_HOME_MAIN_PLAY_ICON_Y                      (136)
#define PUMP_HOME_MAIN_HINT_Y                           (220)
#define PUMP_HOME_MAIN_HINT_TEXT                        "Press to resume"
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
#define PUMP_HOME_MAIN_HINT_COLOR                       (0x00FF00)
#define PUMP_HOME_MAIN_HINT_SHINE_COLOR                 (0x00FF00)
#else
#define PUMP_HOME_MAIN_HINT_COLOR_STIMULATION           (0xAD7C89)
#define PUMP_HOME_MAIN_HINT_COLOR_LACTATION             (0xB9D9FF)
#define PUMP_HOME_MAIN_HINT_COLOR_PROGRAM               (0xB195DF)
#define PUMP_HOME_MAIN_HINT_SHINE_COLOR                 (0xFFFFFF)
#endif
#define PUMP_HOME_MAIN_MASK_W                           (80)
#define PUMP_HOME_MAIN_MASK_H                           (40)
#define PUMP_HOME_MAIN_MASK_Y                           (PUMP_HOME_MAIN_HINT_Y - 8)
#define PUMP_HOME_MAIN_MASK_X_MIN                       (-PUMP_HOME_MAIN_MASK_W)
#define PUMP_HOME_MAIN_MASK_X_MAX                       (PUMP_HOME_MAIN_SCREEN_W)
#define PUMP_HOME_MAIN_MASK_PERIOD_MS                   (3200)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t pump_home_main_bg_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return BASE_RES_ID_SIMULATION_MODE_BG;
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_MODE_BG;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return BASE_RES_ID_PROGRAM_MODE_BG;
        default:
            return BASE_RES_ID_SIMULATION_MODE_BG;
    }
}

static void pump_home_main_mode_title_get(pump_home_base_ui_info_t *info, uint8_t mode,
    char *buf, size_t buf_len)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            snprintf(buf, buf_len, "Stimulation");
            break;
        case BREAST_PUMP_LACTATION_MODE:
            snprintf(buf, buf_len, "Expression");
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            snprintf(buf, buf_len, "Milk Boost");
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            snprintf(buf, buf_len, "Power Pumping");
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            snprintf(buf, buf_len, "Customized %s", info->mode.custom_id_str);
            break;
        default:
            snprintf(buf, buf_len, "");
            break;
    }
}

static uint32_t pump_home_main_hint_color_get(uint8_t mode)
{
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
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

static void pump_home_main_bg_detach(pump_home_base_ui_info_t *info)
{
    if (info->main.bg == NULL || !lv_obj_is_valid(info->main.bg)) {
        return;
    }

    const void *src = lv_image_get_src(info->main.bg);
    if (src == NULL) {
        return;
    }

    /* 预解码背景由 snapshot 模块持有，页面只解绑，不 destroy */
    if (get_predecoded_bg(info->main.shown_bg_id) == src) {
        lv_image_set_src(info->main.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->main.bg);
}

static void pump_home_main_bg_update(pump_home_base_ui_info_t *info, uint8_t mode)
{
    const uint16_t res_id = pump_home_main_bg_res_id_get(mode);
    lv_draw_buf_t *predecoded;

    if (info->main.bg == NULL || !lv_obj_is_valid(info->main.bg)) {
        info->main.bg = lv_image_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.bg);
        lv_obj_move_background(info->main.bg);
    }
    if (info->main.shown_bg_id == res_id && info->force_update == false) {
        return;
    }

    pump_home_main_bg_detach(info);
    predecoded = get_predecoded_bg(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(info->main.bg, predecoded);
    } else {
        ui_img_png_theme_set_src(info->ui, info->main.bg, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->main.bg, LV_ALIGN_CENTER, 0, 0);
    info->main.shown_bg_id = res_id;
}

static void pump_home_main_mode_title_update(pump_home_base_ui_info_t *info, uint8_t mode)
{
    char title[24] = {0};

    pump_home_main_mode_title_get(info, mode, title, sizeof(title));
    if (info->main.mode_title == NULL || !lv_obj_is_valid(info->main.mode_title)) {
        info->main.mode_title = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.mode_title);
        lv_obj_set_style_text_color(info->main.mode_title, lv_color_hex(PUMP_HOME_MAIN_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->main.mode_title, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(info->main.mode_title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.mode_title, 1, 0);
    }

    if (strncmp(info->main.shown_title, title, sizeof(info->main.shown_title)) == 0 &&
        info->force_update == false) {
        return;
    }
    lv_label_set_text(info->main.mode_title, title);
    lv_obj_align(info->main.mode_title, LV_ALIGN_TOP_MID, 0, PUMP_HOME_MAIN_MODE_TITLE_Y);
    strncpy(info->main.shown_title, title, sizeof(info->main.shown_title) - 1);
    info->main.shown_title[sizeof(info->main.shown_title) - 1] = '\0';
}

static void pump_home_main_time_pair_update(lv_obj_t *labels[PUMP_HOME_MAIN_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x, lv_coord_t seg_y)
{
    const lv_coord_t cell_w = PUMP_HOME_MAIN_TIME_SEG_WIDTH / PUMP_HOME_MAIN_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};

    for (int i = 0; i < PUMP_HOME_MAIN_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(PUMP_HOME_MAIN_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], &lv_font_aeonik_soft_69_semibold, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
    }
}

static void pump_home_main_time_m_update(pump_home_base_ui_info_t *info, uint8_t minutes)
{
    if (info->main.shown_minutes == minutes && info->force_update == false) {
        return;
    }

    pump_home_main_time_pair_update(info->main.time_m, pump_home_page_parent_get(info), minutes,
        PUMP_HOME_MAIN_TIME_M_X, PUMP_HOME_MAIN_TIME_M_Y);
    info->main.shown_minutes = minutes;
}

static void pump_home_main_time_s_update(pump_home_base_ui_info_t *info, uint8_t seconds)
{
    if (info->main.shown_seconds == seconds && info->force_update == false) {
        return;
    }

    pump_home_main_time_pair_update(info->main.time_s, pump_home_page_parent_get(info), seconds,
        PUMP_HOME_MAIN_TIME_S_X, PUMP_HOME_MAIN_TIME_S_Y);
    info->main.shown_seconds = seconds;
}

static void pump_home_main_time_colon_update(pump_home_base_ui_info_t *info)
{
    lv_obj_t *parent = pump_home_page_parent_get(info);

    if (info->main.time_colon != NULL && lv_obj_is_valid(info->main.time_colon) &&
        info->force_update == false) {
        return;
    }

    if (info->main.time_colon == NULL || !lv_obj_is_valid(info->main.time_colon)) {
        info->main.time_colon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->main.time_colon);
    }
    ui_img_baj_theme_set_src(info->ui, info->main.time_colon, BASE_RES_ID_CLOCK_COLON_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->main.time_colon, LV_ALIGN_TOP_MID,
        PUMP_HOME_MAIN_TIME_COLON_X, PUMP_HOME_MAIN_TIME_COLON_Y);
}

static void pump_home_main_time_visibility_set(pump_home_base_ui_info_t *info, bool visible)
{
    int i;

    for (i = 0; i < PUMP_HOME_MAIN_TIME_DIGIT_COUNT; i++) {
        pump_home_main_obj_flag_set(info->main.time_m[i], visible);
        pump_home_main_obj_flag_set(info->main.time_s[i], visible);
    }
    pump_home_main_obj_flag_set(info->main.time_colon, visible);
}

static void pump_home_main_time_update(pump_home_base_ui_info_t *info, uint8_t minutes,
    uint8_t seconds, bool paused)
{
    if (!paused) {
        pump_home_main_time_m_update(info, minutes);
        pump_home_main_time_s_update(info, seconds);
        pump_home_main_time_colon_update(info);
    }
    pump_home_main_time_visibility_set(info, !paused);
}

static void pump_home_main_play_icon_update(pump_home_base_ui_info_t *info, bool paused)
{
    if (!paused) {
        pump_home_main_obj_flag_set(info->main.play_icon, false);
        return;
    }

    if (info->main.play_icon == NULL || !lv_obj_is_valid(info->main.play_icon)) {
        info->main.play_icon = lv_image_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.play_icon);
        ui_img_baj_theme_set_src(info->ui, info->main.play_icon, BASE_RES_ID_PLAY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->main.play_icon, LV_ALIGN_TOP_MID, 0, PUMP_HOME_MAIN_PLAY_ICON_Y);
    pump_home_main_obj_flag_set(info->main.play_icon, true);
}

static void pump_home_main_hint_shine_sync(pump_home_base_ui_info_t *info)
{
    lv_coord_t text_x;
    lv_coord_t text_y;

    if (info->main.hint_text == NULL || !lv_obj_is_valid(info->main.hint_text) ||
        info->main.hint_shine == NULL || !lv_obj_is_valid(info->main.hint_shine) ||
        info->main.hint_mask == NULL || !lv_obj_is_valid(info->main.hint_mask)) {
        return;
    }

    lv_obj_update_layout(info->main.hint_text);
    text_x = lv_obj_get_x(info->main.hint_text);
    text_y = lv_obj_get_y(info->main.hint_text);
    lv_obj_set_pos(info->main.hint_shine,
        text_x - info->main.mask_x, text_y - PUMP_HOME_MAIN_MASK_Y);
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
        lv_obj_set_style_text_font(info->main.hint_text, &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(info->main.hint_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.hint_text, 0, 0);
    }
    lv_obj_set_style_text_color(info->main.hint_text, lv_color_hex(color), 0);
    lv_label_set_text(info->main.hint_text, PUMP_HOME_MAIN_HINT_TEXT);
    lv_obj_align(info->main.hint_text, LV_ALIGN_TOP_MID, 0, PUMP_HOME_MAIN_HINT_Y);
    pump_home_main_obj_flag_set(info->main.hint_text, true);
    info->main.shown_hint_color = color;
}

static void pump_home_main_hint_mask_update(pump_home_base_ui_info_t *info, bool paused)
{
    lv_obj_t *parent = pump_home_page_parent_get(info);

    if (!paused) {
        pump_home_main_obj_flag_set(info->main.hint_mask, false);
        return;
    }

    if (info->main.hint_mask != NULL && lv_obj_is_valid(info->main.hint_mask) &&
        info->main.hint_shine != NULL && lv_obj_is_valid(info->main.hint_shine) &&
        info->force_update == false && info->main.shown_paused != 0) {
        pump_home_main_obj_flag_set(info->main.hint_mask, true);
        pump_home_main_hint_shine_sync(info);
        return;
    }

    if (info->main.hint_mask == NULL || !lv_obj_is_valid(info->main.hint_mask)) {
        info->main.hint_mask = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(info->main.hint_mask);
        lv_obj_remove_style_all(info->main.hint_mask);
        lv_obj_set_size(info->main.hint_mask, PUMP_HOME_MAIN_MASK_W, PUMP_HOME_MAIN_MASK_H);
        lv_obj_set_style_bg_opa(info->main.hint_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(info->main.hint_mask, 0, 0);
        lv_obj_set_style_pad_all(info->main.hint_mask, 0, 0);
        lv_obj_clear_flag(info->main.hint_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(info->main.hint_mask, LV_OBJ_FLAG_CLICKABLE);
    }
    if (info->main.shown_paused != 1) {
        info->main.mask_x = PUMP_HOME_MAIN_MASK_X_MIN;
        info->main.mask_tick_ms = get_timestamp();
    }

    if (info->main.hint_shine == NULL || !lv_obj_is_valid(info->main.hint_shine)) {
        info->main.hint_shine = lv_label_create(info->main.hint_mask);
        EXCEPTION_CHECK_VOID(info->main.hint_shine);
        lv_obj_set_style_text_color(info->main.hint_shine,
            lv_color_hex(PUMP_HOME_MAIN_HINT_SHINE_COLOR), 0);
        lv_obj_set_style_text_font(info->main.hint_shine, &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(info->main.hint_shine, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.hint_shine, 0, 0);
    }
    lv_label_set_text(info->main.hint_shine, PUMP_HOME_MAIN_HINT_TEXT);
    lv_obj_align(info->main.hint_mask, LV_ALIGN_TOP_LEFT, info->main.mask_x, PUMP_HOME_MAIN_MASK_Y);
    lv_obj_move_foreground(info->main.hint_mask);
    pump_home_main_obj_flag_set(info->main.hint_mask, true);
    pump_home_main_hint_shine_sync(info);
}

static void pump_home_main_hint_update(pump_home_base_ui_info_t *info, uint8_t mode, bool paused)
{
    pump_home_main_hint_text_update(info, mode, paused);
    pump_home_main_hint_mask_update(info, paused);
}

static void pump_home_main_mask_anim_update(pump_home_base_ui_info_t *info, bool paused)
{
    uint32_t elapsed;
    int32_t x;

    if (!paused) {
        return;
    }
    if (info->main.hint_mask == NULL || !lv_obj_is_valid(info->main.hint_mask)) {
        return;
    }
    if (info->animation.paused_by_transition) {
        return;
    }

    elapsed = get_timestamp() - info->main.mask_tick_ms;
    if (elapsed >= PUMP_HOME_MAIN_MASK_PERIOD_MS) {
        info->main.mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = PUMP_HOME_MAIN_MASK_X_MIN +
        (int32_t)((PUMP_HOME_MAIN_MASK_X_MAX - PUMP_HOME_MAIN_MASK_X_MIN) * elapsed) /
        PUMP_HOME_MAIN_MASK_PERIOD_MS;
    if (x != info->main.mask_x) {
        info->main.mask_x = (int16_t)x;
        lv_obj_set_x(info->main.hint_mask, info->main.mask_x);
        pump_home_main_hint_shine_sync(info);
    }
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
        ui_img_baj_theme_set_src(info->ui, info->main.speed_icon, BASE_RES_ID_SPEED_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->main.speed_value == NULL || !lv_obj_is_valid(info->main.speed_value)) {
        info->main.speed_value = lv_label_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.speed_value);
        lv_obj_set_style_text_color(info->main.speed_value, lv_color_hex(PUMP_HOME_MAIN_SPEED_COLOR), 0);
        lv_obj_set_style_text_font(info->main.speed_value, &lv_font_aeonik_soft_26_medium, 0);
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
    lv_obj_align(info->main.speed_icon, LV_ALIGN_TOP_LEFT, start_x, PUMP_HOME_MAIN_SPEED_ICON_Y);
    lv_obj_align(info->main.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + PUMP_HOME_MAIN_SPEED_ICON_VALUE_GAP, PUMP_HOME_MAIN_SPEED_VALUE_Y);
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
        lv_obj_set_style_text_font(*label, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 4, 0);
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
void pump_home_main_page_ui_build(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    memset(&info->main, 0, sizeof(info->main));
    info->main.shown_bg_id = 0;
    info->main.shown_hint_color = 0xFFFFFFFFU;
    info->main.shown_speed = 0xFF;
    info->main.shown_left_level = 0xFF;
    info->main.shown_right_level = 0xFF;
    info->main.shown_minutes = 0xFF;
    info->main.shown_seconds = 0xFF;
    info->main.shown_paused = 0xFF;
    info->animation.paused_by_transition = 0;
}

void pump_home_main_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc)
{
    uint8_t mode;
    bool paused;
#if IS_SUPPORT_BASE_PUMP_HOME_COMP_BG
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
    uint8_t minutes;
    uint8_t seconds;

    EXCEPTION_CHECK_VOID(info && proc && proc->pump);
    mode = info->mode.new_index;
    paused = (proc->pump_status == BREAST_PUMP_PAUSE);
#if (IS_SUPPORT_BASE_PUMP_HOME_COMP_BG == 0)
    left_level = info->level.left;
    right_level = info->level.right;
#endif
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, setting);
    speed = setting.speed;
    READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    EXCEPTION_CHECK_VOID(result_info);
    hold_time = result_info->work_valid_time / 1000;
    minutes = (hold_time / 60) % 60;
    seconds = hold_time % 60;
#if (IS_SUPPORT_BASE_PUMP_HOME_COMP_BG == 0)
    pump_home_main_bg_update(info, mode);
#endif
    pump_home_main_mode_title_update(info, mode);
    pump_home_main_time_update(info, minutes, seconds, paused);
    pump_home_base_animation_ui_update(info);
    pump_home_main_play_icon_update(info, paused);
    pump_home_main_hint_update(info, mode, paused);
    pump_home_main_mask_anim_update(info, paused);
    pump_home_main_speed_update(info, speed);
    pump_home_main_level_label_update(&info->main.left_level, pump_home_page_parent_get(info), "L",
        left_level, PUMP_HOME_MAIN_LEFT_LEVEL_X, PUMP_HOME_MAIN_LEFT_LEVEL_Y,
        PUMP_HOME_MAIN_LEFT_LEVEL_W, LV_TEXT_ALIGN_RIGHT, &info->main.shown_left_level,
        info->force_update != 0);
    pump_home_main_level_label_update(&info->main.right_level, pump_home_page_parent_get(info), "R",
        right_level, PUMP_HOME_MAIN_RIGHT_LEVEL_X, PUMP_HOME_MAIN_RIGHT_LEVEL_Y,
        PUMP_HOME_MAIN_RIGHT_LEVEL_W, LV_TEXT_ALIGN_LEFT, &info->main.shown_right_level,
        info->force_update != 0);
    info->main.shown_paused = paused ? 1 : 0;
}

void pump_home_main_page_ui_remove(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    pump_home_main_bg_detach(info);
    ui_img_baj_release_decoded_buf(info->main.time_colon);
    ui_img_baj_release_decoded_buf(info->main.play_icon);
    ui_img_baj_release_decoded_buf(info->main.speed_icon);
    memset(&info->main, 0, sizeof(info->main));
}
