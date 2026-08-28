/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_MODE_PAGE_DOT_DIAMETER                 (11)
#define PUMP_MODE_PAGE_DOT_COLOR_ACTIVE             (0xFFFFFF)
#define PUMP_MODE_PAGE_DOT_COLOR_INACTIVE           (0x4D4D4D)

/** 常规模式（刺激/吸乳）是否显示时长；默认关闭，开启后同韵律展示 ≈ Nmin */
#ifndef IS_SUPPORT_PUMP_MODE_PAGE_COMMON_DURATION
#define IS_SUPPORT_PUMP_MODE_PAGE_COMMON_DURATION   (0)
#endif
/** 刺激/吸乳时长文案固定分钟数 */
#define PUMP_MODE_PAGE_COMMON_DURATION_MIN          (30)

/** 刺激：ICON / 文案 / 时间 */
#define PUMP_MODE_PAGE_STIM_ICON_Y                  (87)
#define PUMP_MODE_PAGE_STIM_DURATION_Y              (242)
/** 吸乳：ICON / 文案 / 时间 */
#define PUMP_MODE_PAGE_EXPR_ICON_Y                  (89)
#define PUMP_MODE_PAGE_EXPR_DURATION_Y              (242)
/** 韵律单行：ICON / 文案 / 时间 */
#define PUMP_MODE_PAGE_RHYTHM_1L_ICON_Y             (88)
#define PUMP_MODE_PAGE_RHYTHM_1L_DURATION_Y         (242)
/** 韵律两行：ICON / 文案 / 时间 */
#define PUMP_MODE_PAGE_RHYTHM_2L_ICON_Y             (67)
#define PUMP_MODE_PAGE_RHYTHM_2L_DURATION_Y         (262)

#define PUMP_MODE_PAGE_TITLE_COLOR                  (0xFFFFFF)
#define PUMP_MODE_PAGE_DURATION_COLOR               (0x999999)
/** 模式标题单行超宽则换行；换行后按单/双行固定坐标排 ICON / 时间文案 */
/** 标题最长 32 字符；超出则裁进 shown 并启用 "..." */
#define PUMP_MODE_PAGE_TITLE_MAX_CHARS              (32 - 6)
#define PUMP_MODE_PAGE_TITLE_BUF_LEN                (PUMP_MODE_PAGE_TITLE_MAX_CHARS + 1)
#define PUMP_MODE_PAGE_TITLE_RAW_LEN                (64)
#define PUMP_MODE_PAGE_TITLE_DISPLAY_LEN            (40)

#define PUMP_MODE_PAGE_SCALE_FULL                   (256)
/** 5 点布局第 3 点 X 居中：(屏宽 - 直径) / 2 */
#define PUMP_MODE_PAGE_DOT_X_CENTER \
    ((PANLE_DISPLAY_W - PUMP_MODE_PAGE_DOT_DIAMETER) / 2)

#define PUMP_MODE_TIME_LABEL_Y                      (293)
#define PUMP_MODE_TIME_ICON_Y                       (311)
#define PUMP_MODE_TIME_Y                            (308)
#define PUMP_MODE_TIME_SEG_WIDTH                    (36)
#define PUMP_MODE_TIME_COLON_GAP                    (2)
#define PUMP_MODE_TIME_COLOR                        (0xFFFFFF)
#define PUMP_MODE_TIME_2SEG                         (2)
#define PUMP_MODE_TIME_3SEG                         (3)
/// 两段基准：图标与分钟起点；三段在左侧插入 HH:，MM:SS 位置保持不变
#define PUMP_MODE_TIME_2SEG_ICON_X                  (133)
#define PUMP_MODE_TIME_2SEG_M_X                     (168)
#define PUMP_MODE_TIME_COLON_W_FALLBACK             (8)
#if IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
/** 时间胶囊入场：淡入 + 上移（无抖动/缩放） */
#define PUMP_MODE_TIME_ENTER_ANIM_MS                (280)
#define PUMP_MODE_TIME_ENTER_Y_OFFSET               (24)
#define PUMP_MODE_TIME_ENTER_PROGRESS_FULL          (256)
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_mode_page_obj_scale_set(lv_obj_t *obj, int32_t scale)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_update_layout(obj);
    lv_obj_set_style_transform_pivot_x(obj, lv_obj_get_width(obj) / 2, 0);
    lv_obj_set_style_transform_pivot_y(obj, lv_obj_get_height(obj) / 2, 0);
    lv_obj_set_style_transform_scale(obj, scale, 0);
}

static void pump_mode_page_obj_flag_set(lv_obj_t *obj, bool visible)
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

static lv_obj_t *pump_mode_page_circle_bg_create(lv_obj_t *parent)
{
    lv_obj_t *circle = lv_obj_create(parent);

    EXCEPTION_CHECK_POINT(circle);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE, PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(circle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(circle);
    return circle;
}

static uint8_t pump_mode_page_mode_from_page(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_HOME_PAGE_MODE_0:
            return BREAST_PUMP_STIMULATION_MODE;
        case PUMP_HOME_PAGE_MODE_1:
            return BREAST_PUMP_LACTATION_MODE;
        case PUMP_HOME_PAGE_RHYTHM_0:
            return BREAST_PUMP_MILK_BOOST_MODE;
        case PUMP_HOME_PAGE_RHYTHM_1:
            return BREAST_PUMP_PP_MILK_TRACK_MODE;
        case PUMP_HOME_PAGE_RHYTHM_2:
            return BREAST_PUMP_CUSTOM_MODE;
        default:
            return BREAST_PUMP_STIMULATION_MODE;
    }
}

static uint16_t pump_mode_page_icon_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return BASE_RES_ID_SIMULATION_ICON;
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_ICON;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return BASE_RES_ID_PROGRAM_ICON;
        default:
            return BASE_RES_ID_SIMULATION_ICON;
    }
}

static void pump_mode_page_title_get(pump_home_base_ui_info_t *info, uint8_t mode,
    char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return;
    }

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
            snprintf(buf, buf_len, "%s", info->mode.custom_id_str);
            break;
        default:
            snprintf(buf, buf_len, "");
            break;
    }
}

