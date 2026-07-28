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

#define PUMP_MODE_PAGE_ICON_Y                       (100)
#define PUMP_MODE_PAGE_TITLE_Y                      (214)
#define PUMP_MODE_PAGE_TITLE_COLOR                  (0xFFFFFF)

#define PUMP_MODE_PAGE_SCALE_FULL                   (256)
/** 5 点布局第 3 点 X 居中：(屏宽 - 直径) / 2 */
#define PUMP_MODE_PAGE_DOT_X_CENTER \
    ((PANLE_DISPLAY_W - PUMP_MODE_PAGE_DOT_DIAMETER) / 2)

#define PUMP_MODE_TIME_LABEL_Y                      (293)
#define PUMP_MODE_TIME_ICON_X                       (133)
#define PUMP_MODE_TIME_ICON_Y                       (311)
#define PUMP_MODE_TIME_M_X                          (168)
#define PUMP_MODE_TIME_Y                            (308)
#define PUMP_MODE_TIME_SEG_WIDTH                    (36)
#define PUMP_MODE_TIME_COLON_GAP                    (2)
#define PUMP_MODE_TIME_COLOR                        (0xFFFFFF)

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
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x, lv_coord_t seg_y)
{
    const lv_coord_t cell_w = PUMP_MODE_TIME_SEG_WIDTH / PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};

    for (int i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(PUMP_MODE_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], &lv_font_aeonik_soft_26_medium, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, seg_y);
    }
}

static lv_coord_t pump_mode_chrome_time_sec_x_get(pump_home_base_mode_chrome_info_t *chrome)
{
    lv_coord_t colon_w = 10;

    if (chrome->time_colon != NULL && lv_obj_is_valid(chrome->time_colon)) {
        lv_obj_update_layout(chrome->time_colon);
        colon_w = lv_obj_get_width(chrome->time_colon);
        if (colon_w <= 0) {
            colon_w = 10;
        }
    }
    return PUMP_MODE_TIME_M_X + PUMP_MODE_TIME_SEG_WIDTH + PUMP_MODE_TIME_COLON_GAP +
        colon_w + PUMP_MODE_TIME_COLON_GAP;
}

static void pump_mode_chrome_time_rebuild(pump_home_base_ui_info_t *info)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;
    lv_coord_t colon_x;
    lv_coord_t sec_x;

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

    colon_x = PUMP_MODE_TIME_M_X + PUMP_MODE_TIME_SEG_WIDTH + PUMP_MODE_TIME_COLON_GAP;
    if (chrome->time_colon == NULL || !lv_obj_is_valid(chrome->time_colon)) {
        chrome->time_colon = lv_label_create(root);
        EXCEPTION_CHECK_VOID(chrome->time_colon);
        lv_obj_set_style_text_color(chrome->time_colon, lv_color_hex(PUMP_MODE_TIME_COLOR), 0);
        lv_obj_set_style_text_font(chrome->time_colon, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(chrome->time_colon, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(chrome->time_colon, ":");
    }
    lv_obj_align(chrome->time_colon, LV_ALIGN_TOP_LEFT, colon_x, PUMP_MODE_TIME_Y);
    sec_x = pump_mode_chrome_time_sec_x_get(chrome);

    chrome->shown_minutes = 0xFF;
    chrome->shown_seconds = 0xFF;
    pump_mode_chrome_time_pair_update(chrome->time_m, root, 0,
        PUMP_MODE_TIME_M_X, PUMP_MODE_TIME_Y);
    pump_mode_chrome_time_pair_update(chrome->time_s, root, 0, sec_x, PUMP_MODE_TIME_Y);
}

static void pump_mode_chrome_time_content_update(pump_home_base_ui_info_t *info,
    ui_proc_pump_t *proc)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    const pump_result_info_t *result_info = NULL;
    uint16_t hold_time;
    uint8_t minutes;
    uint8_t seconds;
    bool paused;
    uint16_t icon_id;
    const lv_coord_t colon_x = PUMP_MODE_TIME_M_X + PUMP_MODE_TIME_SEG_WIDTH + PUMP_MODE_TIME_COLON_GAP;
    lv_coord_t sec_x;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    EXCEPTION_CHECK_VOID(result_info);
    hold_time = result_info->work_valid_time / 1000;
    minutes = (hold_time / 60) % 60;
    seconds = hold_time % 60;
    paused = (proc->pump_status == BREAST_PUMP_PAUSE);
    icon_id = paused ? BASE_RES_ID_PLAY_MIN_ICON : BASE_RES_ID_PAUSE_MIN_ICON;
    sec_x = pump_mode_chrome_time_sec_x_get(chrome);

    if (chrome->status_icon != NULL && lv_obj_is_valid(chrome->status_icon) &&
        (chrome->shown_status_icon_id != icon_id || info->force_update ||
         chrome->shown_paused != (paused ? 1 : 0))) {
        ui_img_baj_theme_set_src(info->ui, chrome->status_icon, icon_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(chrome->status_icon, LV_ALIGN_TOP_LEFT,
            PUMP_MODE_TIME_ICON_X, PUMP_MODE_TIME_ICON_Y);
        chrome->shown_status_icon_id = icon_id;
        chrome->shown_paused = paused ? 1 : 0;
    }

    if (chrome->shown_minutes != minutes || info->force_update) {
        pump_mode_chrome_time_pair_update(chrome->time_m, info->root, minutes,
            PUMP_MODE_TIME_M_X, PUMP_MODE_TIME_Y);
        chrome->shown_minutes = minutes;
    }
    if (chrome->shown_seconds != seconds || info->force_update) {
        pump_mode_chrome_time_pair_update(chrome->time_s, info->root, seconds,
            sec_x, PUMP_MODE_TIME_Y);
        chrome->shown_seconds = seconds;
    }
    if (chrome->time_colon != NULL && lv_obj_is_valid(chrome->time_colon)) {
        lv_obj_align(chrome->time_colon, LV_ALIGN_TOP_LEFT, colon_x, PUMP_MODE_TIME_Y);
    }
}

static void pump_mode_chrome_visibility_apply(pump_home_base_ui_info_t *info)
{
    pump_home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    const uint8_t shown = info->page.shown_page_index;
    const bool transitioning = info->page.transitioning != 0;
    /* page0 静止：隐藏；离 page0 动画一开始即显示定位点/时间 */
    const bool chrome_show = !(shown == PUMP_HOME_PAGE_MAIN && !transitioning);
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
    pump_mode_page_obj_flag_set(chrome->time_colon, chrome_show);
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
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
        if (chrome->time_colon != NULL && lv_obj_is_valid(chrome->time_colon)) {
            lv_obj_move_foreground(chrome->time_colon);
        }
        for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
            if (chrome->time_m[i] != NULL && lv_obj_is_valid(chrome->time_m[i])) {
                lv_obj_move_foreground(chrome->time_m[i]);
            }
            if (chrome->time_s[i] != NULL && lv_obj_is_valid(chrome->time_s[i])) {
                lv_obj_move_foreground(chrome->time_s[i]);
            }
        }
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
    lv_obj_align(ctx->icon, LV_ALIGN_TOP_MID, 0, PUMP_MODE_PAGE_ICON_Y);
    ctx->icon_id = res_id;
}

