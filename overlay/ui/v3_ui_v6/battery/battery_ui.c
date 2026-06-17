/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "battery_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct {
    ui_module_lifecycle_ops_t life;
    int (*update)(ui_manager_context_t *thiz, lv_obj_t *root, battery_manager_context_t *battery,
        uint8_t is_display_state);
} battery_ui_ops_t;

static battery_ui_ops_t s_ops = {0};


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
int battery_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, battery_manager_context_t *battery, uint8_t is_display_state)
{
    if (s_ops.update) {
        return s_ops.update(thiz, root, battery, is_display_state);
    }
    LOG_UI("battery update skipped: s_ops.update is null");
    return 0;
}

void battery_ui_init(ui_manager_context_t *thiz)
{
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_current_theme_type();
    }

    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        s_ops.life.init = battery_custom_ui_init;
        s_ops.life.deinit = battery_custom_ui_deinit;
        s_ops.update = battery_custom_ui_update;
    } else {
        s_ops.life.init = battery_base_ui_init;
        s_ops.life.deinit = battery_base_ui_deinit;
        s_ops.update = battery_base_ui_update;
    }

    if (s_ops.life.init) {
        s_ops.life.init(thiz);
    }
}

void battery_ui_deinit(ui_manager_context_t *thiz)
{
    if (s_ops.life.deinit) {
        s_ops.life.deinit(thiz);
    } else {
        LOGE("deinit is null");
    }
}