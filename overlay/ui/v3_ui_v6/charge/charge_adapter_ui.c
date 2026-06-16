/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "charge_adapter_ui_internal.h" // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

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
void ui_display_charge_adapter_screen_prepare(ui_manager_context_t *thiz)
{
    void (*prepare)(ui_manager_context_t *thiz) = NULL;
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_current_theme_type();
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
}

void ui_display_charge_adapter_screen_remove(ui_manager_context_t *thiz)
{
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOG_UI("charge_adapter remove fallback: s_ops.remove is null");
        charge_adapter_base_ui_remove(thiz);
        charge_adapter_custom_ui_remove(thiz);
    }
}

int ui_display_charge_adapter_process(ui_manager_context_t *thiz)
{
    if (s_ops.process) {
        return s_ops.process(thiz);
    }
    return 0;
}