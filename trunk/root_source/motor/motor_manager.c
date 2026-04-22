/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "motor_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define MOTOR_PLAY_THD_WAKELOCK                            "motor_paly_wakelock"
#define MOTOR_PLAY_WAKELOCK_TIMEOUT                        (15 * 60)
#define DEBUG_MOTOR_TASK_PLAY                              1

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static motor_manager_context_t *motor_obj = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void update_play_interrupt_reason(const char *reason)
{
    if (motor_obj->priv.interrupt_play_reason != reason) {
        motor_obj->priv.interrupt_play_reason = (char *)reason;
    }
}

static void motor_play_work_thread(void *args)
{
    motor_task_para_t task_para = {0};
    thread_context_t *thd = (thread_context_t *)args;
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, MOTOR_PLAY_MESQ_NUM, sizeof(motor_task_para_t));
    for (;;) {
        set_thread_running(thd, false);
        LOG_MOTOR("idle");
        MSG_QUEUE_RECV(thd->msg_id, &task_para, OS_WAIT_FOREVER);
        LOG_MOTOR("running timestamp: %ld, id: %d, action: %d", task_para.timestamp, task_para.play_mask_id, task_para.action_id);
        set_thread_running(thd, true);
        update_play_interrupt_reason(NULL);
        if (pm->acquire_wakelock(MOTOR_PLAY_THD_WAKELOCK, (MOTOR_PLAY_WAKELOCK_TIMEOUT)) == 0) {
            if (motor_obj->hook == NULL) {
                continue;
            }
            
            if (motor_obj->hook->motor_policy_prepare) {
                motor_obj->hook->motor_policy_prepare(motor_obj, task_para.action_id);
            }

            if (motor_obj->hook->mode_policy_handler) {
                motor_obj->hook->mode_policy_handler(motor_obj, &task_para);
            }
        } else {
            LOGE("acquire wakelock fail");
        }

        if (pm->release_wakelock(MOTOR_PLAY_THD_WAKELOCK) < 0) {
            LOGE("release wakelock fail");
        }
    } 
}

static bool is_on_motor_task_running(motor_manager_context_t *ctx)
{
    EXCEPTION_CHECK_BOOL(ctx);
    return get_thread_running(&ctx->priv.motor_play_thd);
}

static int internal_start_motor(uint8_t action)
{
    if (action >= MOTOR_POLICY_ACTION_MAX) {
        action = MOTOR_POLICY_START;
    }
    
    motor_task_para_t task_para = {
        .timestamp = get_timestamp(),
        .play_mask_id = ID_MASK_PLAY_BREAST_PUMP_POLICY,
        .action_id = action, 
    };

    os_message_id msg_id = motor_obj->priv.motor_play_thd.msg_id;
    if (is_on_motor_task_running(motor_obj)) {
        update_play_interrupt_reason("play-new-task");
    }
    
    if (msg_id) {
        LOG_MOTOR("timestamp: %ld, id: %d", task_para.timestamp, task_para.play_mask_id);
        MSG_QUEUE_SEND(msg_id, &task_para, 0);
    }

    return 0;
}
 
static int internal_stop_motor(const char *reason)
{
    if (reason) {
        update_play_interrupt_reason(reason);
    } else {
        update_play_interrupt_reason("unknown-reason");
    }
    return 0;
}

static void on_motor_broadcast_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    motor_manager_context_t *thiz = context;
    struct __motor_manager_hook *hook = thiz->hook;
    int breast_pump_state = -1;
    int breast_pump_mode = -1;

    if (hook && hook->on_motor_receiver_route && (hook->on_motor_receiver_route(id, thiz, arg1, arg2, pdata))) {
        LOG_MOTOR("routed return: %lx", id.code);
        return;
    }

    switch (id.code) {
        case ID_BROADCAST_BREAST_PUMP_STATE_CHANGE:
            if (arg1 == BREAST_PUMP_BROADCAST_MODE_CHANGE) {
                breast_pump_mode = arg2;
                if (motor_obj->priv.pump_ctx && motor_obj->priv.runtime.breast_pump_mode != breast_pump_mode) { 
                    int is_pump_working = 0; 
                    READ_PUMP_IS_WORKING(motor_obj->priv.pump_ctx, is_pump_working);
                    if (is_pump_working) {
                        motor_mode_change_handler(thiz, breast_pump_mode);
                        internal_start_motor(MOTOR_POLICY_CHANGE_MODE);
                    }       
                }
                LOG_MOTOR("mode change: %d", breast_pump_mode);
            } else if (arg1 == BREAST_PUMP_BROADCAST_STATE_CHANGE) {
                breast_pump_state = arg2;
                LOG_MOTOR("state change: %d", breast_pump_state);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == PM_BROADCAST_SYS_RESUME) {
                LOG_MOTOR("reset history_node");
            }
            break;
        case ID_BROADCAST_REQ_PM_SUSPEND:
            update_play_interrupt_reason("SUSPEND_BY_USER");
            break;
        default:
            break;
    }
}

