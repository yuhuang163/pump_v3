/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "tests_manager.h"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static tests_manager_context_t *thiz = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t tests_manager_is_enable(void)
{
    LOG_TEST("tests: tests_manager_is_enable %d", thiz->priv.enabled);
    return thiz->priv.enabled == 1;
}

static uint8_t tests_manager_set_enable(uint8_t enable)
{
    LOG_TEST("tests: tests_manager_is_enable");
    thiz->priv.enabled = enable;

    if (enable) {
        if (thiz->priv.tests_timer) {
            START_TIMER(thiz->priv.tests_timer, PRODUCT_TESTS_THREAD_POLLING_PERIOD);
        }
    } else {
        if (thiz->priv.tests_timer) {
            STOP_TIMER(thiz->priv.tests_timer);
        }
    }
}

static uint8_t tests_manager_set_current_mode(uint8_t mode)
{
    thiz->priv.current_mode = mode;
    LOG_TEST("tests: tests_manager_set_current_mode %d", mode);
    return 0;
}

static uint8_t tests_manager_get_current_mode(void)
{
    LOG_TEST("tests: tests_manager_get_current_mode %d", thiz->priv.current_mode);
    return thiz->priv.current_mode;
}

static int get_hw_self_tests_event_process(uint16_t id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
    tests_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    const get_hw_self_tests_process_handle_t *list = thiz->priv.get_hw_self_tests_process_list;
    uint16_t list_len = thiz->priv.get_hw_self_tests_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.handle_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].event_id == id) {
            ret = list[i].event_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.handle_lock);
    if (ret != 0) {
        LOGE("==> put store val failed: %d", id);
    }
    return ret;
}

static int set_hw_self_tests_event_process(uint16_t id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
    tests_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    const set_hw_self_tests_process_handle_t *list = thiz->priv.set_hw_self_tests_process_list;
    uint16_t list_len = thiz->priv.get_hw_self_tests_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.handle_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].event_id == id) {
            ret = list[i].event_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.handle_lock);
    if (ret != 0) {
        LOGE("==> failed: %d", id);
    }
    return ret;
}

static int get_factory_tests_event_process(uint16_t id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
    tests_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    const get_factory_tests_process_handle_t *list = thiz->priv.get_factory_tests_process_list;
    uint16_t list_len = thiz->priv.get_factory_tests_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.handle_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].event_id == id) {
            ret = list[i].event_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.handle_lock);
    if (ret != 0) {
        LOGE("==> failed: %d", id);
    }
    return ret;
}

static int set_factory_tests_event_process(uint16_t id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
    tests_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    const set_factory_tests_process_handle_t *list = thiz->priv.set_factory_tests_process_list;
    uint16_t list_len = thiz->priv.set_factory_tests_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.handle_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].event_id == id) {
            ret = list[i].event_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.handle_lock);
    if (ret != 0) {
        LOGE("==> failed: %d", id);
    }
    return ret;
}

static int get_common_tests_event_process(uint16_t id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
    tests_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    const get_common_tests_process_handle_t *list = thiz->priv.get_commom_tests_process_list;
    uint16_t list_len = thiz->priv.get_common_tests_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.handle_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].event_id == id) {
            ret = list[i].event_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.handle_lock);
    if (ret != 0) {
        LOGE("==> failed: %d", id);
    }
    return ret;
}

static int set_common_tests_event_process(uint16_t id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
    tests_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    const set_common_tests_process_handle_t *list = thiz->priv.set_commom_tests_process_list;
    uint16_t list_len = thiz->priv.set_common_tests_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.handle_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].event_id == id) {
            ret = list[i].event_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.handle_lock);
    if (ret != 0) {
        LOGE("==> failed: %d", id);
    }
    return ret;
}

static void add_product_tests_evt_lisenser(void *user_data, product_tst_report_type_e report_type,  on_product_test_evt_cb_fptr cb)
{
    tests_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.product_test_list;

    MUTEX_LOCK(obj->priv.m_lock);

    // check dumplicate add.
    LIST_FOR_EACH(pos, (head)) {
        product_tst_evt_client_node_t *node = CONTAINER_OF(pos, product_tst_evt_client_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOGE("ignore dumplicate add.");
            MUTEX_UNLOCK(obj->priv.m_lock);
            return;
        }
    }

    product_tst_evt_client_node_t *node = u_malloz(sizeof(product_tst_evt_client_node_t));
    node->cb = cb;
    node->report_type = report_type;
    node->user_data = user_data;
    __list_add(&node->list, head);
    LOG_TEST("add: user_data: %lx, cb: %lx", user_data, cb);

    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    on_product_test_evt_cb_fptr cb = (on_product_test_evt_cb_fptr)data;
    product_tst_evt_client_node_t *node = CONTAINER_OF(pos, product_tst_evt_client_node_t, list);

    if ((node->cb) == (cb)) {
        LOG_TEST("del: user_data: %lx, cb: %lx", node->user_data, node->cb);
        return node;
    }

    return NULL;
}

