/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "power_manager.h"

/*
* DEFINES
*****************************************************************************************
*/
#define LOG_TAG "power_manager"
#define PM_SUSPEND_WORK_PERIOD            (1000)
#define MAX_LOCK_TIME_S                   (3600 * 2)

#define MUTEX_LOCK_PM(tag, __lock) \
do { \
    MUTEX_LOCK(__lock); \
    LOG_PM("pm_suspend_lock=>Take:%s", tag); \
} while (0)

#define MUTEX_UNLOCK_PM(tag, __lock) \
do { \
    LOG_PM("pm_suspend_lock=>Give:%s", tag); \
    MUTEX_UNLOCK(__lock); \
} while (0)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static power_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int pm_check_wakelock(power_manager_context_t *ctx);
static void clear_all_wakelock(power_manager_context_t *ctx);

static void set_suspend_counter(const char *tag, int counter, uint8_t log_out)
{
    thiz->priv.suspend_counter = counter;
    if (thiz->hook && thiz->hook->set_suspend_counter_route) {
        thiz->hook->set_suspend_counter_route(thiz);
    }

    if (log_out) {
        LOG_PM("tag:%s New counter: %d", tag, counter);
    }
}

static const char *print_wakeup_state(int state)
{
    switch (state) {
        case PM_SUSPEND_GOING:
            return "SUSPEND_GOING";
        case PM_SUSPEND_FINISH:
            return "SUSPEND_FINISH";
        case PM_RESUME_GOING:
            return "RESUME_GOING";
        case PM_RESUME_FINISH:
            return "RESUME_FINISH";
        case PM_WILL_POWER_OFF:
            return "WILL_POWER_OFF";
        case PM_POWER_OFF:
            return "POWER_OFF";
        default:
            return "error state";
    }
}

static int get_suspend_timeout(power_manager_context_t *thiz)
{
    return PM_SUSPEND_TIME_S;
}

static void dispatch_pm_event(power_manager_context_t *ctx, int args)
{
    multi_func_msg_data_t msg = {0};
    msg.union_body.parms[0] = (args);
    LOG_PM("args: %d", args);
    MSG_QUEUE_SEND(ctx->priv.pm_suspend_thd.msg_id, (void *)&msg, 0);
}

static int is_pm_suspend_internal(void)
{
    return (thiz->priv.pm_state == ID_SYS_PM_SUSPEND) || (thiz->priv.is_enter_suspend == 1);
}

static int is_pm_suspend(void)
{
    int state = 0;
    MUTEX_LOCK_PM("start", thiz->priv.m_suspend_lock);
    state = is_pm_suspend_internal();
    MUTEX_UNLOCK_PM("end", thiz->priv.m_suspend_lock);
    LOG_PM("pm_state: %d/%d/%d", state, thiz->priv.pm_state, thiz->priv.is_enter_suspend);
    return state;
}

static int pm_check_wakelock(power_manager_context_t *ctx)
{
    int ret = 0;
    char del_caller[40] = {0};
    static int counter = 0;

    MUTEX_LOCK(ctx->priv.m_wake_lock);
    LIST_FOR_EACH(pos, &ctx->priv.wake_lock_list) {
        pm_wake_lock_node_t *node = CONTAINER_OF(pos, pm_wake_lock_node_t, list);
        if (node->timeout_s == 0) {
            LOG_PM("Caller(%s) Timeout", node->caller);
        } else if (node->timeout_s > 0) {
            --node->timeout_s;
            if (node->timeout_s == 0) {
                SEC_STRNCPY(del_caller, node->caller, sizeof(del_caller));
            }
        }
    }
    /// 检查是否有未超时的锁.
    LIST_FOR_EACH(pos, &ctx->priv.wake_lock_list) {
        pm_wake_lock_node_t *node = CONTAINER_OF(pos, pm_wake_lock_node_t, list);
        if (node->timeout_s > 0) {
            ret = 1;
            if (++counter % 5 == 0) {
                LOGW("Current has wakelock(%s), block suspend(%d).",
                    node->caller, node->timeout_s);
            }
            break;
        }
    }
    MUTEX_UNLOCK(ctx->priv.m_wake_lock);
    /// 删掉超时的节点.
    if (strlen(del_caller) > 0) {
        LOGW("LOG_PM timer out auto release: %s", del_caller);
        ctx->release_wakelock(del_caller);
    }

    return ret;
}

