/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_IMU_FIFO
#define SENSOR_THREAD_POLLING_PERIOD         (100)
#define IMU_POLLING_PERIOD                   (SENSOR_THREAD_POLLING_PERIOD - 5)
#define LOG_IMU_FIFO_PERIOD                  (260)
#else
#define SENSOR_THREAD_POLLING_PERIOD         (12)
#define IMU_POLLING_PERIOD                   (SENSOR_THREAD_POLLING_PERIOD - 2)
#define LOG_IMU_FIFO_PERIOD                  (500)
#endif

#define F_SENSOR_POLLING_PERIOD              (160)

typedef struct {
    void *user_data;
    on_sensor_evt_data_cb_fptr cb;
    int report_type;
    int step;
    uint32_t old_cnt;
    list_head_t list;
} sensor_evt_client_node_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static sensor_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
/// sensor事件分发.
static int dispatch_sensor_client(sensor_wrapper_data_t *sensor_data)
{
    sensor_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        sensor_evt_client_node_t *node = CONTAINER_OF(pos, sensor_evt_client_node_t, list);
        if (node->report_type == IMU_EVT_POLLING && 
        (obj->priv.imu_polling_cnt - node->old_cnt >= node->step)) {
            node->old_cnt = obj->priv.imu_polling_cnt;
            node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
        } else if (node->report_type == F_SENSOR_EVT_POLLING && 
        (obj->priv.f_sensor_polling_cnt - node->old_cnt >= node->step)) {
            node->old_cnt = obj->priv.f_sensor_polling_cnt;
            node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);

    return 0;
}

static void imu_irq_hdr_cb(hdr_post_args_t *args)
{
    sensor_manager_context_t *ctx = args->user_data;
    for (uint8_t i = 0; i < IRQ_EVENT_CB_MAX; i++) {
        if (ctx->priv.irq_client_cb[i]) {
            ctx->priv.irq_client_cb[i](ctx->priv.irq_client_token[i]);
        }
    }
    ctx->priv.imu_irq_finished = true;
}

static void imu_irq_callback(uint8_t *data)
{
    sensor_manager_context_t *ctx = thiz;

    if (ctx->priv.imu_irq_finished) {
        ctx->priv.imu_irq_finished = false;
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        pm->pm_wakeup(PM_WAKEUP_REASON_IMU_IRQ);
        POST_TASK_HANDLER_DELAYED_ARG1(imu_irq_hdr_cb, 100, ctx, "imu_irq_event");
    }
}

static void sensor_polling_timer_handler(void *handle)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(thiz->priv.thd.msg_id, &msg, 0);
}

static void add_sensor_evt_lisenser(void *user_data, int report_type, int step, on_sensor_evt_data_cb_fptr cb)
{
    sensor_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;
    
    MUTEX_LOCK(obj->priv.m_lock);

    // check dumplicate add.
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

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    on_sensor_evt_data_cb_fptr cb = (on_sensor_evt_data_cb_fptr)data;
    sensor_evt_client_node_t *node = CONTAINER_OF(pos, sensor_evt_client_node_t, list);
    
    if ((node->cb) == (cb)) {
        LOG_SENSOR("del: user_data: %lx, cb: %lx", node->user_data, node->cb);
        return node;
    }

    return NULL;
}

static void remove_sensor_evt_lisenser(on_sensor_evt_data_cb_fptr cb)
{
    sensor_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, list_del_condition_cb);

    MUTEX_UNLOCK(obj->priv.m_lock);
}

