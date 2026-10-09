/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_finish_ui.h" // IWYU pragma: keep
#include "pump_finish_base_ui_internal.h" // IWYU pragma: keep
#include "pump_finish_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_complete_tip_prepare(ui_manager_context_t *thiz)
{
    pump_finish_base_ui_prepare(thiz);
}

void ui_display_pump_complete_tip_remove(ui_manager_context_t *thiz)
{
    pump_finish_base_ui_remove(thiz);
}

int ui_display_pump_complete_tip_process(ui_manager_context_t *thiz)
{
    return pump_finish_base_ui_process(thiz);
}