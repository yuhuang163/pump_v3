/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "home_base_ui_internal.h" // IWYU pragma: keep
#if IS_SUPPORT_THEME_GUIDE_WINDOW
#include "home_custom_ui_internal.h" // IWYU pragma: keep
#endif
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_UI_TEST                         (0)
#define HOME_POPUP_WINDOW_ID                            (ID_POPUP_UI_PUMP_FINISH_MANUAL)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_THEME_GUIDE_WINDOW
static ui_page_ops_t s_ops = {0};
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_prepare(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_HOME_UI_TEST
    ui_display_system_abnormal_prepare(thiz);
#else
    #if IS_SUPPORT_THEME_GUIDE_WINDOW
    void (*prepare)(ui_manager_context_t *thiz) = NULL;
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_sys_theme_type();
    }
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        prepare = home_custom_ui_prepare;
        s_ops.remove = home_custom_ui_remove;
        s_ops.process = home_custom_ui_process;
    } else {
        prepare = home_base_ui_prepare;
        s_ops.remove = home_base_ui_remove;
        s_ops.process = home_base_ui_process;
    }
    if (prepare) {
        prepare(thiz);
    }
    #else
    home_base_ui_prepare(thiz);
    #endif
#endif
}

void ui_display_home_remove(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_HOME_UI_TEST
    ui_display_system_abnormal_remove(thiz);
#else
    #if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOGE("remove is null");
    }
    #else
    home_base_ui_remove(thiz);
    #endif
#endif
}

int ui_display_home_process(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_HOME_UI_TEST
    ui_display_system_abnormal_process(thiz, SYS_ABNORMAL_COMMUNICATION_ERROR);
#else
    #if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (s_ops.process) {
        return s_ops.process(thiz);
    }
    #else
    return home_base_ui_process(thiz);
    #endif
#endif
    return 0;
}
