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
    /// 打印定时策略参数
    //LOG_MOTOR("time policy: duration:%d, freq:%d, level:%d, mode_type:%d, loop_num:%d", 
    //    policy_obj->duration_seconds, cfg.freq, cfg.level, cfg.mode_type, cfg.loop_num);

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
    //LOG_MOTOR("mode_type:%d", mode_type);
    return PUMP_MODE_IDLE;
}

void policy_remaining_time_update(motor_policy_runtime_t *runtime) 
{
    static uint32_t last_log_time = 0;
    EXCEPTION_CHECK_VOID(runtime);
    uint32_t running_time = runtime->policy_running_time_ms / 1000;
    if (runtime->time_policy_target_time <= running_time) {
        runtime->time_policy_remaining_time = 0;
    } else {
        runtime->time_policy_remaining_time = runtime->time_policy_target_time - running_time;
    }  

    if (get_timestamp_ms(last_log_time) > 2000) {
        last_log_time = get_timestamp();
        LOG_MOTOR("mode:%d, index:%d, time:%d/%d/%d", 
            runtime->breast_pump_mode, runtime->current_timed_policy_index,
            runtime->policy_running_time_ms, runtime->time_policy_target_time, runtime->time_policy_remaining_time);
    }
}

int pump_time_policy_update(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime) 
{     
    policy_remaining_time_update(runtime);
    if (runtime->time_policy_remaining_time == 0) {
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            if (runtime->info[i].state.policy_update_lock == 1) {
                return 0; /// 等待所有泵空闲后切换到下一个时间策略
            }
        }

        if (runtime->timed_policy->next) {
            runtime->current_timed_policy_index++;
            runtime->timed_policy = runtime->timed_policy->next;
            runtime->time_policy_target_time += runtime->timed_policy->duration_seconds;
            runtime->time_policy_remaining_time = runtime->time_policy_target_time - 
                        (runtime->policy_running_time_ms / 1000);
            runtime->current_mode_type = pump_policy_type_get(runtime->timed_policy->policy); 
            if (runtime->timed_policy->next) {
                runtime->next_mode_type = pump_policy_type_get(runtime->timed_policy->next->policy);
            } else {
                runtime->next_mode_type = runtime->current_mode_type;
            }

            for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
                memcpy(&runtime->policy[i], &runtime->timed_policy->policy, sizeof(motor_policy_node_t));
                pump_policy_info_reset(thiz, runtime, i);
            }

            if (!runtime->taste_run_flag) {
                if (thiz->priv.pump_ctx) {
                    thiz->priv.pump_ctx->priv.m_info.step_index = runtime->current_timed_policy_index;
                }
                thiz->notify_motor_status_change(MOTOR_STATUS_EVT_PUMP_STEP_ALL_PARAM_CHANGE);
            }
        } else {
            runtime->all_timed_policy_finish_flag = 1; /// 所有定时策略执行完毕
        }

        if (runtime->timed_policy->policy.core.type == MOTOR_PARAM_TABLE) {
            suction_strategy_mapping_t *mapping = &runtime->timed_policy->policy.core.un.mapping;
            LOG_MOTOR("mode:%d, index:%d, type:%d/%d, time:%d/%d, mapping:%d(%d/%d)", 
                runtime->breast_pump_mode, runtime->current_timed_policy_index,
                runtime->current_mode_type, runtime->next_mode_type,
                runtime->time_policy_remaining_time, runtime->timed_policy->duration_seconds, 
                mapping->mode_type, mapping->level, mapping->speed);
        }
    } else {
#if IS_SUPPORT_PUMP_SYNC
        runtime->freq_sync_flag = true;   
#if 0
        /// 同频同步
        for (uint8_t i = 1; i < BREAST_PUMP_NUM; i++) {
            if (runtime->pump_freq[i] != runtime->pump_freq[0] && runtime->current_mode_type != PUMP_MODE_MIX) {
                runtime->freq_sync_flag = false;
                break;
            }
        }
#else
        /// 同档同步
        for (uint8_t i = 1; i < BREAST_PUMP_NUM; i++) {
            if (runtime->pump_level[i] != runtime->pump_level[0] || runtime->pump_freq[i] != runtime->pump_freq[0]) {
                runtime->freq_sync_flag = false;
                break;
            }
        }
#endif
        if (runtime->freq_sync_flag) {
            for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
                if (runtime->info[i].state.policy_update_lock) {
                    return 0;
                }
            }
        }
#endif
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            if (runtime->info[i].state.policy_update_lock == 0) {
                if (runtime->info[i].state.cycles_count < runtime->info[i].config.loop_num) {
                    runtime->info[i].state.status = SUCTION_STATUS_IDLE;
                    runtime->info[i].state.action = MOTOR_ACTION_IDLE;
                    LOG_MOTOR("dev-%d policy loop, cycles:%d/%d", 
                        i, runtime->info[i].state.cycles_count, runtime->info[i].config.loop_num);
                } else {
                    if (runtime->policy[i].next) {
                        memcpy(&runtime->policy[i], runtime->policy[i].next, sizeof(motor_policy_node_t)); 
                    } else {
                        memcpy(&runtime->policy[i], &runtime->timed_policy->policy, sizeof(motor_policy_node_t));  
                    }
                    pump_policy_info_reset(thiz, runtime, i);
                }
            }
        }
    }
    return 0;
}