/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#if IS_SUPPORT_DEVICE_FSENSOR
#include "sensor_manager.h" 
#include "force_sensor_module.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static force_sensor_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void dispatch_force_sensor_client(sensor_wrapper_data_t *sensor_data, void *user_data)
{
    sensor_evt_client_node_t *node = (sensor_evt_client_node_t *)user_data;
    if (thiz->priv.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

static void force_sensor_polling_handler(void *user_data)
{
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)user_data;
    EXCEPTION_CHECK_VOID(sensor_data);
    if (thiz->priv.dev) {
        thiz->priv.polling_cnt++;
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.dev, OP_CODE_FSENSOR_GET_RAW_DATA, &sensor_data);
    }
}

static int on_post_force_sensor_init(void *ptr)
{
    sensor_manager_context_t *ctx = ptr;
    EXCEPTION_CHECK_INT(ctx);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_INT(pbus);
    thiz->priv.dev = pbus->find_device(ID_DEVICE_FSENSOR);
    ctx->add_sensor_task(ctx, SENSOR_TASK_ID_FSENSOR_POLLING, F_SENSOR_POLLING_PERIOD,
        F_SENSOR_EVT_POLLING, force_sensor_polling_handler, dispatch_force_sensor_client);
    return 0;
}

void *register_module_for_force_sensor_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, force_sensor_context_t);
    thiz = ptr;
    return ADD_SENSOR("force_sensor", ID_SENSOR_FSENSOR, on_post_force_sensor_init, 
        ptr, list_head, sizeof(force_sensor_context_t));
}

#endif