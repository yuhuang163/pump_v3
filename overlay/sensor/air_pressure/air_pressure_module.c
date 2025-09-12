/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
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
void dispatch_air_pressure_client(sensor_wrapper_data_t *sensor_data, sensor_evt_client_node_t *node)
{
    // if (thiz->priv.air_pressure.polling_cnt - node->old_cnt >= node->step) {
    //     node->old_cnt = thiz->priv.air_pressure.polling_cnt;
    //     node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    // }
}

void force_sensor_polling_handler(sensor_wrapper_data_t *sensor_data)
{
    // EXECUTE_EVERY_X_DELAY(AIR_PRESSURE_POLLING_PERIOD, {
    //     if (thiz->priv.air_pressure.dev) {
    //         thiz->priv.air_pressure.polling_cnt++;
    //         DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.air_pressure.dev, OP_CODE_AIR_PRESSURE_GET_RAW_DATA, &sensor_data);
    //     }
    // });
}

void on_perpare_force_sensor_init(sensor_manager_context_t *ptr)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.air_pressure.dev = pbus->find_device(ID_DEVICE_AIR_PRESSURE);
    thiz = ptr;
}