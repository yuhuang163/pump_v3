/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POPUP_WINDOW_BLE_TEXT_COLOR                             (0xFFFFFF)
#define POPUP_WINDOW_BLE_ICON_Y                                 (126)
#define POPUP_WINDOW_BLE_TEXT_Y                                 (210)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static popup_window_text_info_t ble_connect_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {"Device connected", 0, POPUP_WINDOW_BLE_TEXT_Y, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
};

const static popup_window_text_info_t ble_disconnect_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {"Device disconnected", 0, POPUP_WINDOW_BLE_TEXT_Y, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
};

const static popup_window_text_info_t ble_connecting_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {"Connecting", 0, POPUP_WINDOW_BLE_TEXT_Y, 
        &lv_font_aeonik_soft_24_medium, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_ble_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_BT_CONNECT:
            popup_window_tip_ui_update(thiz, popup_type, ble_connect_infos,
                BASE_RES_ID_BLE_CONNECT_POPUP_ICON, POPUP_WINDOW_BLE_ICON_Y);
            break;
        case ID_POPUP_UI_BT_DISCONNECT:
            popup_window_tip_ui_update(thiz, popup_type, ble_disconnect_infos,
                BASE_RES_ID_BLE_DISCONNECT_POPUP_ICON, POPUP_WINDOW_BLE_ICON_Y);
            break;
        case ID_POPUP_UI_BT_CONNECTING:
            popup_window_tip_ui_update(thiz, popup_type, ble_connecting_infos,
                BASE_RES_ID_BLE_DISCONNECT_POPUP_ICON, POPUP_WINDOW_BLE_ICON_Y);
            break;
        default:
            break;
    }
}
