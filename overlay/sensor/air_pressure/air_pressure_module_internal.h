#ifndef __AIR_PRESSURE_MODULE_INTERNAL_H__
#define __AIR_PRESSURE_MODULE_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "sensor_manager.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void dispatch_air_pressure_client(sensor_wrapper_data_t *sensor_data, sensor_evt_client_node_t *node);
void air_pressure_polling_handler(sensor_wrapper_data_t *sensor_data);
void on_perpare_air_pressure_init(sensor_manager_context_t *ptr);
#ifdef __cplusplus
}
#endif

#endif 







