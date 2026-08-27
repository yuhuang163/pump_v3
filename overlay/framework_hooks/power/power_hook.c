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
#define POWER_ON_TIMEOUT_MS_DEFAULT                 (2000 - POWER_ON_BOOT_RUN_TIME_MS)     /// 电源按键超时时间，单位ms
#define POWER_ON_TIMEOUT_MS                         (POWER_ON_TIMEOUT_MS_DEFAULT - get_timestamp_ms(0))
#define POWER_ONOFF_AND_STANDBY_TASK_NAME           "power_onoff_and_standby"
#define POWER_ON_WAIT_WM_READY_RETRY_MAX            50    /// 等待 WM 就绪： 1s 内超时
#define POWER_ON_WAIT_WM_READY_INTERVAL_MS          20


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
static bool is_power_off_by_charging(void)
{
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    EXCEPTION_CHECK_BOOL(battery);
    if (battery->is_vbus_plug_in() > 0) {
        return true; /// 充电状态下禁止进入待机状态.
    }
    return false;
}

#if IS_SUPPORT_POWER_ON_GUIDE_UI
/// 适配器不匹配（VBUS 插入且充电状态为 ERROR）
static bool is_adapter_mismatch(battery_manager_context_t *battery)
{
    if (battery == NULL) {
        return false;
    }
    return battery->is_vbus_plug_in() > 0 &&
           battery->get_charge_status() == CHARGE_STATE_ERROR;
}
#endif

/** OTA / 蓝牙测试不记；其余重启原因都记 */
static bool is_reboot_reason_need_bury(int reason)
{
    switch (reason) {
        case SOFT_BOOT_ENTER_HIDE_BOOT:      /* OTA / hide boot */
        case SOFT_BOOT_TEST_MODE:            /* 蓝牙测试 */
        case SOFT_BOOT_TEST_SIGNALINGMODE:
        case SOFT_BOOT_TEST_NOSIGNALINGMODE:
        case SOFT_BOOT_CDC_OR_TEST_NORMAL:
            return false;
        default:
            return reason != CORE_BOOT_CAUSE_NULL;
    }
}

static void system_power_off(struct power_manager_context *ctx)
{
    LOG_PM("system_power_off");
    ctx->priv.is_system_power_on = false;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    rt_device_node_t *power_dev = pbus->find_device(ID_DEVICE_POWER);
    EXCEPTION_CHECK_VOID(power_dev);
    int args[] = {0};
    for (uint8_t i = 0; i < 3; i++) {
        power_dev->ioctl(power_dev, OP_CODE_POWER_CONTROL, (void *)args);
        msleep(200);
        LOG_PM("power off try: %d", i);
    }
    platform_system_reboot();
}

static bool is_prohibit_pm_suspend(struct power_manager_context *ctx)
{
    EXCEPTION_CHECK_BOOL(ctx);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_BOOL(wm);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    EXCEPTION_CHECK_BOOL(battery);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
    /// 适配器不匹配页：禁止自动关机/待机
    if (is_adapter_mismatch(battery)) {
        LOG_PM("adapter mismatch, prohibit auto power off");
        return true;
    }
#endif
    if (BATTERY_IS_CHARGING(battery->get_charge_status())) {
        if (wm->is_current_window(wm, ID_WINDOW_POWER_OFF) == 0) {
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
            msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_CHARGE;
            wm->dispatch_new_window_args(wm, &msg);
            LOG_PM("goto power off charge window");
        }
        return true; /// 充电状态下禁止进入待机状态.
    }
    return false;
}

/** 开机任务可能早于 WM 线程就绪，切屏前先等 window_boot_completed */
static int dispatch_window_when_wm_ready(window_manager_context_t *wm, multi_func_msg_data_t *msg)
{
    EXCEPTION_CHECK_INT(wm);
    uint16_t retry = 0;

    while (!wm->system_is_ready(wm)) {
        if (retry >= POWER_ON_WAIT_WM_READY_RETRY_MAX) {
            LOG_PM("wait wm ready timeout, drop window:%d", msg->union_body.extra.window_id);
            return -1;
        }
        if ((retry % 25) == 0) {
            LOG_PM("wait wm ready (%d), window:%d", retry, msg->union_body.extra.window_id);
        }
        retry++;
        msleep(POWER_ON_WAIT_WM_READY_INTERVAL_MS);
    }
    return wm->dispatch_new_window_args(wm, msg);
}

