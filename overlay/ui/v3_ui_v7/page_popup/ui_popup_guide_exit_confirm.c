/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_EXIT_CONFIRM_TITLE_Y                              (106)
#define GUIDE_EXIT_CONFIRM_RETURN_BUTTON_Y                      (192)
#define GUIDE_EXIT_CONFIRM_TITLE_COLOR                          (0xFFFFFF)
#define GUIDE_EXIT_CONFIRM_TEXT_COLOR                           (0xA9A7A6)

#define GUIDE_EXIT_COUNTDOWN_TITLE_Y                            (112)
#define GUIDE_EXIT_COUNTDOWN_NUM_Y                              (170)
#define GUIDE_EXIT_COUNTDOWN_TITLE_COLOR                        (0xA9A7A6)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static ui_popup_text_info_t guide_exit_confirm_infos[UI_POPUP_TEXT_COUNT] = {
    {"Exit guidance ?", 0, GUIDE_EXIT_CONFIRM_TITLE_Y,
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, GUIDE_EXIT_CONFIRM_TITLE_COLOR, 4},
    {"Press and hold for 3s", 0, GUIDE_EXIT_CONFIRM_TITLE_Y + 38,
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, GUIDE_EXIT_CONFIRM_TEXT_COLOR, 4},
};

const static ui_popup_text_info_t guide_exit_countdown_infos[UI_POPUP_TEXT_COUNT] = {
    {"Exit guidance", 0, GUIDE_EXIT_COUNTDOWN_TITLE_Y,
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, GUIDE_EXIT_COUNTDOWN_TITLE_COLOR, 4},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_guide_exit_confirm_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    if (popup_type != ID_POPUP_UI_GUIDE_EXIT_CONFIRM) {
        return;
    }
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, guide_exit_confirm_infos);
    ui_popup_icon_update(thiz, 0, UI_IMG_RES_ID_NONE, 0, GUIDE_EXIT_CONFIRM_RETURN_BUTTON_Y);
    ui_popup_display_complete(thiz, popup_type);
}

void ui_popup_guide_exit_countdown_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    if (popup_type != ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN) {
        return;
    }
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, guide_exit_countdown_infos);
    ui_popup_arc_update(thiz);
    ui_popup_num_update(thiz, GUIDE_EXIT_COUNTDOWN_NUM_Y);
    ui_popup_display_complete(thiz, popup_type);
}
