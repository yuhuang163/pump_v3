/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define SYS_ERROR_TITLE_Y                                 (192)
#define SYS_ERROR_TEXT_Y                                  (232)
#define TEMP_ERROR_TITLE_Y                                (192)
#define TEMP_ERROR_TEXT_Y                                 (232)
#define POPUP_WINDOW_ERROR_ICON_Y                         (110)

#define POPUP_WINDOW_ERROR_TITLE_COLOR                    (0xFFFFFF)
#define POPUP_WINDOW_ERROR_TEXT_COLOR                     (0xA9A7A6)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static popup_window_text_info_t sys_error_infos[POPUP_WINDOW_TEXT_COUNT] = {
    { "System error", 0, SYS_ERROR_TITLE_Y,
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, POPUP_WINDOW_ERROR_TITLE_COLOR, 0},
    { "Press and hold knob 10s\nto restart", 0, SYS_ERROR_TEXT_Y,
        &lv_font_aeonik_soft_20_medium, LV_OPA_100, POPUP_WINDOW_ERROR_TEXT_COLOR, 4},
};

const static popup_window_text_info_t temp_error_infos[POPUP_WINDOW_TEXT_COUNT] = {
    { "High battery temperature", 0, TEMP_ERROR_TITLE_Y,
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, POPUP_WINDOW_ERROR_TITLE_COLOR, 0},
    { "Device suspended", 0, TEMP_ERROR_TEXT_Y,
        &lv_font_aeonik_soft_20_medium, LV_OPA_100, POPUP_WINDOW_ERROR_TEXT_COLOR, 0},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_error_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_SYS_ERROR:
            popup_window_static_tip_ui_update(thiz, sys_error_infos,
                BASE_RES_ID_WARNING_ICON, POPUP_WINDOW_ERROR_ICON_Y);
            break;
        case ID_POPUP_UI_TEMP_ERROR:
            popup_window_static_tip_ui_update(thiz, temp_error_infos,
                BASE_RES_ID_WARNING_ICON, POPUP_WINDOW_ERROR_ICON_Y);
            break;
        default:
            break;
    }
}
