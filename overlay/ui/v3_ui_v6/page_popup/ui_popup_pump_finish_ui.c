/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define FINISH_MANUAL_TITLE_Y                                   (108)
#define FINISH_MANUAL_RETURN_BUTTON_Y                           (192)
#define FINISH_MANUAL_TITLE_COLOR                               (0xFFFFFF)
#define FINISH_MANUAL_TEXT_COLOR                                (0xA9A7A6)

#define WILL_FINISH_TIP_TITLE_Y                                 (108)
#define WILL_FINISH_TIP_CONTINUE_BUTTON_Y                       (192)
#define WILL_FINISH_TIP_TITLE_COLOR                             (0xFFFFFF)
#define WILL_FINISH_TIP_TEXT_COLOR                              (0xA9A7A6)

#define FINISH_TIMEOUT_TITLE_Y                                  (150)
#define FINISH_TIMEOUT_TITLE_COLOR                              (0xFFFFFF)
#define FINISH_TIMEOUT_TEXT_COLOR                               (0xA9A7A6)

#define FINISH_CHARGE_ERROR_ICON_Y                              (110)
#define FINISH_CHARGE_ERROR_TITLE_Y                             (150)
#define FINISH_CHARGE_ERROR_TITLE_COLOR                         (0xFFFFFF)
#define FINISH_CHARGE_ERROR_TEXT_COLOR                          (0xA9A7A6)

#define FINISH_LOW_POWER_ICON_Y                                 (110)
#define FINISH_LOW_POWER_TITLE_Y                                (159)
#define FINISH_LOW_POWER_TITLE_COLOR                            (0xFFFFFF)
#define FINISH_LOW_POWER_TEXT_COLOR                             (0xA9A7A6)

#define PUMP_PAUSE_SETTLE_TITLE_Y                               (152)
#define PUMP_PAUSE_SETTLE_DESC_Y                                (190)
#define PUMP_PAUSE_SETTLE_TITLE_COLOR                           (0xFFFFFF)
#define PUMP_PAUSE_SETTLE_TEXT_COLOR                            (0xA9A7A6)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static ui_popup_text_info_t pump_finish_manual_infos[UI_POPUP_TEXT_COUNT] = {
    {"Finish pumping?", 0, FINISH_MANUAL_TITLE_Y, 
        &lv_font_aeonik_soft_28_medium, LV_OPA_100, FINISH_MANUAL_TITLE_COLOR, 4},
    {"Press & hold 3s", 0, FINISH_MANUAL_TITLE_Y + 38, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, FINISH_MANUAL_TEXT_COLOR, 4},
};

const static ui_popup_text_info_t pump_will_finish_infos[UI_POPUP_TEXT_COUNT] = {
    {"Program ending soon", 0, WILL_FINISH_TIP_TITLE_Y, 
        &lv_font_aeonik_soft_28_medium, LV_OPA_100, WILL_FINISH_TIP_TITLE_COLOR, 10},
    {"Continue pumping?", 0, WILL_FINISH_TIP_TITLE_Y + 38, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, WILL_FINISH_TIP_TEXT_COLOR, 4},
};

const static ui_popup_text_info_t pump_finish_timeout_infos[UI_POPUP_TEXT_COUNT] = {
    {"Pumping Timeout", 0, FINISH_TIMEOUT_TITLE_Y, 
        &lv_font_aeonik_soft_28_medium, LV_OPA_100, FINISH_TIMEOUT_TITLE_COLOR, 4},
    {"Auto-stoping...", 0, FINISH_TIMEOUT_TITLE_Y + 38, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, FINISH_TIMEOUT_TEXT_COLOR, 4},
};

