/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_resource.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
void ui_theme_img_resource_init(ui_manager_context_t *thiz, uint16_t id,
    res_theme_type_e theme_type, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz && thiz->priv.res && info);
    if (info->data != NULL && info->id != id) {
        lv_draw_wait_for_finish();
        ui_img_resource_deinit(thiz, info);
    }

    if (theme_type >= RES_THEME_TYPE_MAX) {
        LOGE("===> invalid theme type, theme=%d id=%u", (int)theme_type, (unsigned)id);
        return;
    }

    if (theme_type == RES_THEME_TYPE_NONE) {
        theme_type = RES_THEME_TYPE_BASE;
    }

    int ret = thiz->priv.res->res_data_load_by_theme(theme_type, id, info);

    if (ret != 0) {
        LOGE("===> load img data failed, theme=%d id=%u ret=%d",
            (int)theme_type, (unsigned)id, ret);
    }
}

void ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz && thiz->priv.res && info);
    if (info->data != NULL) {
        lv_draw_wait_for_finish();
    }
    thiz->priv.res->res_data_unload(info);
}
