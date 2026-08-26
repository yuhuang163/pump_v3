/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "pump_seal_leak_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "storage_manager_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_SEAL_LEAK_SCREEN_W                         (360)
#define PUMP_SEAL_LEAK_SCALE_FULL                       (256)
#define PUMP_SEAL_LEAK_PROGRESS_FULL                    (256)
#define PUMP_SEAL_LEAK_PAGE_INDEX_INVALID               (0xFF)
#define PUMP_SEAL_LEAK_CUSTOM_NAME_LEN                  (24)

/** Check 页背景：参考图为黑底；标题为白色，不能用 0xFFFFFF 作底 */
#define PUMP_SEAL_LEAK_CHECK_BG_COLOR                   (0x000000)
#define PUMP_SEAL_LEAK_TITLE_COLOR                      (0xFFFFFF)
#define PUMP_SEAL_LEAK_DESC_COLOR                       (0x999999)
#define PUMP_SEAL_LEAK_TITLE_Y                          (48)
#define PUMP_SEAL_LEAK_TITLE_LINE_SPACE                 (8)
#define PUMP_SEAL_LEAK_ICON_Y                           (86)
#define PUMP_SEAL_LEAK_DESC_Y                           (296)
#define PUMP_SEAL_LEAK_DESC_LINE_SPACE                  (4)
/** 第一页（Air Seal Needs Attention）独立布局 */
#define PUMP_SEAL_LEAK_FIRST_TITLE \
    "Air Seal Needs\nAttention"
#define PUMP_SEAL_LEAK_FIRST_ICON_Y                     (104)
#define PUMP_SEAL_LEAK_FIRST_DESC_Y                     (314)

#define PUMP_SEAL_LEAK_PAUSE_TITLE_Y                    (64)
#define PUMP_SEAL_LEAK_PAUSE_WAVE_Y                     (136)
#define PUMP_SEAL_LEAK_PAUSE_ICON_Y                     (136)
#define PUMP_SEAL_LEAK_PAUSE_HINT_Y                     (204)
#define PUMP_SEAL_LEAK_PAUSE_HINT_LINE_SPACE            (8)
#define PUMP_SEAL_LEAK_PAUSE_WAVE_OPA                   (LV_OPA_60)
#define PUMP_SEAL_LEAK_PAUSE_HINT_TEXT \
    "Press to resume\nPress and hold to end pumping"
#define PUMP_SEAL_LEAK_PAUSE_HINT_COLOR_STIMULATION     (0xAD7C89)
#define PUMP_SEAL_LEAK_PAUSE_HINT_COLOR_LACTATION       (0xB9D9FF)
#define PUMP_SEAL_LEAK_PAUSE_HINT_COLOR_PROGRAM         (0xB195DF)
#define PUMP_SEAL_LEAK_PAUSE_HINT_SHINE_W               (140)

/** 力度/速度：X/宽/间距/对齐与 home 一致，整体相对 home 下移 21 */
#define PUMP_SEAL_LEAK_PAUSE_LEVEL_OFFSET_Y             (21)
#define PUMP_SEAL_LEAK_PAUSE_SPEED_GROUP_WIDTH          (47)
#define PUMP_SEAL_LEAK_PAUSE_SPEED_ICON_Y               (265 + 4 + PUMP_SEAL_LEAK_PAUSE_LEVEL_OFFSET_Y)
#define PUMP_SEAL_LEAK_PAUSE_SPEED_VALUE_Y              (267 + 4 + PUMP_SEAL_LEAK_PAUSE_LEVEL_OFFSET_Y)
#define PUMP_SEAL_LEAK_PAUSE_SPEED_ICON_VALUE_GAP       (6)
#define PUMP_SEAL_LEAK_PAUSE_LEFT_LEVEL_X               (68 + 4)
#define PUMP_SEAL_LEAK_PAUSE_LEFT_LEVEL_Y               (267 + 4 + PUMP_SEAL_LEAK_PAUSE_LEVEL_OFFSET_Y)
#define PUMP_SEAL_LEAK_PAUSE_LEFT_LEVEL_W               (58)
#define PUMP_SEAL_LEAK_PAUSE_RIGHT_LEVEL_X              (227 + 4)
#define PUMP_SEAL_LEAK_PAUSE_RIGHT_LEVEL_Y              (PUMP_SEAL_LEAK_PAUSE_LEFT_LEVEL_Y)
#define PUMP_SEAL_LEAK_PAUSE_RIGHT_LEVEL_W              (58)
#define PUMP_SEAL_LEAK_PAUSE_LEVEL_COLOR                (0xFFFFFF)
#define PUMP_SEAL_LEAK_PAUSE_SPEED_COLOR                (0xFFFFFF)

