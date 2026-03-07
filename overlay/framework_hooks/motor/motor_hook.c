/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "handler_post.h"
#include "breast_pump_mode.h"
#include "breast_pump_policy.h"

/*
* DEFINES
*****************************************************************************************
*/
#define MOTOR_DEBUG_TASK_NAME "motor_debug_task"
#define MOTOR_DEBUG_SYNC_TIME_THRESHOLD   50   /// 同步时间阈值，单位：毫秒

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
static int breast_pump_policy_register(struct motor_manager_context *thiz, add_breast_pump_policy_fptr add_policy_cb)
{
    list_head_t *head = &thiz->priv.policy_list;

    add_policy_cb(head, "stimulation", BREAST_PUMP_STIMULATION_MODE, NULL,
        stimulation_mode_parameter_init, stimulation_mode_policy_get);
    add_policy_cb(head, "pump",  BREAST_PUMP_LACTATION_MODE,  NULL,
        pump_mode_parameter_init,  pump_mode_policy_get);
    add_policy_cb(head, "mix", BREAST_PUMP_MIX_MODE, NULL,
        mix_mode_parameter_init, mix_mode_policy_get);
    add_policy_cb(head, "custom", BREAST_PUMP_CUSTOM_MODE, NULL,
        custom_mode_parameter_init, custom_mode_policy_get);
    add_policy_cb(head, "pp_milk_track",  BREAST_PUMP_PP_MILK_TRACK_MODE,  NULL,
        pp_milk_track_mode_parameter_init,  pp_milk_track_mode_policy_get);
    add_policy_cb(head, "2+8_milk_boost", BREAST_PUMP_MILK_BOOST_MODE, NULL,
        milk_boost_mode_parameter_init, milk_boost_mode_policy_get);
    add_policy_cb(head, "lactation_start", BREAST_PUMP_LACTATION_START_MODE, NULL,
        lactation_start_mode_parameter_init, lactation_start_mode_policy_get);
    add_policy_cb(head, "regular_pumping", BREAST_PUMP_REGULAR_PUMPING_MODE, NULL,
        regular_pumping_mode_parameter_init, regular_pumping_mode_policy_get);
    add_policy_cb(head, "emptying_boost", BREAST_PUMP_EMPTYING_BOOST_MODE, NULL,
        emptying_boost_mode_parameter_init, emptying_boost_mode_policy_get);
    add_policy_cb(head, "relax", BREAST_PUMP_RELAX_MODE, NULL,
        relax_mode_parameter_init, relax_mode_policy_get);
    add_policy_cb(head, "custom_trial", BREAST_PUMP_CUSTOM_TRIAL_MODE, NULL,
        custom_trial_mode_parameter_init, custom_trial_mode_policy_get);
    return 0;
}

static void add_new_breast_pump_policy(list_head_t *head, const char *name, int breast_pump_mode, 
    void *user_data, breast_pump_policy_init_func init_cb, get_policy_obj_func policy_cb)
{
    breast_pump_policy_node_t *node = u_malloz(sizeof(breast_pump_policy_node_t));

    node->name = (char *)name;
    node->user_data = user_data;
    node->breast_pump_mode = breast_pump_mode;
    node->init_cb = init_cb;
    node->get_policy_cb = policy_cb;

    __list_add_tail(&node->list, head);
}

static int breast_pump_policy_init(struct motor_manager_context *ctx)
{
    int ret = 0;
    breast_pump_policy_register(ctx, add_new_breast_pump_policy);
    list_head_t *head = &ctx->priv.policy_list;
    if (head->next != NULL) {
        LIST_FOR_EACH(pos, head) {
            breast_pump_policy_node_t *node = CONTAINER_OF(pos, breast_pump_policy_node_t, list);
            if (node->init_cb) {
                ret = node->init_cb();
            }
        }
    }
    return ret;
}

static int on_motor_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    return 0;
}

static int generate_motor_para_custom(void *user_data, int data_size)
{
    pump_mode_steps_info_t *steps_info = (pump_mode_steps_info_t *) user_data;
    EXCEPTION_CHECK_INT(steps_info);
    return custom_mode_update_by_steps_info(steps_info);
}

