/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_MAIN_SCREEN_W                              (360)

#define HOME_MAIN_MODE_TITLE_Y                          (64 + 4)
#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_MODE_TITLE_COLOR                      (0x00FF00)
#else
#define HOME_MAIN_MODE_TITLE_COLOR                      (0xFFFFFF)
#endif

#define HOME_MAIN_TIME_SEG_WIDTH                        (88)
#define HOME_MAIN_TIME_M_X                              (75 + 4)
#define HOME_MAIN_TIME_M_Y                              (138 + 4)
#define HOME_MAIN_TIME_S_X                              (189 + 4)
#define HOME_MAIN_TIME_S_Y                              (HOME_MAIN_TIME_M_Y)
#define HOME_MAIN_TIME_COLON_X                          (0)
#define HOME_MAIN_TIME_COLON_Y                          (134 + 4)
#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_TIME_COLOR                            (0x00FF00)
#else
#define HOME_MAIN_TIME_COLOR                            (0xFFFFFF)
#endif

#define HOME_MAIN_HINT_Y                                (207 + 4)
#define HOME_MAIN_HINT_TEXT                             "Press to start pumping"
#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_HINT_COLOR                            (0x00FF00)
#define HOME_MAIN_HINT_SHINE_COLOR                      (0x00FF00)
#else
#define HOME_MAIN_HINT_COLOR_STIMULATION                (0xAD7C89)
#define HOME_MAIN_HINT_COLOR_LACTATION                  (0xB9D9FF)
#define HOME_MAIN_HINT_COLOR_PROGRAM                    (0xB195DF)
#define HOME_MAIN_HINT_SHINE_COLOR                      (0xFFFFFF)
#endif

#if HOME_MAIN_HINT_SHINE_SOFT_ENABLE
#define HOME_MAIN_MASK_W                                (140)
#define HOME_MAIN_MASK_H                                (80)
#define HOME_MAIN_MASK_Y                                (HOME_MAIN_HINT_Y - 28)
#else
#define HOME_MAIN_MASK_W                                (80)
#define HOME_MAIN_MASK_H                                (40)
#define HOME_MAIN_MASK_Y                                (HOME_MAIN_HINT_Y - 8)
#endif
#define HOME_MAIN_MASK_X_MIN                            (-HOME_MAIN_MASK_W)
#define HOME_MAIN_MASK_X_MAX                            (HOME_MAIN_SCREEN_W)
#define HOME_MAIN_MASK_PERIOD_MS                        (3200)

#define HOME_MAIN_SPEED_GROUP_WIDTH                     (47)
#define HOME_MAIN_SPEED_ICON_Y                          (265 + 4)
#define HOME_MAIN_SPEED_VALUE_Y                         (267 + 4)
#if IS_SUPPORT_BASE_HOME_COMP_BG
#define HOME_MAIN_SPEED_COLOR                           (0x00FF00)
#else
#define HOME_MAIN_SPEED_COLOR                           (0xFFFFFF)
#endif
#define HOME_MAIN_SPEED_ICON_VALUE_GAP                  (6)

#define HOME_MAIN_LEFT_LEVEL_X                          (72 + 4)
#define HOME_MAIN_LEFT_LEVEL_Y                          (267 + 4)
#define HOME_MAIN_LEFT_LEVEL_W                          (58)
#define HOME_MAIN_RIGHT_LEVEL_X                         (222 + 4)
#define HOME_MAIN_RIGHT_LEVEL_Y                         (HOME_MAIN_LEFT_LEVEL_Y)
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
static uint16_t home_main_bg_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return BASE_RES_ID_SIMULATION_MODE_BG;
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_MODE_BG;
        // case BREAST_PUMP_MIX_MODE:
        //     return BASE_RES_ID_MIXED_MODE_BG;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return BASE_RES_ID_PROGRAM_MODE_BG;
        default:
            return BASE_RES_ID_SIMULATION_MODE_BG;
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

static void home_main_mode_title_get(home_base_ui_info_t *info, uint8_t mode, char *buf, size_t buf_len)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            snprintf(buf, buf_len, "Stimulation");
            break;
        case BREAST_PUMP_LACTATION_MODE:
            snprintf(buf, buf_len, "Expression");
            break;
        case BREAST_PUMP_MIX_MODE:
            snprintf(buf, buf_len, "Mixed");
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

static void home_main_bg_detach(home_base_ui_info_t *info)
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

static void home_main_bg_update(home_base_ui_info_t *info, uint8_t mode)
{
    const uint16_t res_id = home_main_bg_res_id_get(mode);
    lv_draw_buf_t *predecoded;

    if (info->main.bg == NULL || !lv_obj_is_valid(info->main.bg)) {
        info->main.bg = lv_image_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.bg);
        lv_obj_move_background(info->main.bg);
    }
    if (info->main.shown_bg_id == res_id && info->force_update == false) {
        return;
    }

    home_main_bg_detach(info);
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

