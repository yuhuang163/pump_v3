/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "battery_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static battery_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static battery_state_e get_battery_state(int percent, int charge_status)
{
    LOG_MISC("percent: %d, charge_status: %d", percent, charge_status);

    if (percent <= EMPTY_BAT_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_EMPTY;
    }

    if (percent <= LOW_BAT_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_LOW_POWER_SHUTDOWN;
    } 
    
    if (percent <= FORBID_START_BAT_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_FORBID_START;
    } 
    
    if (percent <= LOW_POWER_RUN_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_LOW_POWER_RUN;
    } 

    return BATTERY_STATE_NONE;
}

static uint8_t local_is_vbus_plug_in(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.battery == NULL) {
        return 0;
    }
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_VBUS_IS_PLUG_IN, ret);
    return ret > 0 ? 1 : 0;
}

static uint8_t local_get_battery_percent(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.battery == NULL) {
        return 100;
    }
    uint8_t val = 0;
    if (thiz->priv.battery) {   
        val = DEVICE_READ(thiz->priv.battery);
    } else {
        val = 100;
    }
    return (val > 100) ? 100 : val;
}

static uint8_t local_is_low_battery(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.battery_state == BATTERY_STATE_EMPTY ||
        thiz->priv.battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN) {
        return 1;
    }
    return 0;
}

static uint8_t local_get_charge_status(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    EXCEPTION_CHECK_UINT(thiz->priv.battery);
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_GET_CHARGE_STATUS, ret);
    return ret;
}

static uint8_t local_is_adapter_mismatch(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    return thiz->priv.is_adapter_mismatch;
}

static uint8_t local_get_battery_state(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    return thiz->priv.battery_state;
}

static void polling_battery_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.battery == NULL) {
        return;
    }
    int bat_per = local_get_battery_percent();
    int charge_status = local_get_charge_status();
    battery_state_e battery_state = get_battery_state(bat_per, charge_status);
    LOG_MISC("#now bat percent:%d, state:%d/%d, charging:%d/%d", bat_per, thiz->priv.battery_state,
        battery_state, thiz->priv.is_vbus_plug_in, thiz->priv.is_charging);
    if (thiz->priv.battery_state != battery_state) {
        thiz->priv.battery_state = battery_state;
        if (thiz->priv.battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
            thiz->priv.battery_state == BATTERY_STATE_EMPTY) {
            window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
            if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0 &&
                wm->is_current_window(wm, ID_WINDOW_LOGO) == 0) {
                multi_func_msg_data_t msg = {0};
                msg.union_body.extra.window_id = ID_WINDOW_POPUP_WINDOW;
                msg.union_body.extra.is_bg_dispatch = 0x01;
                msg.union_body.extra.arg1 = ID_POPUP_UI_LOW_POWER;
                msg.union_body.extra.popup_exit_time = 3000;
                msg.union_body.extra.forbid_short_press_exit_popup = true;
                wm->dispatch_new_window_args(wm, &msg);
            } else {
                LOG_MISC("in charge or logo window, ignore popup window.");
            }
            servers_publish_data_args_t args_temp = {
                .arg1 = OLD_PUBLISH_ABNORMAL_LOW_BAT_SHUTDOWN_SOON,
                .arg2 = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE,
                .val = NULL,
            };
            servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
            EXCEPTION_CHECK_VOID(servers_ctx);
            servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
        }
    }
    
    if (thiz->priv.battery_state == BATTERY_STATE_EMPTY ||
        thiz->priv.battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN) {
        SEND_BROADCAST_ARGS2(ID_BROADCAST_BATTERY_STATE_CHANGE, BATTERY_BROADCAST_LOW_POWER_SHUTDOWN, bat_per);   /// 发送电池状态广播.
    } else if (thiz->priv.battery_state == BATTERY_STATE_LOW_POWER_RUN ||
        thiz->priv.battery_state == BATTERY_STATE_FORBID_START) {
        SEND_BROADCAST_ARGS2(ID_BROADCAST_BATTERY_STATE_CHANGE, BATTERY_BROADCAST_LOW_POWER_RUN, bat_per);   /// 发送电池状态广播.
    } else {
        SEND_BROADCAST_ARGS2(ID_BROADCAST_BATTERY_STATE_CHANGE, BATTERY_BROADCAST_CAPACITY_CHANGE, bat_per);   /// 发送电池状态广播.
    }
}

static void polling_battery_voltage_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.battery);
    DEVICE_IOCTL_NO_RET(thiz->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
}

