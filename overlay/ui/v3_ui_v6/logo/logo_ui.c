/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "logo_ui_internal.h" // IWYU pragma: keep
#include "window_screen_logo.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static int res_theme_type = 0;


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
void ui_display_logo_prepare(ui_manager_context_t *thiz)
{
    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_current_theme_type();
    } else {
        res_theme_type = RES_THEME_TYPE_BASE;
    }
#if IS_SUPPORT_LOGO_UI_TEST
    logo_test_ui_prepare(thiz);
#else
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        logo_custom_ui_prepare(thiz);
    } else {
        logo_base_ui_prepare(thiz);
    }
#endif
}

void ui_display_logo_remove(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_LOGO_UI_TEST
    logo_test_ui_remove(thiz);
#else
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        logo_custom_ui_remove(thiz);
    } else {
        logo_base_ui_remove(thiz);
    }
#endif
}

int ui_display_logo_process(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_LOGO_UI_TEST
    return logo_test_ui_process(thiz);
#else
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        return logo_custom_ui_process(thiz);
    } else {
        return logo_base_ui_process(thiz);
    }
#endif
}