/** 单行塞入 max_w 的最长字节前缀长度 */
static uint32_t pump_mode_page_title_fit_len(const char *text, uint32_t text_len,
    const lv_font_t *font, int32_t letter_space, int32_t max_w)
{
    char tmp[PUMP_MODE_PAGE_TITLE_BUF_LEN];
    uint32_t lo = 0;
    uint32_t hi;
    uint32_t best = 0;

    if (text == NULL || font == NULL || text_len == 0 || max_w <= 0) {
        return 0;
    }
    if (text_len >= sizeof(tmp)) {
        text_len = (uint32_t)sizeof(tmp) - 1U;
    }
    hi = text_len;

    while (lo <= hi) {
        uint32_t mid = (lo + hi) / 2U;
        lv_point_t size;

        memcpy(tmp, text, mid);
        tmp[mid] = '\0';
        lv_text_get_size(&size, tmp, font, letter_space, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (size.x <= max_w) {
            best = mid;
            if (mid == text_len) {
                break;
            }
            lo = mid + 1U;
        } else {
            if (mid == 0U) {
                break;
            }
            hi = mid - 1U;
        }
    }
    return best;
}

/**
 * 生成展示文案：
 * - 未超 32 且单行 ≤290 / 两行内可放下：原文 + WRAP
 * - 超 32 或两行仍不够：裁剪并加 "..."
 */
static void pump_mode_page_title_make_display(char *out, size_t out_len, const char *src,
    const lv_font_t *font, int32_t letter_space, bool force_ellipsis)
{
    lv_point_t size;
    uint32_t src_len;
    uint32_t line1_len;
    uint32_t line2_fit;
    const char *rest;
    int32_t ellipsis_w;
    uint32_t i;

    if (out == NULL || out_len == 0) {
        return;
    }
    out[0] = '\0';
    if (src == NULL || font == NULL) {
        return;
    }

    src_len = (uint32_t)strlen(src);
    if (src_len == 0U) {
        return;
    }

    if (!force_ellipsis) {
        lv_text_get_size(&size, src, font, letter_space, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (size.x <= PUMP_MODE_PAGE_TITLE_MAX_W) {
            strncpy(out, src, out_len - 1U);
            out[out_len - 1U] = '\0';
            return;
        }
        lv_text_get_size(&size, src, font, letter_space, 0, PUMP_MODE_PAGE_TITLE_MAX_W, LV_TEXT_FLAG_NONE);
        if (size.y <= (int32_t)font->line_height * 2) {
            strncpy(out, src, out_len - 1U);
            out[out_len - 1U] = '\0';
            return;
        }
    } else {
        char trial[PUMP_MODE_PAGE_TITLE_DISPLAY_LEN];

        snprintf(trial, sizeof(trial), "%s...", src);
        lv_text_get_size(&size, trial, font, letter_space, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (size.x <= PUMP_MODE_PAGE_TITLE_MAX_W) {
            strncpy(out, trial, out_len - 1U);
            out[out_len - 1U] = '\0';
            return;
        }
    }

    line1_len = pump_mode_page_title_fit_len(src, src_len, font, letter_space,
        PUMP_MODE_PAGE_TITLE_MAX_W);
    for (i = line1_len; i > 0U; i--) {
        if (src[i - 1U] == ' ') {
            line1_len = i - 1U;
            break;
        }
    }
    rest = src + line1_len;
    while (*rest == ' ') {
        rest++;
    }

    lv_text_get_size(&size, "...", font, letter_space, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    ellipsis_w = size.x;

    if (!force_ellipsis) {
        lv_text_get_size(&size, rest, font, letter_space, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (size.x <= PUMP_MODE_PAGE_TITLE_MAX_W) {
            snprintf(out, out_len, "%.*s\n%s", (int)line1_len, src, rest);
            return;
        }
    }

    if (ellipsis_w >= PUMP_MODE_PAGE_TITLE_MAX_W) {
        snprintf(out, out_len, "%.*s\n...", (int)line1_len, src);
        return;
    }
    line2_fit = pump_mode_page_title_fit_len(rest, (uint32_t)strlen(rest), font, letter_space,
        PUMP_MODE_PAGE_TITLE_MAX_W - ellipsis_w);
    snprintf(out, out_len, "%.*s\n%.*s...", (int)line1_len, src, (int)line2_fit, rest);
}

/** 入库：超过 32 字符截断并标记需 "..." */
static void pump_mode_page_title_store(pump_home_base_mode_page_ctx_t *ctx, const char *title)
{
    size_t len;

    EXCEPTION_CHECK_VOID(ctx && title);
    len = strlen(title);
    ctx->title_ellipsis = (len > PUMP_MODE_PAGE_TITLE_MAX_CHARS) ? 1U : 0U;
    if (ctx->title_ellipsis) {
        memcpy(ctx->shown_title, title, PUMP_MODE_PAGE_TITLE_MAX_CHARS);
        ctx->shown_title[PUMP_MODE_PAGE_TITLE_MAX_CHARS] = '\0';
    } else {
        strncpy(ctx->shown_title, title, sizeof(ctx->shown_title) - 1U);
        ctx->shown_title[sizeof(ctx->shown_title) - 1U] = '\0';
    }
}

static bool pump_mode_page_is_available(const pump_home_base_ui_info_t *info, uint8_t page_index)
{
    if (page_index >= PUMP_HOME_PAGE_MAX) {
        return false;
    }
    if (page_index == PUMP_HOME_PAGE_RHYTHM_2) {
        return info != NULL && info->mode.custom_mode_active != 0;
    }
    return true;
}

static uint8_t pump_mode_page_dot_count_get(const pump_home_base_ui_info_t *info)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < PUMP_HOME_PAGE_MAX; i++) {
        if (pump_mode_page_is_available(info, i)) {
            count++;
        }
    }
    return count;
}

static uint8_t pump_mode_page_dot_index_get(const pump_home_base_ui_info_t *info, uint8_t page_index)
{
    uint8_t index = 0;

    for (uint8_t i = 0; i < PUMP_HOME_PAGE_MAX; i++) {
        if (!pump_mode_page_is_available(info, i)) {
            continue;
        }
        if (i == page_index) {
            return index;
        }
        index++;
    }
    return 0;
}

/** custom 未激活：5 点绝对坐标（左上角） */
static const lv_point_t pump_mode_dot_pos_5[5] = {
    {130, 11},
    {152, 7},
    {PUMP_MODE_PAGE_DOT_X_CENTER, 5},
    {198, 7},
    {220, 11},
};

/** custom 激活：6 点绝对坐标（左上角） */
static const lv_point_t pump_mode_dot_pos_6[6] = {
    {119, 14},
    {141, 8},
    {163, 5},
    {186, 5},
    {208, 8},
    {230, 14},
};

static void pump_mode_page_dot_place(lv_obj_t *dot, uint8_t dot_index, uint8_t dot_count)
{
    const lv_point_t *table;
    uint8_t table_n;

    EXCEPTION_CHECK_VOID(dot);
    if (dot_count >= 6) {
        table = pump_mode_dot_pos_6;
        table_n = 6;
    } else {
        table = pump_mode_dot_pos_5;
        table_n = 5;
    }
    if (dot_index >= table_n) {
        return;
    }
    lv_obj_set_pos(dot, table[dot_index].x, table[dot_index].y);
}

static pump_home_base_mode_page_ctx_t *pump_mode_page_ctx_get(pump_home_base_ui_info_t *info)
{
    lv_obj_t *page = pump_home_page_parent_get(info);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return NULL;
    }
    return (pump_home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
}

static void pump_mode_chrome_dots_rebuild(pump_home_base_ui_info_t *info, uint8_t selected_page)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;
    const uint8_t dot_count = pump_mode_page_dot_count_get(info);
    const uint8_t selected = pump_mode_page_dot_index_get(info, selected_page);
    uint8_t i;

    EXCEPTION_CHECK_VOID(root);
    chrome->dot_count = dot_count;
    chrome->selected_dot = selected;

    for (i = 0; i < PUMP_HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        if (i >= dot_count) {
            if (chrome->dots[i] != NULL && lv_obj_is_valid(chrome->dots[i])) {
                lv_obj_add_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }

        if (chrome->dots[i] == NULL || !lv_obj_is_valid(chrome->dots[i])) {
            chrome->dots[i] = lv_obj_create(root);
            EXCEPTION_CHECK_VOID(chrome->dots[i]);
            lv_obj_remove_style_all(chrome->dots[i]);
            lv_obj_set_size(chrome->dots[i], PUMP_MODE_PAGE_DOT_DIAMETER, PUMP_MODE_PAGE_DOT_DIAMETER);
            lv_obj_set_style_radius(chrome->dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(chrome->dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(chrome->dots[i], 0, 0);
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_CLICKABLE);
        }
        lv_obj_set_style_bg_color(chrome->dots[i],
            lv_color_hex(i == selected ? PUMP_MODE_PAGE_DOT_COLOR_ACTIVE :
                PUMP_MODE_PAGE_DOT_COLOR_INACTIVE), 0);
        pump_mode_page_dot_place(chrome->dots[i], i, dot_count);
        lv_obj_move_foreground(chrome->dots[i]);
    }
}

static void pump_mode_chrome_time_pair_update(lv_obj_t *labels[PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x, lv_coord_t seg_y, bool hidden)
{
    const lv_coord_t cell_w = PUMP_MODE_TIME_SEG_WIDTH / PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(PUMP_MODE_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], UI_FONT_TITLE, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
        pump_mode_page_obj_flag_set(labels[i], !hidden);
    }
}

static lv_coord_t pump_mode_chrome_time_colon_w_get(pump_home_base_mode_chrome_info_t *chrome)
{
    lv_coord_t colon_w = PUMP_MODE_TIME_COLON_W_FALLBACK;
    lv_obj_t *colon = chrome->time_colon[1];

    if (colon == NULL || !lv_obj_is_valid(colon)) {
        colon = chrome->time_colon[0];
    }
    if (colon != NULL && lv_obj_is_valid(colon)) {
        lv_obj_update_layout(colon);
        colon_w = lv_obj_get_width(colon);
        if (colon_w <= 0) {
            colon_w = PUMP_MODE_TIME_COLON_W_FALLBACK;
        }
    }
    return colon_w;
}

static lv_coord_t pump_mode_chrome_time_unit_span_get(pump_home_base_mode_chrome_info_t *chrome)
{
    return PUMP_MODE_TIME_SEG_WIDTH + PUMP_MODE_TIME_COLON_GAP +
        pump_mode_chrome_time_colon_w_get(chrome) + PUMP_MODE_TIME_COLON_GAP;
}

static void pump_mode_chrome_time_colon_ensure(pump_home_base_mode_chrome_info_t *chrome,
    lv_obj_t *root)
{
    int i;

    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_COLON_COUNT; i++) {
        if (chrome->time_colon[i] == NULL || !lv_obj_is_valid(chrome->time_colon[i])) {
            chrome->time_colon[i] = lv_label_create(root);
            EXCEPTION_CHECK_VOID(chrome->time_colon[i]);
            lv_obj_set_style_text_color(chrome->time_colon[i], lv_color_hex(PUMP_MODE_TIME_COLOR), 0);
            lv_obj_set_style_text_font(chrome->time_colon[i], UI_FONT_TITLE, 0);
            lv_obj_set_style_text_align(chrome->time_colon[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_text(chrome->time_colon[i], ":");
        }
    }
}

static void pump_mode_chrome_time_layout_apply(pump_home_base_ui_info_t *info, uint8_t hours,
    uint8_t minutes, uint8_t seconds, uint8_t digit_size, bool force)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;
    const lv_coord_t unit_span = pump_mode_chrome_time_unit_span_get(chrome);
    const lv_coord_t m_x = PUMP_MODE_TIME_2SEG_M_X;
    const lv_coord_t colon_ms_x = m_x + PUMP_MODE_TIME_SEG_WIDTH + PUMP_MODE_TIME_COLON_GAP;
    const lv_coord_t s_x = colon_ms_x + pump_mode_chrome_time_colon_w_get(chrome) +
        PUMP_MODE_TIME_COLON_GAP;
    lv_coord_t icon_x = PUMP_MODE_TIME_2SEG_ICON_X;
    lv_coord_t h_x = m_x;
    lv_coord_t colon_hm_x = colon_ms_x;
    const bool is_3seg = (digit_size == PUMP_MODE_TIME_3SEG);

    if (is_3seg) {
        h_x = m_x - unit_span;
        colon_hm_x = h_x + PUMP_MODE_TIME_SEG_WIDTH + PUMP_MODE_TIME_COLON_GAP;
        icon_x = PUMP_MODE_TIME_2SEG_ICON_X - unit_span;
    }

    if (chrome->status_icon != NULL && lv_obj_is_valid(chrome->status_icon)) {
        lv_obj_align(chrome->status_icon, LV_ALIGN_TOP_LEFT, icon_x, PUMP_MODE_TIME_ICON_Y);
    }

    if (force || chrome->shown_hours != hours || chrome->shown_digit_size != digit_size) {
        pump_mode_chrome_time_pair_update(chrome->time_h, root, hours, h_x, PUMP_MODE_TIME_Y,
            !is_3seg);
        chrome->shown_hours = hours;
    }

    if (force || chrome->shown_minutes != minutes || chrome->shown_digit_size != digit_size) {
        pump_mode_chrome_time_pair_update(chrome->time_m, root, minutes, m_x, PUMP_MODE_TIME_Y,
            false);
        chrome->shown_minutes = minutes;
    }
    if (force || chrome->shown_seconds != seconds || chrome->shown_digit_size != digit_size) {
        pump_mode_chrome_time_pair_update(chrome->time_s, root, seconds, s_x, PUMP_MODE_TIME_Y,
            false);
        chrome->shown_seconds = seconds;
    }

    if (chrome->time_colon[0] != NULL && lv_obj_is_valid(chrome->time_colon[0])) {
        lv_obj_align(chrome->time_colon[0], LV_ALIGN_TOP_LEFT, colon_hm_x, PUMP_MODE_TIME_Y);
        pump_mode_page_obj_flag_set(chrome->time_colon[0], is_3seg);
    }
    if (chrome->time_colon[1] != NULL && lv_obj_is_valid(chrome->time_colon[1])) {
        lv_obj_align(chrome->time_colon[1], LV_ALIGN_TOP_LEFT, colon_ms_x, PUMP_MODE_TIME_Y);
        pump_mode_page_obj_flag_set(chrome->time_colon[1], true);
    }
    chrome->shown_digit_size = digit_size;
}

static void pump_mode_chrome_time_rebuild(pump_home_base_ui_info_t *info)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;

    EXCEPTION_CHECK_VOID(root);

    if (chrome->time_label == NULL || !lv_obj_is_valid(chrome->time_label)) {
        chrome->time_label = lv_image_create(root);
        EXCEPTION_CHECK_VOID(chrome->time_label);
        lv_obj_clear_flag(chrome->time_label, LV_OBJ_FLAG_CLICKABLE);
        ui_img_baj_theme_set_src(info->ui, chrome->time_label, BASE_RES_ID_PUMP_TIME_LABEL,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(chrome->time_label, LV_ALIGN_TOP_MID, 0, PUMP_MODE_TIME_LABEL_Y);
    }

    if (chrome->status_icon == NULL || !lv_obj_is_valid(chrome->status_icon)) {
        chrome->status_icon = lv_image_create(root);
        EXCEPTION_CHECK_VOID(chrome->status_icon);
        lv_obj_clear_flag(chrome->status_icon, LV_OBJ_FLAG_CLICKABLE);
        chrome->shown_status_icon_id = 0;
        chrome->shown_paused = 0xFF;
    }

    pump_mode_chrome_time_colon_ensure(chrome, root);
    chrome->shown_hours = 0xFF;
    chrome->shown_minutes = 0xFF;
    chrome->shown_seconds = 0xFF;
    chrome->shown_digit_size = 0;
    pump_mode_chrome_time_layout_apply(info, 0, 0, 0, PUMP_MODE_TIME_2SEG, true);
}

static void pump_mode_chrome_time_content_update(pump_home_base_ui_info_t *info,
    ui_proc_pump_t *proc)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
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
    hours = hold_time / 3600;
#if IS_SUPPORT_BASE_TIME_3SEG_TEST
    hours = info->test_hours;
    minutes = info->test_hours;
    seconds = info->test_hours;
#else
    minutes = (hold_time / 60) % 60;
    seconds = hold_time % 60;
#endif
    digit_size = (hours == 0) ? PUMP_MODE_TIME_2SEG : PUMP_MODE_TIME_3SEG;
    paused = (proc->pump_status == BREAST_PUMP_PAUSE);
    icon_id = paused ? BASE_RES_ID_PLAY_MIN_ICON : BASE_RES_ID_PAUSE_MIN_ICON;

    if (chrome->status_icon != NULL && lv_obj_is_valid(chrome->status_icon) &&
        (chrome->shown_status_icon_id != icon_id || info->force_update ||
         chrome->shown_paused != (paused ? 1 : 0))) {
        ui_img_baj_theme_set_src(info->ui, chrome->status_icon, icon_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        chrome->shown_status_icon_id = icon_id;
        chrome->shown_paused = paused ? 1 : 0;
    }

    pump_mode_chrome_time_layout_apply(info, hours, minutes, seconds, digit_size,
        info->force_update != 0);
}

#if IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
static void pump_mode_chrome_time_obj_style_apply(lv_obj_t *obj, lv_opa_t opa, lv_coord_t y_off)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_set_style_opa(obj, opa, 0);
    lv_obj_set_style_translate_y(obj, y_off, 0);
    lv_obj_set_style_transform_scale(obj, PUMP_MODE_PAGE_SCALE_FULL, 0);
}

static void pump_mode_chrome_time_visual_apply(pump_home_base_ui_info_t *info, lv_opa_t opa,
    lv_coord_t y_off)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    uint8_t i;

    pump_mode_chrome_time_obj_style_apply(chrome->time_label, opa, y_off);
    pump_mode_chrome_time_obj_style_apply(chrome->status_icon, opa, y_off);
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_COLON_COUNT; i++) {
        pump_mode_chrome_time_obj_style_apply(chrome->time_colon[i], opa, y_off);
    }
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
        pump_mode_chrome_time_obj_style_apply(chrome->time_h[i], opa, y_off);
        pump_mode_chrome_time_obj_style_apply(chrome->time_m[i], opa, y_off);
        pump_mode_chrome_time_obj_style_apply(chrome->time_s[i], opa, y_off);
    }
}

