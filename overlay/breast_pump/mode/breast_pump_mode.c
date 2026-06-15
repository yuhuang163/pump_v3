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
const motor_policy_cfg_t *get_breast_pump_mode_para(int mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:   
            return stimulation_default;
        case BREAST_PUMP_LACTATION_MODE:   
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
        case BREAST_PUMP_REGULAR_PUMPING_MODE:   
            return regular_pumping_default;
        case BREAST_PUMP_EMPTYING_BOOST_MODE:   
            return emptying_boost_default;
        case BREAST_PUMP_RELAX_MODE:
            return relax_default;
        case BREAST_PUMP_CUSTOM_TRIAL_MODE:
            return custom_trial_default;
        default:
            return 0;
    }
    return NULL;
}

int breast_pump_mode_time_step_number_get(int mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return STIMULATION_MODE_STEP_NUM;
        case BREAST_PUMP_LACTATION_MODE:
            return PUMP_MODE_STEP_NUM;
        case BREAST_PUMP_MIX_MODE:
            return MIX_MODE_STEP_NUM;
        case BREAST_PUMP_CUSTOM_MODE:
            return CUSTOM_MODE_STEP_NUM;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return PP_MODE_STEP_NUM;
        case BREAST_PUMP_MILK_BOOST_MODE:
            return MILK_BOOST_STEP_NUM;
        case BREAST_PUMP_LACTATION_START_MODE:
            return LACTATION_START_STEP_NUM;
        case BREAST_PUMP_REGULAR_PUMPING_MODE:
            return REGULAR_PUMPING_STEP_NUM;
        case BREAST_PUMP_EMPTYING_BOOST_MODE:
            return EMPTYING_BOOST_STEP_NUM;
        case BREAST_PUMP_RELAX_MODE:
            return RELAX_MODE_STEP_NUM;
        case BREAST_PUMP_CUSTOM_TRIAL_MODE:
            return CUSTOM_TRIAL_STEP_NUM;
        default:
            return 0;
    }
}

int breast_pump_mode_time_all_step_number_get(int mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return STIMULATION_MODE_LEN;
        case BREAST_PUMP_LACTATION_MODE:
            return PUMP_MODE_LEN;
        case BREAST_PUMP_MIX_MODE:
            return MIX_MODE_LEN;
        case BREAST_PUMP_CUSTOM_MODE:
            return CUSTOM_MODE_LEN;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return PP_MODE_LEN;
        case BREAST_PUMP_MILK_BOOST_MODE:
            return MILK_BOOST_MODE_LEN;
        case BREAST_PUMP_LACTATION_START_MODE:
            return LACTATION_START_MODE_LEN;
        case BREAST_PUMP_REGULAR_PUMPING_MODE:
            return REGULAR_PUMPING_MODE_LEN;
        case BREAST_PUMP_EMPTYING_BOOST_MODE:
            return EMPTYING_BOOST_MODE_LEN;
        case BREAST_PUMP_RELAX_MODE:
            return RELAX_MODE_LEN;
        case BREAST_PUMP_CUSTOM_TRIAL_MODE:
            return CUSTOM_TRIAL_MODE_LEN;
        default:
            return 0;
    }
}

void breast_pump_setting_reset(sys_pump_info_t *pump_info, int mode)
{
    EXCEPTION_CHECK_VOID(pump_info);
    pump_setting_t *setting = NULL;
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(mode);
    EXCEPTION_CHECK_VOID(cfg);
    uint8_t step_count = breast_pump_mode_time_step_number_get(mode);
    if (step_count == 0 || step_count > ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("step count error, mode: %d, step count: %d", mode, step_count);
        return;
    }

    if (mode >= BREAST_PUMP_MODE_MAX || mode < BREAST_PUMP_STIMULATION_MODE) {
        LOGE("mode error, mode: %d", mode);
        return;
    }
    
    setting = pump_info->setting_wrap[mode].info;
    for (uint8_t i = 0; i < step_count; i++) {
        setting[i].speed = cfg[i].core.un.mapping.speed;
        setting[i].level[0] = cfg[i].core.un.mapping.level;
        setting[i].level[1] = cfg[i].core.un.mapping.level;
    }
}

