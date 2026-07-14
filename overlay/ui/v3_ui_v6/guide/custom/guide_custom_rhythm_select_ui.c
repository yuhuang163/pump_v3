/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_RHYTHM_SELECT_TITLE_COLOR                 (0xBD4060)
#define GUIDE_RHYTHM_SELECT_CAROUSEL_SIDE_COLOR         (0xE89AAB)
#define GUIDE_RHYTHM_SELECT_TIME_COLOR                  (0xD36682)
#define GUIDE_RHYTHM_SELECT_TITLE_ACTIVE_Y                (82)
#define GUIDE_RHYTHM_SELECT_TITLE_INACTIVE_Y              (84)
#define GUIDE_RHYTHM_SELECT_TITLE_GAP                     (24)

#define GUIDE_RHYTHM_SELECT_LABEL_X                       (44)

#define GUIDE_RHYTHM_SELECT_MILK_BOOST_LABEL_Y            (134)
#define GUIDE_RHYTHM_SELECT_MILK_BOOST_TEXT_Y             (172)
#define GUIDE_RHYTHM_SELECT_MILK_BOOST_TIME_Y             (124)
#define GUIDE_RHYTHM_SELECT_POWER_PUMPING_LABEL_Y         (146)
#define GUIDE_RHYTHM_SELECT_POWER_PUMPING_TEXT_Y          (180)
#define GUIDE_RHYTHM_SELECT_POWER_PUMPING_TIME_Y          (124)

#define GUIDE_RHYTHM_SELECT_DESC_LINE_SPACE               (4)

#define GUIDE_RHYTHM_SELECT_SLOT_PREV                     (0)
#define GUIDE_RHYTHM_SELECT_SLOT_CUR                      (1)
#define GUIDE_RHYTHM_SELECT_SLOT_NEXT                     (2)

typedef struct {
    const char *name;
    uint16_t label_res_id;
    int16_t label_y;
    int16_t text_y;
    int16_t time_y;
    uint8_t line_space;
    const char *desc_text;
    const char *time_text;
} guide_rhythm_select_content_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const guide_rhythm_select_content_cfg_t guide_rhythm_select_content_cfgs[] = {
    [BREAST_PUMP_MILK_BOOST_MODE] = {
        "Milk Boost",
        CUSTOM_RES_ID_HOME_WIDTH_LABLE,
        GUIDE_RHYTHM_SELECT_MILK_BOOST_LABEL_Y,
        GUIDE_RHYTHM_SELECT_MILK_BOOST_TEXT_Y,
        GUIDE_RHYTHM_SELECT_MILK_BOOST_TIME_Y,
        GUIDE_RHYTHM_SELECT_DESC_LINE_SPACE,
        "Suitable for daily use\nafter successful\nlactation",
        "≈20 min",
    },
    [BREAST_PUMP_PP_MILK_TRACK_MODE] = {
        "Power Pumping",
        CUSTOM_RES_ID_HOME_NARROW_LABLE,
        GUIDE_RHYTHM_SELECT_POWER_PUMPING_LABEL_Y,
        GUIDE_RHYTHM_SELECT_POWER_PUMPING_TEXT_Y,
        GUIDE_RHYTHM_SELECT_POWER_PUMPING_TIME_Y,
        GUIDE_RHYTHM_SELECT_DESC_LINE_SPACE,
        "Increase milk supply,no\nmore than once a day",
        "≈45 min",
    },
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t guide_rhythm_select_normalize(uint8_t mode)
{
    // return BREAST_PUMP_PP_MILK_TRACK_MODE;
    if (mode == BREAST_PUMP_PP_MILK_TRACK_MODE) {
        return BREAST_PUMP_PP_MILK_TRACK_MODE;
    }
    return BREAST_PUMP_MILK_BOOST_MODE;
}

static uint8_t guide_rhythm_select_neighbor_next_get(uint8_t mode)
{
    mode = guide_rhythm_select_normalize(mode);
    return mode == BREAST_PUMP_MILK_BOOST_MODE ?
        BREAST_PUMP_PP_MILK_TRACK_MODE : BREAST_PUMP_MILK_BOOST_MODE;
}

static uint8_t guide_rhythm_select_neighbor_prev_get(uint8_t mode)
{
    mode = guide_rhythm_select_normalize(mode);
    return mode == BREAST_PUMP_MILK_BOOST_MODE ?
        BREAST_PUMP_PP_MILK_TRACK_MODE : BREAST_PUMP_MILK_BOOST_MODE;
}

static const guide_rhythm_select_content_cfg_t *guide_rhythm_select_content_get(uint8_t mode)
{
    mode = guide_rhythm_select_normalize(mode);
    return &guide_rhythm_select_content_cfgs[mode];
}

static void guide_rhythm_select_bg_create(guide_custom_ui_info_t *info)
{
    info->rhythm_select.bg = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->rhythm_select.bg);
    ui_img_png_theme_set_src(info->ui, info->rhythm_select.bg, CUSTOM_RES_ID_COMMON_BG,
        RES_THEME_TYPE_CUSTOM, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->rhythm_select.bg, LV_ALIGN_CENTER, 0, 0);
}

