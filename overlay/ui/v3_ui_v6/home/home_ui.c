/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "home_base_ui_internal.h" // IWYU pragma: keep
#include "home_custom_ui_internal.h" // IWYU pragma: keep
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
void ui_display_home_prepare(ui_manager_context_t *thiz)
{
    #if IS_SUPPORT_HOME_UI_TEST
    // ui_display_pump_complete_tip_prepare(thiz);
    popup_window_ui_prepare(thiz, HOME_POPUP_WINDOW_ID, 20 * 1000);
    // ui_display_power_off_prepare(thiz);
    // ui_display_factory_reset_prepare(thiz);
    // ui_display_factory_reset_confirm_prepare(thiz);
    // ui_display_airplane_mode_prepare(thiz);
    // ui_display_ota_prepare(thiz);
    // ui_display_charge_adapter_screen_prepare(thiz);
    // ui_display_home_screen_saver_prepare(thiz);
    // ui_display_theme_guide_prepare(thiz);
    #else
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
    #endif
}

void ui_display_home_remove(ui_manager_context_t *thiz)
{
    #if IS_SUPPORT_HOME_UI_TEST
    // ui_display_pump_complete_tip_remove(thiz);
    popup_window_ui_remove(thiz);
    // ui_display_power_off_remove(thiz);
    // ui_display_factory_reset_remove(thiz);
    // ui_display_factory_reset_confirm_remove(thiz);
    // ui_display_airplane_mode_remove(thiz);
    // ui_display_ota_remove(thiz);
    // ui_display_charge_adapter_screen_remove(thiz);
    // ui_display_home_screen_saver_remove(thiz);
    // ui_display_theme_guide_remove(thiz);
    #else
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOGE("remove is null");
    }
    #endif
}

int ui_display_home_process(ui_manager_context_t *thiz)
{
    #if IS_SUPPORT_HOME_UI_TEST
    // ui_display_pump_complete_tip_process(thiz);
    popup_window_ui_process(thiz, HOME_POPUP_WINDOW_ID);
    // ui_display_power_off_process(thiz);
    // ui_display_factory_reset_process(thiz);
    // ui_display_factory_reset_confirm_process(thiz, 0);
    // ui_display_airplane_mode_process(thiz);
    // ui_display_ota_process(thiz);
    // ui_display_charge_adapter_process(thiz);
    // ui_display_home_screen_saver_process(thiz);
    // ui_display_theme_guide_process(thiz);
    #else
    if (s_ops.process) {
        return s_ops.process(thiz);
    }
    #endif
    return 0;
}
