/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_LEVEL_TITLE_Y                         (86)
#define GUIDE_MODE_SELECT_TITLE_Y                   (88)
#define GUIDE_LEVEL_COUNTDOWN_Y                     (44)
#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
#define GUIDE_LEVEL_TITLE_COLOR                     (0x00FF00)
#define GUIDE_LEVEL_COUNTDOWN_COLOR                 (0x00FF00)
#else
#define GUIDE_LEVEL_TITLE_COLOR                     (0xBD4060)
#define GUIDE_LEVEL_COUNTDOWN_COLOR                 (0xBD4060)
#endif

/// 双边力度引导
#define GUIDE_LEVEL_BOTH_LABEL_X                    (43)
#define GUIDE_LEVEL_BOTH_LABEL_Y                    (114)
#define GUIDE_LEVEL_BOTH_GUIDE_HEADLINE_X           (0)
#define GUIDE_LEVEL_BOTH_GUIDE_HEADLINE_Y           (150)
#define GUIDE_LEVEL_BOTH_GUIDE_BODY_X               (0)
#define GUIDE_LEVEL_BOTH_GUIDE_BODY_Y               (174)

/// 双边力度调节（加档 / 减档共用 label）
#define GUIDE_LEVEL_BOTH_ADJUST_LABEL_X             (43)
#define GUIDE_LEVEL_BOTH_ADJUST_LABEL_Y             (133)

/// 双边力度调节-加档
#define GUIDE_LEVEL_BOTH_INCREASE_HEADLINE_X           (0)
#define GUIDE_LEVEL_BOTH_INCREASE_HEADLINE_Y           (168)
#define GUIDE_LEVEL_BOTH_INCREASE_BODY_X               (137)
#define GUIDE_LEVEL_BOTH_INCREASE_BODY_Y               (190)
#define GUIDE_LEVEL_BOTH_INCREASE_KNOB_ICON_X          (108)
#define GUIDE_LEVEL_BOTH_INCREASE_KNOB_ICON_Y          (186)

/// 双边力度调节-减档
#define GUIDE_LEVEL_BOTH_DECREASE_HEADLINE_X            (0)
#define GUIDE_LEVEL_BOTH_DECREASE_HEADLINE_Y            (168)
#define GUIDE_LEVEL_BOTH_DECREASE_BODY_X                (145)
#define GUIDE_LEVEL_BOTH_DECREASE_BODY_Y                (190)
#define GUIDE_LEVEL_BOTH_DECREASE_KNOB_ICON_X           (116)
#define GUIDE_LEVEL_BOTH_DECREASE_KNOB_ICON_Y           (186)

/// 单边力度引导
#define GUIDE_LEVEL_GUIDE_LABEL_X                   (43)
#define GUIDE_LEVEL_GUIDE_LABEL_Y                   (117)
#define GUIDE_LEVEL_GUIDE_HEADLINE_X                (141)
#define GUIDE_LEVEL_GUIDE_HEADLINE_Y                (152)
#define GUIDE_LEVEL_GUIDE_BODY_X                    (0)
#define GUIDE_LEVEL_GUIDE_BODY_Y                    (178)

#define GUIDE_LEVEL_ADJUST_HEADLINE_X               (0)
#define GUIDE_LEVEL_ADJUST_HEADLINE_Y               (152)
#define GUIDE_LEVEL_ADJUST_BODY_X                   (0)
#define GUIDE_LEVEL_ADJUST_BODY_Y                   (176)

#define GUIDE_LEVEL_SIDE_BUTTON_ICON_X              (195)
#define GUIDE_LEVEL_SIDE_BUTTON_ICON_Y              (148)

/// 模式选择引导
#define GUIDE_MODE_GUIDE_LABEL_X                    (38)
#define GUIDE_MODE_GUIDE_LABEL_Y                    (117)
#define GUIDE_MODE_GUIDE_HEADLINE_X                 (141)
#define GUIDE_MODE_GUIDE_HEADLINE_Y                 (152)
#define GUIDE_MODE_GUIDE_BODY_X                     (0)
#define GUIDE_MODE_GUIDE_BODY_Y                     (178)

#define GUIDE_MODE_SELECT_START_ICON_X              (195)
#define GUIDE_MODE_SELECT_START_ICON_Y              (148)

