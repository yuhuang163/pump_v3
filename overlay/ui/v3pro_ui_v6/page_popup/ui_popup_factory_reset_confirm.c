/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_factory_reset_confirm_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    if (popup_type != ID_POPUP_UI_FACTORY_RESET_CONFIRM) {
        return;
    }
    // ui_popup_mask_layer_update(thiz);
    ui_popup_arc_update(thiz);
    // ui_popup_num_update(thiz, 0);
    ui_popup_display_complete(thiz, popup_type);
}
