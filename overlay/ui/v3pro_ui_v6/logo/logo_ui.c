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
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_page_ops_t s_ops = {0};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_logo_prepare(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_LOGO_UI_TEST
    s_ops.remove = logo_test_ui_remove;
    s_ops.process = logo_test_ui_process;
    logo_test_ui_prepare(thiz);
#else
    s_ops.remove = logo_display_ui_remove;
    s_ops.process = logo_display_ui_process;
    logo_display_ui_prepare(thiz);
#endif
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
