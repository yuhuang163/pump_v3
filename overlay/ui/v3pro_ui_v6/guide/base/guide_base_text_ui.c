/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static guide_text_info_t home_text_infos[GUIDE_TEXT_NUM] = {
    {"Operation Guide",
        &lv_font_aeonik_soft_20_medium, 0, 51, 4, GUIDE_TITLE_TEXT_COLOR, LV_OPA_COVER},
    {"Turn right to next page",
        &lv_font_aeonik_soft_16_medium, 0, 81, 4, GUIDE_DESC_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"Press down to exit", &lv_font_aeonik_soft_16_medium, 0,
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
};

const static guide_text_info_t start_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Press to enter/exit\nthe function",
        &lv_font_aeonik_soft_20_medium, 0, 51, 4, GUIDE_TITLE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_DESC_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"Press down to exit", &lv_font_aeonik_soft_16_medium, 0,
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
};

const static guide_text_info_t setting_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Turn to adjust the options\nPress down to confirm",
        &lv_font_aeonik_soft_20_medium, 0, 51, 4, GUIDE_TITLE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_DESC_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"Press down to exit", &lv_font_aeonik_soft_16_medium, 0,
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
};

const static guide_text_info_t stop_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Press", &lv_font_aeonik_soft_20_medium, 78, 51, 4, GUIDE_TITLE_TEXT_COLOR, LV_OPA_COVER},
    {"quick start", &lv_font_aeonik_soft_20_medium, 180, 51, 4, GUIDE_TITLE_TEXT_COLOR, LV_OPA_COVER},
    {"default program", &lv_font_aeonik_soft_20_medium, 0, 75, 4, GUIDE_TITLE_TEXT_COLOR, LV_OPA_COVER},
    {"Press down to exit", &lv_font_aeonik_soft_16_medium, 0,
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR, LV_OPA_COVER},
    {"Cozy Flow", &lv_font_aeonik_soft_16_medium, 0, 166, 4, 0xFFFFFF, LV_OPA_COVER},
    {"1.5 min Stimulation + 20\nmin Expression follow a\nbaby's natural nursing\nrhythm.",
        &lv_font_aeonik_soft_16_medium, 0, 187, 2, 0xFFFFFF, 230},
};

const static guide_text_info_t pump_mode_select_text_infos[GUIDE_TEXT_NUM] = {
    {"Press and hold for 3 seconds\nto stop pumping",
        &lv_font_aeonik_soft_20_medium, 0, 125, 4, 0xFFFFFF, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_DESC_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"Press down to exit", &lv_font_aeonik_soft_16_medium, 0,
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, 0x938383, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
};

const static guide_text_info_t bind_guide_text_infos[GUIDE_TEXT_NUM] = {
    {"Scan to connect for tracking, remote\ncontrol, and custom modes",
        &lv_font_aeonik_soft_16_medium, 0, 212, 4, GUIDE_BIND_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_BIND_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_BIND_TEXT_COLOR, LV_OPA_COVER},
    {"Press down to exit", &lv_font_aeonik_soft_16_medium, 0,
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_TITLE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
    {"", &lv_font_aeonik_soft_16_medium, 0, 0, 4, GUIDE_TEXT_COLOR, LV_OPA_COVER},
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static const guide_text_info_t *guide_base_text_infos_get(uint8_t page_index)
{
    if (page_index == GUIDE_PAGE_HOME) {
        return home_text_infos;
    }
    if (page_index == GUIDE_PAGE_START_PUMP) {
        return start_pump_text_infos;
    }
    if (page_index == GUIDE_PAGE_SETTING_PUMP) {
        return setting_pump_text_infos;
    }
    if (page_index == GUIDE_PAGE_STOP_PUMP) {
        return stop_pump_text_infos;
    }
    if (page_index == GUIDE_PAGE_PUMP_MODE_SELECT) {
        return pump_mode_select_text_infos;
    }
    return bind_guide_text_infos;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_base_text_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    guide_base_ui_info_t *guide_info = guide_base_info_get();
    const guide_text_info_t *guide_text_infos = guide_base_text_infos_get(page_index);

    EXCEPTION_CHECK_VOID(guide_info);
    (void)thiz;
    for (int i = 0; i < guide_info->text_num; i++) {
        if (i >= GUIDE_TEXT_NUM) {
            break;
        }

        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        }
        lv_label_set_text(guide_info->text[i], guide_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(guide_text_infos[i].color), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_text_infos[i].font, 0);
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0);
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_opa(guide_info->text[i], guide_text_infos[i].text_opa, 0);
        if (guide_text_infos[i].text_x == 0) {
            lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_MID, 0, guide_text_infos[i].text_y);
        } else {
            lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT,
                guide_text_infos[i].text_x, guide_text_infos[i].text_y);
        }
    }
}
