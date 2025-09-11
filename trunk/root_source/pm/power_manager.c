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

static void set_suspend_counter(const char *tag, int counter)
{
    thiz->priv.suspend_counter = counter;
    LOG_PM("tag:%s New counter: %d", tag, counter);
}

static const char *print_wakeup_state(pm_wakeup_state_e state)
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
        default:
            return "error state";
    }
}

static void set_pm_state_progress(const char *tag, int state, int t0)
{
    int tmp = state;

    if (thiz->priv.pm_state_change_progress != tmp) {
        thiz->priv.pm_state_change_progress = tmp;
        const char *s = print_wakeup_state((pm_wakeup_state_e)tmp);
        if (t0 > 0) {
            LOG_PM("(%s)New pm_state: %s, time escaped: %d ms", tag, s, t0);
        } else {
            LOG_PM("(%s)New pm_state: %s", tag, s);
        }
    }
}

static int get_suspend_timeout(power_manager_context_t *thiz)
{
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_INT(misc);
    int timeout = PM_SUSPEND_TIME_S;
    if (misc && misc->is_empty_battery()) {
        timeout = PM_SUSPEND_LOW_BAT_TIME_S;
    }
    LOG_PM("suspend_timeout:(%d)s", timeout);
    return timeout;
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

    LIST_FOR_EACH(pos, &ctx->priv.wake_lock_list) {
        pm_wake_lock_node_t *node = CONTAINER_OF(pos, pm_wake_lock_node_t, list);
        if (node->timeout_s > 0) {
            ret = 1;
            if (++counter % 5 == 0) {
                LOGW("LOG_PM Current has wakelock(%s), block suspend(%d).",
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
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);

    if (wm && wm->is_on_window_dispatch()) {
        LOG_PM("on_window_dispatch, ignore");
        return;
    }

    if (pm_check_wakelock(ctx) && !ctx->priv.user_set_to_sleep) {
        LOG_PM("pm_check_wakelock, ignore");
        return;
    }

    int tmp = ctx->priv.suspend_counter;
    if (tmp > 0) {
        LOG_PM("I will goto suspend after: %ds pm_state_change_progress: %s", 
            tmp, print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress));
        set_suspend_counter("suspend_timer", (--tmp));
    } else {
        ota_manager_context_t *ota_ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
        if (ota_ctx && ota_ctx->is_ota_upgrading()) {
            set_suspend_counter("ota_is_running", get_suspend_timeout(ctx));
            return;
        }
        LOG_PM("Now goto suspend!");
        if (!is_pm_suspend_internal()) {
            if (ctx->priv.pm_state_change_progress == PM_RESUME_FINISH) {
                dispatch_pm_event(ctx, ID_SYS_PM_SUSPEND);
            } else {
                LOG_PM("#ignore:pm_state_change_progress: %s",
                    print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress));
            }
        } else {
            LOG_PM("ignore.");
        }
    }
}

static void start_suspend_timer(power_manager_context_t *ctx)
{
    LOG_PM("enter");
    set_suspend_counter("start_suspend_timer", get_suspend_timeout(ctx));
    STOP_TIMER(ctx->priv.suspend_timer);
    START_TIMER(ctx->priv.suspend_timer, PM_SUSPEND_WORK_PERIOD);
}

static void stop_suspend_timer(power_manager_context_t *ctx)
{
    LOG_PM("enter");
    STOP_TIMER(ctx->priv.suspend_timer);
}

static int pm_suspend_internal(power_manager_context_t *ctx)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *pmu_dev = pbus->find_device(ID_DEVICE_PMU);
    uint32_t t0 = get_timestamp();

    LOG_PM("=> START: %ld", t0);
    pbus->suspend();
    DEVICE_IOCTL_NO_RET(pmu_dev, OP_CODE_PMU_SET_SLEEP_MODE);
    LOG_PM("=> END: %ld", get_timestamp() - t0);

    return 0;
}

static int pm_resume_internal(power_manager_context_t *ctx)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *pmu_dev = pbus->find_device(ID_DEVICE_PMU);
    uint32_t t0 = get_timestamp();

    LOG_PM("=> START: %ld", t0);
    pbus->resume();
    DEVICE_IOCTL_NO_RET(pmu_dev, OP_CODE_PMU_SET_ACTIVE_MODE);
    LOG_PM("=> END: %ld", get_timestamp() - t0);

    return 0;
}

