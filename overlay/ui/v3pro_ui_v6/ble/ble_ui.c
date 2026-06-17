/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ble_ui_internal.h" // IWYU pragma: keep

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
    int (*update)(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, uint16_t x, uint16_t y);
} ble_ui_ops_t;

static ble_ui_ops_t s_ops = {0};

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
void ble_ui_init(ui_manager_context_t *thiz)
{
    int res_theme_type = RES_THEME_TYPE_BASE;

    if (thiz && thiz->priv.res) {
        res_theme_type = thiz->priv.res->get_current_theme_type();
    }

    if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
        s_ops.life.init = ble_custom_ui_init;
        s_ops.life.deinit = ble_custom_ui_deinit;
        s_ops.update = ble_custom_ui_update;
    } else {
        s_ops.life.init = ble_base_ui_init;
        s_ops.life.deinit = ble_base_ui_deinit;
        s_ops.update = ble_base_ui_update;
    }

    if (s_ops.life.init) {
        s_ops.life.init(thiz);
    }
}

int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, uint16_t x, uint16_t y)
{
    if (s_ops.update) {
        return s_ops.update(thiz, root, ble, x, y);
    }
    LOG_UI("ble update skipped: s_ops.update is null");
    return 0;
}

void ble_ui_deinit(ui_manager_context_t *thiz)
{
    if (s_ops.life.deinit) {
        s_ops.life.deinit(thiz);
    } else {
        LOGE("deinit is null");
    }
}
