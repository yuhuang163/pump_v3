/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "input_key_code.h"
#include "power_manager.h" // IWYU pragma: keep
#include "power_hook.h"  // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_ON_BOOT_RUN_TIME_MS                   600    /// boot 启动补偿, 冷启动和热启动时间不一致
#define POWER_ON_TIMEOUT_MS_DEFAULT                 (3000 - POWER_ON_BOOT_RUN_TIME_MS)     /// 电源按键超时时间，单位ms
#define POWER_ON_TIMEOUT_MS                         (POWER_ON_TIMEOUT_MS_DEFAULT - get_timestamp_ms(0))
#define POWER_ONOFF_AND_STANDBY_TASK_NAME           "power_onoff_and_standby"


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
static bool is_prohibit_pm_suspend(struct power_manager_context *ctx)
{
    EXCEPTION_CHECK_BOOL(ctx);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_BOOL(wm);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    if (battery && BATTERY_IS_CHARGING(battery->get_charge_status())) {
        if (wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0) {
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
            wm->dispatch_new_window_args(wm, &msg);
        }
        return true; /// 充电状态下禁止进入待机状态.
    }
    return false;
}

static void system_power_event_cb(hdr_post_args_t *args)
{
    int action = args->arg1;
    power_manager_context_t *pm = args->user_data;
    EXCEPTION_CHECK_VOID(pm);
    switch (action) {
        case HDR_ACTION_POWER_ON_DETECT:
            LOG_PM("%s action:%d", args->name, action);
            key_input_context_t *key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
            battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
            pm->priv.is_power_on_timeout_detect = 0;
            if (key && key->is_key_triggered(KEY_INDEX_POWER) == 0) {
#if IS_SUPPORT_PWR_ON_WHEN_PLUG_IN
                if (battery && battery->is_vbus_plug_in() > 0) {
                    pm->system_power_control(true);
                    SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
                } else {
                    pm->system_power_control(false); /// 非充电状态下进行开机超时检测.
                }
#else
                pm->system_power_control(false); 
#endif
            } else {
                pm->system_power_control(true);
                SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
            }
            break;
        case HDR_ACTION_POWER_ON:
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.is_power_on_timeout_detect = 0;
            pm->system_power_control(true);
            SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
            break;
        case HDR_ACTION_POWER_ON_INVALID: 
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.is_power_on_timeout_detect = 0;
            pm->system_power_control(false);
            break;
        case HDR_ACTION_POWER_OFF: 
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.is_power_on_timeout_detect = 0;
            pm->priv.allow_shutdown_or_standby = 0;
            pm->system_power_control(false);
            break;
        case HDR_ACTION_SYS_STANDBY: 
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.is_goto_sys_standby = true;
            SEND_BROADCAST_ARGS1(ID_BROADCAST_REQ_PM_SUSPEND, PM_BROADCAST_USER_REQUEST_SUSPEND);
            break;
        default:
            LOG_PM("%s action:%d, ignore", args->name, action);
            break;
    }
}

