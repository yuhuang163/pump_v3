/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
//#include "factory_protocol_handler.h"
#include "factory_protocol_parse.h"
#include "protocol_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static protocol_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void product_tests_data_recv_parse(void *ctx, uint8_t *p_data, uint16_t ulLen)
{
    //选择协议解析器
    if (thiz->protocol_parse == NULL)
    {
        LOGE("protocol_parse is NULL");
        return;
    }
    thiz->protocol_parse(p_data, ulLen, PROTO_TYPE_FACTORY_TEST);
}

static void product_tests_uart_data_recv_cb(void *ctx, uint8_t *p_data, uint16_t ulLen)
{
    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(p_data);
    if (!p_data || ulLen== 0) {
        LOGE("recv data is NULL or size is 0");
        return;
    }
    LOGI("uart_factory_recv_data[%d]", ulLen);
    LOG_HEX("recv_data hex", p_data, ulLen);
    LOGI("recv_data str %s", p_data);
    // 这里可以添加对接收到的数据的处理逻辑
    product_tests_data_recv_parse(ctx, p_data, ulLen);
}

static uint16_t product_tests_ble_recv_data_cb(void *ctx, uint8_t *p_data, uint16_t ulLen)
{
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(p_data);
    if (!p_data || ulLen== 0) {
        LOGE("recv data is NULL or size is 0");
        return 0;
    }
    LOGI("ble_factory_recv_data[%d]", ulLen);
    LOG_HEX("recv_data hex", p_data, ulLen);
    LOGI("recv_data str %s", p_data);
    // 这里可以添加对接收到的数据的处理逻辑
    product_tests_data_recv_parse(ctx, p_data, ulLen);

    return 1;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void set_product_tests_ble_uart_register(protocol_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);

    /* 注册 BLE 回调（存在时） */
    if (ctx->priv.ble_ctx && ctx->priv.ble_ctx->set_factory_ble_register_cb) {
        ctx->priv.ble_ctx->set_factory_ble_register_cb(product_tests_ble_recv_data_cb);
        LOGI("register factory ble cb");
    } else {
        LOGI("ble ctx or register cb not available");
    }

    /* 注册 UART 回调（存在时） */
    if (ctx->priv.fcuartdev) {
        LOGI("register factory uart cb");
        DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.fcuartdev, OP_CODE_UART_ADD_EVENT_CB, product_tests_uart_data_recv_cb);
    } else {
        LOGI("fcuartdev not available, uart cb not registered");
    }
}

void set_product_tests_ble_uart_unregister(protocol_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);

    if (ctx->priv.ble_ctx && ctx->priv.ble_ctx->set_factory_ble_register_cb) {
        ctx->priv.ble_ctx->set_factory_ble_register_cb(NULL); /// 清除工厂模式蓝牙数据接收回调函数.
        LOGI("unregister factory ble cb");
    } else {
        LOGI("ble ctx or unregister cb not available");
    }

    if (ctx->priv.fcuartdev) { /// 清除工厂模式串口数据接收回调函数.
        LOGI("unregister factory uart cb");
        DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.fcuartdev, OP_CODE_UART_ADD_EVENT_CB, NULL);
    } else {
        LOGI("fcuartdev not available, uart cb not unregistered");
    }
 
}

void product_tests_send_data_via_ble_uart(uint8_t *data, uint16_t data_len)
{
    const uint16_t MAX_SEND_LEN = 256;

    /* 参数验证 */
    if (!data || data_len == 0) {
        LOGI("invalid data or length");
        return;
    }

    if (!thiz) {
        LOGI("protocol manager context is NULL");
        return;
    }

    /* 长度限制 */
    if (data_len > MAX_SEND_LEN) {
        LOGI("data_len(%d) > MAX_SEND_LEN(%d), truncate", data_len, MAX_SEND_LEN);
        data_len = MAX_SEND_LEN;
    }

    /* 通过 BLE 发送 */
    if (thiz->priv.ble_ctx && thiz->priv.ble_ctx->ble_is_connected &&
        thiz->priv.ble_ctx->ble_is_connected()) {
        if (thiz->priv.ble_ctx->dispatch_ble_event_wrapper) {
            int ret = thiz->priv.ble_ctx->dispatch_ble_event_wrapper(
                thiz->priv.ble_ctx, BLE_MSG_FAC_SEND_DATA, data, data_len);
            if (ret < 0) {
                LOGI("send via BLE failed, ret=%d", ret);
            } else {
                LOGI("send data via BLE[%d]", data_len);
            }
        } else {
            LOGI("ble dispatch wrapper missing");
        }
    }

    /* 通过 UART 发送 */
    if (thiz->priv.fcuartdev) {
        rt_device_node_t *uart = (rt_device_node_t *)thiz->priv.fcuartdev;
        if (uart && uart->write) {
            int ret = uart->write(uart, data, data_len);
            if (ret < 0) {
                LOGI("send via UART failed, ret=%d", ret);
            } else {
                LOGI("send data via UART[%d bytes sent]", ret);
            }
        } else {
            LOGI("uart device or write callback missing");
        }
    }
}

uint8_t product_tests_protocol_parse_route(uint8_t *data, uint16_t len)
{
    // 这里添加工厂测试协议的解析逻辑
    LOGI("Factory protocol parse data len: %d", len);
    // 示例：简单返回成功标志
    product_tests_send_data_via_ble_uart(data, len); // 回传数据示例
    return 1;
}

int on_factory_protocol_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    protocol_manager_context_t *ctx = context;

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

/// 初始化工厂协议模块
void on_perpare_factory_protocol_init(protocol_manager_context_t *ctx)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ctx->priv.fcuartdev = pbus->find_device(ID_DEVICE_UART);
    LOGI("fcuartdev device: %p", ctx->priv.fcuartdev);
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    ctx->priv.ble_ctx = ble_ctx;
    LOGI("ble ctx: %p", ctx->priv.ble_ctx);
    thiz = ctx;
}

/// 初始化工厂协议模块
void on_post_factory_protocol_init(protocol_manager_context_t *ctx)
{
    set_product_tests_ble_uart_register(ctx);
    thiz = ctx;
}
