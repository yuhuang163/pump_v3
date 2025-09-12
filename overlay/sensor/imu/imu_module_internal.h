#ifndef __IMU_MODULE_INTERNAL_H__
#define __IMU_MODULE_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void on_perpare_imu_init(sensor_manager_context_t *ptr);
void on_post_imu_init(sensor_manager_context_t *ptr);
void register_imu_module_ctx(sensor_manager_context_t *ptr);
void imu_polling_handler(sensor_wrapper_data_t *sensor_data);
void dispatch_imu_client(sensor_wrapper_data_t *sensor_data, sensor_evt_client_node_t *node);
void on_imu_sensor_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata);

#ifdef __cplusplus
}
#endif

#endif 







