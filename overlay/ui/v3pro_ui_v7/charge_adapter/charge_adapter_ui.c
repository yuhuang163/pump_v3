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

#if IS_SUPPORT_THEME_SWITCH
static ui_page_ops_t s_ops = {0};
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_charge_adapter_screen_prepare(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_SWITCH
    void (*prepare)(ui_manager_context_t *thiz) = NULL;
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_sys_theme_type();
    }
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        prepare = charge_adapter_custom_ui_prepare;
        s_ops.remove = charge_adapter_custom_ui_remove;
        s_ops.process = charge_adapter_custom_ui_process;
    } else {
        prepare = charge_adapter_base_ui_prepare;
        s_ops.remove = charge_adapter_base_ui_remove;
        s_ops.process = charge_adapter_base_ui_process;
    }

    if (prepare) {
        prepare(thiz);
    }
#else
    charge_adapter_base_ui_prepare(thiz);
#endif
}

void ui_display_charge_adapter_screen_remove(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_SWITCH
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOGE("remove is null");
    }
#else   
    charge_adapter_base_ui_remove(thiz);
#endif
}

int ui_display_charge_adapter_process(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_SWITCH
    if (s_ops.process) {
        return s_ops.process(thiz);
    } else {
        LOGE("process is null");
    }
#else
    return charge_adapter_base_ui_process(thiz);
#endif
    return 0;
}