static void guide_rhythm_select_title_style_apply(lv_obj_t *label, bool active)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return;
    }

    lv_obj_set_style_text_color(label, lv_color_hex(active ?
        GUIDE_RHYTHM_SELECT_TITLE_COLOR : GUIDE_RHYTHM_SELECT_CAROUSEL_SIDE_COLOR), 0);
    if (active) {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_26_bold, 0);
    } else {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_22_semibold, 0);
    }
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
}

static lv_coord_t guide_rhythm_select_title_half_width_get(lv_obj_t *label)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return 0;
    }

    lv_obj_update_layout(label);
    const lv_coord_t width = lv_obj_get_width(label);
    return width > 0 ? (lv_coord_t)(width / 2) : 0;
}

static void guide_rhythm_select_carousel_realign(guide_custom_ui_info_t *info)
{
    lv_obj_t *parent = guide_custom_page_parent_get(info);
    const lv_coord_t center_x = lv_obj_get_width(parent) / 2;
    lv_obj_t *labels[3] = {
        info->rhythm_select.select_titles[GUIDE_RHYTHM_SELECT_SLOT_PREV],
        info->rhythm_select.select_titles[GUIDE_RHYTHM_SELECT_SLOT_CUR],
        info->rhythm_select.select_titles[GUIDE_RHYTHM_SELECT_SLOT_NEXT],
    };
    lv_coord_t half_w[3];
    lv_coord_t x_pos[3];

    for (uint8_t i = 0; i < 3; i++) {
        half_w[i] = guide_rhythm_select_title_half_width_get(labels[i]);
    }

    x_pos[GUIDE_RHYTHM_SELECT_SLOT_CUR] = center_x;
    x_pos[GUIDE_RHYTHM_SELECT_SLOT_PREV] = center_x - half_w[GUIDE_RHYTHM_SELECT_SLOT_CUR] -
        GUIDE_RHYTHM_SELECT_TITLE_GAP - half_w[GUIDE_RHYTHM_SELECT_SLOT_PREV];
    x_pos[GUIDE_RHYTHM_SELECT_SLOT_NEXT] = center_x + half_w[GUIDE_RHYTHM_SELECT_SLOT_CUR] +
        GUIDE_RHYTHM_SELECT_TITLE_GAP + half_w[GUIDE_RHYTHM_SELECT_SLOT_NEXT];

    for (uint8_t i = 0; i < 3; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            continue;
        }
        const lv_coord_t title_y = (i == GUIDE_RHYTHM_SELECT_SLOT_CUR) ?
            GUIDE_RHYTHM_SELECT_TITLE_ACTIVE_Y : GUIDE_RHYTHM_SELECT_TITLE_INACTIVE_Y;
        lv_obj_align(labels[i], LV_ALIGN_TOP_MID, x_pos[i] - center_x, title_y);
    }
}

static void guide_rhythm_select_carousel_ensure(guide_custom_ui_info_t *info)
{
    for (uint8_t i = 0; i < 3; i++) {
        if (info->rhythm_select.select_titles[i] != NULL &&
            lv_obj_is_valid(info->rhythm_select.select_titles[i])) {
            continue;
        }
        info->rhythm_select.select_titles[i] = lv_label_create(guide_custom_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->rhythm_select.select_titles[i]);
    }
}

static void guide_rhythm_select_carousel_update(guide_custom_ui_info_t *info, uint8_t center_mode)
{
    const uint8_t prev_mode = guide_rhythm_select_neighbor_prev_get(center_mode);
    const uint8_t next_mode = guide_rhythm_select_neighbor_next_get(center_mode);
    const uint8_t modes[3] = { prev_mode, center_mode, next_mode };

    guide_rhythm_select_carousel_ensure(info);
    for (uint8_t i = 0; i < 3; i++) {
        const guide_rhythm_select_content_cfg_t *cfg = guide_rhythm_select_content_get(modes[i]);
        lv_obj_t *label = info->rhythm_select.select_titles[i];
        if (label == NULL) {
            continue;
        }
        lv_label_set_text(label, cfg->name);
        guide_rhythm_select_title_style_apply(label, i == GUIDE_RHYTHM_SELECT_SLOT_CUR);
    }
    guide_rhythm_select_carousel_realign(info);
}

