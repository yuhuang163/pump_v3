/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "pump_mode_default_config.h"
#include "breast_pump_policy.h"

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_MODE_DEBUG_LOG              (0)

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
const motor_policy_cfg_t *get_breast_pump_mode_para (int mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:   
            return stimulation_default;
        case BREAST_PUMP_PUMP_MODE:   
            return pump_default;
        case BREAST_PUMP_MIX_MODE:   
            return mix_default;
        case BREAST_PUMP_CUSTOM_MODE:   
            return custom_default;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:   
            return pp_milk_track_default;
        case BREAST_PUMP_MILK_BOOST_MODE:   
            return milk_boost_default;
        case BREAST_PUMP_LACTATION_START_MODE:   
            return lactation_start_default;  
    }
    return NULL;
}

int breast_pump_mode_work_time_get(int mode)
{
    uint8_t mode_num = 0;
    int work_time = 0;
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(mode);
    if (cfg == NULL) {
        return NORMAL_MODE_DEFAULT_WORK_TIME;
    }

    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return NORMAL_MODE_DEFAULT_WORK_TIME;
        case BREAST_PUMP_PUMP_MODE:
            return NORMAL_MODE_DEFAULT_WORK_TIME;
        case BREAST_PUMP_MIX_MODE:
            return NORMAL_MODE_DEFAULT_WORK_TIME;
        case BREAST_PUMP_LACTATION_START_MODE:
            return LACTATION_START_MODE_WORK_TIME;
        case BREAST_PUMP_MILK_BOOST_MODE:
            return MILK_BOOST_MODE_WORK_TIME;
        case BREAST_PUMP_CUSTOM_MODE:
            mode_num = CUSTOM_MODE_LEN;
            for (uint8_t i = 0; i < mode_num; i++) {
                work_time += custom_default[i].duration;
            }
            return work_time;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return NORMAL_MODE_DEFAULT_WORK_TIME;
    }
    return NORMAL_MODE_DEFAULT_WORK_TIME;
}

int breast_pump_mode_parameter_output(motor_policy_manager_t *obj, char *name)
{
#if IS_SUPPORT_PUMP_MODE_DEBUG_LOG
    EXCEPTION_CHECK_INT(obj);
    motor_timed_policy_node_t *timed = &obj->head;
    motor_policy_node_t *policy = &timed->policy;
    LOG_MOTOR("%s time policy size:%d", name, obj->timed_policy_count);
    
    for (int i = 0; i < obj->timed_policy_count; i++) {
        LOG_MOTOR("time %d sub policy size:%d", i, timed->sub_policy_count);
        for (int j = 0; j < timed->sub_policy_count; j++) {
            if (policy == NULL) {
                LOGE("policy null");
                break;
            }
            LOG_MOTOR(" -->info level:%d, type:%d, speed:%d; hold time:%d, loop num:%d, sleep time:%d, next:%p", 
                policy->core.un.mapping.level, policy->core.un.mapping.mode_type, policy->core.un.mapping.speed,
                policy->core.config.period, policy->core.config.loop_num,
                policy->next);
            policy = policy->next;
        }

        timed = timed->next;
        if (timed != NULL) {
            policy = &timed->policy;
        }
    }
#endif
    return 0;
}

int breast_pump_mode_parameter_deinit(motor_policy_manager_t *obj)
{
    EXCEPTION_CHECK_INT(obj);
    /// 删除定时策略
    pump_time_policy_empty(obj);
    /// 删除默认定时策略下的子策略
    pump_policy_del(&obj->head);
    /// 删除总策略
    pump_total_policy_deint(obj);

    return 0;
}

motor_policy_manager_t *breast_pump_mode_parameter_init(const motor_policy_cfg_t *cfg, uint8_t cfg_num, char *name)
{
    if (cfg_num == 0) {
        LOGE("mode config number error");
        return NULL;
    }
    motor_policy_manager_t *obj = NULL;
    motor_policy_node_t *policy = NULL;
    motor_policy_node_t *next_policy = NULL;
    motor_timed_policy_node_t *time_policy = NULL;
    motor_timed_policy_node_t *next_time_policy = NULL;

    obj = pump_total_policy_init(cfg[0]);
    time_policy = &obj->head;
    policy = &obj->head.policy;
    for (uint8_t i = 1; i < cfg_num; i++) {
        /// 新增定时策略
        if (cfg[i].policy_type == MOTOR_TIME_POLICY) {
            next_time_policy = pump_time_policy_init(cfg[i]);
            if (next_time_policy == NULL) {
                LOGE("add %s time policy failed !!!", name);
                goto ERROR_EXIT;
            }
            pump_time_policy_add(time_policy, next_time_policy);
            time_policy = next_time_policy;
            policy = &time_policy->policy;
            obj->timed_policy_count++; /// 定时策略个数统计
            continue;
        } 
        /// 新增子策略实体
        next_policy = pump_policy_init(cfg[i]);
        if (next_policy == NULL) {
            LOGE("add %s sub policy failed !!!", name);
            goto ERROR_EXIT;
        }
        time_policy->sub_policy_count++; /// 子策略个数统计
        pump_policy_add(policy, next_policy);
    }
    breast_pump_mode_parameter_output(obj, name);
    return obj;

ERROR_EXIT:
    breast_pump_mode_parameter_deinit(obj);
    return NULL;
}
