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
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_ble_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    res_theme_type_e theme_type = RES_THEME_TYPE_BASE;
    const popup_window_text_info_t *text_infos = NULL;
    uint16_t img_id = 0;
    uint16_t icon_y = 0;

#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (thiz != NULL && thiz->priv.res != NULL) {
        theme_type = thiz->priv.res->get_sys_theme_type();
    }
#endif

    const popup_window_text_info_t ble_connect_infos[POPUP_WINDOW_TEXT_COUNT] = {
        {BLE_CONNECT_TEXT, 0, POPUP_WINDOW_BLE_CONNECT_TEXT_Y,
            UI_FONT_TITLE_MD, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
    };
    const popup_window_text_info_t ble_disconnect_infos[POPUP_WINDOW_TEXT_COUNT] = {
        {BLE_DISCONNECT_TEXT, 0, POPUP_WINDOW_BLE_DISCONNECT_TEXT_Y,
            UI_FONT_TITLE_MD, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR,
            POPUP_WINDOW_BLE_DISCONNECT_LINE_SPACE},
    };
    const popup_window_text_info_t ble_connecting_infos[POPUP_WINDOW_TEXT_COUNT] = {
        {BLE_CONNECTING_TEXT, 0, POPUP_WINDOW_BLE_CONNECTING_TEXT_Y,
            UI_FONT_TITLE_MD, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
    };
    
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    const popup_window_text_info_t ble_custom_connect_infos[POPUP_WINDOW_TEXT_COUNT] = {
        {BLE_CONNECT_TEXT, 0, POPUP_WINDOW_BLE_CUSTOM_TEXT_Y,
            UI_FONT_TITLE_MD, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
    };
    const popup_window_text_info_t ble_custom_disconnect_infos[POPUP_WINDOW_TEXT_COUNT] = {
        {BLE_DISCONNECT_TEXT, 0, POPUP_WINDOW_BLE_CUSTOM_TEXT_Y,
            UI_FONT_TITLE_MD, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
    };
    const popup_window_text_info_t ble_custom_connecting_infos[POPUP_WINDOW_TEXT_COUNT] = {
        {BLE_CONNECTING_TEXT, 0, POPUP_WINDOW_BLE_CUSTOM_TEXT_Y,
            UI_FONT_TITLE_MD, LV_OPA_100, POPUP_WINDOW_BLE_TEXT_COLOR, 0},
    };
#endif

    switch (popup_type) {
        case ID_POPUP_UI_BT_CONNECT:
#if IS_SUPPORT_THEME_GUIDE_WINDOW
            if (theme_type == RES_THEME_TYPE_CUSTOM) {
                text_infos = ble_custom_connect_infos;
                img_id = CUSTOM_RES_ID_SUCCESS_IP_ICON;
                icon_y = POPUP_WINDOW_BLE_CUSTOM_ICON_Y;
                break;
            }
#endif
            text_infos = ble_connect_infos;
            img_id = BASE_RES_ID_BLE_CONNECTING_TIP_ICON;
            icon_y = POPUP_WINDOW_BLE_CONNECT_ICON_Y;
            break;
        case ID_POPUP_UI_BT_DISCONNECT:
#if IS_SUPPORT_THEME_GUIDE_WINDOW
            if (theme_type == RES_THEME_TYPE_CUSTOM) {
                text_infos = ble_custom_disconnect_infos;
                img_id = CUSTOM_RES_ID_WARNING_IP_ICON;
                icon_y = POPUP_WINDOW_BLE_CUSTOM_ICON_Y;
                break;
            }
#endif
            text_infos = ble_disconnect_infos;
            img_id = BASE_RES_ID_BLE_DISCONNECT_TIP_ICON;
            icon_y = POPUP_WINDOW_BLE_DISCONNECT_ICON_Y;
            break;
        case ID_POPUP_UI_BT_CONNECTING:
#if IS_SUPPORT_THEME_GUIDE_WINDOW
            if (theme_type == RES_THEME_TYPE_CUSTOM) {
                text_infos = ble_custom_connecting_infos;
                img_id = CUSTOM_RES_ID_CONNECTING_IP_ICON;
                icon_y = POPUP_WINDOW_BLE_CUSTOM_ICON_Y;
                break;
            }
#endif
            text_infos = ble_connecting_infos;
            img_id = BASE_RES_ID_BLE_CONNECTING_TIP_ICON;
            icon_y = POPUP_WINDOW_BLE_CONNECTING_ICON_Y;
            break;
        default:
            break;
    }

    if (text_infos == NULL) {
        return;
    }

    popup_window_tip_ui_update(thiz, popup_type, text_infos, img_id, icon_y, theme_type);
    popup_window_ble_mask_opaque();
}
