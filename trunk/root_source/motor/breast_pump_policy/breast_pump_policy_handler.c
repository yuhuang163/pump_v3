/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_motor.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "motor_policy.h" // IWYU pragma: keep
#include "breast_pump_policy.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POLICY_WORK_MSG_NUM            2 /// 策略工作消息队列数量

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static os_message_id policy_work_msg_id = NULL; /// 策略工作消息ID.
static os_timer_id policy_work_timer = NULL; /// 策略工作定时器.

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static const motor_policy_manager_t *breast_pump_mode_policy_get(struct motor_manager_context *thiz, int breast_pump_mode)
{
    list_head_t *head = &thiz->priv.policy_list;
    if (head->next != NULL) {
        LIST_FOR_EACH(pos, head) {
            breast_pump_policy_node_t *node = CONTAINER_OF(pos, breast_pump_policy_node_t, list);
            if (node->breast_pump_mode == breast_pump_mode) {
                LOG_MOTOR("==> find name: %s, breast_pump_mode: %d", node->name, breast_pump_mode);
                return node->get_policy_cb(node->user_data);
            }
        }
    }
    LOGE("Oops! ==> not found! breast_pump_mode: %d", breast_pump_mode);
    msleep(500);

    return NULL;
}

static void update_para_to_motor_device(struct motor_manager_context *thiz, uint8_t id, motor_policy_info_t *info)
{
    motor_para_t motor = {0};
    uint32_t args[2] = {0};

    switch (info->state.action) {
        case MOTOR_ACTION_PUMP_ON:
            motor.un.pwm.duty = info->original.pump_level;
            motor.un.pwm.freq = BREAST_PUMP_PUMP_FREQ;
            info->state.pump_duty = motor.un.pwm.duty;
            args[0] =  MOTOR_PWM_PUMP_1 + id;
            args[1] = (uint32_t)&motor;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, args);
            info->state.n_pressure_flag = 1;
            break;

        case MOTOR_ACTION_PUMP_OFF:
            info->state.pump_duty = 0;
            args[0] = MOTOR_PWM_PUMP_1 + id;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
            break;

        case MOTOR_ACTION_PUMP_OFF_AND_VALVE_ON:
            info->state.pump_duty = 0;
            args[0] = MOTOR_PWM_PUMP_1 + id;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
            motor.un.pwm.duty = info->original.pump_level > 0 ? BREAST_PUMP_VALVE_DUTY : 0;
            motor.un.pwm.freq = BREAST_PUMP_VALVE_FREQ;
            info->state.valve_duty = motor.un.pwm.duty;
            args[0] = MOTOR_PWM_VALVE_1 + id;
            args[1] = (uint32_t)&motor;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, args);
            break;

        case MOTOR_ACTION_VALVE_ON:
            motor.un.pwm.duty = info->original.pump_level > 0 ? BREAST_PUMP_VALVE_DUTY : 0;
            motor.un.pwm.freq = BREAST_PUMP_VALVE_FREQ;
            info->state.valve_duty = motor.un.pwm.duty;
            args[0] = MOTOR_PWM_VALVE_1 + id;
            args[1] = (uint32_t)&motor;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, args);
            break;

        case MOTOR_ACTION_VALVE_OFF:
            info->state.valve_duty = 0;
            args[0] = MOTOR_PWM_VALVE_1 + id;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
            info->state.n_pressure_flag = 0;
            break;

        default:
            break;

    }
    info->state.action = MOTOR_ACTION_IDLE;
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void negative_pressure_release(struct motor_manager_context *thiz)
{
    motor_para_t motor = {
        .un.pwm.duty = BREAST_PUMP_VALVE_DUTY,
        .un.pwm.freq = BREAST_PUMP_VALVE_FREQ
    };
    uint32_t args[2] = {0, (uint32_t)&motor};
    LOG_MOTOR("-->step 1");
    args[0] = MOTOR_PWM_ALL_ID;
    DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
    msleep(5);
    LOG_MOTOR("-->step 2");
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        args[0] = i + MOTOR_PWM_VALVE_1;
        DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, args);
    }

    uint16_t wait_time = 120;
    while (wait_time) {
        //if (!thiz->is_can_play_now()) {
        //    break;
        //}
        msleep(10);
        wait_time = wait_time > 10 ? wait_time - 10 : 0;
    }
    
    LOG_MOTOR("-->step 3");
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        args[0] = i + MOTOR_PWM_VALVE_1;
        DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
    }
}

