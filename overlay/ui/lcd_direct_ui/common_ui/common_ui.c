/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep

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
void common_ui_img_resource_init(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.res);
    EXCEPTION_CHECK_VOID(info);
    int ret = thiz->priv.res->res_ui_img_data_load(id, info);
    if (ret != 0) {
        LOGE("===> load img data failed, id=%u ret=%d", (unsigned)id, ret);
    }
}

void common_ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.res);
    EXCEPTION_CHECK_VOID(info);
    thiz->priv.res->res_ui_img_data_unload(info);
}