static int on_key_event_route(struct power_manager_context *ctx, uint16_t keycode, uint16_t keytype)
{
    EXCEPTION_CHECK_INT(ctx);
    if (keycode != KEY_CODE_POWER) {
        return 0;
    }

    /// 待机状态下长按3s退出待机状态.
    if (ctx->priv.is_enter_suspend == true  && keytype == KEY_EVENT_LONG_3S) {
        ctx->pm_wakeup(PM_WAKEUP_REASON_KEY_POWER); /// 唤醒电源管理器.
        LOG_PM("wakeup by key power");
        return 0;
    }

    /// 开机按键超时检测, 如果超时, 则进入关机状态.
    if (keytype == KEY_EVENT_UP && ctx->priv.is_power_on_timeout_detect == 1) {
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)  
        battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
        if (battery && battery->is_vbus_plug_in() > 0) {
            LOG_PM("vbus plug in. ignore");  /// 插电状态下禁止禁止关机
            return 0;
        }
#endif
        POST_TASK_HANDLER_NO_DELAY_ARG1(system_power_event_cb, ctx, HDR_ACTION_POWER_ON_INVALID,
            POWER_ONOFF_AND_STANDBY_TASK_NAME);
        return 0;
    }

    /// 开机检测窗口内收到3s长按, 立即确认开机成功, 避免后续抬键触发invalid关机.
    if (keytype == KEY_EVENT_LONG_3S && ctx->priv.is_power_on_timeout_detect == 1) {
        ctx->priv.is_power_on_timeout_detect = 0;
        if (!ctx->is_system_power_on()) {
            POST_TASK_HANDLER_NO_DELAY_ARG1(system_power_event_cb, ctx, HDR_ACTION_POWER_ON,
                POWER_ONOFF_AND_STANDBY_TASK_NAME);
        }
        return 0;
    }

    /// 允许关机/待机标志设置. 
    if (keytype == KEY_EVENT_DOWN && ctx->priv.allow_shutdown_or_standby == 0 && 
        ctx->is_system_power_on() > 0 && get_timestamp() > 4000) {
        ctx->priv.allow_shutdown_or_standby = 1; /// 允许关机/待机标志.
        LOG_PM("key down on power-on state, allow shutdown or standby: %d", ctx->priv.allow_shutdown_or_standby);
        return 0;
    }

    /// 进入关机状态. 不在工厂重置界面时才允许进入关机状态.(按键冲突)
    if (keytype == KEY_EVENT_LONG_3S && ctx->priv.allow_shutdown_or_standby == 1) {
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)  
        battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
        if (battery && BATTERY_IS_CHARGING(battery->get_charge_status())) {
            if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0) {
                multi_func_msg_data_t msg = {0};
                msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
                wm->dispatch_new_window_args(wm, &msg);
            }
            return 0;
        }
#endif
        /// 不在工厂重置界面时才允许进入关机状态.(按键冲突)
        if (wm && wm->is_current_window(wm, ID_WINDOW_FACTORY_RESET) == 0) {
#if IS_SUPPORT_SLEEP_POWER_OFF
            POST_TASK_HANDLER_NO_DELAY_ARG1(system_power_event_cb, ctx, HDR_ACTION_POWER_OFF,
                POWER_ONOFF_AND_STANDBY_TASK_NAME);
#else
            POST_TASK_HANDLER_NO_DELAY_ARG1(system_power_event_cb, ctx, HDR_ACTION_SYS_STANDBY,
                POWER_ONOFF_AND_STANDBY_TASK_NAME);
#endif
        } else {
            LOG_PM("in factory reset window, ignore");
        }
    } else {
        LOG_PM("key:%d, ignore", keytype);
    }
    return 0;
}

#if (IS_SUPPORT_SLEEP_POWER_OFF == 0)
static int get_pm_broadcast_id(int state)
{
    switch (state) {
        case ID_SYS_PM_RESUME:
            return PM_BROADCAST_SYS_RESUME;
        case ID_SYS_PM_IDLE:
            return PM_BROADCAST_SYS_IDLE;
        case ID_SYS_PM_SUSPEND:
            return PM_BROADCAST_SYS_SUSPEND;
        default:
            return PM_BROADCAST_SYS_IDLE;
    }
}
static void pm_suspend_prepare_enter(power_manager_context_t *ctx)
{
    int counter = 0;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    /// 当成前台跳转,确保任何界面都可以允许进入休眠.
    if (wm && wm->dispatch_new_window_args) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_DUMMY;
        wm->dispatch_new_window_args(wm, &msg);
    }
    
    do {
        uint8_t current_window_id = wm->get_current_window_id();
        LOG_PM("wait(current: %s) enter --> ID_WINDOW_DUMMY(%d) finish.", wm->to_window_name(current_window_id), counter);
        if (wm->is_current_window(wm, ID_WINDOW_DUMMY)) {
            break;
        }
        msleep(100);
    } while (++counter < 50);
    SEND_BROADCAST_SYNC_ARGS1(ID_BROADCAST_PM_STATE_CHANGE, get_pm_broadcast_id(ctx->priv.pm_state));
}

static void set_pm_state_progress(power_manager_context_t *ctx, const char *tag, int state, int t0)
{
    int tmp = state;

    if (ctx->priv.pm_state_change_progress != tmp) {
        ctx->priv.pm_state_change_progress = tmp;
        const char *s = ctx->print_wakeup_state(tmp);
        if (t0 > 0) {
            LOG_PM("New pm_state: %s, time escaped: %d ms", tag, s, t0);
        } else {
            LOG_PM("New pm_state: %s", tag, s);
        }
    }
}

