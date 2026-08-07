/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "windows_manager.h" // IWYU pragma: keep
#include "servers_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static servers_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONSre
*****************************************************************************************
*/
static void on_servers_manager_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    servers_manager_context_t *ctx = context;
    switch (id.code) {
        case ID_BROADCAST_BLE_STATE_CHANGE:
            break;
        default:
            break;
    }

    if (ctx->hook && ctx->hook->on_servers_manager_receiver_route) {
        ctx->hook->on_servers_manager_receiver_route(id, context, arg1, arg2, pdata);
    }
}

static int dispatch_servers_event_wrapper(servers_manager_context_t *ctx, servers_publish_data_id_e msg_id)
{
    EXCEPTION_CHECK_INT(ctx && ctx->priv.servers_thd.msg_id);
    return MSG_QUEUE_SEND(ctx->priv.servers_thd.msg_id, &msg_id, 100);
}

static void servers_work_thread(const void *args)
{
    servers_publish_data_id_e msg = 0;
    thread_context_t *thd = (thread_context_t *)args;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, SERVERS_WORK_DISPATCH_MESQ_NUM,sizeof(servers_publish_data_id_e));
    while (1) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        if (thiz && thiz->hook && thiz->hook->servers_work_thread_route) {
            thiz->hook->servers_work_thread_route(thiz, msg);
        }
    }
}

static int on_post_servers_init(void *ptr)
{
    servers_manager_context_t *ctx = ptr;
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }
    ctx->priv.protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    RUN_NEW_THREAD(&ctx->priv.servers_thd, "servers_thd", servers_work_thread,
        RT_PRIO_SERVERS_DISPATCH_THREAD, SERVERS_THREAD_STACKSIZE);
    /// 注册广播监听.
    ctx->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.bcd->context = ctx;
    ctx->priv.bcd->name = "product_protocol_thread";
    ctx->priv.bcd->id.code |= ID_BROADCAST_BLE_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    ctx->priv.bcd->receiver_cb = on_servers_manager_receiver;
    register_broadcast_receiver(ctx->priv.bcd);
    LOG_SRV("on_post_servers_init");
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_servers_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, servers_manager_context_t);
    thiz = ptr;
    thiz->dispatch_servers_event_wrapper = dispatch_servers_event_wrapper;
    return ADD_MODULE("servers", ID_MODULE_SERVERS, NULL, 
        on_post_servers_init, ptr, list_head, sizeof(servers_manager_context_t));
}

