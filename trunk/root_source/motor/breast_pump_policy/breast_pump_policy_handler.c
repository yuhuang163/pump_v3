/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "motor_policy.h" // IWYU pragma: keep
#include "breast_pump_policy.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POLICY_WORK_MSG_NUM            4 /// 策略工作消息队列数量
#define MOTOR_POLICY_OBJ_MIN_ADDR      0x1000U

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
static bool is_valid_motor_policy_obj(const motor_policy_manager_t *policy_obj)
{
    return policy_obj != NULL && (uintptr_t)policy_obj >= MOTOR_POLICY_OBJ_MIN_ADDR;
}

static const motor_policy_manager_t *breast_pump_mode_policy_get(struct motor_manager_context *thiz, int breast_pump_mode)
{
    list_head_t *head = &thiz->priv.policy_list;
    if (head->next != NULL) {
        LIST_FOR_EACH(pos, head) {
            breast_pump_policy_node_t *node = CONTAINER_OF(pos, breast_pump_policy_node_t, list);
            if (node->breast_pump_mode == breast_pump_mode) {
                LOG_MOTOR("==> find name: %s, breast_pump_mode: %d", node->name, breast_pump_mode);
                if (node->get_policy_cb == NULL) {
                    LOGE("policy get cb null, name: %s, breast_pump_mode: %d", node->name, breast_pump_mode);
                    return NULL;
                }
                const motor_policy_manager_t *policy_obj = node->get_policy_cb(node->user_data);

                if (!is_valid_motor_policy_obj(policy_obj)) {
                    LOGE("invalid policy obj: %p, name: %s, breast_pump_mode: %d",
                        policy_obj, node->name, breast_pump_mode);
                    return NULL;
                } else {
                    LOG_MOTOR("==> policy obj: %p, name: %s, breast_pump_mode: %d",
                        policy_obj, node->name, breast_pump_mode);
                }
                return policy_obj;
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
    switch (info->state.action) {
        case MOTOR_ACTION_PUMP_ON:
            motor.un.pump.pulse_width = info->original.pump_level;
            motor.un.pump.freq = BREAST_PUMP_PUMP_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_PUMP_ON;
            motor.un.pump.duration_ms = info->original.pumping_time == 0 ? 10 : info->original.pumping_time;
            info->state.pump_duty = motor.un.pump.pulse_width / PUMP_DUTY_SCALE_FACTOR;
            info->state.release_wait_time_ms = info->original.valve_on_time;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, &motor);
            info->state.n_pressure_flag = 1;
            break;

        case MOTOR_ACTION_PUMP_OFF:
            motor.un.pump.pulse_width = 0;
            motor.un.pump.freq = BREAST_PUMP_PUMP_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_PUMP_OFF;
            motor.un.pump.duration_ms = info->original.hold_time;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, &motor);
            break;

        case MOTOR_ACTION_PUMP_OFF_AND_VALVE_ON:
            motor.un.pump.pulse_width = info->original.pump_level > 0 ? BREAST_PUMP_VALVE_DUTY * PUMP_DUTY_SCALE_FACTOR : 0;
            motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_PUMP_OFF_AND_VALVE_ON;
            motor.un.pump.duration_ms = info->original.valve_on_time == 0 ? 10 : info->original.valve_on_time;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, &motor);
            break;

        case MOTOR_ACTION_VALVE_ON:
            motor.un.pump.pulse_width =  info->original.pump_level > 0 ? BREAST_PUMP_VALVE_DUTY * PUMP_DUTY_SCALE_FACTOR : 0;
            motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_VALVE_ON;
            motor.un.pump.duration_ms = info->original.valve_on_time == 0 ? 10 : info->original.valve_on_time;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, &motor);
            break;

        case MOTOR_ACTION_VALVE_OFF:
            motor.un.pump.pulse_width = 0;
            motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_VALVE_OFF;
            motor.un.pump.duration_ms = info->original.sleep_time == 0 ? 2 : info->original.sleep_time;
            info->state.release_wait_time_ms = 0;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, &motor);
            info->state.n_pressure_flag = 0;
            break;
        default:
            break;

    }
    info->state.action = MOTOR_ACTION_IDLE;
}

