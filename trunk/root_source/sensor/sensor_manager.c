/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "sensor_manager.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static sensor_manager_context_t *thiz = NULL;
static list_head_t s_sensor_obj_list = {0}; /// 传感器链表.
static const sensor_register_fptr s_sensor_register[] = {
    #if IS_SUPPORT_DEVICE_IMU
        register_module_for_imu_new,
    #endif
    #if IS_SUPPORT_DEVICE_FSENSOR
        register_module_for_force_sensor_new,
    #endif
    #if IS_SUPPORT_DEVICE_AIR_PRESSURE
        register_module_for_air_pressure_new,
    #endif
    #if IS_SUPPORT_DEVICE_LIGHT_SENSOR
        register_module_for_light_sensor_new,
    #endif
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void sensor_polling_timer_handler(void *handle)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(thiz->priv.thd.msg_id, &msg, 0);
}

static void *sensor_evt_list_del_condition_cb(void *data, list_head_t *pos)
{
    on_sensor_evt_data_cb_fptr cb = (on_sensor_evt_data_cb_fptr)data;
    sensor_evt_client_node_t *node = CONTAINER_OF(pos, sensor_evt_client_node_t, list);
    if ((node->cb) == (cb)) {
        LOG_SENSOR("del: user_data: %lx, cb: %lx", node->user_data, node->cb);
        return node;
    }
    return NULL;
}

static void add_sensor_evt_lisenser(void *user_data, int report_type, int step, on_sensor_evt_data_cb_fptr cb)
{
    sensor_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        sensor_evt_client_node_t *node = CONTAINER_OF(pos, sensor_evt_client_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOGE("ignore dumplicate add.");
            MUTEX_UNLOCK(obj->priv.m_lock);
            return;
        }
    }
    sensor_evt_client_node_t *node = u_malloz(sizeof(sensor_evt_client_node_t));
    node->cb = cb;
    node->step = step;
    node->report_type = report_type;
    node->user_data = user_data;
    __list_add(&node->list, head);
    LOG_SENSOR("add: user_data: %lx, cb: %lx", user_data, cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void remove_sensor_evt_lisenser(on_sensor_evt_data_cb_fptr cb)
{
    sensor_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, sensor_evt_list_del_condition_cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void *sensor_task_list_del_condition_cb(void *data, list_head_t *pos)
{
    on_sensor_task_cb_fptr cb = (on_sensor_task_cb_fptr)data;
    sensor_task_node_t *node = CONTAINER_OF(pos, sensor_task_node_t, list);
    if ((node->cb) == (cb)) {
        LOG_SENSOR("del: user_data: %lx, cb: %lx, task_id: %d", 
            node->user_data, node->cb, node->task_id);
        return node;
    }
    return NULL;
}

static void add_sensor_task(void *user_data, int task_id, uint32_t polling_period, 
    int report_type, on_sensor_task_cb_fptr cb, dispatch_sensor_client_fptr dispatch_client_cb)
{
    sensor_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.task_list;
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        sensor_task_node_t *node = CONTAINER_OF(pos, sensor_task_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOG_SENSOR("ignore duplicate add: user_data: %lx, cb: %lx, task_id: %d", user_data, cb, task_id);
            MUTEX_UNLOCK(obj->priv.m_lock);
            return;
        }
    }

    sensor_task_node_t *node = u_malloz(sizeof(sensor_task_node_t));
    node->cb = cb;
    node->task_id = task_id;
    node->report_type = report_type;
    if (polling_period >= SENSOR_THREAD_POLLING_PERIOD) {
        polling_period = polling_period - (SENSOR_THREAD_POLLING_PERIOD / 3); /// 补偿轮询时间.
    } 
    node->polling_period = polling_period;
    node->user_data = user_data;
    node->last_exec_time = get_timestamp();
    node->dispatch_client_cb = dispatch_client_cb;
    __list_add(&node->list, head);
    LOG_SENSOR("add: user_data: %lx, cb: %lx, task_id: %d, polling_period: %lu", 
        user_data, cb, task_id, polling_period);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void remove_sensor_task(on_sensor_task_cb_fptr cb)
{
    sensor_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.task_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, sensor_task_list_del_condition_cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void on_sensor_input_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    sensor_manager_context_t *ctx = context;
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                LOG_SENSOR("STOP_TIMER: sensor polling_timer");
                STOP_TIMER(ctx->priv.polling_timer);
                ctx->priv.do_polling = 0;
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                LOG_SENSOR("START_TIMER: id = 0x%lx, sensor polling_timer, delay_ms: %d",
                        id.code, SENSOR_THREAD_POLLING_PERIOD);
                STOP_TIMER(ctx->priv.polling_timer);
                START_TIMER(ctx->priv.polling_timer, SENSOR_THREAD_POLLING_PERIOD);
                ctx->priv.do_polling = 1;
            }
            break;
        case ID_BROADCAST_OTA_STATE_CHANGE:       
            if (arg1 == OTA_BROADCAST_UI_ENTER) {
                LOG_SENSOR("STOP_TIMER: enter ota-ui screen");
                STOP_TIMER(ctx->priv.polling_timer);
                ctx->priv.do_polling = 0;
            } else if (arg1 == OTA_BROADCAST_UI_EXIT) {
                power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
                if (!pm->is_pm_suspend()) {
                    LOG_SENSOR("START_TIMER: exit ota-ui screen");
                    STOP_TIMER(ctx->priv.polling_timer);
                    START_TIMER(ctx->priv.polling_timer, SENSOR_THREAD_POLLING_PERIOD);
                    ctx->priv.do_polling = 1;
                }
            }
            break;
        default:
            break;
    }

}

