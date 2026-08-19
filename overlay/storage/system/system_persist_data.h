#ifndef __SYSTEM_PERSIST_DATA_H__
#define __SYSTEM_PERSIST_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "storage_manager_api.h" // IWYU pragma: keep
#include "persist_storage.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 系统配置接口
void reset_sys_config_info(storage_manager_context_t *ctx, sys_config_info_t *sys_config);
int write_sys_config_info_to_flash(store_func_args_t args);
int read_sys_config_info_from_flash(store_func_args_t args);
/// 系统状态接口
void reset_sys_state_info(storage_manager_context_t *ctx, sys_state_info_t *sys_state);
int write_sys_state_info_to_flash(store_func_args_t args);
int read_sys_state_info_from_flash(store_func_args_t args);
/// 用户绑定接口
void reset_user_bind_info(storage_manager_context_t *ctx, user_bind_info_t *user_bind);
int write_user_bind_info_to_flash(store_func_args_t args);
int read_user_bind_info_from_flash(store_func_args_t args);
/// 绑定状态（落盘复用 USER_BIND_INFO）
int write_bind_status_to_flash(store_func_args_t args);
int read_bind_status_from_flash(store_func_args_t args);
int user_is_binded_from_ram(store_func_args_t args);
/// UTC时间接口
int write_utc_timestamp_to_flash(store_func_args_t args);
int read_utc_timestamp_from_flash(store_func_args_t args);
/// 系统重启原因
int write_sys_reboot_reason_to_flash(store_func_args_t args);
int read_sys_reboot_reason_from_flash(store_func_args_t args);
int write_sys_reboot_reason_to_ram(store_func_args_t args);
int read_sys_reboot_reason_from_ram(store_func_args_t args);
/// 系统时区
int write_sys_time_zone_to_ram(store_func_args_t args);
int read_sys_time_zone_from_ram(store_func_args_t args);
/// 飞行/旅行锁
int write_ap_mode_onoff_to_ram(store_func_args_t args);
int read_ap_mode_onoff_from_ram(store_func_args_t args);
/// 飞行模式状态
int write_airplane_mode_state_to_ram(store_func_args_t args);
int read_airplane_mode_state_from_ram(store_func_args_t args);
/// 静默升级
int write_slient_upgrade_onoff_to_ram(store_func_args_t args);
int read_slient_upgrade_onoff_from_ram(store_func_args_t args);
/// UI风格(主题)
int write_sys_ui_style_to_ram(store_func_args_t args);
int read_sys_ui_style_from_ram(store_func_args_t args);
/// 日志开关
int write_log_switch_to_ram(store_func_args_t args);
int read_log_switch_from_ram(store_func_args_t args);
/// 系统语言
int write_language_to_ram(store_func_args_t args);
int read_language_from_ram(store_func_args_t args);
/// UI部署(引导)
int write_guide_deployed_to_ram(store_func_args_t args);
int read_guide_deployed_from_ram(store_func_args_t args);
/// 电池健康状态
int write_battery_health_to_ram(store_func_args_t args);
int read_battery_health_from_ram(store_func_args_t args);
/// 上一次数据同步时间
int write_sync_sys_time_to_ram(store_func_args_t args);
int read_sync_sys_time_from_ram(store_func_args_t args);
/// 日志过滤
int write_log_filter_to_ram(store_func_args_t args);
int read_log_filter_from_ram(store_func_args_t args);
/// 自适应调节总开关
int write_global_adaptive_switch_to_ram(store_func_args_t args);
int read_global_adaptive_switch_from_ram(store_func_args_t args);
#if IS_SUPPORT_NIGHT_LIGHT
int write_night_light_gear_to_ram(store_func_args_t args);
int read_night_light_gear_from_ram(store_func_args_t args);
#endif
int write_back_light_level_to_ram(store_func_args_t args);
int read_back_light_level_from_ram(store_func_args_t args);
/// 绑定状态
int write_bind_status_to_ram(store_func_args_t args);
int read_bind_status_from_ram(store_func_args_t args);
/// 电池电量
int write_battery_percent_to_ram(store_func_args_t args);
int read_battery_percent_from_ram(store_func_args_t args);

#ifdef __cplusplus
}
#endif

#endif 







