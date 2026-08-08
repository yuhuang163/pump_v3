/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                        airplane_mode
#define PAGE_NAME_STR                           "airplane_mode"
#define AIRPLANE_MODE_UI_KEY_CODE               (KEY_CODE_TRAVEL_LOCK)
#define AIRPLANE_MODE_UI_KEY_TYPE               (KEY_EVENT_UP | KEY_EVENT_LONG_2S)

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
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
static int airplane_mode_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_AIRPLANE_MODE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}

static int on_airplane_mode_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_airplane_mode_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_AIRPLANE_MODE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    /// 按键操作刷新关机倒计时
    proc->power_off_idle_time = get_timestamp();
    if (keytype == KEY_EVENT_LONG_2S) {
        proc->popup_ui_type = ID_POPUP_UI_AIRPLANE_MODE_PROMPT;
    } else if (keytype == KEY_EVENT_UP) {
        if (proc->popup_ui_type != ID_POPUP_UI_NONE) {
            proc->popup_ui_type = ID_POPUP_UI_NONE;
        }
    } else {
        LOG_WM("airplane mode key event type not supported: %d", keytype);
    }
    return 0;
}

static void on_airplane_mode_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_AIRPLANE_MODE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }

    if (info->charge_status == CHARGE_STATE_ERROR) {
        LOG_WM("charge status error, goto charge screen.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
    }
}

static int entry_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_airplane_mode_t *proc = CONTAINER_OF(thiz, ui_proc_airplane_mode_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_INT(ui);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        ADD_KEYEVENT_CB(proc->key, "airplane_mode_wm", proc, AIRPLANE_MODE_UI_KEY_CODE,
            AIRPLANE_MODE_UI_KEY_TYPE, on_airplane_mode_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_airplane_mode_window_battery_state_change_cb);
        wm_ensure_ble_adv_started();
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        airplane_mode_led_policy(proc, 1);
        airplane_mode_pump_policy(proc);
        start_airplane_mode_ui(proc);
    }
    
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_airplane_mode_t *proc = CONTAINER_OF(thiz, ui_proc_airplane_mode_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE: 
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_airplane_mode_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
            airplane_mode_ui_hdr_remove();
            unregister_broadcast_receiver(&proc->receiver);
            REMOVE_KEYEVENT_CB(proc->key, on_airplane_mode_keyevent_hdr);
            REMOVE_BATTERY_STATE_CB(proc->battery, on_airplane_mode_window_battery_state_change_cb);
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
    ui_proc_airplane_mode_t *proc = u_malloz(sizeof(ui_proc_airplane_mode_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = AIRPLANE_MODE_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    proc->power_off_idle_time = 0;
    /// 配置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = airplane_mode_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}


