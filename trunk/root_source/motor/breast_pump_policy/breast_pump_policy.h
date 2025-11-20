#ifndef __BREAST_PUMP_POLICY_H__
#define __BREAST_PUMP_POLICY_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "motor_policy.h"

struct motor_manager_context;
struct motor_task_para;

/// 策略集合
motor_policy_manager_t *pump_total_policy_init(motor_policy_cfg_t cfg);
int pump_total_policy_deint(motor_policy_manager_t *total_policy);
/// 定时策略
motor_timed_policy_node_t *pump_time_policy_init(motor_policy_cfg_t cfg);
int pump_time_policy_add(motor_timed_policy_node_t *time_policy, motor_timed_policy_node_t *next_time_policy);
int pump_time_policy_empty(motor_policy_manager_t *total_policy);
int pump_time_policy_update(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime);
/// 策略实体
motor_policy_node_t *pump_policy_init(motor_policy_cfg_t cfg);
int pump_policy_del(motor_timed_policy_node_t *time_policy);
int pump_policy_add(motor_policy_node_t *policy, motor_policy_node_t *next_policy);
int pump_policy_update(motor_policy_info_t *info, uint8_t pump_id);
void pump_policy_info_reset(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime, uint8_t pump_id);
/// 补偿策略
int pump_compensation_policy_update(struct motor_manager_context *thiz, motor_policy_original_para_t *para);
/// 用户策略，力度/频率调节
int pump_user_policy_update(struct motor_manager_context *thiz, suction_strategy_mapping_t *map, uint8_t pump_id);
/// 吸奶策略处理
int breast_pump_mode_policy_handler(struct motor_manager_context *thiz, struct motor_task_para *play_para);
/// 吸奶策略准备
void breast_pump_policy_prepare(struct motor_manager_context *thiz, uint8_t action);
/// 负压释放
void negative_pressure_release(struct motor_manager_context *thiz);
/// 爬升策略
int pump_climb_policy_update(struct motor_manager_context *thiz, suction_strategy_mapping_t *map);

#ifdef __cplusplus
}
#endif

#endif 
