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
#define CHARGE_UI_KEY_TYPE                       (KEY_EVENT_LONG_3S | KEY_EVENT_LONG_10S)
#define CHARGE_PAGE_POLICY_TASK_NAME             "charge_page_policy"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(CHARGE_PAGE_POLICY_TASK_NAME);\
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
static void charge_display_exit_cb(ui_proc_charge_t *proc)
{
    multi_func_msg_data_t msg = {0};
    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->airplane && proc->airplane->is_on_airplane_mode()) {
        LOG_WM("goto airplane mode screen");
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->wm->is_window_guidance_deployed() == 0) {
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }    
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
            if (proc->led->hook->set_night_light_level) {   
                proc->led->hook->set_night_light_level(proc->led, 0);
            }
        #endif
        }

        if (proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
            show_charge_adapter_ui(proc);
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
            if (proc->ble && proc->ble->ble_is_connected() == 0 &&
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
    /// 配置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = NULL;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}



