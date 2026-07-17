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
#define GUIDE_MODE_SELECT_TITLE_COLOR                   (0xBD4060)
#define GUIDE_MODE_SELECT_CAROUSEL_SIDE_COLOR           (0xE89AAB)
#define GUIDE_MODE_SELECT_TITLE_ACTIVE_Y                (82)
#define GUIDE_MODE_SELECT_TITLE_INACTIVE_Y              (84)
#define GUIDE_MODE_SELECT_TITLE_GAP                     (24)
#define GUIDE_MODE_SELECT_COUNTDOWN_Y                   (44) //(32)
#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
#define GUIDE_MODE_SELECT_COUNTDOWN_COLOR               (0x00FF00)
#else
#define GUIDE_MODE_SELECT_COUNTDOWN_COLOR               (0xBD4060)
#endif

#define GUIDE_MODE_SELECT_LABEL_X                       (44)
#define GUIDE_MODE_SELECT_STIMULATION_LABEL_Y           (124 - 8)
#define GUIDE_MODE_SELECT_STIMULATION_TEXT_Y            (156 - 8)
#define GUIDE_MODE_SELECT_EXPRESSION_LABEL_Y            (124 - 8)
#define GUIDE_MODE_SELECT_EXPRESSION_TEXT_Y             (156 - 8)
#define GUIDE_MODE_SELECT_MIXED_LABEL_Y                 (120 - 8)
#define GUIDE_MODE_SELECT_MIXED_TEXT_Y                  (158 - 8)

#define GUIDE_MODE_SELECT_SLOT_PREV                     (0)
#define GUIDE_MODE_SELECT_SLOT_CUR                      (1)
#define GUIDE_MODE_SELECT_SLOT_NEXT                     (2)

typedef struct {
    const char *name;
    uint16_t label_res_id;
    int16_t label_y;
    int16_t text_y;
    uint8_t line_space;
    const char *desc_text;
} guide_mode_select_content_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const guide_mode_select_content_cfg_t guide_mode_select_content_cfgs[] = {
    [BREAST_PUMP_STIMULATION_MODE] = {
        "Stimulation",
        CUSTOM_RES_ID_HOME_NARROW_LABLE,
        GUIDE_MODE_SELECT_STIMULATION_LABEL_Y,
        GUIDE_MODE_SELECT_STIMULATION_TEXT_Y,
        4,
        "Fast, gentle suction to\ntrigger let-down",
    },
    [BREAST_PUMP_LACTATION_MODE] = {
        "Expression",
        CUSTOM_RES_ID_HOME_NARROW_LABLE,
        GUIDE_MODE_SELECT_EXPRESSION_LABEL_Y,
        GUIDE_MODE_SELECT_EXPRESSION_TEXT_Y,
        4,
        "Slow, deep suction for\nefficient milk expression",
    },
    [BREAST_PUMP_MIX_MODE] = {
        "Mixed",
        CUSTOM_RES_ID_HOME_WIDTH_LABLE,
        GUIDE_MODE_SELECT_MIXED_LABEL_Y,
        GUIDE_MODE_SELECT_MIXED_TEXT_Y,
        4,
        "Efficiently promote\nlet-down and better\nemptying",
    },
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t guide_mode_select_normalize(uint8_t mode)
{
    // return BREAST_PUMP_LACTATION_MODE;
    if (!PUMP_IS_COMMON_MODE(mode)) {
        return BREAST_PUMP_STIMULATION_MODE;
    }
    return mode;
}

static uint8_t guide_mode_select_neighbor_next_get(uint8_t mode)
{
    mode = guide_mode_select_normalize(mode);
    return mode < BREAST_PUMP_NORMAL_MODE_UPPER ?
        (uint8_t)(mode + 1) : BREAST_PUMP_NORMAL_MODE_LOWER;
}

static uint8_t guide_mode_select_neighbor_prev_get(uint8_t mode)
{
    mode = guide_mode_select_normalize(mode);
    return mode > BREAST_PUMP_NORMAL_MODE_LOWER ?
        (uint8_t)(mode - 1) : BREAST_PUMP_NORMAL_MODE_UPPER;
}

static const guide_mode_select_content_cfg_t *guide_mode_select_content_get(uint8_t mode)
{
    mode = guide_mode_select_normalize(mode);
    return &guide_mode_select_content_cfgs[mode];
}

static void guide_mode_select_bg_create(guide_custom_ui_info_t *info)
{
    info->mode_select.bg = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->mode_select.bg);
    ui_img_png_theme_set_src(info->ui, info->mode_select.bg, CUSTOM_RES_ID_COMMON_BG,
        RES_THEME_TYPE_CUSTOM, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->mode_select.bg, LV_ALIGN_CENTER, 0, 0);
}