static int write_imu_calibration(const imu_cali_data_t *cali)
{
    storage_context_t *st = (storage_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_INT(st);

    int ret = st->put_store_val(STORE_ID_FACTORY_IMU_CALI_DATA, 0, 0, (const void *)cali);
    LOG_SENSOR("write: ret=%d", ret);

    return ret;
}

static const imu_cali_data_t *get_imu_cali_data(void)
{

    storage_context_t *st = (storage_context_t *)find_module(ID_MODULE_STORAGE);
    return (const imu_cali_data_t *)&st->priv.factory_data.imu;
}

static void add_imu_irq_callback(uint8_t irq_num, void *token, on_imu_irq_callback_fptr cb)
{
    if (irq_num < IRQ_EVENT_CB_MAX) {
        sensor_manager_context_t *ctx = thiz;
        ctx->priv.irq_client_token[irq_num] = token;
        ctx->priv.irq_client_cb[irq_num] = cb;
        LOG_SENSOR("---add token: 0x%lx", (uint32_t)token);
    }
}

static void remove_imu_irq_callback(uint8_t irq_num, void *token)
{
    if (irq_num < IRQ_EVENT_CB_MAX) {
        sensor_manager_context_t *ctx = thiz;
        if (ctx->priv.irq_client_token[irq_num] == token) {
            ctx->priv.irq_client_token[irq_num] = NULL;
            ctx->priv.irq_client_cb[irq_num] = NULL;
            LOG_SENSOR("---remove token: 0x%lx", (uint32_t)token);
        }
    }
}

static void add_imu_irq_cb_hanlder(hdr_post_args_t *args)
{
    sensor_manager_context_t *ctx = args->user_data;
    rt_device_node_t *dev = ctx->priv.sensor;

    if (dev && dev->ioctl) {
        LOGI("add irq cb");
        DEVICE_IOCTL_ARGS(dev, OP_CODE_IMU_ADD_IRQ_CALLBACK, imu_irq_callback);
    }
}

static void on_sensor_input_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    sensor_manager_context_t *ctx = context;

    switch (id.code) {
        case ID_BROADCAST_SYS_BOOT_COMPLETED:
            {
                POST_TASK_HANDLER_DELAYED_ARG1(add_imu_irq_cb_hanlder, 2000, ctx, "add_irq_callback:task");
            }
        case ID_BROADCAST_PM_STATE_CHANGE:
            {
                int pm_state = arg1;
                if (pm_state == ID_SYS_PM_SUSPEND) {
                    LOGI("STOP_TIMER: sensor polling_timer");
                    STOP_TIMER(ctx->priv.polling_timer);
                    ctx->priv.do_polling = 0;
                } else {
                    LOGI("START_TIMER: id = 0x%lx, sensor polling_timer, delay_ms: %d", id.code, SENSOR_THREAD_POLLING_PERIOD);
                    STOP_TIMER(ctx->priv.polling_timer);
                    START_TIMER(ctx->priv.polling_timer, SENSOR_THREAD_POLLING_PERIOD);
                    ctx->priv.do_polling = 1;
                }
                // enable irq.
                ctx->imu_irq_enable(ctx);
            }
            break;
        case ID_BROADCAST_OTA_UI_SCREEN_STATE_CHANGE:
            {
                if (arg1 == 1) {
                    LOGI("STOP_TIMER: enter ota-ui screen");
                    STOP_TIMER(ctx->priv.polling_timer);
                    ctx->priv.do_polling = 0;
                } else {
                    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
                    if (!pm->is_pm_suspend()) {
                        LOGI("START_TIMER: exit ota-ui screen");
                        STOP_TIMER(ctx->priv.polling_timer);
                        START_TIMER(ctx->priv.polling_timer, SENSOR_THREAD_POLLING_PERIOD);
                        ctx->priv.do_polling = 1;
                    }
                }
            }
            break;
        default:
            break;
    }

}

static void imu_irq_enable(void *arg)
{
    sensor_manager_context_t *ctx = (sensor_manager_context_t *)arg; 
    DEVICE_IOCTL_NO_RET(ctx->priv.sensor, OP_CODE_IMU_ENABLE_IRQ);
}

static void imu_irq_disable(void *arg)
{
    sensor_manager_context_t *ctx = (sensor_manager_context_t *)arg; 
    DEVICE_IOCTL_NO_RET(ctx->priv.sensor, OP_CODE_IMU_DISABLE_IRQ);
}

static int is_on_running(void)
{
    return thiz->priv.do_polling == 1;
}

