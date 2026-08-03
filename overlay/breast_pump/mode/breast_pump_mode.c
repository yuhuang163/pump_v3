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
static os_mutex_id s_policy_manager_obj_lock = NULL;

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
    #if IS_PUMP_STEPS_USE_SAME_SETTING
    for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        setting[i].speed = BREAST_PUMP_SPEED_DEFAULT;
        setting[i].level[0] = BREAST_PUMP_LEVEL_DEFAULT;
        setting[i].level[1] = BREAST_PUMP_LEVEL_DEFAULT;
    }
    #else
    for (uint8_t i = 0; i < step_count; i++) {
        #if 0
        setting[i].speed = cfg[i].core.un.mapping.speed;
        setting[i].level[0] = cfg[i].core.un.mapping.level;
        setting[i].level[1] = cfg[i].core.un.mapping.level;
        #else
        setting[i].speed = DEFAULT_SPEED_VALUE;
        setting[i].level[0] = DEFAULT_LEVEL_VALUE;
        setting[i].level[1] = DEFAULT_LEVEL_VALUE;
        #endif
    }
    #endif
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
        steps_info->data[step_idx].duration = cfg[i].duration;
        steps_info->data[step_idx].policy_type = cfg[i].policy_type;
        steps_info->total_time += cfg[i].duration;
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

static void policy_manager_obj_lock_init(void)
{
    if (s_policy_manager_obj_lock == NULL) {
        MUTEX_INIT(&s_policy_manager_obj_lock, policy_manager_obj_mutex);
    }
}

void policy_manager_obj_lock(void)
{
    policy_manager_obj_lock_init();
    MUTEX_LOCK(s_policy_manager_obj_lock);
}

void policy_manager_obj_unlock(void)
{
    policy_manager_obj_lock_init();
    MUTEX_UNLOCK(s_policy_manager_obj_lock);
}

void pump_setting_update_ram(breast_pump_manager_context_t *ctx,
    pump_setting_t *src, uint8_t step_idx)
{
    ctx->priv.setting[step_idx].level[0] = src->level[0];
    ctx->priv.setting[step_idx].level[1] = src->level[1];
    ctx->priv.setting[step_idx].speed = src->speed;
    LOG_MOTOR("same-mode update_ram step=%d L=%d R=%d speed=%d",
        step_idx, src->level[0], src->level[1], src->speed);
}

void pump_setting_store(breast_pump_manager_context_t *ctx, int mode, uint8_t step_idx)
{
    if (ctx->priv.st) {
        ctx->priv.st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO,
            mode, step_idx, &ctx->priv.setting[step_idx]);
        LOG_MOTOR("same-mode store mode=%d step=%d L=%d R=%d speed=%d",
            mode, step_idx,
            ctx->priv.setting[step_idx].level[0],
            ctx->priv.setting[step_idx].level[1],
            ctx->priv.setting[step_idx].speed);
    }
}

/**
 * @brief 在单个模式策略中，同步相同 mode_type 的 step 设置
 * @param skip_step_index 跳过的源 step（0xFF 表示不跳过）
 * @param update_runtime_ram 是否更新当前运行时 setting[]（仅当前实际模式需要）
 * @param store_mode 写入存储用的 mode（可为具体模式或 BREAST_PUMP_ALL_MODE_AND_PROGRAM）
 */
static void pump_setting_sync_mode_type_in_manager(breast_pump_manager_context_t *ctx,
    pump_setting_t *setting, const motor_policy_manager_t *manager,
    uint8_t target_mode_type, uint8_t skip_step_index,
    bool update_runtime_ram, int store_mode, bool is_need_store)
{
    const motor_timed_policy_node_t *time_policy = &manager->head;
    int idx = 0;

    while (time_policy != NULL && idx < ROOT_BREAST_PUMP_STEP_MAX) {
        if (idx != skip_step_index &&
            time_policy->policy.core.un.mapping.mode_type == target_mode_type) {
            if (update_runtime_ram) {
                pump_setting_update_ram(ctx, setting, idx);
            }
            if (is_need_store) {
                if (update_runtime_ram) {
                    pump_setting_store(ctx, store_mode, idx);
                } else if (ctx->priv.st) {
                    /// 其他模式只写该模式存储，避免误改当前运行时 setting[]
                    ctx->priv.st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO,
                        store_mode, idx, setting);
                    LOG_MOTOR("cross-mode store mode=%d step=%d L=%d R=%d speed=%d",
                        store_mode, idx, setting->level[0], setting->level[1], setting->speed);
                }
            }
        }
        time_policy = time_policy->next;
        idx++;
    }
}

