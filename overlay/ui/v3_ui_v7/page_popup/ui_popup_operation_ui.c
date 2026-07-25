/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define UI_POPUP_TEXT_COLOR                                        0xE6DFDF
#define UI_POPUP_TIP_ICON_Y                                        (130)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static ui_popup_text_info_t operation_success_infos[UI_POPUP_TEXT_COUNT] = {
    { "Success", 0, 212, &lv_font_aeonik_soft_20_medium, LV_OPA_100, UI_POPUP_TEXT_COLOR, 0},
};

const static ui_popup_text_info_t operation_failed_infos[UI_POPUP_TEXT_COUNT] = {
    { "Invalid operation", 0, 212, &lv_font_aeonik_soft_20_medium, LV_OPA_100, UI_POPUP_TEXT_COLOR, 0},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_operation_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_OP_SUCCESS:
            // ui_popup_tip_ui_update(thiz, popup_type, operation_success_infos,
            //     BASE_RES_ID_ALLOW_ICON, UI_POPUP_TIP_ICON_Y);
            break;
        case ID_POPUP_UI_OP_FAILED:
            // ui_popup_tip_ui_update(thiz, popup_type, operation_failed_infos,
            //     BASE_RES_ID_BAN_ICON, UI_POPUP_TIP_ICON_Y);
            break;
        default:
            break;
    }
}
