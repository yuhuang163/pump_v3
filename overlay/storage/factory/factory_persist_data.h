#ifndef __FACTORY_PERSIST_DATA_H__
#define __FACTORY_PERSIST_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/

/// 工厂数据接口
#if IS_SUPPORT_DEVICE_IMU
int write_imu_cali_data_to_flash(store_func_args_t args);
int read_imu_cali_data_from_flash(store_func_args_t args);
#endif
int write_factory_dev_info_to_flash(store_func_args_t args);
int read_factory_dev_info_from_flash(store_func_args_t args);
int write_factory_flag_info_to_flash(store_func_args_t args);
int read_factory_flag_info_from_flash(store_func_args_t args);

/// 工厂标志信息子项接口
int write_factory_deploy_flag_to_flash(store_func_args_t args);
int write_factory_age_flag_to_flash(store_func_args_t args);
int write_factory_age_display_flag_to_flash(store_func_args_t args);
#if IS_SUPPORT_DEVICE_FSENSOR
int write_factory_fsensor_cali_flag_to_flash(store_func_args_t args);
#endif

/// 工厂标志信息子项接口
int read_factory_deploy_flag_from_ram(store_func_args_t args);
int read_factory_age_flag_from_ram(store_func_args_t args);
int read_factory_age_display_flag_from_ram(store_func_args_t args);
#if IS_SUPPORT_DEVICE_FSENSOR
int read_factory_fsensor_cali_flag_from_ram(store_func_args_t args);
#endif

#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
int read_factory_light_sensor_calib_data_from_ram(store_func_args_t args);
#endif

int read_factory_suction_table_from_flash(store_func_args_t args);
int read_factory_suction_table_from_ram(store_func_args_t args);

int read_factory_motor_type_from_ram(store_func_args_t args);
int read_factory_battery_type_from_ram(store_func_args_t args);

int init_factory_persist_info(storage_manager_context_t *ctx);
#ifdef __cplusplus
}
#endif

#endif 