static const motor_policy_manager_t *pump_setting_find_policy_manager(
    motor_manager_context_t *motor, int breast_pump_mode)
{
    list_head_t *head = &motor->priv.policy_list;

    if (head->next == NULL) {
        return NULL;
    }
    LIST_FOR_EACH(pos, head) {
        breast_pump_policy_node_t *node =
            CONTAINER_OF(pos, breast_pump_policy_node_t, list);
        if (node->breast_pump_mode == breast_pump_mode && node->get_policy_cb) {
            return node->get_policy_cb(node->user_data);
        }
    }
    return NULL;
}

static uint8_t pump_setting_get_step_mode_type(const motor_policy_manager_t *manager,
    uint8_t step_index)
{
    const motor_timed_policy_node_t *time_policy = &manager->head;
    int idx = 0;

    while (time_policy != NULL && idx <= step_index && idx < ROOT_BREAST_PUMP_STEP_MAX) {
        if (idx == step_index) {
            return time_policy->policy.core.un.mapping.mode_type;
        }
        time_policy = time_policy->next;
        idx++;
    }
    return 0xFF;
}

/**
 * @brief 按 mode_type 同步泵设置（策略取自 policy_list 注册态，非 runtime）
 * @param ctx 泵管理上下文
 * @param setting 源设置
 * @param step_index 源 step 索引
 * @param mode 目标模式；为 BREAST_PUMP_ALL_MODE_AND_PROGRAM 时按当前实际模式查策略
 * @param is_need_store 是否写入存储
 * @note IS_PUMP_SETTING_SYNC_MODE_TYPE_ALL_MODES=0 仅当前模式内同步；
 *       =1 时同步所有已注册模式中相同 mode_type 的时间策略
 */
void pump_setting_sync_by_mode_type(breast_pump_manager_context_t *ctx,
    pump_setting_t *setting, uint8_t step_index, int mode, bool is_need_store)
{
    /// 全局参数模式下使用当前实际模式
    int actual_mode = (mode == BREAST_PUMP_ALL_MODE_AND_PROGRAM) ?
        ctx->priv.m_info.mode : mode;
    const motor_policy_manager_t *manager = NULL;
    uint8_t target_mode_type = 0xFF;

    policy_manager_obj_lock();

    motor_manager_context_t *motor = ctx->priv.motor;
    if (!motor) {
        LOGE("sync_by_mode_type: motor is NULL");
        policy_manager_obj_unlock();
        return;
    }

    manager = pump_setting_find_policy_manager(motor, actual_mode);
    if (!manager) {
        LOGE("sync_by_mode_type: manager not found for mode=%d", actual_mode);
        policy_manager_obj_unlock();
        return;
    }

    target_mode_type = pump_setting_get_step_mode_type(manager, step_index);
    if (target_mode_type == 0xFF) {
        LOG_MOTOR("sync_by_mode_type: mode_type not found, mode=%d step=%d",
            actual_mode, step_index);
        policy_manager_obj_unlock();
        return;
    }

#if IS_PUMP_SETTING_SYNC_MODE_TYPE_ALL_MODES
    /// 遍历所有已注册模式，同步相同 mode_type 的时间策略
    list_head_t *head = &motor->priv.policy_list;
    if (head->next != NULL) {
        LIST_FOR_EACH(pos, head) {
            breast_pump_policy_node_t *node =
                CONTAINER_OF(pos, breast_pump_policy_node_t, list);
            const motor_policy_manager_t *mode_manager = NULL;
            bool is_actual_mode = (node->breast_pump_mode == actual_mode);

            if (!node->get_policy_cb) {
                continue;
            }
            mode_manager = node->get_policy_cb(node->user_data);
            if (!mode_manager) {
                continue;
            }

            pump_setting_sync_mode_type_in_manager(ctx, setting, mode_manager,
                target_mode_type,
                is_actual_mode ? step_index : 0xFF,
                is_actual_mode,
                node->breast_pump_mode,
                is_need_store);
        }
    }
#else
    /// 仅当前模式内：同步相同 mode_type 的其他 step
    pump_setting_sync_mode_type_in_manager(ctx, setting, manager,
        target_mode_type, step_index, true, mode, is_need_store);
#endif

    policy_manager_obj_unlock();
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
