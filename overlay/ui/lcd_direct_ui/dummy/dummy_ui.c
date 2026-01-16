/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_VAR_TEST                0

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static lv_obj_t *dummy_root;

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
void ui_display_dummy_prepare(ui_manager_context_t *thiz)
{
    LVGL_OBJ_CLEAN();
    LOG_UI("ui_display_dummy_prepare");
}

void ui_display_dummy_remove(ui_manager_context_t *thiz)
{
    LVGL_OBJ_DELETE(dummy_root);
}

int ui_display_dummy_process(ui_manager_context_t *thiz)
{
    return 0;
}