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
#define IS_SUPPORT_GUIDE_UI_TEST                     (0)

#if IS_SUPPORT_GUIDE_UI_TEST
#define GUIDE_TITLE_TEXT_COLOR                      (0x00FF00)
#define GUIDE_CARD_TEXT_COLOR                       (0x00FF00)
#define GUIDE_EXIT_TEXT_COLOR                       (0x00FF00)
#else
#define GUIDE_TITLE_TEXT_COLOR                      (0xBD4060)
#define GUIDE_CARD_TEXT_COLOR                       (0xA0183B)
#define GUIDE_EXIT_TEXT_COLOR                       (0xC87A8C)
#endif

#define GUIDE_TITLE_Y                               (68)
#define GUIDE_LABEL_X                               (43)
#define GUIDE_LABEL_Y                               (97)
#define GUIDE_LABEL_TEXT_Y                          (132)
#define GUIDE_EXIT_TEXT_Y                           (216)
#define GUIDE_EXIT_ICON_X                           (165)
#define GUIDE_EXIT_ICON_Y                           (210)
#define GUIDE_MODE_BUTTON_X                         (137)
#define GUIDE_PROGRAM_BUTTON_X                      (169)
#define GUIDE_PLAN_BUTTON_Y                         (128)

typedef struct {
    const char *text;
    uint16_t y_offset;
    uint8_t line_space;
} guide_custom_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static guide_custom_text_info_t guide_custom_label_infos[GUIDE_PAGE_MAX] = {
    [GUIDE_PAGE_PUMP_LEVEL_DOUBLE_ADJUST] =
        {"Rotate the knob to\nadjust both sides' level", GUIDE_LABEL_TEXT_Y, 4},
    [GUIDE_PAGE_PUMP_LEVEL_SINGLE_SELECT] =
        {"Press L/R to enter\nsingle-side adjustment", GUIDE_LABEL_TEXT_Y, 4},
    [GUIDE_PAGE_PUMP_LEVEL_LEFT_ADJUST] =
        {"Rotate the knob to\nadjust the left side", GUIDE_LABEL_TEXT_Y, 4},
    [GUIDE_PAGE_PUMP_LEVEL_RIGHT_ADJUST] =
        {"Rotate the knob to\nadjust the right side", GUIDE_LABEL_TEXT_Y, 4},
    [GUIDE_PAGE_PUMP_MODE_SELECT] =
        {"Press                   to select\na pumping plan", GUIDE_LABEL_TEXT_Y + 2, 6},
    [GUIDE_PAGE_PUMP_MODE_UNSELECT] =
        {"No pumping plan\nselected", GUIDE_LABEL_TEXT_Y, 4},
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool guide_label_need_update(const guide_custom_ui_info_t *info)
{
    return (info->label.force_update != 0 || info->force_update != 0);
}

static guide_custom_text_id_e guide_custom_text_id_get(uint8_t page_index)
{
    if (page_index >= GUIDE_PAGE_MAX) {
        return GUIDE_CUSTOM_TEXT_DOUBLE_ADJUST;
    }
    return (guide_custom_text_id_e)page_index;
}

static void guide_label_title_update(guide_custom_ui_info_t *info)
{
    if (info->label.title == NULL || !lv_obj_is_valid(info->label.title)) {
        info->label.title = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.title);
        lv_obj_set_style_text_color(info->label.title, lv_color_hex(GUIDE_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(info->label.title, &lv_font_aeonik_soft_24_bold, 0);
        lv_obj_set_style_text_align(info->label.title, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(info->label.title, "Operation Guidance");
        lv_obj_align(info->label.title, LV_ALIGN_TOP_MID, 0, GUIDE_TITLE_Y);
    }
}

static void guide_label_bg_update(guide_custom_ui_info_t *info)
{
    if (info->label.label == NULL || !lv_obj_is_valid(info->label.label)) {
        info->label.label = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.label);
        ui_img_baj_theme_set_src(info->ui, info->label.label, CUSTOM_RES_ID_GUIDE_LABLE,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(info->label.label, LV_ALIGN_TOP_LEFT, GUIDE_LABEL_X, GUIDE_LABEL_Y);
    }
}

static void guide_label_text_update(guide_custom_ui_info_t *info, guide_custom_text_id_e text_id)
{
    if (info->label.text == NULL || !lv_obj_is_valid(info->label.text)) {
        info->label.text = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.text);
        lv_obj_set_style_text_color(info->label.text, lv_color_hex(GUIDE_CARD_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(info->label.text, &lv_font_aeonik_soft_18_regular, 0);
        lv_obj_set_style_text_align(info->label.text, LV_TEXT_ALIGN_CENTER, 0);
    }

    if (text_id >= GUIDE_CUSTOM_TEXT_MAX) {
        lv_obj_add_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
        info->label.text_id = GUIDE_CUSTOM_TEXT_MAX;
        return;
    }

    if (!guide_label_need_update(info) && info->label.text_id == text_id) {
        return;
    }

    const guide_custom_text_info_t *text_info = &guide_custom_label_infos[text_id];
    lv_obj_clear_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->label.text, text_info->text);
    lv_obj_set_style_text_line_space(info->label.text, text_info->line_space, 0);
    lv_obj_align(info->label.text, LV_ALIGN_TOP_MID, 0, text_info->y_offset);
    info->label.text_id = text_id;
}

static void guide_label_plan_button_update(guide_custom_ui_info_t *info, bool show)
{
    static const uint16_t button_res_ids[2] = {
        CUSTOM_RES_ID_MODE_BUTTON_ICON,
        CUSTOM_RES_ID_PROGRAM_BUTTON_ICON,
    };

    for (uint8_t i = 0; i < 2; i++) {
        if (info->label.button[i] == NULL || !lv_obj_is_valid(info->label.button[i])) {
            info->label.button[i] = lv_image_create(info->root);
            EXCEPTION_CHECK_VOID(info->label.button[i]);
            lv_obj_add_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (!show) {
        for (uint8_t i = 0; i < 2; i++) {
            if (info->label.button[i] == NULL || !lv_obj_is_valid(info->label.button[i])) {
                continue;
            }
            if (info->label.button_img_id[i] != 0) {
                ui_img_baj_release_decoded_buf(info->label.button[i]);
                info->label.button_img_id[i] = 0;
            }
            lv_obj_add_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    for (uint8_t i = 0; i < 2; i++) {
        if (info->label.button[i] == NULL || !lv_obj_is_valid(info->label.button[i])) {
            continue;
        }

        const uint16_t img_id = button_res_ids[i];
        if (!guide_label_need_update(info) && info->label.button_img_id[i] == img_id) {
            lv_obj_clear_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        lv_obj_clear_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
        ui_img_baj_theme_set_src(info->ui, info->label.button[i], img_id,
            RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
        if (i == 0) {
            lv_obj_align(info->label.button[i], LV_ALIGN_TOP_LEFT,
                GUIDE_MODE_BUTTON_X, GUIDE_PLAN_BUTTON_Y);
        } else {
            lv_obj_align(info->label.button[i], LV_ALIGN_TOP_LEFT,
                GUIDE_PROGRAM_BUTTON_X, GUIDE_PLAN_BUTTON_Y);
        }
        info->label.button_img_id[i] = img_id;
    }
}

static void guide_label_exit_update(guide_custom_ui_info_t *info)
{
    if (info->label.exit_text == NULL || !lv_obj_is_valid(info->label.exit_text)) {
        info->label.exit_text = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.exit_text);
        lv_obj_set_style_text_color(info->label.exit_text, lv_color_hex(GUIDE_EXIT_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(info->label.exit_text, &lv_font_aeonik_soft_18_regular, 0);
        lv_obj_set_style_text_align(info->label.exit_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(info->label.exit_text, "Press          to exit");
        lv_obj_align(info->label.exit_text, LV_ALIGN_TOP_MID, 0, GUIDE_EXIT_TEXT_Y);
    }

    if (info->label.return_icon == NULL || !lv_obj_is_valid(info->label.return_icon)) {
        info->label.return_icon = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.return_icon);
        ui_img_baj_theme_set_src(info->ui, info->label.return_icon,
            CUSTOM_RES_ID_RETURN_BUTTON_24X24_ICON, RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(info->label.return_icon, LV_ALIGN_TOP_LEFT, GUIDE_EXIT_ICON_X, GUIDE_EXIT_ICON_Y);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_pump_label_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->label.label);
    ui_img_baj_release_decoded_buf(info->label.return_icon);
    for (uint8_t i = 0; i < 2; i++) {
        ui_img_baj_release_decoded_buf(info->label.button[i]);
        info->label.button_img_id[i] = 0;
    }
    info->label.text_id = GUIDE_CUSTOM_TEXT_MAX;
}

void guide_custom_pump_label_ui_update(guide_custom_ui_info_t *info, uint8_t page_index)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    if (info->label.page_index != page_index) {
        info->label.page_index = page_index;
        info->label.force_update = true;
    }

    guide_label_title_update(info);
    guide_label_bg_update(info);
    const guide_custom_text_id_e text_id = guide_custom_text_id_get(page_index);
    guide_label_text_update(info, text_id);
    guide_label_plan_button_update(info, text_id == GUIDE_CUSTOM_TEXT_MODE_SELECT);
    guide_label_exit_update(info);
    info->label.force_update = false;
}
