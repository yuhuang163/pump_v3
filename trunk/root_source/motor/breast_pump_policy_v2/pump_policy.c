/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pump_parameters_table.h" 
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_policy.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_RUN_CHECK_ENABLE                        0 /// 调试开关
#define PUMP_RUN_CHECK_INTERVAL                      1
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
motor_policy_node_t *pump_policy_init(motor_policy_cfg_t cfg) 
{
    motor_policy_node_t *policy_obj = u_malloz(sizeof(motor_policy_node_t));
    if (policy_obj == NULL) {
        LOGE("policy malloc fail");
        return NULL;
    }

    motor_policy_core_t *m_para = &policy_obj->core;
    memcpy(m_para, &cfg.core, sizeof(motor_policy_core_t));
    return policy_obj;
}

motor_policy_node_t *pump_policy_init_by_steps_info(pump_mode_step_param_t cfg) 
{
    motor_policy_node_t *policy_obj = u_malloz(sizeof(motor_policy_node_t));
    if (policy_obj == NULL) {
        LOGE("policy malloc fail");
        return NULL;
    }
    motor_policy_core_t *m_para = &policy_obj->core;
    m_para->type = MOTOR_PARAM_TABLE;
    m_para->un.mapping.speed = cfg.freq;
    m_para->un.mapping.level = cfg.level;
    m_para->un.mapping.mode_type = cfg.mode_type;
    m_para->config.loop_num = cfg.loop_num;
    return policy_obj;
}

int pump_policy_add(motor_policy_node_t *policy, motor_policy_node_t *next_policy) 
{
    if (policy == NULL || next_policy == NULL) {
        return -1;       
    }
    policy->next = next_policy;

    return 0;
}

int pump_policy_del(motor_timed_policy_node_t *time_policy) 
{
    motor_policy_node_t *next_policy = NULL;
    motor_policy_node_t *current_policy = time_policy->policy.next;
    EXCEPTION_CHECK_INT(time_policy);
    
    for (;;) {
        if (time_policy->sub_policy_count <= 1 || current_policy == NULL) {
            break;
        }
        next_policy = current_policy->next;
        u_free(current_policy);
        current_policy = next_policy;
        time_policy->sub_policy_count--;
    }

    return 0;
}

int pump_policy_update(motor_policy_info_t *info, uint8_t pump_id, motor_timer_event_msg_t msg) 
{ 
    // if (msg.msg_type == MOTOR_DRIVER_TIMER_EVENT_MSG) {
    //     LOG_MOTOR("pump_policy_update: msg_type: %d, pump_id: %d, action: %d", msg.msg_type, msg.pump_id, msg.action);
    // }
    switch (info->state.status) {
        case SUCTION_STATUS_IDLE: 
            info->state.policy_update_lock = 1;
            info->state.status = SUCTION_STATUS_GENERATE;
            info->state.action = MOTOR_ACTION_PUMP_ON;
            break;

        case SUCTION_STATUS_GENERATE:
            if (msg.msg_type == MOTOR_DRIVER_TIMER_EVENT_MSG &&
                msg.pump_id == pump_id && msg.un.action == MOTOR_PUMP_ON) {
                if (info->original.hold_time > 0) {
                    info->state.status = SUCTION_STATUS_HOLD;
                    info->state.action = MOTOR_ACTION_PUMP_OFF;
                } else {
                    info->state.status = SUCTION_STATUS_RELEASE;
                    info->state.action = MOTOR_ACTION_PUMP_OFF_AND_VALVE_ON;
                }
            }
            break;

        case SUCTION_STATUS_HOLD:
            if (msg.msg_type == MOTOR_DRIVER_TIMER_EVENT_MSG &&
                msg.pump_id == pump_id && msg.un.action == MOTOR_PUMP_OFF) {
                info->state.status = SUCTION_STATUS_RELEASE;
                info->state.action = MOTOR_ACTION_VALVE_ON;
            }
            break;

        case SUCTION_STATUS_RELEASE:
            if (msg.msg_type == MOTOR_DRIVER_TIMER_EVENT_MSG && msg.pump_id == pump_id && 
                (msg.un.action == MOTOR_VALVE_ON || msg.un.action == MOTOR_PUMP_OFF_AND_VALVE_ON)) {
                info->state.status = SUCTION_STATUS_SLEEP;
                info->state.action = MOTOR_ACTION_VALVE_OFF;
            }
            break;

        case SUCTION_STATUS_SLEEP:
            if (msg.msg_type == MOTOR_DRIVER_TIMER_EVENT_MSG && 
                msg.pump_id == pump_id && msg.un.action == MOTOR_VALVE_OFF) {
                info->state.status = SUCTION_STATUS_UPDATE_POLICY;
                info->state.action = MOTOR_ACTION_IDLE;
                info->state.cycles_count++;
                info->state.policy_update_lock = 0;
            }
            break;
        case SUCTION_STATUS_UPDATE_POLICY:
        
            break;
        default:
            LOG_MOTOR("unknown status: %d, action: %d, lock: %d", 
                info->state.status, info->state.action, info->state.policy_update_lock);
            break;
    }

    return 0;
}

