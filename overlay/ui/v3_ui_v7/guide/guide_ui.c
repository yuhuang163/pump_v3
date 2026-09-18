/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_guide_prepare(ui_manager_context_t *thiz)
{
    /// 引导页不区分主题，统一走 base UI
    guide_base_ui_prepare(thiz);
}

void ui_display_guide_remove(ui_manager_context_t *thiz)
{
    guide_base_ui_remove(thiz);
}

int ui_display_guide_process(ui_manager_context_t *thiz)
{
    return guide_base_ui_process(thiz);
}