static void remove_product_tests_evt_lisenser(on_product_test_evt_cb_fptr cb)
{
    tests_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.product_test_list;

    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, list_del_condition_cb);

    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void product_tests_trigger_timer_handler(void *handle)
{
    multi_func_msg_data_t msg = {0};
    msg.union_body.product_test_extra.event_type = TST_POLL_EVENT;
    MSG_QUEUE_SEND(thiz->priv.tests_thd.msg_id, &msg, 0);
}

static void on_product_tests_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    tests_manager_context_t *ctx = context;

    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_product_tests_br_receiver_route(id, context, arg1, arg2, pdata);
    }

    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            break;
        case ID_BROADCAST_BATTERY_STATE_CHANGE:
            break;
        default:
            break;
    }

}

static void product_tests_manager_internal_dispatch_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    multi_func_msg_data_t msg = {0};
    tests_manager_context_t *obj = thiz;
    CREATE_MSG_QUEUE(&thd->msg_id, PRODUCT_TEST_EVT_MESQ_NUM);
    CREATE_TIMER(&obj->priv.tests_timer, "product_tests_timer", obj, OS_TIMER_PERIODIC,
                 product_tests_trigger_timer_handler);
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        //LOG_TEST("product_tests loop recv msg.");
         /// 调用外部钩子函数处理.
        if (obj->hook && obj->hook->on_post_init_route) {
            obj->hook->product_tests_thread_route(obj, &msg);
        }
    }
}

static int on_prepare_tests_init(void *self)
{
    tests_manager_context_t *ctx = (tests_manager_context_t *)self;

    /// 钩子参数初始化
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }

    MUTEX_INIT(&ctx->priv.m_lock, product_test_mutex);
    USE_INIT_LIST_HEAD(&ctx->priv.product_test_list);

    return 0;
}

static int on_post_tests_init(void *self)
{
    tests_manager_context_t *ctx = (tests_manager_context_t *)self;

        /// 钩子参数初始化
    if (ctx->hook && ctx->hook->on_perpare_init_route) {
        ctx->hook->on_perpare_init_route(ctx);
    }

    /// 注册广播监听.
    ctx->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.bcd->context = ctx;
    ctx->priv.bcd->name = "product_test_thread";
    ctx->priv.bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_BATTERY_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    ctx->priv.bcd->receiver_cb = on_product_tests_receiver;
    register_broadcast_receiver(ctx->priv.bcd);

    /// 启动线程.
    RUN_NEW_THREAD(&ctx->priv.tests_thd, "product_test_thd",
        product_tests_manager_internal_dispatch_thread, RT_PRIO_PRODUCT_TESTS_DISPATCH_THREAD, INPUT_PRODUCT_TESTS_THREAD_STACKSIZE);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_tests_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, tests_manager_context_t);

    ptr->tests_manager_is_enable = tests_manager_is_enable;
    ptr->tests_manager_set_enable = tests_manager_set_enable;
    ptr->tests_manager_set_current_mode = tests_manager_set_current_mode;
    ptr->tests_manager_get_current_mode = tests_manager_get_current_mode;
    ptr->add_product_tests_evt_lisenser = add_product_tests_evt_lisenser;
    ptr->remove_product_tests_evt_lisenser = remove_product_tests_evt_lisenser;
    ptr->get_hw_self_tests_event_process = get_hw_self_tests_event_process;
    ptr->set_hw_self_tests_event_process = set_hw_self_tests_event_process;
    ptr->get_factory_tests_event_process = get_factory_tests_event_process;
    ptr->set_factory_tests_event_process = set_factory_tests_event_process;
    ptr->get_common_tests_event_process = get_common_tests_event_process;
    ptr->set_common_tests_event_process = set_common_tests_event_process;
    thiz = ptr;
    return ADD_MODULE("tests", ID_MODULE_TESTS, on_prepare_tests_init, on_post_tests_init,
                      ptr, list_head, sizeof(tests_manager_context_t));
}


