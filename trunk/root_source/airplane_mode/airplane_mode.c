/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "airplane_mode.h"

/*
* DEFINES
*****************************************************************************************
*/
#define CONTINUOUS_MOVE_THRESHOLD               15         //连续移动判定阈值(unit : s)。
#define CONTINUOUS_MOVE_MAX                     180        //连续移动判定阈值(unit : s)。默认 3分钟
#define CHECK_PERIOD                            3600000    //被动进入后，间隔检查的时间间隔(unit : ms)。默认1h
#define CHECK_TIME                              180000     //被动进入后，总检查时间。(uint : ms)默认3分钟。

#define AIRPLANE_PM_LOCK                        "airplane_pm_wake_lock"
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static airplane_mode_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int play_notify(int on)
{
    LOG_AIR("on: %d", on);

    #if 0
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR); 
    if (on) {
        motor->play_vibrate_notify("enter airplane_mode", VIBRATE_TYPE_LONG_MELODY);
    } else {
        motor->play_vibrate_notify("exit airplane_mode", VIBRATE_TYPE_LONG_MELODY);
    }
    #endif
    
    return 0;
}

static int popup_exit_cb(void *user_data, int args1, int args2)
{
#if 0 
    airplane_mode_context_t *thiz = user_data;
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    int which_type = args1;
    pm->release_wakelock(AIRPLANE_PM_LOCK);

    if (which_type == SETTINGS_AIRPLANE_MODE_ENTER) {
        LOG_AIR("popup_exit_cb: enter - %d -- %d", args1, args2);
        ble_manager_context_t *ble_m = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
        if (ble_m == NULL) {
            LOGE("ble module not found!!!");
            return -1;
        }

        thiz->set_airplane_mode(thiz->priv.enter_state);
        // 发送状态切换同步广播，通知感兴趣的组件.
        SEND_BROADCAST_SYNC_ARGS1(ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE, SETTINGS_AIRPLANE_MODE_ENTER);
        if (ble_m->is_bt_connected()) {
            LOG_AIR("bt is connected");

            DISPATCH_GO_NEW_WINDOW(wm, ID_WINDOW_AIRPLANE_MODE);
        } else {
            LOG_AIR("bt is disconnected");
            pm->enable_pmu_sleep("airplane sleep", true);
            SEND_BROADCAST_ARGS0(ID_BROADCAST_REQ_PM_SUSPEND_BY_USER);
        }
    } else {
        LOG_AIR("popup_exit_cb: exit - %d -- %d", args1, args2);
        // 发送状态切换同步广播，通知感兴趣的组件.
        SEND_BROADCAST_SYNC_ARGS1(ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE, SETTINGS_AIRPLANE_MODE_EXIT);
        if (!st->get_store_ram_val(STORE_ID_FACTORY_DEPLOY_FLAG)) {
            DISPATCH_GO_NEW_WINDOW(wm, ID_WINDOW_FACTORY_MODE);
        } else {
            GO_BACK_TO_HISTORY(wm);
        }
    }
#endif
    return 0;
}

static int popup_airplane_screen(window_manager_context_t *wm, int which)
{
    if (wm && wm->dispatch_new_window_args) {
        multi_func_msg_data_t msg = {0};
        LOG_AIR("popup_bg_which = %d", which);
        msg.union_body.extra.window_id = ID_WINDOW_POPUP_WINDOW;
        msg.union_body.extra.popup_bg_res_index = ID_POPUP_WINDOW_AIRPLANE_MODE;
        msg.union_body.extra.popup_bg_which = which;
        msg.union_body.extra.popup_exit_time = POP_AIRPLANE_WINDOW_TIME;
        msg.union_body.extra.user_data = thiz;
        msg.union_body.extra.popup_exit_cb = popup_exit_cb;
        wm->dispatch_new_window_args(wm, &msg);
    }
    return 0;
}

