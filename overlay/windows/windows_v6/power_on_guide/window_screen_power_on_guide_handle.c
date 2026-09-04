/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h"
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_power_on_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                        power_on_guide
#define PAGE_NAME_STR                           "power_on_guide"
#define POWER_ON_GUIDE_BROADCAST_TASK_NAME      "power_on_guide_broadcast_task"
#define POWER_ON_GUIDE_POWER_OFF_TASK_NAME      "power_on_guide_power_off_task"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_ON_GUIDE_BROADCAST_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_ON_GUIDE_POWER_OFF_TASK_NAME); \
    IF_SUPPORT_POWER_ON_GUIDE_UI_HDR_REMOVE(); \
} while (0)

#if IS_SUPPORT_POWER_ON_GUIDE_UI
#define IF_SUPPORT_POWER_ON_GUIDE_UI_HDR_REMOVE() \
    power_on_guide_power_on_hold_ui_hdr_remove()
#define POWER_ON_GUIDE_HOLD_KEY_CODE            (KEY_CODE_POWER)
#define POWER_ON_GUIDE_HOLD_KEY_TYPE            (KEY_EVENT_DOWN | KEY_EVENT_UP)
#else
#define IF_SUPPORT_POWER_ON_GUIDE_UI_HDR_REMOVE()
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
INIT_WINDOW_CONTEXT();

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int power_on_guide_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_power_on_guide_t *proc = (ui_proc_power_on_guide_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_POWER_ON_GUIDE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}

static int wakup_enter_new_window_policy(ui_proc_power_on_guide_t *proc, int wakeup_reason)
{
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_POWER_ON_GUIDE, 0);
    tests_manager_context_t *tests = proc->tests;
    multi_func_msg_data_t msg = {0};
    if (tests && tests->tests_manager_get_current_mode() != TST_NONE_TYPE) {
        msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
        LOG_WM("goto product test mode window.");
    } else {
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        LOG_WM("goto home window.");
    }
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
    return 0;
}

static void power_on_guide_broadcast_receive_ui_cb(hdr_post_args_t *args)
{
    window_broadcast_args_t *bcd_args = (window_broadcast_args_t *)args->user_data;
    EXCEPTION_CHECK_VOID(bcd_args);
    ui_proc_power_on_guide_t *proc = (ui_proc_power_on_guide_t *)bcd_args->context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_POWER_ON_GUIDE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    switch (bcd_args->code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            LOG_WM("received PM_STATE_CHANGE.");
            if (bcd_args->arg1 != PM_BROADCAST_SYS_RESUME) {
                LOG_WM("ignore: %d\n", bcd_args->arg2);
                break;
            }
            LOG_WM("PM_RESUME, wakeup_reason: %d.", bcd_args->arg2);
            wakup_enter_new_window_policy(proc, bcd_args->arg2);
            break;
        default:
            break;
   }
}

static void on_power_on_guide_window_receive(broadcast_mask_id_t id, void *context,
                                 int32_t arg1, int32_t arg2, broadcast_priv_data_t *pdata)
{
    ui_proc_power_on_guide_t *proc = (ui_proc_power_on_guide_t *)context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_POWER_ON_GUIDE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    proc->broadcast_args.context = proc;
    proc->broadcast_args.code = id.code;
    proc->broadcast_args.arg1 = arg1;
    proc->broadcast_args.arg2 = arg2;
    if (pdata != NULL && pdata->size > 0) {
        memcpy(&proc->broadcast_args.priv_data, pdata, sizeof(broadcast_priv_data_t));
    } else {
        memset(&proc->broadcast_args.priv_data, 0, sizeof(proc->broadcast_args.priv_data));
    }
    POST_WM_TASK_HANDLER_NO_DELAY(power_on_guide_broadcast_receive_ui_cb, &proc->broadcast_args, POWER_ON_GUIDE_BROADCAST_TASK_NAME);
}

#if IS_SUPPORT_POWER_ON_GUIDE_UI
static void power_on_guide_hold_key_event_handler(ui_proc_power_on_guide_t *proc, uint16_t keytype)
{
    if (proc->current_ui_type != POWER_ON_GUIDE_UI_TYPE_POWER_ON_HOLD) {
        return;
    }
    if (keytype == KEY_EVENT_DOWN) {
        proc->power_on_hold_holding = 1;
        proc->last_progress_ani_time = get_timestamp();
        LOG_WM("power on hold key down.");
    } else if (keytype == KEY_EVENT_UP) {
        proc->power_on_hold_holding = 0;
        proc->last_progress_ani_time = get_timestamp();
        LOG_WM("power on hold key up, reset progress.");
    }
}