static void pm_suspend_check_handler(power_manager_context_t *ctx)
{
    static uint32_t log_interval = 0;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    log_interval++;
    if (wm && wm->is_on_window_dispatch(wm)) {
        LOG_PM("%s window dispatch, ignore.", wm->to_window_name(wm->get_current_window_id()));
        goto EXIT_LABEL;
    }

    if (pm_check_wakelock(ctx) && !ctx->priv.user_set_to_sleep) {
        LOG_PM("pm_check_wakelock, ignore");
        goto EXIT_LABEL;
    }

    uint32_t tmp = ctx->priv.suspend_counter;
#if IS_SUPPORT_POWER_OFF_REMINDER
    if (tmp > POWER_OFF_REMINDER_TIME) {
#else
    if (tmp > 0) {
#endif
        if (log_interval % 10 == 0) {
            LOG_PM("%ds after goto suspend, pm_state: %s current window: %s", 
                (tmp / 1000), print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress),
                wm->to_window_name(wm->get_current_window_id()));
        }
        uint16_t offset = get_timestamp() - ctx->priv.pm_check_time_ms;
        if (offset > PM_SUSPEND_WORK_PERIOD) {
            LOGE("time offset too large, offset: %d", offset);
            offset = PM_SUSPEND_WORK_PERIOD;
        }
        tmp = tmp > offset ? tmp - offset : 0;
        set_suspend_counter("suspend_timer", tmp, false);
    } else {
        if (thiz->hook && thiz->hook->is_prohibit_pm_suspend) {
            if (thiz->hook->is_prohibit_pm_suspend(thiz) == true) {
                set_suspend_counter("now goto suspend, but not allow, ignore", 
                    get_suspend_timeout(ctx), true);
                return;
            }
        }
        LOG_PM("Now goto suspend!"); /// 正常进入待机状态.
#if IS_SUPPORT_POWER_OFF_REMINDER
        set_suspend_counter("power_off_reminder", 0, true);
#endif
        if (!is_pm_suspend_internal()) {
            if (ctx->priv.pm_state_change_progress == PM_RESUME_FINISH) {
                dispatch_pm_event(ctx, ID_SYS_PM_SUSPEND);
            } else {
                LOG_PM("#ignore, pm progress: %s",
                    print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress));
            }
        } else {
            LOG_PM("ignore.");
        }
    }
EXIT_LABEL:    
    ctx->priv.pm_check_time_ms = get_timestamp();
}

static void stop_pm_suspend_timer(power_manager_context_t *ctx)
{
    LOG_PM("enter");
    EXCEPTION_CHECK_VOID(ctx);
    STOP_TIMER(ctx->priv.suspend_timer);
}

static void start_suspend_timer(power_manager_context_t *ctx)
{
    LOG_PM("enter");
    set_suspend_counter("start_suspend_timer", get_suspend_timeout(ctx), true);
    STOP_TIMER(ctx->priv.suspend_timer);
    START_TIMER(ctx->priv.suspend_timer, PM_SUSPEND_WORK_PERIOD);
    ctx->priv.pm_check_time_ms = get_timestamp();
}

static int pm_key_event_handler(void *user_data, uint16_t keycode, uint16_t keytype)
{
    power_manager_context_t *ctx = user_data;
    if (ctx->hook && ctx->hook->on_key_event_route) {
        ctx->hook->on_key_event_route(ctx, keycode, keytype);
    }
    
    if (ctx->priv.is_goto_sys_standby == true) {
        if (keytype == KEY_EVENT_UP || keytype == KEY_EVENT_DOWN) {
            ctx->priv.is_goto_sys_standby = false;
        }
        LOG_PM("is_goto_sys_standby, ignore");
        return 0;
    } 

    if (keytype != KEY_EVENT_DOWN) {
        if (ctx->priv.pm_state_change_progress == PM_WILL_POWER_OFF) {
            dispatch_pm_event(ctx, ID_SYS_PM_RESUME);
        } else {
            set_suspend_counter("non-wakeup-key-reset", get_suspend_timeout(ctx), false);
        }
    }
    return 0;
}

static void pm_goto_suspend_by_user(void)
{
    /// 禁止进入待机状态的钩子.
    if (thiz->hook && thiz->hook->is_prohibit_pm_suspend) {
        if (thiz->hook->is_prohibit_pm_suspend(thiz) == true) {
            set_suspend_counter("user request suspend, but not allow, ignore",
                 get_suspend_timeout(thiz), true);
            return;
        }
    }
    /// 正常进入待机状态.
    thiz->priv.user_set_to_sleep = true;
    set_suspend_counter("PM_SUSPEND_BY_USER", 0, true);
    clear_all_wakelock(thiz);
    pm_suspend_check_handler(thiz);
}