static void guide_mode_select_title_style_apply(lv_obj_t *label, bool active)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return;
    }

    lv_obj_set_style_text_color(label, lv_color_hex(active ?
        GUIDE_MODE_SELECT_TITLE_COLOR : GUIDE_MODE_SELECT_CAROUSEL_SIDE_COLOR), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    if (active) {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_26_bold, 0);
    } else {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_22_semibold, 0);
    }
}

static lv_coord_t guide_mode_select_title_half_width_get(lv_obj_t *label)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return 0;
    }

    lv_obj_update_layout(label);
    const lv_coord_t width = lv_obj_get_width(label);
    return width > 0 ? (lv_coord_t)(width / 2) : 0;
}

static void guide_mode_select_carousel_realign(guide_custom_ui_info_t *info)
{
    lv_obj_t *parent = guide_custom_page_parent_get(info);
    const lv_coord_t center_x = lv_obj_get_width(parent) / 2;
    lv_obj_t *labels[3] = {
        info->mode_select.select_titles[GUIDE_MODE_SELECT_SLOT_PREV],
        info->mode_select.select_titles[GUIDE_MODE_SELECT_SLOT_CUR],
        info->mode_select.select_titles[GUIDE_MODE_SELECT_SLOT_NEXT],
    };
    lv_coord_t half_w[3];
    lv_coord_t x_pos[3];

    for (uint8_t i = 0; i < 3; i++) {
        half_w[i] = guide_mode_select_title_half_width_get(labels[i]);
    }

    x_pos[GUIDE_MODE_SELECT_SLOT_CUR] = center_x;
    x_pos[GUIDE_MODE_SELECT_SLOT_PREV] = center_x - half_w[GUIDE_MODE_SELECT_SLOT_CUR] -
        GUIDE_MODE_SELECT_TITLE_GAP - half_w[GUIDE_MODE_SELECT_SLOT_PREV];
    x_pos[GUIDE_MODE_SELECT_SLOT_NEXT] = center_x + half_w[GUIDE_MODE_SELECT_SLOT_CUR] +
        GUIDE_MODE_SELECT_TITLE_GAP + half_w[GUIDE_MODE_SELECT_SLOT_NEXT];

    for (uint8_t i = 0; i < 3; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            continue;
        }
        const lv_coord_t title_y = (i == GUIDE_MODE_SELECT_SLOT_CUR) ?
            GUIDE_MODE_SELECT_TITLE_ACTIVE_Y : GUIDE_MODE_SELECT_TITLE_INACTIVE_Y;
        lv_obj_align(labels[i], LV_ALIGN_TOP_MID, x_pos[i] - center_x, title_y);
    }
}

static void guide_mode_select_carousel_ensure(guide_custom_ui_info_t *info)
{
    for (uint8_t i = 0; i < 3; i++) {
        if (info->mode_select.select_titles[i] != NULL &&
            lv_obj_is_valid(info->mode_select.select_titles[i])) {
            continue;
        }
        info->mode_select.select_titles[i] = lv_label_create(guide_custom_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->mode_select.select_titles[i]);
    }
}

static void guide_mode_select_carousel_update(guide_custom_ui_info_t *info, uint8_t center_mode)
{
    const uint8_t prev_mode = guide_mode_select_neighbor_prev_get(center_mode);
    const uint8_t next_mode = guide_mode_select_neighbor_next_get(center_mode);
    const uint8_t modes[3] = { prev_mode, center_mode, next_mode };

    guide_mode_select_carousel_ensure(info);
    for (uint8_t i = 0; i < 3; i++) {
        const guide_mode_select_content_cfg_t *cfg = guide_mode_select_content_get(modes[i]);
        lv_obj_t *label = info->mode_select.select_titles[i];
        if (label == NULL) {
            continue;
        }
        lv_label_set_text(label, cfg->name);
        guide_mode_select_title_style_apply(label, i == GUIDE_MODE_SELECT_SLOT_CUR);
    }
    guide_mode_select_carousel_realign(info);
}