static int pm_key_event_handler(void *user_data, uint16_t keycode, uint16_t keytype)
{
    power_manager_context_t *ctx = user_data;

    // LOG_PM("SHORT==>keyevent(%d,%d) event.", keycode, keytype);
    if (keycode == KEY_CODE_POWER) {
        ctx->pm_wakeup(PM_WAKEUP_REASON_KEY_POWER);
    } else if (keycode == KEY_CODE_MODE) {
        ctx->pm_wakeup(PM_WAKEUP_REASON_KEY_MODE);
    } else {
        set_suspend_counter("non-wakeup-key-reset", get_suspend_timeout(ctx));
        LOG_PM("not wakeup-key, only reset");
    }
    return 0;
}

/// 监听外部唤醒源输入广播.
static void on_pm_broadcast_receive(broadcast_mask_id_t id,
            void *context, int reason, int arg2, broadcast_priv_data_t *pdata)
{
    power_manager_context_t *ctx = context;

    LOG_PM("id: 0x%lx, reason: %d, arg2: %d", id.code, reason, arg2);

    if (id.code == ID_BROADCAST_REQ_PM_SUSPEND_BY_USER) {
        LOG_PM("==>PM_SUSPEND_BY_USER#");
        if (arg2 == SUSPEND_BY_BLE) {
            #if IS_SUPPORT_DEEP_SLEEP_FUNCTION
            dispatch_pmu_sleep_event(ctx);
            #endif
        } else {
            ctx->priv.user_set_to_sleep = true;
            set_suspend_counter("PM_SUSPEND_BY_USER", 0);
            clear_all_wakelock(ctx);
            pm_suspend_check_handler(ctx);
        }
        return;
    }

    if (id.code == ID_BROADCAST_SYS_SHUTDOWN) {
        LOG_PM("==>ID_BROADCAST_SYS_SHUTDOWN");
        ctx->priv.is_shutdown = 1;
        ctx->enable_pmu_sleep("SYS_SHUTDOWN", false);
        return;
    }

    if (id.code == ID_BROADCAST_SYS_BOOT_COMPLETED) {
        LOG_PM("==>ID_BROADCAST_SYS_BOOT_COMPLETED");
        start_suspend_timer(ctx);
        return;
    }

    switch (reason) {
        case PM_WAKEUP_REASON_ENTER_NEW_SCREEN:
            ctx->pm_wakeup((pm_wakeup_reason_e)reason);
            break;
    }
}

/// 待机前处理, 保证所有广播事件处理完成后再进入dummy.
static void pm_suspend_prepare_enter(power_manager_context_t *ctx)
{
    int counter = 0;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    SEND_BROADCAST_SYNC_ARGS1(ID_BROADCAST_PM_STATE_CHANGE, ctx->priv.pm_state);
    /// 当成前台跳转,确保任何界面都可以允许进入休眠.
    if (wm && wm->dispatch_new_window_args) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_DUMMY;
        wm->dispatch_new_window_args(wm, &msg);
    }
    
    do {
        uint8_t current_window_id = wm->get_current_window_id();
        LOG_PM("wait(current: %s) enter --> ID_WINDOW_DUMMY(%d) finish.", wm->to_window_name(current_window_id), counter);
        if (wm->is_current_window(ID_WINDOW_DUMMY)) {
            break;
        }
        msleep(100);
    } while (++counter < 50);
}

/// 唤醒后处理.
static void pm_resume_post_enter(power_manager_context_t *ctx)
{
    int counter = 0;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    LOG_PM("pm_state: %d, wakeup_reason: %d", ctx->priv.pm_state, ctx->priv.wakeup_reason);
    SEND_BROADCAST_ARGS2(ID_BROADCAST_PM_STATE_CHANGE, ctx->priv.pm_state, ctx->priv.wakeup_reason);

    do {
        LOG_PM("wait exit --> ID_WINDOW_DUMMY(%d) finish.", counter);
        if (!wm->is_current_window(ID_WINDOW_DUMMY)) {
            break;
        }
        msleep(100);
    } while (++counter < 50);
}

