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
 
static int internal_stop_motor(void)
{
    int ret = -1;
    uint32_t args[1] = {MOTOR_PWM_ALL_ID};
    if (motor_obj->priv.motor_dev && motor_obj->priv.motor_dev->ioctl) {
        ret = DEVICE_IOCTL_ARGS(motor_obj->priv.motor_dev, OP_CODE_MOTOR_STOP_PLAY, args);
    }
    memset(motor_obj->priv.runtime.info, 0, sizeof(motor_policy_info_t) * BREAST_PUMP_NUM);
    LOG_MOTOR("ret: %d", ret);
    return ret;
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
            {
                breast_pump_state = arg2;
                LOG_MOTOR("state change: %d", breast_pump_state);
            }
            break;
        case ID_BROADCAST_BREAST_PUMP_MODE_CHANGE:
            {
                if (motor_obj->priv.pump_ctx && motor_obj->priv.pump_ctx->is_on_working() > 0) {
                    internal_start_motor(MOTOR_POLICY_CHANGE_MODE);
                }
                LOG_MOTOR("mode change: %d", breast_pump_mode);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE:
            {
                if (arg1 == ID_SYS_PM_RESUME) {
                    LOG_MOTOR("reset history_node");
                }
            }
            break;
        case ID_BROADCAST_REQ_PM_SUSPEND_BY_USER:
            {
                update_play_interrupt_reason("SUSPEND_BY_USER");
            }
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
    thiz->priv.m_bdc.id.code |= ID_BROADCAST_BREAST_PUMP_MODE_CHANGE;
    thiz->priv.m_bdc.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    thiz->priv.m_bdc.id.code |= ID_BROADCAST_UPDATE_MOTOR_WORK_PARAMETER;
    thiz->priv.m_bdc.id.code |= ID_BROADCAST_REQ_PM_SUSPEND_BY_USER;
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
    return internal_stop_motor();
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

    if (motor_obj->priv.pump_ctx->is_on_working() == 0 && motor_obj->priv.user_debug_status == 0) {
        LOG_MOTOR("exit reason: not on pumping");
        update_play_interrupt_reason("pumping_state:err");
        return 0;
    }

    return 1;
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
    motor_obj = ptr;
    
    return ADD_MODULE("motor", ID_MODULE_MOTOR, 0, on_post_motor_init, 
        ptr, list_head, sizeof(motor_manager_context_t));
}
