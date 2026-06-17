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
static ui_page_ops_t s_ops = {0};

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
    void (*prepare)(ui_manager_context_t *thiz) = NULL;
    int res_theme_type = RES_THEME_TYPE_BASE;

#if IS_SUPPORT_LOGO_UI_TEST
    prepare = logo_test_ui_prepare;
    s_ops.remove = logo_test_ui_remove;
    s_ops.process = logo_test_ui_process;
#else
    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_current_theme_type();
    }

    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        prepare = logo_custom_ui_prepare;
        s_ops.remove = logo_custom_ui_remove;
        s_ops.process = logo_custom_ui_process;
    } else {
        prepare = logo_base_ui_prepare;
        s_ops.remove = logo_base_ui_remove;
        s_ops.process = logo_base_ui_process;
    }
#endif

    if (prepare) {
        prepare(thiz);
    }
}

void ui_display_logo_remove(ui_manager_context_t *thiz)
{
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOGE("remove is null");
    }
}

int ui_display_logo_process(ui_manager_context_t *thiz)
{
    if (s_ops.process) {
        return s_ops.process(thiz);
    }
    return 0;
}
