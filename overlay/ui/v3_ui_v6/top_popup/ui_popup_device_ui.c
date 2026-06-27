/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POPUP_WINDOW_DEVICE_TEXT_COLOR                             (0xFFFFFF)
#define POPUP_WINDOW_DEVICE_ICON_Y                                 (130)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static popup_window_text_info_t device_bind_infos[POPUP_WINDOW_TEXT_COUNT] = {
    { "Paired successfully", 0, 210, 
        &lv_font_aeonik_soft_16_medium, LV_OPA_100, POPUP_WINDOW_DEVICE_TEXT_COLOR, 0},
};

const static popup_window_text_info_t device_unbind_infos[POPUP_WINDOW_TEXT_COUNT] = {
    { "", 0, 0, 
        &lv_font_aeonik_soft_20_medium, LV_OPA_100, POPUP_WINDOW_DEVICE_TEXT_COLOR, 0},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_device_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_DEVICE_BIND:
            popup_window_tip_ui_update(thiz, popup_type, device_bind_infos,
                BASE_RES_ID_BIND_ICON, POPUP_WINDOW_DEVICE_ICON_Y);
            break;
        case ID_POPUP_UI_DEVICE_UNBIND:
            popup_window_tip_ui_update(thiz, popup_type, device_unbind_infos,
                BASE_RES_ID_UNBIND_ICON, POPUP_WINDOW_DEVICE_ICON_Y);
            break;
        default:
            break;
    }
}