#if IS_SUPPORT_PUMP_DUTY_ADJUST
static void pump_duty_adjust_to_motor_device(struct motor_manager_context *thiz, uint8_t id, int16_t pulse_width)
{
    if (abs(pulse_width) > 2000) {
        //LOGE("pulse_width is out of range: %d", pulse_width);
        //return;
    }
    motor_para_t motor = {0};
    motor.un.pump.pulse_width = pulse_width;
    motor.un.pump.freq = BREAST_PUMP_PUMP_FREQ;
    motor.un.pump.pump_id = id;
    motor.un.pump.action = MOTOR_PUMP_DUTY_ADJUST;
    motor.un.pump.duration_ms = 0;
    DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_SET_PARAMETER, &motor);
}
#endif

static void policy_work_timer_cb(os_timer_id *timer)
{
    motor_timer_event_msg_t msg = {
        .msg_type = MOTOR_TASK_HEARTBEAT_TIMER_MSG,
        .un.action = 0,
        .pump_id = 0,
    };
    MSG_QUEUE_SEND(policy_work_msg_id, &msg, 0);
}

static void on_motor_timer_event_cb(uint8_t pump_id, motor_pump_status_e action)
{
    motor_timer_event_msg_t msg = {
        .msg_type = MOTOR_DRIVER_TIMER_EVENT_MSG,
        .un.action = action,
        .pump_id = pump_id,
    };
    MSG_QUEUE_SEND(policy_work_msg_id, &msg, 0);
}

static void breast_pump_start_prepare(struct motor_manager_context *thiz, const motor_policy_manager_t *policy_obj)
{
    motor_policy_runtime_t *body = &thiz->priv.runtime;
    body->manager = (motor_policy_manager_t *)policy_obj;
    body->timed_policy = &body->manager->head;
    body->current_mode_type = pump_policy_type_get(body->timed_policy->policy);
    if (body->timed_policy->next) {
        body->next_mode_type = pump_policy_type_get(body->timed_policy->next->policy);
    } else {
        body->next_mode_type = body->current_mode_type;
    }
    body->current_timed_policy_index = 0;
    body->policy_running_time_ms = 0;
    body->time_policy_remaining_time = body->timed_policy->duration_seconds;
    body->time_policy_target_time =  body->timed_policy->duration_seconds;
#if IS_SUPPORT_CLIMB_POLICY
    body->climb.onoff = 1;
    body->climb.count = 0;  
    body->climb.start_time = 0;
#endif
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        memcpy(&body->policy[i], &body->manager->head.policy, sizeof(motor_policy_node_t));
    }
    negative_pressure_release(thiz, 200);
    if (!body->taste_run_flag) {
        thiz->priv.pump_ctx->priv.m_info.step_index = 0;
    }
    LOG_MOTOR("mode:%d, index:%d, type:%d/%d, time:%d/%d", 
        body->breast_pump_mode, body->current_timed_policy_index,
        body->current_mode_type, body->next_mode_type,
        body->time_policy_remaining_time, body->timed_policy->duration_seconds);
} 

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void negative_pressure_release(struct motor_manager_context *thiz, uint16_t wait_time_ms)
{   
    motor_para_t motor = {0};
    motor.un.pump.pulse_width = 0;
    motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
    motor.un.pump.action = MOTOR_PUMP_OFF_AND_VALVE_OFF;
    motor.un.pump.duration_ms = 0;
    LOG_MOTOR("wait_time_ms: %d", wait_time_ms);
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        motor.un.pump.pump_id = i;
        DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, &motor);
    }
    msleep(5);
    LOG_MOTOR("-->step 2");
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        motor.un.pump.pump_id = i;
        motor.un.pump.pulse_width = BREAST_PUMP_VALVE_DUTY * PUMP_DUTY_SCALE_FACTOR;
        motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
        motor.un.pump.action = MOTOR_PUMP_OFF_AND_VALVE_ON;
        DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, &motor);
    }
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        pump_policy_info_reset(thiz, &thiz->priv.runtime, i);
    }
    while (wait_time_ms) {
        msleep(10);
        wait_time_ms = wait_time_ms > 10 ? wait_time_ms - 10 : 0;
    }
    LOG_MOTOR("-->step 3");
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        motor.un.pump.pulse_width = 0;
        motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
        motor.un.pump.action = MOTOR_PUMP_OFF_AND_VALVE_OFF;
        motor.un.pump.duration_ms = 0;
        motor.un.pump.pump_id = i;
        DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, &motor);
    }
}

