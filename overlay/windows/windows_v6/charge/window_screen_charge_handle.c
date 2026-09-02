/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                         charge
#define PAGE_NAME_STR                            "charge"
#define CHARGE_UI_KEY_CODE                       (KEY_CODE_POWER)
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define CHARGE_UI_KEY_TYPE                       (KEY_EVENT_CLICK | KEY_EVENT_LONG_2S | KEY_EVENT_LONG_3S | KEY_EVENT_LONG_10S)
#else
#define CHARGE_UI_KEY_TYPE                       (KEY_EVENT_LONG_2S | KEY_EVENT_LONG_3S | KEY_EVENT_LONG_10S)
#endif
#define CHARGE_PAGE_POLICY_TASK_NAME             "charge_page_policy"
#define CHARGE_BLE_TEARDOWN_TASK_NAME            "charge_ble_teardown"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(CHARGE_PAGE_POLICY_TASK_NAME);\
    REMOVE_TASK_HANDLER_DIRECTLY(CHARGE_BLE_TEARDOWN_TASK_NAME);\
} while (0)

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
static int charge_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_charge_t *proc = (ui_proc_charge_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_CHARGE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}

static void charge_display_exit_cb(ui_proc_charge_t *proc)
{
    multi_func_msg_data_t msg = {0};
    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->led && proc->led->hook &&
        proc->led->hook->get_night_light_startup_switch &&
        proc->led->hook->get_night_light_startup_switch(proc->led) &&
        proc->led->hook->set_night_light_onoff) {
        proc->led->hook->set_night_light_onoff(proc->led, true);
        if (proc->led->hook->set_night_light_adaptive_dimming_switch) {
            proc->led->hook->set_night_light_adaptive_dimming_switch(proc->led, 1);
        }
    }
#endif

    if (proc->airplane && proc->airplane->is_on_airplane_mode()) {
        LOG_WM("goto airplane mode screen");
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

#if IS_SUPPORT_GUIDE_WINDOW
    if (is_need_show_guide_window(proc->wm)) {
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#else
    if (proc->wm->is_window_guidance_deployed() == 0) {
        proc->wm->set_window_guidance_deployed_flag(true);
    }
#endif

    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void charge_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_charge_t *proc = args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_CHARGE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    LOG_WM("margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    multi_func_msg_data_t msg = {0};
    switch (action) {
        case CHARGE_HDR_EXIT_CHARGE_POLICY:
            LOG_WM("%s", args->name);
            charge_display_exit_cb(proc);
            break;
        default:
            break;
    }
}

static void charge_ble_teardown_cb(hdr_post_args_t *args)
{
    ui_proc_charge_t *proc = args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_CHARGE);
    EXCEPTION_CHECK_VOID(proc->ble);
    if (proc->margic != UI_WINDOW_MARGIC_CODE) {
        LOG_WM("invalid margic, ignore");
        return;
    }
    LOG_WM("%s", args->name);
    /// 适配器不匹配：延时后断开，确保上报事件完成；不开广播
    if (proc->ble->ble_is_connected() > 0) {
        proc->ble->set_adv_speed(BLE_OPT_ADV_STOP);
        proc->ble->disconnect_bt_by_user();
    } else {
        proc->ble->bt_adv_stop_by_user();
    }
}

/// 适配器不匹配：不开广播；已连接则延时1s断开，便于事件上报
static void charge_ble_adapter_mismatch_policy(ui_proc_charge_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->ble);

    if (proc->wm && proc->wm->is_window_factory_test_deployed() == 0) {
        LOG_WM("factory test mode, ignore adapter mismatch");
        return;
    }

    if (proc->ble->ble_is_connected() > 0) {
        LOG_WM("adapter mismatch, delay 1s disconnect ble");
        POST_WM_TASK_HANDLER_DELAYED_ARG1(charge_ble_teardown_cb, 1000, proc,
            CHARGE_BLE_TEARDOWN_TASK_NAME);
    } else {
        LOG_WM("adapter mismatch, stop ble adv");
        proc->ble->bt_adv_stop_by_user();
    }
}

static void on_charge_window_battery_state_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_charge_t *proc = (ui_proc_charge_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_CHARGE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }

    if (info && info->charge_status == CHARGE_STATE_ERROR) {
        show_charge_adapter_ui(proc);
        charge_ble_adapter_mismatch_policy(proc);
    } else {
        POST_WM_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                CHARGE_HDR_EXIT_CHARGE_POLICY, CHARGE_PAGE_POLICY_TASK_NAME);
    } 
}

