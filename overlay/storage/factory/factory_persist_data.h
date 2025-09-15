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
storage_manager_context_t *get_storage_manager_context(void);

/// 工厂数据接口
#if IS_SUPPORT_DEVICE_IMU
int storage_write_imu_cali_data(store_func_args_t *args);
int storage_read_imu_cali_data(store_func_args_t *args);
#endif
int storage_write_factory_dev_info(store_func_args_t *args);
int storage_read_factory_dev_info(store_func_args_t *args);
int storage_read_factory_flag_info(store_func_args_t *args);

/// 工厂标志信息子项接口
int storage_write_factory_deploy_flag(store_func_args_t *args);
int storage_write_factory_age_flag(store_func_args_t *args);
int storage_write_factory_age_display_flag(store_func_args_t *args);
#if IS_SUPPORT_DEVICE_FSENSOR
int storage_write_factory_fsensor_cali_flag(store_func_args_t *args);
#endif

/// 工厂标志信息子项接口
int storage_read_factory_deploy_flag(store_func_args_t *args);
int storage_read_factory_age_flag(store_func_args_t *args);
int storage_read_factory_age_display_flag(store_func_args_t *args);
#if IS_SUPPORT_DEVICE_FSENSOR
int storage_read_factory_fsensor_cali_flag(store_func_args_t *args);
#endif

/// 系统配置接口
void storage_reset_sys_config_info(sys_config_info_t *sys_config);
int storage_write_sys_config_info(store_func_args_t *args);
int storage_read_sys_config_info(store_func_args_t *args);

/// 系统状态接口
void storage_reset_sys_state_info(sys_state_info_t *sys_state);
int storage_write_sys_state_info(store_func_args_t *args);
int storage_read_sys_state_info(store_func_args_t *args);

/// UTC接口
int storage_write_utc_timestamp(store_func_args_t *args);
int storage_read_utc_timestamp(store_func_args_t *args);

/// 系统配置子项接口
int storage_write_sys_time_zone(store_func_args_t *args);
int storage_write_ap_mode_onoff(store_func_args_t *args);
int storage_write_slient_upgrade_onoff(store_func_args_t *args);
int storage_write_sys_ui_style(store_func_args_t *args);
int storage_write_log_switch(store_func_args_t *args);
int storage_write_language(store_func_args_t *args);
int storage_write_sound_mode_onoff(store_func_args_t *args);
int storage_write_microseism_onoff(store_func_args_t *args);
int storage_write_ai_mode_onoff(store_func_args_t *args);

/// 系统配置子项接口
int storage_read_sys_time_zone(store_func_args_t *args);
int storage_read_ap_mode_onoff(store_func_args_t *args);
int storage_read_slient_upgrade_onoff(store_func_args_t *args);
int storage_read_sys_ui_style(store_func_args_t *args);
int storage_read_log_switch(store_func_args_t *args);
int storage_read_language(store_func_args_t *args);
int storage_read_sound_mode_onoff(store_func_args_t *args);
int storage_read_microseism_onoff(store_func_args_t *args);
int storage_read_ai_mode_onoff(store_func_args_t *args);

/// 系统状态子项接口
int storage_write_guide_deployed(store_func_args_t *args);
int storage_write_battery_health(store_func_args_t *args);
int storage_write_sync_sys_time(store_func_args_t *args);
int storage_write_log_filter(store_func_args_t *args);

/// 系统状态子项接口
int storage_read_guide_deployed(store_func_args_t *args);
int storage_read_battery_health(store_func_args_t *args);
int storage_read_sync_sys_time(store_func_args_t *args);
int storage_read_log_filter(store_func_args_t *args);
int storage_user_is_binded(store_func_args_t *args);

/// 用户绑定信息接口
void storage_reset_user_bind_info(user_bind_info_t *user_bind);
int storage_write_user_bind_info(store_func_args_t *args);
int storage_read_user_bind_info(store_func_args_t *args);

/// 系统配置子项接口
int init_system_persist_info(storage_manager_context_t *ctx);
int system_persist_info_sync_flash(storage_manager_context_t *ctx);

/// 工厂数据接口
int init_factory_persist_info(storage_manager_context_t *ctx);

/// 系统版本信息接口
int init_system_ver_info(storage_manager_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif 







