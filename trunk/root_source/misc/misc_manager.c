/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#ifndef MISC_POLLING_PERIOD
#define MISC_POLLING_PERIOD                               (100)  
#endif

#ifndef MISC_CHARGE_POLLING_PERIOD
#define MISC_CHARGE_POLLING_PERIOD                        (15)  
#endif

#ifndef MISC_THD_POLLING_PERIOD
#define MISC_THD_POLLING_PERIOD                           (1500)
#endif

#ifndef SYSINFO_PRINT_PERIOD
#define SYSINFO_PRINT_PERIOD                              (5000)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static misc_manager_context_t *thiz = NULL;

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
static bool check_if_low_battery(misc_manager_context_t *ctx)
{
    int percent = 0;
    DEVICE_READ_ARGS(ctx->priv.battery, &percent, sizeof(percent));

    if (percent <= LOW_BAT_PERCENTAGE) {
        return 1;
    }
    return 0;
}

static void polling_battery_status(misc_manager_context_t *ctx, window_manager_context_t *wm,
               power_manager_context_t *pm, uint8_t is_plug_state_changed)
{
    uint8_t bat_tmp = check_if_low_battery(ctx);
    if (ctx->priv.low_battery != bat_tmp) {
        ctx->priv.low_battery = bat_tmp;
    }

    if (!is_plug_state_changed && ctx->priv.low_battery) {      
        if (ctx->priv.battery_ui_has_show == 0) {
            if (!pm->is_pm_suspend()) {
                ctx->show_battery_ui_if_need(wm, false); /// 低电量提醒UI,仅在未休眠时弹出提醒.
            }
        }
        msleep(1000);
    }

    int bat_per = DEVICE_READ(ctx->priv.battery);
    LOG_MISC("#now bat percent: %d, is_low_battery: %d, is_plug: %d", bat_per, ctx->priv.low_battery, is_plug_state_changed);
    SEND_BROADCAST_ARGS2(ID_BROADCAST_UPDATE_BATTERY_INFO, ctx->priv.low_battery, bat_per);   /// 发送电池状态广播.
}

static void polling_charge_status(misc_manager_context_t *ctx, window_manager_context_t *wm,
                power_manager_context_t *pm, uint8_t *is_plug_state_changed)
{
    uint8_t plug_status = 0, chage_state = 0;

    chage_state = DEVICE_IOCTL(ctx->priv.battery, OP_CODE_BAT_GET_CHARGE_STATUS);
    if (chage_state == CHARGE_STATE_CHARGING || chage_state == CHARGE_STATE_FULL) {
        plug_status = 1;
    } else {
        plug_status = 0;
    }

    if (plug_status == *is_plug_state_changed) {
        return;
    }
    LOG_MISC("charge:%d", chage_state);

    #if IS_SUPPORT_AIRPLANE_MODE
    /// 飞行模式下不弹充电界面
    airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AP_MODE);
    if (ap && ap->is_on_airplane_mode()) {
        LOG_MISC("airplane_mode ignore");
        return;
    }
    #endif

    *is_plug_state_changed = plug_status;
    if (chage_state != CHARGE_STATE_ERROR) {
        if (*is_plug_state_changed) {
            if (pm->is_pm_suspend()) {
                pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_IN);
            } else {
                LOG_MISC("#show_charge_ui_if_need");
                ctx->show_charge_ui_if_need(wm, false);
            }
        } else {
            if (!pm->is_pm_suspend()) {
                pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_OUI);
            }
        }
    }
    SEND_BROADCAST_ARGS1(ID_BROADCAST_CHARGE_PLUG_IN_STATE, chage_state);  /// 发送充电状态广播.
}

static void misc_work_broadcast_receive(broadcast_mask_id_t id, void *context, int32_t args0, int32_t args1,
                                        broadcast_priv_data_t *pdata)
{
    misc_manager_context_t *ctx = (misc_manager_context_t *)context;
    if (id.code == ID_BROADCAST_PM_STATE_CHANGE) {
        int pm_state = args0;
        if (pm_state == ID_SYS_PM_SUSPEND) {
            LOGI("STOP_TIMER: misc_timer");
            STOP_TIMER(ctx->priv.misc_timer);
        } else {
            LOGI("START_TIMER: id = 0x%lx, misc_timer", id.code);
            START_TIMER(ctx->priv.misc_timer, MISC_POLLING_PERIOD);
            ctx->priv.battery_ui_has_show = 0;
        }
    } else if (id.code == ID_BROADCAST_PM_FREEZE_STATE_CHANGE) {
        platform_sysinfo_print(ctx);
    }
}

static void charge_irq_callback(uint8_t *data)
{
    misc_manager_context_t *ctx = thiz;
    int misc_msg = 0;
    
    if (ctx == NULL) {
        LOGE("thiz is NULL");
        return;
    }
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm->get_current_window_id() != ID_WINDOW_CHARGE) {
        MSG_QUEUE_SEND(ctx->priv.misc_work_thd.msg_id, &misc_msg, 0);
    }
}

static void on_misc_timer_cb(void *handle)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(thiz->priv.misc_work_thd.msg_id, &msg, 0);
}

