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
static sensor_manager_context_t *thiz = NULL;

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
void dispatch_force_sensor_client(sensor_wrapper_data_t *sensor_data, sensor_evt_client_node_t *node)
{
    if (thiz->priv.f_sensor.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.f_sensor.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

void force_sensor_polling_handler(sensor_wrapper_data_t *sensor_data)
{
    EXECUTE_EVERY_X_DELAY(F_SENSOR_POLLING_PERIOD, {
        if (thiz->priv.f_sensor.dev) {
            thiz->priv.f_sensor.polling_cnt++;
            DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.f_sensor.dev, OP_CODE_FSENSOR_GET_RAW_DATA, &sensor_data);
        }
    });
}

void on_perpare_force_sensor_init(sensor_manager_context_t *ptr)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.f_sensor.dev = pbus->find_device(ID_DEVICE_FSENSOR);
    thiz = ptr;
}
#endif