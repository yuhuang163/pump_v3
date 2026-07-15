/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_LEVEL_STEP_Y                          (54)
#define GUIDE_LEVEL_TITLE_Y                         (86)
#define GUIDE_LEVEL_COUNTDOWN_Y                     (36)
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
#define GUIDE_LEVEL_STEP_COLOR                      (0x00FF00)
#define GUIDE_LEVEL_TITLE_COLOR                     (0x00FF00)
#define GUIDE_LEVEL_COUNTDOWN_COLOR                 (0x00FF00)
#else
#define GUIDE_LEVEL_STEP_COLOR                      (0xCA5C77)
#define GUIDE_LEVEL_TITLE_COLOR                     (0xBD4060)
#define GUIDE_LEVEL_COUNTDOWN_COLOR                 (0xBD4060)
#endif

#define GUIDE_LEVEL_WIDE_LABEL_X                    (49)
#if IS_SUPPORT_BASE_GUIDE_LABEL_TEXT_CMP
#define GUIDE_LEVEL_WIDE_LABEL_Y                    (360)
#else
#define GUIDE_LEVEL_WIDE_LABEL_Y                    (125)
#endif
#define GUIDE_LEVEL_WIDE_LABEL_TEXT_Y               (177)

#define GUIDE_LEVEL_NARROW_LABEL_X                  (36)
#if IS_SUPPORT_BASE_GUIDE_LABEL_TEXT_CMP
#define GUIDE_LEVEL_NARROW_LABEL_Y                  (360)
#else
#define GUIDE_LEVEL_NARROW_LABEL_Y                  (124)
#endif
#define GUIDE_LEVEL_NARROW_LABEL_TEXT_Y             (174)

#define GUIDE_LEVEL_SIDE_BUTTON_ICON_X              (196)
#define GUIDE_LEVEL_SIDE_BUTTON_ICON_Y              (170)

#define GUIDE_MODE_SELECT_START_ICON_X              (188)
#define GUIDE_MODE_SELECT_START_ICON_Y              (168)

#if IS_SUPPORT_BASE_GUIDE_COMP_BG
#define GUIDE_PUMP_PAUSE_GUIDE_TEXT_COLOR           (0x00FF00)
#else
#define GUIDE_PUMP_PAUSE_GUIDE_TEXT_COLOR           (0xBD4060)
#endif
#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_Y              (194)
#define GUIDE_PUMP_PAUSE_GUIDE_HINT_Y               (234)
#define GUIDE_PUMP_PAUSE_GUIDE_ICON_X               (107)
#define GUIDE_PUMP_PAUSE_GUIDE_ICON_Y               (230)

#if IS_SUPPORT_BASE_GUIDE_COMP_BG
#define GUIDE_PUMP_STOP_GUIDE_TEXT_COLOR            (0x00FF00)
#else
#define GUIDE_PUMP_STOP_GUIDE_TEXT_COLOR            (0xBD4060)
#endif
#define GUIDE_PUMP_STOP_GUIDE_TITLE_Y               (194)
#define GUIDE_PUMP_STOP_GUIDE_HINT_Y                (234)
#define GUIDE_PUMP_STOP_GUIDE_ICON_X                (115)
#define GUIDE_PUMP_STOP_GUIDE_ICON_Y                (230)