static void home_main_mode_title_update(home_base_ui_info_t *info, uint8_t mode)
{
    char title[24] = {0};

    home_main_mode_title_get(info, mode, title, sizeof(title));
    if (info->main.mode_title == NULL || !lv_obj_is_valid(info->main.mode_title)) {
        info->main.mode_title = lv_label_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.mode_title);
        lv_obj_set_style_text_color(info->main.mode_title, lv_color_hex(HOME_MAIN_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->main.mode_title, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(info->main.mode_title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.mode_title, 1, 0); // Figma 2%: 26 * 0.02 ≈ 0.52 → 1px
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

static void home_main_time_pair_update(lv_obj_t *labels[HOME_MAIN_TIME_DIGIT_COUNT], lv_obj_t *parent,
    uint8_t value, lv_coord_t seg_x, lv_coord_t seg_y)
{
    const lv_coord_t cell_w = HOME_MAIN_TIME_SEG_WIDTH / HOME_MAIN_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};

    for (int i = 0; i < HOME_MAIN_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(HOME_MAIN_TIME_COLOR), 0);
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

static void home_main_time_m_update(home_base_ui_info_t *info, uint8_t minutes)
{
    if (info->main.shown_minutes == minutes && info->force_update == false) {
        return;
    }

    home_main_time_pair_update(info->main.time_m, home_page_parent_get(info), minutes,
        HOME_MAIN_TIME_M_X, HOME_MAIN_TIME_M_Y);
    info->main.shown_minutes = minutes;
}

static void home_main_time_s_update(home_base_ui_info_t *info, uint8_t seconds)
{
    if (info->main.shown_seconds == seconds && info->force_update == false) {
        return;
    }

    home_main_time_pair_update(info->main.time_s, home_page_parent_get(info), seconds,
        HOME_MAIN_TIME_S_X, HOME_MAIN_TIME_S_Y);
    info->main.shown_seconds = seconds;
}

static void home_main_time_colon_update(home_base_ui_info_t *info)
{
    lv_obj_t *parent = home_page_parent_get(info);

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
        HOME_MAIN_TIME_COLON_X, HOME_MAIN_TIME_COLON_Y);
}

static void home_main_time_update(home_base_ui_info_t *info, uint8_t minutes, uint8_t seconds)
{
    home_main_time_m_update(info, minutes);
    home_main_time_s_update(info, seconds);
    home_main_time_colon_update(info);
}

static void home_main_hint_shine_sync(home_base_ui_info_t *info)
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
        text_x - info->main.mask_x, text_y - HOME_MAIN_MASK_Y);
}

#if HOME_MAIN_HINT_SHINE_SOFT_ENABLE
static lv_draw_buf_t *home_main_hint_mask_a8_from_rgba(const lv_draw_buf_t *rgba)
{
    lv_draw_buf_t *a8;
    uint32_t w;
    uint32_t h;
    uint32_t y;
    uint32_t x;

    if (rgba == NULL || rgba->data == NULL) {
        return NULL;
    }
    if (rgba->header.cf != LV_COLOR_FORMAT_ARGB8888 &&
        rgba->header.cf != LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED) {
        return NULL;
    }

    w = rgba->header.w;
    h = rgba->header.h;
    a8 = lv_draw_buf_create(w, h, LV_COLOR_FORMAT_A8, LV_STRIDE_AUTO);
    if (a8 == NULL || a8->data == NULL) {
        if (a8 != NULL) {
            lv_draw_buf_destroy(a8);
        }
        return NULL;
    }

    for (y = 0; y < h; y++) {
        const lv_color32_t *src =
            (const lv_color32_t *)(rgba->data + y * rgba->header.stride);
        uint8_t *dst = a8->data + y * a8->header.stride;
        for (x = 0; x < w; x++) {
            uint32_t a = src[x].alpha;
            /* 抛物线压两端：两侧更透、中间更亮，软边更明显 */
            uint32_t edge = (x * (w - 1U - x) * 4U * 255U) / (w * w);
            if (edge > 255U) {
                edge = 255U;
            }
            a = (a * edge) / 255U;
            /* 再平方一次，拉大两端与中心的对比 */
            a = (a * a) / 255U;
            dst[x] = (uint8_t)a;
        }
    }
    return a8;
}

