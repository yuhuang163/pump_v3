/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "charge_adapter_base_ui_internal.h" // IWYU pragma: keep
#include "charge_adapter_custom_ui_internal.h" // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_charge_adapter_screen_prepare(ui_manager_context_t *thiz)
{
    charge_adapter_base_ui_prepare(thiz);
}

void ui_display_charge_adapter_screen_remove(ui_manager_context_t *thiz)
{
    charge_adapter_base_ui_remove(thiz);
}

int ui_display_charge_adapter_process(ui_manager_context_t *thiz)
{
    return charge_adapter_base_ui_process(thiz);
}