#define PUMP_SEAL_LEAK_DOT_DIAMETER                     (11)
#define PUMP_SEAL_LEAK_DOT_COLOR_ACTIVE                 (0xFFFFFF)
#define PUMP_SEAL_LEAK_DOT_COLOR_INACTIVE               (0x4D4D4D)
#define PUMP_SEAL_LEAK_DOT_COUNT                        (PUMP_SEAL_LEAK_PAGE_MAX)

#ifndef PUMP_SEAL_LEAK_PAGE_TRANSITION_MS
#define PUMP_SEAL_LEAK_PAGE_TRANSITION_MS               (400)
#endif
#ifndef PUMP_SEAL_LEAK_PAGE_TRANSITION_OPA_MIN
#define PUMP_SEAL_LEAK_PAGE_TRANSITION_OPA_MIN          (102)
#endif

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *title;
    lv_obj_t *icon;
    lv_obj_t *desc;
    lv_obj_t *wave;
    lv_obj_t *play_icon;
    lv_obj_t *hint_text;
    ui_shine_info_t hint_shine;
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;
    lv_obj_t *right_level;
    lv_draw_buf_t *icon_buf;
    lv_draw_buf_t *wave_buf;
    uint8_t page_index;
    uint16_t shown_bg_id;
    uint16_t shown_icon_id;
    uint16_t shown_wave_id;
} pump_seal_leak_page_ctx_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *page;
    lv_obj_t *peer_page;
    lv_obj_t *dots[PUMP_SEAL_LEAK_DOT_COUNT];
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t transitioning;
    uint8_t enter_instant;
    int8_t slide_dir;
    int16_t progress;
    uint32_t start_ms;
    uint8_t selected_dot;
    uint8_t force_update;
    uint8_t pump_mode;
    uint8_t mode_type;
    uint8_t left_level;
    uint8_t right_level;
    uint8_t speed;
    char custom_id_str[PUMP_SEAL_LEAK_CUSTOM_NAME_LEN];
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
} pump_seal_leak_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_seal_leak_ui_info_t *s_leak_info;

/** 4 点沿顶部圆弧排布（参考模式选择 5/6 点内圈） */
static const lv_point_t s_leak_dot_pos[PUMP_SEAL_LEAK_DOT_COUNT] = {
    {141, 8},
    {163, 5},
    {185, 5},
    {207, 8},
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool pump_seal_leak_is_check_page(uint8_t page_index)
{
    return page_index == PUMP_SEAL_LEAK_PAGE_CHECK_FIT ||
           page_index == PUMP_SEAL_LEAK_PAGE_CHECK_TUBING ||
           page_index == PUMP_SEAL_LEAK_PAGE_CHECK_CUP;
}

static uint16_t pump_seal_leak_check_icon_id_get(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_FIT:
            return BASE_RES_ID_CHECK_TUBING_ICON;
        case PUMP_SEAL_LEAK_PAGE_CHECK_TUBING:
            return BASE_RES_ID_CHECK_FIT_ICON;
        case PUMP_SEAL_LEAK_PAGE_CHECK_CUP:
            return BASE_RES_ID_CHECK_CUP_ICON;
        default:
            return BASE_RES_ID_NONE;
    }
}

static const char *pump_seal_leak_check_title_get(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_FIT:
            return PUMP_SEAL_LEAK_FIRST_TITLE;
        case PUMP_SEAL_LEAK_PAGE_CHECK_TUBING:
            return "Check Fit";
        case PUMP_SEAL_LEAK_PAGE_CHECK_CUP:
            return "Check Cup";
        default:
            return "";
    }
}

static const char *pump_seal_leak_check_desc_get(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_FIT:
            return "Reinsert tubing firmly";
        case PUMP_SEAL_LEAK_PAGE_CHECK_TUBING:
            return "Center the nipple\nSeal flange firmly";
        case PUMP_SEAL_LEAK_PAGE_CHECK_CUP:
            return "Snap rim shut\nSeat duckbill valve";
        default:
            return "";
    }
}

static uint16_t pump_seal_leak_pause_bg_id_get(uint8_t mode)
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

static uint32_t pump_seal_leak_pause_hint_color_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return PUMP_SEAL_LEAK_PAUSE_HINT_COLOR_STIMULATION;
        case BREAST_PUMP_LACTATION_MODE:
            return PUMP_SEAL_LEAK_PAUSE_HINT_COLOR_LACTATION;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return PUMP_SEAL_LEAK_PAUSE_HINT_COLOR_PROGRAM;
        default:
            return PUMP_SEAL_LEAK_PAUSE_HINT_COLOR_STIMULATION;
    }
}