static int enter_airplane_mode_internal(int state)
{   
    int wait_retry = 0;
    ota_manager_context_t *ota_ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    if (ota_ctx->is_ota_upgrading()) {
        LOG_AIR("ota upgrading, forbid enter airplane");
        return -1;
    }
    
    while (1) {
        if (!thiz->priv.wm->is_current_window(ID_WINDOW_BEGIN) || wait_retry > 60) {
            break;
        }  
        wait_retry++;
        LOG_AIR("wait_retry: %d", wait_retry);
        msleep(50);
    }

    LOG_AIR("enter state: %d", state);
    thiz->priv.enter_state = state;

    /// 禁用IMU中断
    rt_device_node_t *pmu_dev = thiz->priv.pbus->find_device(ID_DEVICE_PMU);
    if (pmu_dev && pmu_dev->ioctl) {
        int agrs[1] = {false};
        DEVICE_IOCTL_ARGS_NO_RET(pmu_dev, OP_CODE_PMU_SET_IMU_WAKEUP, agrs);
    }
    /// 播放音频
    play_notify(true);
    ///弹出画面
    popup_airplane_screen(thiz->priv.wm, SETTINGS_AIRPLANE_MODE_ENTER);

    return 0;
}

static int enter_airplane_mode(void)
{
    return enter_airplane_mode_internal(AIRPLANE_MODE_BY_USER_ACTIVE_ENTER);
}

static void hdr_enter_ap_cb(hdr_post_args_t *args)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)args->user_data;
    LOG_AIR("name: %s", args->name);
    airplane->enter();
}

static int delay_enter_airplane(const char *tag, int delay_ms)
{
    ota_manager_context_t *ota_ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    if (ota_ctx->is_ota_upgrading()) {
        LOG_AIR("on ota upgrading, not allow!(tag:%s)", tag);
        return -1;
    }
    LOG_AIR("tag: %s, delay_ms: %d", tag, delay_ms);
    POST_TASK_HANDLER_DELAYED_ARG1(hdr_enter_ap_cb, delay_ms, thiz, "delay_enter");

    return 0;
}

static int exit_airplane_mode(int wakeup_reason)
{
    int wait_retry = 0;

    LOG_AIR("exit wakeup_reason: %d", wakeup_reason);
    thiz->priv.is_backgroud_check = false;
    while (1) {
        if (!thiz->priv.wm->is_current_window(ID_WINDOW_BEGIN) || wait_retry > 60) {
            break;
        }
        wait_retry++;
        LOG_AIR("wait_retry: %d", wait_retry);
        msleep(50);
    }
    /// 启用IMU唤醒
    rt_device_node_t *pmu_dev = thiz->priv.pbus->find_device(ID_DEVICE_PMU);
    if (pmu_dev && pmu_dev->ioctl) {
        int agrs[1] = {true};
        DEVICE_IOCTL_ARGS_NO_RET(pmu_dev, OP_CODE_PMU_SET_IMU_WAKEUP, agrs);
    }
    /// 取消被动退出回调
    REMOVE_TASK_HANDLER_DIRECTLY("hdr_airplane_mode_cb");

    /// 取消周期定时器
    thiz->priv.pbus->del_period_wakeup_cb();
    thiz->priv.pm->release_wakelock(AIRPLANE_PM_LOCK);
    msleep(150);

    thiz->set_airplane_mode(AIRPLANE_MODE_EXIT);
    play_notify(false);
    popup_airplane_screen(thiz->priv.wm, SETTINGS_AIRPLANE_MODE_EXIT);

    return 0;
}

static int is_on_airplane_mode(void)
{
    return thiz->priv.is_on;
}

static void set_airplane_mode(int state)
{
    thiz->priv.is_on = state;
    LOG_AIR("state: %d", state);
}

static void hdr_airplane_mode_cb(hdr_post_args_t *args)
{
    int is_on_ap = thiz->is_on_airplane_mode();

    thiz->priv.pm->release_wakelock(AIRPLANE_PM_LOCK);
    LOG_AIR("%s, is_on_ap: %d", args->name, is_on_ap);
    if (is_on_ap == AIRPLANE_MODE_EXIT) {
        return;
    }
    LOG_AIR("period wake up ...");

    /// 取消周期定时器
    thiz->priv.pbus->del_period_wakeup_cb();
    exit_airplane_mode(PM_WAKEUP_REASON_BACKGROUND_TASK);   
}

