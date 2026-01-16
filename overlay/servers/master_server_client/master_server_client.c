/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ble_manager_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "master_server_client.h" // IWYU pragma: keep
#include "common_service.h"
#include "app_protocol_handler.h"
/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static protocol_manager_context_t *protocol_ctx = NULL;
static ble_manager_context_t *ble_ctx = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void app_old_protocol_frame_callback(uint8_t *frame_data, uint16_t frame_len)
{
    LOG_PROTOCOL("app_old_protocol_frame_callback");
    LOG_HEX("data", frame_data, frame_len);
    old_protocol_frame_node_t frame_full = {0};
    protocol_ctx->app_old_protocol_frame_deserialize(frame_data, frame_len, &frame_full);
    app_old_comm_handle_request_cmd(frame_full.cid, frame_full.cal, frame_full.cab);
}

static void on_master_server_event_cb(void *user_data, uint8_t *data, uint16_t data_len)
{
    LOG_PROTOCOL("==> on_master_server_event_cb: data=%p, len=%d", data, data_len);
    protocol_ctx->app_old_protocol_assemble_packet(data, data_len, app_old_protocol_frame_callback);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int on_master_app_old_protocol_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    servers_manager_context_t *ctx = context;

    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            {

            }
            break;
            case ID_BROADCAST_BLE_CONNECT_STATE_CHANGE:
            {

            }
            break;
        case ID_BROADCAST_SYS_POWER_STATE_CHANGE:
            {

            }
            break;
        case ID_BROADCAST_SYS_BOOT_COMPLETED:
            {

            }
            break;
        default:
            break;
    }
   return 0;
}

void master_server_client_init(void)
{
    ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_VOID(ble_ctx);
    protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_VOID(protocol_ctx);
    ble_ctx->add_ble_evt_lisenser(ble_ctx, DEVICE_BLE_MASTER_MSG_CHANNEL, on_master_server_event_cb);
    app_old_protocol_ctx_init();
}

int app_old_protocol_server_data_publish(app_old_comm_cmd_id_e cid, servers_publish_data_args_t *args)
{
    app_old_comm_handle_response_cmd(cid, sizeof(servers_publish_data_args_t), args);
    return true;
}