static void pump_mode_page_title_build(pump_home_base_ui_info_t *info, pump_home_base_mode_page_ctx_t *ctx)
{
    char title[24] = {0};
    lv_obj_t *parent = pump_home_page_parent_get(info);

    pump_mode_page_title_get(info, ctx->mode, title, sizeof(title));

    if (ctx->title == NULL || !lv_obj_is_valid(ctx->title)) {
        ctx->title = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(ctx->title);
        lv_obj_set_style_text_color(ctx->title, lv_color_hex(PUMP_MODE_PAGE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(ctx->title, &lv_font_aeonik_soft_36_medium, 0);
        lv_obj_set_style_text_align(ctx->title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(ctx->title, 0, 0);
    }
    lv_label_set_text(ctx->title, title);
    lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, PUMP_MODE_PAGE_TITLE_Y);
    strncpy(ctx->shown_title, title, sizeof(ctx->shown_title) - 1);
    ctx->shown_title[sizeof(ctx->shown_title) - 1] = '\0';
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
    LVGL_OBJ_DELETE(info->mode_chrome.time_colon);
    info->mode_chrome.time_colon = NULL;
    for (i = 0; i < PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT; i++) {
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
    if (ctx->circle_bg != NULL && lv_obj_is_valid(ctx->circle_bg)) {
        lv_obj_move_background(ctx->circle_bg);
    }
}

void pump_home_mode_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc)
{
    pump_home_base_mode_page_ctx_t *ctx;
    char title[24] = {0};

    EXCEPTION_CHECK_VOID(info && proc);
    ctx = pump_mode_page_ctx_get(info);
    if (ctx == NULL) {
        return;
    }

    /// 个性化 ID 变化时刷新 Customized 文案
    if (ctx->mode == BREAST_PUMP_CUSTOM_MODE || info->force_update) {
        pump_mode_page_title_get(info, ctx->mode, title, sizeof(title));
        if (strncmp(ctx->shown_title, title, sizeof(ctx->shown_title)) != 0 ||
            info->force_update) {
            if (ctx->title != NULL && lv_obj_is_valid(ctx->title)) {
                lv_label_set_text(ctx->title, title);
                lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, PUMP_MODE_PAGE_TITLE_Y);
            }
            strncpy(ctx->shown_title, title, sizeof(ctx->shown_title) - 1);
            ctx->shown_title[sizeof(ctx->shown_title) - 1] = '\0';
        }
    }

    if (!pump_home_page_ui_is_transitioning(info)) {
        pump_home_mode_page_transition_apply(pump_home_page_parent_get(info),
            PUMP_MODE_PAGE_SCALE_FULL, false);
    }
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
            &lv_font_aeonik_soft_29_medium : &lv_font_aeonik_soft_36_medium;

        pump_mode_page_obj_scale_set(ctx->title, PUMP_MODE_PAGE_SCALE_FULL);
        lv_obj_set_style_text_font(ctx->title, font, 0);
        lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, PUMP_MODE_PAGE_TITLE_Y);
    }
}