static int pm_suspend_internal(power_manager_context_t *ctx)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *pmu_dev = pbus->find_device(ID_DEVICE_PMU);
    uint32_t t0 = get_timestamp();
    LOG_PM("=> START: %ld", t0);
    pbus->suspend();
    DEVICE_IOCTL_NO_RET(pmu_dev, OP_CODE_PMU_SET_SLEEP_MODE);
    LOG_PM("=> END: %ld", get_timestamp_ms(t0));
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
    LOG_PM("=> END: %ld", get_timestamp_ms(t0));

    return 0;
}

static void pm_resume_post_enter(power_manager_context_t *ctx)
{
    int counter = 0;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    LOG_PM("pm_state: %d, wakeup_reason: %d", ctx->priv.pm_state, ctx->priv.wakeup_reason);
    SEND_BROADCAST_ARGS2(ID_BROADCAST_PM_STATE_CHANGE, 
        get_pm_broadcast_id(ctx->priv.pm_state), ctx->priv.wakeup_reason);

    do {
        LOG_PM("wait exit --> ID_WINDOW_DUMMY(%d) finish.", counter);
        if (!wm->is_current_window(wm, ID_WINDOW_DUMMY)) {
            break;
        }
        msleep(100);
    } while (++counter < 50);
}
#endif

static int on_pm_suspend_route(struct power_manager_context *ctx, int state)
{
    EXCEPTION_CHECK_INT(ctx);
#if IS_SUPPORT_SLEEP_POWER_OFF
    ota_manager_context_t *ota_ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    if ((ota_ctx && ota_ctx->is_ota_upgrading())) {
        LOG_PM("is on ota_upgrade, err state.");
        return -1;
    }
    LOG_PM("==> pm suspend");
#if IS_SUPPORT_POWER_OFF_REMINDER
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(wm);
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_POPUP_WINDOW;
    msg.union_body.extra.is_bg_dispatch = 0x01;
    msg.union_body.extra.arg1 = ID_POPUP_UI_WILL_POWER_OFF;
    msg.union_body.extra.popup_exit_time = 3000;
    msg.union_body.extra.user_data = ctx;
    msg.union_body.extra.forbid_short_press_exit_popup = true;
    wm->dispatch_new_window_args(wm, &msg); 
#else
    ctx->priv.allow_shutdown_or_standby = 0;
    ctx->system_power_control(false);
#endif
    return 0;
#else
    uint32_t t0 = 0;
    ota_manager_context_t *ota_ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    if ((ota_ctx && ota_ctx->is_ota_upgrading())) {
        LOG_PM("is on ota_upgrade, err state.");
        return -1;
    }

    if (ctx->priv.pm_state_change_progress == PM_RESUME_FINISH) {
        t0 = get_timestamp();
        LOG_PM("==> pm suspend");
        set_pm_state_progress(ctx, __func__, PM_SUSPEND_GOING, 0);
        ctx->priv.pm_state = state;
        ctx->priv.is_enter_suspend = 1;
        pm_suspend_prepare_enter(ctx);
        pm_suspend_internal(ctx);
        ctx->priv.user_set_to_sleep = false;
        set_pm_state_progress(ctx, __func__, PM_SUSPEND_FINISH, get_timestamp_ms(t0));
        return 0;
    } else {
        LOG_PM("is not resume finish, err state ignore");
        return -1;
    }
#endif
}

static int on_pm_resume_route(struct power_manager_context *ctx, int state)
{
    EXCEPTION_CHECK_INT(ctx);
#if IS_SUPPORT_SLEEP_POWER_OFF
    LOG_PM("==> pm resume");
    return 0;
#else
    uint32_t t0 = 0;
    if (ctx->priv.pm_state_change_progress == PM_SUSPEND_FINISH) {
        t0 = get_timestamp();
        LOG_PM("==> pm resume");
        set_pm_state_progress(ctx, __func__, PM_RESUME_GOING, 0);
        REMOVE_TASK_HANDLER_DIRECTLY("period_wakeup_goto_sleep");
        pm_resume_internal(ctx);
        ctx->priv.pm_state = state;
        pm_resume_post_enter(ctx);
        ctx->priv.is_enter_suspend = 0;
        set_pm_state_progress(ctx, __func__, PM_RESUME_FINISH, (get_timestamp() - t0));
        return 0;
    } else {
        LOG_PM("is not suspend finish, err state ignore");
        return -1;
    }
#endif
}