static int on_post_motor_init(void *ptr)
{
    motor_manager_context_t *thiz = (motor_manager_context_t *)ptr;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    thiz->priv.pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    thiz->priv.motor_dev = pbus->find_device(ID_DEVICE_MOTOR);

    /// 注册广播监听.
    thiz->priv.m_bdc.context = thiz;
    thiz->priv.m_bdc.name = "motor_manager";
    thiz->priv.m_bdc.id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    thiz->priv.m_bdc.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    thiz->priv.m_bdc.id.code |= ID_BROADCAST_REQ_PM_SUSPEND;
    thiz->priv.m_bdc.receiver_cb = on_motor_broadcast_receiver;
    register_broadcast_receiver(&thiz->priv.m_bdc);

    /// 钩子参数初始化
    if (thiz->hook && thiz->hook->on_post_init_route) {
        thiz->hook->on_post_init_route(thiz);
    }

    MUTEX_INIT(&thiz->priv.motor_play_thd.lock_id, m_motor_play_lock);
    motor_policy_init(thiz);
    RUN_NEW_THREAD(&thiz->priv.motor_play_thd, "motor_play_thd", motor_play_work_thread, 
                   RT_PRIO_MOTOR_DISPATCH_THREAD, MOTOR_PLAY_THREAD_STACKSIZE);

    return 0;
}

static int start_motor(motor_policy_action_e action)
{
    motor_obj->priv.is_user_stoped = 0;
    if (action != MOTOR_USER_DEBUG_POLICY_START) {
        motor_obj->priv.user_debug_status = 0;
    }
    return internal_start_motor(action);   
}

static int stop_motor(void)
{
    motor_obj->priv.is_user_stoped = 1;
    motor_obj->priv.user_debug_status = 0;
    return internal_stop_motor("user-stop");
}

static int synchronous_stop_motor(void)
{
    motor_obj->priv.is_user_stoped = 1;
    motor_obj->priv.user_debug_status = 0;
    internal_stop_motor("synchronous-stop");
    uint32_t start_time = get_timestamp();
    while (is_on_motor_task_running(motor_obj)) {
        msleep(10);
        if (get_timestamp_ms(start_time) > 100) {
            LOGE("==> stop timeout");
            return -1;
        }
        start_time = get_timestamp();
    }
    LOG_MOTOR("==> stop success");
    return 0;
}

static int get_motor_state(void)
{
    return (int)motor_obj->priv.motor_state;
}

int motor_is_can_play_now(void)
{
    if (motor_obj->priv.interrupt_play_reason) {
        LOG_MOTOR("exit reason: %s", motor_obj->priv.interrupt_play_reason);
        return 0;
    }
    
    if (motor_obj->priv.pump_ctx && motor_obj->priv.user_debug_status == 0) {
        int is_pump_working = 0; 
        READ_PUMP_IS_WORKING(motor_obj->priv.pump_ctx, is_pump_working);
        if (is_pump_working == 0) {
            LOG_MOTOR("exit reason: not on pumping");
            update_play_interrupt_reason("pumping_state:err");
            return 0;
        }   
    }
    return 1;
}

static int get_runtime_policy_index(motor_policy_type_e type)
{
    if (type == MOTOR_TIME_POLICY) {
        return motor_obj->priv.runtime.current_timed_policy_index;
    } else {
        return 0;
    }
}
static int set_runtime_policy_index(motor_policy_type_e type, int index)
{
    if (type == MOTOR_TIME_POLICY) {
        motor_obj->priv.runtime.current_timed_policy_index = index;
    } else {
        LOG_MOTOR("set runtime policy index error, type: %d", type);
    }
    return 0;
}


static uint8_t get_runtime_mode_type(uint8_t is_next_mode)
{
    if (is_next_mode) {
        return motor_obj->priv.runtime.next_mode_type;
    } else {
        return motor_obj->priv.runtime.current_mode_type;
    }
}

static uint8_t get_runtime_policy_mode_type(void)
{
    return motor_obj->priv.runtime.current_policy_mode_type;
}

static uint32_t get_time_policy_remaining_time(void)
{
    return motor_obj->priv.runtime.time_policy_remaining_time;
}

static int set_time_policy_remaining_time(uint32_t remaining_time)
{
    motor_obj->priv.runtime.time_policy_remaining_time = remaining_time;
    return 0;
}

static uint32_t get_current_policy_pump_time(uint8_t pump_id)
{
    return motor_obj->priv.runtime.info[pump_id].original.pumping_time;
}

#if MOTOR_DRIVER_VERSION == MOTOR_DRIVER_V2
static uint32_t get_policy_running_time(void)
{
    return motor_obj->priv.runtime.policy_running_time_ms;
}
#endif

static uint8_t get_timed_policy_finish_flag(void)
{
    return motor_obj->priv.runtime.all_timed_policy_finish_flag;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_motor_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, motor_manager_context_t);
    ptr->get_motor_state = get_motor_state;
    ptr->start_motor = start_motor;
    ptr->stop_motor = stop_motor;
    ptr->is_on_motor_task_running = is_on_motor_task_running;
    ptr->is_can_play_now = motor_is_can_play_now;
    ptr->get_runtime_policy_index = get_runtime_policy_index;
    ptr->set_runtime_policy_index = set_runtime_policy_index;
    ptr->get_runtime_mode_type = get_runtime_mode_type;
    ptr->get_runtime_policy_mode_type = get_runtime_policy_mode_type;
    ptr->get_time_policy_remaining_time = get_time_policy_remaining_time;
    ptr->set_time_policy_remaining_time = set_time_policy_remaining_time;
    ptr->get_current_policy_pump_time = get_current_policy_pump_time;
#if MOTOR_DRIVER_VERSION == MOTOR_DRIVER_V2
    ptr->get_policy_running_time = get_policy_running_time; 
#endif
    ptr->get_timed_policy_finish_flag = get_timed_policy_finish_flag;
    ptr->synchronous_stop_motor = synchronous_stop_motor;
    motor_obj = ptr;
    
    return ADD_MODULE("motor", ID_MODULE_MOTOR, 0, on_post_motor_init, 
        ptr, list_head, sizeof(motor_manager_context_t));
}
