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

static void polling_battery_status(void *user_data)
{
    // LOG_MISC("polling_battery_status");
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

    if (!thiz->priv.is_plug_state_changed && thiz->priv.low_battery) {      
        if (thiz->priv.battery_ui_has_show == 0) {
            if (!pm->is_pm_suspend()) {
                LOG_MISC("#show_battery_ui_if_need");
                // ctx->show_battery_ui_if_need(wm, false); /// 低电量提醒UI,仅在未休眠时弹出提醒.
            }
        }
        msleep(1000);
    }

    int bat_per = DEVICE_READ(thiz->priv.battery);
    LOG_MISC("#now bat percent: %d, is_low_battery: %d, is_plug: %d", 
        bat_per, thiz->priv.low_battery, thiz->priv.is_plug_state_changed);
    SEND_BROADCAST_ARGS2(ID_BROADCAST_UPDATE_BATTERY_INFO, thiz->priv.low_battery, bat_per);   /// 发送电池状态广播.
}

#if IS_SUPPORT_BATTERY_DIRECT_SAMPLING
static void polling_battery_voltage_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.battery);
    DEVICE_IOCTL_NO_RET(thiz->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
}
#endif

static void polling_charge_status(void *user_data)
{
    // LOG_MISC("polling_charge_status");
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.battery == NULL) {
        return;
    }
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    uint8_t plug_status = 0, chage_state = 0;
    chage_state = DEVICE_IOCTL(thiz->priv.battery, OP_CODE_BAT_GET_CHARGE_STATUS);
    if (chage_state == CHARGE_STATE_CHARGING || chage_state == CHARGE_STATE_FULL) {
        plug_status = 1;
    } else {
        plug_status = 0;
    }

    if (plug_status == thiz->priv.is_plug_state_changed) {
        return;
    }
    LOG_MISC("charge:%d", chage_state);

/// 飞行模式下不弹充电界面
#if IS_SUPPORT_AIRPLANE_MODE
    airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    if (ap && ap->is_on_airplane_mode()) {
        LOG_MISC("airplane_mode ignore");
        return;
    }
#endif

    thiz->priv.is_plug_state_changed = plug_status;
    if (chage_state != CHARGE_STATE_ERROR) {
        if (thiz->priv.is_plug_state_changed) {
            if (pm->is_pm_suspend()) {
                pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_IN);
            } else {
                LOG_MISC("#show_charge_ui_if_need");
                // ctx->show_charge_ui_if_need(wm, false);
            }
        } else {
            if (!pm->is_pm_suspend()) {
                pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_OUI);
            }
        }
    }
    SEND_BROADCAST_ARGS1(ID_BROADCAST_CHARGE_STATE_CHANGE, chage_state);  /// 发送充电状态广播.
}

static void charge_irq_callback(uint8_t *data)
{
    EXCEPTION_CHECK_VOID(thiz);
    int misc_msg = 0;
    
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    if (wm->get_current_window_id() != ID_WINDOW_CHARGE) {
        misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
        EXCEPTION_CHECK_VOID(misc);
        misc->trigger_misc_work(MISC_WORK_MSG_ID_CHARGE_INTERRUPT);
    }
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

static void local_set_charge_ui_show_flag(int flag)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (flag != thiz->priv.charge_ui_has_show) {
        thiz->priv.charge_ui_has_show = flag;
        LOG_MISC("charge.ui_has_show: %d", flag);
    }
}

static void local_set_battery_ui_show_flag(int flag)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (flag != thiz->priv.battery_ui_has_show) {
        thiz->priv.battery_ui_has_show = flag;
        LOG_MISC("battery.ui_has_show: %d", flag);
    }
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

static uint8_t local_is_charge_plug_in(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.battery == NULL) {
        return 0;
    }
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_CHARGE_IS_PLUG_IN, ret);
    return ret > 0 ? 1 : 0;
}

static uint8_t local_get_charge_status(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    EXCEPTION_CHECK_UINT(thiz->priv.battery);
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_GET_CHARGE_STATUS, ret);
    return ret;
}

static uint8_t local_charge_show_ui_if_need(void *wm, int is_fg)
{
    window_manager_context_t *wm_ctx = (window_manager_context_t *)wm;
    multi_func_msg_data_t msg = {0};
    EXCEPTION_CHECK_UINT(wm_ctx);
    EXCEPTION_CHECK_UINT(thiz->priv.battery);
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_CHARGE_IS_PLUG_IN, plug_in);
    LOG_MISC("plug_in: %d, ui_has_show: %d, is_fg: %d", plug_in, thiz->priv.charge_ui_has_show, is_fg);
    if (plug_in && !thiz->priv.charge_ui_has_show) {
        if (!is_fg) {
            msg.union_body.extra.is_background = 0x1;
        } 
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        wm_ctx->dispatch_new_window_args(wm_ctx, &msg);
        return 1;
    }
    LOG_MISC("#ignore");
    return 0;
}

static uint8_t local_battery_show_ui_if_need(void *wm, int is_fg)
{
    window_manager_context_t *wm_ctx = (window_manager_context_t *)wm;
    multi_func_msg_data_t msg = {0};
    EXCEPTION_CHECK_UINT(wm_ctx);
    EXCEPTION_CHECK_UINT(thiz->priv.battery);
    LOG_MISC("battery show ui, is_low_battery %d, ui_has_show %d, is_fg %d", thiz->is_low_battery(),
            thiz->priv.battery_ui_has_show, is_fg);
    if (thiz->is_low_battery() && (0 == thiz->priv.battery_ui_has_show)) {
        if (!is_fg) {
            msg.union_body.extra.is_background = 0x1;
        } 
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        wm_ctx->dispatch_new_window_args(wm_ctx, &msg);
        return 1;
    }
    return 0;
}

static int on_post_battery_init(void *ptr)
{
    battery_manager_context_t *ctx = ptr;
    if (ctx->priv.battery) {
        DEVICE_IOCTL_NO_RET(thiz->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
    }
    thiz->priv.low_battery = check_if_low_battery();
    misc_manager_context_t *misc = ctx->priv.misc;
    if (misc) {
        misc->add_misc_task(misc, MISC_TASK_ID_CHARGE_POLLING, 0, 
            MISC_WORK_MSG_ID_TIMER | MISC_WORK_MSG_ID_CHARGE_INTERRUPT, polling_charge_status);
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_POLLING, 1500, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_status);
#if IS_SUPPORT_BATTERY_DIRECT_SAMPLING
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_VOLTAGE_POLLING, 0, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_voltage_status);
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
    ptr->is_charge_plug_in = local_is_charge_plug_in;
    ptr->get_charge_status = local_get_charge_status;
    ptr->set_charge_ui_show_flag = local_set_charge_ui_show_flag;
    ptr->show_charge_ui_if_need = local_charge_show_ui_if_need;
    ptr->set_state_change_listener = local_state_change_listener;

    ptr->is_low_battery = local_is_low_battery;
    ptr->is_empty_battery = is_empty_battery;
    ptr->get_battery_percent = local_get_battery_percent;
    ptr->set_battery_ui_show_flag = local_set_battery_ui_show_flag;
    ptr->show_battery_ui_if_need = local_battery_show_ui_if_need;
    ptr->set_state_battery_listener = local_state_battery_listener;
    thiz = ptr;
    return ADD_MODULE("battery", ID_MODULE_BATTERY, on_perpare_battery_init, on_post_battery_init, 
        ptr, list_head, sizeof(battery_manager_context_t));
}