static int generate_motor_para_custom_trial(void *user_data, int data_size)
{
    pump_mode_steps_info_t *steps_info = (pump_mode_steps_info_t *) user_data;
    EXCEPTION_CHECK_INT(steps_info);
    return custom_trial_mode_update_by_steps_info(steps_info);
}

static int generate_motor_para(struct motor_manager_context *ctx, int mode, void *user_data, int data_size)
{
    uint8_t stop_flag = 0;
    int ret = -1;
    /// 替换新参数前检测当前模式参数是否正在使用
    if (ctx->priv.runtime.breast_pump_mode == mode && ctx->priv.motor_state != MOTOR_POLICY_IDLE) {
        ctx->synchronous_stop_motor();
        stop_flag = 1;
    }

    switch (mode) {
        case BREAST_PUMP_CUSTOM_MODE:
            ret = generate_motor_para_custom(user_data, data_size);
            break;
        case BREAST_PUMP_CUSTOM_TRIAL_MODE:
            ret = generate_motor_para_custom_trial(user_data, data_size);
            break;
        default:
            break;
    }

    if (stop_flag) {
        ctx->start_motor(MOTOR_POLICY_CONTINUE);
    }
    return ret;
}

static void motor_debug_finish_cb(hdr_post_args_t *args)
{
    motor_manager_context_t *ctx = (motor_manager_context_t *)args->user_data;
    EXCEPTION_CHECK_VOID(ctx);
    ctx->stop_motor();
    ctx->priv.user_debug_status = 0;
}