static void sensor_manager_internal_dispatch_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    uint8_t msg = 0;
    sensor_wrapper_data_t sensor_data = {0};
    uint16_t log_time = 0;

    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, INPUT_DISPATCH_SENSOR_EVT_MESQ_NUM, 1);
    sensor_manager_context_t *obj = thiz;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *fsensor_dev = pbus->find_device(ID_DEVICE_FSENSOR);
    rt_device_node_t *imu_dev = obj->priv.sensor;
    CREATE_TIMER(&obj->priv.polling_timer, "sensor_polling_timer", obj, OS_TIMER_PERIODIC, sensor_polling_timer_handler);
    START_TIMER(obj->priv.polling_timer, SENSOR_THREAD_POLLING_PERIOD);

    #if IS_SUPPORT_IMU_FIFO
    //常驻内存 无需释放
    sensor_data.imu_data.un.fifo.data = u_malloc(US_IMU_FIFO_SIZE);
    #endif

    obj->imu_irq_enable(obj);
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        EXECUTE_EVERY_X_DELAY(IMU_POLLING_PERIOD, {
            #if IS_SUPPORT_IMU_FIFO
            if (imu_dev) {                 
                obj->imu_polling_cnt++;
                sensor_data.imu_data.un.fifo.data_cnt = 0;
                DEVICE_IOCTL_ARGS(imu_dev, OP_CODE_IMU_GET_IMU_FIFO_DATA, &sensor_data.imu_data.un.fifo);
            }
            #else
            if (imu_dev) {
                obj->priv.imu_polling_cnt++;
                DEVICE_READ_ARGS(imu_dev, &sensor_data.imu_data.un.raw, 12);
            }
            #endif
        });

        EXECUTE_EVERY_X_DELAY(F_SENSOR_POLLING_PERIOD, {
            if (fsensor_dev) {
                obj->priv.f_sensor_polling_cnt++;
                DEVICE_IOCTL_ARGS_NO_RET(fsensor_dev, OP_CODE_FSENSOR_GET_RAW_DATA, &sensor_data);
            }
        });

        dispatch_sensor_client(&sensor_data);

        log_time++;
        if ((log_time % LOG_IMU_FIFO_PERIOD) == 0) {
            dump_imu(&sensor_data.imu_data);
        }
    }
}

static int on_post_sensor_init(void *ptr)
{
    sensor_manager_context_t *ctx = ptr;

    /// 注册广播监听.
    ctx->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.bcd->context = ctx;
    ctx->priv.bcd->name = "sensor_input_thread";
    ctx->priv.bcd->id.code |= ID_BROADCAST_SYS_BOOT_COMPLETED;
    ctx->priv.bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.bcd->id.code |= ID_BROADCAST_OTA_UI_SCREEN_STATE_CHANGE;
    ctx->priv.bcd->receiver_cb = on_sensor_input_receiver;
    register_broadcast_receiver(ctx->priv.bcd);

    /// 启动线程.
    RUN_NEW_THREAD(&ctx->priv.thd, "sensor_thd", 
        sensor_manager_internal_dispatch_thread, RT_PRIO_SENSOR_INPUT_DISPATCH_THREAD, INPUT_SENSOR_THREAD_STACKSIZE);

    return 0;
}

static int on_perpare_sensor_init(void *self)
{
    sensor_manager_context_t *ptr = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.sensor = pbus->find_device(ID_DEVICE_IMU);
    ptr->priv.imu_irq_finished = true;
    MUTEX_INIT(&ptr->priv.m_lock, sensor_client_mutex);
    INIT_LIST_HEAD(&ptr->priv.client_list);

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_sensor_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, sensor_manager_context_t);
    ptr->add_sensor_evt_lisenser = add_sensor_evt_lisenser;
    ptr->remove_sensor_evt_lisenser = remove_sensor_evt_lisenser;
    ptr->write_imu_calibration = write_imu_calibration;
    ptr->get_imu_cali_data = get_imu_cali_data;
    ptr->add_imu_irq_callback = add_imu_irq_callback;
    ptr->remove_imu_irq_callback = remove_imu_irq_callback;
    ptr->imu_irq_enable = imu_irq_enable;
    ptr->imu_irq_disable = imu_irq_disable;
    ptr->is_on_running = is_on_running;

    thiz = ptr;
    return ADD_MODULE("sensor", ID_MODULE_SENSOR, on_perpare_sensor_init, on_post_sensor_init, 
        ptr, list_head, sizeof(sensor_manager_context_t));
}