static void on_pm_broadcast_receive(broadcast_mask_id_t id,
            void *context, int reason, int arg2, broadcast_priv_data_t *pdata)
{
    power_manager_context_t *ctx = context;
    LOG_PM("id: 0x%lx, reason: %d, arg2: %d", id.code, reason, arg2);
    /// 钩子处理.
    if (ctx->hook && ctx->hook->on_broadcast_receive_route) {
        ctx->hook->on_broadcast_receive_route(id, context, reason, arg2, pdata);
    }

    switch (id.code) {
        case ID_BROADCAST_REQ_PM_SUSPEND:
            pm_goto_suspend_by_user();
            return;
        case ID_BROADCAST_SYS_STATE_CHANGE: 
            if (reason == SYSTEM_BROADCAST_POWER_ON || reason == SYSTEM_BROADCAST_POWER_OFF) {
                ctx->enable_pmu_sleep("SYS_POWER_STATE_CHANGE", false);
            } else if (reason == SYSTEM_BROADCAST_BOOT_COMPLETED) {
                start_suspend_timer(ctx);
            }
            return;
        case ID_BROADCAST_OTA_STATE_CHANGE: 
            if (reason == OTA_UPGRADE_STATE_START) {
                set_suspend_counter("ota start", get_suspend_timeout(ctx), true);
            }
            return;
        default:
            break;
    }
    /// 唤醒处理.
    if (reason == PM_WAKEUP_REASON_ENTER_NEW_SCREEN) {
        ctx->pm_wakeup((pm_wakeup_reason_e)reason);
        return;
    }
}

static void pm_period_wakeup_event_cb(const char *reason, wakeup_type_e wakeup_type)
{
    power_manager_context_t *ctx = thiz;
    LOG_PM("reason: %s, type: %d", reason, wakeup_type);
    switch (wakeup_type) {
        case WAKEUP_TYPE_POWER_KEY:
            break;
        case WAKEUP_TYPE_UNKNOWN:
            LOG_PM("WAKEUP_TYPE_UNKNOWN");
            ctx->pm_wakeup(PM_WAKEUP_REASON_IMU_IRQ);
            break;
        case WAKEUP_TYPE_IMU_INTR:
            LOG_PM("WAKEUP_TYPE_IMU_INTR");
            ctx->pm_wakeup(PM_WAKEUP_REASON_IMU_IRQ);
            break;
        case WAKEUP_TYPE_CHARGING:
            LOG_PM("WAKEUP_TYPE_CHARGING");
            ctx->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_IN);
            break;
        case WAKEUP_TYPE_TIMING:
            break;
        case WAKEUP_TYPE_BLE:
            break;
        default:
            break;
    }
}

static void set_period_wake_up(int ms)
{
    /// 注册定时唤醒接口.
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    pbus->add_period_wakeup_cb(LOG_TAG, ms, pm_period_wakeup_event_cb);
}

static void on_suspend_timer_cb(void *handle)
{
    pm_suspend_check_handler(thiz);
}

static void pm_sleep_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    multi_func_msg_data_t msg = {0};
    power_manager_context_t *ctx = CONTAINER_OF(thd, power_manager_context_t, priv.pm_suspend_thd);
    if (ctx->priv.wakeup_reason == 0) {
        ctx->priv.wakeup_reason = PM_WAKEUP_REASON_BOOT;
    }
    ctx->priv.pm_state_change_progress = PM_RESUME_FINISH; /// 初始化状态为恢复完成.
    CREATE_MSG_QUEUE(&thd->msg_id, PM_WORK_DISPATCH_MESQ_NUM);
    CREATE_TIMER(&ctx->priv.suspend_timer, "suspend_timer", ctx, OS_TIMER_PERIODIC, on_suspend_timer_cb);
    ctx->priv.pm_state = ID_SYS_PM_RESUME;
    start_suspend_timer(ctx);
    for (;;) {
        LOG_PM("==>thread sleep: %s", print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress));
        thd->is_running = 0;
        MSG_QUEUE_RECV(thd->msg_id, (void *)&msg, OS_WAIT_FOREVER);
        thd->is_running = 1;
        int state = msg.union_body.parms[0];
        LOG_PM("==>thread wakeup last: %s, state: %d", 
            print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress), state);
        switch (state) {
            default:
                LOG_PM("*******error pm_state: %d*******", ctx->priv.pm_state);
                break;
            case ID_SYS_PM_SUSPEND:
                MUTEX_LOCK_PM("PM_SUSPEND", ctx->priv.m_suspend_lock);
                if (ctx->hook && ctx->hook->pm_suspend_route) {
                    if (ctx->hook->pm_suspend_route(ctx, state) == 0) {
                        stop_pm_suspend_timer(ctx);
                        // ctx->priv.user_set_to_sleep = false;
                    }
                }
                MUTEX_UNLOCK_PM("PM_SUSPEND", ctx->priv.m_suspend_lock);
                break;
            case ID_SYS_PM_RESUME:
                MUTEX_LOCK_PM("PM_RESUME", ctx->priv.m_suspend_lock);  
                if (ctx->hook && ctx->hook->pm_resume_route) {
                    if (ctx->hook->pm_resume_route(ctx, state) == 0) {
                        start_suspend_timer(ctx);
                    }
                }
                MUTEX_UNLOCK_PM("PM_RESUME", ctx->priv.m_suspend_lock);
                break;
        }
    }
}

