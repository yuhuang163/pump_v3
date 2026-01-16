/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "root_log_define.h"
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
    /// 执行回调.
    if (thiz->priv.air_pressure.polling_cnt - node->old_cnt >= node->step) {
        node->old_cnt = thiz->priv.air_pressure.polling_cnt;
        node->cb(node->user_data, (uint8_t *)sensor_data, sizeof(sensor_wrapper_data_t));
    }
}

void air_pressure_polling_handler(sensor_wrapper_data_t *sensor_data)
{        
    EXECUTE_EVERY_X_DELAY(thiz->priv.air_pressure.sampling_interval, {
        if (thiz->priv.air_pressure.dev) {
            thiz->priv.air_pressure.polling_cnt++;
            DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.air_pressure.dev, OP_CODE_AIR_PRESSURE_GET_STATUS,
                    sensor_data->air_pressure_data);
            sensor_data->air_pressure_serial_num++;
            sensor_data->sensor_mask |= SENSOR_MASK_AIR_PRESSURE;
        }
    });
}

void on_perpare_air_pressure_init(sensor_manager_context_t *ptr)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.air_pressure.dev = pbus->find_device(ID_DEVICE_AIR_PRESSURE);
    ptr->priv.air_pressure.sampling_interval = AIR_PRESSURE_POLLING_PERIOD;
    LOG_SENSOR("air pressure device: %p", ptr->priv.air_pressure.dev);
    thiz = ptr;
}
#endif
