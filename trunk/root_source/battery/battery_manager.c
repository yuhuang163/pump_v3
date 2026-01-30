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
static bool check_if_low_battery(void)
{
    EXCEPTION_CHECK_BOOL(thiz);
    EXCEPTION_CHECK_BOOL(thiz->priv.battery);
    int percent = 0;
    DEVICE_READ_ARGS(thiz->priv.battery, &percent, sizeof(percent));
    return percent <= LOW_BAT_PERCENTAGE ? 1 : 0;
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

static void local_state_change_listener(on_charge_state_change_fptr cb)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.m_charge_listener_cb = cb;
    LOG_MISC("thiz->charge.callback: %lx", TO_PRINT(cb));
}

static void local_state_battery_listener(on_battery_state_change_fptr cb)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.m_battery_listener_cb = cb;
    LOG_MISC("battery.m_listener_cb: %lx", TO_PRINT(cb));
}

static uint8_t local_is_low_battery(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    return thiz->priv.low_battery;
}

static uint8_t is_empty_battery(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    int percent = 0;
    if (thiz->priv.battery) {
        DEVICE_READ_ARGS(thiz->priv.battery, &percent, sizeof(percent));
    } else {
        percent = 100;
    }
    
    if (percent <= EMPTY_BAT_PERCENTAGE) {
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

static void polling_battery_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.battery == NULL) {
        return;
    }
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    uint8_t bat_tmp = check_if_low_battery();
    if (thiz->priv.low_battery != bat_tmp) {
        thiz->priv.low_battery = bat_tmp;
    }
    int bat_per = DEVICE_READ(thiz->priv.battery);
    LOG_MISC("#now bat percent: %d, is_low_battery: %d, is_plug: %d, is_charging: %d", 
        bat_per, thiz->priv.low_battery, thiz->priv.is_vbus_plug_in, thiz->priv.is_charging);
    SEND_BROADCAST_ARGS2(ID_BROADCAST_BATTERY_STATE_CHANGE, 
        thiz->priv.low_battery ? BATTERY_BROADCAST_LOW_BATTERY : BATTERY_BROADCAST_CAPACITY_CHANGE, bat_per);   /// 发送电池状态广播.
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
            pm->acquire_wakelock("charge_plug_in", -1);
        }
    } else {
        if (pm->is_pm_suspend()) {
            pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_OUI);
        } else {
            LOG_MISC("charge_plug_out");
            pm->release_wakelock("charge_plug_in");
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
    if (event == CHARGE_EVENT_ADAPTER_MISMATCH) {
        LOG_MISC("charge event: adapter mismatch");
        thiz->priv.is_adapter_mismatch = 1;
        multi_func_msg_data_t __msg = {0};
        __msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        LOG_WM("goto charge screen");
        wm->dispatch_new_window_args(wm, &__msg);
        return;
    }
    
    if (thiz->priv.is_adapter_mismatch) {
        thiz->priv.is_adapter_mismatch = 0;
        LOG_MISC("charge event: adapter mismatch recover");
        SEND_BROADCAST_ARGS1(ID_BROADCAST_BATTERY_STATE_CHANGE, BATTERY_BROADCAST_CHARGE_ADAPTER_MISMATCH_RECOVER);
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
    thiz->priv.low_battery = check_if_low_battery();
    thiz->priv.is_vbus_plug_in = 0;
    thiz->priv.is_charging = 0;
    misc_manager_context_t *misc = ctx->priv.misc;
    if (misc) {
        DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_IS_SUPPORT_BATTERY, support_battery);
        if (support_battery <= 0) {
            LOG_MISC("battery is not supported");
            return 0;
        }
        misc->add_misc_task(misc, MISC_TASK_ID_CHARGE_POLLING, 0, 
            MISC_WORK_MSG_ID_TIMER | MISC_WORK_MSG_ID_CHARGE_INTERRUPT, polling_charge_status);
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_POLLING, 1500, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_status);
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_VOLTAGE_POLLING, 0, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_voltage_status);
    }

    if (ctx->priv.battery) {
        DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.battery, OP_CODE_BAT_ADD_CHARGE_EVENT_CALLBACK, charge_event_callback);
    }
    return 0;
}

static int on_perpare_battery_init(void *self)
{
    battery_manager_context_t *ptr = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    ptr->priv.battery = pbus->find_device(ID_DEVICE_BATTERY);
    ptr->priv.led_dev = pbus->find_device(ID_DEVICE_LED);

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
    ptr->set_state_change_listener = local_state_change_listener;

    ptr->is_low_battery = local_is_low_battery;
    ptr->is_empty_battery = is_empty_battery;
    ptr->get_battery_percent = local_get_battery_percent;
    ptr->is_adapter_mismatch = local_is_adapter_mismatch;
    ptr->set_state_battery_listener = local_state_battery_listener;
    thiz = ptr;
    return ADD_MODULE("battery", ID_MODULE_BATTERY, on_perpare_battery_init, on_post_battery_init, 
        ptr, list_head, sizeof(battery_manager_context_t));
}