static void system_power_event_cb(hdr_post_args_t *args)
{
    int action = args->arg1;
    power_manager_context_t *pm = args->user_data;
    EXCEPTION_CHECK_VOID(pm);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    EXCEPTION_CHECK_VOID(battery);
    multi_func_msg_data_t msg = {0};
    switch (action) {
        case HDR_ACTION_POWER_ON_DETECT:
            LOG_PM("%s action:%d", args->name, action, pm->priv.is_power_on_timeout_detect);
            if (pm->priv.is_power_on_timeout_detect == 0) {
                return;
            }
            pm->priv.is_power_on_timeout_detect = 0;
            key_input_context_t *key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
            if (key && key->is_key_triggered(KEY_INDEX_POWER) == 0) {
                #if IS_SUPPORT_POWER_OFF_WHEN_CHARGING
                if (is_power_off_by_charging()) {
                    pm->priv.is_system_power_on = true;
                    SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
                    DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_POWER_ON, 0, 0);
                    msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
                    msg.union_body.extra.arg1 = 0;
                    LOG_PM("goto power off charge screen");
                    dispatch_window_when_wm_ready(wm, &msg);
                    return;
                }
                #endif
                #if IS_SUPPORT_POWER_ON_GUIDE_UI
                pm->priv.is_system_power_on = true;
                SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
                DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_POWER_ON, 0, 0);
                if (is_adapter_mismatch(battery)) {
                    msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
                    LOG_PM("power on detect key up, adapter mismatch, goto charge");
                } else {
                    msg.union_body.extra.window_id = ID_WINDOW_DUMMY;
                    LOG_PM("power on detect key up, goto power on guide");
                }
                dispatch_window_when_wm_ready(wm, &msg);
                #else
                system_power_off(pm);
                #endif
            } else {
                pm->priv.is_system_power_on = true;
                SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
                DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_POWER_ON, 0, 0);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
                if (is_adapter_mismatch(battery)) {
                    msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
                    LOG_PM("adapter mismatch, goto charge screen");
                } else {
                    msg.union_body.extra.window_id = ID_WINDOW_LOGO;
                    LOG_PM("goto logo screen");
                }
#else
                msg.union_body.extra.window_id = ID_WINDOW_LOGO;
                LOG_PM("goto logo screen");
#endif
                dispatch_window_when_wm_ready(wm, &msg);
            }
            break;
        case HDR_ACTION_POWER_ON:
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.is_system_power_on = true;
            SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_POWER_ON, 0, 0);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
            if (is_adapter_mismatch(battery)) {
                msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
                LOG_PM("adapter mismatch, goto charge screen");
            } else {
                msg.union_body.extra.window_id = ID_WINDOW_LOGO;
                LOG_PM("goto logo screen");
            }
#else
            msg.union_body.extra.window_id = ID_WINDOW_LOGO;
            LOG_PM("goto logo screen");
#endif
            dispatch_window_when_wm_ready(wm, &msg);
            /// 记录重启原因到 RAM 并埋点 0x61（排除 OTA / 蓝牙测试 /）
            if (platform_is_startup_caused_by_reboot()) {
                int reason = (int)platform_get_sys_reboot_reason();
                if (is_reboot_reason_need_bury(reason)) {
                    storage_manager_context_t *st =
                        (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
                    if (st && st->put_store_ram_val) {
                        st->put_store_ram_val(STORE_ID_SYS_REBOOT_REASON, reason, 0, NULL);
                    }
                    DATA_BURY_EVENT_TRIGGER_EXCEPTION(DATA_BURY_ERROR_SYSTEM_ERROR_REBOOT, reason);
                    LOG_PM("record system reboot reason: %d", reason);
                }
            }
            break;
        case HDR_ACTION_POWER_ON_BY_CHARGING:    
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.is_system_power_on = true;
            SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_POWER_ON, 0, 0);
            msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
            msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_CHARGE;
            LOG_PM("goto power off charge screen");
            dispatch_window_when_wm_ready(wm, &msg);
            break;
#if IS_SUPPORT_POWER_ON_GUIDE_UI
        case HDR_ACTION_POWER_ON_GUIDE:
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.is_system_power_on = true;
            SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_ON);
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_POWER_ON, 0, 0);
            if (is_adapter_mismatch(battery)) {
                msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
                LOG_PM("adapter mismatch, goto charge screen");
            } else {
                msg.union_body.extra.window_id = ID_WINDOW_DUMMY;
                LOG_PM("goto power on guide (dummy)");
            }
            dispatch_window_when_wm_ready(wm, &msg);
            break;