static int on_post_pm_init(void *ptr)
{
    power_manager_context_t *ctx = ptr;
    /// 默认打开pmu sleep.
    ctx->priv.pmu_sleep_enable = true;
    ctx->priv.user_set_to_sleep = false;
    /// 钩子处理.
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }
    /// 注册感兴趣的广播事件.
    ctx->priv.pm_bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.pm_bcd->context = ctx;
    ctx->priv.pm_bcd->name = "power_ctx";
    ctx->priv.pm_bcd->id.code |= ID_BROADCAST_REQ_PM_SUSPEND;
    ctx->priv.pm_bcd->id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    ctx->priv.pm_bcd->id.code |= ID_BROADCAST_OTA_STATE_CHANGE;
    ctx->priv.pm_bcd->receiver_cb = on_pm_broadcast_receive;
    register_broadcast_receiver(ctx->priv.pm_bcd);
    /// 注册按键唤醒.
    uint16_t keycode = KEY_CODE_ALL;
    uint16_t keytype = KEY_TYPE_ALL;
    key_input_context_t *input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    ADD_KEYEVENT_CB_WITH_SHIELD_LEVEL(input, "pm", ctx, keycode, keytype, pm_key_event_handler,
        KEY_HANDLER_SHIELD_FORBIDDEN);
    RUN_NEW_THREAD(&ctx->priv.pm_suspend_thd, "pm_suspend_thd",
        pm_sleep_thread, RT_PRIO_PM_SLEEP_THREAD, PM_SUSPEND_THREAD_STACKSIZE);
    return 0;
}

static void *list_del_all_cb(void *data, list_head_t *pos)
{
    pm_wake_lock_node_t *node = CONTAINER_OF(pos, pm_wake_lock_node_t, list);
    LOG_PM("clear: (%s)", node->caller);

    return node;
}

static void clear_all_wakelock(power_manager_context_t *ctx)
{
    list_head_t *head = &ctx->priv.wake_lock_list;

    LOG_PM("enter");
    MUTEX_LOCK(ctx->priv.m_wake_lock);
    LIST_EMPTY(head, list_del_all_cb);
    MUTEX_UNLOCK(ctx->priv.m_wake_lock);
}

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    const char *caller = (const char *)data;
    pm_wake_lock_node_t *node = CONTAINER_OF(pos, pm_wake_lock_node_t, list);

    if (strncmp(node->caller, caller, sizeof(node->caller) -1) == 0) {
        LOG_PM("remove(%s/%d) ok", node->caller, node->timeout_s);
        return node;
    }

    return NULL;
}

static int pm_acquire_wakelock(const char *caller, int timeout_s)
{
    power_manager_context_t *ctx = thiz;

    if (ctx->priv.user_set_to_sleep) {
        LOG_PM("user_set_to_sleep ignore, caller: %s", caller);
        return -1;
    }

    if (ctx->is_on_suspend_gotoing()) {
        LOG_PM("suspend gotoing ignore: %s", caller);
        return -1;
    }

    MUTEX_LOCK(ctx->priv.m_wake_lock);
    LIST_FOR_EACH(pos, &ctx->priv.wake_lock_list) {
        pm_wake_lock_node_t *node = CONTAINER_OF(pos, pm_wake_lock_node_t, list);
        if ((strncmp(node->caller, caller, sizeof(node->caller) -1) == 0) && (node->timeout_s == timeout_s)) {
            LOGW("LOG_PM dumplicate add, ignore(%s)", node->caller);
            MUTEX_UNLOCK(ctx->priv.m_wake_lock);
            return -1;
        }
    }

    pm_wake_lock_node_t *node = u_malloz(sizeof(pm_wake_lock_node_t));
    SEC_STRNCPY(node->caller, caller, sizeof(node->caller));
    node->timeout_s = (timeout_s == -1) ? MAX_LOCK_TIME_S : timeout_s;
    __list_add(&node->list, &ctx->priv.wake_lock_list);
    MUTEX_UNLOCK(ctx->priv.m_wake_lock);
    LOG_PM("NEW add(%s/%d)", node->caller, node->timeout_s);
    set_suspend_counter("pm_acquire_wakelock", get_suspend_timeout(thiz), true);
    return 0;
}

