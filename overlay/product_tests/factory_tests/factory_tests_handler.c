/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_tests_handler.h"
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
static int factory_event_printf(factory_tst_event_type_e event)
{
    char *str = "null";
    switch(event)
    {
        case FAC_TST_READ_SW_VER_EVENT: str = "FAC_TST_READ_SW_VER_EVENT"; break;
        case FAC_TST_READ_DEV_MAC_EVENT: str = "FAC_TST_READ_DEV_MAC_EVENT"; break;
        case FAC_TST_MOTOR_CTR_EVENT: str = "FAC_TST_MOTOR_CTR_EVENT"; break;
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

int get_factory_tests_sw_ver(tests_func_args_t args)
{
    int ret = 0;
    char *sw_ver = "1.0.0";
    EXCEPTION_CHECK_INT(args.val);
    strncpy((char *)args.val, sw_ver, strlen(sw_ver) + 1);
    LOGI("factory test get sw ver: %s", (char *)args.val);

    //test code
    multi_func_msg_data_t msg = {0};
    msg.union_body.product_test_extra.event_type = FAC_TST_READ_SW_VER_EVENT;
    EXCEPTION_CHECK_INT(thiz);
    MSG_QUEUE_SEND(thiz->priv.tests_thd.msg_id, &msg, 0);
    LOGI("factory test ver event sent.");
    return ret;
}

void factory_tests_thread_handler(struct tests_manager_context *ctx, void *msg)
{
    multi_func_msg_data_t *m = (multi_func_msg_data_t *)msg;
    factory_tst_event_type_e event = (factory_tst_event_type_e)m->union_body.product_test_extra.event_type;
    uint16_t length = m->union_body.product_test_extra.data_length;
    //LOGI("product_tests loop recv msg. %d", event);
    factory_event_printf(event);
    switch (event) {
        case FAC_TST_READ_SW_VER_EVENT:
            {
                dispatch_listen_factory_event_report(ctx, FAC_TST_READ_SW_VER_EVENT, m->union_body.product_test_extra.extra_data, length);
                if (m->union_body.product_test_extra.extra_data) {
                    u_free(m->union_body.product_test_extra.extra_data);
                }
            }
            break;
        case FAC_TST_READ_DEV_MAC_EVENT:
            {

            }
            break;
        case FAC_TST_MOTOR_CTR_EVENT:
            {

            }
            break;
        default:
            break;
    }

}

int on_factory_tests_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
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
int dispatch_listen_factory_event_report(tests_manager_context_t *ctx, uint16_t event_id, void *data, uint16_t data_len)
{
    list_head_t *head = &ctx->priv.product_test_list;

    MUTEX_LOCK(ctx->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) {
        product_tst_evt_client_node_t *node = CONTAINER_OF(pos, product_tst_evt_client_node_t, list);
        if (node->report_type == TST_FACTORY_TYPE ) {
            node->cb(node->user_data, event_id, data, data_len);
        }
    }
    MUTEX_UNLOCK(ctx->priv.m_lock);

    return 0;
}

/// 初始化工厂测试模块，获取工厂测试单独需要相关设备句柄.
void on_perpare_factory_tests_init(tests_manager_context_t *ctx)
{
    thiz = ctx;
}

/// 初始化工厂测试模块，获取设备存储的产测信息等操作.
void on_post_factory_tests_init(tests_manager_context_t *ctx)
{
    thiz = ctx;
}