typedef struct {
    const char *text;
    uint16_t label_res_id;
    int16_t label_x;
    int16_t label_y;
    int16_t text_y;
    uint8_t line_space;
    uint16_t button_icon_res_id;
} guide_level_guide_page_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const guide_level_guide_page_cfg_t guide_level_guide_page_cfgs[GUIDE_PAGE_MAX] = {
    [GUIDE_PAGE_LEVEL_BOTH_GUIDE] = {
        "Rotate the knob\nAdjust suction levels\nfor both sides",
        BASE_RES_ID_HOME_WIDTH_LABLE,
        GUIDE_LEVEL_WIDE_LABEL_X, GUIDE_LEVEL_WIDE_LABEL_Y,
        GUIDE_LEVEL_WIDE_LABEL_TEXT_Y, 4, 0,
    },
    [GUIDE_PAGE_LEVEL_BOTH] = {
        "Rotate the knob\nAdjust suction levels\nfor both sides",
        BASE_RES_ID_HOME_WIDTH_LABLE,
        GUIDE_LEVEL_WIDE_LABEL_X, GUIDE_LEVEL_WIDE_LABEL_Y,
        GUIDE_LEVEL_WIDE_LABEL_TEXT_Y, 4, 0,
    },
    [GUIDE_PAGE_LEVEL_LEFT_GUIDE] = {
        "Press       \nEnter left adjustment",
        BASE_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 6, BASE_RES_ID_GUIDE_LEFT_BUTTON_ICON,
    },
    [GUIDE_PAGE_LEVEL_LEFT] = {
        "Rotate the knob\nAdjust left level",
        BASE_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 4, 0,
    },
    [GUIDE_PAGE_LEVEL_RIGHT_GUIDE] = {
        "Press       \nEnter right adjustment",
        BASE_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 6, BASE_RES_ID_GUIDE_RIGHT_BUTTON_ICON,
    },
    [GUIDE_PAGE_LEVEL_RIGHT] = {
        "Rotate the knob\nAdjust right level",
        BASE_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 4, 0,
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

static void guide_level_guide_bg_create(guide_base_ui_info_t *info)
{
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    info->level_guide.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.bg);
    lv_image_set_src(info->level_guide.bg, background_snapshot);
    lv_obj_align(info->level_guide.bg, LV_ALIGN_CENTER, 0, 0);
#else
    info->level_guide.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.bg);
    ui_img_png_theme_set_src(info->ui, info->level_guide.bg, BASE_RES_ID_COMMON_BG,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->level_guide.bg, LV_ALIGN_CENTER, 0, 0);
#endif
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_label_step_header_create(guide_base_ui_info_t *info, lv_obj_t **step, lv_obj_t **title,
    const char *step_text, const char *title_text)
{
    EXCEPTION_CHECK_VOID(info);

    if (step != NULL && step_text != NULL) {
        *step = lv_label_create(guide_page_parent_get(info));
        EXCEPTION_CHECK_VOID(*step);
        lv_obj_set_style_text_color(*step, lv_color_hex(GUIDE_LEVEL_STEP_COLOR), 0);
        lv_obj_set_style_text_font(*step, &lv_font_aeonik_soft_20_medium, 0);
        lv_obj_set_style_text_align(*step, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(*step, step_text);
        lv_obj_align(*step, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_STEP_Y);
    }

    if (title != NULL && title_text != NULL) {
        *title = lv_label_create(guide_page_parent_get(info));
        EXCEPTION_CHECK_VOID(*title);
        lv_obj_set_style_text_color(*title, lv_color_hex(GUIDE_LEVEL_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(*title, &lv_font_aeonik_soft_24_semibold, 0);
        lv_obj_set_style_text_align(*title, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(*title, title_text);
        lv_obj_align(*title, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_TITLE_Y);
    }
}

void guide_label_level_header_create(guide_base_ui_info_t *info, lv_obj_t **step, lv_obj_t **title)
{
    guide_label_step_header_create(info, step, title, NULL, "Adjust to comfortable");
}

static void guide_level_idle_countdown_ui_create(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);

    info->level_guide.countdown_num = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.countdown_num);
    lv_obj_set_style_text_color(info->level_guide.countdown_num,
        lv_color_hex(GUIDE_LEVEL_COUNTDOWN_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.countdown_num, &lv_font_aeonik_soft_36_regular, 0);
    lv_obj_set_style_text_align(info->level_guide.countdown_num, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->level_guide.countdown_num, "3s");
    lv_obj_align(info->level_guide.countdown_num, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_COUNTDOWN_Y);
    lv_obj_add_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
    info->level_guide.shown_countdown_sec = 0xFF;
}

void guide_label_instruction_card_create(guide_base_ui_info_t *info, lv_obj_t **card,
    uint16_t res_id, int16_t x, int16_t y)
{
    EXCEPTION_CHECK_VOID(info && card);

    *card = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*card);
    ui_img_baj_theme_set_src(info->ui, *card, res_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(*card, LV_ALIGN_TOP_LEFT, x, y);
}

void guide_label_instruction_text_create(guide_base_ui_info_t *info, lv_obj_t **text,
    const char *content, int16_t y, uint8_t line_space)
{
    EXCEPTION_CHECK_VOID(info && text && content);

    *text = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*text);
    lv_obj_set_style_text_color(*text, lv_color_hex(GUIDE_BASE_LABEL_CARD_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(*text, &lv_font_aeonik_soft_18_regular, 0);
    lv_obj_set_style_text_align(*text, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(*text, content);
    lv_obj_set_style_text_line_space(*text, line_space, 0);
    lv_obj_align(*text, LV_ALIGN_TOP_MID, 0, y);
}

void guide_label_inline_icon_create(guide_base_ui_info_t *info, lv_obj_t **icon,
    uint16_t res_id, int16_t x, int16_t y)
{
    EXCEPTION_CHECK_VOID(info && icon);

    *icon = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(*icon);
    ui_img_baj_theme_set_src(info->ui, *icon, res_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(*icon, LV_ALIGN_TOP_LEFT, x, y);
}

void guide_level_guide_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->level_guide.page_index == GUIDE_PAGE_PUMP_PAUSE_GUIDE ||
        info->level_guide.page_index == GUIDE_PAGE_PUMP_STOP_GUIDE) {
        ui_img_png_release_decoded_buf(info->level_guide.bg);
    } else {
#if !LV_USE_SNAPSHOT
        ui_img_png_release_decoded_buf(info->level_guide.bg);
#endif
    }
    ui_img_baj_release_decoded_buf(info->level_guide.card);
    ui_img_baj_release_decoded_buf(info->level_guide.button_icon);
}

void guide_level_guide_ui_build(guide_base_ui_info_t *info, uint8_t page_index)
{
    const guide_level_guide_page_cfg_t *cfg = guide_level_guide_page_cfg_get(page_index);

    EXCEPTION_CHECK_VOID(info && info->ui && cfg);

    info->level_guide.page_index = page_index;
#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    if (!guide_base_level_theme_is_active(info)) {
        guide_level_guide_bg_create(info);
    }
#endif
    if (GUIDE_PAGE_IS_LEVEL_INTERACTIVE(page_index)) {
        guide_label_step_header_create(info, NULL, &info->level_guide.title,
            NULL, "Adjust to comfortable");
        guide_level_idle_countdown_ui_create(info);
    } else {
        guide_label_level_header_create(info, &info->level_guide.step, &info->level_guide.title);
    }
    guide_label_instruction_card_create(info, &info->level_guide.card, cfg->label_res_id,
        cfg->label_x, cfg->label_y);
    guide_label_instruction_text_create(info, &info->level_guide.text, cfg->text,
        cfg->text_y, cfg->line_space);
    if (cfg->button_icon_res_id != 0) {
        guide_label_inline_icon_create(info, &info->level_guide.button_icon,
            cfg->button_icon_res_id, GUIDE_LEVEL_SIDE_BUTTON_ICON_X,
            GUIDE_LEVEL_SIDE_BUTTON_ICON_Y);
    }
}

void guide_level_idle_countdown_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    uint8_t remain_sec;
    char num_buf[8];

    EXCEPTION_CHECK_VOID(info && proc);
    if (!GUIDE_PAGE_IS_LEVEL_INTERACTIVE(proc->page_index)) {
        return;
    }
    if (info->level_guide.countdown_num == NULL || !lv_obj_is_valid(info->level_guide.countdown_num)) {
        return;
    }

    remain_sec = guide_pump_level_idle_remain_sec_get(proc);
    if (info->level_guide.shown_countdown_sec == remain_sec) {
        return;
    }

    if (remain_sec == 0) {
        lv_obj_add_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_snprintf(num_buf, sizeof(num_buf), "%us", remain_sec);
        lv_label_set_text(info->level_guide.countdown_num, num_buf);
        lv_obj_align(info->level_guide.countdown_num, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_COUNTDOWN_Y);
        lv_obj_clear_flag(info->level_guide.countdown_num, LV_OBJ_FLAG_HIDDEN);
    }
    info->level_guide.shown_countdown_sec = remain_sec;
}

void guide_mode_select_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_MODE_SELECT_GUIDE;
#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    if (!guide_base_level_theme_is_active(info)) {
        guide_level_guide_bg_create(info);
    }
#endif
    guide_label_step_header_create(info, &info->level_guide.step, &info->level_guide.title,
        NULL, "Choose pumping way");
    guide_label_instruction_card_create(info, &info->level_guide.card, BASE_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y);
    guide_label_instruction_text_create(info, &info->level_guide.text,
        "Press         \nswitch between 3 modes", GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 6);
    guide_label_inline_icon_create(info, &info->level_guide.button_icon,
        BASE_RES_ID_MODE_BUTTON_ICON, GUIDE_MODE_SELECT_START_ICON_X,
        GUIDE_MODE_SELECT_START_ICON_Y);
}

void guide_rhythm_select_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_RHYTHM_SELECT_GUIDE;
#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    if (!guide_base_level_theme_is_active(info)) {
        guide_level_guide_bg_create(info);
    }
