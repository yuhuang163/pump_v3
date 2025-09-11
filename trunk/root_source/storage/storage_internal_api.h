#ifndef __STORAGE_INTERNAL_API_H__
#define __STORAGE_INTERNAL_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
// 存储接口
int put_store_val(persist_store_id_e id, int arg1, int arg2, const void *val_in);
int get_store_val(persist_store_id_e id, int arg1, int arg2, void *val_out);
int del_store_val(persist_store_id_e id, int arg1, int arg2, const void *val_in);
int get_store_ram_val(persist_store_id_e id);
void put_store_ram_val(persist_store_id_e id, int args);
int user_database_put(storage_context_t *ctx, database_key_t key, const uint8_t *buf, int buf_len);
int user_database_get(storage_context_t *ctx, database_key_t key, uint8_t *buf, int buf_len);
int user_database_remove(storage_context_t *ctx, database_key_t key);
int user_database_reset(storage_context_t *ctx);

storage_context_t *get_storage_manager_context(void);

// 工厂数据接口
int storage_write_imu_cali_data(int arg1, int arg2, const void *val_in);
int storage_read_imu_cali_data(int arg1, int arg2, void *val_in);
int storage_write_factory_dev_info(int arg1, int arg2, const void *val_in);
int storage_read_factory_dev_info(int arg1, int arg2, void *val_in);
int storage_read_factory_flag_info(int arg1, int arg2, void *val_in);

// 工厂标志信息子项接口
int storage_write_factory_deploy_flag(int arg1, int arg2, const void *val_in);
int storage_write_factory_age_flag(int arg1, int arg2, const void *val_in);
int storage_write_factory_age_display_flag(int arg1, int arg2, const void *val_in);
int storage_write_factory_fsensor_cali_flag(int arg1, int arg2, const void *val_in);

// 工厂标志信息子项接口
int storage_read_factory_deploy_flag(void);
int storage_read_factory_age_flag(void);
int storage_read_factory_age_display_flag(void);
int storage_read_factory_fsensor_cali_flag(void);

// 系统配置接口
void storage_reset_sys_config_info(sys_config_info_t *sys_config);
int storage_write_sys_config_info(int arg1, int arg2, const void *val_in);
int storage_read_sys_config_info(int arg1, int arg2, void *val_in);

// 系统状态接口
void storage_reset_sys_state_info(sys_state_info_t *sys_state);
int storage_write_sys_state_info(int arg1, int arg2, const void *val_in);
int storage_read_sys_state_info(int arg1, int arg2, void *val_in);

// UTC接口
int storage_write_utc_timestamp(int arg1, int arg2, const void *val_in);
int storage_read_utc_timestamp(int arg1, int arg2, void *val_in);

// 系统配置子项接口
void storage_write_sys_time_zone(int args);
void storage_write_ap_mode_onoff(int args);
void storage_write_slient_upgrade_onoff(int args);
void storage_write_sys_ui_style(int args);
void storage_write_log_switch(int args);
void storage_write_language(int args);
void storage_write_sound_mode_onoff(int args);
void storage_write_microseism_onoff(int args);
void storage_write_ai_mode_onoff(int args);

// 系统配置子项接口
int storage_read_sys_time_zone(void);
int storage_read_ap_mode_onoff(void);
int storage_read_slient_upgrade_onoff(void);
int storage_read_sys_ui_style(void);
int storage_read_log_switch(void);
int storage_read_language(void);
int storage_read_sound_mode_onoff(void);
int storage_read_microseism_onoff(void);
int storage_read_ai_mode_onoff(void);

// 系统状态子项接口
void storage_write_guide_deployed(int args);
void storage_write_battery_health(int args);
void storage_write_brush_header_life(int args);
void storage_write_sync_sys_time(int args);
void storage_write_log_filter(int args);

// 系统状态子项接口
int storage_read_guide_deployed(void);
int storage_read_battery_health(void);
int storage_read_sync_sys_time(void);
int storage_read_log_filter(void);
int storage_user_is_binded(void);

// 用户绑定信息接口
void storage_reset_user_bind_info(user_bind_info_t *user_bind);
int storage_write_user_bind_info(int arg1, int arg2, const void *val_in);
int storage_read_user_bind_info(int arg1, int arg2, void *val_in);

// 系统配置子项接口
int init_system_persist_info(storage_context_t *ctx);
int system_persist_info_sync_flash(storage_context_t *ctx);

// 工厂数据接口
int init_factory_persist_info(storage_context_t *ctx);

// 系统版本信息接口
int init_system_ver_info(storage_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif 