static void home_main_hint_mask_a8_release(home_base_ui_info_t *info)
{
    if (info == NULL || info->main.hint_mask_a8 == NULL) {
        return;
    }
    if (info->main.hint_mask != NULL && lv_obj_is_valid(info->main.hint_mask)) {
        lv_obj_set_style_bitmap_mask_src(info->main.hint_mask, NULL, 0);
    }
    lv_draw_buf_destroy(info->main.hint_mask_a8);
    info->main.hint_mask_a8 = NULL;
}

static void home_main_hint_mask_a8_prepare(home_base_ui_info_t *info, lv_obj_t *parent)
{
    lv_obj_t *tmp_img;
    const lv_draw_buf_t *rgba;

    if (info->main.hint_mask_a8 != NULL) {
        return;
    }

    tmp_img = lv_image_create(parent);
    EXCEPTION_CHECK_VOID(tmp_img);
    lv_obj_add_flag(tmp_img, LV_OBJ_FLAG_HIDDEN);
    if (ui_img_png_theme_set_src(info->ui, tmp_img, BASE_RES_ID_HOME_TEXT_MASK,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888) == 0) {
        rgba = (const lv_draw_buf_t *)lv_image_get_src(tmp_img);
        info->main.hint_mask_a8 = home_main_hint_mask_a8_from_rgba(rgba);
    }
    ui_img_png_release_decoded_buf(tmp_img);
    lv_obj_delete(tmp_img);

    if (info->main.hint_mask_a8 != NULL && info->main.hint_mask != NULL &&
        lv_obj_is_valid(info->main.hint_mask)) {
        lv_obj_set_style_bitmap_mask_src(info->main.hint_mask, info->main.hint_mask_a8, 0);
    }
}
#endif /* HOME_MAIN_HINT_SHINE_SOFT_ENABLE */

static void home_main_hint_text_update(home_base_ui_info_t *info, uint8_t mode)
{
    const uint32_t color = home_main_hint_color_get(mode);

    if (info->main.hint_text != NULL && lv_obj_is_valid(info->main.hint_text) &&
        info->main.shown_hint_color == color && info->force_update == false) {
        return;
    }

    if (info->main.hint_text == NULL || !lv_obj_is_valid(info->main.hint_text)) {
        info->main.hint_text = lv_label_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.hint_text);
        lv_obj_set_style_text_font(info->main.hint_text, &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(info->main.hint_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.hint_text, 1, 0); // Figma 2%: 19 * 0.02 ≈ 0.38 → 1px
    }
    lv_obj_set_style_text_color(info->main.hint_text, lv_color_hex(color), 0);
    lv_label_set_text(info->main.hint_text, HOME_MAIN_HINT_TEXT);
    lv_obj_align(info->main.hint_text, LV_ALIGN_TOP_MID, 0, HOME_MAIN_HINT_Y);
    info->main.shown_hint_color = color;
}