static void guide_mode_select_label_card_update(guide_custom_ui_info_t *info, uint16_t res_id,
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
    } else if (info->mode_select.shown_label_res_id != res_id || info->force_update) {
        ui_img_baj_release_decoded_buf(info->label.label);
        ui_img_baj_theme_set_src(info->ui, info->label.label, res_id,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->label.label, LV_ALIGN_TOP_LEFT, GUIDE_MODE_SELECT_LABEL_X, y);
#endif
}

static void guide_mode_select_desc_text_update(guide_custom_ui_info_t *info, const char *content,
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

static void guide_mode_select_label_update(guide_custom_ui_info_t *info, uint8_t mode)
{
    const guide_mode_select_content_cfg_t *cfg = guide_mode_select_content_get(mode);

    guide_mode_select_label_card_update(info, cfg->label_res_id, cfg->label_y);
    guide_mode_select_desc_text_update(info, cfg->desc_text, cfg->text_y, cfg->line_space);
    info->mode_select.shown_label_res_id = cfg->label_res_id;
}

static void guide_custom_mode_select_idle_countdown_ui_create(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);

    info->mode_select.countdown_num = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->mode_select.countdown_num);
    lv_obj_set_style_text_color(info->mode_select.countdown_num,
        lv_color_hex(GUIDE_MODE_SELECT_COUNTDOWN_COLOR), 0);
    // lv_obj_set_style_text_font(info->mode_select.countdown_num, &lv_font_aeonik_soft_36_regular, 0);
    lv_obj_set_style_text_font(info->mode_select.countdown_num, &lv_font_aeonik_soft_32_medium, 0);
    lv_obj_set_style_text_align(info->mode_select.countdown_num, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->mode_select.countdown_num, "3s");
    lv_obj_align(info->mode_select.countdown_num, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SELECT_COUNTDOWN_Y);
    lv_obj_add_flag(info->mode_select.countdown_num, LV_OBJ_FLAG_HIDDEN);
    info->mode_select.shown_countdown_sec = 0xFF;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_mode_select_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_png_release_decoded_buf(info->mode_select.bg);
    info->mode_select.shown_label_res_id = 0;
    info->mode_select.shown_mode_index = 0xFF;
}

void guide_custom_mode_select_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->mode_select.shown_mode_index = 0xFF;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    if (!guide_custom_level_theme_is_active(info)) {
        guide_mode_select_bg_create(info);
    }
#endif
    guide_custom_mode_select_idle_countdown_ui_create(info);
    guide_mode_select_carousel_ensure(info);
}

void guide_custom_mode_select_ui_update(guide_custom_ui_info_t *info, ui_proc_guide_t *proc)
{
    uint8_t center_mode;

    EXCEPTION_CHECK_VOID(info && proc);
    center_mode = guide_mode_select_normalize(proc->ui_mode_index);
    if (center_mode == info->mode_select.shown_mode_index && info->force_update == false) {
        return;
    }

    guide_mode_select_carousel_update(info, center_mode);
    guide_mode_select_label_update(info, center_mode);
    info->mode_select.shown_mode_index = center_mode;
}

void guide_custom_mode_select_idle_countdown_ui_update(guide_custom_ui_info_t *info,
    ui_proc_guide_t *proc)
{
    uint8_t remain_sec;
    char num_buf[8];

    EXCEPTION_CHECK_VOID(info && proc);
    if (proc->page_index != GUIDE_PAGE_MODE_SELECT) {
        return;
    }
    if (info->mode_select.countdown_num == NULL || !lv_obj_is_valid(info->mode_select.countdown_num)) {
        return;
    }

    remain_sec = guide_mode_select_idle_remain_sec_get(proc);
    if (info->mode_select.shown_countdown_sec == remain_sec) {
        return;
    }

    guide_custom_page_num_ui_set_visible(info, remain_sec == 0);
    if (remain_sec == 0) {
        lv_obj_add_flag(info->mode_select.countdown_num, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_snprintf(num_buf, sizeof(num_buf), "%us", remain_sec);
        lv_label_set_text(info->mode_select.countdown_num, num_buf);
        lv_obj_align(info->mode_select.countdown_num, LV_ALIGN_TOP_MID, 0, GUIDE_MODE_SELECT_COUNTDOWN_Y);
        lv_obj_clear_flag(info->mode_select.countdown_num, LV_OBJ_FLAG_HIDDEN);
    }
    info->mode_select.shown_countdown_sec = remain_sec;
}