static void polling_charge_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    uint8_t plug_status = local_is_vbus_plug_in() ;
    if (plug_status == thiz->priv.is_vbus_plug_in) {
        return;
    }
    LOG_MISC("charge:%d", plug_status);
    thiz->priv.is_vbus_plug_in = plug_status;
    if (thiz->priv.is_vbus_plug_in) {
        if (pm->is_pm_suspend()) {
            pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_IN);
        } else {
            LOG_MISC("charge_plug_in");
            // pm->acquire_wakelock("charge_plug_in", -1);
        }
        if (BATTERY_IS_CHARGING(local_get_charge_status())) {
            thiz->priv.is_charging = 1;
        } else {
            thiz->priv.is_charging = 0;
        }
    } else {
        if (pm->is_pm_suspend()) {
            pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_OUI);
        } else {
            LOG_MISC("charge_plug_out");
            // pm->release_wakelock("charge_plug_in");
        }
    }
    SEND_BROADCAST_ARGS2(ID_BROADCAST_BATTERY_STATE_CHANGE, BATTERY_BROADCAST_CHARGE_STATE_CHANGE,
        thiz->priv.is_vbus_plug_in);  /// 发送充电状态广播.
}

static void charge_event_callback(charge_event_e event)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    EXCEPTION_CHECK_VOID(thiz);
    LOG_MISC("charge event: %d", event);

    /// 上报适配器是否匹配
    servers_publish_data_args_t args_temp = {
        .arg1 = OLD_PUBLISH_ABNORMAL_ADAPTER_ERROR,
        .arg2 = event == CHARGE_EVENT_ADAPTER_MISMATCH ? OLD_PUBLISH_ABNORMAL_STATE_ACTIVE : OLD_PUBLISH_ABNORMAL_STATE_CLEARED,
        .val = NULL,
    };
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(servers_ctx);
    servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
    
    if (event == CHARGE_EVENT_ADAPTER_MISMATCH) {
#if IS_SUPPORT_PD_CHARGE_ADAPTER_MISMATCH
        thiz->priv.is_adapter_mismatch = 1;
        multi_func_msg_data_t __msg = {0};
        __msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        LOG_WM("charge adapter mismatch goto charge screen");
        wm->dispatch_new_window_args(wm, &__msg);
#else
        LOG_MISC("charge adapter mismatch ignore");
        thiz->priv.is_adapter_mismatch = 0;
#endif
        return;
    }
    
    if (thiz->priv.is_adapter_mismatch) {
        thiz->priv.is_adapter_mismatch = 0;
        LOG_MISC("charge event: adapter mismatch recover");
        SEND_BROADCAST_ARGS1(ID_BROADCAST_BATTERY_STATE_CHANGE, 
            BATTERY_BROADCAST_CHARGE_ADAPTER_MISMATCH_RECOVER);
    }

    int misc_msg = 0;
    if (wm->get_current_window_id() != ID_WINDOW_CHARGE) {
        misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
        EXCEPTION_CHECK_VOID(misc);
        misc->trigger_misc_work(MISC_WORK_MSG_ID_CHARGE_INTERRUPT);
    }
}

static int on_post_battery_init(void *ptr)
{
    battery_manager_context_t *ctx = ptr;
    if (ctx->priv.battery) {
        DEVICE_IOCTL_NO_RET(thiz->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
    }
    thiz->priv.battery_state = BATTERY_STATE_NONE;
    thiz->priv.is_vbus_plug_in = 0;
    thiz->priv.is_charging = 0;
    misc_manager_context_t *misc = ctx->priv.misc;
    if (misc) {
#if IS_SUPPORT_DEVICE_CHARGE
        misc->add_misc_task(misc, MISC_TASK_ID_CHARGE_POLLING, 0, 
            MISC_WORK_MSG_ID_TIMER | MISC_WORK_MSG_ID_CHARGE_INTERRUPT, polling_charge_status);
#endif
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_POLLING, 1500, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_status);
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_VOLTAGE_POLLING, 0, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_voltage_status);
    }

    if (ctx->priv.battery) {
        DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.battery, OP_CODE_BAT_ADD_CHARGE_EVENT_CALLBACK, charge_event_callback);
        DEVICE_IOCTL_HAS_RET(ctx->priv.battery, OP_CODE_BAT_PD_IS_MISMATCH, ret);
#if IS_SUPPORT_PD_CHARGE_ADAPTER_MISMATCH
        thiz->priv.is_adapter_mismatch = ret ? 1 : 0;
#else
        thiz->priv.is_adapter_mismatch = 0;
#endif
    }

    return 0;
}

static int on_perpare_battery_init(void *self)
{
    battery_manager_context_t *ptr = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    ptr->priv.battery = pbus->find_device(ID_DEVICE_BATTERY);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_battery_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, battery_manager_context_t);

    ptr->priv.battery_percent = 5;
    ptr->is_vbus_plug_in = local_is_vbus_plug_in;
    ptr->get_charge_status = local_get_charge_status;
    ptr->is_low_battery = local_is_low_battery;
    ptr->get_battery_state = local_get_battery_state;
    ptr->get_battery_percent = local_get_battery_percent;
    ptr->is_adapter_mismatch = local_is_adapter_mismatch;
    thiz = ptr;
    return ADD_MODULE("battery", ID_MODULE_BATTERY, on_perpare_battery_init, on_post_battery_init, 
        ptr, list_head, sizeof(battery_manager_context_t));
}