static int pm_release_wakelock(const char *caller)
{
    power_manager_context_t *ctx = thiz;
    int ret = -1;
    EXCEPTION_CHECK_INT(caller);
    MUTEX_LOCK(ctx->priv.m_wake_lock);
    ret = __list_del(&ctx->priv.wake_lock_list, (void *)caller, list_del_condition_cb);
    MUTEX_UNLOCK(ctx->priv.m_wake_lock);
    LOG_PM("caller: %s, ret: %d", caller, ret);
    return ret;
}

static int external_wakeup_policy(power_manager_context_t *ctx, pm_wakeup_reason_e reason)
{
    ctx->priv.is_back_ground_wakeup = false;
    switch (reason) {
        case PM_WAKEUP_REASON_IMU_IRQ:
#if IS_SUPPORT_AIRPLANE_MODE
            airplane_mode_context_t *airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
            if (airplane && airplane->is_on_airplane_mode()) {
                LOGW("blocked by ap mode!");
                return -1;
            }
#endif
            break;
        case PM_WAKEUP_REASON_BACKGROUND_TASK:
            ctx->priv.is_back_ground_wakeup = true;
            break;
        default:
            break;
    }
    return 0;
}

static const char *wake_up_reason_name(pm_wakeup_reason_e reason)
{
    switch (reason) {
        case PM_WAKEUP_REASON_BOOT:
            return "wakeup:BOOT";
        case PM_WAKEUP_REASON_KEY_POWER:
            return "wakeup:KEY-POWER-IRQ";
        case PM_WAKEUP_REASON_KEY_MODE:
            return "wakeup:KEY-MODE-IRQ";
        case PM_WAKEUP_REASON_ENTER_NEW_SCREEN:
            return "wakeup:enter_new_screen";
        case PM_WAKEUP_REASON_IMU_IRQ:
            return "wakeup:IMU-IRQ";
        case PM_WAKEUP_REASON_BLE_REQ:
            return "wakeup:BLE-IRQ";
        case PM_WAKEUP_REASON_OTA:
            return "wakeup:OTA-task";
        case PM_WAKEUP_REASON_CHARGE_PLUG_IN:
            return "wakeup:CHARGE_PLUG_IN";
        case PM_WAKEUP_REASON_CHARGE_PLUG_OUT:
            return "wakeup:CHARGE_PLUG_OUT";
        case PM_WAKEUP_REASON_BACKGROUND_TASK:
            return "wakeup:BACKGROUND_TASK";
        case PM_WAKEUP_REASON_FACTORY_RESUME:
            return "wakeup:FACTORY_RESUME";
        default:
            return "unknow reason";
    }
}

