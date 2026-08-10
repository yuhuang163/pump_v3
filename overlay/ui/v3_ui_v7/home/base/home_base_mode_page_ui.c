/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_MODE_PAGE_DOT_DIAMETER                 (11)
#define HOME_MODE_PAGE_DOT_COLOR_ACTIVE             (0xFFFFFF)
#define HOME_MODE_PAGE_DOT_COLOR_INACTIVE           (0x4D4D4D)

/** 常规模式（刺激/吸乳）是否显示时长；默认关闭，开启后同韵律展示 ≈ Nmin */
#ifndef IS_SUPPORT_HOME_MODE_PAGE_COMMON_DURATION
#define IS_SUPPORT_HOME_MODE_PAGE_COMMON_DURATION   (0)
#endif
/** 刺激/吸乳时长文案固定分钟数 */
#define HOME_MODE_PAGE_COMMON_DURATION_MIN          (30)

/** 刺激：ICON / 文案 / 时间 */
#define HOME_MODE_PAGE_STIM_ICON_Y                  (87)
#define HOME_MODE_PAGE_STIM_TITLE_Y                 (195)
#define HOME_MODE_PAGE_STIM_DURATION_Y              (242)
/** 吸乳：ICON / 文案 / 时间 */
#define HOME_MODE_PAGE_EXPR_ICON_Y                  (89)
#define HOME_MODE_PAGE_EXPR_TITLE_Y                 (195)
#define HOME_MODE_PAGE_EXPR_DURATION_Y              (242)
/** 韵律单行：ICON / 文案 / 时间 */
#define HOME_MODE_PAGE_RHYTHM_1L_ICON_Y             (88)
#define HOME_MODE_PAGE_RHYTHM_1L_TITLE_Y            (195)
#define HOME_MODE_PAGE_RHYTHM_1L_DURATION_Y         (242)
/** 韵律两行：ICON / 文案 / 时间 */
#define HOME_MODE_PAGE_RHYTHM_2L_ICON_Y             (67)
#define HOME_MODE_PAGE_RHYTHM_2L_TITLE_Y            (168)
#define HOME_MODE_PAGE_RHYTHM_2L_DURATION_Y         (262)

#define HOME_MODE_PAGE_TITLE_COLOR                  (0xFFFFFF)
#define HOME_MODE_PAGE_DURATION_COLOR               (0x999999)
/** 模式标题单行超宽则换行；换行后按单/双行固定坐标排 ICON / 时间文案 */
#define HOME_MODE_PAGE_TITLE_MAX_W                  (300)
/** 标题最长 32 字符；超出则裁进 shown 并启用 "..." */
#define HOME_MODE_PAGE_TITLE_MAX_CHARS              (32 - 6)
#define HOME_MODE_PAGE_TITLE_BUF_LEN                (HOME_MODE_PAGE_TITLE_MAX_CHARS + 1)
/** title_get 临时缓冲（可长于 32，再截断入库） */
#define HOME_MODE_PAGE_TITLE_RAW_LEN                (64)
/** 展示缓冲：含换行与 "..." */
#define HOME_MODE_PAGE_TITLE_DISPLAY_LEN            (40)

#define HOME_MODE_PAGE_TIP_H                        (63)
/** 底部提示图 Y：贴底留边 */
#define HOME_MODE_PAGE_TIP_Y                        (PANLE_DISPLAY_H - HOME_MODE_PAGE_TIP_H - 8)
#define HOME_MODE_PAGE_TIP_BASE_OPA                 (LV_OPA_70)

#define HOME_MODE_PAGE_SCALE_FULL                   (256)
/** 5 点布局第 3 点 X 居中：(屏宽 - 直径) / 2 */
#define HOME_MODE_PAGE_DOT_X_CENTER \
    ((PANLE_DISPLAY_W - HOME_MODE_PAGE_DOT_DIAMETER) / 2)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/** 提示文案预解码缓存（本模块持有，页面侧勿 destroy） */
static lv_draw_buf_t *s_mode_select_tip_predecoded = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_mode_page_obj_scale_set(lv_obj_t *obj, int32_t scale)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_update_layout(obj);
    lv_obj_set_style_transform_pivot_x(obj, lv_obj_get_width(obj) / 2, 0);
    lv_obj_set_style_transform_pivot_y(obj, lv_obj_get_height(obj) / 2, 0);
    lv_obj_set_style_transform_scale(obj, scale, 0);
}

