/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "pump_mode_default_config.h"
#include "breast_pump_policy.h"
#include "app_protocol_pump_ctrl_server.h"
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
            /// todo: 补全自定义模式参数
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

int breast_pump_mode_time_step_number_get(int mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return STIMULATION_MODE_LEN;
        case BREAST_PUMP_LACTATION_MODE:
            return PUMP_MODE_LEN;
        case BREAST_PUMP_MIX_MODE:
            return MIX_MODE_LEN / 2; /// 混合模式步数为2，因为短吸和长吸各占一个步数.
        case BREAST_PUMP_CUSTOM_MODE:
            /// todo: 补全自定义模式步数
            return CUSTOM_MODE_LEN;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return PP_MODE_LEN;
        case BREAST_PUMP_MILK_BOOST_MODE:
            return MILK_BOOST_MODE_LEN;
        case BREAST_PUMP_LACTATION_START_MODE:
            return LACTATION_START_MODE_LEN;
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
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:   
            setting = pump_info->stimulation_setting;
            for (uint8_t i = 0; i < step_count; i++) {
                setting[i].speed = cfg[i].core.un.mapping.speed;
                setting[i].level[0] = cfg[i].core.un.mapping.level;
                setting[i].level[1] = cfg[i].core.un.mapping.level;
            }
            break;
        case BREAST_PUMP_LACTATION_MODE:   
            setting = pump_info->pump_setting;
            for (uint8_t i = 0; i < step_count; i++) {
                setting[i].speed = cfg[i].core.un.mapping.speed;
                setting[i].level[0] = cfg[i].core.un.mapping.level;
                setting[i].level[1] = cfg[i].core.un.mapping.level;
            }
            break;
        case BREAST_PUMP_MIX_MODE:   
            setting = pump_info->mix_setting;
            for (uint8_t i = 0; i < step_count; i++) {
                setting[i].speed = cfg[i].core.un.mapping.speed;
                setting[i].level[0] = cfg[i].core.un.mapping.level;
                setting[i].level[1] = cfg[i].core.un.mapping.level;
            }
            break;
        case BREAST_PUMP_CUSTOM_MODE:   
            setting = pump_info->custom_setting;
            for (uint8_t i = 0; i < step_count; i++) {
                setting[i].speed = cfg[i].core.un.mapping.speed;
                setting[i].level[0] = cfg[i].core.un.mapping.level;
                setting[i].level[1] = cfg[i].core.un.mapping.level;
            }
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:   
            setting = pump_info->pp_milk_track_setting;
            for (uint8_t i = 0; i < step_count; i++) {
                setting[i].speed = cfg[i].core.un.mapping.speed;
                setting[i].level[0] = cfg[i].core.un.mapping.level;
                setting[i].level[1] = cfg[i].core.un.mapping.level;
            }
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:   
            setting = pump_info->milk_boost_setting;
            for (uint8_t i = 0; i < step_count; i++) {
                setting[i].speed = cfg[i].core.un.mapping.speed;
                setting[i].level[0] = cfg[i].core.un.mapping.level;
                setting[i].level[1] = cfg[i].core.un.mapping.level;
            }
            break;
        case BREAST_PUMP_LACTATION_START_MODE:   
            setting = pump_info->lactation_start_setting;  
            for (uint8_t i = 0; i < step_count; i++) {
                setting[i].speed = cfg[i].core.un.mapping.speed;
                setting[i].level[0] = cfg[i].core.un.mapping.level;
                setting[i].level[1] = cfg[i].core.un.mapping.level;
            }
            break;
        default:
            LOGE("mode error, mode: %d", mode);
            break;
    }
}

void breast_pump_custom_step_para_reset(sys_pump_info_t *pump_info)
{
    const motor_policy_cfg_t *cfg = custom_default;
    EXCEPTION_CHECK_VOID(cfg);

    user_def_pump_steps_data_t *custom_steps_parameter = &pump_info->custom_steps_parameter;

    // 初始化参数
    custom_steps_parameter->id.dat32 = (uint32_t)0; // 标识默认参数
    custom_steps_parameter->custom_total_time = 0;  // 重置总时长

    // 遍历配置，跳过子策略(MOTOR_TIME_SUB_POLICY)，只统计主策略
    uint8_t step_idx = 0;
    for (uint8_t i = 0; i < CUSTOM_MODE_LEN && step_idx < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        // 跳过子策略，子策略不单独计为一个step
        if (cfg[i].policy_type == MOTOR_TIME_SUB_POLICY) {
            continue;
        }

        custom_steps_parameter->data[step_idx].freq = cfg[i].core.un.mapping.speed;
        custom_steps_parameter->data[step_idx].level = cfg[i].core.un.mapping.level;
        custom_steps_parameter->data[step_idx].time = cfg[i].duration; // 单位s

        // 根据 loop_num 确定模式类型
        uint8_t loop_num = cfg[i].core.config.loop_num;
        if (loop_num == 5) {
            // 混合模式1: 5次短吸 + 1次长吸
            custom_steps_parameter->data[step_idx].mode = APP_OLD_COMM_MODE_MIX1;
        } else if (loop_num == 2) {
            // 混合模式2: 2次短吸 + 1次长吸
            custom_steps_parameter->data[step_idx].mode = APP_OLD_COMM_MODE_MIX2;
        } else {
            // 普通模式: SUCTION_MODE_SHORT(1)->0x01, SUCTION_MODE_LONG(2)->0x02
            custom_steps_parameter->data[step_idx].mode = cfg[i].core.un.mapping.mode_type;
        }

        custom_steps_parameter->custom_total_time += custom_steps_parameter->data[step_idx].time;

        LOG_MOTOR("ud-mode[%d]=%02x", step_idx, custom_steps_parameter->data[step_idx].mode);
        LOG_MOTOR("ud-level[%d]=%02x", step_idx, custom_steps_parameter->data[step_idx].level);
        LOG_MOTOR("ud-time[%d]=%04x", step_idx, custom_steps_parameter->data[step_idx].time);

        step_idx++;
    }

    custom_steps_parameter->num = step_idx;
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
