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
#define POPUP_WINDOW_BLE_CONNECT_ICON_Y                         (109)
#define POPUP_WINDOW_BLE_CONNECT_TEXT_Y                         (215)
#define POPUP_WINDOW_BLE_DISCONNECT_ICON_Y                      (109)
#define POPUP_WINDOW_BLE_DISCONNECT_TEXT_Y                      (215)
#define POPUP_WINDOW_BLE_DISCONNECT_LINE_SPACE                  (8)
#define POPUP_WINDOW_BLE_CONNECTING_ICON_Y                      (109)
#define POPUP_WINDOW_BLE_CONNECTING_TEXT_Y                      (215)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void popup_window_ble_mask_opaque(void)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL || info->mask.layer == NULL || !lv_obj_is_valid(info->mask.layer)) {
        return;
    }
    lv_obj_set_style_bg_color(info->mask.layer, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(info->mask.layer, LV_OPA_COVER, 0);
}

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static popup_window_text_info_t ble_connect_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {"Device Connected", 0, POPUP_WINDOW_BLE_CONNECT_TEXT_Y,
        &lv_font_aeonik_soft_29_medium, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
};

const static popup_window_text_info_t ble_disconnect_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {"Device disconnected", 0, POPUP_WINDOW_BLE_DISCONNECT_TEXT_Y,
        &lv_font_aeonik_soft_29_medium, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR,
        POPUP_WINDOW_BLE_DISCONNECT_LINE_SPACE},
};

const static popup_window_text_info_t ble_connecting_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {"Device Connecting", 0, POPUP_WINDOW_BLE_CONNECTING_TEXT_Y,
        &lv_font_aeonik_soft_19_medium, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
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
                BASE_RES_ID_BLE_CONNECT_TIP_ICON, POPUP_WINDOW_BLE_CONNECT_ICON_Y);
            popup_window_ble_mask_opaque();
            break;
        case ID_POPUP_UI_BT_DISCONNECT:
            popup_window_tip_ui_update(thiz, popup_type, ble_disconnect_infos,
                BASE_RES_ID_BLE_DISCONNECT_TIP_ICON, POPUP_WINDOW_BLE_DISCONNECT_ICON_Y);
            popup_window_ble_mask_opaque();
            break;
        case ID_POPUP_UI_BT_CONNECTING:
            popup_window_tip_ui_update(thiz, popup_type, ble_connecting_infos,
                0, POPUP_WINDOW_BLE_CONNECTING_ICON_Y);
            popup_window_ble_mask_opaque();
            break;
        default:
            break;
    }
}