void pump_policy_info_reset(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime, uint8_t pump_id) 
{
    uint16_t pump_level = 0, total_time = 0;
    suction_strategy_mapping_t map_para = {0};
    motor_policy_info_t *info = &runtime->info[pump_id];
    motor_policy_node_t *policy = &runtime->policy[pump_id];
    /// 默认策略
    memset(&info->state, 0, sizeof(motor_policy_state_t));
    memcpy(&info->config, &policy->core.config, sizeof(suction_policy_config_t));
    /// 透传策略 透传策略不进行策略处理，直接使用原始参数
    if (policy->core.type == MOTOR_PARAM_PASS_THROUGH) {
        memcpy(&info->original, &policy->core.un.original, sizeof(motor_policy_original_para_t));
        info->state.target_pressure = 0;
        total_time = info->original.pumping_time + info->original.hold_time + info->original.valve_on_time + info->original.sleep_time;
        runtime->pump_freq[pump_id] = (ONE_MIN_TIME_MS / total_time);
        runtime->pump_level[pump_id] = 0; 
        return;
    }
    /// 获取策略参数
    memcpy(&map_para, &policy->core.un.mapping, sizeof(suction_strategy_mapping_t));
    /// 空闲模式，吸力设定为0
    if (map_para.mode_type == SUCTION_MODE_IDLE) {
        memset(&info->original, 0, sizeof(motor_policy_original_para_t));
        LOG_MOTOR("-->goto idle policy");
        info->state.target_pressure  = 0;
        runtime->pump_freq[pump_id]  = 0; 
        runtime->pump_level[pump_id] = 0;
        return;
    }
    if (runtime->taste_run_flag) {
        /// 体验模式用户策略
        taste_mode_user_policy_update(thiz, &map_para, pump_id);
    } else {
        /// 用户设定力度与频率(速度)策略
        pump_user_policy_update(thiz, &map_para, pump_id);
    }
    /// 爬升策略
#if IS_SUPPORT_CLIMB_POLICY
    pump_climb_policy_update(thiz, &map_para);
#endif
    runtime->pump_level[pump_id] = map_para.level >= BREAST_PUMP_LEVEL_MAX ? (BREAST_PUMP_LEVEL_MAX - 1) : map_para.level;
    runtime->pump_speed[pump_id] = map_para.speed >= SUCTION_SPEED_MAX ? (SUCTION_SPEED_MAX - 1) : map_para.speed;
    /// 通过map参数获取吸力参数
    if (map_para.mode_type == SUCTION_MODE_SHORT || map_para.mode_type == SUCTION_MODE_MIX_SHORT) {
        pump_level = short_level_table[runtime->pump_level[pump_id]]; /// 获取负压等级
        info->original.pump_level    = short_suction_table[pump_level].pump_level * PUMP_DUTY_SCALE_FACTOR;
        info->original.pumping_time  = short_suction_table[pump_level].pumping_time;
        info->original.valve_on_time = short_suction_table[pump_level].valve_on_time;
        info->original.hold_time     = SHORT_SUCTION_HOLD_TIME; 
        runtime->pump_freq[pump_id] = short_suction_freq_table[runtime->pump_speed[pump_id]];
        if (runtime->pump_freq[pump_id] == 0) {
            LOGW("invalid short suction freq");
            runtime->pump_freq[pump_id] = SHORT_SUCTION_DEFAULT_FREQ; /// 使用默认频率
        }
        total_time = ONE_MIN_TIME_MS / runtime->pump_freq[pump_id];
        info->original.sleep_time    = total_time - info->original.pumping_time - 
                            info->original.valve_on_time - info->original.hold_time;           
        if (info->original.sleep_time < PUMP_SLEEP_TIME_MIN || info->original.sleep_time > PUMP_SLEEP_TIME_MAX) {
            info->original.sleep_time = PUMP_SLEEP_TIME_MIN;
            LOG_MOTOR("work period error");
        }
        LOG_MOTOR("short suction: s_time:%d, p_time:%d, v_time:%d, level:%d/%d, speed:%d, d_time:%d/%d/%d",
            info->original.sleep_time, info->original.pumping_time, info->original.valve_on_time,
            runtime->pump_level[pump_id], info->original.pump_level, runtime->pump_speed[pump_id],
            total_time, runtime->pump_freq[pump_id], runtime->freq_sync_flag);
    } else {
        runtime->pump_freq[pump_id] = long_pump_freq_table[runtime->pump_level[pump_id]][runtime->pump_speed[pump_id]];
        if (runtime->pump_freq[pump_id] == 0) {
            LOGW("invalid long suction freq:%d/%d", map_para.level, map_para.speed);
            runtime->pump_freq[pump_id] = LONG_SUCTION_DEFAULT_FREQ; /// 使用默认频率
        }
        pump_level = long_level_table[runtime->pump_level[pump_id]]; /// 获取负压等级
        info->original.pump_level    = long_suction_table[pump_level].pump_level * PUMP_DUTY_SCALE_FACTOR;
        info->original.pumping_time  = long_suction_table[pump_level].pumping_time;
        info->original.valve_on_time = long_suction_table[pump_level].valve_on_time;
        info->original.hold_time     = LONG_SUCTION_HOLD_TIME; 
        total_time = ONE_MIN_TIME_MS / runtime->pump_freq[pump_id];
        info->original.sleep_time    = total_time - info->original.pumping_time - info->original.valve_on_time - 
                info->original.hold_time;
        if (info->original.sleep_time < PUMP_SLEEP_TIME_MIN || info->original.sleep_time > PUMP_SLEEP_TIME_MAX) {
            info->original.sleep_time = PUMP_SLEEP_TIME_MIN;
            LOG_MOTOR("work period error");
        } 
        LOG_MOTOR("long suction: s_time:%d, p_time:%d, v_time:%d, level:%d/%d, speed:%d, d_time:%d/%d/%d",
            info->original.sleep_time, info->original.pumping_time, info->original.valve_on_time,
            runtime->pump_level[pump_id], info->original.pump_level, runtime->pump_speed[pump_id], 
            total_time, runtime->pump_freq[pump_id], runtime->freq_sync_flag);
    }
    info->state.target_pressure = pump_level == SUCTION_LEVEL_0KPA ? 0 : (pump_level + SUCTION_DEFAULT_LEVEL_VALUE);
    /// 通过算法线性补偿
    pump_compensation_policy_update(thiz, &info->original); 
    
}