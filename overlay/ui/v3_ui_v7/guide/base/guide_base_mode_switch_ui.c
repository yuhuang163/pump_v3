/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_font_img_tip.h" // IWYU pragma: keep
#include "storage_manager_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_MODE_SWITCH_SCALE_FULL                    (256)
#define GUIDE_MODE_SWITCH_PROGRESS_FULL                 (256)
#define GUIDE_MODE_SWITCH_SLOT_INVALID                  (0xFF)

#define GUIDE_MODE_SWITCH_TRANSITION_MS                 (400)
#define GUIDE_MODE_SWITCH_TRANSITION_HERO_MS            (120)
#define GUIDE_MODE_SWITCH_HERO_SCALE_MIN                ((256 * 85) / 100)
#define GUIDE_MODE_SWITCH_SLIDE_PROGRESS \
    (((GUIDE_MODE_SWITCH_TRANSITION_MS - GUIDE_MODE_SWITCH_TRANSITION_HERO_MS) * 256) / \
     GUIDE_MODE_SWITCH_TRANSITION_MS)
#define GUIDE_MODE_SWITCH_CIRCLE_SIZE                   (260)
#define GUIDE_MODE_SWITCH_CIRCLE_COLOR                  (0x181818)
#define GUIDE_MODE_SWITCH_OPA_MIN                       (102)

#define GUIDE_MODE_SWITCH_DOT_DIAMETER                  (11)
#define GUIDE_MODE_SWITCH_DOT_COLOR_ACTIVE              (0xFFFFFF)
#define GUIDE_MODE_SWITCH_DOT_COLOR_INACTIVE            (0x4D4D4D)

#define GUIDE_MODE_SWITCH_TITLE_COLOR                   (0xFFFFFF)
#define GUIDE_MODE_SWITCH_DURATION_COLOR                (0x999999)

#define GUIDE_MODE_SWITCH_TIP_Y                         (PANLE_DISPLAY_H - GUIDE_MODE_SWITCH_TIP_H - 8)
#define GUIDE_MODE_SWITCH_TIP_BASE_OPA                  (LV_OPA_70)

#define GUIDE_MODE_SWITCH_SLIDE_NONE                    (0)
#define GUIDE_MODE_SWITCH_SLIDE_LEFT                    (1)
#define GUIDE_MODE_SWITCH_SLIDE_RIGHT                   (2)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/** 4 点布局（无首页 / Custom） */
static const lv_point_t s_guide_mode_dot_pos_4[GUIDE_MODE_SWITCH_SLOT_COUNT] = {
    {141, 8},
    {163, 5},
    {186, 5},
    {208, 8},
};

