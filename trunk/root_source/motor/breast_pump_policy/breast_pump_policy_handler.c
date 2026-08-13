/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "motor_policy.h" // IWYU pragma: keep
#include "breast_pump_policy.h" // IWYU pragma: keep
#if IS_SUPPORT_MAKEUP_SUCTION
#include "pump_parameters_table.h"
#endif

/*
* DEFINES
*****************************************************************************************
*/
#define POLICY_WORK_MSG_NUM            8 /// 策略工作消息队列数量
#define MOTOR_POLICY_OBJ_MIN_ADDR      0x1000U

/// 放气等待可打断（默认随补吸策略开启）
#ifndef IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT
#if IS_SUPPORT_MAKEUP_SUCTION
#define IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT  1
#else
#define IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT  0
#endif
#endif

#if IS_SUPPORT_MAKEUP_SUCTION
#ifndef MAKEUP_SUCTION_LEVEL
#define MAKEUP_SUCTION_LEVEL            BREAST_PUMP_LEVEL_1  /// 补吸档位（吸乳1档）
#endif
#ifndef MAKEUP_SUCTION_LOOP_NUM
#define MAKEUP_SUCTION_LOOP_NUM         3   /// 补吸循环次数
#endif
#ifndef MAKEUP_SUCTION_SPEED
#define MAKEUP_SUCTION_SPEED            SUCTION_SPEED_LOW    /// 补吸速度档
#endif
#ifndef MAKEUP_SUCTION_RELEASE_MS
#define MAKEUP_SUCTION_RELEASE_MS       200 /// 补吸前/后放气等待时间
#endif
#ifndef MAKEUP_SUCTION_ABORT_RELEASE_MS
#define MAKEUP_SUCTION_ABORT_RELEASE_MS 50  /// 补吸被快速打断时放气等待时间
#endif
#endif

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
                LOG_MOTOR("==> find name: %s/%d", node->name, breast_pump_mode);
                if (node->get_policy_cb == NULL) {
                    LOGE("policy get cb null, name: %s/%d", node->name, breast_pump_mode);
                    return NULL;
                }
                const motor_policy_manager_t *policy_obj = node->get_policy_cb(node->user_data);

                if (!is_valid_motor_policy_obj(policy_obj)) {
                    LOGE("invalid policy obj: %p, name: %s/%d",
                        policy_obj, node->name, breast_pump_mode);
                    return NULL;
                } else {
                    LOG_MOTOR("==> policy obj: %p, name: %s/%d",
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
            info->state.action_start_time = get_timestamp();
            info->state.action_duration_ms = motor.un.pump.duration_ms + 100; 
            info->state.n_pressure_flag = 1;
            break;

        case MOTOR_ACTION_PUMP_OFF:
            motor.un.pump.pulse_width = 0;
            motor.un.pump.freq = BREAST_PUMP_PUMP_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_PUMP_OFF;
            motor.un.pump.duration_ms = info->original.hold_time;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, &motor);
            info->state.action_start_time = get_timestamp();
            info->state.action_duration_ms = motor.un.pump.duration_ms + 100;
            break;

        case MOTOR_ACTION_PUMP_OFF_AND_VALVE_ON:
            motor.un.pump.pulse_width = info->original.pump_level > 0 ? BREAST_PUMP_VALVE_DUTY * PUMP_DUTY_SCALE_FACTOR : 0;
            motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_PUMP_OFF_AND_VALVE_ON;
            motor.un.pump.duration_ms = info->original.valve_on_time == 0 ? 10 : info->original.valve_on_time;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, &motor);
            info->state.action_start_time = get_timestamp();
            info->state.action_duration_ms = motor.un.pump.duration_ms + 100;
            break;

        case MOTOR_ACTION_VALVE_ON:
            motor.un.pump.pulse_width =  info->original.pump_level > 0 ? BREAST_PUMP_VALVE_DUTY * PUMP_DUTY_SCALE_FACTOR : 0;
            motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_VALVE_ON;
            motor.un.pump.duration_ms = info->original.valve_on_time == 0 ? 10 : info->original.valve_on_time;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_START_PLAY, &motor);
            info->state.action_start_time = get_timestamp();
            info->state.action_duration_ms = motor.un.pump.duration_ms + 100;
            break;

        case MOTOR_ACTION_VALVE_OFF:
            motor.un.pump.pulse_width = 0;
            motor.un.pump.freq = BREAST_PUMP_VALVE_FREQ;
            motor.un.pump.pump_id = id;
            motor.un.pump.action = MOTOR_VALVE_OFF;
            motor.un.pump.duration_ms = info->original.sleep_time == 0 ? 2 : info->original.sleep_time;
            info->state.release_wait_time_ms = 0;
            DEVICE_IOCTL_ARGS(thiz->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, &motor);
            info->state.action_start_time = get_timestamp();
            info->state.action_duration_ms = motor.un.pump.duration_ms + 100;
            info->state.n_pressure_flag = 0;
            break;    
        default: 
            break;

    }
    info->state.action = MOTOR_ACTION_IDLE;
}

