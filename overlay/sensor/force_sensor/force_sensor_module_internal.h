#ifndef __FORCE_SENSOR_MODULE_INTERNAL_H__
#define __FORCE_SENSOR_MODULE_INTERNAL_H__

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
void dispatch_force_sensor_client(sensor_wrapper_data_t *sensor_data, sensor_evt_client_node_t *node);
void force_sensor_polling_handler(sensor_wrapper_data_t *sensor_data);
void on_perpare_force_sensor_init(sensor_manager_context_t *ptr);
#ifdef __cplusplus
}
#endif

#endif 