#endif
    guide_label_step_header_create(info, &info->level_guide.step, &info->level_guide.title,
        NULL, "Choose pumping way");
    guide_label_instruction_card_create(info, &info->level_guide.card, BASE_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y);
    guide_label_instruction_text_create(info, &info->level_guide.text,
        "Press         \nswitch program", GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 6);
    guide_label_inline_icon_create(info, &info->level_guide.button_icon,
        BASE_RES_ID_PROGRAM_BUTTON_ICON, GUIDE_MODE_SELECT_START_ICON_X,
        GUIDE_MODE_SELECT_START_ICON_Y);
}

void guide_pump_pause_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_PUMP_PAUSE_GUIDE;
#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    info->level_guide.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.bg);
    ui_img_png_theme_set_src(info->ui, info->level_guide.bg, BASE_RES_ID_GUIDE_BG,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->level_guide.bg, LV_ALIGN_CENTER, 0, 0);
#endif

    info->level_guide.title = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.title);
    lv_obj_set_style_text_color(info->level_guide.title,
        lv_color_hex(GUIDE_PUMP_PAUSE_GUIDE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.title, &lv_font_aeonik_soft_24_semibold, 0);
    lv_obj_set_style_text_align(info->level_guide.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->level_guide.title, "Last Step");
    lv_obj_align(info->level_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_PAUSE_GUIDE_TITLE_Y);

    info->level_guide.text = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.text);
    lv_obj_set_style_text_color(info->level_guide.text,
        lv_color_hex(GUIDE_PUMP_PAUSE_GUIDE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.text, &lv_font_aeonik_soft_20_medium, 0);
    lv_obj_set_style_text_align(info->level_guide.text, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->level_guide.text, "Press         to pause pumping");
    lv_obj_align(info->level_guide.text, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_PAUSE_GUIDE_HINT_Y);

    guide_label_inline_icon_create(info, &info->level_guide.button_icon,
        BASE_RES_ID_GUIDE_START_BUTTON_ICON, GUIDE_PUMP_PAUSE_GUIDE_ICON_X,
        GUIDE_PUMP_PAUSE_GUIDE_ICON_Y);
}