static void period_wakeup_event_cb(const char *reason, wakeup_type_e wakeup_type)
{
    LOG_AIR("airplane wakeup_reason: %d", wakeup_type);

    /// 停止周期检测定时器
    thiz->priv.pbus->del_period_wakeup_cb();
    if (wakeup_type == WAKEUP_TYPE_TIMING) {
        thiz->priv.pm->pm_wakeup(PM_WAKEUP_REASON_BACKGROUND_TASK);
        LOG_AIR("background check");
        /// 3分钟内没有震动，则退出飞行模式
        POST_TASK_HANDLER_DELAYED_ARG2(hdr_airplane_mode_cb, CHECK_TIME, 0, thiz, "hdr_airplane_mode_cb"); 
        thiz->priv.is_backgroud_check = true;
        thiz->priv.pm->acquire_wakelock(AIRPLANE_PM_LOCK, (CHECK_TIME /1000 + 10));
        /// 启用IMU唤醒
        rt_device_node_t *pmu_dev = thiz->priv.pbus->find_device(ID_DEVICE_PMU);
        if (pmu_dev && pmu_dev->ioctl) {
            int agrs[1] = {true};
            DEVICE_IOCTL_ARGS_NO_RET(pmu_dev, OP_CODE_PMU_SET_IMU_WAKEUP, agrs);
        }
    } else {
        thiz->priv.pm->release_wakelock(AIRPLANE_PM_LOCK);
    }
}

static void on_imu_irq_callback(void *user_data)
{
    static airplane_detect_state_e state = AIRPLANE_DETECT_STATE_INIT;

    if (thiz->priv.is_breath_pump_working) {
        LOG_AIR("@brush is working"); 
        state = AIRPLANE_DETECT_STATE_INIT;
        return;
    }
    
    if (thiz->priv.misc->is_charge_plug_in()) {
        state = AIRPLANE_DETECT_STATE_INIT;
        return;
    }

    if (thiz->priv.wm->is_current_window(ID_WINDOW_FACTORY_MODE)) {
        LOG_AIR("is in factory mode, ignore."); 
        state = AIRPLANE_DETECT_STATE_INIT;
        return;
    } 

    if (thiz->is_on_airplane_mode() == AIRPLANE_MODE_EXIT) {
        static uint32_t irq_time_last = 0;
        static uint32_t irq_time_now = 0;
        static uint32_t wakeup_time_first_time = 0;

        switch (state) {
            case AIRPLANE_DETECT_STATE_INIT:
                state = AIRPLANE_DETECT_STATE_FIRST_TIME_IRQ;
                break;

            case AIRPLANE_DETECT_STATE_FIRST_TIME_IRQ:
                wakeup_time_first_time = thiz->priv.time_sys->get_time();
                irq_time_last = thiz->priv.time_sys->get_time();
                LOG_AIR("first wakeup_time_first_time: %lus", wakeup_time_first_time);
                state = AIRPLANE_DETECT_STATE_CONTINUOUS_MOVING;
                break;

            case AIRPLANE_DETECT_STATE_CONTINUOUS_MOVING:
                irq_time_now = thiz->priv.time_sys->get_time();
                LOG_AIR("wakeup_time_now: %lus", irq_time_now);
                if (irq_time_now - irq_time_last < CONTINUOUS_MOVE_THRESHOLD) { //30S内算连续移动
                    if (irq_time_now - wakeup_time_first_time > CONTINUOUS_MOVE_MAX) { // 连续移动超过 n S
                        LOG_AIR("is moving, more than three minutes");
                        state = AIRPLANE_DETECT_STATE_FIRST_TIME_IRQ;
                        if (thiz->priv.pbus->add_period_wakeup_cb) {
                            thiz->priv.pbus->add_period_wakeup_cb("CONTINUOUS_MOVING", CHECK_PERIOD, period_wakeup_event_cb); 
                        }
                        enter_airplane_mode_internal(AIRPLANE_MODE_BY_IMU_PASSIVE_ENTER);
                    } else {
                        LOG_AIR("continuous_moving %lus", (irq_time_now - wakeup_time_first_time));
                    }
                } else {
                    state = AIRPLANE_DETECT_STATE_FIRST_TIME_IRQ;
                }
                irq_time_last = thiz->priv.time_sys->get_time();
                break;

            default:
                break;
        }
    } else if (thiz->priv.is_backgroud_check) {
        thiz->priv.is_backgroud_check = false;
        // 被动进入的，周期唤醒后，进入此处，则继续睡眠
        LOG_AIR("on_airplane_mode_but_imu_move, continue sleep");
        thiz->priv.pm->release_wakelock(AIRPLANE_PM_LOCK);
        // 禁用IMU唤醒
        rt_device_node_t *pmu_dev = thiz->priv.pbus->find_device(ID_DEVICE_PMU);
        if (pmu_dev && pmu_dev->ioctl) {
            int agrs[1] = {false};
            DEVICE_IOCTL_ARGS_NO_RET(pmu_dev, OP_CODE_PMU_SET_IMU_WAKEUP, agrs);
        }
        
        if (thiz->priv.pbus->add_period_wakeup_cb) {
            thiz->priv.pbus->add_period_wakeup_cb("is_backgroud_check", CHECK_PERIOD, period_wakeup_event_cb); 
        }
        thiz->set_airplane_mode(AIRPLANE_MODE_BY_IMU_PASSIVE_ENTER);
        thiz->priv.pm->enable_pmu_sleep("airplane sleep", true);
        SEND_BROADCAST_ARGS0(ID_BROADCAST_REQ_PM_SUSPEND_BY_USER);
    }
}