static void guide_rhythm_select_label_card_update(guide_custom_ui_info_t *info, uint16_t res_id,
    int16_t y)
{
#if IS_SUPPORT_CUSTOM_GUIDE_LABEL_TEXT_CMP
    (void)info;
    (void)res_id;
    (void)y;
    return;
#else
    if (info->label.label == NULL || !lv_obj_is_valid(info->label.label)) {
        info->label.label = lv_image_create(guide_custom_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->label.label);
        ui_img_baj_theme_set_src(info->ui, info->label.label, res_id,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    } else if (info->rhythm_select.shown_label_res_id != res_id || info->force_update) {
        ui_img_baj_release_decoded_buf(info->label.label);
        ui_img_baj_theme_set_src(info->ui, info->label.label, res_id,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->label.label, LV_ALIGN_TOP_LEFT, GUIDE_RHYTHM_SELECT_LABEL_X, y);
#endif
}

static void guide_rhythm_select_desc_text_update(guide_custom_ui_info_t *info, const char *content,
    int16_t y, uint8_t line_space)
{
    if (info->label.text == NULL || !lv_obj_is_valid(info->label.text)) {
        info->label.text = lv_label_create(guide_custom_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->label.text);
        lv_obj_set_style_text_color(info->label.text, lv_color_hex(GUIDE_CUSTOM_LABEL_CARD_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(info->label.text, &lv_font_aeonik_soft_18_regular, 0);
        lv_obj_set_style_text_align(info->label.text, LV_TEXT_ALIGN_CENTER, 0);
    }

    lv_label_set_text(info->label.text, content);
    lv_obj_set_style_text_line_space(info->label.text, line_space, 0);
    lv_obj_align(info->label.text, LV_ALIGN_TOP_MID, 0, y);
}

static void guide_rhythm_select_time_update(guide_custom_ui_info_t *info, const char *time_text,
    int16_t y)
{
    if (info->rhythm_select.time == NULL || !lv_obj_is_valid(info->rhythm_select.time)) {
        info->rhythm_select.time = lv_label_create(guide_custom_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->rhythm_select.time);
        lv_obj_set_style_text_color(info->rhythm_select.time,
            lv_color_hex(GUIDE_RHYTHM_SELECT_TIME_COLOR), 0);
        lv_obj_set_style_text_font(info->rhythm_select.time, &lv_font_aeonik_soft_20_medium, 0);
        lv_obj_set_style_text_align(info->rhythm_select.time, LV_TEXT_ALIGN_CENTER, 0);
    }

    const bool time_changed = info->force_update ||
        strncmp(info->rhythm_select.shown_time_text, time_text,
            sizeof(info->rhythm_select.shown_time_text)) != 0;

    if (time_changed) {
        lv_label_set_text(info->rhythm_select.time, time_text);
        strncpy(info->rhythm_select.shown_time_text, time_text,
            sizeof(info->rhythm_select.shown_time_text) - 1);
        info->rhythm_select.shown_time_text[sizeof(info->rhythm_select.shown_time_text) - 1] = '\0';
    }
    lv_obj_align(info->rhythm_select.time, LV_ALIGN_TOP_MID, 0, y);
}

static void guide_rhythm_select_label_update(guide_custom_ui_info_t *info, uint8_t mode)
{
    const guide_rhythm_select_content_cfg_t *cfg = guide_rhythm_select_content_get(mode);

    guide_rhythm_select_label_card_update(info, cfg->label_res_id, cfg->label_y);
    guide_rhythm_select_desc_text_update(info, cfg->desc_text, cfg->text_y, cfg->line_space);
    info->rhythm_select.shown_label_res_id = cfg->label_res_id;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_rhythm_select_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_png_release_decoded_buf(info->rhythm_select.bg);
    info->rhythm_select.shown_label_res_id = 0;
    info->rhythm_select.shown_mode_index = 0xFF;
    info->rhythm_select.shown_time_text[0] = '\0';
}

void guide_custom_rhythm_select_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->rhythm_select.shown_mode_index = 0xFF;
    info->rhythm_select.shown_time_text[0] = '\0';
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    if (!guide_custom_level_theme_is_active(info)) {
        guide_rhythm_select_bg_create(info);
    }
#endif
    guide_rhythm_select_carousel_ensure(info);
}

void guide_custom_rhythm_select_ui_update(guide_custom_ui_info_t *info, ui_proc_guide_t *proc)
{
    uint8_t center_mode;
    const guide_rhythm_select_content_cfg_t *cfg;

    EXCEPTION_CHECK_VOID(info && proc);
    center_mode = guide_rhythm_select_normalize(proc->ui_mode_index);
    if (center_mode == info->rhythm_select.shown_mode_index && info->force_update == false) {
        return;
    }

    cfg = guide_rhythm_select_content_get(center_mode);
    guide_rhythm_select_carousel_update(info, center_mode);
    guide_rhythm_select_time_update(info, cfg->time_text, cfg->time_y);
    guide_rhythm_select_label_update(info, center_mode);
    info->rhythm_select.shown_mode_index = center_mode;
}