/// pmu定时wakeup回调.
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
    uint32_t t0 = 0;

    power_manager_context_t *ctx = CONTAINER_OF(thd, power_manager_context_t, priv.pm_suspend_thd);
    if (ctx->priv.wakeup_reason == 0) {
        ctx->priv.wakeup_reason = PM_WAKEUP_REASON_BOOT;
    }

    set_pm_state_progress("init", PM_RESUME_FINISH, 0);
    CREATE_MSG_QUEUE(&thd->msg_id, PM_WORK_DISPATCH_MESQ_NUM);
    ota_manager_context_t *ota_ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);

    /// 初始化定时器.
    CREATE_TIMER(&ctx->priv.suspend_timer, "suspend_timer", ctx, OS_TIMER_PERIODIC, on_suspend_timer_cb);
    start_suspend_timer(ctx);

    for (;;) {
        LOG_PM("==>thread sleep: %s", print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress));
        thd->is_running = 0;
        MSG_QUEUE_RECV(thd->msg_id, (void *)&msg, OS_WAIT_FOREVER);
        thd->is_running = 1;

        int state = msg.union_body.parms[0];
        LOG_PM("==>thread wakeup last: %s, state: %d", print_wakeup_state((pm_wakeup_state_e)ctx->priv.pm_state_change_progress), state);

        switch (state) {
            default:
                LOG_PM("*******error pm_state: %d*******", ctx->priv.pm_state);
                break;

            case ID_SYS_PM_SUSPEND:
                MUTEX_LOCK_PM("PM_SUSPEND", ctx->priv.m_suspend_lock);
                if (!(ota_ctx && ota_ctx->is_ota_upgrading())) {
                    if (ctx->priv.pm_state_change_progress == PM_RESUME_FINISH) {
                        t0 = get_timestamp();
                        LOG_PM("==>ID_SYS_PM_SUSPEND");
                        set_pm_state_progress(__func__, PM_SUSPEND_GOING, 0);
                        ctx->priv.pm_state = state;
                        ctx->priv.is_enter_suspend = 1;
                        stop_suspend_timer(ctx);
                        pm_suspend_prepare_enter(ctx);
                        pm_suspend_internal(ctx);
                        ctx->priv.user_set_to_sleep = false;
                        set_pm_state_progress(__func__, PM_SUSPEND_FINISH, (get_timestamp() - t0));
                        #if IS_SUPPORT_DEEP_SLEEP_FUNCTION
                        dispatch_pmu_sleep_event(ctx);
                        #endif
                    } else {
                        LOG_PM("ID_SYS_PM_SUSPEND: err state ignore");
                    }
                } else {
                    LOGE("is on ota_upgrade, err state.");
                }
                MUTEX_UNLOCK_PM("PM_SUSPEND", ctx->priv.m_suspend_lock);
                break;

            case ID_SYS_PM_RESUME:
                MUTEX_LOCK_PM("PM_RESUME", ctx->priv.m_suspend_lock);
                if (ctx->priv.pm_state_change_progress == PM_SUSPEND_FINISH) {
                    t0 = get_timestamp();
                    LOG_PM("==>ID_SYS_PM_RESUME");
                    set_pm_state_progress(__func__, PM_RESUME_GOING, 0);
                    REMOVE_TASK_HANDLER_DIRECTLY("period_wakeup_goto_sleep");
                    pm_resume_internal(ctx);
                    start_suspend_timer(ctx);
                    ctx->priv.pm_state = state;
                    pm_resume_post_enter(ctx);
                    ctx->priv.is_enter_suspend = 0;
                    set_pm_state_progress(__func__, PM_RESUME_FINISH, (get_timestamp() - t0));
                } else {
                    LOG_PM("ID_SYS_PM_RESUME: err state ignore");
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

    /// 注册感兴趣的广播事件.
    ctx->priv.pm_bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.pm_bcd->context = ctx;
    ctx->priv.pm_bcd->name = "power_ctx";
    ctx->priv.pm_bcd->id.code |= ID_BROADCAST_PM_WAKEUP_BY_USER;
    ctx->priv.pm_bcd->id.code |= ID_BROADCAST_REQ_PM_SUSPEND_BY_USER;
    ctx->priv.pm_bcd->id.code |= ID_BROADCAST_SYS_SHUTDOWN;
    ctx->priv.pm_bcd->id.code |= ID_BROADCAST_SYS_BOOT_COMPLETED;
    ctx->priv.pm_bcd->receiver_cb = on_pm_broadcast_receive;
    register_broadcast_receiver(ctx->priv.pm_bcd);

    /// 注册按键唤醒.
    uint16_t keycode = KEY_CODE_POWER | KEY_CODE_MODE;
    uint16_t keytype = KEY_EVENT_CLICK;
    key_input_context_t *input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    keytype |= KEY_EVENT_DOWN;
    keytype |= KEY_EVENT_UP;
    keytype |= KEY_EVENT_LONG_2S;
    keytype |= KEY_EVENT_LONG_5S;
    keytype |= KEY_EVENT_LONG_9S;
    input->add_keyevent_handler(ctx, keycode, keytype,  pm_key_event_handler);

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
            LOGE("LOG_PM dumplicate add, ignore(%s)", node->caller);
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

    /// 复位计数器.
    set_suspend_counter("pm_acquire_wakelock", get_suspend_timeout(thiz));

    return 0;
}

static int pm_release_wakelock(const char *caller)
{
    power_manager_context_t *ctx = thiz;
    int ret = -1;

    if (!caller) {
        LOG_PM("invalid caller");
        return ret;
    }

    MUTEX_LOCK(ctx->priv.m_wake_lock);
    ret = __list_del(&ctx->priv.wake_lock_list, (void *)caller, list_del_condition_cb);
    MUTEX_UNLOCK(ctx->priv.m_wake_lock);
    LOG_PM("caller: %s, ret: %d", caller, ret);

    return ret;
}

static int external_wakeup_policy(power_manager_context_t *ctx, pm_wakeup_reason_e reason)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AP_MODE);

    ctx->priv.is_back_ground_wakeup = false;

    switch (reason) {
        case PM_WAKEUP_REASON_IMU_IRQ:
            if (airplane && airplane->is_on_airplane_mode()) {
                LOGW("blocked by ap mode!");
                return -1;
            }
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
        case PM_WAKEUP_REASON_CHARGE_PLUG_OUI:
            return "wakeup:CHARGE_PLUG_OUI";
        case PM_WAKEUP_REASON_BACKGROUND_TASK:
            return "wakeup:BACKGROUND_TASK";
        default:
            return "unknow reason";
    }
}