static void on_aiplane_mode_receive_internal(broadcast_mask_id_t id, void *context, int arg1, int arg2,
                                            broadcast_priv_data_t *pdata)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)context;

    if (id.code == ID_BROADCAST_BREATH_PUMP_STATE_CHANGE) {
        airplane->priv.is_breath_pump_working = arg1;
        LOG_AIR("is_brush_working: %d, brush_state: %d", arg1, arg2);
    }
}

static int on_post_airplane_init(void *ptr)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)ptr;
    input_dispatcher_context_t *input = (input_dispatcher_context_t *)find_module(ID_MODULE_INPUT);

    /// 广播监听泵状态改变.
    airplane->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    airplane->priv.bcd->context = airplane;
    airplane->priv.bcd->name = "aiplane_mode";
    airplane->priv.bcd->id.code |= ID_BROADCAST_BREATH_PUMP_STATE_CHANGE;
    airplane->priv.bcd->receiver_cb = on_aiplane_mode_receive_internal;
    register_broadcast_receiver(airplane->priv.bcd);
    sensor_input_context_t *sensor = input->get_sensor_obj();
    sensor->add_imu_irq_callback(IRQ_EVENT_CB_AIRPLANE, airplane, on_imu_irq_callback);
    return 0;
}

static int on_perpare_airplane_init(void *self)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)thiz;
    airplane->priv.wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    airplane->priv.pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    airplane->priv.pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    airplane->priv.misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    airplane->priv.time_sys = (time_system_t *)find_module(ID_MODULE_TIME);
    
    return 0;
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_airplane_mode_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, airplane_mode_context_t);

    ptr->enter = enter_airplane_mode;
    ptr->exit = exit_airplane_mode;
    ptr->is_on_airplane_mode = is_on_airplane_mode;
    ptr->set_airplane_mode = set_airplane_mode;
    ptr->delay_enter = delay_enter_airplane;
    thiz = ptr;

    return ADD_MODULE("ap_mode", ID_MODULE_AP_MODE, on_perpare_airplane_init, 
        on_post_airplane_init, ptr, list_head, sizeof(airplane_mode_context_t));
}