static void pump_mode_chrome_time_enter_anim_exec(void *var, int32_t progress)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)var;
    lv_opa_t opa;
    lv_coord_t y_off;

    if (info == NULL) {
        return;
    }
    if (progress < 0) {
        progress = 0;
    } else if (progress > PUMP_MODE_TIME_ENTER_PROGRESS_FULL) {
        progress = PUMP_MODE_TIME_ENTER_PROGRESS_FULL;
    }

    opa = (lv_opa_t)((progress * LV_OPA_COVER) / PUMP_MODE_TIME_ENTER_PROGRESS_FULL);
    y_off = (lv_coord_t)((PUMP_MODE_TIME_ENTER_Y_OFFSET *
        (PUMP_MODE_TIME_ENTER_PROGRESS_FULL - progress)) / PUMP_MODE_TIME_ENTER_PROGRESS_FULL);
    pump_mode_chrome_time_visual_apply(info, opa, y_off);
}

static void pump_mode_chrome_time_enter_anim_ready(lv_anim_t *anim)
{
    pump_home_base_ui_info_t *info;

    if (anim == NULL) {
        return;
    }
    info = (pump_home_base_ui_info_t *)anim->var;
    if (info == NULL) {
        return;
    }
    info->mode_chrome.time_enter_animating = 0;
    pump_mode_chrome_time_visual_apply(info, LV_OPA_COVER, 0);
}