static void pm_wakeup(pm_wakeup_reason_e reason)
{
    const char *reason_str = wake_up_reason_name(reason);
    misc_manager_context_t *misc_ctx = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
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
        } else if (misc_ctx->is_charge_plug_in()) {
            LOG_PM("charging. ignore");
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

    set_suspend_counter("pm_wakeup", get_suspend_timeout(thiz));
    if (reason == PM_WAKEUP_REASON_CHARGE_PLUG_OUI) {
        LOG_PM("CHARGE_PLUG_OUI blocked");
        return;
    }

    if (is_pm_suspend_internal()) {
        if (thiz->priv.pm_state_change_progress == PM_SUSPEND_FINISH) {
            LOG_PM("reason(%s) PM_RESUME_START", reason_str);
            thiz->priv.wakeup_reason = reason;
            dispatch_pm_event(thiz, ID_SYS_PM_RESUME);
        } else {
            LOG_PM("ignore->reason(%s), pm_state_change_progress: %d, pm_state: %d, " \
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
            LOG_PM("IOT唤醒状态下点亮背光 reason: %s", reason_str);
            SEND_BROADCAST_ARGS0(ID_BROADCAST_SET_BLACKLIGHT_BY_USER);
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
    if (!thiz->priv.pmu_sleep_enable) {
        airplane_mode_context_t *airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AP_MODE);
        if (airplane && airplane->is_on_airplane_mode()) {
            enable_pmu_sleep("self", true);
            return 1;
        }
    }
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

static bool is_on_is_shutdown(void)
{
    return thiz->priv.is_shutdown == 1;
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
    ptr->is_background_wakeup = is_background_wakeup;
    ptr->is_on_suspend_gotoing = is_on_suspend_gotoing;
    ptr->is_on_is_shutdown = is_on_is_shutdown;
    ptr->pm_get_wakeup_reason = pm_get_wakeup_reason;

    MUTEX_INIT(&ptr->priv.m_wake_lock, mutex_caller_wakelock);
    MUTEX_INIT(&ptr->priv.m_suspend_lock, mutex_suspend_lock);
    INIT_LIST_HEAD(&ptr->priv.wake_lock_list);
    thiz = ptr;

    return ADD_MODULE("pm", ID_MODULE_POWER, 0, on_post_pm_init, 
        ptr, list_head, sizeof(power_manager_context_t));
}
