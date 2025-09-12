/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "sensor_manager.h"

#if IS_SUPPORT_DEVICE_IMU
#include "imu_module_internal.h"
#endif
#if IS_SUPPORT_DEVICE_FSENSOR
#include "force_sensor_module_internal.h"
#endif
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
#include "air_pressure_module_internal.h"
#endif

/*
* DEFINES
*****************************************************************************************
*/

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
        if (node->report_type == IMU_EVT_POLLING ) {
            #if IS_SUPPORT_DEVICE_IMU
            dispatch_imu_client(sensor_data, node);
            #endif
        } else if (node->report_type == F_SENSOR_EVT_POLLING) {
            #if IS_SUPPORT_DEVICE_FSENSOR
            dispatch_force_sensor_client(sensor_data, node);
            #endif
        } else if (node->report_type == AIR_PRESSURE_EVT_POLLING) {
            #if IS_SUPPORT_DEVICE_AIR_PRESSURE
            dispatch_air_pressure_client(sensor_data, node);
            #endif
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);

    return 0;
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

static void on_sensor_input_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    sensor_manager_context_t *ctx = context;

    #if IS_SUPPORT_DEVICE_IMU
    on_imu_sensor_receiver(id, context, arg1, arg2, pdata);
    #endif

    switch (id.code) {
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

static int is_on_running(void)
{
    return thiz->priv.do_polling == 1;
}

static void sensor_manager_internal_dispatch_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    uint8_t msg = 0;
    sensor_wrapper_data_t sensor_data = {0};
    sensor_manager_context_t *obj = thiz;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, INPUT_DISPATCH_SENSOR_EVT_MESQ_NUM, 1);
    CREATE_TIMER(&obj->priv.polling_timer, "sensor_polling_timer", obj, OS_TIMER_PERIODIC, sensor_polling_timer_handler);
    START_TIMER(obj->priv.polling_timer, SENSOR_THREAD_POLLING_PERIOD);
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        #if IS_SUPPORT_DEVICE_IMU
        imu_polling_handler(&sensor_data);
        #endif
        #if IS_SUPPORT_DEVICE_FSENSOR
        force_sensor_polling_handler(&sensor_data);
        #endif
        #if IS_SUPPORT_DEVICE_AIR_PRESSURE
        air_pressure_polling_handler(&sensor_data);
        #endif
        dispatch_sensor_client(&sensor_data);
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
    #if IS_SUPPORT_DEVICE_IMU
    on_post_imu_init(ctx);
    #endif
    /// 启动线程.
    RUN_NEW_THREAD(&ctx->priv.thd, "sensor_thd", 
        sensor_manager_internal_dispatch_thread, RT_PRIO_SENSOR_INPUT_DISPATCH_THREAD, INPUT_SENSOR_THREAD_STACKSIZE);

    return 0;
}

static int on_perpare_sensor_init(void *self)
{
    sensor_manager_context_t *ptr = self;
    #if IS_SUPPORT_DEVICE_IMU
    on_perpare_imu_init(ptr);
    #endif
    #if IS_SUPPORT_DEVICE_FSENSOR
    on_perpare_force_sensor_init(ptr);
    #endif
    #if IS_SUPPORT_DEVICE_AIR_PRESSURE
    on_perpare_air_pressure_init(ptr);
    #endif
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
    ptr->is_on_running = is_on_running;
    #if IS_SUPPORT_DEVICE_IMU
    register_imu_module_ctx(ptr);
    #endif
    thiz = ptr;
    return ADD_MODULE("sensor", ID_MODULE_SENSOR, on_perpare_sensor_init, on_post_sensor_init, 
        ptr, list_head, sizeof(sensor_manager_context_t));
}