#if IS_SUPPORT_PUMP_POLICY_KICKOFF_OPT
static void pump_policy_run_devices(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime,
    motor_timer_event_msg_t *msg, bool kickoff_only)
{
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (runtime->policy[i].core.un.mapping.mode_type == SUCTION_MODE_IDLE) {
            continue;
        }
        if (kickoff_only && (runtime->info[i].state.status != SUCTION_STATUS_IDLE ||
            runtime->info[i].state.policy_update_lock != 0)) {
            continue;
        }
        pump_policy_update(&runtime->info[i], i, *msg);
        if (runtime->info[i].state.action != MOTOR_ACTION_IDLE) {
            update_para_to_motor_device(thiz, i, &runtime->info[i]);
        }
    }
}
#endif

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

static void policy_work_timer_cb(void *arg)
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

    if (MSG_QUEUE_SEND(policy_work_msg_id, &msg, 0) != 0) {
        LOGE("driver msg send failed");
    } else {
        // LOG_MOTOR("motor %d msg, action:%d", pump_id, action);
    }
}

#if IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT
/// user-stop 继续放气；play-new-task/同步停/休眠等快速打断
static bool is_pressure_release_should_abort(struct motor_manager_context *thiz)
{
    const char *reason = thiz->priv.interrupt_play_reason;

    if (reason == NULL || strcmp(reason, "user-stop") == 0) {
        return false;
    }
    LOG_MOTOR("release abort: %s", reason);
    return true;
}
#endif

/// 负压放气；开启 IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT 时等待可打断，返回是否被打断
static bool negative_pressure_release(struct motor_manager_context *thiz, uint16_t wait_time_ms)
{
    motor_para_t motor = {0};
#if IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT
    bool aborted = false;
#endif

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

    /// 注意：此处不做 pump_policy_info_reset。
    /// 退出路径上 change_motor_mode 可能已切换策略，若在此 reset 会用旧 climb 状态推进新策略，破坏渐变。

    while (wait_time_ms) {
#if IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT
        if (is_pressure_release_should_abort(thiz)) {
            aborted = true;
            break;
        }
#endif
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
#if IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT
    return aborted;
#else
    return false;
#endif
}

static void breast_pump_policy_exit_release(struct motor_manager_context *thiz, motor_policy_runtime_t *runtime)
{
    uint16_t wait_time_ms = 0;

    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (runtime->info[i].state.release_wait_time_ms > wait_time_ms) {
            wait_time_ms = runtime->info[i].state.release_wait_time_ms;
        }
    }
    negative_pressure_release(thiz, wait_time_ms == 0 ? 200 : wait_time_ms);
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
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        memcpy(&body->policy[i], &body->manager->head.policy, sizeof(motor_policy_node_t));
    }
#if IS_SUPPORT_CLIMB_POLICY
    pump_climb_policy_mark_auto(body->climb); /// 须在 policy_info_reset 前，保证首拍即开始爬升
#endif
    negative_pressure_release(thiz, 200);
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        pump_policy_info_reset(thiz, body, i);
    }
    if (!body->taste_run_flag) {
        thiz->priv.pump_ctx->priv.m_info.step_index = 0;
    }
    LOG_MOTOR("mode:%d, index:%d, type:%d/%d, time:%d/%d", 
        body->breast_pump_mode, body->current_timed_policy_index,
        body->current_mode_type, body->next_mode_type,
        body->time_policy_remaining_time, body->timed_policy->duration_seconds);
}

