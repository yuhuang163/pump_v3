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
#define GUIDE_LEVEL_STEP_Y                          (54)
#define GUIDE_LEVEL_TITLE_Y                         (86)
#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
#define GUIDE_LEVEL_STEP_COLOR                      (0x00FF00)
#define GUIDE_LEVEL_TITLE_COLOR                     (0x00FF00)
#else
#define GUIDE_LEVEL_STEP_COLOR                      (0xCA5C77)
#define GUIDE_LEVEL_TITLE_COLOR                     (0xBD4060)
#endif

#define GUIDE_LEVEL_WIDE_LABEL_X                    (43)
#define GUIDE_LEVEL_WIDE_LABEL_Y                    (112)
#define GUIDE_LEVEL_WIDE_LABEL_TEXT_Y               (150)

#define GUIDE_LEVEL_NARROW_LABEL_X                  (43)
#define GUIDE_LEVEL_NARROW_LABEL_Y                  (117)
#define GUIDE_LEVEL_NARROW_LABEL_TEXT_Y             (148)

#define GUIDE_LEVEL_SIDE_BUTTON_ICON_X              (194)
#define GUIDE_LEVEL_SIDE_BUTTON_ICON_Y              (144)

#define GUIDE_MODE_SELECT_START_ICON_X              (153)
#define GUIDE_MODE_SELECT_START_ICON_Y              (144)

#define GUIDE_PUMP_START_GUIDE_ICON_X               (160)
#define GUIDE_PUMP_START_GUIDE_ICON_Y               (144)

#define GUIDE_PUMP_STOP_GUIDE_ICON_X                (144)
#define GUIDE_PUMP_STOP_GUIDE_ICON_Y                (144)

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
        "Rotate the knob\nAdjust suction levels for\nboth sides",
        CUSTOM_RES_ID_HOME_WIDTH_LABLE,
        GUIDE_LEVEL_WIDE_LABEL_X, GUIDE_LEVEL_WIDE_LABEL_Y,
        GUIDE_LEVEL_WIDE_LABEL_TEXT_Y, 4, 0,
    },
    [GUIDE_PAGE_LEVEL_BOTH] = {
        "Rotate the knob\nAdjust suction levels for\nboth sides",
        CUSTOM_RES_ID_HOME_WIDTH_LABLE,
        GUIDE_LEVEL_WIDE_LABEL_X, GUIDE_LEVEL_WIDE_LABEL_Y,
        GUIDE_LEVEL_WIDE_LABEL_TEXT_Y, 4, 0,
    },
    [GUIDE_PAGE_LEVEL_LEFT_GUIDE] = {
        "Press        \nAdjust left level",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 8, CUSTOM_RES_ID_GUIDE_LEFT_BUTTON_ICON,
    },
    [GUIDE_PAGE_LEVEL_LEFT] = {
        "Press        \nAdjust left level",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 8, CUSTOM_RES_ID_GUIDE_LEFT_BUTTON_ICON,
    },
    [GUIDE_PAGE_LEVEL_RIGHT_GUIDE] = {
        "Press        \nAdjust right level",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 8, CUSTOM_RES_ID_GUIDE_RIGHT_BUTTON_ICON,
    },
    [GUIDE_PAGE_LEVEL_RIGHT] = {
        "Press        \nAdjust right level",
        CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y,
        GUIDE_LEVEL_NARROW_LABEL_TEXT_Y, 8, CUSTOM_RES_ID_GUIDE_RIGHT_BUTTON_ICON,
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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_label_step_header_create(guide_custom_ui_info_t *info, lv_obj_t **step, lv_obj_t **title,
    const char *step_text, const char *title_text)
{
    EXCEPTION_CHECK_VOID(info);

    if (step != NULL && step_text != NULL) {
        *step = lv_label_create(guide_custom_page_parent_get(info));
        EXCEPTION_CHECK_VOID(*step);
        lv_obj_set_style_text_color(*step, lv_color_hex(GUIDE_LEVEL_STEP_COLOR), 0);
        lv_obj_set_style_text_font(*step, &lv_font_aeonik_soft_20_semibold, 0);
        lv_obj_set_style_text_align(*step, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(*step, step_text);
        lv_obj_align(*step, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_STEP_Y);
    }

    if (title != NULL && title_text != NULL) {
        *title = lv_label_create(guide_custom_page_parent_get(info));
        EXCEPTION_CHECK_VOID(*title);
        lv_obj_set_style_text_color(*title, lv_color_hex(GUIDE_LEVEL_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(*title, &lv_font_aeonik_soft_24_semibold, 0);
        lv_obj_set_style_text_align(*title, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(*title, title_text);
        lv_obj_align(*title, LV_ALIGN_TOP_MID, 0, GUIDE_LEVEL_TITLE_Y);
    }
}

void guide_custom_label_level_header_create(guide_custom_ui_info_t *info, lv_obj_t **step, lv_obj_t **title)
{
    guide_custom_label_step_header_create(info, step, title, "Step 1", "Adjust to comfortable");
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
    lv_obj_set_style_text_font(*text, &lv_font_aeonik_soft_18_regular, 0);
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

void guide_custom_level_guide_ui_build(guide_custom_ui_info_t *info, uint8_t page_index)
{
    const guide_level_guide_page_cfg_t *cfg = guide_level_guide_page_cfg_get(page_index);

    EXCEPTION_CHECK_VOID(info && info->ui && cfg);

    info->level_guide.page_index = page_index;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    guide_level_guide_bg_create(info);
#endif
    guide_custom_label_level_header_create(info, &info->level_guide.step, &info->level_guide.title);
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, cfg->label_res_id,
        cfg->label_x, cfg->label_y);
    guide_custom_label_instruction_text_create(info, &info->level_guide.text, cfg->text,
        cfg->text_y, cfg->line_space);
    if (cfg->button_icon_res_id != 0) {
        guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
            cfg->button_icon_res_id, GUIDE_LEVEL_SIDE_BUTTON_ICON_X,
            GUIDE_LEVEL_SIDE_BUTTON_ICON_Y);
    }
}

void guide_custom_mode_select_guide_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_MODE_SELECT_GUIDE;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    guide_level_guide_bg_create(info);
#endif
    guide_custom_label_step_header_create(info, &info->level_guide.step, &info->level_guide.title,
        "Step 2", "Choose pumping way");
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y);
    guide_custom_label_instruction_text_create(info, &info->level_guide.text,
        "Press          to switch\nbetween 3 modes", GUIDE_LEVEL_NARROW_LABEL_TEXT_Y + 2, 4);
    guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
        CUSTOM_RES_ID_MODE_BUTTON_ICON, GUIDE_MODE_SELECT_START_ICON_X,
        GUIDE_MODE_SELECT_START_ICON_Y);
}

void guide_custom_rhythm_select_guide_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_RHYTHM_SELECT_GUIDE;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    guide_level_guide_bg_create(info);
#endif
    guide_custom_label_step_header_create(info, &info->level_guide.step, &info->level_guide.title,
        "Step 2", "Choose pumping way");
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y);
    guide_custom_label_instruction_text_create(info, &info->level_guide.text,
        "Press          to switch\nprogram", GUIDE_LEVEL_NARROW_LABEL_TEXT_Y + 2, 4);
    guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
        CUSTOM_RES_ID_PROGRAM_BUTTON_ICON, GUIDE_MODE_SELECT_START_ICON_X,
        GUIDE_MODE_SELECT_START_ICON_Y);
}

