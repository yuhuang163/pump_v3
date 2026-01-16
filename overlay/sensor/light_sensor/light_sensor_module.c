/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR
#include "sensor_manager.h" // IWYU pragma: keep
#include "light_sensor_module_internal.h" // IWYU pragma: keep
#include "light_sensor_module.h" // IWYU pragma: keep

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
void dispatch_light_sensor_client(sensor_wrapper_data_t *sensor_data, sensor_evt_client_node_t *node)
{
    if (thiz->priv.light_sensor.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.light_sensor.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

void light_sensor_polling_handler(sensor_wrapper_data_t *sensor_data)
{
    static uint16_t log_time = 0;
    
    EXECUTE_EVERY_X_DELAY(LIGHT_SENSOR_POLLING_PERIOD, {
        if (thiz->priv.light_sensor.dev) {
            thiz->priv.light_sensor.polling_cnt++;
            DEVICE_READ_ARGS(thiz->priv.light_sensor.dev, &sensor_data->light_sensor_data.lux_level, 1);
            // LOG_SENSOR("light_sensor_data.lux_level: %d", sensor_data->light_sensor_data.lux_level);
        }
    });
}

void on_perpare_light_sensor_init(sensor_manager_context_t *ptr)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.light_sensor.dev = pbus->find_device(ID_DEVICE_LIGHT_SENSOR);
    thiz = ptr;
}

#endif