#if IS_SUPPORT_MAKEUP_SUCTION
/// 仅 user-stop（暂停/结束）进入补吸；新任务/同步停/休眠等不补吸
static bool is_makeup_suction_should_run(struct motor_manager_context *thiz)
{
    const char *reason = thiz->priv.interrupt_play_reason;

    if (MAKEUP_SUCTION_LOOP_NUM == 0 || thiz->priv.runtime.taste_run_flag) {
        return false;
    }
    if (reason == NULL || strcmp(reason, "user-stop") != 0) {
        return false;
    }
    if (thiz->priv.suction_para == NULL) {
        return false;
    }
    return true;
}

/// 补吸过程中快速退出，复用放气打断判定
static bool is_makeup_suction_should_abort(struct motor_manager_context *thiz)
{
#if IS_SUPPORT_PRESSURE_RELEASE_INTERRUPT
    return is_pressure_release_should_abort(thiz);
#else
    (void)thiz;
    return false;
#endif
}

static int makeup_suction_fill_original(struct motor_manager_context *thiz, uint8_t pump_id,
    motor_policy_original_para_t *original)
{
    uint8_t level = MAKEUP_SUCTION_LEVEL;
    uint8_t speed = MAKEUP_SUCTION_SPEED;
    uint16_t freq = 0;
    uint16_t total_time = 0;

    (void)pump_id;
    if (level >= BREAST_PUMP_LEVEL_MAX) {
        level = BREAST_PUMP_LEVEL_1;
    }
#if IS_USE_SPECTRA_SUCTION_CURVE
    if (speed >= PUMP_LONG_SUCTION_SPEED_MAX) {
        speed = SUCTION_SPEED_LOW;
    }
    {
        const suction_parameters_t *para = &thiz->priv.suction_para->long_suction_table[level][speed];
        original->pump_level = para->pump_level * PUMP_DUTY_SCALE_FACTOR;
        original->pumping_time = para->pumping_time;
        original->valve_on_time = para->valve_on_time;
        original->hold_time = para->hold_time;
        freq = thiz->priv.suction_para->long_pump_freq_table[speed];
    }
#else
    if (speed >= SUCTION_SPEED_MAX) {
        speed = SUCTION_SPEED_LOW;
    }
    {
        uint16_t pump_level_kpa = long_level_table[level];
        original->pump_level = thiz->priv.suction_para->long_suction_table[pump_level_kpa].pump_level * PUMP_DUTY_SCALE_FACTOR;
        original->pumping_time = thiz->priv.suction_para->long_suction_table[pump_level_kpa].pumping_time;
        original->valve_on_time = thiz->priv.suction_para->long_suction_table[pump_level_kpa].valve_on_time;
        original->hold_time = LONG_SUCTION_HOLD_TIME;
        freq = thiz->priv.suction_para->long_pump_freq_table[level][speed];
    }
#endif
    if (freq == 0) {
        freq = LONG_SUCTION_DEFAULT_FREQ;
    }
    total_time = ONE_MIN_TIME_MS / freq;
    if (total_time <= (original->pumping_time + original->valve_on_time + original->hold_time)) {
        original->sleep_time = PUMP_SLEEP_TIME_MIN;
    } else {
        original->sleep_time = total_time - original->pumping_time - original->valve_on_time - original->hold_time;
        if (original->sleep_time < PUMP_SLEEP_TIME_MIN || original->sleep_time > PUMP_SLEEP_TIME_MAX) {
            original->sleep_time = PUMP_SLEEP_TIME_MIN;
        }
    }
    return 0;
}

