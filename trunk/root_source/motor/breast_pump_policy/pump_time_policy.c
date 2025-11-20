/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_policy.h"

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

int pump_time_policy_update(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime) 
{ 
    /// 检测当前时间策略是否执行完毕
    uint32_t hold_time = get_timestamp_ms(runtime->timed_policy_start_time)  / 1000; 
    if (hold_time >= runtime->timed_policy->duration_seconds) {
        /// 执行下一个时间策略
        runtime->timed_policy = runtime->timed_policy->next == NULL ? \
                &runtime->manager->head : runtime->timed_policy->next;
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
        return 0;
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
                // LOG_MOTOR("dev-%d goto next policy", id);
            } else {
                memcpy(&runtime->policy[i], &runtime->timed_policy->policy, sizeof(motor_policy_node_t)); /// 切换到第一个子策略 
                // LOG_MOTOR("dev-%d goto first policy", id);
            }
            pump_policy_info_reset(thiz, runtime, i);
        }
    }
    return 0;
}