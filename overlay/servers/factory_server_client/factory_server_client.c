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
    LOG_SRV("err_code: %d", err_code);
    LOG_HEX("data", data, len);
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_VOID(ble_ctx);
    /// 发送数据主要ack应答
    ble_ctx->dispatch_ble_event_wrapper(ble_ctx, BLE_MSG_FAC_SEND_DATA, (uint8_t *)data, len);
#if USE_PLATFORM_TRACE
    peripheral_bus_context_t *bus_ctx =
        (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(bus_ctx);
    rt_device_node_t *node = bus_ctx->find_device(ID_DEVICE_UART);
    EXCEPTION_CHECK_VOID(node);
    node->write(node, data, len);
#endif
}

static void on_factory_server_event_cb(void *user_data, uint8_t *data, uint16_t data_len)   
{
    LOG_SRV("==> on_factory_server_event_cb: data=%p, len=%d", data, data_len);
    /// 缺少组包功能，先直接订阅
    comm_protocol_subscribe_context_t ctx = {
        .buffer = data,
        .size = data_len,
        .cb = on_packet_subscribe_callback,
    };
    LOG_SRV("==> ctx.buffer=%p, ctx.size=%d", ctx.buffer, ctx.size);
    comm_protocol_subscribe_handler(&ctx);
    LOG_SRV("==> end");
}

#if USE_PLATFORM_TRACE
static void on_factory_server_event_cb_callback(void *ctx, uint8_t *p_data, uint16_t ulLen)
{
    // 忽略空数据或NULL指针
    if (ulLen == 0 || p_data == NULL) {
        return;
    }
    
    // 过滤单个字节的错误数据（FF/FE可能是UART错误状态或噪声）
    if (ulLen == 1 && (p_data[0] == 0xFF || p_data[0] == 0xFE)) {
        LOGW("filtered single byte error data: 0x%02X", p_data[0]);
        return;
    }
    
    LOG_SRV("ctx=%p, p_data=%p, ulLen=%d", ctx, p_data, ulLen);
    on_factory_server_event_cb(ctx, p_data, ulLen);
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void factory_server_client_init(void)
{
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_VOID(ble_ctx);
    ble_ctx->add_ble_evt_lisenser(ble_ctx, DEVICE_BLE_FAC_MSG_CHANNEL, on_factory_server_event_cb);

#if USE_PLATFORM_TRACE
    peripheral_bus_context_t *bus_ctx = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(bus_ctx);
    rt_device_node_t *node = bus_ctx->find_device(ID_DEVICE_UART);
    EXCEPTION_CHECK_VOID(node);
    node->ioctl(node, OP_CODE_UART_ADD_EVENT_CB, on_factory_server_event_cb_callback);
#endif
}

int factory_server_data_publish(servers_publish_data_id_e id, servers_publish_data_args_t *args)
{
    comm_protocol_publish_func func = get_comm_protocol_publish_func(id);
    EXCEPTION_CHECK_INT(func);
    return func(args);
}