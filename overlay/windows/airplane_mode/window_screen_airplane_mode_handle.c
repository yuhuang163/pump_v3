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
#define AIRPLANE_MODE_UI_KEY_TYPE               (KEY_EVENT_LONG_5S)
#define AIRPLANE_MODE_UI_KEY_DOWN_TYPE          (KEY_EVENT_DOWN)
#define AIRPLANE_MODE_UI_KEY_UP_TYPE            (KEY_EVENT_UP)
#define AIRPLANE_MODE_TIMEOUT_TASK_NAME         "airplane_mode_timeout_task"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(AIRPLANE_MODE_TIMEOUT_TASK_NAME); \
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
static void airplane_mode_timeout_cb(hdr_post_args_t *args)
{
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_AIRPLANE_MODE);
    LOG_WM("airplane mode timeout, goto pump window.");
    SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_OFF);
}

static void on_airplane_mode_window_receive(broadcast_mask_id_t id, void *context,
                                 int32_t pm_state, int32_t wakeup_reason, broadcast_priv_data_t *pdata)
{
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)context;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_AIRPLANE_MODE);
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE: 
            break;
        case ID_BROADCAST_BATTERY_STATE_CHANGE: 
            break;
        default:
            break;
    }
}

static int on_airplane_mode_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_airplane_mode_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_AIRPLANE_MODE, 0);
    if (keytype == AIRPLANE_MODE_UI_KEY_DOWN_TYPE) {
        proc->popup_ui_type = AIRPLANE_MODE_POPUP_UI_EXIT;
    } else if (keytype == AIRPLANE_MODE_UI_KEY_UP_TYPE) {
        if (proc->popup_ui_type != AIRPLANE_MODE_POPUP_UI_NONE) {
            proc->popup_ui_type = AIRPLANE_MODE_POPUP_UI_NONE;
        }
    } else {
        LOG_WM("airplane mode key event type not supported: %d", keytype);
    }
    return 0;
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
        if (proc->key) {
            proc->key->add_keyevent_handler("airplane_mode_wm", proc, AIRPLANE_MODE_UI_KEY_CODE, \
            AIRPLANE_MODE_UI_KEY_TYPE | AIRPLANE_MODE_UI_KEY_DOWN_TYPE | AIRPLANE_MODE_UI_KEY_UP_TYPE,
                 on_airplane_mode_keyevent_hdr, KEY_HANDLER_SHIELD_LEVEL_1);

        } else {
            LOG_WM("key not present, ignore.");
        }
        POST_TASK_HANDLER_DELAYED_ARG1(airplane_mode_timeout_cb, AIRPLANE_MODE_TIMEOUT_TIME, 
            proc, AIRPLANE_MODE_TIMEOUT_TASK_NAME);
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
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_airplane_mode_ui(proc);
            unregister_broadcast_receiver(&proc->receiver);
            airplane_mode_ui_hdr_remove();
            REMOVE_ALL_HANDER_TASK();
            if (proc->key) {
                proc->key->remove_keyevent_handler(on_airplane_mode_keyevent_hdr);
            }
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
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
    proc->show_countdown_time = 0;
    /// 监听PM状态改变.
    proc->receiver.id.code = ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.id.code |= ID_BROADCAST_BATTERY_STATE_CHANGE;
    proc->receiver.context = proc;
    proc->receiver.name = "airplane_mode_screen";
    proc->receiver.receiver_cb = on_airplane_mode_window_receive;
    register_broadcast_receiver(&proc->receiver);
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