/// 泵停止引导
#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
#define GUIDE_PUMP_STOP_GUIDE_TITLE_COLOR           (0x00FF00)
#define GUIDE_PUMP_STOP_GUIDE_HINT_COLOR            (0x00FF00)
#else
#define GUIDE_PUMP_STOP_GUIDE_TITLE_COLOR           (0xBD4060)
#define GUIDE_PUMP_STOP_GUIDE_HINT_COLOR            (0xA0183B)
#endif
#define GUIDE_PUMP_STOP_GUIDE_TITLE_Y               (96)
#define GUIDE_PUMP_STOP_GUIDE_LABEL_X               (38)
#define GUIDE_PUMP_STOP_GUIDE_LABEL_Y               (125)
#define GUIDE_PUMP_STOP_GUIDE_PRESS_X               (101)
#define GUIDE_PUMP_STOP_GUIDE_PRESS_Y               (172)
#define GUIDE_PUMP_STOP_GUIDE_HINT_X                (152)
#define GUIDE_PUMP_STOP_GUIDE_HINT_Y                (172)
#define GUIDE_PUMP_STOP_GUIDE_ICON_X                (236)
#define GUIDE_PUMP_STOP_GUIDE_ICON_Y                (168)

#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
#define GUIDE_PAGE_NUM_CURRENT_COLOR                (0x00FF00)
#define GUIDE_PAGE_NUM_SUFFIX_COLOR                 (0x00FF00)
#else
#define GUIDE_PAGE_NUM_CURRENT_COLOR                (0xBD4060)
#define GUIDE_PAGE_NUM_SUFFIX_COLOR                 (0xDE7F93)
#endif