#endif
        case HDR_ACTION_POWER_ON_INVALID: 
            LOG_PM("%s action:%d", args->name, action);
            system_power_off(pm);
            break;
        case HDR_ACTION_POWER_OFF: 
            LOG_PM("%s action:%d", args->name, action);
            pm->priv.allow_shutdown_or_standby = 0;
            msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
            msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_MANUAL;
            LOG_PM("goto power off charge screen");
            dispatch_window_when_wm_ready(wm, &msg);
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

    /// 待机状态下长按2s退出待机状态.
    if (ctx->priv.is_enter_suspend == true  && keytype == KEY_EVENT_LONG_2S) {
        ctx->pm_wakeup(PM_WAKEUP_REASON_KEY_POWER); /// 唤醒电源管理器.
        LOG_PM("wakeup by key power");
        return 0;
    }

    /// 开机按键超时检测, 如果超时, 则进入关机状态.
    if (keytype == KEY_EVENT_UP && ctx->priv.is_power_on_timeout_detect == 1) {
        ctx->priv.is_power_on_timeout_detect = 0;
        #if IS_SUPPORT_POWER_OFF_WHEN_CHARGING
        if (is_power_off_by_charging()) {
            uint32_t timeout_ms = POWER_ON_TIMEOUT_MS;
            LOG_PM("power on time: %d", timeout_ms);
            if (timeout_ms > POWER_ON_TIMEOUT_MS_DEFAULT) {
                timeout_ms = 100;
            }
            POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, timeout_ms, HDR_ACTION_POWER_ON_BY_CHARGING, ctx,
                POWER_ONOFF_AND_STANDBY_TASK_NAME);
            return 0;
        }
        #endif

        #if IS_SUPPORT_POWER_ON_GUIDE_UI
        /// 短按未满足开机时长：进入 dummy 长按开机引导
        POST_TASK_HANDLER_NO_DELAY_ARG1(system_power_event_cb, ctx, HDR_ACTION_POWER_ON_GUIDE,
            POWER_ONOFF_AND_STANDBY_TASK_NAME);
        #else
        POST_TASK_HANDLER_NO_DELAY_ARG1(system_power_event_cb, ctx, HDR_ACTION_POWER_ON_INVALID,
            POWER_ONOFF_AND_STANDBY_TASK_NAME);
        #endif
        
        return 0;
    }

    /// 开机检测窗口内收到2s长按, 立即确认开机成功, 避免后续抬键触发invalid关机.
    if (keytype == KEY_EVENT_LONG_2S && ctx->priv.is_power_on_timeout_detect == 1) {
        ctx->priv.is_power_on_timeout_detect = 0;
        if (!ctx->is_system_power_on()) {
            POST_TASK_HANDLER_NO_DELAY_ARG1(system_power_event_cb, ctx, HDR_ACTION_POWER_ON,
                POWER_ONOFF_AND_STANDBY_TASK_NAME);
        }
        return 0;
    }

    LOG_PM("key:%d, ignore, power state:%d", keytype, ctx->is_system_power_on());
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
    msg.union_body.extra.arg1 = ID_POPUP_UI_WILL_POWER_OFF;
    msg.union_body.extra.popup_exit_time = 3000;
    msg.union_body.extra.user_data = ctx;
    msg.union_body.extra.forbid_short_press_exit_popup = false;
    wm->dispatch_new_window_args(wm, &msg); 
    set_pm_state_progress(ctx, __func__, PM_WILL_POWER_OFF, 0);
#else
    ctx->priv.allow_shutdown_or_standby = 0;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(wm);
#if V3_UX_VERSION == V3_UX_V6
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        breast_pump_manager_context_t *pump =
            (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
        if (pump) {
            int pump_status = 0;
            READ_PUMP_WORK_STATUS(pump, pump_status);
            if (pump_status == BREAST_PUMP_PAUSE) {
                LOG_PM("pump window paused, notify pause settle before power off");
                wm->post_window_event(__func__, ID_WINDOW_PUMP,
                    WINDOW_ASYNC_EVT_PUMP_PAUSE_SETTLE, NULL, 0);
                set_pm_state_progress(ctx, __func__, PM_POWER_OFF, 0);
                return 0;
            }
        }
    }
#endif
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
    msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_AUTO;
    LOG_PM("goto power off charge screen");
    wm->dispatch_new_window_args(wm, &msg); 
    set_pm_state_progress(ctx, __func__, PM_POWER_OFF, 0);
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
    set_pm_state_progress(ctx, __func__, PM_RESUME_FINISH, 0);
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
        if (ctx->is_system_power_on() > 0) {
            ctx->priv.allow_shutdown_or_standby = 1;
        }
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
    uint32_t timeout_ms = POWER_ON_TIMEOUT_MS;
    if (timeout_ms > POWER_ON_TIMEOUT_MS_DEFAULT) {
        timeout_ms = 100;
    }

    if (platform_is_startup_caused_by_reboot()) {
        LOG_PM("power on by reboot");
        ctx->priv.is_power_on_timeout_detect = 0;
        POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, timeout_ms, HDR_ACTION_POWER_ON, ctx,
            POWER_ONOFF_AND_STANDBY_TASK_NAME);
        return 0;
    }

    key_input_context_t *key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    if (key && key->is_key_triggered(KEY_INDEX_POWER) == 0) {
        ctx->priv.is_power_on_timeout_detect = 0;
#if IS_SUPPORT_POWER_OFF_WHEN_CHARGING
        if (is_power_off_by_charging()) {
            LOG_PM("power on by charging, timeout: %d", timeout_ms);
            POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, timeout_ms, 
                HDR_ACTION_POWER_ON_BY_CHARGING, ctx, POWER_ONOFF_AND_STANDBY_TASK_NAME);
            return 0;
        }
#endif
#if IS_SUPPORT_POWER_ON_GUIDE_UI
        /// 插电开机未按住 power：进入 dummy 长按开机引导
        LOG_PM("power on guide by plug-in, timeout: %d", timeout_ms);
        POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, timeout_ms,
            HDR_ACTION_POWER_ON_GUIDE, ctx, POWER_ONOFF_AND_STANDBY_TASK_NAME);
#else
        system_power_off(ctx);
#endif
        return 0;
    }
    LOG_PM("key down, power on timeout: %d", timeout_ms);
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