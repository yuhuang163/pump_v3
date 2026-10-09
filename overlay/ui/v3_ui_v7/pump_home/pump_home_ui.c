/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep
#include "pump_home_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz)
{
    pump_home_base_ui_prepare(thiz);
}

void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz)
{
    pump_home_base_ui_remove(thiz);
}

int ui_display_pump_home_screen_process(ui_manager_context_t *thiz)
{
    return pump_home_base_ui_process(thiz);
}

void ui_display_pump_home_page_switch(pump_home_page_slide_dir_e dir)
{
    pump_home_base_ui_page_switch(dir);
}