void breast_pump_mode_steps_info_reset(sys_pump_mode_info_t *mode_info, int mode)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(mode);
    EXCEPTION_CHECK_VOID(cfg);
    uint8_t step_count = breast_pump_mode_time_all_step_number_get(mode);
    uint8_t step_idx = 0, time_step_num = 0;
    if (step_count == 0 || step_count > ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("step count error, mode: %d, step count: %d", mode, step_count);
        return;
    }
    pump_mode_steps_info_t *steps_info = &mode_info->steps_info[mode];
    steps_info->id.dat32   = 0;    /// 标识为0表示默认参数
    steps_info->total_time = 0;  /// 重置总时长
    for (uint8_t i = 0; i < step_count; i++) {
        steps_info->data[step_idx].freq = cfg[i].core.un.mapping.speed;
        steps_info->data[step_idx].level = cfg[i].core.un.mapping.level;
        steps_info->data[step_idx].mode_type = cfg[i].core.un.mapping.mode_type;
        steps_info->data[step_idx].loop_num = cfg[i].core.config.loop_num;
        steps_info->data[step_idx].duration = cfg[i].duration; /// 单位s
        steps_info->data[step_idx].policy_type = cfg[i].policy_type;
        steps_info->total_time += steps_info->data[step_idx].duration;
        // LOG_MOTOR("steps%d policy=%d, mode=%d, level=%d, duration=%d",
        //     step_idx, steps_info->data[step_idx].policy_type, steps_info->data[step_idx].mode_type,
        //     steps_info->data[step_idx].level, steps_info->data[step_idx].duration);
        step_idx++;
        if (cfg[i].policy_type == MOTOR_TIME_POLICY) {
            time_step_num++;
        }
    }
    steps_info->policy_num = step_idx;
    steps_info->time_policy_num = time_step_num;
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
        case BREAST_PUMP_LACTATION_MODE:
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
                work_time += cfg[i].duration;
            }
            return work_time;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return POWER_PUMPING_MODE_WORK_TIME;
        case BREAST_PUMP_REGULAR_PUMPING_MODE:
            return REGULAR_PUMPING_MODE_WORK_TIME;
        case BREAST_PUMP_EMPTYING_BOOST_MODE:
            return EMPTYING_BOOST_MODE_WORK_TIME;
        case BREAST_PUMP_RELAX_MODE:
            return RELAX_MODE_WORK_TIME;
    }
    return NORMAL_MODE_DEFAULT_WORK_TIME;
}

int breast_pump_mode_step_target_time_get(int mode, int step)
{
    if (step > ROOT_BREAST_PUMP_STEP_MAX || mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("step error, mode: %d, step: %d", mode, step);
        return 0;
    }
    pump_mode_steps_info_t steps_info;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, mode, 0, &steps_info);
    return steps_info.data[step].duration;
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
            LOG_MOTOR(" -->info level:%d, type:%d, speed:%d; period:%d, loop num:%d, next:%p",
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
    EXCEPTION_CHECK_POINT(cfg);
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
    if (obj == NULL) {
        LOGE("init %s total policy failed", name);
        return NULL;
    }
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
        policy = next_policy;
    }
    breast_pump_mode_parameter_output(obj, name);
    return obj;

ERROR_EXIT:
    breast_pump_mode_parameter_deinit(obj);
    return NULL;
}

motor_policy_manager_t *pump_mode_parameter_init_by_steps_info(pump_mode_steps_info_t *steps_info, char *name)
{
    EXCEPTION_CHECK_POINT(steps_info);
    if (steps_info->policy_num == 0 || steps_info->policy_num > ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("%s policy num error:%d", name, steps_info->policy_num);
        return NULL;
    }
    motor_policy_manager_t *obj = NULL;
    motor_policy_node_t *policy = NULL;
    motor_policy_node_t *next_policy = NULL;
    motor_timed_policy_node_t *time_policy = NULL;
    motor_timed_policy_node_t *next_time_policy = NULL;

    obj = pump_total_policy_init_by_steps_info(steps_info->data[0]);
    if (obj == NULL) {
        LOGE("init %s total policy failed", name);
        return NULL;
    }
    time_policy = &obj->head;
    policy = &obj->head.policy;
    for (uint8_t i = 1; i < steps_info->policy_num; i++) {
        if (steps_info->data[i].policy_type == MOTOR_TIME_POLICY) {
            next_time_policy = pump_time_policy_init_by_steps_info(steps_info->data[i]);
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
        next_policy = pump_policy_init_by_steps_info(steps_info->data[i]);
        if (next_policy == NULL) {
            LOGE("add %s sub policy failed !!!", name);
            goto ERROR_EXIT;
        }
        time_policy->sub_policy_count++; /// 子策略个数统计
        pump_policy_add(policy, next_policy);
        policy = next_policy;
    }
    breast_pump_mode_parameter_output(obj, name);
    return obj;

ERROR_EXIT:
    breast_pump_mode_parameter_deinit(obj);
    return NULL;
}