static int is_on_running(void)
{
    return thiz->priv.do_polling == 1;
}

static void dispatch_sensor_client(sensor_manager_context_t *obj, sensor_wrapper_data_t *sensor_data)
{
    list_head_t *head = &obj->priv.client_list;
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        sensor_evt_client_node_t *node = CONTAINER_OF(pos, sensor_evt_client_node_t, list);
        LIST_FOR_EACH(pos, (&obj->priv.task_list)) {
            sensor_task_node_t *task_node = CONTAINER_OF(pos, sensor_task_node_t, list);
            if (task_node->report_type == node->report_type && task_node->dispatch_client_cb) {
                task_node->dispatch_client_cb(sensor_data, node);
            }
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void dispatch_sensor_task_client(sensor_manager_context_t *obj, sensor_wrapper_data_t *sensor_data)
{
    uint32_t current_time = get_timestamp();
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (&obj->priv.task_list)) {
        sensor_task_node_t *node = CONTAINER_OF(pos, sensor_task_node_t, list);
        uint32_t elapsed = current_time - node->last_exec_time;
        if (elapsed >= node->polling_period) {
            if (node->cb) {
                node->cb(sensor_data); /// 执行回调.
            }
            node->last_exec_time = current_time; /// 更新执行时间.
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void sensor_manager_internal_dispatch_thread(const void *args)
{
    uint8_t msg = 0;
    uint16_t log_time = 0;
    uint32_t start_time = get_timestamp();
    thread_context_t *thd = (thread_context_t *)args;
    sensor_wrapper_data_t sensor_data = {0};
    sensor_manager_context_t *obj = thiz;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, INPUT_DISPATCH_SENSOR_EVT_MESQ_NUM, 1);
    CREATE_TIMER(&obj->priv.polling_timer, "sensor_polling_timer", obj, OS_TIMER_PERIODIC, 
        sensor_polling_timer_handler);
    START_TIMER(obj->priv.polling_timer, SENSOR_THREAD_POLLING_PERIOD);
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        dispatch_sensor_task_client(obj, &sensor_data);
        dispatch_sensor_client(obj, &sensor_data);
        log_time++;
        if (log_time >= 500) {
            LOG_SENSOR("sensor polling: %d, time: %d", log_time, get_timestamp_ms(start_time));
            start_time = get_timestamp();
            log_time = 0;
        }
    }
}

static const void *find_sensor(sensor_module_desc_id_e id)
{
    static void *__cache[ID_SENSOR_MAX] = {0};
    list_head_t *list_head = &s_sensor_obj_list;

    if (!(id > ID_SENSOR_BEGIN && id < ID_SENSOR_MAX)) {
        LOGE("invalid module id!");
        return NULL;
    }

    if (__cache[id] != NULL) {
        find_ok:
        return (const void *)__cache[id];
    }

    LIST_FOR_EACH(pos, list_head) {
        sensor_module_node_t *node = CONTAINER_OF(pos, sensor_module_node_t, list);
        if (node->id == id) {
            __cache[id] = node->module_ptr;
            goto find_ok;
        }
    }
    return NULL;
}

static int register_sensor_module(void)
{
    list_head_t *list_head = &s_sensor_obj_list;
    uint32_t total_mem_usage = 0;
    for (int i = 0; i < LENGTH_OF_ARRAY(s_sensor_register); i++) {
        sensor_register_fptr f = s_sensor_register[i];
        sensor_module_node_t *node = f(list_head);
        if (node) {
            total_mem_usage += node->mem_usage;
            LOGI("%d: --- [%s/%d]has registered, mem_usage:%lu/%lu (Byte)", 
                i, node->name, node->id, node->mem_usage, total_mem_usage);
        }
    }
    return 0;
}

static int init_sensor_module(sensor_manager_context_t *ctx)
{
    list_head_t *list_head = &s_sensor_obj_list;
    LIST_FOR_EACH(pos, list_head) {
        sensor_module_node_t *node = CONTAINER_OF(pos, sensor_module_node_t, list);
        if (node->post_init) {
            node->post_init(ctx);
        }
    }
    return 0;
}

static int on_post_sensor_init(void *ptr)
{
    sensor_manager_context_t *ctx = ptr;
    /// 注册广播监听.
    ctx->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.bcd->context = ctx;
    ctx->priv.bcd->name = "sensor_input_thread";
    ctx->priv.bcd->id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_OTA_STATE_CHANGE;
    ctx->priv.bcd->receiver_cb = on_sensor_input_receiver;
    register_broadcast_receiver(ctx->priv.bcd);
    /// 启动线程.
    RUN_NEW_THREAD(&ctx->priv.thd, "sensor_thd", sensor_manager_internal_dispatch_thread,
         RT_PRIO_SENSOR_INPUT_DISPATCH_THREAD, INPUT_SENSOR_THREAD_STACKSIZE);
    return 0;
}

static int on_perpare_sensor_init(void *self)
{
    sensor_manager_context_t *ptr = self;
    register_sensor_module();
    init_sensor_module(ptr);
    MUTEX_INIT(&ptr->priv.m_lock, sensor_client_mutex);
    USE_INIT_LIST_HEAD(&ptr->priv.client_list);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *add_new_sensor(const char *name, sensor_module_desc_id_e id, 
    int (*post_init_cb)(void *ptr), void *ptr, list_head_t *list_head, uint32_t mem_usage)
{
    MALLOC_FOR_PTR(node, sensor_module_node_t);
    node->name = name;
    node->id = id;
    node->post_init = post_init_cb;
    node->module_ptr = ptr;
    node->mem_usage = (mem_usage + sizeof(sensor_module_node_t));
    __list_add_tail(&node->list, list_head);
    return node;
}

void *register_module_for_sensor_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, sensor_manager_context_t);
    ptr->add_sensor_evt_lisenser = add_sensor_evt_lisenser;
    ptr->remove_sensor_evt_lisenser = remove_sensor_evt_lisenser;
    ptr->add_sensor_task = add_sensor_task;
    ptr->remove_sensor_task = remove_sensor_task;
    ptr->is_on_running = is_on_running;
    ptr->find_sensor = find_sensor;
    thiz = ptr;
    return ADD_MODULE("sensor", ID_MODULE_SENSOR, on_perpare_sensor_init, on_post_sensor_init, 
        ptr, list_head, sizeof(sensor_manager_context_t));
}
