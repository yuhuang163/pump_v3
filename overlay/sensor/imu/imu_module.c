/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
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
static sensor_manager_context_t *thiz = NULL;

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
    int ret = st->put_store_val(STORE_ID_FACTORY_IMU_CALI_DATA, 0, 0, (const void *)cali);
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
    sensor_manager_context_t *ctx = args->user_data;
    for (uint8_t i = 0; i < IMU_IRQ_EVENT_CB_MAX; i++) {
        if (ctx->priv.imu.irq_client_cb[i]) {
            ctx->priv.imu.irq_client_cb[i](ctx->priv.imu.irq_client_token[i]);
        }
    }
    ctx->priv.imu.irq_finished = true;
}

static void imu_irq_callback(uint8_t *data)
{
    sensor_manager_context_t *ctx = thiz;
    if (ctx->priv.imu.irq_finished) {
        ctx->priv.imu.irq_finished = false;
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        pm->pm_wakeup(PM_WAKEUP_REASON_IMU_IRQ);
        POST_TASK_HANDLER_DELAYED_ARG1(imu_irq_hdr_cb, 100, ctx, "imu_irq_event");
    }
}

static void add_imu_irq_callback(uint8_t irq_num, void *token, on_imu_irq_callback_fptr cb)
{
    if (irq_num < IMU_IRQ_EVENT_CB_MAX) {
        sensor_manager_context_t *ctx = thiz;
        ctx->priv.imu.irq_client_token[irq_num] = token;
        ctx->priv.imu.irq_client_cb[irq_num] = cb;
        LOG_SENSOR("---add token: 0x%lx", (uint32_t)token);
    }
}

static void remove_imu_irq_callback(uint8_t irq_num, void *token)
{
    if (irq_num < IMU_IRQ_EVENT_CB_MAX) {
        sensor_manager_context_t *ctx = thiz;
        if (ctx->priv.imu.irq_client_token[irq_num] == token) {
            ctx->priv.imu.irq_client_token[irq_num] = NULL;
            ctx->priv.imu.irq_client_cb[irq_num] = NULL;
            LOG_SENSOR("---remove token: 0x%lx", (uint32_t)token);
        }
    }
}

static void add_imu_irq_cb_hanlder(hdr_post_args_t *args)
{
    sensor_manager_context_t *ctx = args->user_data;
    rt_device_node_t *dev = ctx->priv.imu.dev;
    if (dev && dev->ioctl) {
        LOGI("add irq cb");
        DEVICE_IOCTL_ARGS(dev, OP_CODE_IMU_ADD_IRQ_CALLBACK, imu_irq_callback);
    }
}

static void imu_irq_enable(void *arg)
{
    sensor_manager_context_t *ctx = (sensor_manager_context_t *)arg; 
    DEVICE_IOCTL_NO_RET(ctx->priv.imu.dev, OP_CODE_IMU_ENABLE_IRQ);
}

static void imu_irq_disable(void *arg)
{
    sensor_manager_context_t *ctx = (sensor_manager_context_t *)arg; 
    DEVICE_IOCTL_NO_RET(ctx->priv.imu.dev, OP_CODE_IMU_DISABLE_IRQ);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void dispatch_imu_client(sensor_wrapper_data_t *sensor_data, sensor_evt_client_node_t *node)
{
    if (thiz->priv.imu.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.imu.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

void on_imu_sensor_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    sensor_manager_context_t *ctx = context;
    if (id.code == ID_BROADCAST_SYS_BOOT_COMPLETED) {
        POST_TASK_HANDLER_DELAYED_ARG1(add_imu_irq_cb_hanlder, 2000, ctx, "add_irq_callback:task");
    } else if (id.code == ID_BROADCAST_PM_STATE_CHANGE) {
        imu_irq_enable(ctx);
    }
}

void imu_polling_handler(sensor_wrapper_data_t *sensor_data)
{
    static uint16_t log_time = 0;
    
    EXECUTE_EVERY_X_DELAY(IMU_POLLING_PERIOD, {
        #if IS_SUPPORT_IMU_FIFO
        if (thiz->priv.imu.dev) {                 
            thiz->priv.imu.polling_cnt++;
            sensor_data->imu_data.un.fifo.data_cnt = 0;
            DEVICE_IOCTL_ARGS(thiz->priv.imu.dev, OP_CODE_IMU_GET_IMU_FIFO_DATA, &sensor_data->imu_data.un.fifo);
        }
        #else
        if (thiz->priv.imu.dev) {
            thiz->priv.imu.polling_cnt++;
            DEVICE_READ_ARGS(thiz->priv.imu.dev, &sensor_data->imu_data.un.raw, 12);
        }
        #endif
    });

    log_time++;
    if ((log_time % 100) == 0) {
        dump_imu(&sensor_data->imu_data);
    }
}

void on_post_imu_init(sensor_manager_context_t *ptr)
{
    #if IS_SUPPORT_IMU_FIFO
    //常驻内存 无需释放
    sensor_data.imu_data.un.fifo.data = u_malloc(US_IMU_FIFO_SIZE);
    #endif
    imu_irq_enable(ptr);
}

void on_perpare_imu_init(sensor_manager_context_t *ptr)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.imu.dev = pbus->find_device(ID_DEVICE_IMU);
    ptr->priv.imu.irq_finished = true;
}

void register_imu_module_ctx(sensor_manager_context_t *ptr)
{
    ptr->imu.write_calibration = write_imu_calibration;
    ptr->imu.get_cali_data = read_imu_cali_data;
    ptr->imu.add_irq_callback = add_imu_irq_callback;
    ptr->imu.remove_irq_callback = remove_imu_irq_callback;
    ptr->imu.irq_enable = imu_irq_enable;
    ptr->imu.irq_disable = imu_irq_disable;
    thiz = ptr;
}
