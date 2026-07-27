/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_PAUSE_SETTLE_TITLE_Y                               (152)
#define PUMP_PAUSE_SETTLE_DESC_Y                                (190)
#define PUMP_PAUSE_SETTLE_TITLE_COLOR                           (0xFFFFFF)
#define PUMP_PAUSE_SETTLE_TEXT_COLOR                            (0xA9A7A6)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static popup_window_text_info_t pump_pause_settle_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {"Stop pumping...", 0, PUMP_PAUSE_SETTLE_TITLE_Y,
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, PUMP_PAUSE_SETTLE_TITLE_COLOR, 4},
    {"Inactive for 15 minutes", 0, PUMP_PAUSE_SETTLE_DESC_Y,
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, PUMP_PAUSE_SETTLE_TEXT_COLOR, 4},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_pump_stop_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_PUMP_PAUSE_SETTLE:
            popup_window_mask_layer_update(thiz);
            popup_window_text_update(thiz, pump_pause_settle_infos);
            popup_window_arc_update(thiz);
            popup_window_display_complete(thiz, popup_type);
            break;
        default:
            break;
    }
}