static void pump_mode_chrome_time_enter_anim_stop(pump_home_base_ui_info_t *info)
{
    if (info == NULL) {
        return;
    }
    lv_anim_delete(info, pump_mode_chrome_time_enter_anim_exec);
    info->mode_chrome.time_enter_animating = 0;
    pump_mode_chrome_time_visual_apply(info, LV_OPA_COVER, 0);
}

static void pump_mode_chrome_time_enter_anim_start(pump_home_base_ui_info_t *info)
{
    lv_anim_t anim;

    EXCEPTION_CHECK_VOID(info);
    lv_anim_delete(info, pump_mode_chrome_time_enter_anim_exec);
    info->mode_chrome.time_enter_animating = 1;
    pump_mode_chrome_time_visual_apply(info, LV_OPA_TRANSP, PUMP_MODE_TIME_ENTER_Y_OFFSET);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, info);
    lv_anim_set_time(&anim, PUMP_MODE_TIME_ENTER_ANIM_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&anim, pump_mode_chrome_time_enter_anim_exec);
    lv_anim_set_completed_cb(&anim, pump_mode_chrome_time_enter_anim_ready);
    lv_anim_set_values(&anim, 0, PUMP_MODE_TIME_ENTER_PROGRESS_FULL);
    lv_anim_start(&anim);
}
#endif /* IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM */

static void pump_mode_chrome_visibility_apply(pump_home_base_ui_info_t *info)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    const uint8_t shown = info->page.shown_page_index;
    const bool transitioning = info->page.transitioning != 0;
    /*
     * 离 page0：目标已非 MAIN 即显示；滑回 page0：目标为 MAIN 即隐藏，
     * 不等整段过渡结束，避免时间胶囊滞留过久。
     */
    const bool chrome_show = transitioning ?
        (info->page.page_index != PUMP_HOME_PAGE_MAIN) :
        (shown != PUMP_HOME_PAGE_MAIN);
#if IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
    const bool time_rising = chrome_show && (chrome->time_shown == 0);
    /* 仅从 page0 滑出时播入场位移；直达/模式页间切换保持稳态 */
    const bool time_enter_anim = time_rising && transitioning &&
        (shown == PUMP_HOME_PAGE_MAIN);
#endif
    uint8_t i;

    for (i = 0; i < PUMP_HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        if (chrome->dots[i] == NULL || !lv_obj_is_valid(chrome->dots[i])) {
            continue;
        }
        if (chrome_show && i < chrome->dot_count) {
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(chrome->dots[i]);
        } else {
            lv_obj_add_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    pump_mode_page_obj_flag_set(chrome->time_label, chrome_show);
    pump_mode_page_obj_flag_set(chrome->status_icon, chrome_show);
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_COLON_COUNT; i++) {
        const bool colon_show = chrome_show &&
            (i == 1 || chrome->shown_digit_size == PUMP_MODE_TIME_3SEG);
        pump_mode_page_obj_flag_set(chrome->time_colon[i], colon_show);
    }
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
        pump_mode_page_obj_flag_set(chrome->time_h[i],
            chrome_show && chrome->shown_digit_size == PUMP_MODE_TIME_3SEG);
        pump_mode_page_obj_flag_set(chrome->time_m[i], chrome_show);
        pump_mode_page_obj_flag_set(chrome->time_s[i], chrome_show);
    }
    if (chrome_show) {
        if (chrome->time_label != NULL && lv_obj_is_valid(chrome->time_label)) {
            lv_obj_move_foreground(chrome->time_label);
        }
        if (chrome->status_icon != NULL && lv_obj_is_valid(chrome->status_icon)) {
            lv_obj_move_foreground(chrome->status_icon);
        }
        for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_COLON_COUNT; i++) {
            if (chrome->time_colon[i] != NULL && lv_obj_is_valid(chrome->time_colon[i])) {
                lv_obj_move_foreground(chrome->time_colon[i]);
            }
        }
        for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
            if (chrome->time_h[i] != NULL && lv_obj_is_valid(chrome->time_h[i])) {
                lv_obj_move_foreground(chrome->time_h[i]);
            }
            if (chrome->time_m[i] != NULL && lv_obj_is_valid(chrome->time_m[i])) {
                lv_obj_move_foreground(chrome->time_m[i]);
            }
            if (chrome->time_s[i] != NULL && lv_obj_is_valid(chrome->time_s[i])) {
                lv_obj_move_foreground(chrome->time_s[i]);
            }
        }
#if IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
        if (time_enter_anim) {
            pump_mode_chrome_time_enter_anim_start(info);
        } else if (time_rising) {
            pump_mode_chrome_time_enter_anim_stop(info);
        }
    } else {
        pump_mode_chrome_time_enter_anim_stop(info);
    }
    chrome->time_shown = chrome_show ? 1 : 0;
#else
    }
#endif
}

