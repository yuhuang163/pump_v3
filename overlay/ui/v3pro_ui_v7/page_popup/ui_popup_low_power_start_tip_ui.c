/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

#if IS_SUPPORT_LOW_POWER_FORBID_START_TIP

/*
* DEFINES
*****************************************************************************************
*/
#define LOW_POWER_START_TIP_ICON_Y                         (95)
#define LOW_POWER_START_TIP_TITLE_Y                        (165)
#define LOW_POWER_START_TIP_TITLE_COLOR                    (0xFFFFFF)
#define LOW_POWER_START_TIP_TITLE_LINE_SPACE               (8)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static ui_popup_text_info_t low_power_start_tip_infos[UI_POPUP_TEXT_COUNT] = {
    { "Low battery, unable to\nstart milk suction, please\ncharge promptly.", 0, LOW_POWER_START_TIP_TITLE_Y,
        &lv_font_aeonik_soft_26_medium, LV_OPA_100, LOW_POWER_START_TIP_TITLE_COLOR,
        LOW_POWER_START_TIP_TITLE_LINE_SPACE},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_low_power_start_tip_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    if (popup_type != ID_POPUP_UI_LOW_POWER_START_TIP) {
        return;
    }

    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, low_power_start_tip_infos);
    ui_popup_single_icon_update(thiz, BASE_RES_ID_FORBIDDEN_TIP_ICON, LOW_POWER_START_TIP_ICON_Y);
}

#endif /* IS_SUPPORT_LOW_POWER_FORBID_START_TIP */
