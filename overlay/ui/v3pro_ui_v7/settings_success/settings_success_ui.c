/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "settings_success_ui.h" // IWYU pragma: keep
#include "settings_success_base_ui_internal.h" // IWYU pragma: keep
#if IS_SUPPORT_THEME_GUIDE_WINDOW
#include "settings_success_custom_ui_internal.h" // IWYU pragma: keep
#endif

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
void ui_display_settings_success_prepare(ui_manager_context_t *thiz, uint8_t style)
{
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    void (*prepare)(ui_manager_context_t *thiz, uint8_t style) = NULL;
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_sys_theme_type();
    }
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        prepare = settings_success_custom_ui_prepare;
        s_ops.remove = settings_success_custom_ui_remove;
        s_ops.process = settings_success_custom_ui_process;
    } else {
        prepare = settings_success_base_ui_prepare;
        s_ops.remove = settings_success_base_ui_remove;
        s_ops.process = settings_success_base_ui_process;
    }
    if (prepare) {
        prepare(thiz, style);
    }
#else
    settings_success_base_ui_prepare(thiz, style);
#endif
}

void ui_display_settings_success_remove(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOGE("remove is null");
    }
#else
    settings_success_base_ui_remove(thiz);
#endif
}

int ui_display_settings_success_process(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (s_ops.process) {
        return s_ops.process(thiz);
    }
    return 0;
#else
    return settings_success_base_ui_process(thiz);
#endif
}