static void pm_wakeup(pm_wakeup_reason_e reason)
{
    const char *reason_str = wake_up_reason_name(reason);
    LOG_PM("pm_wakeup reason: %d(%s)", reason, reason_str);
    if (reason == PM_WAKEUP_REASON_ENTER_NEW_SCREEN) {
        if (thiz->priv.wakeup_reason == PM_WAKEUP_REASON_IOT) {
            if (is_pm_suspend_internal()) {
                if (thiz->priv.pm_state_change_progress == PM_SUSPEND_FINISH) {
                    LOG_PM("reason(%s) PM_RESUME_START", reason_str);
                    dispatch_pm_event(thiz, ID_SYS_PM_RESUME);
                }
            }
            return;
        }   
        battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
        if (battery && battery->is_vbus_plug_in() > 0) {
            LOG_PM("vbus plug in. ignore");
            return;
        }
    }

    if (external_wakeup_policy(thiz, reason) < 0) {
        LOG_PM("external_wakeup blocked");
        return;
    }

    if (thiz->priv.user_set_to_sleep) {
        LOG_PM("user_set_to_sleep blocked");
        return;
    }

    set_suspend_counter("pm_wakeup", get_suspend_timeout(thiz), true);
    if (reason == PM_WAKEUP_REASON_CHARGE_PLUG_OUT) {
        LOG_PM("CHARGE_PLUG_OUT blocked");
        return;
    }

    if (is_pm_suspend_internal()) {
        if (thiz->priv.pm_state_change_progress == PM_SUSPEND_FINISH) {
            LOG_PM("reason(%s) PM_RESUME_START", reason_str);
            thiz->priv.wakeup_reason = reason;
            dispatch_pm_event(thiz, ID_SYS_PM_RESUME);
        } else {
            LOG_PM("ignore->reason(%s), pm progress: %d, pm_state: %d, " \
                "is_enter_suspend: %d, suspend_counter: %d, pmu_sleep_enable: %d, " \
                "user_set_to_sleep: %d, is_back_ground_wakeup: %d,", 
                reason_str, 
                thiz->priv.pm_state_change_progress, 
                thiz->priv.pm_state, 
                thiz->priv.is_enter_suspend, 
                thiz->priv.suspend_counter, 
                thiz->priv.pmu_sleep_enable,
                thiz->priv.user_set_to_sleep,
                thiz->priv.is_back_ground_wakeup
                );
        }
    } else {
        if (thiz->priv.wakeup_reason == PM_WAKEUP_REASON_IOT && reason != PM_WAKEUP_REASON_IOT) {
            // LOG_PM("IOT唤醒状态下点亮背光 reason: %s", reason_str);
            thiz->priv.wakeup_reason = reason;
        } else {
            LOG_PM("pm_suspend: pm_state: %d, is_enter_suspend: %d", thiz->priv.pm_state, thiz->priv.is_enter_suspend);
        }
    }
}

static int enable_pmu_sleep(const char *caller, bool enable)
{
    if (thiz->priv.pmu_sleep_enable != enable) {
        thiz->priv.pmu_sleep_enable = enable;
        LOG_PM("enable:%d, caller(%s)", enable, caller);
    }

    return 0;
}

static int is_pmu_sleep(void)
{
#if IS_SUPPORT_AIRPLANE_MODE
    if (!thiz->priv.pmu_sleep_enable) {
        airplane_mode_context_t *airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
        if (airplane && airplane->is_on_airplane_mode()) {
            enable_pmu_sleep("self", true);
            return 1;
        }
    }
#endif
    return thiz->priv.pmu_sleep_enable;
}

static pm_wakeup_reason_e pm_get_wakeup_reason(void)
{
    return thiz->priv.wakeup_reason;
}

static bool is_background_wakeup(void)
{
    return thiz->priv.is_back_ground_wakeup;
}

static bool is_on_suspend_gotoing(void)
{
    return (thiz->priv.pm_state_change_progress == PM_SUSPEND_GOING);
}

static bool is_pm_idle(void)
{
    return (thiz->priv.pm_state == ID_SYS_PM_IDLE);
}

static bool is_system_power_on(void)
{
    return thiz->priv.is_system_power_on;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_pm_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, power_manager_context_t);
    ptr->is_pm_suspend = is_pm_suspend;
    ptr->acquire_wakelock = pm_acquire_wakelock;
    ptr->release_wakelock = pm_release_wakelock;
    ptr->pm_wakeup = pm_wakeup;
    ptr->enable_pmu_sleep = enable_pmu_sleep;
    ptr->is_pmu_sleep = is_pmu_sleep;
    ptr->set_period_wake_up = set_period_wake_up;
    ptr->pm_get_wakeup_reason = pm_get_wakeup_reason;
    ptr->print_wakeup_state = print_wakeup_state;
    ptr->is_on_suspend_gotoing = is_on_suspend_gotoing;
    ptr->is_background_wakeup = is_background_wakeup;
    ptr->is_system_power_on = is_system_power_on;
    ptr->is_pm_idle = is_pm_idle;
    MUTEX_INIT(&ptr->priv.m_wake_lock, mutex_caller_wakelock);
    MUTEX_INIT(&ptr->priv.m_suspend_lock, mutex_suspend_lock);
    USE_INIT_LIST_HEAD(&ptr->priv.wake_lock_list);
    thiz = ptr;

    return ADD_MODULE("pm", ID_MODULE_POWER, 0, on_post_pm_init, 
        ptr, list_head, sizeof(power_manager_context_t));
}
