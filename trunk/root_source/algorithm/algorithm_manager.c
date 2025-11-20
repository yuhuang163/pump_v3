/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static algo_manager_context_t *algo_obj = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int algo_version_get(void)
{
    return 0;
}

static void algo_on_receive_internal(broadcast_mask_id_t id, void *context, int args1, int args2, broadcast_priv_data_t *pdata)
{
    algo_manager_context_t *ctx = context;
    if (ctx->hook && ctx->hook->on_algo_receiver_route) {
        ctx->hook->on_algo_receiver_route(id, context, args1, args2, pdata);
    }
}

static int on_perpare_algo_init(void *self)
{
    algo_manager_context_t *ctx = self;
    return 0;
}

static int on_post_algo_init(void *ptr)
{
    algo_manager_context_t *ctx = ptr;
    broadcast_obj_t *bdc = &ctx->priv.m_bcd;
    bdc->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    bdc->id.code |= ID_BROADCAST_SYS_BOOT_COMPLETED;
    bdc->id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    bdc->context = ctx;
    bdc->name = "algo_fusion";
    bdc->receiver_cb = algo_on_receive_internal;
    register_broadcast_receiver(bdc);
    /// 钩子参数初始化
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_algo_fusion_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, algo_manager_context_t);
    ptr->get_algo_version = algo_version_get;
    ptr->priv.algo_inited = 0;
    algo_obj = ptr;

    return ADD_MODULE("algo", ID_MODULE_ALGO_HUB, on_perpare_algo_init, 
        on_post_algo_init, ptr, list_head, sizeof(algo_manager_context_t));
}
 