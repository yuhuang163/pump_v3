/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
#include "sensor_manager.h" 
#include "air_pressure_module.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static air_pressure_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void dispatch_air_pressure_client(sensor_wrapper_data_t *sensor_data, void *user_data)
{
    sensor_evt_client_node_t *node = (sensor_evt_client_node_t *)user_data;
    EXCEPTION_CHECK_VOID(node);
    if (thiz->priv.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

static void air_pressure_polling_handler(void *user_data)
{   
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)user_data;
    EXCEPTION_CHECK_VOID(sensor_data);
    if (thiz->priv.dev) {
        thiz->priv.polling_cnt++;
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.dev, OP_CODE_AIR_PRESSURE_GET_STATUS,
                sensor_data->air_pressure_data);
        sensor_data->air_pressure_serial_num++;
        sensor_data->sensor_mask |= SENSOR_MASK_AIR_PRESSURE;
    }
}

static uint16_t get_air_pressure_sampling_interval(void)
{
    return thiz->priv.sampling_interval;
}

static void set_air_pressure_sampling_interval(uint16_t interval)
{
    if (thiz->priv.sampling_interval != interval) {
        thiz->priv.sampling_interval = interval;
        DEVICE_IOCTL_HAS_RET(thiz->priv.dev, OP_CODE_AIR_PRESSURE_IS_SUPPORT_AIR_PRESSURE, support_air_pressure);
        if (support_air_pressure <= 0) {
            LOG_SENSOR("air pressure is not supported, skip set sampling interval");
            return;
        }
        sensor_manager_context_t *ctx = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
        EXCEPTION_CHECK_VOID(ctx);
        ctx->remove_sensor_task(air_pressure_polling_handler);
        ctx->add_sensor_task(ctx, SENSOR_TASK_ID_AIR_PRESSURE_POLLING, thiz->priv.sampling_interval,
            AIR_PRESSURE_EVT_POLLING, air_pressure_polling_handler, dispatch_air_pressure_client);
        LOG_SENSOR("set_air_pressure_sampling_interval: %d", thiz->priv.sampling_interval);
    }
}

static int on_post_air_pressure_init(void *ptr)
{
    sensor_manager_context_t *ctx = ptr;
    EXCEPTION_CHECK_INT(ctx);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_INT(pbus);
    thiz->priv.dev = pbus->find_device(ID_DEVICE_AIR_PRESSURE);
    thiz->priv.sampling_interval = AIR_PRESSURE_POLLING_PERIOD;
    DEVICE_IOCTL_HAS_RET(thiz->priv.dev, OP_CODE_AIR_PRESSURE_IS_SUPPORT_AIR_PRESSURE, support_air_pressure);
    if (support_air_pressure <= 0) {
        LOG_SENSOR("air pressure is not supported, skip init");
        return 0;
    }
    ctx->add_sensor_task(ctx, SENSOR_TASK_ID_AIR_PRESSURE_POLLING, thiz->priv.sampling_interval,
        AIR_PRESSURE_EVT_POLLING, air_pressure_polling_handler, dispatch_air_pressure_client);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_air_pressure_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, air_pressure_context_t);
    ptr->get_sampling_interval = get_air_pressure_sampling_interval;
    ptr->set_sampling_interval = set_air_pressure_sampling_interval;
    thiz = ptr;
    return ADD_SENSOR("air_pressure", ID_SENSOR_AIR_PRESSURE, on_post_air_pressure_init, 
        ptr, list_head, sizeof(air_pressure_context_t));
}
#endif
