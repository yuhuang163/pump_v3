/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "protocol_manager_api.h"
#include "comm_protocol_builder.h"
#include "comm_protocol_parser.h"
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static protocol_manager_context_t *thiz = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t get_packet_seq(void)
{
    return thiz->priv.packet_seq++;
}

static void on_protocol_broadcast_receive(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    protocol_manager_context_t *ctx = context;
    switch (id.code) {
        case ID_BROADCAST_BLE_STATE_CHANGE:
            comm_protocol_assemble_reset();
            break;
        default:
            break;
    }
}

static int on_post_protocol_init(void *self)
{
    protocol_manager_context_t *ctx = self;
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    ctx->priv.ble_ctx = ble_ctx;

    ctx->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.bcd->context = ctx;
    ctx->priv.bcd->name = "protocol_manager";
    ctx->priv.bcd->id.code |= ID_BROADCAST_BLE_STATE_CHANGE;
    ctx->priv.bcd->receiver_cb = on_protocol_broadcast_receive;
    register_broadcast_receiver(ctx->priv.bcd);
    LOG_PROTOCOL("ble ctx: %p", ctx->priv.ble_ctx);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_protocol_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, protocol_manager_context_t);
    ptr->get_packet_seq = get_packet_seq;
    ptr->comm_protocol_builder_init = comm_protocol_builder_init;
    ptr->comm_protocol_builder_init_service = comm_protocol_builder_init_service;
    ptr->comm_protocol_builder_add_tlv = comm_protocol_builder_add_tlv;
    ptr->comm_protocol_builder_finalize = comm_protocol_builder_finalize;
    ptr->comm_protocol_builder_free = comm_protocol_builder_free;
    ptr->comm_protocol_assemble_packet = comm_protocol_assemble_packet;
    ptr->comm_protocol_assemble_reset = comm_protocol_assemble_reset;
    ptr->comm_protocol_frame_deserialize = comm_protocol_frame_deserialize;
    ptr->app_old_protocol_assemble_packet = app_old_protocol_assemble_packet;
    ptr->app_old_protocol_frame_deserialize = app_old_protocol_frame_deserialize;
    ptr->app_old_protocol_frame_builder_send = app_old_protocol_frame_builder_send;
    thiz = ptr;
    return ADD_MODULE("protocol", ID_MODULE_PROTOCOL, NULL, on_post_protocol_init,
                      ptr, list_head, sizeof(protocol_manager_context_t));
}