#if (ROOT_PRODUCT_ID != ROOT_PRODUCT_V3PRO_ID)
static void on_power_on_guide_window_battery_state_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_power_on_guide_t *proc = (ui_proc_power_on_guide_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_POWER_ON_GUIDE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    if (proc->current_ui_type != POWER_ON_GUIDE_UI_TYPE_POWER_ON_HOLD) {
        return;
    }
    if (info == NULL || info->charge_status != CHARGE_STATE_ERROR) {
        return;
    }
    if (proc->battery == NULL || proc->battery->is_vbus_plug_in() == 0) {
        return;
    }
    LOG_WM("adapter mismatch on power on guide, goto charge screen");
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}
#endif
#else
static void power_on_guide_power_off_task_handler(void *user_data)
{
    ui_proc_power_on_guide_t *proc = (ui_proc_power_on_guide_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_POWER_ON_GUIDE);
    LOGW("==========POWER_OFF==========");
    uint16_t sleep_time = 50;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *power_dev = pbus->find_device(ID_DEVICE_POWER);
    if (power_dev == NULL) {
        LOGE("power dev not found, reboot");
        msleep(50);
        platform_system_reboot();
    }
    rt_device_node_t *led_dev = pbus->find_device(ID_DEVICE_LED);
    if (led_dev) {
        led_status_msg_t msg = {
            .led_index = PUMP_LED_ALL_ID,
            .led_status = {
                .mode = LED_MODE_ONOFF,
                .un.onoff = {
                    .status = LED_STATUS_OFF,
                },
            },
        };
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        LOG_WM("set led off");
        sleep_time = 100;
    }
    msleep(sleep_time); /// 数据同步延时
    int arg1[] = {0};
    power_dev->ioctl(power_dev, OP_CODE_POWER_CONTROL, (void *)arg1);
}
#endif

static int on_power_on_guide_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_power_on_guide_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_POWER_ON_GUIDE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    LOG_WM("power_on_guide key event: %d, %d", keycode, keytype);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
    if (proc->current_ui_type == POWER_ON_GUIDE_UI_TYPE_POWER_ON_HOLD && keycode == KEY_CODE_POWER) {
        power_on_guide_hold_key_event_handler(proc, keytype);
        return 0;
    }
#endif
    if (keytype == KEY_EVENT_LONG_10S) {
        msleep(1000);
        platform_system_reboot();
    }
    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_power_on_guide_t *proc = CONTAINER_OF(thiz, ui_proc_power_on_guide_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_INT(ui);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
        ADD_KEYEVENT_CB(proc->key, "power_on_guide_wm", proc, POWER_ON_GUIDE_HOLD_KEY_CODE,
            POWER_ON_GUIDE_HOLD_KEY_TYPE | KEY_EVENT_LONG_10S, on_power_on_guide_keyevent_hdr);
#if (ROOT_PRODUCT_ID != ROOT_PRODUCT_V3PRO_ID)
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_power_on_guide_window_battery_state_cb);
#endif
#else
        ADD_KEYEVENT_CB(proc->key, "power_on_guide_wm", proc, (KEY_CODE_POWER), KEY_EVENT_LONG_10S,
            on_power_on_guide_keyevent_hdr);
#endif
        proc->enter_power_on_guide_time = platform_get_utc_time();
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
#if (ROOT_PRODUCT_ID != ROOT_PRODUCT_V3PRO_ID)
        /// 进引导前若已识别为适配器不匹配，直接进不匹配页
        if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
            proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
            LOG_WM("adapter mismatch on create, goto charge screen");
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
            return 0;
        }
#endif
        show_power_on_guide_power_on_hold_ui(proc);
#else
        POST_WM_TASK_HANDLER_NO_DELAY(power_on_guide_power_off_task_handler, proc, POWER_ON_GUIDE_POWER_OFF_TASK_NAME);
#endif
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_power_on_guide_t *proc = CONTAINER_OF(thiz, ui_proc_power_on_guide_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
            stop_power_on_guide_ui_animation(proc);
#endif
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            unregister_broadcast_receiver(&proc->receiver);
            REMOVE_KEYEVENT_CB(proc->key, on_power_on_guide_keyevent_hdr);
#if IS_SUPPORT_POWER_ON_GUIDE_UI && (ROOT_PRODUCT_ID != ROOT_PRODUCT_V3PRO_ID)
            REMOVE_BATTERY_STATE_CB(proc->battery, on_power_on_guide_window_battery_state_cb);
#endif
            u_free(proc);
            proc = NULL;
            LOG_WM("%s window on destory--end", PAGE_NAME_STR);
            break;
        default:
            break;
    }

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
window_enter_result_e PAGE_HANDLE_NAME(PAGE_STATIC_NAME)(window_manager_context_t *wm, uint8_t window_id)
{
    LOG_WM("%s window enter, time: %ld.", PAGE_NAME_STR, get_timestamp());
    ui_proc_power_on_guide_t *proc = u_malloz(sizeof(ui_proc_power_on_guide_t));
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = POWER_ON_GUIDE_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->tests = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
#if IS_SUPPORT_POWER_ON_GUIDE_UI
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
#endif
    /// 监听PM状态改变.
    proc->receiver.id.code = ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.context = proc;
    proc->receiver.name = "power_on_guide_screen";
    proc->receiver.receiver_cb = on_power_on_guide_window_receive;
    register_broadcast_receiver(&proc->receiver);
    /// 配置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = power_on_guide_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}
