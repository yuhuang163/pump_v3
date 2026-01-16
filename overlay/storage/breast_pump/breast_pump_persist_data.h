#ifndef __BREAST_PUMP_PERSIST_DATA_H__
#define __BREAST_PUMP_PERSIST_DATA_H__

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
/// 泵奶配置接口
void reset_pump_info(storage_manager_context_t *ctx, sys_pump_info_t *pump_info);
int write_pump_info_to_flash(store_func_args_t args);
int read_pump_info_from_flash(store_func_args_t args);

/// 泵奶数据子项接口
void reset_pump_data_head(storage_manager_context_t *ctx, breast_pump_data_head_t *pump_data_head);
int write_pump_data_head_to_flash(store_func_args_t args);
int read_pump_data_head_from_flash(store_func_args_t args);
int write_pump_data_body_to_flash(store_func_args_t args);
int read_pump_data_body_from_flash(store_func_args_t args);
int del_pump_data_body_from_flash(store_func_args_t args);

/// 泵奶设置信息接口
int write_pump_setting_info_to_ram(store_func_args_t args);
int read_pump_setting_info_from_ram(store_func_args_t args);

/// 泵奶自定义步骤参数接口
int write_pump_custom_steps_parameter_to_ram(store_func_args_t args);
int read_pump_custom_steps_parameter_from_ram(store_func_args_t args);

/// 泵奶时间信息接口
int write_pump_time_to_ram(store_func_args_t args);
int read_pump_time_from_ram(store_func_args_t args);

/// 泵奶异常提醒信息接口
int write_pump_exception_remind_to_ram(store_func_args_t args);
int read_pump_exception_remind_from_ram(store_func_args_t args);

/// 泵奶模式切换提醒信息接口
int write_pump_mode_change_remind_to_ram(store_func_args_t args);
int read_pump_mode_change_remind_from_ram(store_func_args_t args);

/// 泵奶完成提醒信息接口
int write_pump_finish_remind_to_ram(store_func_args_t args);
int read_pump_finish_remind_from_ram(store_func_args_t args);

/// 泵奶控制类型接口
int write_pump_control_type_to_ram(store_func_args_t args);
int read_pump_control_type_from_ram(store_func_args_t args);

/// 泵奶数据同步接口
int pump_data_info_sync_flash(storage_manager_context_t *ctx, pump_result_info_t *body);
/// 获取泵奶数据接口
int get_pump_data_info_from_flash(int num, void *body);
/// 删除泵奶数据接口
int delete_pump_data_info_from_flash(int num);
/// 填充泵奶数据接口
int fill_in_pump_data_info(int num);

/// 埋点数据接口
void reset_pump_buried_info(storage_manager_context_t *ctx, sys_buried_info_list_t *buried_info_list);
int write_pump_buried_info_to_flash(store_func_args_t args);
int read_pump_buried_info_from_flash(store_func_args_t args);
int write_pump_buried_info_to_ram(store_func_args_t args);

#ifdef __cplusplus
}
#endif

#endif 







