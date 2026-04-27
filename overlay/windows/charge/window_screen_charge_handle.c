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
#define EXIT_CHARGE_ADAPTER_MISMATCH_TASK_NAME  "exit_charge_adapter_mismatch"
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
    REMOVE_TASK_HANDLER_DIRECTLY(EXIT_CHARGE_ADAPTER_MISMATCH_TASK_NAME); \
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
    ui_proc_charge_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_CHARGE);
    LOG_WM("margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    switch (action) {
        case CHARGE_HDR_EXIT_CHARGE_POLICY:
            LOG_WM("%s", args->name);
            EXCEPTION_CHECK_VOID(proc->wm);
            multi_func_msg_data_t msg = {0};
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
            proc->exit_charge_window_flag = 0;
            break;
        case CHARGE_HDR_SCREEN_TIMEOUT_DETECT:
            LOG_WM("%s", args->name);
            break;
        case CHARGE_HDR_CHARGE_SCREEN_ON:
            LOG_WM("%s", args->name);
            break;
        default:
            break;
    }
}

static void on_charge_window_receive(broadcast_mask_id_t id, void *context, int arg1, int arg2,
                                     broadcast_priv_data_t *pdata)
{
    ui_proc_charge_t *proc = (ui_proc_charge_t *)context;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_CHARGE);
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE: 
            break;
        case ID_BROADCAST_BATTERY_STATE_CHANGE:
            if (arg1 == BATTERY_BROADCAST_CHARGE_STATE_CHANGE) {
                if (BATTERY_IS_CHARGING(arg2) == 0) {
                    proc->exit_charge_window_flag = 1;
                    LOG_WM("post exit charge policy task");
                    POST_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                        CHARGE_HDR_EXIT_CHARGE_POLICY, EXIT_CHARGE_POLICY_TASK_NAME);
                }
            }  else if (arg1 == BATTERY_BROADCAST_CHARGE_ADAPTER_MISMATCH) {
                LOG_WM("show charge adapter ui");
                show_charge_adapter_ui(proc);
            } else if (arg1 == BATTERY_BROADCAST_CHARGE_ADAPTER_MISMATCH_RECOVER) {
                proc->exit_charge_window_flag = 1;
                LOG_WM("post exit charge adapter mismatch task");
                POST_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                    CHARGE_HDR_EXIT_CHARGE_POLICY, EXIT_CHARGE_ADAPTER_MISMATCH_TASK_NAME);
            } else if (arg1 == BATTERY_BROADCAST_CHARGE_PLUG_IN) {
                if (proc->exit_charge_window_flag == 1) {
                    proc->exit_charge_window_flag = 0;
                    LOG_WM("remove exit charge policy task");
                    REMOVE_TASK_HANDLER_DIRECTLY(EXIT_CHARGE_POLICY_TASK_NAME);
                    REMOVE_TASK_HANDLER_DIRECTLY(EXIT_CHARGE_ADAPTER_MISMATCH_TASK_NAME);
                }
            } else {
                LOG_WM("charge window receive broadcast: %d, %d", arg1, arg2);
            }
            break;      
        default:
            break;
    }
}

static int on_charge_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_charge_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_CHARGE, 0);
    if (keytype == KEY_EVENT_LONG_3S) {
        if (keycode == KEY_CODE_POWER && proc->battery->is_adapter_mismatch() == 0) {
            LOG_WM("key post exit charge policy task");
            proc->exit_charge_window_flag = 1;
            POST_TASK_HANDLER_NO_DELAY_ARG1(charge_ui_common_cb, proc, 
                CHARGE_HDR_EXIT_CHARGE_POLICY, EXIT_CHARGE_POLICY_TASK_NAME);
        }
    } else if (keytype == KEY_EVENT_CLICK) {
        if (keycode == KEY_CODE_POWER) {
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
                on_charge_keyevent_hdr, KEY_HANDLER_SHIELD_LEVEL_1);
        } else {
            LOG_WM("key not present, ignore.");
        }
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (proc->led && proc->led->hook) {
            /*
             * 插 USB 的“伪关机/充电界面”必须强制灭灯并清 BLINK 状态机，
             * 不能仅依赖亮度置零。
             */
            if (proc->led->set_led_onoff) {
                proc->led->set_led_onoff(PUMP_LED_ALL_ID, LED_STATUS_OFF);
            } else if (proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, 0);
            }
            
            if (proc->led->hook->set_night_light_level) {   
                proc->led->hook->set_night_light_level(proc->led, 0);
            }
        }

        if (proc->battery->is_adapter_mismatch()) {
            show_charge_adapter_ui(proc);
        } else {
            show_charging_ui(proc);
        }
        
        proc->charge_screen_on_time = get_timestamp();
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
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_charge_common_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            unregister_broadcast_receiver(&proc->receiver);
            if (proc->key) {
                proc->key->remove_keyevent_handler(on_charge_keyevent_hdr);
            } else {
                LOG_WM("key not present, ignore.");
            }
            if (proc->pm) {
                proc->pm->release_wakelock("charge_screen");
            } else {
                LOG_WM("pm not present, ignore.");
            }
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
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    /// 监听PM状态改变.
    proc->receiver.id.code = ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.id.code |= ID_BROADCAST_BATTERY_STATE_CHANGE;
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
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = NULL;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}