static int on_charge_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_charge_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_CHARGE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    if (keycode != KEY_CODE_POWER) {
        return 0;
    }

#if IS_SUPPORT_POWER_ON_GUIDE_UI
    /// 适配器不匹配页
    if (proc->current_ui_type == CHARGE_UI_TYPE_ADAPTER ||
        (proc->battery && proc->battery->get_charge_status() == CHARGE_STATE_ERROR)) {
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
        if (keytype == KEY_EVENT_CLICK) {
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_DUMMY;
            LOG_WM("adapter mismatch short press, goto power on guide");
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
            return 0;
        }
#endif
        if (keytype == KEY_EVENT_LONG_2S && 
            proc->wm && proc->wm->is_window_factory_test_deployed() == 0) {
            LOG_WM("factory test mode, goto logo screen");
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_LOGO;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
        } else if (keytype == KEY_EVENT_LONG_10S) {
            LOG_WM("charge page reboot");
            msleep(20);
            platform_system_reboot();
        } else {
            LOG_WM("adapter mismatch, ignore power key");
        }
        return 0;
    }
#endif
    if (keytype == KEY_EVENT_LONG_3S) {
        if (proc->battery && proc->battery->get_charge_status() != CHARGE_STATE_ERROR) {
            LOG_WM("key post exit charge policy task");
            POST_WM_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                CHARGE_HDR_EXIT_CHARGE_POLICY, CHARGE_PAGE_POLICY_TASK_NAME);
        }
    } else if (keytype == KEY_EVENT_LONG_10S) { 
        LOG_WM("charge page reboot");
        msleep(20);
        platform_system_reboot();
    }
    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_charge_t *proc = CONTAINER_OF(thiz, ui_proc_charge_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        ADD_KEYEVENT_CB(proc->key, "charge_wm", proc, CHARGE_UI_KEY_CODE, CHARGE_UI_KEY_TYPE,
            on_charge_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_charge_window_battery_state_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (proc->led && proc->led->hook) {
            if (proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, 0);
            }

#if IS_SUPPORT_NIGHT_LIGHT
            if (proc->led && proc->led->hook) {
                if (proc->led->hook->set_night_light_adaptive_dimming_switch) {
                    proc->led->hook->set_night_light_adaptive_dimming_switch(proc->led, 0);
                }
                
                if (proc->led->hook->set_night_light_onoff) {
                    proc->led->hook->set_night_light_onoff(proc->led, false);
                }
            }
#endif
        }

        if (proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
            show_charge_adapter_ui(proc);
            charge_ble_adapter_mismatch_policy(proc);
        } else {
            POST_WM_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                CHARGE_HDR_EXIT_CHARGE_POLICY, CHARGE_PAGE_POLICY_TASK_NAME);
        }

        if (proc->pm) {
            proc->pm->acquire_wakelock("charge_screen", -1); /// 永久持有锁.
        }
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_charge_t *proc = CONTAINER_OF(thiz, ui_proc_charge_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE: 
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_charge_common_ui(proc);
            /// 离开充电页前取消延时拆除，避免退出后再次关广播
            REMOVE_TASK_HANDLER_DIRECTLY(CHARGE_BLE_TEARDOWN_TASK_NAME);
            if (proc->ble && proc->ble->get_adv_speed() == BLE_OPT_ADV_STOP) {
                if (proc->ble->ble_is_connected() == 0) {
                    LOG_WM("ble adv speed stop, restart adv");
                    proc->ble->bt_adv_start_by_user();
                } else {
                    /// 断连进行中：清 STOP，让 disconnect_suc 可重开广播
                    LOG_WM("ble disconnecting with adv stop, clear stop flag");
                    proc->ble->set_adv_speed(BLE_OPT_ADV_FAST);
                }
            } else if (proc->ble && proc->ble->ble_is_connected() == 0 &&
                       proc->ble->get_adv_state() == 0) {
                proc->ble->bt_adv_start_by_user();
            }
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
            REMOVE_BATTERY_STATE_CB(proc->battery, on_charge_window_battery_state_cb);
            REMOVE_KEYEVENT_CB(proc->key, on_charge_keyevent_hdr);
            if (proc->pm) {
                proc->pm->release_wakelock("charge_screen");
            }
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
    ui_proc_charge_t *proc = u_malloz(sizeof(ui_proc_charge_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = CHARGE_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    /// 配置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = charge_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}



