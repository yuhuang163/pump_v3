/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_policy.h"
#include <stdint.h>

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
motor_timed_policy_node_t *pump_time_policy_init(motor_policy_cfg_t cfg) 
{
    motor_timed_policy_node_t *policy_obj = u_malloz(sizeof(motor_timed_policy_node_t));
    if (policy_obj == NULL) {
        LOGE("policy malloc fail !!!");
        return NULL;
    }

    motor_policy_core_t *m_para = &policy_obj->policy.core;
    policy_obj->sub_policy_count = 1;
    policy_obj->duration_seconds = cfg.duration;
    memcpy(m_para, &cfg.core, sizeof(motor_policy_core_t));
    return policy_obj;
}

motor_timed_policy_node_t *pump_time_policy_init_by_steps_info(pump_mode_step_param_t cfg) 
{
    motor_timed_policy_node_t *policy_obj = u_malloz(sizeof(motor_timed_policy_node_t));
    if (policy_obj == NULL) {
        LOGE("policy malloc fail !!!");
        return NULL;
    }
    motor_policy_core_t *m_para = &policy_obj->policy.core;
    policy_obj->sub_policy_count = 1;
    policy_obj->duration_seconds = cfg.duration;
    m_para->type = MOTOR_PARAM_TABLE;
    m_para->un.mapping.speed = cfg.freq;
    m_para->un.mapping.level = cfg.level;
    m_para->un.mapping.mode_type = cfg.mode_type;
    m_para->config.loop_num = cfg.loop_num;

    return policy_obj;
}

int pump_time_policy_add(motor_timed_policy_node_t *time_policy, motor_timed_policy_node_t *next_time_policy) 
{
    if (time_policy == NULL || next_time_policy == NULL) {
        return -1;       
    }
    time_policy->next = next_time_policy;
    return 0;
}

int pump_time_policy_empty(motor_policy_manager_t *total_policy) 
{ 
    motor_timed_policy_node_t *next_policy = NULL;
    motor_timed_policy_node_t *current_policy = total_policy->head.next;
    EXCEPTION_CHECK_INT(total_policy);
    for (;;) {
        if (total_policy->timed_policy_count <= 1 || current_policy == NULL) {
            break;
        }
        next_policy = current_policy->next;
        /// 删除定时策略前先删除该定时策略下的子策略
        pump_policy_del(current_policy);
        /// 删除定时策略
        u_free(current_policy);
        current_policy = next_policy;
        total_policy->timed_policy_count--;
    }
    return 0;
}

uint8_t pump_policy_type_get(motor_policy_node_t policy)
{
    uint8_t mode_type = policy.core.un.mapping.mode_type;
    if (mode_type == SUCTION_MODE_SHORT) {
        return PUMP_MODE_STIMULATION;
    } else if (mode_type == SUCTION_MODE_LONG) {
        return PUMP_MODE_LACTATION;
    } else if (mode_type == SUCTION_MODE_MIX_LONG || mode_type == SUCTION_MODE_MIX_SHORT) {
        return PUMP_MODE_MIX;
    }
    return PUMP_MODE_IDLE;
}

int pump_time_policy_update(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime) 
{ 
    /// 检测当前时间策略是否执行完毕
    uint32_t hold_time = (get_timestamp_ms(runtime->timed_policy_start_time) + \
                          runtime->time_policy_running_time) / 1000; 
                          
    if (hold_time >= runtime->timed_policy->duration_seconds) {
        /// 执行下一个时间策略
        if (runtime->timed_policy->next) {
            runtime->current_timed_policy_index++;
            runtime->timed_policy = runtime->timed_policy->next;
            runtime->time_policy_running_time = 0;
        } else {
            runtime->current_timed_policy_index = 0;
            runtime->timed_policy = &runtime->manager->head;
            runtime->time_policy_running_time = 0;
            runtime->all_timed_policy_finish_flag = 1;
        }
        runtime->time_policy_remaining_time = runtime->timed_policy->duration_seconds;
        /// 更新模式类型
        runtime->current_mode_type = pump_policy_type_get(runtime->timed_policy->policy);
        if (runtime->timed_policy->next) {  
            runtime->next_mode_type = pump_policy_type_get(runtime->timed_policy->next->policy);
        } else {
            runtime->next_mode_type = pump_policy_type_get(runtime->manager->head.policy);
        }
        LOG_MOTOR("mode %d type cur: %d, next: %d, index: %d", runtime->breast_pump_mode,
             runtime->current_mode_type, runtime->next_mode_type, runtime->current_timed_policy_index);
        /// 更新步数索引.
        if (thiz->priv.pump_ctx && (!runtime->taste_run_flag)) {
            thiz->priv.pump_ctx->priv.m_info.step_index = runtime->current_timed_policy_index;
        }
        /// 释放负压
        negative_pressure_release(thiz);
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            memcpy(&runtime->policy[i], &runtime->timed_policy->policy, sizeof(motor_policy_node_t));
            pump_policy_info_reset(thiz, runtime, i);
        }
        runtime->timed_policy_start_time = get_timestamp();
        LOG_MOTOR("goto next time policy, time:%d type:%d, next:%p, duration:%d, level:%d", 
            runtime->timed_policy_start_time, runtime->timed_policy->policy.core.un.mapping.mode_type, 
            runtime->timed_policy->next, runtime->timed_policy->duration_seconds, 
            runtime->timed_policy->policy.core.un.mapping.level);

        if (!runtime->taste_run_flag) {
            /// 每一步定时策略更新后，将当前的策略的setting通知app
            servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
            EXCEPTION_CHECK_INT(servers_ctx && servers_ctx->hook);
            servers_publish_data_args_t args = {
                .arg1 = OLD_PUBLISH_DATA_PUMP_STEP_ALL_PARAM_CHANGE,
                .arg2 = 0,
                .val = NULL,
            };
            servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);
        }
        return 0;
    } else {
        runtime->time_policy_remaining_time = runtime->timed_policy->duration_seconds - hold_time;
    }

    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        /// 检测到子策略切换请求
        if (runtime->info[i].state.policy_update_req) {
            /// 循环执行当前子策略
            if (runtime->info[i].state.cycles_count < runtime->info[i].config.loop_num) {
                runtime->info[i].state.status = SUCTION_STATUS_IDLE;
                runtime->info[i].state.action = MOTOR_ACTION_IDLE;
                runtime->info[i].state.policy_update_req = 0;
                // LOG_MOTOR("dev-%d policy loop", id);
                return 0;
            }

            if (runtime->policy[i].next) {
                memcpy(&runtime->policy[i], runtime->policy[i].next, sizeof(motor_policy_node_t)); /// 切换到下一个子策略
                runtime->current_policy_index++;
                // LOG_MOTOR("dev-%d goto next policy", id);
            } else {
                memcpy(&runtime->policy[i], &runtime->timed_policy->policy, sizeof(motor_policy_node_t)); /// 切换到第一个子策略 
                runtime->current_policy_index = 0;
                // LOG_MOTOR("dev-%d goto first policy", id);
            }
            pump_policy_info_reset(thiz, runtime, i);
        }
    }
    return 0;
}