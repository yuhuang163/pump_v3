/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "key_input_manager_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "input_key_code.h"
#include "power_hook.h"  // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_ON_BOOT_RUN_TIME_MS                   200     /// boot 启动补偿, 冷启动和热启动时间不一致
#define POWER_ON_TIMEOUT_MS_DEFAULT                 (3000 - POWER_ON_BOOT_RUN_TIME_MS)     /// 电源按键超时时间，单位ms
#define POWER_ON_TIMEOUT_MS                         (POWER_ON_TIMEOUT_MS_DEFAULT - get_timestamp_ms(0))
#define POWER_ON_DETECT_TASK_NAME                   "power_on_detect"
#define POWER_ON_INVALID_TASK_NAME                  "power_on_invalid"
#define POWER_OFF_TASK_NAME                         "power_off"
#define SYS_STANDBY_TASK_NAME                       "sys_standby"

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
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_BOOL(misc);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_BOOL(wm);
    if (misc->is_charge_plug_in() > 0) {
        if (wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0) {
            LOG_PM("charge, but not in charge window, dispatch charge window.");
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
    LOG_PM("power event, action:%d.", args->arg1);
    int action = args->arg1;
    power_manager_context_t *pm = args->user_data;
    EXCEPTION_CHECK_VOID(pm);
    switch (action) {
        case HDR_ACTION_POWER_ON_DETECT: {
            LOG_PM("%s", args->name);
            key_input_context_t *key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
            misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
            if (key && key->is_key_triggered(KEY_INDEX_POWER) == 0) {
                if ((misc && misc->is_charge_plug_in() > 0) || 
                    platform_is_startup_caused_by_reboot()) {
                    pm->priv.is_power_on_timeout_detect = 0;   
                    pm->system_power_control(true);
                    SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_POWER_STATE_CHANGE, ID_BROADCAST_SYS_POWER_ON);
                } else {
                    pm->system_power_control(false); /// 非充电状态下进行开机超时检测.
                }
            } else {
                pm->priv.is_power_on_timeout_detect = 0;   
                pm->system_power_control(true);
                SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_POWER_STATE_CHANGE, ID_BROADCAST_SYS_POWER_ON);
            }
        }
        break;

        case HDR_ACTION_POWER_ON_INVALID: {
            LOG_UI("%s", args->name);
            pm->priv.is_power_on_timeout_detect = 0;
            REMOVE_TASK_HANDLER_DIRECTLY(POWER_ON_DETECT_TASK_NAME);
            pm->system_power_control(false);
        }
        break;

        case HDR_ACTION_POWER_OFF: {
            LOG_PM("%s", args->name);
            pm->priv.allow_shutdown_or_standby = 0;
            pm->system_power_control(false);
        }
        break;

        case HDR_ACTION_SYS_STANDBY: {
            LOG_PM("%s", args->name);
            pm->priv.is_goto_sys_standby = true;
            SEND_BROADCAST_ARGS0(ID_BROADCAST_REQ_PM_SUSPEND_BY_USER);
        }
        break;

        default:
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
        return 0;
    }
    /// 开机按键超时检测, 如果超时, 则进入关机状态.
    if (keytype == KEY_EVENT_UP && ctx->priv.is_power_on_timeout_detect == 1) {
        misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
        if (misc && misc->is_charge_plug_in() > 0) {
            LOG_PM("charging. ignore");
            return 0;
        }
        POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, 10, HDR_ACTION_POWER_ON_INVALID,
            ctx, POWER_ON_INVALID_TASK_NAME);
        return 0;
    }
    /// 允许关机/待机标志设置.
    if (ctx->priv.allow_shutdown_or_standby == 0 && keytype == KEY_EVENT_DOWN) {
        ctx->priv.allow_shutdown_or_standby = ctx->is_system_power_on(); /// 允许关机/待机标志.
        LOG_PM("allow_shutdown_or_standby: %d", ctx->priv.allow_shutdown_or_standby);
        return 0;
    }
    /// 进入待机状态.
    if (keytype == KEY_EVENT_LONG_3S && ctx->priv.allow_shutdown_or_standby == 1) {
        POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, 0, HDR_ACTION_SYS_STANDBY,
            ctx, SYS_STANDBY_TASK_NAME);
    }
    return 0;
}

static int on_post_power_init_route(struct power_manager_context *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    ctx->priv.pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    uint32_t timeout_ms = POWER_ON_TIMEOUT_MS;
    if (timeout_ms > POWER_ON_TIMEOUT_MS_DEFAULT) {
        timeout_ms = 100;
    }
    ctx->priv.is_power_on_timeout_detect = 1;
    POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, POWER_ON_TIMEOUT_MS, HDR_ACTION_POWER_ON_DETECT,
        ctx, POWER_ON_DETECT_TASK_NAME);
    return 0;
}

static int on_power_broadcast_receive_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    power_manager_context_t *ctx = context;
    EXCEPTION_CHECK_INT(ctx);
    switch (id.code) {
        case ID_BROADCAST_SYS_POWER_STATE_CHANGE: {
            if (arg1 == ID_BROADCAST_SYS_POWER_OFF) {
                LOG_PM("--> system power off"); /// 1000ms 后关机.
                POST_TASK_HANDLER_DELAYED_ARG2(system_power_event_cb, 1000, HDR_ACTION_POWER_OFF,
                    ctx, POWER_OFF_TASK_NAME);
            }
        }
        break;
    }
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
    ctx->hook->is_prohibit_pm_suspend = is_prohibit_pm_suspend;
}