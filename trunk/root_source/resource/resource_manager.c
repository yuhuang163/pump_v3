/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "resource_manager.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep

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
static void hdr_async_init_cb(hdr_post_args_t *args)
{

}
 
static int wait_init_completed(void *res)
{
    resource_manager_context_t *ctx = res;

    if (ctx->priv.res_init_completed) {
        return 0;
    }

    if (ctx->priv.sem_init) {
        LOG_RES("U_SEM_WAIT...");
        U_SEM_WAIT(ctx->priv.sem_init, 2000);
        LOG_RES("U_SEM_WAIT exit.");
    }

    ctx->priv.res_init_completed = true;

    return 0;
}

static int on_post_res_init(void *ptr) 
{
    resource_manager_context_t *ctx = ptr;

    U_SEM_INIT(&ctx->priv.sem_init, 1);
    POST_TASK_HANDLER_NO_DELAY(hdr_async_init_cb, ptr, "async_init:res");
    return 0;
}

static int on_perpare_res_init(void *self)
{
    return 0;
}

static int uninstall_resource(int which)
{
    return 0;
}
 
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
resource_manager_context_t *refs_res_manager_obj(void)
{
    return thiz;
}

/// 资源管理实例接口.
void *register_module_for_res_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, resource_manager_context_t);

    ptr->uninstall_resource = uninstall_resource;
    ptr->wait_init_completed = wait_init_completed;
    MUTEX_INIT(&ptr->priv.m_fs_rw_lock, fs_rw_lock);
    thiz = ptr;

    return ADD_MODULE("res", ID_MODULE_RES, on_perpare_res_init, on_post_res_init, 
        ptr, list_head, sizeof(resource_manager_context_t));
}

