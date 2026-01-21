/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#if IS_SUPPORT_DEVICE_IMU
#include "sensor_manager.h" 
#include "imu_module.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static imu_sensor_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int write_imu_calibration(const imu_cali_data_t *cali)
{
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_INT(st);
    int ret = st->put_store_val(STORE_ID_FACTORY_IMU_CALI_DATA, 0, 0, (void *)cali);
    LOG_SENSOR("write: ret=%d", ret);

    return ret;
}

static const imu_cali_data_t *read_imu_cali_data(void)
{
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    return (const imu_cali_data_t *)&st->priv.factory_data.imu;
}

static void imu_irq_hdr_cb(hdr_post_args_t *args)
{
    for (uint8_t i = 0; i < IMU_IRQ_EVENT_CB_MAX; i++) {
        if (thiz->priv.irq_client_cb[i]) {
            thiz->priv.irq_client_cb[i](thiz->priv.irq_client_token[i]);
        }
    }
    thiz->priv.irq_finished = true;
}

static void imu_irq_callback(uint8_t *data)
{
    if (thiz->priv.irq_finished) {
        thiz->priv.irq_finished = false;
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        pm->pm_wakeup(PM_WAKEUP_REASON_IMU_IRQ);
        POST_TASK_HANDLER_DELAYED_ARG1(imu_irq_hdr_cb, 100, thiz, "imu_irq_event");
    }
}

static void add_imu_irq_callback(uint8_t irq_num, void *token, on_imu_irq_callback_fptr cb)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (irq_num < IMU_IRQ_EVENT_CB_MAX) {
        thiz->priv.irq_client_token[irq_num] = token;
        thiz->priv.irq_client_cb[irq_num] = cb;
        LOG_SENSOR("---add token: 0x%lx", (uint32_t)token);
    }
}

static void remove_imu_irq_callback(uint8_t irq_num, void *token)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (irq_num < IMU_IRQ_EVENT_CB_MAX) {
        if (thiz->priv.irq_client_token[irq_num] == token) {
            thiz->priv.irq_client_token[irq_num] = NULL;
            thiz->priv.irq_client_cb[irq_num] = NULL;
            LOG_SENSOR("---remove token: 0x%lx", (uint32_t)token);
        }
    }
}

static void add_imu_irq_cb_hanlder(hdr_post_args_t *args)
{
    rt_device_node_t *dev = thiz->priv.dev;
    if (dev && dev->ioctl) {
        LOG_SENSOR("add irq cb");
        DEVICE_IOCTL_ARGS(thiz->priv.dev, OP_CODE_IMU_ADD_IRQ_CALLBACK, imu_irq_callback);
    }
}

static void imu_irq_enable(void *arg)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.dev);
    DEVICE_IOCTL_NO_RET(thiz->priv.dev, OP_CODE_IMU_ENABLE_IRQ);
}

static void imu_irq_disable(void *arg)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.dev);
    DEVICE_IOCTL_NO_RET(thiz->priv.dev, OP_CODE_IMU_DISABLE_IRQ);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void dispatch_imu_client(sensor_wrapper_data_t *sensor_data, void *user_data)
{
    sensor_evt_client_node_t *node = (sensor_evt_client_node_t *)user_data;
    EXCEPTION_CHECK_VOID(node);
    if (thiz->priv.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

static void imu_polling_handler(void *user_data)
{
    static uint16_t log_time = 0;
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)user_data;
    EXCEPTION_CHECK_VOID(sensor_data);
#if IS_SUPPORT_IMU_FIFO
    if (thiz->priv.dev) {                 
        thiz->priv.polling_cnt++;
        sensor_data->imu_data.un.fifo.data_cnt = 0;
        DEVICE_IOCTL_ARGS(thiz->priv.dev, OP_CODE_IMU_GET_IMU_FIFO_DATA, &sensor_data->imu_data.un.fifo);
    }
#else
    if (thiz->priv.dev) {
        thiz->priv.polling_cnt++;
        DEVICE_READ_ARGS(thiz->priv.dev, &sensor_data->imu_data.un.raw, 12);
    }
#endif
    log_time++;
    if ((log_time % 100) == 0) {
        dump_imu(&sensor_data->imu_data);
    }
}

static int on_post_imu_init(void *ptr)
{
    sensor_manager_context_t *ctx = ptr;
    EXCEPTION_CHECK_INT(ctx);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_INT(pbus);
    thiz->priv.dev = pbus->find_device(ID_DEVICE_IMU);
    ctx->add_sensor_task(ctx, SENSOR_TASK_ID_IMU_POLLING, IMU_POLLING_PERIOD,
        IMU_EVT_POLLING, imu_polling_handler, dispatch_imu_client);
#if IS_SUPPORT_IMU_FIFO
    /// 常驻内存 无需释放
    sensor_data.imu_data.un.fifo.data = u_malloc(US_IMU_FIFO_SIZE);
#endif
    thiz->priv.irq_finished = true;
    imu_irq_enable(ptr);
}

void *register_module_for_imu_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, imu_sensor_context_t);
    ptr->write_calibration = write_imu_calibration;
    ptr->get_cali_data = read_imu_cali_data;
    ptr->add_irq_callback = add_imu_irq_callback;
    ptr->remove_irq_callback = remove_imu_irq_callback;
    ptr->irq_enable = imu_irq_enable;
    ptr->irq_disable = imu_irq_disable;
    thiz = ptr;
    return ADD_SENSOR("imu", ID_SENSOR_IMU, on_post_imu_init, ptr, list_head,
         sizeof(imu_sensor_context_t));
}
#endif