static void pump_seal_leak_pause_title_get(pump_seal_leak_ui_info_t *info, char *buf, size_t buf_len)
{
    switch (info->pump_mode) {
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
            snprintf(buf, buf_len, "%s", info->custom_id_str);
            break;
        default:
            snprintf(buf, buf_len, "");
            break;
    }
}

static uint16_t pump_seal_leak_wave_frame_id_get(uint8_t mode, uint8_t mode_type)
{
    switch (mode_type) {
        case PUMP_MODE_STIMULATION:
            return PUMP_IS_COMMON_MODE(mode) ?
                BASE_RES_ID_STIMULATION_ANIM_START : BASE_RES_ID_PROGRAM_STIMULATION_ANIM_START;
        case PUMP_MODE_LACTATION:
            return PUMP_IS_COMMON_MODE(mode) ?
                BASE_RES_ID_EXPRESSION_ANIM_START : BASE_RES_ID_PROGRAM_EXPRESSION_ANIM_START;
        case PUMP_MODE_MIX:
            return BASE_RES_ID_STIMULATION_ANIM_START;
        case PUMP_MODE_IDLE:
            return BASE_RES_ID_PROGRAM_RELAX_ANIM_START;
        default:
            return BASE_RES_ID_STIMULATION_ANIM_START;
    }
}

static void pump_seal_leak_obj_flag_set(lv_obj_t *obj, bool visible)
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

static lv_obj_t *pump_seal_leak_label_create(lv_obj_t *parent, const lv_font_t *font,
    uint32_t color, lv_coord_t y, int32_t line_space)
{
    lv_obj_t *label = lv_label_create(parent);

    if (label == NULL) {
        return NULL;
    }
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    if (line_space != 0) {
        lv_obj_set_style_text_line_space(label, line_space, 0);
    }
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y);
    return label;
}

static pump_seal_leak_page_ctx_t *pump_seal_leak_page_ctx_get(lv_obj_t *page)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return NULL;
    }
    return (pump_seal_leak_page_ctx_t *)lv_obj_get_user_data(page);
}

static void pump_seal_leak_page_ctx_release(pump_seal_leak_page_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    ui_shine_remove(&ctx->hint_shine);
    if (ctx->bg != NULL && lv_obj_is_valid(ctx->bg)) {
        const void *src = lv_image_get_src(ctx->bg);

        if (src != NULL && get_predecoded_bg(ctx->shown_bg_id) == src) {
            lv_image_set_src(ctx->bg, NULL);
        } else {
            ui_img_png_release_decoded_buf(ctx->bg);
        }
    }
    if (ctx->icon != NULL && lv_obj_is_valid(ctx->icon)) {
        lv_image_set_src(ctx->icon, NULL);
    }
    if (ctx->play_icon != NULL && lv_obj_is_valid(ctx->play_icon)) {
        lv_image_set_src(ctx->play_icon, NULL);
    }
    if (ctx->wave != NULL && lv_obj_is_valid(ctx->wave)) {
        lv_image_set_src(ctx->wave, NULL);
    }
    if (ctx->icon_buf != NULL) {
        lv_draw_buf_destroy(ctx->icon_buf);
        ctx->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(ctx->icon);
        ui_img_baj_release_decoded_buf(ctx->play_icon);
    }
    if (ctx->wave_buf != NULL) {
        lv_draw_buf_destroy(ctx->wave_buf);
        ctx->wave_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(ctx->wave);
    }
    ui_img_baj_release_decoded_buf(ctx->speed_icon);
    u_free(ctx);
}

static void pump_seal_leak_page_content_remove(lv_obj_t *page)
{
    pump_seal_leak_page_ctx_t *ctx = pump_seal_leak_page_ctx_get(page);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    lv_obj_set_user_data(page, NULL);
    pump_seal_leak_page_ctx_release(ctx);
}

