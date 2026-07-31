/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            home
#define PAGE_NAME_STR                               "home"
/** 含 L+R 组合键（KEY_CODE_FACTORY_RESET）以进入双边力度调节 */
#define HOME_UI_KEY_CODE                            (ALL_KEY_CODE_NO_COMBINE | KEY_CODE_FACTORY_RESET)
#define HOME_UI_KEY_TYPE                            (KEY_EVENT_CLICK | KEY_EVENT_UP | KEY_EVENT_LONG_2S)
#define HOME_BROADCAST_TASK_NAME                    "home_broadcast_task"
#define POWER_OFF_HOLD_COMPLETE_TASK_NAME           "power_off_hold_complete"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_BROADCAST_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_OFF_HOLD_COMPLETE_TASK_NAME); \
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
static void start_pump_handler(ui_proc_home_t *proc)
{
    uint8_t battery_state = BATTERY_STATE_NONE;

    if (proc->battery) {
        battery_state = proc->battery->get_battery_state();
    }

    if (battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        battery_state == BATTERY_STATE_EMPTY ||
        battery_state == BATTERY_STATE_FORBID_START) {
        LOG_WM("battery state is low, ignore key event.");
        return;
    }

    if (proc->wm) {
        multi_func_msg_data_t msg = {0};
        LOG_WM("goto pump window.");
        msg.union_body.extra.window_id = ID_WINDOW_PUMP;
        msg.union_body.extra.arg1 = KEY_CODE_START;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
    }
}

static void home_start_key_event_handler(ui_proc_home_t *proc)
{
    start_pump_handler(proc);
}

static void home_knob_left_event_cb(ui_proc_home_t *proc)
{
#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->current_ui_type == HOME_UI_TYPE_NIGHT_LIGHT_ADJUST) {
        switch_home_night_light_adjust(proc, false);
        return;
    }
#endif
    if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST) {
        home_pump_level_adjust_handler(proc, false);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
        home_pump_both_level_adjust_handler(proc, false);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_SPEED_ADJUST) {
        home_pump_speed_adjust_handler(proc, false);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_HOME) {
        home_mode_adjust_handler(proc, true);
    }
}

static void home_knob_right_event_cb(ui_proc_home_t *proc)
{
#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->current_ui_type == HOME_UI_TYPE_NIGHT_LIGHT_ADJUST) {
        switch_home_night_light_adjust(proc, true);
        return;
    }
#endif

    if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST) {
        home_pump_level_adjust_handler(proc, true);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
        home_pump_both_level_adjust_handler(proc, true);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_SPEED_ADJUST) {
        home_pump_speed_adjust_handler(proc, true);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_HOME) {
        home_mode_adjust_handler(proc, false);
    }
}

static void home_mode_key_event_cb(ui_proc_home_t *proc, uint8_t mode_index)
{
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
        start_home_ui_animation(proc);
    }
    home_mode_set_handler(proc, mode_index);
}

static void home_power_key_event_cb(ui_proc_home_t *proc)
{
    if (proc->power_off_hold_ignore_click) {
        proc->power_off_hold_ignore_click = 0;
        LOG_WM("ignore power click after power off hold.");
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST) {
        home_level_adjust_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
        home_both_level_adjust_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_SPEED_ADJUST) {
        home_speed_adjust_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_HOME) {
        if (proc->ui_mode_page_index > HOME_MODE_PAGE_INDEX_NONE) {
            home_mode_confirm_handler(proc);
            show_home_settings_success_ui(proc);
        } else {
            start_pump_handler(proc);
        }
        return;
    }

#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->current_ui_type == HOME_UI_TYPE_HOME) {
        uint8_t battery_state = proc->battery->get_battery_state();
        if (battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
            battery_state == BATTERY_STATE_EMPTY ||
            battery_state == BATTERY_STATE_FORBID_START) {
            LOG_WM("battery state is low, ignore power key event.");
            return;
        }
        
#if IS_SUPPORT_DEVICE_NIGHT_LIGHT_TOGGLE
        if (proc->led && proc->led->hook && proc->led->hook->toggle_night_light) {
            proc->led->hook->toggle_night_light(proc->led);
            show_home_night_light_adjust_ui(proc);
        }
        return;
#endif
    }
#endif
    LOG_WM("ignore key event.");
}

static void home_click_event_cb(ui_proc_home_t *proc, uint16_t keycode)
{
    switch (keycode) {
        case KEY_CODE_STIMULATION_MODE:
            home_mode_key_event_cb(proc, BREAST_PUMP_STIMULATION_MODE);
            break;
        case KEY_CODE_LACTATION_MODE:
            home_mode_key_event_cb(proc, BREAST_PUMP_LACTATION_MODE);
            break;
        case KEY_CODE_POWER:
            home_power_key_event_cb(proc);
            break;
        case KEY_CODE_FREQUENCY:
            show_home_speed_adjust_ui(proc);
            break;
        case KEY_CODE_START:
            home_start_key_event_handler(proc);
            break;
        case KEY_CODE_KNOB_LEFT:
            home_knob_left_event_cb(proc);
            break;
        case KEY_CODE_KNOB_RIGHT:
            home_knob_right_event_cb(proc);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            if (proc->current_ui_type != HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
                show_home_level_adjust_ui(proc, false);
            }
            break;
        case KEY_CODE_LEFT_CONTROL:
            if (proc->current_ui_type != HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
                show_home_level_adjust_ui(proc, true);
            }
            break;
        case KEY_CODE_FACTORY_RESET:
            /* 同时短按 L+R → 双边力度调节 */
            show_home_both_level_adjust_ui(proc);
            break;
        default:
            break;
    }
}

