/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "home_screen_saver_base_ui_internal.h" // IWYU pragma: keep
#include "home_screen_saver_custom_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

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
void ui_display_home_screen_saver_prepare(ui_manager_context_t *thiz)
{
    home_screen_saver_base_ui_prepare(thiz);
}

void ui_display_home_screen_saver_remove(ui_manager_context_t *thiz)
{
    home_screen_saver_base_ui_remove(thiz);
}

int ui_display_home_screen_saver_process(ui_manager_context_t *thiz)
{
    return home_screen_saver_base_ui_process(thiz);
}
