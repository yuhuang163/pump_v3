/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static algo_manager_context_t *algo_obj = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void get_algo_data(algo_breast_pump_data_t *algo_data)
{
    memcpy(algo_data, &algo_obj->priv.algo_data, sizeof(algo_breast_pump_data_t));
}

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    on_algo_result_cb_ptr cb = (on_algo_result_cb_ptr)data;
    algo_evt_client_node_t *node = CONTAINER_OF(pos, algo_evt_client_node_t, list);
    if ((node->cb) == (cb)) {
        return node;
    }
    return NULL;
}

static void algo_on_receive_internal(broadcast_mask_id_t id, void *context, int args1, int args2, broadcast_priv_data_t *pdata)
{
    algo_manager_context_t *ctx = context;
    if (ctx->hook && ctx->hook->on_algo_receiver_route) {
        ctx->hook->on_algo_receiver_route(id, context, args1, args2, pdata);
    }
}

void algo_manager_dispatch_to_clients(algo_manager_context_t *ctx)
{
    list_head_t *head = &ctx->priv.client_list;
    algo_breast_pump_data_t result = {0};
    MUTEX_LOCK(ctx->priv.m_lock);
    memcpy(&ctx->priv.m_result, &ctx->priv.algo_data, sizeof(algo_breast_pump_data_t));
    memcpy(&result, &ctx->priv.m_result, sizeof(algo_breast_pump_data_t));
    LIST_FOR_EACH(pos, (head)) {
        algo_evt_client_node_t *node = CONTAINER_OF(pos, algo_evt_client_node_t, list);
        if (node->cb) {
            node->cb(node->user_data, &result);
        }
    }
    MUTEX_UNLOCK(ctx->priv.m_lock);
}

static int on_perpare_algo_init(void *self)
{
    algo_manager_context_t *ctx = self;
    return 0;
}

static int on_post_algo_init(void *ptr)
{
    algo_manager_context_t *ctx = ptr;
    ctx->priv.motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    ctx->priv.sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    ctx->priv.pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    broadcast_obj_t *bdc = &ctx->priv.m_bcd;
    bdc->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    bdc->id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    bdc->context = ctx;
    bdc->name = "algo_fusion";
    bdc->receiver_cb = algo_on_receive_internal;
    register_broadcast_receiver(bdc);
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }
    return 0;
}

static int algo_add_result_listener(void *user_data, on_algo_result_cb_ptr cb)
{
    list_head_t *head = &algo_obj->priv.client_list;
    MUTEX_LOCK(algo_obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) {
        algo_evt_client_node_t *node = CONTAINER_OF(pos, algo_evt_client_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOG_ALGO("ignore duplicate add.");
            MUTEX_UNLOCK(algo_obj->priv.m_lock);
            return -1;
        }
    }
    algo_evt_client_node_t *node = u_malloz(sizeof(algo_evt_client_node_t));
    node->cb = cb;
    node->user_data = user_data;
    __list_add(&node->list, head);
    MUTEX_UNLOCK(algo_obj->priv.m_lock);
    return 0;
}

static void algo_remove_result_listener(on_algo_result_cb_ptr cb)
{
    list_head_t *head = &algo_obj->priv.client_list;
    MUTEX_LOCK(algo_obj->priv.m_lock);
    __list_del(head, cb, list_del_condition_cb);
    MUTEX_UNLOCK(algo_obj->priv.m_lock);
}

static void set_algo_enabled(bool enabled)
{
    EXCEPTION_CHECK_VOID(algo_obj);
    algo_obj->priv.is_algo_enabled = enabled;
    LOG_ALGO("algo enabled: %d", enabled);
}

static bool get_algo_enabled(void)
{
    EXCEPTION_CHECK_INT(algo_obj);
    return algo_obj->priv.is_algo_enabled;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_algo_fusion_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, algo_manager_context_t);
    ptr->priv.algo_inited = 0;
    ptr->priv.is_algo_enabled = true;
    MUTEX_INIT(&ptr->priv.m_lock, algo_client_mutex);
    MUTEX_INIT(&ptr->priv.sampling_lock, algo_sampling_mutex);
    USE_INIT_LIST_HEAD(&ptr->priv.client_list);
    ptr->get_algo_data = get_algo_data;
    ptr->add_algo_result_listener = algo_add_result_listener;
    ptr->remove_algo_result_listener = algo_remove_result_listener;
    ptr->set_algo_enabled = set_algo_enabled;
    ptr->get_algo_enabled = get_algo_enabled;
    algo_obj = ptr;
    return ADD_MODULE("algo", ID_MODULE_ALGO_HUB, on_perpare_algo_init, 
        on_post_algo_init, ptr, list_head, sizeof(algo_manager_context_t));
}
 