/** 按模式与是否两行文案取 ICON / 标题 / 时间 Y */
static void pump_mode_page_layout_y_get(uint8_t mode, bool multi_line,
    lv_coord_t *icon_y, lv_coord_t *title_y, lv_coord_t *duration_y)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            *icon_y = PUMP_MODE_PAGE_STIM_ICON_Y;
            *title_y = PUMP_MODE_PAGE_STIM_TITLE_Y;
            *duration_y = PUMP_MODE_PAGE_STIM_DURATION_Y;
            break;
        case BREAST_PUMP_LACTATION_MODE:
            *icon_y = PUMP_MODE_PAGE_EXPR_ICON_Y;
            *title_y = PUMP_MODE_PAGE_EXPR_TITLE_Y;
            *duration_y = PUMP_MODE_PAGE_EXPR_DURATION_Y;
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            if (multi_line) {
                *icon_y = PUMP_MODE_PAGE_RHYTHM_2L_ICON_Y;
                *title_y = PUMP_MODE_PAGE_RHYTHM_2L_TITLE_Y;
                *duration_y = PUMP_MODE_PAGE_RHYTHM_2L_DURATION_Y;
            } else {
                *icon_y = PUMP_MODE_PAGE_RHYTHM_1L_ICON_Y;
                *title_y = PUMP_MODE_PAGE_RHYTHM_1L_TITLE_Y;
                *duration_y = PUMP_MODE_PAGE_RHYTHM_1L_DURATION_Y;
            }
            break;
        default:
            *icon_y = PUMP_MODE_PAGE_RHYTHM_1L_ICON_Y;
            *title_y = PUMP_MODE_PAGE_RHYTHM_1L_TITLE_Y;
            *duration_y = PUMP_MODE_PAGE_RHYTHM_1L_DURATION_Y;
            break;
    }
}

/**
 * 模式标题超宽自动换行；两行仍不够则第二行加 "..."。
 * ICON / 文案 / 时间按模式与单双行使用固定 Y。
 */