typedef struct {
    const char *headline;
    const char *body;
    uint16_t label_res_id;
    int16_t label_x;
    int16_t label_y;
    int16_t headline_x;
    int16_t headline_y;
    int16_t body_x;
    int16_t body_y;
    uint8_t line_space;
    uint16_t button_icon_res_id;
    int16_t icon_x;
    int16_t icon_y;
} guide_level_guide_page_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const guide_level_guide_page_cfg_t guide_level_guide_page_cfgs[GUIDE_PAGE_MAX] = {
    [GUIDE_PAGE_LEVEL_BOTH_GUIDE] = {
        "Rotate the knob", "Adjust suction levels\nfor both sides",
        CUSTOM_RES_ID_HOME_WIDTH_LABLE,
        GUIDE_LEVEL_BOTH_LABEL_X, GUIDE_LEVEL_BOTH_LABEL_Y,
        GUIDE_LEVEL_BOTH_GUIDE_HEADLINE_X, GUIDE_LEVEL_BOTH_GUIDE_HEADLINE_Y,
        GUIDE_LEVEL_BOTH_GUIDE_BODY_X, GUIDE_LEVEL_BOTH_GUIDE_BODY_Y,
        4, 0, 0, 0,
    },
    [GUIDE_PAGE_LEVEL_BOTH_INCREASE] = {
        "Rotate the knob", "right to level 5",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_BOTH_ADJUST_LABEL_X, GUIDE_LEVEL_BOTH_ADJUST_LABEL_Y,
        GUIDE_LEVEL_BOTH_INCREASE_HEADLINE_X, GUIDE_LEVEL_BOTH_INCREASE_HEADLINE_Y,
        GUIDE_LEVEL_BOTH_INCREASE_BODY_X, GUIDE_LEVEL_BOTH_INCREASE_BODY_Y,
        4, CUSTOM_RES_ID_RIGHT_KNOB_ICON,
        GUIDE_LEVEL_BOTH_INCREASE_KNOB_ICON_X, GUIDE_LEVEL_BOTH_INCREASE_KNOB_ICON_Y,
    },
    [GUIDE_PAGE_LEVEL_BOTH_DECREASE] = {
        "Rotate the knob", "left to level 1",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_BOTH_ADJUST_LABEL_X, GUIDE_LEVEL_BOTH_ADJUST_LABEL_Y,
        GUIDE_LEVEL_BOTH_DECREASE_HEADLINE_X, GUIDE_LEVEL_BOTH_DECREASE_HEADLINE_Y,
        GUIDE_LEVEL_BOTH_DECREASE_BODY_X, GUIDE_LEVEL_BOTH_DECREASE_BODY_Y,
        4, CUSTOM_RES_ID_LEFT_KNOB_ICON,
        GUIDE_LEVEL_BOTH_DECREASE_KNOB_ICON_X, GUIDE_LEVEL_BOTH_DECREASE_KNOB_ICON_Y,
    },
    [GUIDE_PAGE_LEVEL_LEFT_GUIDE] = {
        "Press        ", "Enter left adjustment",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_GUIDE_LABEL_X, GUIDE_LEVEL_GUIDE_LABEL_Y,
        GUIDE_LEVEL_GUIDE_HEADLINE_X, GUIDE_LEVEL_GUIDE_HEADLINE_Y,
        GUIDE_LEVEL_GUIDE_BODY_X, GUIDE_LEVEL_GUIDE_BODY_Y,
        6, CUSTOM_RES_ID_GUIDE_LEFT_BUTTON_ICON,
        GUIDE_LEVEL_SIDE_BUTTON_ICON_X, GUIDE_LEVEL_SIDE_BUTTON_ICON_Y,
    },
    [GUIDE_PAGE_LEVEL_LEFT] = {
        "Rotate the knob", "Adjust left level",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_GUIDE_LABEL_X, GUIDE_LEVEL_GUIDE_LABEL_Y,
        GUIDE_LEVEL_ADJUST_HEADLINE_X, GUIDE_LEVEL_ADJUST_HEADLINE_Y,
        GUIDE_LEVEL_ADJUST_BODY_X, GUIDE_LEVEL_ADJUST_BODY_Y,
        4, 0, 0, 0,
    },
    [GUIDE_PAGE_LEVEL_RIGHT_GUIDE] = {
        "Press        ", "Enter right adjustment ",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_GUIDE_LABEL_X, GUIDE_LEVEL_GUIDE_LABEL_Y,
        GUIDE_LEVEL_GUIDE_HEADLINE_X, GUIDE_LEVEL_GUIDE_HEADLINE_Y,
        GUIDE_LEVEL_GUIDE_BODY_X, GUIDE_LEVEL_GUIDE_BODY_Y,
        6, CUSTOM_RES_ID_GUIDE_RIGHT_BUTTON_ICON,
        GUIDE_LEVEL_SIDE_BUTTON_ICON_X, GUIDE_LEVEL_SIDE_BUTTON_ICON_Y,
    },
    [GUIDE_PAGE_LEVEL_RIGHT] = {
        "Rotate the knob", "Adjust right level",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_GUIDE_LABEL_X, GUIDE_LEVEL_GUIDE_LABEL_Y,
        GUIDE_LEVEL_ADJUST_HEADLINE_X, GUIDE_LEVEL_ADJUST_HEADLINE_Y,
        GUIDE_LEVEL_ADJUST_BODY_X, GUIDE_LEVEL_ADJUST_BODY_Y,
        4, 0, 0, 0,
    },
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static const guide_level_guide_page_cfg_t *guide_level_guide_page_cfg_get(uint8_t page_index)
{
    if (page_index >= GUIDE_PAGE_MAX) {
        return NULL;
    }
    return &guide_level_guide_page_cfgs[page_index];
}

static void guide_level_guide_bg_create(guide_custom_ui_info_t *info)
{
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    info->level_guide.bg = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.bg);
    lv_image_set_src(info->level_guide.bg, background_snapshot);
    lv_obj_align(info->level_guide.bg, LV_ALIGN_CENTER, 0, 0);
#else
    info->level_guide.bg = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.bg);
    ui_img_png_theme_set_src(info->ui, info->level_guide.bg, CUSTOM_RES_ID_COMMON_BG,
        RES_THEME_TYPE_CUSTOM, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->level_guide.bg, LV_ALIGN_CENTER, 0, 0);
#endif
}