void guide_custom_pump_start_guide_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_PUMP_START_GUIDE;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    guide_level_guide_bg_create(info);
#endif
    guide_custom_label_step_header_create(info, &info->level_guide.step, &info->level_guide.title,
        "Step 3", "Start / Stop pumping");
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y);
    guide_custom_label_instruction_text_create(info, &info->level_guide.text,
        "Press           to start\npumping", GUIDE_LEVEL_NARROW_LABEL_TEXT_Y + 2, 4);
    guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
        CUSTOM_RES_ID_GUIDE_START_BUTTON_MIN_ICON, GUIDE_PUMP_START_GUIDE_ICON_X,
        GUIDE_PUMP_START_GUIDE_ICON_Y);
}

void guide_custom_pump_stop_guide_ui_build(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);

    info->level_guide.page_index = GUIDE_PAGE_PUMP_STOP_GUIDE;
#if (IS_SUPPORT_CUSTOM_GUIDE_COMP_BG == 0)
    guide_level_guide_bg_create(info);
#endif
    guide_custom_label_step_header_create(info, &info->level_guide.step, &info->level_guide.title,
        "Step 3", "Start / Stop pumping");
    guide_custom_label_instruction_card_create(info, &info->level_guide.card, CUSTOM_RES_ID_GUIDE_LABLE,
        GUIDE_LEVEL_NARROW_LABEL_X, GUIDE_LEVEL_NARROW_LABEL_Y);
    guide_custom_label_instruction_text_create(info, &info->level_guide.text,
        "Press           and hold to\nstop pumping", GUIDE_LEVEL_NARROW_LABEL_TEXT_Y + 2, 4);
    guide_custom_label_inline_icon_create(info, &info->level_guide.button_icon,
        CUSTOM_RES_ID_GUIDE_RETURN_BUTTON_ICON, GUIDE_PUMP_STOP_GUIDE_ICON_X,
        GUIDE_PUMP_STOP_GUIDE_ICON_Y);
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
