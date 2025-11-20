/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "product_tests_common.h" 
#include "tests_manager.h"  // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static tests_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int common_event_printf(comm_tst_event_type_e event)
{
    char *str = "null";
    switch(event)
    {
        case COMM_TST_READ_SW_VER_EVENT: str = "COMM_TST_READ_SW_VER_EVENT"; break;
        case COMM_TST_READ_DEV_MAC_EVENT: str = "COMM_TST_READ_DEV_MAC_EVENT"; break;
        case COMM_TST_MOTOR_CTR_EVENT: str = "COMM_TST_MOTOR_CTR_EVENT"; break;
        // 部分事件不需要打印
        default:
            return -1;
    }
    LOGI("product tst event-- %s = %d", str, event);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void common_tests_thread_handler(struct tests_manager_context *ctx, void *msg)
{
    multi_func_msg_data_t *m = (multi_func_msg_data_t *)msg;
    comm_tst_event_type_e event = (comm_tst_event_type_e)m->union_body.product_test_extra.event_type;
    //LOGI("product_tests loop recv msg. %d", event);
    common_event_printf(event);
    switch (event) {
        case COMM_TST_READ_SW_VER_EVENT:
            {

            }
            break;
        case COMM_TST_READ_DEV_MAC_EVENT:
            {

            }
            break;
        case COMM_TST_MOTOR_CTR_EVENT:
            {

            }
            break;
        default:
            break;
    }

}

int on_common_tests_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    tests_manager_context_t *ctx = context;

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

/// 监听事件上报事
int dispatch_listen_common_event_report(tests_manager_context_t *ctx, uint16_t event_id, void *data, uint16_t data_len)
{
    list_head_t *head = &ctx->priv.product_test_list;

    MUTEX_LOCK(ctx->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) {
        product_tst_evt_client_node_t *node = CONTAINER_OF(pos, product_tst_evt_client_node_t, list);
        if (node->report_type == TST_COMMON_TYPE ) {
            node->cb(node->user_data, event_id, data, data_len);
        }
    }
    MUTEX_UNLOCK(ctx->priv.m_lock);

    return 0;
}

void on_perpare_common_test_init(tests_manager_context_t *ctx)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ctx->priv.comm_ctx.airdev = pbus->find_device(ID_DEVICE_AIR_PRESSURE);
    ctx->priv.comm_ctx.fcuartdev = pbus->find_device(ID_DEVICE_UART);
    LOGI("air pressure device: %p", ctx->priv.comm_ctx.airdev);
    LOGI("fcuartdev device: %p", ctx->priv.comm_ctx.fcuartdev);

    breast_pump_manager_context_t *breast_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    ctx->priv.comm_ctx.pump = breast_ctx;
    ctx->priv.comm_ctx.ble_ctx = ble_ctx;

    thiz = ctx;
}



void on_post_common_test_init(tests_manager_context_t *ctx)
{
    thiz = ctx;
}