static lv_obj_t *home_mode_page_circle_bg_create(lv_obj_t *parent)
{
    lv_obj_t *circle = lv_obj_create(parent);

    EXCEPTION_CHECK_POINT(circle);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE, HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(circle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(circle);
    return circle;
}

static uint8_t home_mode_page_mode_from_page(uint8_t page_index)
{
    switch (page_index) {
        case HOME_MODE_PAGE_INDEX_STIMULATION_SELECT:
            return BREAST_PUMP_STIMULATION_MODE;
        case HOME_MODE_PAGE_INDEX_EXPRESSION_SELECT:
            return BREAST_PUMP_LACTATION_MODE;
        case HOME_MODE_PAGE_INDEX_MILK_BOOST_SELECT:
            return BREAST_PUMP_MILK_BOOST_MODE;
        case HOME_MODE_PAGE_INDEX_POWER_PUMPING_SELECT:
            return BREAST_PUMP_PP_MILK_TRACK_MODE;
        case HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT:
            return BREAST_PUMP_CUSTOM_MODE;
        default:
            return BREAST_PUMP_STIMULATION_MODE;
    }
}

static uint16_t home_mode_page_icon_res_id_get(uint8_t mode)
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

static void home_mode_page_title_get(home_base_ui_info_t *info, uint8_t mode, char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return;
    }

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
            // snprintf(buf, buf_len, "Power Pumping Power Pumping Expression Milk Boost");
            snprintf(buf, buf_len, "Power Pumping");
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
static uint32_t home_mode_page_title_fit_len(const char *text, uint32_t text_len,
    const lv_font_t *font, int32_t letter_space, int32_t max_w)
{
    char tmp[HOME_MODE_PAGE_TITLE_BUF_LEN];
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

/** 入库：超过 32 字符截断并标记需 "..." */
static void home_mode_page_title_store(home_base_mode_page_ctx_t *ctx, const char *title)
{
    size_t len;

    EXCEPTION_CHECK_VOID(ctx && title);
    len = strlen(title);
    ctx->title_ellipsis = (len > HOME_MODE_PAGE_TITLE_MAX_CHARS) ? 1U : 0U;
    if (ctx->title_ellipsis) {
        memcpy(ctx->shown_title, title, HOME_MODE_PAGE_TITLE_MAX_CHARS);
        ctx->shown_title[HOME_MODE_PAGE_TITLE_MAX_CHARS] = '\0';
    } else {
        strncpy(ctx->shown_title, title, sizeof(ctx->shown_title) - 1U);
        ctx->shown_title[sizeof(ctx->shown_title) - 1U] = '\0';
    }
}

/**
 * 生成展示文案：
 * - 未超 32 且单行 ≤290 / 两行内可放下：原文 + WRAP
 * - 超 32 或两行仍不够：裁剪并加 "..."
 */
static void home_mode_page_title_make_display(char *out, size_t out_len, const char *src,
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
        if (size.x <= HOME_MODE_PAGE_TITLE_MAX_W) {
            strncpy(out, src, out_len - 1U);
            out[out_len - 1U] = '\0';
            return;
        }
        lv_text_get_size(&size, src, font, letter_space, 0, HOME_MODE_PAGE_TITLE_MAX_W, LV_TEXT_FLAG_NONE);
        if (size.y <= (int32_t)font->line_height * 2) {
            strncpy(out, src, out_len - 1U);
            out[out_len - 1U] = '\0';
            return;
        }
    } else {
        /* 超 32：优先单行 "原文..." */
        char trial[HOME_MODE_PAGE_TITLE_DISPLAY_LEN];

        snprintf(trial, sizeof(trial), "%s...", src);
        lv_text_get_size(&size, trial, font, letter_space, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (size.x <= HOME_MODE_PAGE_TITLE_MAX_W) {
            strncpy(out, trial, out_len - 1U);
            out[out_len - 1U] = '\0';
            return;
        }
    }

    line1_len = home_mode_page_title_fit_len(src, src_len, font, letter_space,
        HOME_MODE_PAGE_TITLE_MAX_W);
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
        if (size.x <= HOME_MODE_PAGE_TITLE_MAX_W) {
            snprintf(out, out_len, "%.*s\n%s", (int)line1_len, src, rest);
            return;
        }
    }

    if (ellipsis_w >= HOME_MODE_PAGE_TITLE_MAX_W) {
        snprintf(out, out_len, "%.*s\n...", (int)line1_len, src);
        return;
    }
    line2_fit = home_mode_page_title_fit_len(rest, (uint32_t)strlen(rest), font, letter_space,
        HOME_MODE_PAGE_TITLE_MAX_W - ellipsis_w);
    snprintf(out, out_len, "%.*s\n%.*s...", (int)line1_len, src, (int)line2_fit, rest);
}

static bool home_mode_page_is_available(const home_base_ui_info_t *info, uint8_t page_index)
{
    if (page_index >= HOME_PAGE_MAX) {
        return false;
    }
    if (page_index == HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT) {
        return info != NULL && info->mode.custom_mode_active != 0;
    }
    return true;
}

static uint8_t home_mode_page_dot_count_get(const home_base_ui_info_t *info)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < HOME_PAGE_MAX; i++) {
        if (home_mode_page_is_available(info, i)) {
            count++;
        }
    }
    return count;
}

