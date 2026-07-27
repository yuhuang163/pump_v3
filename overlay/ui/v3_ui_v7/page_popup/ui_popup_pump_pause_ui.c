/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_PAUSE_ICON_Y                                      (118)
#define PUMP_PAUSE_MANUAL_TITLE_Y                              (136)
#define PUMP_PAUSE_MANUAL_DESC_Y                               (174)
#define PUMP_PAUSE_MANUAL_TITLE_COLOR                          (0xFFFFFF)
#define PUMP_PAUSE_MANUAL_DESC_COLOR                           (0xA9A7A6)

#define PUMP_PAUSE_TIMEOUT_TITLE_Y                             (136)
#define PUMP_PAUSE_TIMEOUT_DESC_Y                              (174)
#define PUMP_PAUSE_TIMEOUT_TITLE_COLOR                         (0xFFFFFF)
#define PUMP_PAUSE_TIMEOUT_DESC_COLOR                          (0xA9A7A6)

#define PUMP_RUNING_30M_TIP_ICON_Y                             (112)
#define PUMP_RUNING_30M_TIP_TITLE_Y                            (194)
#define PUMP_RUNING_30M_TIP_TITLE_COLOR                        (0xFFFFFF)
#define PUMP_RUNING_30M_TIP_DESC_COLOR                         (0xA9A7A6)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static ui_popup_text_info_t pump_pause_manual_infos[UI_POPUP_TEXT_COUNT] = {
    { "Pumping paused", 0, PUMP_PAUSE_MANUAL_TITLE_Y, 
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, PUMP_PAUSE_MANUAL_TITLE_COLOR, 4},
    { "Auto shutoff after 15\nminutes of inactivity", 0, PUMP_PAUSE_MANUAL_DESC_Y, 
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, PUMP_PAUSE_MANUAL_DESC_COLOR, 4},
};

const static ui_popup_text_info_t pump_pause_timeout_infos[UI_POPUP_TEXT_COUNT] = {
    { "Pumping paused", 0, PUMP_PAUSE_TIMEOUT_TITLE_Y, 
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, PUMP_PAUSE_TIMEOUT_TITLE_COLOR, 4},
    { "Auto shutoff after 15\nminutes of inactivity", 0, PUMP_PAUSE_TIMEOUT_DESC_Y, 
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, PUMP_PAUSE_TIMEOUT_DESC_COLOR, 4},
};

const static ui_popup_text_info_t pump_runing_30m_tip_infos[UI_POPUP_TEXT_COUNT] = {
    { "Pumping over 30 minutes\nplease take a short break", 0, PUMP_RUNING_30M_TIP_TITLE_Y, 
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, PUMP_RUNING_30M_TIP_TITLE_COLOR, 4},
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_popup_pump_pause_manual_update(ui_manager_context_t *thiz)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_pause_manual_infos);
}

static void ui_popup_pump_pause_timeout_update(ui_manager_context_t *thiz)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_pause_timeout_infos);
}

static void ui_popup_pump_runing_30m_tip_update(ui_manager_context_t *thiz)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_runing_30m_tip_infos);
    ui_popup_single_icon_update(thiz, UI_IMG_RES_ID_NONE, PUMP_RUNING_30M_TIP_ICON_Y);
    ui_popup_display_complete(thiz, ID_POPUP_UI_PUMP_RUNING_30M_TIP);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_pump_pause_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_PUMP_PAUSE_MANUAL:
            ui_popup_pump_pause_manual_update(thiz);
            break;
        case ID_POPUP_UI_PUMP_PAUSE_TIMEOUT:
            ui_popup_pump_pause_timeout_update(thiz);
            break;
        case ID_POPUP_UI_PUMP_RUNING_30M_TIP:
            ui_popup_pump_runing_30m_tip_update(thiz);
            break;
        default:
            break;
    }
}
