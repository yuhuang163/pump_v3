/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_server_client.h" // IWYU pragma: keep
#include "common_service.h"

/*
* DEFINES
*****************************************************************************************
*/
#define USE_PLATFORM_TRACE 1
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
static void on_packet_subscribe_callback(int err_code, uint8_t *data, uint16_t len)
{
    // LOG_SRV("err_code: %d", err_code);
    LOG_HEX("data", data, len);
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_VOID(ble_ctx);
    /// 发送数据主要ack应答
    ble_ctx->dispatch_ble_event_wrapper(ble_ctx, BLE_MSG_FAC_SEND_DATA, (uint8_t *)data, len);
}

static void on_ota_server_event_cb(void *user_data, uint8_t *data, uint16_t data_len)   
{
    comm_protocol_subscribe_context_t ctx = {
        .buffer = data,
        .size = data_len,
        .cb = on_packet_subscribe_callback,
    };
    comm_protocol_subscribe_assemble_handler(&ctx);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ota_server_client_init(void)
{
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_VOID(ble_ctx);
    ble_ctx->add_ble_evt_lisenser(ble_ctx, DEVICE_BLE_REMOTE_MSG_CHANNEL, on_ota_server_event_cb);
}

int ota_server_data_publish(servers_publish_data_id_e id, servers_publish_data_args_t *args)
{
    comm_protocol_publish_func func = get_comm_protocol_publish_func(id);
    EXCEPTION_CHECK_INT(func);
    return func(args);
}