static void motor_debug_with_user_para(motor_manager_context_t *ctx, uint8_t id, void *user_data)
{
    motor_para_t motor = {0};
    uint32_t args[2] = {0};
    if (id >= MOTOR_USER_DEBUG_ID_MAX) {
        LOG_MOTOR("motor debug id: %d, out of range", id);
        return;
    }
    EXCEPTION_CHECK_VOID(ctx);
    if (id == MOTOR_USER_DEBUG_PUMP_OFF || id == MOTOR_USER_DEBUG_VALVE_OFF) {
        ctx->stop_motor();
        ctx->priv.user_debug_status = 0;
        REMOVE_TASK_HANDLER_DIRECTLY(MOTOR_DEBUG_TASK_NAME);
        LOG_MOTOR("motor debug stop, id: %d", id);
        return;
    }
    if (ctx->priv.user_debug_status) {
        ctx->stop_motor();
        REMOVE_TASK_HANDLER_DIRECTLY(MOTOR_DEBUG_TASK_NAME);
    }
    ctx->priv.user_debug_status = 1;
    if (id == MOTOR_USER_DEBUG_POLICY) {
        EXCEPTION_CHECK_VOID(user_data);
        motor_policy_original_para_t *para = (motor_policy_original_para_t *)user_data;
        ctx->synchronous_stop_motor();
        ctx->priv.user_debug_status = 1;
        /// 释放旧的策略
        if (ctx->priv.user_debug_policy != NULL) {
            breast_pump_mode_parameter_deinit(ctx->priv.user_debug_policy);
            ctx->priv.user_debug_policy = NULL;
        }
        /// 创建新的策略并赋值给 ctx->priv.user_debug_policy
        ctx->priv.user_debug_policy = u_malloz(sizeof(motor_policy_manager_t));
        if (ctx->priv.user_debug_policy == NULL) {
            LOGE("Failed to allocate memory for user debug policy");
            return;
        }
        motor_policy_core_t *core = &ctx->priv.user_debug_policy->head.policy.core;
        motor_policy_original_para_t *original = &core->un.original;
        ctx->priv.user_debug_policy->timed_policy_count = 1;
        ctx->priv.user_debug_policy->head.duration_seconds = 1200;   /// 20分钟
        ctx->priv.user_debug_policy->head.sub_policy_count = 1;
        core->type = MOTOR_PARAM_PASS_THROUGH;
        memcpy(original, para, sizeof(motor_policy_original_para_t));
        core->config.loop_num = 1;
        LOG_MOTOR("user debug policy: pump_level: %d, pumping_time: %d, valve_on_time: %d, sleep_time: %d, hold_time: %d",
            para->pump_level, para->pumping_time, para->valve_on_time, para->sleep_time, para->hold_time);
        ctx->start_motor(MOTOR_USER_DEBUG_POLICY_START);
    }
    else if (id == MOTOR_USER_TASTE_STIMULATION_POLICY) {
        EXCEPTION_CHECK_VOID(user_data);
        LOG_MOTOR("MOTOR_USER_TASTE_STIMULATION_POLICY");
        motor_multiple_policy_cfg_t *para = (motor_multiple_policy_cfg_t *)user_data;
        ctx->synchronous_stop_motor();
        ctx->priv.user_debug_status = 1;
        /// 释放旧的策略
        if (ctx->priv.user_debug_policy != NULL) {
            breast_pump_mode_parameter_deinit(ctx->priv.user_debug_policy);
            ctx->priv.user_debug_policy = NULL;
        }
        /// 创建新的策略并赋值给 ctx->priv.user_debug_policy
        ctx->priv.user_debug_policy = breast_pump_mode_parameter_init(para->policy_cfg_list, para->policy_cfg_count, "user_taste_stimulation");
        ctx->start_motor(MOTOR_USER_DEBUG_POLICY_START);
    }
    else if (id == MOTOR_USER_TASTE_PUMP_POLICY) {
        EXCEPTION_CHECK_VOID(user_data);
        LOG_MOTOR("MOTOR_USER_TASTE_PUMP_POLICY");
        motor_multiple_policy_cfg_t *para = (motor_multiple_policy_cfg_t *)user_data;
        ctx->synchronous_stop_motor();
        ctx->priv.user_debug_status = 1;
        /// 释放旧的策略
        if (ctx->priv.user_debug_policy != NULL) {
            breast_pump_mode_parameter_deinit(ctx->priv.user_debug_policy);
            ctx->priv.user_debug_policy = NULL;
        }
        /// 创建新的策略并赋值给 ctx->priv.user_debug_policy
        ctx->priv.user_debug_policy = breast_pump_mode_parameter_init(para->policy_cfg_list, para->policy_cfg_count, "user_taste_pump");
        ctx->start_motor(MOTOR_USER_DEBUG_POLICY_START);
    }
    else if (id == MOTOR_USER_TASTE_MIX1_POLICY) {
        EXCEPTION_CHECK_VOID(user_data);
        LOG_MOTOR("MOTOR_USER_TASTE_MIX1_POLICY");
        motor_multiple_policy_cfg_t *para = (motor_multiple_policy_cfg_t *)user_data;
        ctx->synchronous_stop_motor();
        ctx->priv.user_debug_status = 1;
        /// 释放旧的策略
        if (ctx->priv.user_debug_policy != NULL) {
            breast_pump_mode_parameter_deinit(ctx->priv.user_debug_policy);
            ctx->priv.user_debug_policy = NULL;
        }
        /// 创建新的策略并赋值给 ctx->priv.user_debug_policy
        ctx->priv.user_debug_policy = breast_pump_mode_parameter_init(para->policy_cfg_list, para->policy_cfg_count, "user_taste_mix1");
        ctx->start_motor(MOTOR_USER_DEBUG_POLICY_START);
    }
    else if (id == MOTOR_USER_TASTE_MIX2_POLICY) {
        EXCEPTION_CHECK_VOID(user_data);
        LOG_MOTOR("MOTOR_USER_TASTE_MIX2_POLICY");
        motor_multiple_policy_cfg_t *para = (motor_multiple_policy_cfg_t *)user_data;
        ctx->synchronous_stop_motor();
        ctx->priv.user_debug_status = 1;
        /// 释放旧的策略
        if (ctx->priv.user_debug_policy != NULL) {
            breast_pump_mode_parameter_deinit(ctx->priv.user_debug_policy);
            ctx->priv.user_debug_policy = NULL;
        }
        /// 创建新的策略并赋值给 ctx->priv.user_debug_policy
        ctx->priv.user_debug_policy = breast_pump_mode_parameter_init(para->policy_cfg_list, para->policy_cfg_count, "user_taste_mix2");
        ctx->start_motor(MOTOR_USER_DEBUG_POLICY_START);
    }
    else if (id == MOTOR_USER_TASTE_POLICY_STOP || id == MOTOR_USER_TASTE_POLICY_EXIT) {
        ctx->priv.user_debug_status = 0;
        ctx->synchronous_stop_motor();
        /// 清理用户调试策略
        if (ctx->priv.user_debug_policy != NULL) {
            breast_pump_mode_parameter_deinit(ctx->priv.user_debug_policy);
            ctx->priv.user_debug_policy = NULL;
        }
        LOG_MOTOR("motor debug stop, id: %d", id);
    }
    else if (id == MOTOR_USER_DEBUG_PUMP_ON) {
        EXCEPTION_CHECK_VOID(user_data);
        motor_debug_para_t *para = (motor_debug_para_t *)user_data;
        motor.un.pwm.duty = para->duty;
        motor.un.pwm.freq = BREAST_PUMP_VALVE_FREQ;
        args[1] = (uint32_t)&motor;
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            args[0] = MOTOR_PWM_PUMP_1 + i;
            DEVICE_IOCTL_ARGS(ctx->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, args);
        }
        if (para->hold_time < MOTOR_DEBUG_SYNC_TIME_THRESHOLD) {
            msleep(para->hold_time);   
            args[0] = MOTOR_PWM_ALL_ID;
            DEVICE_IOCTL_ARGS(ctx->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
            ctx->priv.user_debug_status = 0;
        } else {
            POST_TASK_HANDLER_DELAYED_ARG1(motor_debug_finish_cb, para->hold_time, ctx,
                    MOTOR_DEBUG_TASK_NAME);
        }
        LOG_MOTOR("motor debug pump on, duty: %d, hold_time: %d", para->duty, para->hold_time);
    } else if (id == MOTOR_USER_DEBUG_VALVE_ON) {
        EXCEPTION_CHECK_VOID(user_data);
        motor_debug_para_t *para = (motor_debug_para_t *)user_data;
        motor.un.pwm.duty = para->duty;
        motor.un.pwm.freq = BREAST_PUMP_VALVE_FREQ;
        args[1] = (uint32_t)&motor;
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            args[0] = MOTOR_PWM_VALVE_1 + i;
            DEVICE_IOCTL_ARGS(ctx->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, args);
        }
        if (para->hold_time < MOTOR_DEBUG_SYNC_TIME_THRESHOLD) {
            msleep(para->hold_time);   
            args[0] = MOTOR_PWM_ALL_ID;
            DEVICE_IOCTL_ARGS(ctx->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
            ctx->priv.user_debug_status = 0;
        } else {
            POST_TASK_HANDLER_DELAYED_ARG1(motor_debug_finish_cb, para->hold_time, ctx,
                    MOTOR_DEBUG_TASK_NAME);
        }
        LOG_MOTOR("motor debug valve on, duty: %d, hold_time: %d", para->duty, para->hold_time);
    }
}

static void motor_debug_with_taste_setting(motor_manager_context_t *ctx, void *user_data)
{
    taste_setting_t *setting = (taste_setting_t *)user_data;
    EXCEPTION_CHECK_VOID(setting);
    memcpy(&ctx->priv.runtime.taste_setting, setting, sizeof(taste_setting_t));
    LOG_MOTOR("motor debug taste setting: level 0: %d, level 1: %d, speed: %d", setting->level[0], setting->level[1],
              setting->speed);
}

static int on_post_motor_init_route(struct motor_manager_context *ctx)
{
    /// 将模式策略参数挂载到列表中s
    breast_pump_policy_init(ctx);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_motor_hook(motor_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __motor_manager_hook));
    ctx->hook->on_post_init_route = on_post_motor_init_route;
    ctx->hook->on_motor_receiver_route = on_motor_receiver_route;
    ctx->hook->generate_motor_para = generate_motor_para;
    ctx->hook->motor_policy_prepare = breast_pump_policy_prepare;
    ctx->hook->mode_policy_handler = breast_pump_mode_policy_handler;
    ctx->hook->motor_debug_with_user_para = motor_debug_with_user_para;
}