void guide_pump_stop_guide_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_PUMP_STOP_GUIDE;
#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    info->level_guide.bg = lv_image_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.bg);
    ui_img_png_theme_set_src(info->ui, info->level_guide.bg, BASE_RES_ID_GUIDE_BG,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    lv_obj_align(info->level_guide.bg, LV_ALIGN_CENTER, 0, 0);
#endif

    info->level_guide.title = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.title);
    lv_obj_set_style_text_color(info->level_guide.title,
        lv_color_hex(GUIDE_PUMP_STOP_GUIDE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.title, &lv_font_aeonik_soft_24_semibold, 0);
    lv_obj_set_style_text_align(info->level_guide.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->level_guide.title, "Last Step");
    lv_obj_align(info->level_guide.title, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_STOP_GUIDE_TITLE_Y);

    info->level_guide.text = lv_label_create(guide_page_parent_get(info));
    EXCEPTION_CHECK_VOID(info->level_guide.text);
    lv_obj_set_style_text_color(info->level_guide.text,
        lv_color_hex(GUIDE_PUMP_STOP_GUIDE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->level_guide.text, &lv_font_aeonik_soft_20_medium, 0);
    lv_obj_set_style_text_align(info->level_guide.text, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(info->level_guide.text, "Press         to stop pumping");
    lv_obj_align(info->level_guide.text, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_STOP_GUIDE_HINT_Y);

    guide_label_inline_icon_create(info, &info->level_guide.button_icon,
        BASE_RES_ID_GUIDE_RETURN_BUTTON_ICON, GUIDE_PUMP_STOP_GUIDE_ICON_X,
        GUIDE_PUMP_STOP_GUIDE_ICON_Y);
}

void guide_pump_label_ui_remove(guide_base_ui_info_t *info)
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

void guide_pump_label_ui_build(guide_base_ui_info_t *info, uint8_t page_index)
{
    (void)info;
    (void)page_index;
}
