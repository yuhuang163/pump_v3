/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep
#if IS_SUPPORT_THEME_GUIDE_WINDOW
#include "pump_home_custom_ui_internal.h" // IWYU pragma: keep
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_THEME_GUIDE_WINDOW
static ui_page_ops_t s_ops = {0};
static void (*s_page_switch)(pump_home_page_slide_dir_e dir);
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    void (*prepare)(ui_manager_context_t *thiz) = NULL;
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_sys_theme_type();
    }
    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        prepare = pump_home_custom_ui_prepare;
        s_ops.remove = pump_home_custom_ui_remove;
        s_ops.process = pump_home_custom_ui_process;
        s_page_switch = pump_home_custom_ui_page_switch;
    } else {
        prepare = pump_home_base_ui_prepare;
        s_ops.remove = pump_home_base_ui_remove;
        s_ops.process = pump_home_base_ui_process;
        s_page_switch = pump_home_base_ui_page_switch;
    }

    if (prepare) {
        prepare(thiz);
    }
#else
    pump_home_base_ui_prepare(thiz);
#endif
}

void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (s_ops.remove) {
        s_ops.remove(thiz);
    } else {
        LOGE("remove is null");
    }
    s_page_switch = NULL;
#else
    pump_home_base_ui_remove(thiz);
#endif
}

int ui_display_pump_home_screen_process(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (s_ops.process) {
        return s_ops.process(thiz);
    }
    return 0;
#else
    return pump_home_base_ui_process(thiz);
#endif
}

void ui_display_pump_home_page_switch(pump_home_page_slide_dir_e dir)
{
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (s_page_switch) {
        s_page_switch(dir);
    }
#else
    pump_home_base_ui_page_switch(dir);
#endif
}
