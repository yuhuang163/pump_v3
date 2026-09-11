/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define COMMON_COUNT_DOWN_Y                               (146)
#define POWER_OFF_TEXT_COLOR                              (0xA9A7A6)
#define POWER_OFF_COUNT_DOWN_Y                            (166)

#define LOW_POWER_TEXT_COLOR                              (0xA9A7A6)
#define LOW_POWER_COUNT_DOWN_Y                            (166)

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_power_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    const popup_window_text_info_t low_power_infos[POPUP_WINDOW_TEXT_COUNT] = {
        {LOW_POWER_POPUP_TITLE_TEXT, 0, LOW_POWER_TITLE_Y,
            UI_FONT_HINT, LV_OPA_100, POWER_OFF_TEXT_COLOR, 0},
    };

    switch (popup_type) {
        case ID_POPUP_UI_LOW_POWER:
            popup_window_arc_countdown_ui_update(thiz, popup_type, low_power_infos, 0, LOW_POWER_COUNT_DOWN_Y);
            break;
        default:
            break;
    }
}