static void guide_custom_label_step_header_create(guide_custom_ui_info_t *info, lv_obj_t **title,
    const char *title_text, int16_t title_y)
{
    EXCEPTION_CHECK_VOID(info && title && title_text);

    *title = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*title);
    lv_obj_set_style_text_color(*title, lv_color_hex(GUIDE_LEVEL_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(*title, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(*title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(*title, title_text);
    lv_obj_align(*title, LV_ALIGN_TOP_MID, 0, title_y);
}

static void guide_custom_label_top_align(lv_obj_t *obj, int16_t x, int16_t y)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    if (x == 0) {
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, y);
    } else {
        lv_obj_align(obj, LV_ALIGN_TOP_LEFT, x, y);
    }
}

static void guide_custom_label_instruction_card_update(guide_custom_ui_info_t *info, lv_obj_t **card,
    uint16_t res_id, int16_t x, int16_t y)
{
    EXCEPTION_CHECK_VOID(info && card);

#if IS_SUPPORT_CUSTOM_GUIDE_LABEL_TEXT_CMP
    *card = NULL;
    (void)res_id;
    (void)x;
    (void)y;
    return;
#else
    if (*card == NULL || !lv_obj_is_valid(*card)) {
        guide_custom_label_instruction_card_create(info, card, res_id, x, y);
        return;
    }
    ui_img_baj_release_decoded_buf(*card);
    ui_img_baj_theme_set_src(info->ui, *card, res_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(*card, LV_ALIGN_TOP_LEFT, x, y);
#endif
}

static void guide_custom_label_inline_icon_update(guide_custom_ui_info_t *info, lv_obj_t **icon,
    uint16_t res_id, int16_t x, int16_t y)
{
    EXCEPTION_CHECK_VOID(info && icon);

    if (*icon == NULL || !lv_obj_is_valid(*icon)) {
        guide_custom_label_inline_icon_create(info, icon, res_id, x, y);
        return;
    }
    ui_img_baj_release_decoded_buf(*icon);
    ui_img_baj_theme_set_src(info->ui, *icon, res_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(*icon, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_clear_flag(*icon, LV_OBJ_FLAG_HIDDEN);
}

static void guide_custom_label_instruction_headline_body_create(guide_custom_ui_info_t *info,
    lv_obj_t **headline, lv_obj_t **body, const char *headline_text, const char *body_text,
    int16_t headline_x, int16_t headline_y, int16_t body_x, int16_t body_y, uint8_t line_space)
{
    EXCEPTION_CHECK_VOID(info && headline && body && headline_text && body_text);

    *headline = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*headline);
    lv_obj_set_style_text_color(*headline, lv_color_hex(GUIDE_CUSTOM_LABEL_CARD_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(*headline, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(*headline, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(*headline, headline_text);
    if (headline_x == 0) {
        lv_obj_align(*headline, LV_ALIGN_TOP_MID, 0, headline_y);
    } else {
        lv_obj_align(*headline, LV_ALIGN_TOP_LEFT, headline_x, headline_y);
    }

    *body = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*body);
    lv_obj_set_style_text_color(*body, lv_color_hex(GUIDE_CUSTOM_LABEL_CARD_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(*body, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(*body, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(*body, body_text);
    lv_obj_set_style_text_line_space(*body, line_space, 0);
    if (body_x == 0) {
        lv_obj_align(*body, LV_ALIGN_TOP_MID, 0, body_y);
    } else {
        lv_obj_align(*body, LV_ALIGN_TOP_LEFT, body_x, body_y);
    }
}

static void guide_custom_level_idle_countdown_ui_create(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);

    info->level_guide.countdown_num = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.countdown_num);
    lv_obj_set_style_text_color(info->level_guide.countdown_num,
        lv_color_hex(GUIDE_LEVEL_COUNTDOWN_COLOR), 0);
    // lv_obj_set_style_text_font(info->level_guide.countdown_num, &lv_font_aeonik_soft_36_regular, 0);
    lv_obj_set_style_text_font(info->level_guide.countdown_num, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(info->level_guide.countdown_num, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->level_guide.countdown_num, "3s");
    lv_obj_align(info->level_guide.countdown_num, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_COUNTDOWN_Y);
    lv_obj_add_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
    info->level_guide.shown_countdown_sec = 0xFF;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

void guide_custom_label_level_header_create(guide_custom_ui_info_t *info, lv_obj_t **title)
{
    guide_custom_label_step_header_create(info, title, "Adjust to comfortable", GUIDE_LEVEL_TITLE_Y);
}

void guide_custom_label_instruction_card_create(guide_custom_ui_info_t *info, lv_obj_t **card,
    uint16_t res_id, int16_t x, int16_t y)
{
    EXCEPTION_CHECK_VOID(info && card);

#if IS_SUPPORT_CUSTOM_GUIDE_LABEL_TEXT_CMP
    *card = NULL;
    (void)res_id;
    (void)x;
    (void)y;
    return;
#else
    *card = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*card);
    ui_img_baj_theme_set_src(info->ui, *card, res_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(*card, LV_ALIGN_TOP_LEFT, x, y);
#endif
}

void guide_custom_label_instruction_text_create(guide_custom_ui_info_t *info, lv_obj_t **text,
    const char *content, int16_t y, uint8_t line_space)
{
    EXCEPTION_CHECK_VOID(info && text && content);

    *text = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*text);
    lv_obj_set_style_text_color(*text, lv_color_hex(GUIDE_CUSTOM_LABEL_CARD_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(*text, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(*text, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(*text, content);
    lv_obj_set_style_text_line_space(*text, line_space, 0);
    lv_obj_align(*text, LV_ALIGN_TOP_MID, 0, y);
}

void guide_custom_label_inline_icon_create(guide_custom_ui_info_t *info, lv_obj_t **icon,
    uint16_t res_id, int16_t x, int16_t y)
{
    EXCEPTION_CHECK_VOID(info && icon);

    *icon = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*icon);
    ui_img_baj_theme_set_src(info->ui, *icon, res_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(*icon, LV_ALIGN_TOP_LEFT, x, y);
}

void guide_custom_level_guide_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
#if !LV_USE_SNAPSHOT
    ui_img_png_release_decoded_buf(info->level_guide.bg);
#endif
    ui_img_baj_release_decoded_buf(info->level_guide.card);
    ui_img_baj_release_decoded_buf(info->level_guide.button_icon);
}

bool guide_custom_level_guide_same_shell(uint8_t old_page, uint8_t new_page)
{
    if (old_page == new_page) {
        return true;
    }
    if (old_page >= GUIDE_PAGE_LEVEL_BOTH_GUIDE && old_page <= GUIDE_PAGE_LEVEL_BOTH_DECREASE &&
        new_page >= GUIDE_PAGE_LEVEL_BOTH_GUIDE && new_page <= GUIDE_PAGE_LEVEL_BOTH_DECREASE) {
        return true;
    }
    if (old_page >= GUIDE_PAGE_LEVEL_LEFT_GUIDE && old_page <= GUIDE_PAGE_LEVEL_LEFT &&
        new_page >= GUIDE_PAGE_LEVEL_LEFT_GUIDE && new_page <= GUIDE_PAGE_LEVEL_LEFT) {
        return true;
    }
    if (old_page >= GUIDE_PAGE_LEVEL_RIGHT_GUIDE && old_page <= GUIDE_PAGE_LEVEL_RIGHT &&
        new_page >= GUIDE_PAGE_LEVEL_RIGHT_GUIDE && new_page <= GUIDE_PAGE_LEVEL_RIGHT) {
        return true;
    }
    return false;
}

void guide_custom_level_guide_ui_build(guide_custom_ui_info_t *info, uint8_t page_index)
{
    const guide_level_guide_page_cfg_t *cfg = guide_level_guide_page_cfg_get(page_index);

    EXCEPTION_CHECK_VOID(info && info->ui && cfg);

    info->level_guide.page_index = page_index;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    if (!guide_custom_level_theme_is_active(info)) {
        guide_level_guide_bg_create(info);
    }
#endif
    if (GUIDE_PAGE_SHOW_LEVEL_COUNTDOWN(page_index)) {
        guide_custom_label_step_header_create(info, &info->level_guide.title,
            "Adjust to comfortable", GUIDE_LEVEL_TITLE_Y);
        guide_custom_level_idle_countdown_ui_create(info);
    } else if (GUIDE_PAGE_IS_LEVEL_INTERACTIVE(page_index)) {
        guide_custom_label_step_header_create(info, &info->level_guide.title,
            "Adjust to comfortable", GUIDE_LEVEL_TITLE_Y);
    } else {
        guide_custom_label_level_header_create(info, &info->level_guide.title);
    }
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, cfg->label_res_id,
        cfg->label_x, cfg->label_y);
    guide_custom_label_instruction_headline_body_create(info, &info->level_guide.step,
        &info->level_guide.text, cfg->headline, cfg->body, cfg->headline_x, cfg->headline_y,
        cfg->body_x, cfg->body_y, cfg->line_space);
    if (cfg->button_icon_res_id != 0) {
        guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
            cfg->button_icon_res_id, cfg->icon_x, cfg->icon_y);
    }
}

void guide_custom_level_guide_ui_update(guide_custom_ui_info_t *info, uint8_t page_index)
{
    const guide_level_guide_page_cfg_t *cfg = guide_level_guide_page_cfg_get(page_index);

    EXCEPTION_CHECK_VOID(info && info->ui && cfg);

    info->level_guide.page_index = page_index;

    if (info->level_guide.title == NULL || !lv_obj_is_valid(info->level_guide.title)) {
        if (GUIDE_PAGE_SHOW_LEVEL_COUNTDOWN(page_index)) {
            guide_custom_label_step_header_create(info, &info->level_guide.title,
                "Adjust to comfortable", GUIDE_LEVEL_TITLE_Y);
            guide_custom_level_idle_countdown_ui_create(info);
        } else if (GUIDE_PAGE_IS_LEVEL_INTERACTIVE(page_index)) {
            guide_custom_label_step_header_create(info, &info->level_guide.title,
                "Adjust to comfortable", GUIDE_LEVEL_TITLE_Y);
        } else {
            guide_custom_label_level_header_create(info, &info->level_guide.title);
        }
    } else {
        lv_label_set_text(info->level_guide.title, "Adjust to comfortable");
        lv_obj_align(info->level_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_TITLE_Y);
        lv_obj_clear_flag(info->level_guide.title, LV_OBJ_FLAG_HIDDEN);
    }

    if (GUIDE_PAGE_SHOW_LEVEL_COUNTDOWN(page_index)) {
        if (info->level_guide.countdown_num == NULL ||
            !lv_obj_is_valid(info->level_guide.countdown_num)) {
            guide_custom_level_idle_countdown_ui_create(info);
        } else {
            lv_obj_clear_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
        }
    } else if (info->level_guide.countdown_num != NULL &&
        lv_obj_is_valid(info->level_guide.countdown_num)) {
        lv_obj_add_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
    }

    guide_custom_label_instruction_card_update(info, &info->level_guide.card, cfg->label_res_id,
        cfg->label_x, cfg->label_y);

    if (info->level_guide.step == NULL || !lv_obj_is_valid(info->level_guide.step) ||
        info->level_guide.text == NULL || !lv_obj_is_valid(info->level_guide.text)) {
        guide_custom_label_instruction_headline_body_create(info, &info->level_guide.step,
            &info->level_guide.text, cfg->headline, cfg->body, cfg->headline_x, cfg->headline_y,
            cfg->body_x, cfg->body_y, cfg->line_space);
    } else {
        lv_label_set_text(info->level_guide.step, cfg->headline);
        guide_custom_label_top_align(info->level_guide.step, cfg->headline_x, cfg->headline_y);
        lv_label_set_text(info->level_guide.text, cfg->body);
        lv_obj_set_style_text_line_space(info->level_guide.text, cfg->line_space, 0);
        guide_custom_label_top_align(info->level_guide.text, cfg->body_x, cfg->body_y);
        lv_obj_clear_flag(info->level_guide.step, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info->level_guide.text, LV_OBJ_FLAG_HIDDEN);
    }

    if (cfg->button_icon_res_id != 0) {
        guide_custom_label_inline_icon_update(info, &info->level_guide.button_icon,
            cfg->button_icon_res_id, cfg->icon_x, cfg->icon_y);
    } else if (info->level_guide.button_icon != NULL &&
        lv_obj_is_valid(info->level_guide.button_icon)) {
        lv_obj_add_flag(info->level_guide.button_icon, LV_OBJ_FLAG_HIDDEN);
    }
}

void guide_custom_level_idle_countdown_ui_update(guide_custom_ui_info_t *info, ui_proc_guide_t *proc)
{
    uint8_t remain_sec;
    char num_buf[8];

    EXCEPTION_CHECK_VOID(info && proc);
    if (!GUIDE_PAGE_SHOW_LEVEL_COUNTDOWN(proc->page_index)) {
        return;
    }
    if (info->level_guide.countdown_num == NULL || !lv_obj_is_valid(info->level_guide.countdown_num)) {
        guide_custom_level_idle_countdown_ui_create(info);
        if (info->level_guide.countdown_num == NULL) {
            return;
        }
    }

    remain_sec = guide_pump_level_idle_remain_sec_get(proc);
    if (info->level_guide.shown_countdown_sec == remain_sec) {
        return;
    }

    if (remain_sec == 0) {
        lv_obj_add_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
        guide_custom_page_num_ui_set_visible(info, true);
    } else {
        guide_custom_page_num_ui_set_visible(info, false);
        lv_snprintf(num_buf, sizeof(num_buf), "%us", remain_sec);
        lv_label_set_text(info->level_guide.countdown_num, num_buf);
        lv_obj_align(info->level_guide.countdown_num, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_COUNTDOWN_Y);
        lv_obj_clear_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(info->level_guide.countdown_num);
    }
    info->level_guide.shown_countdown_sec = remain_sec;
}

void guide_custom_mode_select_guide_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_MODE_SELECT_GUIDE;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    if (!guide_custom_level_theme_is_active(info)) {
        guide_level_guide_bg_create(info);
    }
#endif
    guide_custom_label_step_header_create(info, &info->level_guide.title,
        "Choose pumping way", GUIDE_MODE_SELECT_TITLE_Y);
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, CUSTOM_RES_ID_GUIDE_WIDTH_LABLE,
        GUIDE_MODE_GUIDE_LABEL_X, GUIDE_MODE_GUIDE_LABEL_Y);
    guide_custom_label_instruction_headline_body_create(info, &info->level_guide.step,
        &info->level_guide.text, "Press         ", "Switch between 3 modes",
        GUIDE_MODE_GUIDE_HEADLINE_X, GUIDE_MODE_GUIDE_HEADLINE_Y,
        GUIDE_MODE_GUIDE_BODY_X, GUIDE_MODE_GUIDE_BODY_Y, 4);
    guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
        CUSTOM_RES_ID_MODE_BUTTON_ICON, GUIDE_MODE_SELECT_START_ICON_X,
        GUIDE_MODE_SELECT_START_ICON_Y);
}

void guide_custom_rhythm_select_guide_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_RHYTHM_SELECT_GUIDE;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    if (!guide_custom_level_theme_is_active(info)) {
        guide_level_guide_bg_create(info);
    }
#endif
    guide_custom_label_step_header_create(info, &info->level_guide.title,
        "Choose pumping way", GUIDE_MODE_SELECT_TITLE_Y);
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, CUSTOM_RES_ID_GUIDE_WIDTH_LABLE,
        GUIDE_MODE_GUIDE_LABEL_X, GUIDE_MODE_GUIDE_LABEL_Y);
    guide_custom_label_instruction_headline_body_create(info, &info->level_guide.step,
        &info->level_guide.text, "Press         ", "Switch programs",
        GUIDE_MODE_GUIDE_HEADLINE_X, GUIDE_MODE_GUIDE_HEADLINE_Y,
        GUIDE_MODE_GUIDE_BODY_X, GUIDE_MODE_GUIDE_BODY_Y, 4);
    guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
        CUSTOM_RES_ID_PROGRAM_BUTTON_ICON, GUIDE_MODE_SELECT_START_ICON_X,
        GUIDE_MODE_SELECT_START_ICON_Y);
}

void guide_custom_pump_stop_guide_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_PUMP_STOP_GUIDE;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    guide_level_guide_bg_create(info);
#endif

    info->level_guide.title = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.title);
    lv_obj_set_style_text_color(info->level_guide.title,
        lv_color_hex(GUIDE_PUMP_STOP_GUIDE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.title, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(info->level_guide.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->level_guide.title, "Finish pumping");
    lv_obj_align(info->level_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_STOP_GUIDE_TITLE_Y);

    guide_custom_label_instruction_card_create(info, &info->level_guide.card, CUSTOM_RES_ID_GUIDE_WIDTH_LABLE,
        GUIDE_PUMP_STOP_GUIDE_LABEL_X, GUIDE_PUMP_STOP_GUIDE_LABEL_Y);

    info->level_guide.step = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.step);
    lv_obj_set_style_text_color(info->level_guide.step,
        lv_color_hex(GUIDE_PUMP_STOP_GUIDE_HINT_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.step, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(info->level_guide.step, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(info->level_guide.step, "Press");
    lv_obj_align(info->level_guide.step, LV_ALIGN_TOP_LEFT, GUIDE_PUMP_STOP_GUIDE_PRESS_X,
        GUIDE_PUMP_STOP_GUIDE_PRESS_Y);

    info->level_guide.text = lv_label_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.text);
    lv_obj_set_style_text_color(info->level_guide.text,
        lv_color_hex(GUIDE_PUMP_STOP_GUIDE_HINT_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.text, &lv_font_aeonik_soft_19_medium, 0);
    lv_obj_set_style_text_align(info->level_guide.text, LV_TEXT_ALIGN_CENTER, 0);
    // lv_label_set_text(info->level_guide.text, "to stop pumping");
    lv_label_set_text(info->level_guide.text, "& hold 3s");
    lv_obj_align(info->level_guide.text, LV_ALIGN_TOP_LEFT, GUIDE_PUMP_STOP_GUIDE_HINT_X, GUIDE_PUMP_STOP_GUIDE_HINT_Y);

    guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
        CUSTOM_RES_ID_GUIDE_RETURN_BUTTON_MIN_ICON, GUIDE_PUMP_STOP_GUIDE_ICON_X,
        GUIDE_PUMP_STOP_GUIDE_ICON_Y);
}

void guide_custom_page_num_ui_build(guide_custom_ui_info_t *info, uint8_t page_index)
{
    lv_obj_t *parent;
    char cur_buf[4];
    char total_buf[4];
    uint8_t page_num;
    lv_coord_t current_w;
    lv_coord_t suffix_w;
    lv_coord_t total_w;
    lv_coord_t group_w;
    lv_coord_t parent_w;
    lv_coord_t current_x;
    lv_coord_t suffix_x;
    lv_coord_t total_x;
    lv_coord_t current_y = GUIDE_CUSTOM_PAGE_NUM_CURRENT_Y;
    lv_coord_t suffix_y = GUIDE_CUSTOM_PAGE_NUM_SUFFIX_Y;
    lv_coord_t total_y = GUIDE_CUSTOM_PAGE_NUM_TOTAL_Y;

    EXCEPTION_CHECK_VOID(info);
    info->page_num.current = NULL;
    info->page_num.suffix = NULL;
    info->page_num.total = NULL;
    if (!GUIDE_PAGE_SHOW_NUM(page_index)) {
        return;
    }

    parent = guide_custom_page_parent_get(info);
    EXCEPTION_CHECK_VOID(parent);
    page_num = guide_page_num_of(page_index);
    if (page_index == GUIDE_PAGE_MODE_SELECT || page_index == GUIDE_PAGE_RHYTHM_SELECT) {
        current_y -= 4;
        suffix_y -= 4;
        total_y -= 4;
    }
    lv_snprintf(cur_buf, sizeof(cur_buf), "%u", page_num);
    lv_snprintf(total_buf, sizeof(total_buf), "%u", GUIDE_PAGE_NUM_TOTAL);

    info->page_num.current = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(info->page_num.current);
    lv_obj_set_style_text_color(info->page_num.current, lv_color_hex(GUIDE_PAGE_NUM_CURRENT_COLOR), 0);
    lv_obj_set_style_text_font(info->page_num.current, &lv_font_aeonik_soft_19_medium, 0);
    lv_label_set_text(info->page_num.current, cur_buf);
    lv_obj_update_layout(info->page_num.current);
    current_w = lv_obj_get_width(info->page_num.current);

    info->page_num.suffix = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(info->page_num.suffix);
    lv_obj_set_style_text_color(info->page_num.suffix, lv_color_hex(GUIDE_PAGE_NUM_SUFFIX_COLOR), 0);
    lv_obj_set_style_text_font(info->page_num.suffix, &lv_font_aeonik_soft_19_medium, 0);
    lv_label_set_text(info->page_num.suffix, "/");
    lv_obj_update_layout(info->page_num.suffix);
    suffix_w = lv_obj_get_width(info->page_num.suffix);

    info->page_num.total = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(info->page_num.total);
    lv_obj_set_style_text_color(info->page_num.total, lv_color_hex(GUIDE_PAGE_NUM_SUFFIX_COLOR), 0);
    lv_obj_set_style_text_font(info->page_num.total, &lv_font_aeonik_soft_19_medium, 0);
    lv_label_set_text(info->page_num.total, total_buf);
    lv_obj_update_layout(info->page_num.total);
    total_w = lv_obj_get_width(info->page_num.total);

    lv_obj_update_layout(parent);
    parent_w = lv_obj_get_width(parent);
    group_w = current_w + suffix_w + total_w;
    current_x = (parent_w - group_w) / 2;
    suffix_x = current_x + current_w;
    total_x = suffix_x + suffix_w;
    lv_obj_align(info->page_num.current, LV_ALIGN_TOP_LEFT, current_x, current_y);
    lv_obj_align(info->page_num.suffix, LV_ALIGN_TOP_LEFT, suffix_x, suffix_y);
    lv_obj_align(info->page_num.total, LV_ALIGN_TOP_LEFT, total_x, total_y);
}

void guide_custom_page_num_ui_set_visible(guide_custom_ui_info_t *info, bool visible)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->page_num.current != NULL && lv_obj_is_valid(info->page_num.current)) {
        if (visible) {
            lv_obj_clear_flag(info->page_num.current, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(info->page_num.current, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (info->page_num.suffix != NULL && lv_obj_is_valid(info->page_num.suffix)) {
        if (visible) {
            lv_obj_clear_flag(info->page_num.suffix, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(info->page_num.suffix, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (info->page_num.total != NULL && lv_obj_is_valid(info->page_num.total)) {
        if (visible) {
            lv_obj_clear_flag(info->page_num.total, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(info->page_num.total, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void guide_custom_pump_label_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->label.label);
    ui_img_baj_release_decoded_buf(info->label.return_icon);
    ui_img_baj_release_decoded_buf(info->label.icon);
    for (uint8_t i = 0; i < 2; i++) {
        ui_img_baj_release_decoded_buf(info->label.button[i]);
        info->label.button_img_id[i] = 0;
    }
    info->label.icon_img_id = 0;
    info->label.text_id = GUIDE_PAGE_MAX;
}

void guide_custom_pump_label_ui_build(guide_custom_ui_info_t *info, uint8_t page_index)
{
    (void)info;
    (void)page_index;
}