/*
* LOCAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
static void guide_mode_switch_transition_apply(lv_obj_t *page, int32_t hero_scale,
    bool transitioning);

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t guide_mode_switch_icon_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return BASE_RES_ID_STIMULATION_ICON;
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_ICON;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return BASE_RES_ID_PROGRAM_ICON;
        default:
            return BASE_RES_ID_STIMULATION_ICON;
    }
}

static void guide_mode_switch_title_get(uint8_t mode, char *buf, size_t buf_len)
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
        default:
            buf[0] = '\0';
            break;
    }
}

static bool guide_mode_switch_has_duration(uint8_t mode)
{
    return mode == BREAST_PUMP_MILK_BOOST_MODE || mode == BREAST_PUMP_PP_MILK_TRACK_MODE;
}

static void guide_mode_switch_duration_text_get(uint8_t mode, char *buf, size_t buf_len)
{
    storage_manager_context_t *st;
    int pump_time = 0;

    if (buf == NULL || buf_len == 0) {
        return;
    }
    buf[0] = '\0';
    if (!guide_mode_switch_has_duration(mode)) {
        return;
    }
    st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st != NULL && st->get_store_ram_val != NULL) {
        st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, mode, 0, &pump_time);
    }
    snprintf(buf, buf_len, UI_STR_DURATION_APPROX_MIN_FMT, pump_time / 60);
}

static void guide_mode_switch_obj_scale_set(lv_obj_t *obj, int32_t scale)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_update_layout(obj);
    lv_obj_set_style_transform_pivot_x(obj, lv_obj_get_width(obj) / 2, 0);
    lv_obj_set_style_transform_pivot_y(obj, lv_obj_get_height(obj) / 2, 0);
    lv_obj_set_style_transform_scale(obj, scale, 0);
}

static void guide_mode_switch_page_transform_apply(lv_obj_t *page, int16_t offset_x,
    int32_t scale, lv_opa_t opa)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    lv_obj_align(page, LV_ALIGN_CENTER, offset_x, 0);
    lv_obj_set_style_transform_pivot_x(page, lv_obj_get_width(page) / 2, 0);
    lv_obj_set_style_transform_pivot_y(page, lv_obj_get_height(page) / 2, 0);
    lv_obj_set_style_transform_scale(page, scale, 0);
    lv_obj_set_style_opa(page, opa, 0);
}

static void guide_mode_switch_page_transform_reset(lv_obj_t *page)
{
    guide_mode_switch_page_transform_apply(page, 0, GUIDE_MODE_SWITCH_SCALE_FULL, LV_OPA_COVER);
}

static guide_base_mode_switch_card_t *guide_mode_switch_card_from_page(lv_obj_t *page)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return NULL;
    }
    return (guide_base_mode_switch_card_t *)lv_obj_get_user_data(page);
}

static lv_obj_t *guide_mode_switch_circle_create(lv_obj_t *parent)
{
    lv_obj_t *circle = lv_obj_create(parent);

    EXCEPTION_CHECK_POINT(circle);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, GUIDE_MODE_SWITCH_CIRCLE_SIZE, GUIDE_MODE_SWITCH_CIRCLE_SIZE);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(GUIDE_MODE_SWITCH_CIRCLE_COLOR), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(circle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(circle);
    return circle;
}

static void guide_mode_switch_icon_detach(guide_base_mode_switch_card_t *card)
{
    const void *src;

    if (card == NULL || card->icon == NULL || !lv_obj_is_valid(card->icon)) {
        return;
    }
    src = lv_image_get_src(card->icon);
    if (src == NULL) {
        return;
    }
    if (card->icon_predecoded || get_predecoded_icon(card->icon_id) == src) {
        lv_image_set_src(card->icon, NULL);
        card->icon_predecoded = 0;
        return;
    }
    ui_img_png_release_decoded_buf(card->icon);
}

static void guide_mode_switch_hero_layout(guide_base_mode_switch_card_t *card)
{
    if (card == NULL) {
        return;
    }
    if (card->icon != NULL && lv_obj_is_valid(card->icon)) {
        lv_obj_align(card->icon, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SWITCH_ICON_Y);
    }
    if (card->title != NULL && lv_obj_is_valid(card->title)) {
        lv_obj_align(card->title, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SWITCH_TITLE_Y);
    }
    if (card->duration != NULL && lv_obj_is_valid(card->duration) &&
        !lv_obj_has_flag(card->duration, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_align(card->duration, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SWITCH_DURATION_Y);
    }
}

static void guide_mode_switch_card_content_build(guide_base_ui_info_t *info, lv_obj_t *page,
    uint8_t slot)
{
    guide_base_mode_switch_card_t *card;
    char title[32] = {0};
    char duration[16] = {0};
    lv_draw_buf_t *predecoded;
    uint16_t res_id;

    EXCEPTION_CHECK_VOID(info && page);
    card = (guide_base_mode_switch_card_t *)u_malloz(sizeof(*card));
    EXCEPTION_CHECK_VOID(card);
    lv_obj_set_user_data(page, card);

    card->slot = slot;
    card->mode = guide_mode_switch_mode_from_slot(slot);
    card->circle_bg = guide_mode_switch_circle_create(page);

    res_id = guide_mode_switch_icon_res_id_get(card->mode);
    card->icon = lv_image_create(page);
    EXCEPTION_CHECK_VOID(card->icon);
    predecoded = get_predecoded_icon(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(card->icon, predecoded);
        card->icon_predecoded = 1;
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, card->icon, res_id, RES_THEME_TYPE_BASE,
            UI_IMG_PNG_DECODE_RGBA8888);
    }
    card->icon_id = res_id;

    guide_mode_switch_title_get(card->mode, title, sizeof(title));
    card->title = lv_label_create(page);
    EXCEPTION_CHECK_VOID(card->title);
    lv_label_set_text(card->title, title);
    lv_obj_set_style_text_color(card->title, lv_color_hex(GUIDE_MODE_SWITCH_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(card->title, UI_FONT_TITLE_LG, 0);
    lv_obj_set_style_text_align(card->title, LV_TEXT_ALIGN_CENTER, 0);

    if (guide_mode_switch_has_duration(card->mode)) {
        guide_mode_switch_duration_text_get(card->mode, duration, sizeof(duration));
        card->duration = lv_label_create(page);
        EXCEPTION_CHECK_VOID(card->duration);
        lv_label_set_text(card->duration, duration);
        lv_obj_set_style_text_color(card->duration, lv_color_hex(GUIDE_MODE_SWITCH_DURATION_COLOR), 0);
        lv_obj_set_style_text_font(card->duration, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_align(card->duration, LV_TEXT_ALIGN_CENTER, 0);
    }

    if (card->circle_bg != NULL && lv_obj_is_valid(card->circle_bg)) {
        lv_obj_move_background(card->circle_bg);
    }
    guide_mode_switch_hero_layout(card);
    guide_mode_switch_transition_apply(page, GUIDE_MODE_SWITCH_SCALE_FULL, false);
}

static void guide_mode_switch_card_content_remove(lv_obj_t *page)
{
    guide_base_mode_switch_card_t *card = guide_mode_switch_card_from_page(page);

    if (card == NULL) {
        return;
    }
    guide_mode_switch_icon_detach(card);
    lv_obj_set_user_data(page, NULL);
    u_free(card);
}

static lv_obj_t *guide_mode_switch_container_create(guide_base_ui_info_t *info)
{
    lv_obj_t *page = lv_obj_create(info->root);

    if (page == NULL) {
        return NULL;
    }
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_CLICKABLE);
    guide_mode_switch_page_transform_reset(page);
    return page;
}

static void guide_mode_switch_dots_rebuild(guide_base_ui_info_t *info, uint8_t selected)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;
    uint8_t i;

    EXCEPTION_CHECK_VOID(info->root);
    for (i = 0; i < GUIDE_MODE_SWITCH_SLOT_COUNT; i++) {
        if (ms->dots[i] == NULL || !lv_obj_is_valid(ms->dots[i])) {
            ms->dots[i] = lv_obj_create(info->root);
            EXCEPTION_CHECK_VOID(ms->dots[i]);
            lv_obj_remove_style_all(ms->dots[i]);
            lv_obj_set_size(ms->dots[i], GUIDE_MODE_SWITCH_DOT_DIAMETER, GUIDE_MODE_SWITCH_DOT_DIAMETER);
            lv_obj_set_style_radius(ms->dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(ms->dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(ms->dots[i], 0, 0);
            lv_obj_clear_flag(ms->dots[i], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(ms->dots[i], LV_OBJ_FLAG_CLICKABLE);
        }
        lv_obj_set_style_bg_color(ms->dots[i],
            lv_color_hex(i == selected ? GUIDE_MODE_SWITCH_DOT_COLOR_ACTIVE :
                GUIDE_MODE_SWITCH_DOT_COLOR_INACTIVE), 0);
        lv_obj_set_pos(ms->dots[i], s_guide_mode_dot_pos_4[i].x, s_guide_mode_dot_pos_4[i].y);
        lv_obj_clear_flag(ms->dots[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(ms->dots[i]);
    }
    ms->selected_dot = selected;
}

static lv_draw_buf_t *guide_mode_switch_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->mode_switch.tip_buf != NULL) {
        return info->mode_switch.tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->mode_switch.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->mode_switch.tip_buf;
}

static void guide_mode_switch_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(info->root);
    if (ms->tip_img != NULL && lv_obj_is_valid(ms->tip_img) &&
        ms->tip_shine.mask != NULL && lv_obj_is_valid(ms->tip_shine.mask) &&
        ms->tip_shine.shine != NULL && lv_obj_is_valid(ms->tip_shine.shine)) {
        return;
    }

    predecoded = guide_mode_switch_tip_predecode(info);
    if (ms->tip_img == NULL || !lv_obj_is_valid(ms->tip_img)) {
        ms->tip_img = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(ms->tip_img);
        lv_obj_clear_flag(ms->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(ms->tip_img, predecoded);
            ms->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(ms->tip_img, GUIDE_MODE_SWITCH_TIP_BASE_OPA, 0);
        lv_obj_align(ms->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SWITCH_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_MODE_SWITCH_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_prepare_from_tip(&ms->tip_shine, &style, predecoded);
    ui_shine_update(&ms->tip_shine, info->root, &style, ms->tip_img, 1);
    lv_obj_move_foreground(ms->tip_img);
    ui_shine_show(&ms->tip_shine);
}

static void guide_mode_switch_chrome_update(guide_base_ui_info_t *info)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;
    ui_shine_style_t style;
    uint8_t selected = ms->transitioning ? ms->slot_index : ms->shown_slot_index;

    if (selected >= GUIDE_MODE_SWITCH_SLOT_COUNT) {
        selected = 0;
    }
    if (selected != ms->selected_dot || info->force_update) {
        guide_mode_switch_dots_rebuild(info, selected);
    }
    if (info->force_update) {
        guide_mode_switch_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_MODE_SWITCH_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&ms->tip_shine, &style, ms->tip_img, 0);
}

static void guide_mode_switch_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning)
{
    guide_base_mode_switch_card_t *card;
    const bool small = transitioning && (hero_scale < GUIDE_MODE_SWITCH_SCALE_FULL);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    card = guide_mode_switch_card_from_page(page);
    if (card == NULL) {
        return;
    }

    if (card->circle_bg != NULL && lv_obj_is_valid(card->circle_bg)) {
        if (small) {
            lv_obj_clear_flag(card->circle_bg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_background(card->circle_bg);
        } else {
            lv_obj_add_flag(card->circle_bg, LV_OBJ_FLAG_HIDDEN);
        }
        guide_mode_switch_obj_scale_set(card->circle_bg, GUIDE_MODE_SWITCH_SCALE_FULL);
    }

    guide_mode_switch_obj_scale_set(card->icon, hero_scale);
    if (card->title != NULL && lv_obj_is_valid(card->title)) {
        const lv_font_t *font = small ?
            UI_FONT_TITLE_MD : UI_FONT_TITLE_LG;
        guide_mode_switch_obj_scale_set(card->title, GUIDE_MODE_SWITCH_SCALE_FULL);
        lv_obj_set_style_text_font(card->title, font, 0);
    }

    if (card->duration != NULL && lv_obj_is_valid(card->duration)) {
        if (!small && guide_mode_switch_has_duration(card->mode)) {
            lv_obj_clear_flag(card->duration, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(card->duration, LV_OBJ_FLAG_HIDDEN);
        }
    }
    guide_mode_switch_hero_layout(card);
}

static void guide_mode_switch_transition_lerp(guide_base_ui_info_t *info)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;
    const int32_t p = ms->progress;
    const int32_t slide_end = GUIDE_MODE_SWITCH_SLIDE_PROGRESS;
    const int32_t hero_scale_min = GUIDE_MODE_SWITCH_HERO_SCALE_MIN;
    const int32_t opa_min = GUIDE_MODE_SWITCH_OPA_MIN;
    const int32_t dist = GUIDE_MODE_SWITCH_CIRCLE_SIZE + GUIDE_MODE_SWITCH_CIRCLE_GAP;
    int32_t slide_p;
    int32_t scale_p;
    int32_t inv_slide;
    int16_t out_x;
    int16_t in_x;
    int32_t out_hero;
    int32_t in_hero;
    int32_t out_opa_i;
    int32_t in_opa_i;

    if (p < slide_end) {
        slide_p = (p * GUIDE_MODE_SWITCH_PROGRESS_FULL) / slide_end;
        scale_p = 0;
    } else if (p >= GUIDE_MODE_SWITCH_PROGRESS_FULL) {
        slide_p = GUIDE_MODE_SWITCH_PROGRESS_FULL;
        scale_p = GUIDE_MODE_SWITCH_PROGRESS_FULL;
    } else {
        slide_p = GUIDE_MODE_SWITCH_PROGRESS_FULL;
        scale_p = ((p - slide_end) * GUIDE_MODE_SWITCH_PROGRESS_FULL) /
                  (GUIDE_MODE_SWITCH_PROGRESS_FULL - slide_end);
    }
    inv_slide = GUIDE_MODE_SWITCH_PROGRESS_FULL - slide_p;

    if (ms->slide_dir == GUIDE_MODE_SWITCH_SLIDE_LEFT) {
        out_x = (int16_t)((-dist * slide_p) / GUIDE_MODE_SWITCH_PROGRESS_FULL);
        in_x = (int16_t)((dist * inv_slide) / GUIDE_MODE_SWITCH_PROGRESS_FULL);
    } else {
        out_x = (int16_t)((dist * slide_p) / GUIDE_MODE_SWITCH_PROGRESS_FULL);
        in_x = (int16_t)((-dist * inv_slide) / GUIDE_MODE_SWITCH_PROGRESS_FULL);
    }

    out_hero = hero_scale_min;
    in_hero = hero_scale_min +
        ((GUIDE_MODE_SWITCH_SCALE_FULL - hero_scale_min) * scale_p) / GUIDE_MODE_SWITCH_PROGRESS_FULL;
    out_opa_i = LV_OPA_COVER - ((LV_OPA_COVER - opa_min) * slide_p) / GUIDE_MODE_SWITCH_PROGRESS_FULL;
    in_opa_i = opa_min + ((LV_OPA_COVER - opa_min) * slide_p) / GUIDE_MODE_SWITCH_PROGRESS_FULL;

    guide_mode_switch_page_transform_apply(ms->page, out_x, GUIDE_MODE_SWITCH_SCALE_FULL,
        (lv_opa_t)out_opa_i);
    guide_mode_switch_page_transform_apply(ms->peer_page, in_x, GUIDE_MODE_SWITCH_SCALE_FULL,
        (lv_opa_t)in_opa_i);
    guide_mode_switch_transition_apply(ms->page, out_hero, true);
    guide_mode_switch_transition_apply(ms->peer_page, in_hero, true);
}

static void guide_mode_switch_transition_finish(guide_base_ui_info_t *info)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;
    const uint8_t new_slot = ms->slot_index;

    guide_mode_switch_card_content_remove(ms->page);
    LVGL_OBJ_DELETE(ms->page);
    ms->page = ms->peer_page;
    ms->peer_page = NULL;
    ms->shown_slot_index = new_slot;
    ms->transitioning = 0;
    ms->slide_dir = GUIDE_MODE_SWITCH_SLIDE_NONE;
    ms->progress = 0;
    guide_mode_switch_page_transform_reset(ms->page);
    guide_mode_switch_transition_apply(ms->page, GUIDE_MODE_SWITCH_SCALE_FULL, false);
}

static void guide_mode_switch_transition_tick(guide_base_ui_info_t *info)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;
    uint32_t elapsed;
    int32_t progress;

    if (!ms->transitioning) {
        return;
    }
    elapsed = get_timestamp() - ms->start_ms;
    if (elapsed >= GUIDE_MODE_SWITCH_TRANSITION_MS) {
        progress = GUIDE_MODE_SWITCH_PROGRESS_FULL;
    } else {
        progress = (int32_t)((elapsed * GUIDE_MODE_SWITCH_PROGRESS_FULL) /
            GUIDE_MODE_SWITCH_TRANSITION_MS);
    }
    ms->progress = (int16_t)progress;
    guide_mode_switch_transition_lerp(info);
    if (progress >= GUIDE_MODE_SWITCH_PROGRESS_FULL) {
        guide_mode_switch_transition_finish(info);
    }
}

static void guide_mode_switch_goto(guide_base_ui_info_t *info, uint8_t slot)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;
    uint8_t dir;

    EXCEPTION_CHECK_VOID(info);
    if (ms->transitioning) {
        return;
    }
    if (ms->page == NULL || !lv_obj_is_valid(ms->page)) {
        return;
    }
    if (slot == ms->shown_slot_index || slot >= GUIDE_MODE_SWITCH_SLOT_COUNT) {
        ms->slot_index = slot;
        return;
    }

    dir = (slot > ms->shown_slot_index) ?
        GUIDE_MODE_SWITCH_SLIDE_LEFT : GUIDE_MODE_SWITCH_SLIDE_RIGHT;

    ms->peer_page = guide_mode_switch_container_create(info);
    EXCEPTION_CHECK_VOID(ms->peer_page);
    guide_mode_switch_card_content_build(info, ms->peer_page, slot);

    ms->slot_index = slot;
    ms->slide_dir = dir;
    ms->transitioning = 1;
    ms->progress = 0;
    ms->start_ms = get_timestamp();
    guide_mode_switch_transition_lerp(info);
}

static void guide_mode_switch_switch_instant(guide_base_ui_info_t *info, uint8_t slot)
{
    guide_base_mode_switch_info_t *ms = &info->mode_switch;

    if (ms->page != NULL && lv_obj_is_valid(ms->page)) {
        guide_mode_switch_card_content_remove(ms->page);
        LVGL_OBJ_DELETE(ms->page);
        ms->page = NULL;
    }
    if (ms->peer_page != NULL && lv_obj_is_valid(ms->peer_page)) {
        guide_mode_switch_card_content_remove(ms->peer_page);
        LVGL_OBJ_DELETE(ms->peer_page);
        ms->peer_page = NULL;
    }

    if (slot >= GUIDE_MODE_SWITCH_SLOT_COUNT) {
        slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;
    }
    ms->slot_index = slot;
    ms->shown_slot_index = slot;
    ms->transitioning = 0;
    ms->slide_dir = GUIDE_MODE_SWITCH_SLIDE_NONE;
    ms->progress = 0;

    ms->page = guide_mode_switch_container_create(info);
    EXCEPTION_CHECK_VOID(ms->page);
    guide_mode_switch_card_content_build(info, ms->page, slot);
    guide_mode_switch_transition_apply(ms->page, GUIDE_MODE_SWITCH_SCALE_FULL, false);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_mode_switch_ui_remove(guide_base_ui_info_t *info)
{
    guide_base_mode_switch_info_t *ms;
    uint8_t i;

    EXCEPTION_CHECK_VOID(info);
    ms = &info->mode_switch;
    if (ms->page != NULL && lv_obj_is_valid(ms->page)) {
        guide_mode_switch_card_content_remove(ms->page);
        LVGL_OBJ_DELETE(ms->page);
    }
    if (ms->peer_page != NULL && lv_obj_is_valid(ms->peer_page)) {
        guide_mode_switch_card_content_remove(ms->peer_page);
        LVGL_OBJ_DELETE(ms->peer_page);
    }
    for (i = 0; i < GUIDE_MODE_SWITCH_SLOT_COUNT; i++) {
        LVGL_OBJ_DELETE(ms->dots[i]);
        ms->dots[i] = NULL;
    }
    ui_shine_remove(&ms->tip_shine);
    if (ms->tip_img != NULL && lv_obj_is_valid(ms->tip_img)) {
        lv_image_set_src(ms->tip_img, NULL);
        LVGL_OBJ_DELETE(ms->tip_img);
    }
    if (ms->tip_buf != NULL) {
        lv_draw_buf_destroy(ms->tip_buf);
        ms->tip_buf = NULL;
    }
    memset(ms, 0, sizeof(*ms));
    ms->shown_slot_index = GUIDE_MODE_SWITCH_SLOT_INVALID;
}

void guide_mode_switch_ui_build(guide_base_ui_info_t *info)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;

    EXCEPTION_CHECK_VOID(info && info->root);
    memset(&info->mode_switch, 0, sizeof(info->mode_switch));
    info->mode_switch.shown_slot_index = GUIDE_MODE_SWITCH_SLOT_INVALID;

    if (info->ui != NULL) {
        ui_proc_guide_t *proc = (ui_proc_guide_t *)info->ui->priv.play_info.user_data;
        if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
            slot = (uint8_t)proc->ui_mode_index;
        }
    }

    guide_mode_switch_tip_rebuild(info);
    guide_mode_switch_dots_rebuild(info, slot);
    guide_mode_switch_switch_instant(info, slot);
    info->mode_switch.ready = 1;
}

void guide_mode_switch_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    guide_base_mode_switch_info_t *ms;
    uint8_t target;

    EXCEPTION_CHECK_VOID(info && proc);
    ms = &info->mode_switch;
    if (!ms->ready) {
        guide_mode_switch_ui_build(info);
    }

    target = (uint8_t)proc->ui_mode_index;
    if (target >= GUIDE_MODE_SWITCH_SLOT_COUNT) {
        target = GUIDE_MODE_SWITCH_SLOT_STIMULATION;
    }

    if (ms->shown_slot_index == GUIDE_MODE_SWITCH_SLOT_INVALID) {
        guide_mode_switch_switch_instant(info, target);
    } else if (!ms->transitioning && ms->shown_slot_index != target) {
        guide_mode_switch_goto(info, target);
    }

    guide_mode_switch_transition_tick(info);
    if (ms->transitioning) {
        guide_mode_switch_transition_lerp(info);
    }
    guide_mode_switch_chrome_update(info);
}