static void home_main_hint_mask_update(home_base_ui_info_t *info)
{
    lv_obj_t *parent = home_page_parent_get(info);

    if (info->main.hint_mask != NULL && lv_obj_is_valid(info->main.hint_mask) &&
        info->main.hint_shine != NULL && lv_obj_is_valid(info->main.hint_shine) &&
        info->force_update == false) {
        home_main_hint_shine_sync(info);
        return;
    }

    if (info->main.hint_mask == NULL || !lv_obj_is_valid(info->main.hint_mask)) {
        info->main.hint_mask = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(info->main.hint_mask);
        lv_obj_remove_style_all(info->main.hint_mask);
        lv_obj_set_size(info->main.hint_mask, HOME_MAIN_MASK_W, HOME_MAIN_MASK_H);
        lv_obj_set_style_bg_opa(info->main.hint_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(info->main.hint_mask, 0, 0);
        lv_obj_set_style_pad_all(info->main.hint_mask, 0, 0);
        lv_obj_clear_flag(info->main.hint_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(info->main.hint_mask, LV_OBJ_FLAG_CLICKABLE);
        info->main.mask_x = HOME_MAIN_MASK_X_MIN;
        info->main.mask_tick_ms = get_timestamp();
    }

    if (info->main.hint_shine == NULL || !lv_obj_is_valid(info->main.hint_shine)) {
        info->main.hint_shine = lv_label_create(info->main.hint_mask);
        EXCEPTION_CHECK_VOID(info->main.hint_shine);
        lv_obj_set_style_text_color(info->main.hint_shine, lv_color_hex(HOME_MAIN_HINT_SHINE_COLOR), 0);
        lv_obj_set_style_text_font(info->main.hint_shine, &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(info->main.hint_shine, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(info->main.hint_shine, 1, 0);
    }
    lv_label_set_text(info->main.hint_shine, HOME_MAIN_HINT_TEXT);

#if HOME_MAIN_HINT_SHINE_SOFT_ENABLE
    home_main_hint_mask_a8_prepare(info, parent);
#endif

    lv_obj_align(info->main.hint_mask, LV_ALIGN_TOP_LEFT, info->main.mask_x, HOME_MAIN_MASK_Y);
    lv_obj_move_foreground(info->main.hint_mask);
    home_main_hint_shine_sync(info);
}

static void home_main_hint_update(home_base_ui_info_t *info, uint8_t mode)
{
    home_main_hint_text_update(info, mode);
    home_main_hint_mask_update(info);
}

static void home_main_mask_anim_update(home_base_ui_info_t *info)
{
    uint32_t elapsed;
    int32_t x;

    if (info->main.hint_mask == NULL || !lv_obj_is_valid(info->main.hint_mask)) {
        return;
    }
    if (info->animation.paused_by_transition) {
        return;
    }

    elapsed = get_timestamp() - info->main.mask_tick_ms;
    if (elapsed >= HOME_MAIN_MASK_PERIOD_MS) {
        info->main.mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = HOME_MAIN_MASK_X_MIN +
        (int32_t)((HOME_MAIN_MASK_X_MAX - HOME_MAIN_MASK_X_MIN) * elapsed) / HOME_MAIN_MASK_PERIOD_MS;
    if (x != info->main.mask_x) {
        info->main.mask_x = (int16_t)x;
        lv_obj_set_x(info->main.hint_mask, info->main.mask_x);
        home_main_hint_shine_sync(info);
    }
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
        ui_img_baj_theme_set_src(info->ui, info->main.speed_icon, BASE_RES_ID_SPEED_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->main.speed_value == NULL || !lv_obj_is_valid(info->main.speed_value)) {
        info->main.speed_value = lv_label_create(home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->main.speed_value);
        lv_obj_set_style_text_color(info->main.speed_value, lv_color_hex(HOME_MAIN_SPEED_COLOR), 0);
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
    total_w = icon_w + HOME_MAIN_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > HOME_MAIN_SPEED_GROUP_WIDTH) {
        total_w = HOME_MAIN_SPEED_GROUP_WIDTH;
    }
    start_x = (HOME_MAIN_SCREEN_W - total_w) / 2;
    lv_obj_align(info->main.speed_icon, LV_ALIGN_TOP_LEFT, start_x, HOME_MAIN_SPEED_ICON_Y);
    lv_obj_align(info->main.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + HOME_MAIN_SPEED_ICON_VALUE_GAP, HOME_MAIN_SPEED_VALUE_Y);
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
        lv_obj_set_style_text_font(*label, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 4, 0); // Figma 16%: 26 * 0.16 ≈ 4.16 → 4px
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
    EXCEPTION_CHECK_VOID(info);
#if HOME_MAIN_HINT_SHINE_SOFT_ENABLE
    home_main_hint_mask_a8_release(info);
#endif
    memset(&info->main, 0, sizeof(info->main));
    info->main.shown_bg_id = 0;
    info->main.shown_hint_color = 0xFFFFFFFFU;
    info->main.shown_speed = 0xFF;
    info->main.shown_left_level = 0xFF;
    info->main.shown_right_level = 0xFF;
    info->main.shown_minutes = 0xFF;
    info->main.shown_seconds = 0xFF;
    info->animation.paused_by_transition = 0;
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
    home_main_time_update(info, 0, 0);
    home_main_hint_update(info, mode);
    home_main_mask_anim_update(info);
    home_main_speed_update(info, speed);
    home_main_level_label_update(&info->main.left_level, home_page_parent_get(info), "L",
        left_level, HOME_MAIN_LEFT_LEVEL_X, HOME_MAIN_LEFT_LEVEL_Y, HOME_MAIN_LEFT_LEVEL_W,
        LV_TEXT_ALIGN_RIGHT, &info->main.shown_left_level, info->force_update != 0);
    home_main_level_label_update(&info->main.right_level, home_page_parent_get(info), "R",
        right_level, HOME_MAIN_RIGHT_LEVEL_X, HOME_MAIN_RIGHT_LEVEL_Y, HOME_MAIN_RIGHT_LEVEL_W,
        LV_TEXT_ALIGN_LEFT, &info->main.shown_right_level, info->force_update != 0);
}

void home_main_page_ui_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    home_main_bg_detach(info);
    ui_img_baj_release_decoded_buf(info->main.time_colon);
    ui_img_baj_release_decoded_buf(info->main.speed_icon);
#if HOME_MAIN_HINT_SHINE_SOFT_ENABLE
    home_main_hint_mask_a8_release(info);
#endif
    memset(&info->main, 0, sizeof(info->main));
}