void breast_pump_policy_prepare(struct motor_manager_context *thiz, uint8_t action)
{
    motor_policy_runtime_t *body = &thiz->priv.runtime;
    const motor_policy_manager_t *policy_obj = NULL;
    int pump_mode = 0;
    char *mode_name = "unknow";
    READ_PUMP_WORK_MODE(thiz->priv.pump_ctx, pump_mode);
    READ_PUMP_WORK_MODE_NAME(thiz->priv.pump_ctx, &mode_name);
    body->taste_run_flag = 0; /// 常规模式下，体验模式标志位清零
    if (action == MOTOR_USER_DEBUG_POLICY_START) {
        body->taste_run_flag = 1;
        policy_obj = thiz->priv.user_debug_policy;
        LOG_MOTOR("user debug policy");
    } else if (action == MOTOR_POLICY_CONTINUE) {
        if (body->breast_pump_mode == pump_mode) {
            LOG_MOTOR("continue %s policy, step_index: %d", mode_name, body->current_timed_policy_index);
            negative_pressure_release(thiz, 200);
            return;
        } else {
            LOG_MOTOR("breast pump mode changed, need to restart policy");
            body->policy_running_time_ms = 0;
            body->breast_pump_mode = pump_mode;
            policy_obj = breast_pump_mode_policy_get(thiz, body->breast_pump_mode);
            LOG_MOTOR("%s mode policy", mode_name);
        }
    } else {
        body->policy_running_time_ms = 0;
        body->breast_pump_mode = pump_mode;
        policy_obj = breast_pump_mode_policy_get(thiz, body->breast_pump_mode);
        LOG_MOTOR("%s mode policy", mode_name);
    }
    if (!is_valid_motor_policy_obj(policy_obj)) {
        LOGE("invalid policy obj: %p, mode: %s, breast_pump_mode: %d",
            policy_obj, mode_name, body->breast_pump_mode);
        // return; /// 不返回, 触发重启复现问题
    }
    breast_pump_start_prepare(thiz, policy_obj);
    if (body->taste_run_flag) {
        thiz->notify_motor_status_change(MOTOR_STATUS_EVT_TASTE_START);
    }
}

int breast_pump_mode_policy_handler(struct motor_manager_context *thiz, motor_task_para_t *play_para)
{
    motor_timer_event_msg_t msg = {0};
    int exit_code = EXIT_REASON_PLAY_COMPLETED;
    motor_policy_runtime_t *runtime = &thiz->priv.runtime;
    LOG_MOTOR("===enter===");
    EXCEPTION_CHECK_INT(runtime->manager);
    MSG_QUEUE_RESET(policy_work_msg_id);
    START_TIMER(policy_work_timer, PUMP_POLICY_RUN_TIME_INTERVAL);
    uint32_t policy_start_time_ms = get_timestamp();
    while (1) {
        memset(&msg, 0, sizeof(motor_timer_event_msg_t));
        MSG_QUEUE_RECV(policy_work_msg_id, &msg, OS_WAIT_FOREVER);
        runtime->policy_running_time_ms += get_timestamp_ms(policy_start_time_ms);
        policy_start_time_ms = get_timestamp();
        if (!thiz->is_can_play_now()) {
            exit_code = EXIT_REASON_INTERRUPT;
            break;
        }
        pump_time_policy_update(thiz, runtime);  ///  时间策略
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {   
            if (runtime->policy[i].core.un.mapping.mode_type == SUCTION_MODE_IDLE) {
                continue;
            } 
            pump_policy_update(&runtime->info[i], i, msg);    
            update_para_to_motor_device(thiz, i, &runtime->info[i]);   
        }
#if IS_SUPPORT_PUMP_DUTY_ADJUST
        if (msg.msg_type == MOTOR_PUMP_LEVEL_ADJUST_MSG) {
            for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
                if (runtime->compensation_delta_pwm[i] != msg.un.pulse_width[i]) {
                    runtime->compensation_delta_pwm[i] = msg.un.pulse_width[i];
                    pump_duty_adjust_to_motor_device(thiz, i, runtime->compensation_delta_pwm[i]);
                }
            }
        }
#endif
    }
    uint16_t wait_time_ms = 0;
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (runtime->info[i].state.release_wait_time_ms > wait_time_ms) {
            wait_time_ms = runtime->info[i].state.release_wait_time_ms;
        }
    }
    negative_pressure_release(thiz, wait_time_ms == 0 ? 200 : wait_time_ms);
    STOP_TIMER(policy_work_timer);
    if (runtime->taste_run_flag) {
        thiz->notify_motor_status_change(MOTOR_STATUS_EVT_TASTE_STOP);
        runtime->taste_run_flag = 0;
    }