const static ui_popup_text_info_t pump_finish_charge_error_infos[UI_POPUP_TEXT_COUNT] = {
    {"Incompatible adapter", 0, FINISH_CHARGE_ERROR_TITLE_Y, 
        &lv_font_aeonik_soft_28_medium, LV_OPA_100, FINISH_CHARGE_ERROR_TITLE_COLOR, 4},
    {"Auto-stoping...", 0, FINISH_CHARGE_ERROR_TITLE_Y + 38, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, FINISH_CHARGE_ERROR_TEXT_COLOR, 4},
};

const static ui_popup_text_info_t pump_finish_low_power_infos[UI_POPUP_TEXT_COUNT] = {
    {"Low power", 0, FINISH_LOW_POWER_TITLE_Y, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, FINISH_LOW_POWER_TITLE_COLOR, 4},
    {"Ending automatically", 0, FINISH_LOW_POWER_TITLE_Y + 36, 
        &lv_font_aeonik_soft_20_medium, LV_OPA_100, FINISH_LOW_POWER_TEXT_COLOR, 4},
};

const static ui_popup_text_info_t pump_pause_settle_infos[UI_POPUP_TEXT_COUNT] = {
    {"Stop pumping...", 0, PUMP_PAUSE_SETTLE_TITLE_Y, 
        &lv_font_aeonik_soft_28_medium, LV_OPA_100, PUMP_PAUSE_SETTLE_TITLE_COLOR, 4},
    {"Inactive for 15 minutes", 0, PUMP_PAUSE_SETTLE_DESC_Y, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, PUMP_PAUSE_SETTLE_TEXT_COLOR, 4},
};  

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_popup_pump_finish_manual_update(ui_manager_context_t *thiz)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_finish_manual_infos);
    ui_popup_icon_update(thiz, 0, BASE_RES_ID_RETURN_BUTTON_ICON, 0, FINISH_MANUAL_RETURN_BUTTON_Y);
    ui_popup_display_complete(thiz, ID_POPUP_UI_PUMP_FINISH_MANUAL);
}

static void ui_popup_pump_will_finish_update(ui_manager_context_t *thiz)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_will_finish_infos);
    ui_popup_icon_update(thiz, 0, BASE_RES_ID_START_BUTTON_ICON, 0, WILL_FINISH_TIP_CONTINUE_BUTTON_Y);
    ui_popup_arc_update(thiz);
    ui_popup_display_complete(thiz, ID_POPUP_UI_PUMP_WILL_FINISH_TIP);
}

static void ui_popup_pump_finish_timeout_update(ui_manager_context_t *thiz)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_finish_timeout_infos);
    ui_popup_arc_update(thiz);
    ui_popup_display_complete(thiz, ID_POPUP_UI_PUMP_FINISH_TIMEOUT);
}

static void ui_popup_pump_finish_charge_error_update(ui_manager_context_t *thiz,
    popup_ui_id_e popup_type)
{
    ui_popup_mask_layer_update(thiz);
    // ui_popup_single_icon_update(thiz, BASE_RES_ID_BAN_ICON, FINISH_CHARGE_ERROR_ICON_Y);
    ui_popup_text_update(thiz, pump_finish_charge_error_infos);
    ui_popup_arc_update(thiz);
    ui_popup_display_complete(thiz, popup_type);
}

static void ui_popup_pump_pause_settle_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_pause_settle_infos);
    ui_popup_arc_update(thiz);
    ui_popup_display_complete(thiz, popup_type);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_pump_finish_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
            ui_popup_pump_finish_manual_update(thiz);
            break;
        case ID_POPUP_UI_PUMP_WILL_FINISH_TIP:
            ui_popup_pump_will_finish_update(thiz);
            break;
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
            ui_popup_pump_finish_timeout_update(thiz);
            break;
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
            ui_popup_pump_finish_charge_error_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
            ui_popup_arc_countdown_update(thiz, popup_type, pump_finish_low_power_infos);
            break;
        case ID_POPUP_UI_PUMP_PAUSE_SETTLE:
            ui_popup_pump_pause_settle_update(thiz, popup_type);
            break;
        default:
            break;
    }
}