void breast_pump_policy_prepare(struct motor_manager_context *thiz, uint8_t action)
{
    motor_policy_runtime_t *body = &thiz->priv.runtime;
    const motor_policy_manager_t *policy_obj = NULL;
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(thiz->priv.pump_ctx, pump_mode);
    char *mode_name = "unknow";
    READ_PUMP_WORK_MODE_NAME(thiz->priv.pump_ctx, &mode_name);
    body->taste_run_flag = 0;//常规模式下，体验模式标志位清零
    if (action == MOTOR_USER_DEBUG_POLICY_START) {
        body->taste_run_flag = 1;
        body->all_timed_policy_finish_flag = 0;
        policy_obj = thiz->priv.user_debug_policy;
        LOG_MOTOR("user debug policy");
    } else if (action == MOTOR_POLICY_CONTINUE) {
        if (body->breast_pump_mode == pump_mode) {
            LOG_MOTOR("continue %s policy, step_index: %d", mode_name, body->current_timed_policy_index);
            body->timed_policy_start_time = get_timestamp();
            negative_pressure_release(thiz);
            for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
                //memcpy(&body->policy[i], &body->manager->head.policy, sizeof(motor_policy_node_t));
                pump_policy_info_reset(thiz, body, i);
            } 
            return;
        } else {
            body->all_timed_policy_finish_flag = 0;
            LOG_MOTOR("breast pump mode changed, need to restart policy");
            body->breast_pump_mode = pump_mode;
            policy_obj = breast_pump_mode_policy_get(thiz, body->breast_pump_mode);
            LOG_MOTOR("%s mode policy", mode_name);
        }
    } else {
        body->all_timed_policy_finish_flag = 0;
        body->breast_pump_mode = pump_mode;
        policy_obj = breast_pump_mode_policy_get(thiz, body->breast_pump_mode);
        LOG_MOTOR("%s mode policy", mode_name);
    }
    body->manager = (motor_policy_manager_t *)policy_obj;
    EXCEPTION_CHECK_VOID(policy_obj);
#if IS_SUPPORT_CLIMB_POLICY
    body->is_climb_policy = 1;
#endif
    body->timed_policy = &body->manager->head;
    body->current_mode_type = pump_policy_type_get(body->timed_policy->policy);
    if (body->timed_policy->next) {
        body->next_mode_type = pump_policy_type_get(body->timed_policy->next->policy);
    } else {
        body->next_mode_type = body->current_mode_type;
    }
    LOG_MOTOR("mode %d current type: %d, next type: %d", body->breast_pump_mode,
         body->current_mode_type, body->next_mode_type);
    body->current_timed_policy_index = 0;
    body->time_policy_running_time = 0;
    body->time_policy_remaining_time = body->timed_policy->duration_seconds;
    negative_pressure_release(thiz);
    body->timed_policy_start_time = get_timestamp();
    body->manager_start_time = body->timed_policy_start_time;
    /// 更新pump缓存当前策略步数索引,体验模式下不更新PUMP步数索引
    if (!body->taste_run_flag) {
        thiz->priv.pump_ctx->priv.m_info.step_index = 0;
    }
    
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        memcpy(&body->policy[i], &body->manager->head.policy, sizeof(motor_policy_node_t));
        pump_policy_info_reset(thiz, body, i);
    }
    /// 策略重启通知app
    if (body->taste_run_flag) {
        thiz->notify_motor_status_change(MOTOR_STATUS_EVT_TASTE_START);
    }
    
}

int breast_pump_mode_policy_handler(struct motor_manager_context *thiz, motor_task_para_t *play_para)
{
    uint8_t msg = 0;
    int exit_code = EXIT_REASON_PLAY_COMPLETED;
    motor_policy_runtime_t *runtime = &thiz->priv.runtime;

    LOG_MOTOR("===enter===");
    EXCEPTION_CHECK_INT(runtime->manager);
    // negative_pressure_release(thiz);
    MSG_QUEUE_RESET(policy_work_msg_id);
    START_TIMER(policy_work_timer, PUMP_POLICY_RUN_TIME_INTERVAL);
    while (1) {
        MSG_QUEUE_RECV(policy_work_msg_id, &msg, OS_WAIT_FOREVER);
        if (!thiz->is_can_play_now()) {
            exit_code = EXIT_REASON_INTERRUPT;
            break;
        }
        
        pump_time_policy_update(thiz, runtime);  ///  时间策略
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {   
            if (runtime->policy[i].core.un.mapping.mode_type == SUCTION_MODE_IDLE) {
                continue;
            } 
            pump_policy_update(&runtime->info[i], i);    
            update_para_to_motor_device(thiz, i, &runtime->info[i]);   
        }    
        // msleep(PUMP_POLICY_RUN_TIME_INTERVAL);
    }
    runtime->time_policy_running_time += get_timestamp_ms(runtime->timed_policy_start_time);
    negative_pressure_release(thiz);
    STOP_TIMER(policy_work_timer);
    if (runtime->taste_run_flag) {
        thiz->notify_motor_status_change(MOTOR_STATUS_EVT_TASTE_STOP);
    }
    runtime->taste_run_flag = 0;

    LOG_MOTOR("exit:%d", exit_code);

    return exit_code;
}

static void policy_work_timer_cb(os_timer_id *timer)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(policy_work_msg_id, &msg, 0);
}
 
void motor_policy_init(struct motor_manager_context *thiz)
{
    CREATE_TIMER(&policy_work_timer, "policy_work", thiz,
         OS_TIMER_PERIODIC, policy_work_timer_cb);
    CREATE_MSG_TYPE_QUEUE(&policy_work_msg_id, POLICY_WORK_MSG_NUM, 1);
}