static void set_suspend_counter_route(power_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    if (ctx->priv.suspend_counter < PM_IDLE_TIME_S && ctx->priv.pm_state == ID_SYS_PM_RESUME) {
        ctx->priv.pm_state = ID_SYS_PM_IDLE;
        LOG_PM("==> pm idle enter");
        SEND_BROADCAST_ARGS1(ID_BROADCAST_PM_STATE_CHANGE, PM_BROADCAST_SYS_IDLE);
    } else if (ctx->priv.suspend_counter > PM_IDLE_TIME_S && ctx->priv.pm_state == ID_SYS_PM_IDLE) {
        ctx->priv.pm_state = ID_SYS_PM_RESUME;
        LOG_PM("==> pm idle release");
        SEND_BROADCAST_ARGS1(ID_BROADCAST_PM_STATE_CHANGE, PM_BROADCAST_SYS_IDLE_RELEASE);
    }
}

static int on_power_broadcast_receive_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    power_manager_context_t *ctx = context;
    EXCEPTION_CHECK_INT(ctx);
    switch (id.code) {
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                LOG_PM("--> system power off"); /// 1000ms 后关机.
                POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, 1000, HDR_ACTION_POWER_OFF,
                    ctx, POWER_ONOFF_AND_STANDBY_TASK_NAME);
            }
            break;
        default:
            break;
    }
    return 0;
}

static int on_post_power_init_route(struct power_manager_context *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    ctx->priv.pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
#if IS_SUPPORT_PWR_ON_WHEN_PLUG_IN
    if ((battery && battery->is_vbus_plug_in() > 0) ||
        platform_is_startup_caused_by_reboot()) {
        key_input_context_t *key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);  
        if (key && key->is_key_triggered(KEY_INDEX_POWER)) {
            ctx->priv.is_power_on_timeout_detect = 1;
        }
        /// Keep dedicated task name for delayed power-on path to preserve existing scheduling behavior.
        POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, 500, HDR_ACTION_POWER_ON, ctx,
            POWER_ONOFF_AND_STANDBY_TASK_NAME);
        return 0;
    }
#else
    if (platform_is_startup_caused_by_reboot()) {
        key_input_context_t *key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
        if (key && key->is_key_triggered(KEY_INDEX_POWER)) {
            ctx->priv.is_power_on_timeout_detect = 1;
        }
        LOG_PM("power on by reboot");
        POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, 500, HDR_ACTION_POWER_ON, ctx,
            POWER_ONOFF_AND_STANDBY_TASK_NAME);
        return 0;
    }
#endif
    uint32_t timeout_ms = POWER_ON_TIMEOUT_MS;
    LOG_PM("power on time: %d", timeout_ms);
    if (timeout_ms > POWER_ON_TIMEOUT_MS_DEFAULT) {
        timeout_ms = POWER_ON_TIMEOUT_MS_DEFAULT;
    }
    ctx->priv.is_power_on_timeout_detect = 1;
    POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, timeout_ms, HDR_ACTION_POWER_ON_DETECT,
        ctx, POWER_ONOFF_AND_STANDBY_TASK_NAME);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_power_manager_hook(power_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __power_manager_hook));
    ctx->hook->on_post_init_route = on_post_power_init_route;
    ctx->hook->on_key_event_route = on_key_event_route;
    ctx->hook->on_broadcast_receive_route = on_power_broadcast_receive_route;
    ctx->hook->pm_suspend_route = on_pm_suspend_route;
    ctx->hook->pm_resume_route = on_pm_resume_route;
    ctx->hook->set_suspend_counter_route = set_suspend_counter_route;
    ctx->hook->is_prohibit_pm_suspend = is_prohibit_pm_suspend;
}