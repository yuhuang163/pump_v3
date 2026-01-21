/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR
#include "sensor_manager.h" // IWYU pragma: keep
#include "light_sensor_module.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static light_sensor_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void dispatch_light_sensor_client(sensor_wrapper_data_t *sensor_data, void *user_data)
{
    sensor_evt_client_node_t *node = (sensor_evt_client_node_t *)user_data;
    EXCEPTION_CHECK_VOID(node);
    // LOG_SENSOR("node->report_type=%d", node->report_type);
    if (thiz->priv.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

static void light_sensor_polling_handler(void *user_data)
{
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)user_data;
    if (thiz->priv.dev) {
        thiz->priv.polling_cnt++;
        DEVICE_READ_ARGS(thiz->priv.dev, &sensor_data->light_sensor_data.lux_level, 1);
    }
}

static int on_post_light_sensor_init(void *ptr)
{
    sensor_manager_context_t *ctx = ptr;
    EXCEPTION_CHECK_INT(ctx);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_INT(pbus);
    thiz->priv.dev = pbus->find_device(ID_DEVICE_LIGHT_SENSOR);
    ctx->add_sensor_task(ctx, SENSOR_TASK_ID_LIGHT_SENSOR_POLLING, LIGHT_SENSOR_POLLING_PERIOD,
        LIGHT_SENSOR_EVT_POLLING, light_sensor_polling_handler, dispatch_light_sensor_client);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_light_sensor_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, light_sensor_context_t);
    thiz = ptr;
    return ADD_SENSOR("light_sensor", ID_SENSOR_LIGHT_SENSOR, on_post_light_sensor_init, 
        ptr, list_head, sizeof(light_sensor_context_t));
}
#endif