static int on_home_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_home_t *proc = user_data;
    WINDOW_CURRENT_ID_GUARD_INT(proc, ID_WINDOW_HOME, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    if (proc->current_ui_type == HOME_UI_TYPE_HOME_IDLE) {
        LOG_WM("system idle, ignore key event.");
        return 0;
    }

    if (keytype == KEY_EVENT_LONG_2S && keycode == KEY_CODE_POWER) {
        home_power_off_hold_key_long_2s_handler(proc);
        return 0;
    }
    if (keytype == KEY_EVENT_UP && keycode == KEY_CODE_POWER) {
        home_power_off_hold_key_up_handler(proc);
        return 0;
    }
    if (keytype != KEY_EVENT_CLICK) {
        return 0;
    }

    /* 泵奶完成页 / 设置成功页仅限制 start，其它按键正常响应 */
    if ((proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT ||
         proc->current_ui_type == HOME_UI_TYPE_SETTINGS_SUCCESS) &&
        keycode == KEY_CODE_START) {
        LOG_WM("result/success ui, ignore start key.");
        return 0;
    }

    /* 关机确认页期间忽略其它 click，避免误触 */
    if (proc->current_ui_type == HOME_UI_TYPE_POWER_OFF_HOLD) {
        return 0;
    }

    home_click_event_cb(proc, keycode);
    return 0;
}

static void home_broadcast_receive_ui_cb(hdr_post_args_t *args)
{
    window_broadcast_args_t *bcd_args = (window_broadcast_args_t *)args->user_data;
    EXCEPTION_CHECK_VOID(bcd_args);
    ui_proc_home_t *proc = (ui_proc_home_t *)bcd_args->context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    switch (bcd_args->code) {
        case ID_BROADCAST_PM_STATE_CHANGE: 
            LOG_WM("pm state: %d", bcd_args->arg1);
            if (bcd_args->arg1 == PM_BROADCAST_SYS_IDLE) {
                start_home_idle_ui_animation(proc);
            } else if (bcd_args->arg1 == PM_BROADCAST_SYS_IDLE_RELEASE) {
                proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
                start_home_ui_animation(proc);
            } 
            break;
        case ID_BROADCAST_SYS_STATE_CHANGE:
            LOG_WM("system state: %d, %d", bcd_args->arg1, bcd_args->arg2);
            if (bcd_args->arg1 == SYSTEM_BROADCAST_UTC_TIME_CHANGE) {
                
            }
            break;
        default:
            break;
   }
}

static void on_home_ui_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
    broadcast_priv_data_t *pdata)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_HOME);
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
    POST_WM_TASK_HANDLER_NO_DELAY(home_broadcast_receive_ui_cb, &proc->broadcast_args, HOME_BROADCAST_TASK_NAME);
}

static void on_home_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    
    if (info->charge_status == CHARGE_STATE_ERROR && proc->current_ui_type != HOME_UI_TYPE_PUMP_RESULT) {
        LOG_WM("charge status error, goto charge screen.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (info->battery_state == BATTERY_STATE_EMPTY || 
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN || 
        info->battery_state == BATTERY_STATE_FORBID_START) {
    } else {

    }
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_home_t *proc = CONTAINER_OF(thiz, ui_proc_home_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        ADD_KEYEVENT_CB(proc->key, "home_wm", proc, HOME_UI_KEY_CODE, HOME_UI_KEY_TYPE,
            on_home_ui_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_home_window_battery_state_change_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
        }

        if (proc->extra.arg1 == HOME_SUB_UI_TYPE_PUMP_RESULT) {
            show_home_pump_result_ui(proc);
            return 0;
        }
        proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
        start_home_ui_animation(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_home_t *proc = CONTAINER_OF(thiz, ui_proc_home_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_home_common_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
            unregister_broadcast_receiver(&proc->receiver);
            REMOVE_KEYEVENT_CB(proc->key, on_home_ui_keyevent_hdr);
            REMOVE_BATTERY_STATE_CB(proc->battery, on_home_window_battery_state_change_cb);
            u_free(proc);
            proc = NULL;
            LOG_WM("%s window exit--end", PAGE_NAME_STR);
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
    ui_proc_home_t *proc = u_malloz(sizeof(ui_proc_home_t));
    SET_WINDOW_CONTEXT(proc);
    window_entry_extra_t *extra = (window_entry_extra_t *)wm->get_window_entry_extra(wm);
    memcpy(&proc->extra, extra, sizeof(window_entry_extra_t));
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "home_screen";
    proc->receiver.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    proc->receiver.receiver_cb = on_home_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    wm->dispatch_new_window(w_ctx);
    
    return ID_WINDOW_DISPLAY_OK;
}

