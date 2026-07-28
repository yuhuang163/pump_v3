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
#if IS_SUPPORT_THEME_SWITCH
static ui_page_ops_t s_ops = {0};
#endif

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
#if IS_SUPPORT_THEME_SWITCH
    void (*prepare)(ui_manager_context_t *thiz) = NULL;
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_sys_theme_type();
    }
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        prepare = home_screen_saver_custom_ui_prepare;
        s_ops.remove = home_screen_saver_custom_ui_remove;
        s_ops.process = home_screen_saver_custom_ui_process;
    } else {
        prepare = home_screen_saver_base_ui_prepare;
        s_ops.remove = home_screen_saver_base_ui_remove;
        s_ops.process = home_screen_saver_base_ui_process;
    }

    if (prepare) {
        prepare(thiz);
    }
#else
    home_screen_saver_base_ui_prepare(thiz);
#endif
}

void ui_display_home_screen_saver_remove(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_SWITCH
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOGE("remove is null");
    }
#else
    home_screen_saver_base_ui_remove(thiz);
#endif
}

int ui_display_home_screen_saver_process(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_SWITCH
    if (s_ops.process) {
        return s_ops.process(thiz);
    } else {
        LOGE("process is null");
        return 0;
    }
#else
    return home_screen_saver_base_ui_process(thiz);
#endif
}