static void makeup_suction_prepare(struct motor_manager_context *thiz)
{
    motor_policy_runtime_t *runtime = &thiz->priv.runtime;

    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        motor_policy_node_t *policy = &runtime->policy[i];
        motor_policy_info_t *info = &runtime->info[i];

        memset(policy, 0, sizeof(*policy));
        policy->core.type = MOTOR_PARAM_PASS_THROUGH;
        policy->core.config.loop_num = MAKEUP_SUCTION_LOOP_NUM;
        makeup_suction_fill_original(thiz, i, &policy->core.un.original);

        memset(&info->state, 0, sizeof(info->state));
        memcpy(&info->config, &policy->core.config, sizeof(suction_policy_config_t));
        memcpy(&info->original, &policy->core.un.original, sizeof(motor_policy_original_para_t));
        runtime->pump_level[i] = MAKEUP_SUCTION_LEVEL;
        runtime->pump_speed[i] = MAKEUP_SUCTION_SPEED;
        runtime->reset_time[i] = get_timestamp();
    }
    LOG_MOTOR("makeup prepare level:%d, loop:%d, speed:%d",
        MAKEUP_SUCTION_LEVEL, MAKEUP_SUCTION_LOOP_NUM, MAKEUP_SUCTION_SPEED);
}

static bool is_makeup_suction_completed(motor_policy_runtime_t *runtime)
{
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (runtime->info[i].state.cycles_count < runtime->info[i].config.loop_num) {
            return false;
        }
        if (runtime->info[i].state.policy_update_lock) {
            return false;
        }
    }
    return true;
}

static void makeup_suction_advance_cycles(motor_policy_runtime_t *runtime)
{
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        /// 仅在一轮吸放结束（UPDATE_POLICY）后推进，避免 IDLE/lock==0 时重复打印
        if (runtime->info[i].state.status != SUCTION_STATUS_UPDATE_POLICY ||
            runtime->info[i].state.policy_update_lock != 0) {
            continue;
        }
        
        if (runtime->info[i].state.cycles_count < runtime->info[i].config.loop_num) {
            runtime->info[i].state.status = SUCTION_STATUS_IDLE;
            runtime->info[i].state.action = MOTOR_ACTION_IDLE;
            runtime->reset_time[i] = get_timestamp();
            LOG_MOTOR("makeup dev-%d loop:%d/%d",
                i, runtime->info[i].state.cycles_count, runtime->info[i].config.loop_num);
        }
    }
}