static void pump_mode_page_hero_layout(pump_home_base_mode_page_ctx_t *ctx)
{
    const lv_font_t *font;
    int32_t letter_space;
    int32_t line_space;
    lv_point_t size;
    lv_coord_t title_y;
    lv_coord_t icon_y;
    lv_coord_t duration_y;
    lv_coord_t title_h = 0;
    bool has_duration = false;
    bool multi_line = false;
    char display[PUMP_MODE_PAGE_TITLE_DISPLAY_LEN];

    if (ctx == NULL || ctx->title == NULL || !lv_obj_is_valid(ctx->title)) {
        return;
    }

    font = lv_obj_get_style_text_font(ctx->title, 0);
    letter_space = lv_obj_get_style_text_letter_space(ctx->title, 0);
    line_space = lv_obj_get_style_text_line_space(ctx->title, 0);
    if (font == NULL) {
        return;
    }

    pump_mode_page_title_make_display(display, sizeof(display), ctx->shown_title,
        font, letter_space, ctx->title_ellipsis != 0);
    lv_label_set_text(ctx->title, display);

    lv_text_get_size(&size, display, font, letter_space, line_space, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    if (size.x > PUMP_MODE_PAGE_TITLE_MAX_W || strchr(display, '\n') != NULL) {
        lv_label_set_long_mode(ctx->title, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(ctx->title, PUMP_MODE_PAGE_TITLE_MAX_W);
    } else {
        lv_label_set_long_mode(ctx->title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(ctx->title, LV_SIZE_CONTENT);
    }

    lv_obj_update_layout(ctx->title);
    title_h = lv_obj_get_height(ctx->title);
    multi_line = (title_h > font->line_height) || (strchr(display, '\n') != NULL);
    has_duration = (ctx->duration != NULL && lv_obj_is_valid(ctx->duration) &&
        !lv_obj_has_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN));

    pump_mode_page_layout_y_get(ctx->mode, multi_line, &icon_y, &title_y, &duration_y);

    lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, title_y);
    if (ctx->icon != NULL && lv_obj_is_valid(ctx->icon)) {
        lv_obj_align(ctx->icon, LV_ALIGN_TOP_MID, 0, icon_y);
    }
    if (has_duration) {
        lv_obj_align(ctx->duration, LV_ALIGN_TOP_MID, 0, duration_y);
    }
}

static void pump_mode_page_icon_detach(pump_home_base_mode_page_ctx_t *ctx)
{
    if (ctx == NULL || ctx->icon == NULL || !lv_obj_is_valid(ctx->icon)) {
        return;
    }

    const void *src = lv_image_get_src(ctx->icon);
    if (src == NULL) {
        return;
    }

    /* 预解码 ICON 由 snapshot 持有，页面只解绑，不 destroy */
    if (ctx->icon_predecoded || get_predecoded_icon(ctx->icon_id) == src) {
        lv_image_set_src(ctx->icon, NULL);
        ctx->icon_predecoded = 0;
        return;
    }
    ui_img_png_release_decoded_buf(ctx->icon);
}

static void pump_mode_page_icon_build(pump_home_base_ui_info_t *info, pump_home_base_mode_page_ctx_t *ctx)
{
    lv_obj_t *parent = pump_home_page_parent_get(info);
    const uint16_t res_id = pump_mode_page_icon_res_id_get(ctx->mode);
    lv_draw_buf_t *predecoded;

    if (ctx->icon == NULL || !lv_obj_is_valid(ctx->icon)) {
        ctx->icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(ctx->icon);
    }

    pump_mode_page_icon_detach(ctx);
    predecoded = get_predecoded_icon(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(ctx->icon, predecoded);
        ctx->icon_predecoded = 1;
    } else {
        ui_img_png_theme_set_src(info->ui, ctx->icon, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
        ctx->icon_predecoded = 0;
    }
    ctx->icon_id = res_id;
}

static void pump_mode_page_title_build(pump_home_base_ui_info_t *info, pump_home_base_mode_page_ctx_t *ctx)
{
    char title[PUMP_MODE_PAGE_TITLE_RAW_LEN] = {0};
    lv_obj_t *parent = pump_home_page_parent_get(info);

    pump_mode_page_title_get(info, ctx->mode, title, sizeof(title));

    if (ctx->title == NULL || !lv_obj_is_valid(ctx->title)) {
        ctx->title = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(ctx->title);
        lv_obj_set_style_text_color(ctx->title, lv_color_hex(PUMP_MODE_PAGE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(ctx->title, UI_FONT_TITLE_LG, 0);
        lv_obj_set_style_text_align(ctx->title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(ctx->title, 0, 0);
    }
    pump_mode_page_title_store(ctx, title);
}

static bool pump_mode_page_has_duration(uint8_t mode)
{
    if (mode == BREAST_PUMP_MILK_BOOST_MODE ||
        mode == BREAST_PUMP_PP_MILK_TRACK_MODE ||
        mode == BREAST_PUMP_CUSTOM_MODE) {
        return true;
    }
#if IS_SUPPORT_PUMP_MODE_PAGE_COMMON_DURATION
    if (mode == BREAST_PUMP_STIMULATION_MODE ||
        mode == BREAST_PUMP_LACTATION_MODE) {
        return true;
    }
#endif
    return false;
}

static void pump_mode_page_duration_text_get(pump_home_base_ui_info_t *info, uint8_t mode,
    char *buf, size_t buf_len)
{
    int pump_time = 0;

    if (buf == NULL || buf_len == 0) {
        return;
    }
    buf[0] = '\0';
    if (!pump_mode_page_has_duration(mode)) {
        return;
    }
#if IS_SUPPORT_PUMP_MODE_PAGE_COMMON_DURATION
    if (mode == BREAST_PUMP_STIMULATION_MODE || mode == BREAST_PUMP_LACTATION_MODE) {
        snprintf(buf, buf_len, UI_STR_DURATION_APPROX_MIN_FMT, PUMP_MODE_PAGE_COMMON_DURATION_MIN);
        return;
    }
#endif
    if (info != NULL && info->st != NULL) {
        info->st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, mode, 0, &pump_time);
    }
    snprintf(buf, buf_len, UI_STR_DURATION_APPROX_MIN_FMT, pump_time / 60);
}

static void pump_mode_page_duration_build(pump_home_base_ui_info_t *info,
    pump_home_base_mode_page_ctx_t *ctx)
{
    char duration[16] = {0};
    lv_obj_t *parent = pump_home_page_parent_get(info);

    if (!pump_mode_page_has_duration(ctx->mode)) {
        if (ctx->duration != NULL && lv_obj_is_valid(ctx->duration)) {
            lv_obj_add_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        }
        ctx->shown_duration[0] = '\0';
        return;
    }

    pump_mode_page_duration_text_get(info, ctx->mode, duration, sizeof(duration));

    if (ctx->duration == NULL || !lv_obj_is_valid(ctx->duration)) {
        ctx->duration = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(ctx->duration);
        lv_obj_set_style_text_color(ctx->duration, lv_color_hex(PUMP_MODE_PAGE_DURATION_COLOR), 0);
        lv_obj_set_style_text_font(ctx->duration, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_align(ctx->duration, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(ctx->duration, 0, 0);
    }
    lv_label_set_text(ctx->duration, duration);
    lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
    strncpy(ctx->shown_duration, duration, sizeof(ctx->shown_duration) - 1);
    ctx->shown_duration[sizeof(ctx->shown_duration) - 1] = '\0';
}

static void pump_mode_page_duration_content_update(pump_home_base_ui_info_t *info,
    pump_home_base_mode_page_ctx_t *ctx)
{
    char duration[16] = {0};

    if (!pump_mode_page_has_duration(ctx->mode)) {
        return;
    }
    pump_mode_page_duration_text_get(info, ctx->mode, duration, sizeof(duration));
    if (strncmp(ctx->shown_duration, duration, sizeof(ctx->shown_duration)) == 0 &&
        !info->force_update) {
        return;
    }
    if (ctx->duration == NULL || !lv_obj_is_valid(ctx->duration)) {
        pump_mode_page_duration_build(info, ctx);
        pump_mode_page_hero_layout(ctx);
        return;
    }
    lv_label_set_text(ctx->duration, duration);
    lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
    strncpy(ctx->shown_duration, duration, sizeof(ctx->shown_duration) - 1);
    ctx->shown_duration[sizeof(ctx->shown_duration) - 1] = '\0';
    pump_mode_page_hero_layout(ctx);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_mode_chrome_prepare(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->root);
    if (info->mode_chrome.ready) {
        return;
    }
    memset(&info->mode_chrome, 0, sizeof(info->mode_chrome));
    pump_mode_chrome_time_rebuild(info);
    pump_mode_chrome_dots_rebuild(info, PUMP_HOME_PAGE_MAIN);
    info->mode_chrome.ready = 1;
    pump_mode_chrome_visibility_apply(info);
}

void pump_home_mode_chrome_remove(pump_home_base_ui_info_t *info)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(info);
#if IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
    pump_mode_chrome_time_enter_anim_stop(info);
#endif
    for (i = 0; i < PUMP_HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        LVGL_OBJ_DELETE(info->mode_chrome.dots[i]);
        info->mode_chrome.dots[i] = NULL;
    }
    ui_img_baj_release_decoded_buf(info->mode_chrome.time_label);
    LVGL_OBJ_DELETE(info->mode_chrome.time_label);
    info->mode_chrome.time_label = NULL;
    ui_img_baj_release_decoded_buf(info->mode_chrome.status_icon);
    LVGL_OBJ_DELETE(info->mode_chrome.status_icon);
    info->mode_chrome.status_icon = NULL;
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_COLON_COUNT; i++) {
        LVGL_OBJ_DELETE(info->mode_chrome.time_colon[i]);
        info->mode_chrome.time_colon[i] = NULL;
    }
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
        LVGL_OBJ_DELETE(info->mode_chrome.time_h[i]);
        info->mode_chrome.time_h[i] = NULL;
        LVGL_OBJ_DELETE(info->mode_chrome.time_m[i]);
        info->mode_chrome.time_m[i] = NULL;
        LVGL_OBJ_DELETE(info->mode_chrome.time_s[i]);
        info->mode_chrome.time_s[i] = NULL;
    }
    memset(&info->mode_chrome, 0, sizeof(info->mode_chrome));
}

void pump_home_mode_chrome_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc)
{
    uint8_t selected_page;

    EXCEPTION_CHECK_VOID(info);
    if (!info->mode_chrome.ready) {
        pump_home_mode_chrome_prepare(info);
    }

    selected_page = info->page.transitioning ?
        info->page.page_index : info->page.shown_page_index;
    if (selected_page >= PUMP_HOME_PAGE_MAX) {
        selected_page = PUMP_HOME_PAGE_MAIN;
    }

    {
        const uint8_t dot_count = pump_mode_page_dot_count_get(info);
        const uint8_t selected = pump_mode_page_dot_index_get(info, selected_page);
        if (dot_count != info->mode_chrome.dot_count ||
            selected != info->mode_chrome.selected_dot ||
            info->force_update) {
            pump_mode_chrome_dots_rebuild(info, selected_page);
        }
    }
    pump_mode_chrome_time_content_update(info, proc);
    pump_mode_chrome_visibility_apply(info);
}

void pump_home_mode_page_ui_build(pump_home_base_ui_info_t *info, uint8_t page_index)
{
    pump_home_base_mode_page_ctx_t *ctx;
    lv_obj_t *page;

    EXCEPTION_CHECK_VOID(info);
    page = pump_home_page_parent_get(info);
    EXCEPTION_CHECK_VOID(page);

    ctx = (pump_home_base_mode_page_ctx_t *)u_malloz(sizeof(pump_home_base_mode_page_ctx_t));
    EXCEPTION_CHECK_VOID(ctx);
    lv_obj_set_user_data(page, ctx);

    ctx->page_index = page_index;
    ctx->mode = pump_mode_page_mode_from_page(page_index);

    pump_home_mode_chrome_prepare(info);
    ctx->circle_bg = pump_mode_page_circle_bg_create(page);
    pump_mode_page_icon_build(info, ctx);
    pump_mode_page_title_build(info, ctx);
    pump_mode_page_duration_build(info, ctx);
    pump_mode_page_hero_layout(ctx);
    if (ctx->circle_bg != NULL && lv_obj_is_valid(ctx->circle_bg)) {
        lv_obj_move_background(ctx->circle_bg);
    }
    /* 直达显示：稳态全尺寸；过渡入场时随后由 lerp 覆盖为缩小态 */
    pump_home_mode_page_transition_apply(page, PUMP_MODE_PAGE_SCALE_FULL, false);
}

void pump_home_mode_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc)
{
    pump_home_base_mode_page_ctx_t *ctx;
    char title[PUMP_MODE_PAGE_TITLE_RAW_LEN] = {0};
    char capped[PUMP_MODE_PAGE_TITLE_BUF_LEN] = {0};
    uint8_t need_ellipsis;

    EXCEPTION_CHECK_VOID(info && proc);
    ctx = pump_mode_page_ctx_get(info);
    if (ctx == NULL) {
        return;
    }

    /// 个性化模式：刷新 App 下发的名称
    if (ctx->mode == BREAST_PUMP_CUSTOM_MODE || info->force_update) {
        pump_mode_page_title_get(info, ctx->mode, title, sizeof(title));
        need_ellipsis = (strlen(title) > PUMP_MODE_PAGE_TITLE_MAX_CHARS) ? 1U : 0U;
        if (need_ellipsis) {
            memcpy(capped, title, PUMP_MODE_PAGE_TITLE_MAX_CHARS);
            capped[PUMP_MODE_PAGE_TITLE_MAX_CHARS] = '\0';
        } else {
            strncpy(capped, title, sizeof(capped) - 1U);
            capped[sizeof(capped) - 1U] = '\0';
        }
        if (strncmp(ctx->shown_title, capped, sizeof(ctx->shown_title)) != 0 ||
            ctx->title_ellipsis != need_ellipsis ||
            info->force_update) {
            pump_mode_page_title_store(ctx, title);
            pump_mode_page_hero_layout(ctx);
        }
    }
    pump_mode_page_duration_content_update(info, ctx);
    /* 缩放复位只在切页过渡里 apply；稳态每帧设 FULL 会反复 update_layout */
}

void pump_home_mode_page_ui_remove(pump_home_base_ui_info_t *info)
{
    pump_home_base_mode_page_ctx_t *ctx;
    lv_obj_t *page;

    EXCEPTION_CHECK_VOID(info);
    page = pump_home_page_parent_get(info);
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }

    ctx = (pump_home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
    if (ctx == NULL) {
        return;
    }

    pump_mode_page_icon_detach(ctx);
    lv_obj_set_user_data(page, NULL);
    u_free(ctx);
}

void pump_home_mode_page_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning)
{
    pump_home_base_mode_page_ctx_t *ctx;
    /* 放大未完成前保持圆+小字号；放大完成（或静止）再隐藏圆并切 36 */
    const bool small = transitioning && (hero_scale < PUMP_MODE_PAGE_SCALE_FULL);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    ctx = (pump_home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
    if (ctx == NULL) {
        return;
    }

    if (ctx->circle_bg != NULL && lv_obj_is_valid(ctx->circle_bg)) {
        if (small) {
            lv_obj_clear_flag(ctx->circle_bg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_background(ctx->circle_bg);
        } else {
            lv_obj_add_flag(ctx->circle_bg, LV_OBJ_FLAG_HIDDEN);
        }
        /* 圆背景不参与缩放 */
        pump_mode_page_obj_scale_set(ctx->circle_bg, PUMP_MODE_PAGE_SCALE_FULL);
    }

    /* ICON 跟 hero_scale；标题不缩放，放大完成后再 29→36 */
    pump_mode_page_obj_scale_set(ctx->icon, hero_scale);
    if (ctx->title != NULL && lv_obj_is_valid(ctx->title)) {
        const lv_font_t *font = small ?
            UI_FONT_TITLE_MD : UI_FONT_TITLE_LG;

        pump_mode_page_obj_scale_set(ctx->title, PUMP_MODE_PAGE_SCALE_FULL);
        lv_obj_set_style_text_font(ctx->title, font, 0);
    }

    /* 总时间：圆背景消失后展示，圆背景出现时隐藏 */
    if (ctx->duration != NULL && lv_obj_is_valid(ctx->duration)) {
        if (!small && pump_mode_page_has_duration(ctx->mode)) {
            lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        }
    }
    /* 字号/显隐变化后按是否换行重排 ICON / 标题 / 时间 */
    pump_mode_page_hero_layout(ctx);
}
