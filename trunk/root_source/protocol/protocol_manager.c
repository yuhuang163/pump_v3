/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "protocol_manager_api.h"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static protocol_manager_context_t *thiz = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void on_protocol_manager_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    protocol_manager_context_t *ctx = context;

    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_protocol_manager_receiver_route(id, context, arg1, arg2, pdata);
    }

    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            {

            }
            break;
        case ID_BROADCAST_UPDATE_BATTERY_INFO:
            {

            }
            break;
        default:
            break;
    }

}

static uint8_t protocol_parse(uint8_t *data, uint16_t len, uint8_t proto_type)
{
    if (PROTO_TYPE_FACTORY_TEST == proto_type) {
        if (thiz->hook && thiz->hook->factory_tests_protocol_parse_route) {
            return thiz->hook->factory_tests_protocol_parse_route(data, len);
        }
    }
    else if (PROTO_TYPE_APP_COMMINICATION == proto_type) {
        if (thiz->hook && thiz->hook->app_commication_protocol_parse_route) {
            return thiz->hook->app_commication_protocol_parse_route(data, len);
        }
    }
    return 0;
}

static int on_prepare_protocol_manager_init(void *self)
{
    protocol_manager_context_t *ctx = (protocol_manager_context_t *)self;

    /// 钩子参数初始化
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }


    return 0;
}

static int on_post_protocol_manager_init(void *self)
{
    protocol_manager_context_t *ctx = (protocol_manager_context_t *)self;

        /// 钩子参数初始化
    if (ctx->hook && ctx->hook->on_perpare_init_route) {
        ctx->hook->on_perpare_init_route(ctx);
    }

    /// 注册广播监听.
    ctx->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.bcd->context = ctx;
    ctx->priv.bcd->name = "product_protocol_thread";
    ctx->priv.bcd->id.code |= ID_BROADCAST_SYS_BOOT_COMPLETED;
    ctx->priv.bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_UPDATE_BATTERY_INFO;
    ctx->priv.bcd->id.code |= ID_BROADCAST_SYS_POWER_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_BLE_CONNECT_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_SYS_BOOT_COMPLETED;
    ctx->priv.bcd->receiver_cb = on_protocol_manager_receiver;
    register_broadcast_receiver(ctx->priv.bcd);

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_protocol_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, protocol_manager_context_t);

    ptr->protocol_parse = protocol_parse;
    thiz = ptr;
    return ADD_MODULE("protocol", ID_MODULE_PROTOCOL, on_prepare_protocol_manager_init, on_post_protocol_manager_init,
                      ptr, list_head, sizeof(protocol_manager_context_t));
}


