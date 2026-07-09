/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            home
#define PAGE_NAME_STR                               "home"
#define HOME_UI_KEY_CODE                            ALL_KEY_CODE_NO_COMBINE
#define HOME_UI_KEY_TYPE                            KEY_EVENT_CLICK
#define HOME_BROADCAST_TASK_NAME                    "home_broadcast_task"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_BROADCAST_TASK_NAME); \
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
static void home_start_key_event_handler(ui_proc_home_t *proc)
{
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_HOME);
    EXCEPTION_CHECK_VOID(proc->battery);
    uint8_t battery_state = proc->battery->get_battery_state();
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

static void home_knob_left_event_cb(ui_proc_home_t *proc)
{
#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->current_ui_type == HOME_UI_TYPE_NIGHT_LIGHT_ADJUST) {
        switch_home_night_light_adjust(proc, false);
        return;
    }
#endif
    if (proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT) {
        start_home_ui_animation(proc);
    }
    home_pump_level_adjust_handler(proc, false);
}

static void home_knob_right_event_cb(ui_proc_home_t *proc)
{
#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->current_ui_type == HOME_UI_TYPE_NIGHT_LIGHT_ADJUST) {
        switch_home_night_light_adjust(proc, true);
        return;
    }
#endif

    if (proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT) {
        start_home_ui_animation(proc);
    }
    home_pump_level_adjust_handler(proc, true);
}

static void home_power_key_event_cb(ui_proc_home_t *proc)
{
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

    if (proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT) {
        start_home_ui_animation(proc);
        return;
    }
    LOG_WM("ignore key event.");
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

    if (keycode == KEY_CODE_CANCEL) {
        home_cancel_key_event_handler(proc);
        return 0;
    }
    
#if 1
    if (proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT) {
        LOG_WM("pump result, ignore key event.");
        return 0;
    }
#endif

    switch (keycode) {
        case KEY_CODE_MODE:
            home_pump_mode_adjust_handler(proc, false);
            break;
        case KEY_CODE_BREASTFEEDING:
            #if IS_SUPPORT_PUMP_RHYTHM_MODE_SELECT
            home_pump_mode_adjust_handler(proc, true);
            #endif
            break;
        case KEY_CODE_POWER:
            home_power_key_event_cb(proc);
            break;
        case KEY_CODE_START:
            if (proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT) {
                LOG_WM("pump result, ignore key event.");
            } else {
                home_start_key_event_handler(proc);
            }
            break;
        case KEY_CODE_KNOB_LEFT:
            home_knob_left_event_cb(proc);
            break;
        case KEY_CODE_KNOB_RIGHT:
            home_knob_right_event_cb(proc);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            home_pump_level_adjust_type_update(proc, false);
            break;
        case KEY_CODE_LEFT_CONTROL:
            home_pump_level_adjust_type_update(proc, true);
            break;
        default:
            break;
    }
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
                start_home_ui_animation(proc);
            } 
            break;
        case ID_BROADCAST_SYS_STATE_CHANGE:
            LOG_WM("system state: %d, %d", bcd_args->arg1, bcd_args->arg2);
            if (bcd_args->arg1 == SYSTEM_BROADCAST_UTC_TIME_CHANGE) {
                // home_page_display_type_update(proc);
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
        if (proc->home_display_type != HOME_UI_DISPLAY_CLOCK_LOW_POWER &&
            proc->home_display_type != HOME_UI_DISPLAY_LOW_POWER) {
            // home_page_display_type_update(proc);
        }
    } else {
        if (proc->home_display_type == HOME_UI_DISPLAY_CLOCK_LOW_POWER ||
            proc->home_display_type == HOME_UI_DISPLAY_LOW_POWER) {
            // home_page_display_type_update(proc);
        }
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
        } else if (proc->extra.arg1 == HOME_SUB_UI_TYPE_PUMP_MODE_UNSELECT) {
            proc->home_display_type = HOME_UI_DISPLAY_DEFAULT;
        } else if (proc->extra.arg1 == HOME_SUB_UI_TYPE_PUMP_MODE_ADJUST) {
            home_pump_mode_adjust_enter(proc, false);
        } else if (proc->extra.arg1 == HOME_SUB_UI_TYPE_PUMP_RHYTHM_ADJUST) {
            home_pump_mode_adjust_enter(proc, true);
        } else {
            home_page_display_type_update(proc);
        }
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