// 杂项任务处理线程.
static void misc_work_polling_thread(const void *args)
{
    uint8_t is_plug_state_changed = 0, sys_ready = 0, misc_msg = 0;
    thread_context_t *thd = (thread_context_t *)args;
    misc_manager_context_t *ctx = thiz;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, MISC_WORK_DISPATCH_MESQ_NUM, 1);
    CREATE_TIMER(&ctx->priv.misc_timer, "misc-timer", ctx, OS_TIMER_PERIODIC, on_misc_timer_cb);
    START_TIMER(ctx->priv.misc_timer, MISC_POLLING_PERIOD);
    if (ctx->priv.battery && ctx->priv.battery->ioctl) {
        DEVICE_IOCTL_ARGS(ctx->priv.battery, OP_CODE_BAT_CHARGE_ADD_IRQ_CALLBACK, (void *)charge_irq_callback);
    }

    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &misc_msg, OS_WAIT_FOREVER);
        if (ctx->priv.battery && ctx->priv.battery->ioctl) {
            DEVICE_IOCTL_NO_RET(ctx->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
        }

        if (!sys_ready && wm) {
            sys_ready = wm->system_is_ready();
            continue;
        }

        /// 轮询充电状态
        if (ctx->priv.battery && ctx->priv.battery->ioctl) {
            polling_charge_status(ctx, wm, pm, &is_plug_state_changed);
        }

        EXECUTE_EVERY_X_DELAY(MISC_THD_POLLING_PERIOD, {
            /// 轮询电池状态.
            if (ctx->priv.battery && ctx->priv.battery->ioctl) {
                polling_battery_status(ctx, wm, pm, is_plug_state_changed);
            }
        });

        /// 系统资源情况打印
        EXECUTE_EVERY_X_DELAY(SYSINFO_PRINT_PERIOD, {
            platform_sysinfo_print(ctx); 
        });
    }
}

static uint8_t local_get_battery_percent(void)
{
    EXCEPTION_CHECK_UINT(thiz->priv.battery);
    uint16_t val = DEVICE_READ(thiz->priv.battery);
    val = (val) ? val : 1;
    LOG_MISC("get_percent: %d", val);

    return val;
}

static void local_set_charge_ui_show_flag(int flag)
{
    if (flag != thiz->priv.charge_ui_has_show) {
        thiz->priv.charge_ui_has_show = flag;
        LOG_MISC("charge.ui_has_show: %d", flag);
    }
}

static void local_set_battery_ui_show_flag(int flag)
{
    if (flag != thiz->priv.battery_ui_has_show) {
        thiz->priv.battery_ui_has_show = flag;
        LOG_MISC("battery.ui_has_show: %d", flag);
    }
}

static int on_post_misc_init(void *ptr)
{
    misc_manager_context_t *ctx = ptr;

    /// 注册广播监听.
    ctx->priv.misc_bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.misc_bcd->context = ctx;
    ctx->priv.misc_bcd->name = "misc_bcd_ctx";
    ctx->priv.misc_bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.misc_bcd->id.code |= ID_BROADCAST_PM_FREEZE_STATE_CHANGE;
    ctx->priv.misc_bcd->receiver_cb = misc_work_broadcast_receive;
    register_broadcast_receiver(ctx->priv.misc_bcd);
    
    /// 电量状态初始化
    DEVICE_IOCTL_NO_RET(thiz->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
    ctx->priv.low_battery = check_if_low_battery(ctx);
    RUN_NEW_THREAD(&ctx->priv.misc_work_thd, "misc_thd",
        misc_work_polling_thread, RT_PRIO_MISC_DISPATCH_THREAD, MISC_THREAD_STACKSIZE);

    return 0;
}

static int on_perpare_misc_init(void *self)
{
    misc_manager_context_t *ptr = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.battery = pbus->find_device(ID_DEVICE_BATTERY);

    return 0;
}

static void local_state_change_listener(on_charge_state_change_fptr cb)
{
    thiz->priv.m_charge_listener_cb = cb;
    LOG_MISC("thiz->charge.callback: %lx", TO_PRINT(cb));
}

static void local_state_battery_listener(on_battery_state_change_fptr cb)
{
    thiz->priv.m_battery_listener_cb = cb;
    LOG_MISC("battery.m_listener_cb: %lx", TO_PRINT(cb));
}

static uint8_t local_is_low_battery(void)
{
    return thiz->priv.low_battery;
}

static uint8_t is_empty_battery(void)
{
    int percent = 0;
    rt_device_node_t *dev = thiz->priv.battery;
    DEVICE_READ_ARGS(dev, &percent, sizeof(percent));
    if (percent <= EMPTY_BAT_PERCENTAGE) {
        return 1;
    }
    return 0;
}

static uint8_t local_is_charge_plug_in(void)
{
    EXCEPTION_CHECK_UINT(thiz->priv.battery);
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_CHARGE_IS_PLUG_IN, ret);
    if (ret != -1) {
        return ret;
    }
    LOGE("charge_if not ready!");
    return CHARGE_STATE_ERROR;
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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_misc_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, misc_manager_context_t);

    ptr->priv.battery_percent = 5;
    ptr->is_charge_plug_in = local_is_charge_plug_in;
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
    return ADD_MODULE("misc", ID_MODULE_MISC, on_perpare_misc_init, on_post_misc_init, 
        ptr, list_head, sizeof(misc_manager_context_t));
}
