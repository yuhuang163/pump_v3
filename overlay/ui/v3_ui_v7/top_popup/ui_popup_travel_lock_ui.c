/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define TRAVEL_LOCK_TITLE_COLOR                           (0xFFFFFF)
#define TRAVEL_LOCK_ARC_DONE_COLOR                        (0xFFFFFF)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static popup_window_text_info_t travel_lock_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {TRAVEL_LOCK_TITLE_TEXT, 0, TRAVEL_LOCK_TITLE_Y,
        UI_FONT_TITLE, LV_OPA_100, TRAVEL_LOCK_TITLE_COLOR,
        TRAVEL_LOCK_TITLE_LINE_SPACE},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_travel_lock_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type, uint32_t target_time)
{
    switch (popup_type) {
        case ID_POPUP_UI_TRAVEL_LOCK_COUNTDOWN:
            popup_window_progress_arc_countdown_ui_update(thiz, popup_type,
                travel_lock_infos, TRAVEL_LOCK_ARC_DONE_COLOR);
            break;
        default:
            break;
    }
}