static uint8_t home_mode_page_dot_index_get(const home_base_ui_info_t *info, uint8_t page_index)
{
    uint8_t index = 0;

    for (uint8_t i = 0; i < HOME_PAGE_MAX; i++) {
        if (!home_mode_page_is_available(info, i)) {
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
static const lv_point_t home_mode_dot_pos_5[5] = {
    {130, 11},
    {152, 7},
    {HOME_MODE_PAGE_DOT_X_CENTER, 5},
    {198, 7},
    {220, 11},
};

/** custom 激活：6 点绝对坐标（左上角） */
static const lv_point_t home_mode_dot_pos_6[6] = {
    {119, 14},
    {141, 8},
    {163, 5},
    {186, 5},
    {208, 8},
    {230, 14},
};

static void home_mode_page_dot_place(lv_obj_t *dot, uint8_t dot_index, uint8_t dot_count)
{
    const lv_point_t *table;
    uint8_t table_n;

    EXCEPTION_CHECK_VOID(dot);
    if (dot_count >= 6) {
        table = home_mode_dot_pos_6;
        table_n = 6;
    } else {
        table = home_mode_dot_pos_5;
        table_n = 5;
    }
    if (dot_index >= table_n) {
        return;
    }
    lv_obj_set_pos(dot, table[dot_index].x, table[dot_index].y);
}

static home_base_mode_page_ctx_t *home_mode_page_ctx_get(home_base_ui_info_t *info)
{
    lv_obj_t *page = home_page_parent_get(info);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return NULL;
    }
    return (home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
}

static bool home_mode_page_is_mode_like(uint8_t page_index)
{
    return page_index == HOME_PAGE_MODE_0 ||
           page_index == HOME_PAGE_MODE_1 ||
           page_index == HOME_PAGE_RHYTHM_0 ||
           page_index == HOME_PAGE_RHYTHM_1 ||
           page_index == HOME_PAGE_RHYTHM_2;
}

static void home_mode_chrome_dots_rebuild(home_base_ui_info_t *info, uint8_t selected_page)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;
    const uint8_t dot_count = home_mode_page_dot_count_get(info);
    const uint8_t selected = home_mode_page_dot_index_get(info, selected_page);
    uint8_t i;

    EXCEPTION_CHECK_VOID(root);
    chrome->dot_count = dot_count;
    chrome->selected_dot = selected;

    for (i = 0; i < HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
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
            lv_obj_set_size(chrome->dots[i], HOME_MODE_PAGE_DOT_DIAMETER, HOME_MODE_PAGE_DOT_DIAMETER);
            lv_obj_set_style_radius(chrome->dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(chrome->dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(chrome->dots[i], 0, 0);
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_CLICKABLE);
        }
        lv_obj_set_style_bg_color(chrome->dots[i],
            lv_color_hex(i == selected ? HOME_MODE_PAGE_DOT_COLOR_ACTIVE :
                HOME_MODE_PAGE_DOT_COLOR_INACTIVE), 0);
        home_mode_page_dot_place(chrome->dots[i], i, dot_count);
        lv_obj_move_foreground(chrome->dots[i]);
    }
}

static lv_draw_buf_t *home_mode_select_tip_predecode(ui_manager_context_t *ui)
{
    if (s_mode_select_tip_predecoded != NULL) {
        return s_mode_select_tip_predecoded;
    }
#if HOME_BASE_MODE_SELECT_TIP_FROM_CODE
    (void)ui;
    {
        const ui_img_data_t *tip = ui_font_img_mode_select_tip_get();
        if (tip == NULL || tip->data == NULL || tip->size == 0U) {
            return NULL;
        }
        s_mode_select_tip_predecoded = ui_img_png_decode_draw_buf(tip->data, tip->size,
            UI_IMG_PNG_DECODE_RGBA8888);
    }
#else
    if (ui == NULL) {
        return NULL;
    }
    s_mode_select_tip_predecoded = ui_img_baj_theme_decode_draw_buf(ui,
        BASE_RES_ID_MODE_SELECT_TIP, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
#endif
    return s_mode_select_tip_predecoded;
}

static void home_mode_chrome_tip_rebuild(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(root);
    if (chrome->tip_img != NULL && lv_obj_is_valid(chrome->tip_img) &&
        chrome->tip_shine.mask != NULL && lv_obj_is_valid(chrome->tip_shine.mask) &&
        chrome->tip_shine.shine != NULL && lv_obj_is_valid(chrome->tip_shine.shine)) {
        return; /* anim handles sync */
    }

    predecoded = home_mode_select_tip_predecode(info->ui);

    if (chrome->tip_img == NULL || !lv_obj_is_valid(chrome->tip_img)) {
        chrome->tip_img = lv_image_create(root);
        EXCEPTION_CHECK_VOID(chrome->tip_img);
        lv_obj_clear_flag(chrome->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(chrome->tip_img, predecoded);
            chrome->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(chrome->tip_img, HOME_MODE_PAGE_TIP_BASE_OPA, 0);
        lv_obj_align(chrome->tip_img, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = HOME_MODE_PAGE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;

    ui_shine_prepare_from_tip(&chrome->tip_shine, &style, predecoded);
    ui_shine_update(&chrome->tip_shine, root, &style, chrome->tip_img, 1);
    lv_obj_move_foreground(chrome->tip_img);
    ui_shine_show(&chrome->tip_shine);
}

static void home_mode_chrome_tip_update(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    ui_shine_style_t style;
    uint8_t paused = 0;

    if (info->force_update) {
        home_mode_chrome_tip_rebuild(info);
    }

    if (chrome->tip_shine.mask != NULL && lv_obj_is_valid(chrome->tip_shine.mask) &&
        lv_obj_has_flag(chrome->tip_shine.mask, LV_OBJ_FLAG_HIDDEN)) {
        paused = 1;
    }

    ui_shine_style_init(&style);
    style.mask_y = HOME_MODE_PAGE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&chrome->tip_shine, &style, chrome->tip_img, paused);
}

static void home_mode_chrome_visibility_apply(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    const uint8_t shown = info->page.shown_page_index;
    const uint8_t target = info->page.page_index;
    const bool transitioning = info->page.transitioning != 0;
    /* page0 静止：隐藏；离 page0 动画一开始即显示定位点 */
    const bool dots_show = !(shown == HOME_PAGE_MAIN && !transitioning);
    /* page0 相关（在主页或进出主页过渡）：隐藏提示文案 */
    const bool page0_related =
        (shown == HOME_PAGE_MAIN) ||
        (transitioning && target == HOME_PAGE_MAIN);
    const bool tip_show = !page0_related && home_mode_page_is_mode_like(shown);
    uint8_t i;

    for (i = 0; i < HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        if (chrome->dots[i] == NULL || !lv_obj_is_valid(chrome->dots[i])) {
            continue;
        }
        if (dots_show && i < chrome->dot_count) {
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(chrome->dots[i]);
        } else {
            lv_obj_add_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (chrome->tip_img != NULL && lv_obj_is_valid(chrome->tip_img)) {
        if (tip_show) {
            lv_obj_clear_flag(chrome->tip_img, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(chrome->tip_img);
        } else {
            lv_obj_add_flag(chrome->tip_img, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (chrome->tip_shine.mask != NULL && lv_obj_is_valid(chrome->tip_shine.mask)) {
        if (tip_show) {
            ui_shine_show(&chrome->tip_shine);
        } else {
            ui_shine_hide(&chrome->tip_shine);
        }
    }
}

/** 按模式与是否两行文案取 ICON / 标题 / 时间 Y */
static void home_mode_page_layout_y_get(uint8_t mode, bool multi_line,
    lv_coord_t *icon_y, lv_coord_t *title_y, lv_coord_t *duration_y)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            *icon_y = HOME_MODE_PAGE_STIM_ICON_Y;
            *title_y = HOME_MODE_PAGE_STIM_TITLE_Y;
            *duration_y = HOME_MODE_PAGE_STIM_DURATION_Y;
            break;
        case BREAST_PUMP_LACTATION_MODE:
            *icon_y = HOME_MODE_PAGE_EXPR_ICON_Y;
            *title_y = HOME_MODE_PAGE_EXPR_TITLE_Y;
            *duration_y = HOME_MODE_PAGE_EXPR_DURATION_Y;
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            if (multi_line) {
                *icon_y = HOME_MODE_PAGE_RHYTHM_2L_ICON_Y;
                *title_y = HOME_MODE_PAGE_RHYTHM_2L_TITLE_Y;
                *duration_y = HOME_MODE_PAGE_RHYTHM_2L_DURATION_Y;
            } else {
                *icon_y = HOME_MODE_PAGE_RHYTHM_1L_ICON_Y;
                *title_y = HOME_MODE_PAGE_RHYTHM_1L_TITLE_Y;
                *duration_y = HOME_MODE_PAGE_RHYTHM_1L_DURATION_Y;
            }
            break;
        default:
            *icon_y = HOME_MODE_PAGE_RHYTHM_1L_ICON_Y;
            *title_y = HOME_MODE_PAGE_RHYTHM_1L_TITLE_Y;
            *duration_y = HOME_MODE_PAGE_RHYTHM_1L_DURATION_Y;
            break;
    }
}

/**
 * 模式标题超宽自动换行；两行仍不够则第二行加 "..."。
 * ICON / 文案 / 时间按模式与单双行使用固定 Y。
 */
static void home_mode_page_hero_layout(home_base_mode_page_ctx_t *ctx)
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
    char display[HOME_MODE_PAGE_TITLE_DISPLAY_LEN];

    if (ctx == NULL || ctx->title == NULL || !lv_obj_is_valid(ctx->title)) {
        return;
    }

    font = lv_obj_get_style_text_font(ctx->title, 0);
    letter_space = lv_obj_get_style_text_letter_space(ctx->title, 0);
    line_space = lv_obj_get_style_text_line_space(ctx->title, 0);
    if (font == NULL) {
        return;
    }

    /* 始终基于原文重算展示态（字号切换时 "..." 裁剪点会变） */
    home_mode_page_title_make_display(display, sizeof(display), ctx->shown_title,
        font, letter_space, ctx->title_ellipsis != 0);
    lv_label_set_text(ctx->title, display);

    lv_text_get_size(&size, display, font, letter_space, line_space, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    if (size.x > HOME_MODE_PAGE_TITLE_MAX_W || strchr(display, '\n') != NULL) {
        lv_label_set_long_mode(ctx->title, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(ctx->title, HOME_MODE_PAGE_TITLE_MAX_W);
    } else {
        lv_label_set_long_mode(ctx->title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(ctx->title, LV_SIZE_CONTENT);
    }

    lv_obj_update_layout(ctx->title);
    title_h = lv_obj_get_height(ctx->title);
    multi_line = (title_h > font->line_height) || (strchr(display, '\n') != NULL);
    has_duration = (ctx->duration != NULL && lv_obj_is_valid(ctx->duration) &&
        !lv_obj_has_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN));

    home_mode_page_layout_y_get(ctx->mode, multi_line, &icon_y, &title_y, &duration_y);

    lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, title_y);
    if (ctx->icon != NULL && lv_obj_is_valid(ctx->icon)) {
        lv_obj_align(ctx->icon, LV_ALIGN_TOP_MID, 0, icon_y);
    }
    if (has_duration) {
        lv_obj_align(ctx->duration, LV_ALIGN_TOP_MID, 0, duration_y);
    }
}

static void home_mode_page_icon_detach(home_base_mode_page_ctx_t *ctx)
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

static void home_mode_page_icon_build(home_base_ui_info_t *info, home_base_mode_page_ctx_t *ctx)
{
    lv_obj_t *parent = home_page_parent_get(info);
    const uint16_t res_id = home_mode_page_icon_res_id_get(ctx->mode);
    lv_draw_buf_t *predecoded;

    if (ctx->icon == NULL || !lv_obj_is_valid(ctx->icon)) {
        ctx->icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(ctx->icon);
    }

    home_mode_page_icon_detach(ctx);
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

static void home_mode_page_title_build(home_base_ui_info_t *info, home_base_mode_page_ctx_t *ctx)
{
    char title[HOME_MODE_PAGE_TITLE_RAW_LEN] = {0};
    lv_obj_t *parent = home_page_parent_get(info);

    home_mode_page_title_get(info, ctx->mode, title, sizeof(title));

    if (ctx->title == NULL || !lv_obj_is_valid(ctx->title)) {
        ctx->title = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(ctx->title);
        lv_obj_set_style_text_color(ctx->title, lv_color_hex(HOME_MODE_PAGE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(ctx->title, &lv_font_aeonik_soft_33_medium, 0);
        lv_obj_set_style_text_align(ctx->title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(ctx->title, 0, 0);
    }
    home_mode_page_title_store(ctx, title);
}

static bool home_mode_page_has_duration(uint8_t mode)
{
    if (mode == BREAST_PUMP_MILK_BOOST_MODE ||
        mode == BREAST_PUMP_PP_MILK_TRACK_MODE ||
        mode == BREAST_PUMP_CUSTOM_MODE) {
        return true;
    }
#if IS_SUPPORT_HOME_MODE_PAGE_COMMON_DURATION
    if (mode == BREAST_PUMP_STIMULATION_MODE ||
        mode == BREAST_PUMP_LACTATION_MODE) {
        return true;
    }
#endif
    return false;
}

static void home_mode_page_duration_text_get(home_base_ui_info_t *info, uint8_t mode,
    char *buf, size_t buf_len)
{
    int pump_time = 0;

    if (buf == NULL || buf_len == 0) {
        return;
    }
    buf[0] = '\0';
    if (!home_mode_page_has_duration(mode)) {
        return;
    }
#if IS_SUPPORT_HOME_MODE_PAGE_COMMON_DURATION
    if (mode == BREAST_PUMP_STIMULATION_MODE || mode == BREAST_PUMP_LACTATION_MODE) {
        snprintf(buf, buf_len, "≈ %dmin", HOME_MODE_PAGE_COMMON_DURATION_MIN);
        return;
    }
#endif
    if (info != NULL && info->st != NULL) {
        info->st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, mode, 0, &pump_time);
    }
    snprintf(buf, buf_len, "≈ %dmin", pump_time / 60);
}

static void home_mode_page_duration_build(home_base_ui_info_t *info, home_base_mode_page_ctx_t *ctx)
{
    char duration[16] = {0};
    lv_obj_t *parent = home_page_parent_get(info);

    if (!home_mode_page_has_duration(ctx->mode)) {
        if (ctx->duration != NULL && lv_obj_is_valid(ctx->duration)) {
            lv_obj_add_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        }
        ctx->shown_duration[0] = '\0';
        return;
    }

    home_mode_page_duration_text_get(info, ctx->mode, duration, sizeof(duration));

    if (ctx->duration == NULL || !lv_obj_is_valid(ctx->duration)) {
        ctx->duration = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(ctx->duration);
        lv_obj_set_style_text_color(ctx->duration, lv_color_hex(HOME_MODE_PAGE_DURATION_COLOR), 0);
        lv_obj_set_style_text_font(ctx->duration, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(ctx->duration, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(ctx->duration, 0, 0);
    }
    lv_label_set_text(ctx->duration, duration);
    lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
    strncpy(ctx->shown_duration, duration, sizeof(ctx->shown_duration) - 1);
    ctx->shown_duration[sizeof(ctx->shown_duration) - 1] = '\0';
}

static void home_mode_page_duration_content_update(home_base_ui_info_t *info,
    home_base_mode_page_ctx_t *ctx)
{
    char duration[16] = {0};

    if (!home_mode_page_has_duration(ctx->mode)) {
        return;
    }
    home_mode_page_duration_text_get(info, ctx->mode, duration, sizeof(duration));
    if (strncmp(ctx->shown_duration, duration, sizeof(ctx->shown_duration)) == 0 &&
        !info->force_update) {
        return;
    }
    if (ctx->duration == NULL || !lv_obj_is_valid(ctx->duration)) {
        home_mode_page_duration_build(info, ctx);
        home_mode_page_hero_layout(ctx);
        return;
    }
    lv_label_set_text(ctx->duration, duration);
    lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
    strncpy(ctx->shown_duration, duration, sizeof(ctx->shown_duration) - 1);
    ctx->shown_duration[sizeof(ctx->shown_duration) - 1] = '\0';
    home_mode_page_hero_layout(ctx);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_mode_chrome_prepare(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->root);
    if (info->mode_chrome.ready) {
        return;
    }
    memset(&info->mode_chrome, 0, sizeof(info->mode_chrome));
    home_mode_chrome_tip_rebuild(info);
    home_mode_chrome_dots_rebuild(info, HOME_PAGE_MAIN);
    info->mode_chrome.ready = 1;
    home_mode_chrome_visibility_apply(info);
}

void home_mode_chrome_remove(home_base_ui_info_t *info)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(info);
    for (i = 0; i < HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        LVGL_OBJ_DELETE(info->mode_chrome.dots[i]);
        info->mode_chrome.dots[i] = NULL;
    }
    /* 预解码 tip 由本模块静态持有，仅删控件、不 destroy draw_buf */
    ui_shine_remove(&info->mode_chrome.tip_shine);
    if (info->mode_chrome.tip_img != NULL && lv_obj_is_valid(info->mode_chrome.tip_img)) {
        if (info->mode_chrome.tip_predecoded) {
            lv_image_set_src(info->mode_chrome.tip_img, NULL);
        } else {
            ui_img_baj_release_decoded_buf(info->mode_chrome.tip_img);
        }
        LVGL_OBJ_DELETE(info->mode_chrome.tip_img);
        info->mode_chrome.tip_img = NULL;
    }
    memset(&info->mode_chrome, 0, sizeof(info->mode_chrome));
}

void home_mode_chrome_update(home_base_ui_info_t *info)
{
    uint8_t selected_page;

    EXCEPTION_CHECK_VOID(info);
    if (!info->mode_chrome.ready) {
        home_mode_chrome_prepare(info);
    }

    selected_page = info->page.transitioning ?
        info->page.page_index : info->page.shown_page_index;
    if (selected_page >= HOME_PAGE_MAX) {
        selected_page = HOME_PAGE_MAIN;
    }

    {
        const uint8_t dot_count = home_mode_page_dot_count_get(info);
        const uint8_t selected = home_mode_page_dot_index_get(info, selected_page);
        if (dot_count != info->mode_chrome.dot_count ||
            selected != info->mode_chrome.selected_dot ||
            info->force_update) {
            home_mode_chrome_dots_rebuild(info, selected_page);
        }
    }
    home_mode_chrome_visibility_apply(info);
    home_mode_chrome_tip_update(info);
}

void home_mode_page_ui_build(home_base_ui_info_t *info, uint8_t page_index)
{
    home_base_mode_page_ctx_t *ctx;
    lv_obj_t *page;

    EXCEPTION_CHECK_VOID(info);
    page = home_page_parent_get(info);
    EXCEPTION_CHECK_VOID(page);

    ctx = (home_base_mode_page_ctx_t *)u_malloz(sizeof(home_base_mode_page_ctx_t));
    EXCEPTION_CHECK_VOID(ctx);
    lv_obj_set_user_data(page, ctx);

    ctx->page_index = page_index;
    ctx->mode = home_mode_page_mode_from_page(page_index);

    home_mode_chrome_prepare(info);
    ctx->circle_bg = home_mode_page_circle_bg_create(page);
    home_mode_page_icon_build(info, ctx);
    home_mode_page_title_build(info, ctx);
    home_mode_page_duration_build(info, ctx);
    home_mode_page_hero_layout(ctx);
    if (ctx->circle_bg != NULL && lv_obj_is_valid(ctx->circle_bg)) {
        lv_obj_move_background(ctx->circle_bg);
    }
    /* 直达显示：稳态全尺寸；过渡入场时随后由 lerp 覆盖为缩小态 */
    home_mode_page_transition_apply(page, HOME_MODE_PAGE_SCALE_FULL, false);
}

void home_mode_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc)
{
    home_base_mode_page_ctx_t *ctx;
    char title[HOME_MODE_PAGE_TITLE_RAW_LEN] = {0};
    char capped[HOME_MODE_PAGE_TITLE_BUF_LEN] = {0};
    uint8_t need_ellipsis;

    EXCEPTION_CHECK_VOID(info && proc);
    ctx = home_mode_page_ctx_get(info);
    if (ctx == NULL) {
        return;
    }

    /// 个性化模式：刷新 App 下发的名称
    if (ctx->mode == BREAST_PUMP_CUSTOM_MODE || info->force_update) {
        home_mode_page_title_get(info, ctx->mode, title, sizeof(title));
        need_ellipsis = (strlen(title) > HOME_MODE_PAGE_TITLE_MAX_CHARS) ? 1U : 0U;
        if (need_ellipsis) {
            memcpy(capped, title, HOME_MODE_PAGE_TITLE_MAX_CHARS);
            capped[HOME_MODE_PAGE_TITLE_MAX_CHARS] = '\0';
        } else {
            strncpy(capped, title, sizeof(capped) - 1U);
            capped[sizeof(capped) - 1U] = '\0';
        }
        if (strncmp(ctx->shown_title, capped, sizeof(ctx->shown_title)) != 0 ||
            ctx->title_ellipsis != need_ellipsis ||
            info->force_update) {
            home_mode_page_title_store(ctx, title);
            home_mode_page_hero_layout(ctx);
        }
    }
    home_mode_page_duration_content_update(info, ctx);
    /* 缩放复位只在切页过渡里 apply；稳态每帧设 FULL 会反复 update_layout */
}

void home_mode_page_ui_remove(home_base_ui_info_t *info)
{
    home_base_mode_page_ctx_t *ctx;
    lv_obj_t *page;

    EXCEPTION_CHECK_VOID(info);
    page = home_page_parent_get(info);
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }

    ctx = (home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
    if (ctx == NULL) {
        return;
    }

    home_mode_page_icon_detach(ctx);
    lv_obj_set_user_data(page, NULL);
    u_free(ctx);
}

void home_mode_page_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning)
{
    home_base_mode_page_ctx_t *ctx;
    /* 放大未完成前保持圆+小字号；放大完成（或静止）再隐藏圆并切 36 */
    const bool small = transitioning && (hero_scale < HOME_MODE_PAGE_SCALE_FULL);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    ctx = (home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
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
        home_mode_page_obj_scale_set(ctx->circle_bg, HOME_MODE_PAGE_SCALE_FULL);
    }

    /* ICON 跟 hero_scale；标题不缩放，放大完成后再 29→36 */
    home_mode_page_obj_scale_set(ctx->icon, hero_scale);
    if (ctx->title != NULL && lv_obj_is_valid(ctx->title)) {
        const lv_font_t *font = small ?
            &lv_font_aeonik_soft_29_medium : &lv_font_aeonik_soft_33_medium;

        home_mode_page_obj_scale_set(ctx->title, HOME_MODE_PAGE_SCALE_FULL);
        lv_obj_set_style_text_font(ctx->title, font, 0);
    }

    /* 总时间：圆背景消失后展示，圆背景出现时隐藏 */
    if (ctx->duration != NULL && lv_obj_is_valid(ctx->duration)) {
        if (!small && home_mode_page_has_duration(ctx->mode)) {
            lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        }
    }
    /* 字号/显隐变化后按是否换行重排 ICON / 标题 / 时间 */
    home_mode_page_hero_layout(ctx);
}
