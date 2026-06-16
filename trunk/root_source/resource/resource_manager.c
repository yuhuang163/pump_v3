/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "resource_manager_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "resource_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static resource_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void res_data_unload(res_img_info_t *img)
{
    EXCEPTION_CHECK_VOID(img);
#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_EXTERNAL_FLASH
    if (img->data) {
        (void)u_free_psram(img->data, RES_PSRAM_TYPE);
        img->data = NULL;
    }
#endif
    memset(img, 0, sizeof(*img));
}

static void on_res_init_cb(hdr_post_args_t *args)
{
    EXCEPTION_CHECK_VOID(thiz);
    int ret = img_res_init(thiz);
    if (!thiz->priv.res_init_completed) {
        thiz->priv.res_init_completed = true;
    }
}

static int wait_init_completed(void *res)
{
    resource_manager_context_t *ctx = res;    
    uint32_t timeout = 0;
    while (timeout < 200) {
        timeout += 10;
        if (ctx->priv.res_init_completed) {
            return 0;
        }
        msleep(10);
    }
    LOGE("wait res init timeout (timeout=%u)", timeout);
    return -1;
}

static int install_resource(int which)
{
    if (which == RES_THEME_TYPE_CUSTOM) {
        return install_img_res(thiz, RES_THEME_TYPE_CUSTOM);
    } 
    return 0;
}

static int uninstall_resource(int which)
{
    if (which == RES_THEME_TYPE_CUSTOM) {
        return uninstall_img_res(thiz, RES_THEME_TYPE_CUSTOM);
    }
    return 0;
}

static int base_res_data_load_impl(uint16_t img_id, res_img_info_t *out)
{
    return res_data_load_with_theme(thiz, RES_THEME_TYPE_BASE, img_id, out);
}

static int theme_res_data_load_impl(uint16_t img_id, res_img_info_t *out)
{
#if IS_SUPPORT_UI_THEME_RES
    return res_data_load_with_theme(thiz, thiz->priv.theme_type, img_id, out);
#else
    return res_data_load_with_theme(thiz, RES_THEME_TYPE_BASE, img_id, out);
#endif 
}

static int get_img_res_version(res_theme_type_e theme_type)
{
    if (theme_type == RES_THEME_TYPE_BASE) {
        return thiz->priv.img_base_res_version;
    } else if (theme_type == RES_THEME_TYPE_CUSTOM) {
        return thiz->priv.img_custom_res_version;
    }
    return 0;
}

static int get_current_theme_type(void)
{
    thiz->priv.theme_type = RES_THEME_TYPE_CUSTOM;
    return thiz->priv.theme_type;
}

static int on_post_res_init(void *ptr) 
{
    resource_manager_context_t *ctx = ptr;
    ctx->priv.res_init_completed = false;
    POST_TASK_HANDLER_NO_DELAY(on_res_init_cb, ptr, "res_init");

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_res_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, resource_manager_context_t);

    ptr->uninstall_resource = uninstall_resource;
    ptr->wait_init_completed = wait_init_completed;
    ptr->base_res_data_load = base_res_data_load_impl;
    ptr->theme_res_data_load = theme_res_data_load_impl;
    ptr->res_data_unload = res_data_unload;
    ptr->get_img_res_version = get_img_res_version;
    ptr->get_current_theme_type = get_current_theme_type;
    MUTEX_INIT(&ptr->priv.m_fs_rw_lock, fs_rw_lock);
    thiz = ptr;

    return ADD_MODULE("res", ID_MODULE_RES, NULL, on_post_res_init, 
        ptr, list_head, sizeof(resource_manager_context_t));
}