static lv_obj_t *pump_seal_leak_page_container_create(pump_seal_leak_ui_info_t *info)
{
    lv_obj_t *page = lv_obj_create(info->root);

    if (page == NULL) {
        return NULL;
    }
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(page, lv_color_hex(PUMP_SEAL_LEAK_CHECK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_align(page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_transform_pivot_x(page, PUMP_SEAL_LEAK_SCREEN_W / 2, 0);
    lv_obj_set_style_transform_pivot_y(page, PUMP_SEAL_LEAK_SCREEN_W / 2, 0);
    lv_obj_set_style_transform_scale(page, PUMP_SEAL_LEAK_SCALE_FULL, 0);
    lv_obj_set_style_opa(page, LV_OPA_COVER, 0);
    return page;
}

static void pump_seal_leak_check_page_build(pump_seal_leak_ui_info_t *info, lv_obj_t *page,
    uint8_t page_index)
{
    pump_seal_leak_page_ctx_t *ctx = (pump_seal_leak_page_ctx_t *)lv_obj_get_user_data(page);
    uint16_t icon_id = pump_seal_leak_check_icon_id_get(page_index);

    bool is_first = (page_index == PUMP_SEAL_LEAK_PAGE_CHECK_FIT);
    lv_coord_t title_y = PUMP_SEAL_LEAK_TITLE_Y;
    lv_coord_t icon_y = is_first ? PUMP_SEAL_LEAK_FIRST_ICON_Y : PUMP_SEAL_LEAK_ICON_Y;
    lv_coord_t desc_y = is_first ? PUMP_SEAL_LEAK_FIRST_DESC_Y : PUMP_SEAL_LEAK_DESC_Y;
    int32_t title_line_space = is_first ? PUMP_SEAL_LEAK_TITLE_LINE_SPACE : 0;

    EXCEPTION_CHECK_VOID(ctx);
    lv_obj_set_style_bg_color(page, lv_color_hex(PUMP_SEAL_LEAK_CHECK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    ctx->title = pump_seal_leak_label_create(page, &lv_font_aeonik_soft_26_medium,
        PUMP_SEAL_LEAK_TITLE_COLOR, title_y, title_line_space);
    EXCEPTION_CHECK_VOID(ctx->title);
    lv_label_set_text(ctx->title, pump_seal_leak_check_title_get(page_index));

    ctx->icon = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->icon);
    ctx->icon_buf = ui_img_baj_theme_decode_draw_buf(info->ui, icon_id, RES_THEME_TYPE_BASE,
        UI_IMG_BAJ_DECODE_RGBA8888);
    if (ctx->icon_buf != NULL) {
        lv_image_set_src(ctx->icon, ctx->icon_buf);
    } else {
        ui_img_baj_theme_set_src(info->ui, ctx->icon, icon_id, RES_THEME_TYPE_BASE,
            UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(ctx->icon, LV_ALIGN_TOP_MID, 0, icon_y);
    ctx->shown_icon_id = icon_id;

    ctx->desc = pump_seal_leak_label_create(page, &lv_font_aeonik_soft_19_medium,
        PUMP_SEAL_LEAK_DESC_COLOR, desc_y, PUMP_SEAL_LEAK_DESC_LINE_SPACE);
    EXCEPTION_CHECK_VOID(ctx->desc);
    lv_label_set_text(ctx->desc, pump_seal_leak_check_desc_get(page_index));
}

static void pump_seal_leak_pause_hint_style_get(ui_shine_style_t *style)
{
    ui_shine_style_init(style);
    style->mask_w = PUMP_SEAL_LEAK_PAUSE_HINT_SHINE_W;
    style->mask_y = PUMP_SEAL_LEAK_PAUSE_HINT_Y;
    style->screen_w = PUMP_SEAL_LEAK_SCREEN_W;
}

static void pump_seal_leak_pause_page_build(pump_seal_leak_ui_info_t *info, lv_obj_t *page)
{
    pump_seal_leak_page_ctx_t *ctx = (pump_seal_leak_page_ctx_t *)lv_obj_get_user_data(page);
    char title[PUMP_SEAL_LEAK_CUSTOM_NAME_LEN] = {0};
    uint16_t bg_id = pump_seal_leak_pause_bg_id_get(info->pump_mode);
    uint16_t wave_id = pump_seal_leak_wave_frame_id_get(info->pump_mode, info->mode_type);
    uint32_t hint_color = pump_seal_leak_pause_hint_color_get(info->pump_mode);
    lv_draw_buf_t *predecoded_bg;
    ui_shine_style_t style;
    char speed_text[8] = {0};
    char level_text[8] = {0};
    lv_coord_t icon_w = 0;
    lv_coord_t value_w = 0;
    lv_coord_t total_w;
    lv_coord_t start_x;

    EXCEPTION_CHECK_VOID(ctx);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(page, lv_color_black(), 0);

    ctx->bg = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->bg);
    lv_obj_move_background(ctx->bg);
    predecoded_bg = get_predecoded_bg(bg_id);
    if (predecoded_bg != NULL) {
        lv_image_set_src(ctx->bg, predecoded_bg);
    } else {
        ui_img_png_theme_set_src(info->ui, ctx->bg, bg_id, RES_THEME_TYPE_BASE,
            UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(ctx->bg, LV_ALIGN_CENTER, 0, 0);
    ctx->shown_bg_id = bg_id;

    pump_seal_leak_pause_title_get(info, title, sizeof(title));
    ctx->title = pump_seal_leak_label_create(page, &lv_font_aeonik_soft_26_medium,
        PUMP_SEAL_LEAK_TITLE_COLOR, PUMP_SEAL_LEAK_PAUSE_TITLE_Y, 0);
    EXCEPTION_CHECK_VOID(ctx->title);
    lv_label_set_text(ctx->title, title);

    ctx->wave = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->wave);
    ctx->wave_buf = ui_img_baj_theme_decode_draw_buf(info->ui, wave_id, RES_THEME_TYPE_BASE,
        UI_IMG_BAJ_DECODE_RGBA8888);
    if (ctx->wave_buf != NULL) {
        lv_image_set_src(ctx->wave, ctx->wave_buf);
    } else {
        ui_img_baj_theme_set_src(info->ui, ctx->wave, wave_id, RES_THEME_TYPE_BASE,
            UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_set_style_opa(ctx->wave, PUMP_SEAL_LEAK_PAUSE_WAVE_OPA, 0);
    lv_obj_align(ctx->wave, LV_ALIGN_TOP_MID, 0, PUMP_SEAL_LEAK_PAUSE_WAVE_Y);
    ctx->shown_wave_id = wave_id;

    ctx->play_icon = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->play_icon);
    ctx->icon_buf = ui_img_baj_theme_decode_draw_buf(info->ui, BASE_RES_ID_PLAY_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    if (ctx->icon_buf != NULL) {
        lv_image_set_src(ctx->play_icon, ctx->icon_buf);
    } else {
        ui_img_baj_theme_set_src(info->ui, ctx->play_icon, BASE_RES_ID_PLAY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(ctx->play_icon, LV_ALIGN_TOP_MID, 0, PUMP_SEAL_LEAK_PAUSE_ICON_Y);
    ctx->shown_icon_id = BASE_RES_ID_PLAY_ICON;

    ctx->hint_text = pump_seal_leak_label_create(page, &lv_font_aeonik_soft_19_medium,
        hint_color, PUMP_SEAL_LEAK_PAUSE_HINT_Y, PUMP_SEAL_LEAK_PAUSE_HINT_LINE_SPACE);
    EXCEPTION_CHECK_VOID(ctx->hint_text);
    lv_label_set_text(ctx->hint_text, PUMP_SEAL_LEAK_PAUSE_HINT_TEXT);

    pump_seal_leak_pause_hint_style_get(&style);
    ctx->hint_shine.mask_x = (int16_t)(-(int16_t)style.mask_w);
    ctx->hint_shine.mask_tick_ms = get_timestamp();
    ui_shine_prepare_from_text(&ctx->hint_shine, &style, PUMP_SEAL_LEAK_PAUSE_HINT_TEXT,
        &lv_font_aeonik_soft_19_medium, 0, PUMP_SEAL_LEAK_PAUSE_HINT_LINE_SPACE);
    ui_shine_update(&ctx->hint_shine, page, &style, ctx->hint_text, 1);
    ui_shine_show(&ctx->hint_shine);

    ctx->speed_icon = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->speed_icon);
    ui_img_baj_theme_set_src(info->ui, ctx->speed_icon, BASE_RES_ID_SPEED_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    ctx->speed_value = lv_label_create(page);
    EXCEPTION_CHECK_VOID(ctx->speed_value);
    lv_obj_set_style_text_color(ctx->speed_value, lv_color_hex(PUMP_SEAL_LEAK_PAUSE_SPEED_COLOR), 0);
    lv_obj_set_style_text_font(ctx->speed_value, &lv_font_aeonik_soft_26_medium, 0);
    snprintf(speed_text, sizeof(speed_text), "%u", (unsigned)(info->speed + 1));
    lv_label_set_text(ctx->speed_value, speed_text);
    lv_obj_update_layout(ctx->speed_icon);
    lv_obj_update_layout(ctx->speed_value);
    icon_w = lv_obj_get_width(ctx->speed_icon);
    value_w = lv_obj_get_width(ctx->speed_value);
    total_w = icon_w + PUMP_SEAL_LEAK_PAUSE_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > PUMP_SEAL_LEAK_PAUSE_SPEED_GROUP_WIDTH) {
        total_w = PUMP_SEAL_LEAK_PAUSE_SPEED_GROUP_WIDTH;
    }
    start_x = (PUMP_SEAL_LEAK_SCREEN_W - total_w) / 2;
    lv_obj_align(ctx->speed_icon, LV_ALIGN_TOP_LEFT, start_x, PUMP_SEAL_LEAK_PAUSE_SPEED_ICON_Y);
    lv_obj_align(ctx->speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + PUMP_SEAL_LEAK_PAUSE_SPEED_ICON_VALUE_GAP,
        PUMP_SEAL_LEAK_PAUSE_SPEED_VALUE_Y);

    ctx->left_level = lv_label_create(page);
    EXCEPTION_CHECK_VOID(ctx->left_level);
    lv_label_set_long_mode(ctx->left_level, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(ctx->left_level, PUMP_SEAL_LEAK_PAUSE_LEFT_LEVEL_W);
    lv_obj_set_style_text_color(ctx->left_level, lv_color_hex(PUMP_SEAL_LEAK_PAUSE_LEVEL_COLOR), 0);
    lv_obj_set_style_text_font(ctx->left_level, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(ctx->left_level, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_letter_space(ctx->left_level, 3, 0);
    snprintf(level_text, sizeof(level_text), "L%u", (unsigned)info->left_level);
    lv_label_set_text(ctx->left_level, level_text);
    lv_obj_align(ctx->left_level, LV_ALIGN_TOP_LEFT, PUMP_SEAL_LEAK_PAUSE_LEFT_LEVEL_X,
        PUMP_SEAL_LEAK_PAUSE_LEFT_LEVEL_Y);

    ctx->right_level = lv_label_create(page);
    EXCEPTION_CHECK_VOID(ctx->right_level);
    lv_label_set_long_mode(ctx->right_level, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(ctx->right_level, PUMP_SEAL_LEAK_PAUSE_RIGHT_LEVEL_W);
    lv_obj_set_style_text_color(ctx->right_level, lv_color_hex(PUMP_SEAL_LEAK_PAUSE_LEVEL_COLOR), 0);
    lv_obj_set_style_text_font(ctx->right_level, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(ctx->right_level, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_letter_space(ctx->right_level, 3, 0);
    snprintf(level_text, sizeof(level_text), "R%u", (unsigned)info->right_level);
    lv_label_set_text(ctx->right_level, level_text);
    lv_obj_align(ctx->right_level, LV_ALIGN_TOP_LEFT, PUMP_SEAL_LEAK_PAUSE_RIGHT_LEVEL_X,
        PUMP_SEAL_LEAK_PAUSE_RIGHT_LEVEL_Y);
}

static void pump_seal_leak_page_content_build(pump_seal_leak_ui_info_t *info, lv_obj_t *page,
    uint8_t page_index)
{
    pump_seal_leak_page_ctx_t *ctx;

    EXCEPTION_CHECK_VOID(info && page);
    ctx = (pump_seal_leak_page_ctx_t *)u_malloz(sizeof(pump_seal_leak_page_ctx_t));
    EXCEPTION_CHECK_VOID(ctx);
    ctx->page_index = page_index;
    lv_obj_set_user_data(page, ctx);

    if (pump_seal_leak_is_check_page(page_index)) {
        pump_seal_leak_check_page_build(info, page, page_index);
    } else {
        pump_seal_leak_pause_page_build(info, page);
    }
}

static void pump_seal_leak_page_transform_apply(lv_obj_t *page, int16_t offset_x, lv_opa_t opa)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    lv_obj_align(page, LV_ALIGN_CENTER, offset_x, 0);
    lv_obj_set_style_opa(page, opa, 0);
}

static void pump_seal_leak_dots_update(pump_seal_leak_ui_info_t *info, uint8_t selected)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(info && info->root);
    info->selected_dot = selected;
    for (i = 0; i < PUMP_SEAL_LEAK_DOT_COUNT; i++) {
        if (info->dots[i] == NULL || !lv_obj_is_valid(info->dots[i])) {
            info->dots[i] = lv_obj_create(info->root);
            EXCEPTION_CHECK_VOID(info->dots[i]);
            lv_obj_remove_style_all(info->dots[i]);
            lv_obj_set_size(info->dots[i], PUMP_SEAL_LEAK_DOT_DIAMETER, PUMP_SEAL_LEAK_DOT_DIAMETER);
            lv_obj_set_style_radius(info->dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(info->dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(info->dots[i], 0, 0);
            lv_obj_clear_flag(info->dots[i], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(info->dots[i], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_pos(info->dots[i], s_leak_dot_pos[i].x, s_leak_dot_pos[i].y);
        }
        lv_obj_set_style_bg_color(info->dots[i],
            lv_color_hex(i == selected ? PUMP_SEAL_LEAK_DOT_COLOR_ACTIVE :
                PUMP_SEAL_LEAK_DOT_COLOR_INACTIVE), 0);
        lv_obj_move_foreground(info->dots[i]);
        pump_seal_leak_obj_flag_set(info->dots[i], true);
    }
}

static void pump_seal_leak_page_transition_lerp(pump_seal_leak_ui_info_t *info)
{
    const int32_t slide_span = PUMP_SEAL_LEAK_SCREEN_W;
    const int32_t opa_min = PUMP_SEAL_LEAK_PAGE_TRANSITION_OPA_MIN;
    int32_t slide_p = info->progress;
    int16_t out_x;
    int16_t in_x;
    int32_t out_opa_i;
    int32_t in_opa_i;
    int8_t dir = info->slide_dir;

    if (slide_p < 0) {
        slide_p = 0;
    } else if (slide_p > PUMP_SEAL_LEAK_PROGRESS_FULL) {
        slide_p = PUMP_SEAL_LEAK_PROGRESS_FULL;
    }

    if (dir == (int8_t)PUMP_HOME_PAGE_SLIDE_LEFT) {
        out_x = (int16_t)((-slide_span * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL);
        in_x = (int16_t)(slide_span + out_x);
    } else {
        out_x = (int16_t)((slide_span * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL);
        in_x = (int16_t)(-slide_span + out_x);
    }

    out_opa_i = LV_OPA_COVER -
        ((LV_OPA_COVER - opa_min) * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL;
    in_opa_i = opa_min +
        ((LV_OPA_COVER - opa_min) * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL;

    pump_seal_leak_page_transform_apply(info->page, out_x, (lv_opa_t)out_opa_i);
    pump_seal_leak_page_transform_apply(info->peer_page, in_x, (lv_opa_t)in_opa_i);
}

static void pump_seal_leak_page_switch_instant(pump_seal_leak_ui_info_t *info, uint8_t page_index)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->page != NULL && lv_obj_is_valid(info->page)) {
        pump_seal_leak_page_content_remove(info->page);
        LVGL_OBJ_DELETE(info->page);
    }
    if (info->peer_page != NULL && lv_obj_is_valid(info->peer_page)) {
        pump_seal_leak_page_content_remove(info->peer_page);
        LVGL_OBJ_DELETE(info->peer_page);
    }
    info->page = pump_seal_leak_page_container_create(info);
    EXCEPTION_CHECK_VOID(info->page);
    pump_seal_leak_page_content_build(info, info->page, page_index);
    info->page_index = page_index;
    info->shown_page_index = page_index;
    info->transitioning = 0;
    info->slide_dir = PUMP_HOME_PAGE_SLIDE_NONE;
    info->progress = 0;
    pump_seal_leak_dots_update(info, page_index);
}

static void pump_seal_leak_page_transition_finish(pump_seal_leak_ui_info_t *info)
{
    const uint8_t new_index = info->page_index;

    pump_seal_leak_page_content_remove(info->page);
    LVGL_OBJ_DELETE(info->page);
    info->page = info->peer_page;
    info->peer_page = NULL;
    info->shown_page_index = new_index;
    info->transitioning = 0;
    info->slide_dir = PUMP_HOME_PAGE_SLIDE_NONE;
    info->progress = 0;
    pump_seal_leak_page_transform_apply(info->page, 0, LV_OPA_COVER);
    pump_seal_leak_dots_update(info, new_index);
}

static void pump_seal_leak_page_transition_tick(pump_seal_leak_ui_info_t *info)
{
    uint32_t elapsed;
    int32_t progress;

    if (!info->transitioning) {
        return;
    }
    elapsed = get_timestamp() - info->start_ms;
    if (elapsed >= PUMP_SEAL_LEAK_PAGE_TRANSITION_MS) {
        progress = PUMP_SEAL_LEAK_PROGRESS_FULL;
    } else {
        progress = (int32_t)((elapsed * PUMP_SEAL_LEAK_PROGRESS_FULL) / PUMP_SEAL_LEAK_PAGE_TRANSITION_MS);
    }
    info->progress = (int16_t)progress;
    pump_seal_leak_page_transition_lerp(info);
    if (progress >= PUMP_SEAL_LEAK_PROGRESS_FULL) {
        pump_seal_leak_page_transition_finish(info);
    }
}

static void pump_seal_leak_page_goto(pump_seal_leak_ui_info_t *info, uint8_t page_index)
{
    pump_home_page_slide_dir_e dir;

    EXCEPTION_CHECK_VOID(info);
    if (info->transitioning) {
        return;
    }
    if (info->page == NULL || !lv_obj_is_valid(info->page)) {
        return;
    }
    if (page_index == info->shown_page_index || page_index >= PUMP_SEAL_LEAK_PAGE_MAX) {
        info->page_index = page_index;
        return;
    }

    dir = (page_index > info->shown_page_index) ?
            PUMP_HOME_PAGE_SLIDE_LEFT : PUMP_HOME_PAGE_SLIDE_RIGHT;
    info->peer_page = pump_seal_leak_page_container_create(info);
    EXCEPTION_CHECK_VOID(info->peer_page);
    pump_seal_leak_page_content_build(info, info->peer_page, page_index);
    info->page_index = page_index;
    info->slide_dir = (int8_t)dir;
    info->transitioning = 1;
    info->progress = 0;
    info->start_ms = get_timestamp();
    pump_seal_leak_page_transition_lerp(info);
    pump_seal_leak_dots_update(info, page_index);
}

static void pump_seal_leak_pause_shine_anim_update(lv_obj_t *page)
{
    pump_seal_leak_page_ctx_t *ctx = pump_seal_leak_page_ctx_get(page);
    ui_shine_style_t style;

    if (ctx == NULL || ctx->page_index != PUMP_SEAL_LEAK_PAGE_PAUSE) {
        return;
    }
    pump_seal_leak_pause_hint_style_get(&style);
    ui_shine_anim_update(&ctx->hint_shine, &style, ctx->hint_text, 0);
}

static void pump_seal_leak_root_init(void)
{
    if (s_leak_info->root == NULL || !lv_obj_is_valid(s_leak_info->root)) {
        s_leak_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(s_leak_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_leak_info->root, lv_color_hex(PUMP_SEAL_LEAK_CHECK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(s_leak_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_leak_info->root, 0, 0);
    lv_obj_set_style_pad_all(s_leak_info->root, 0, 0);
    lv_obj_align(s_leak_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(s_leak_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_leak_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_seal_leak_runtime_sync(pump_seal_leak_ui_info_t *info, ui_proc_pump_t *proc)
{
    pump_setting_t setting;
    int time_index = 0;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(info && proc && proc->pump);
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, setting);
    info->left_level = setting.level[0];
    info->right_level = setting.level[1];
    info->speed = setting.speed;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    info->pump_mode = (uint8_t)pump_mode;
    if (proc->motor && proc->motor->get_runtime_mode_type) {
        info->mode_type = proc->motor->get_runtime_mode_type(false);
    }
    if (info->st) {
        info->st->get_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE,
            sizeof(info->custom_id_str), info->custom_id_str);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_seal_leak_prepare(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_VOID(thiz);
    if (s_leak_info == NULL) {
        s_leak_info = (pump_seal_leak_ui_info_t *)u_malloz(sizeof(pump_seal_leak_ui_info_t));
        EXCEPTION_CHECK_VOID(s_leak_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_seal_leak_root_init();
    s_leak_info->ui = thiz;
    s_leak_info->force_update = true;
    s_leak_info->enter_instant = 1;
    s_leak_info->shown_page_index = PUMP_SEAL_LEAK_PAGE_INDEX_INVALID;
    s_leak_info->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    pump_seal_leak_runtime_sync(s_leak_info, proc);
}

void ui_display_pump_seal_leak_remove(ui_manager_context_t *thiz)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(thiz && s_leak_info && s_leak_info->root);
    if (s_leak_info->page != NULL && lv_obj_is_valid(s_leak_info->page)) {
        pump_seal_leak_page_content_remove(s_leak_info->page);
        LVGL_OBJ_DELETE(s_leak_info->page);
    }
    if (s_leak_info->peer_page != NULL && lv_obj_is_valid(s_leak_info->peer_page)) {
        pump_seal_leak_page_content_remove(s_leak_info->peer_page);
        LVGL_OBJ_DELETE(s_leak_info->peer_page);
    }
    for (i = 0; i < PUMP_SEAL_LEAK_DOT_COUNT; i++) {
        LVGL_OBJ_DELETE(s_leak_info->dots[i]);
    }
    LVGL_OBJ_DELETE(s_leak_info->root);
    u_free(s_leak_info);
    s_leak_info = NULL;
}

int ui_display_pump_seal_leak_process(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;
    uint8_t target_index;

    EXCEPTION_CHECK_INT(thiz && s_leak_info && s_leak_info->root);
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    pump_seal_leak_runtime_sync(s_leak_info, proc);

    target_index = (uint8_t)proc->seal_leak_page_index;
    if (target_index >= PUMP_SEAL_LEAK_PAGE_MAX) {
        target_index = PUMP_SEAL_LEAK_PAGE_CHECK_FIT;
    }

    if (s_leak_info->shown_page_index == PUMP_SEAL_LEAK_PAGE_INDEX_INVALID ||
        s_leak_info->enter_instant) {
        if (s_leak_info->shown_page_index != target_index) {
            pump_seal_leak_page_switch_instant(s_leak_info, target_index);
        }
        s_leak_info->enter_instant = 0;
    } else if (!s_leak_info->transitioning &&
               s_leak_info->shown_page_index != target_index) {
        pump_seal_leak_page_goto(s_leak_info, target_index);
    }

    pump_seal_leak_page_transition_tick(s_leak_info);
    pump_seal_leak_pause_shine_anim_update(s_leak_info->page);
    pump_seal_leak_pause_shine_anim_update(s_leak_info->peer_page);
    pump_seal_leak_dots_update(s_leak_info, s_leak_info->page_index);
    s_leak_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump seal leak fps: %d, page:%d", thiz->priv.fps_count,
            proc->seal_leak_page_index);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
