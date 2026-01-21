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
#define PAGE_STATIC_NAME                        charge
#define PAGE_NAME_STR                           "charge"
#define EXIT_CHARGE_POLICY_TASK_NAME            "exit_charge_policy"
#define CHARGE_SCREEN_TIMEOUT_DETECT_TASK_NAME  "charge_screen_timeout_detect"
#define CHARGE_SCREEN_ON_TASK_NAME              "charge_screen_on"

#define CHARGE_UI_KEY_CODE                       (KEY_CODE_POWER |KEY_CODE_START | KEY_CODE_MODE | KEY_CODE_FREQUENCY | \
                                                  KEY_CODE_BREASTFEEDING | KEY_CODE_LEFT_CONTROL | KEY_CODE_RIGHT_CONTROL | \
                                                  KEY_CODE_KNOB_LEFT | KEY_CODE_KNOB_RIGHT)
#define CHARGE_UI_KEY_TYPE                       (KEY_EVENT_CLICK|KEY_EVENT_LONG_3S)

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(EXIT_CHARGE_POLICY_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(CHARGE_SCREEN_TIMEOUT_DETECT_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(CHARGE_SCREEN_ON_TASK_NAME); \
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
static void charge_ui_common_cb(hdr_post_args_t *args)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }
    ui_proc_charge_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    LOG_WM("charge_ui_common_cb, margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    switch (action) {
        case CHARGE_HDR_EXIT_CHARGE_POLICY: {
            LOG_WM("%s", args->name);
            EXCEPTION_CHECK_VOID(proc->wm);
            multi_func_msg_data_t msg = {0};
            LOG_WM("goto pump window.");
            msg.union_body.extra.window_id = ID_WINDOW_PUMP;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
            proc->goto_pump_window_flag = 0;
        }
        break;
        case CHARGE_HDR_SCREEN_TIMEOUT_DETECT: {
            LOG_WM("%s", args->name);
        }
        break;
        case CHARGE_HDR_CHARGE_SCREEN_ON: {
            LOG_WM("%s", args->name);
        }
        break;

        default:
            break;
    }
}

static void on_charge_window_receive(broadcast_mask_id_t id, void *context, int arg1, int arg2,
                                     broadcast_priv_data_t *pdata)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }
    ui_proc_charge_t *proc = (ui_proc_charge_t *)context;
    EXCEPTION_CHECK_VOID(proc);
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE: 
            break;
        case ID_BROADCAST_CHARGE_STATE_CHANGE:
            if (arg1 != CHARGE_STATE_CHARGING && arg1 != CHARGE_STATE_FULL) {
                proc->goto_pump_window_flag = 1;
                POST_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                    CHARGE_HDR_EXIT_CHARGE_POLICY, EXIT_CHARGE_POLICY_TASK_NAME);
            } else {
                if (proc->goto_pump_window_flag == 1) {
                    proc->goto_pump_window_flag = 0;
                    REMOVE_TASK_HANDLER_DIRECTLY(EXIT_CHARGE_POLICY_TASK_NAME);
                }
            }
            break;      
        default:
            break;
    }
}

static int on_charge_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return 0;
    }

    ui_proc_charge_t *proc = user_data;
    if (keytype == KEY_EVENT_LONG_3S) {
        if (keycode == KEY_CODE_POWER) {
            proc->goto_pump_window_flag = 1;
            POST_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                CHARGE_HDR_EXIT_CHARGE_POLICY, EXIT_CHARGE_POLICY_TASK_NAME);
        }
    } else if (keytype == KEY_EVENT_CLICK) {
        if(keycode == KEY_CODE_POWER) {
            switch_charge_night_light_onoff_ui(proc);
        }
        proc->charge_screen_on_time = get_timestamp();
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
        if (proc->key) {
            proc->key->add_keyevent_handler("charge_wm", proc, CHARGE_UI_KEY_CODE, CHARGE_UI_KEY_TYPE,
                on_charge_keyevent_hdr);
        }
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        show_charging_ui(proc);
        proc->charge_screen_on_time = get_timestamp();
        // POST_TASK_HANDLER_DELAYED_ARG2(charge_screen_off_policy, proc, 1000, 0, CHARGE_SCREEN_OFF_TASK_NAME);
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
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_charging_ui(proc);
            unregister_broadcast_receiver(&proc->receiver);
            REMOVE_ALL_HANDER_TASK();
            if (proc->key) {
                proc->key->remove_keyevent_handler(on_charge_keyevent_hdr);
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
    ui_proc_charge_t *proc = u_malloz(sizeof(ui_proc_charge_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = CHARGE_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    proc->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    /// 监听PM状态改变.
    proc->receiver.id.code = ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.id.code |= ID_BROADCAST_CHARGE_STATE_CHANGE;
    proc->receiver.context = proc;
    proc->receiver.name = "charge_screen";
    proc->receiver.receiver_cb = on_charge_window_receive;
    register_broadcast_receiver(&proc->receiver);
    /// 配置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->user_data = NULL;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}