static void breast_pump_makeup_suction_handler(struct motor_manager_context *thiz)
{
    motor_timer_event_msg_t msg = {0};
    motor_policy_runtime_t *runtime = &thiz->priv.runtime;
    motor_policy_node_t saved_policy[BREAST_PUMP_NUM];
    uint8_t saved_level[BREAST_PUMP_NUM];
    uint8_t saved_speed[BREAST_PUMP_NUM];
    uint16_t wait_time_ms = 0;
    bool aborted = false;
    bool policy_overwritten = false;

    EXCEPTION_CHECK_VOID(thiz);
    if (!is_makeup_suction_should_run(thiz)) {
        return;
    }

    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (runtime->info[i].state.release_wait_time_ms > wait_time_ms) {
            wait_time_ms = runtime->info[i].state.release_wait_time_ms;
        }
    }
    if (wait_time_ms == 0) {
        wait_time_ms = MAKEUP_SUCTION_RELEASE_MS;
    }

    LOG_MOTOR("===makeup enter===");
    /// 先放气，再进入补吸循环
    if (negative_pressure_release(thiz, wait_time_ms)) {
        LOG_MOTOR("makeup abort after release");
        return;
    }

    /// 备份暂停前策略，避免补吸覆盖后影响 CONTINUE 恢复
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        saved_policy[i] = runtime->policy[i];
        saved_level[i] = runtime->pump_level[i];
        saved_speed[i] = runtime->pump_speed[i];
    }
    makeup_suction_prepare(thiz);
    policy_overwritten = true;
    MSG_QUEUE_RESET(policy_work_msg_id);

    while (1) {
        if (is_makeup_suction_should_abort(thiz)) {
            aborted = true;
            break;
        }
        memset(&msg, 0, sizeof(motor_timer_event_msg_t));
        MSG_QUEUE_RECV(policy_work_msg_id, &msg, OS_WAIT_FOREVER);
        if (is_makeup_suction_should_abort(thiz)) {
            aborted = true;
            break;
        }

        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            pump_policy_update(&runtime->info[i], i, msg);
            if (runtime->info[i].state.action != MOTOR_ACTION_IDLE) {
                update_para_to_motor_device(thiz, i, &runtime->info[i]);
            }
        }
        makeup_suction_advance_cycles(runtime);
        if (is_makeup_suction_completed(runtime)) {
            LOG_MOTOR("makeup completed");
            break;
        }
    }

    negative_pressure_release(thiz, aborted ? MAKEUP_SUCTION_ABORT_RELEASE_MS : MAKEUP_SUCTION_RELEASE_MS);
    if (policy_overwritten) {
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            runtime->policy[i] = saved_policy[i];
            runtime->pump_level[i] = saved_level[i];
            runtime->pump_speed[i] = saved_speed[i];
        }
    }
    LOG_MOTOR("===makeup exit, abort:%d===", aborted);
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
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
        breast_pump_start_prepare(thiz, policy_obj);
        thiz->notify_motor_status_change(MOTOR_STATUS_EVT_TASTE_START);
        LOG_MOTOR("user debug policy");
    } else if (action == MOTOR_POLICY_CONTINUE && body->breast_pump_mode == pump_mode) {
        LOG_MOTOR("continue %s policy, step_index: %d", mode_name, body->current_timed_policy_index);
#if IS_SUPPORT_CLIMB_POLICY
        pump_climb_policy_mark_auto(body->climb);
#endif
        negative_pressure_release(thiz, 200);
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            pump_policy_info_reset(thiz, body, i);
        }
    } else {
        body->policy_running_time_ms = 0;
        body->breast_pump_mode = pump_mode;
        policy_obj = breast_pump_mode_policy_get(thiz, body->breast_pump_mode);
        LOG_MOTOR("%s mode policy", mode_name);
        if (!is_valid_motor_policy_obj(policy_obj)) {
            LOGE("invalid policy obj: %p, mode: %s/%d", policy_obj, mode_name, body->breast_pump_mode);
            // 不返回, 触发重启复现问题
        }
        breast_pump_start_prepare(thiz, policy_obj);
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
#if IS_SUPPORT_PUMP_POLICY_KICKOFF_OPT
        pump_policy_run_devices(thiz, runtime, &msg, false);
        pump_time_policy_update(thiz, runtime);
        pump_policy_run_devices(thiz, runtime, &msg, true);
#else
        pump_time_policy_update(thiz, runtime);  ///  时间策略
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {   
            if (runtime->policy[i].core.un.mapping.mode_type == SUCTION_MODE_IDLE) {
                continue;
            } 
            pump_policy_update(&runtime->info[i], i, msg);
            if (runtime->info[i].state.action != MOTOR_ACTION_IDLE) {
                update_para_to_motor_device(thiz, i, &runtime->info[i]);
            }
        }
#endif

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
#if IS_SUPPORT_MAKEUP_SUCTION
    if (is_makeup_suction_should_run(thiz)) {
        /// 暂停/吸奶结束：主循环退出后进入补吸
        breast_pump_makeup_suction_handler(thiz);
    } else {
        breast_pump_policy_exit_release(thiz, runtime);
    }
#else
    breast_pump_policy_exit_release(thiz, runtime);
#endif
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
    if (!thiz->priv.is_suction_compensation_enabled) {
        //LOG_MOTOR("suction compensation is disabled");
        return;
    }
    
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
#if IS_SUPPORT_CLIMB_POLICY
    pump_climb_policy_mark_auto(body->climb); /// 切模式时清空，避免退出路径误用旧 climb 状态
#endif
    if (is_continue) {
        thiz->start_motor(MOTOR_POLICY_CHANGE_MODE);
    }
    LOG_MOTOR("mode change: %d, type: %d, next_type: %d, time: %d", 
        mode, body->current_mode_type, body->next_mode_type, body->time_policy_remaining_time);
    return 0;
}

void motor_policy_finish(struct motor_manager_context *thiz)
{
    motor_policy_runtime_t *runtime = &thiz->priv.runtime;
    runtime->current_timed_policy_index = 0;
    if (runtime->manager != NULL) {
        runtime->timed_policy = &runtime->manager->head;
        runtime->current_mode_type = pump_policy_type_get(runtime->timed_policy->policy);
        if (runtime->timed_policy->next != NULL) {
            runtime->next_mode_type = pump_policy_type_get(runtime->timed_policy->next->policy);
        } else {
            runtime->next_mode_type = runtime->current_mode_type;
        }
    }
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