#if IS_SUPPORT_PUMP_DUTY_ADJUST
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        runtime->compensation_delta_pwm[i] = 0;
    }
#endif
    LOG_MOTOR("exit:%d", exit_code);
    return exit_code;
}
 
#if IS_SUPPORT_PUMP_DUTY_ADJUST
static void pump_algo_compensation_callback(void *user_data, algo_breast_pump_data_t *result)
{
    /// 气压补偿回调打印
    motor_manager_context_t *thiz = (motor_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
    motor_policy_runtime_t *runtime = &thiz->priv.runtime;
    if (thiz->get_motor_state() == 0) {
        return;
    }
    
    if (runtime->compensation_delta_pwm[0] == result->wear_detection.compensation_delta_pwm[0] &&
        runtime->compensation_delta_pwm[1] == result->wear_detection.compensation_delta_pwm[1]) {
        return;
    }
    LOG_MOTOR("compensation status %d/%d; %d/%d", 
        result->wear_detection.compensation_status[0], result->wear_detection.compensation_status[1], 
        result->wear_detection.compensation_delta_pwm[0], result->wear_detection.compensation_delta_pwm[1]);
    motor_timer_event_msg_t msg = {
        .msg_type = MOTOR_PUMP_LEVEL_ADJUST_MSG,
        .un.pulse_width[0] = result->wear_detection.compensation_delta_pwm[0],
        .un.pulse_width[1] = result->wear_detection.compensation_delta_pwm[1],
        .pump_id = 0,
    };
    MSG_QUEUE_SEND(policy_work_msg_id, &msg, 0);
}
#endif

int change_motor_mode(struct motor_manager_context *thiz, uint8_t mode, bool is_continue)
{
    EXCEPTION_CHECK_INT(thiz);
    motor_policy_runtime_t *body = &thiz->priv.runtime;
    const motor_policy_manager_t *policy_obj = breast_pump_mode_policy_get(thiz, mode);
    if (!is_valid_motor_policy_obj(policy_obj)) {
        LOGE("invalid policy obj: %p, pump_mode: %d", policy_obj, mode);
        return -1;
    }
    body->manager = (motor_policy_manager_t *)policy_obj;
    body->timed_policy = &body->manager->head;
    body->breast_pump_mode = mode;
    body->current_mode_type = pump_policy_type_get(body->timed_policy->policy);
    if (body->timed_policy->next) {
        body->next_mode_type = pump_policy_type_get(body->timed_policy->next->policy);
    } else {
        body->next_mode_type = body->current_mode_type;
    }
    body->current_timed_policy_index = 0;
    body->time_policy_remaining_time = body->timed_policy->duration_seconds;
    body->time_policy_target_time = body->timed_policy->duration_seconds;
    body->policy_running_time_ms = 0;
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        memcpy(&body->policy[i], &body->timed_policy->policy, sizeof(motor_policy_node_t));
    }
    if (is_continue) {
        thiz->start_motor(MOTOR_POLICY_CHANGE_MODE);
    }
    LOG_MOTOR("mode change: %d, type: %d, next_type: %d, time: %d", 
        mode, body->current_mode_type, body->next_mode_type, body->time_policy_remaining_time);
    return 0;
}

void motor_policy_init(struct motor_manager_context *thiz)
{
    // negative_pressure_release(thiz, 200);
    CREATE_TIMER(&policy_work_timer, "policy_work", thiz, OS_TIMER_PERIODIC, policy_work_timer_cb);
    CREATE_MSG_TYPE_QUEUE(&policy_work_msg_id, POLICY_WORK_MSG_NUM, sizeof(motor_timer_event_msg_t));
    DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_ADD_TIMER_EVENT_CALLBACK, &on_motor_timer_event_cb);
    /// 注册算法气压补偿回调
#if IS_SUPPORT_PUMP_DUTY_ADJUST
    algo_manager_context_t *algo_ctx = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    EXCEPTION_CHECK_VOID(algo_ctx && algo_ctx->hook);
    algo_ctx->add_algo_result_listener(thiz, (on_algo_result_cb_ptr)pump_algo_compensation_callback);
#endif
}

