/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            home
#define PAGE_NAME_STR                               "home"
/** 含 L+R 组合键（KEY_CODE_BOTH_CONTROL）以进入双边力度调节 */
#define HOME_UI_KEY_CODE                            (ALL_KEY_CODE_NO_COMBINE | KEY_CODE_BOTH_CONTROL | \
                                                     KEY_CODE_TRAVEL_LOCK | KEY_CODE_FACTORY_RESET)
#define HOME_UI_KEY_TYPE                            (KEY_EVENT_CLICK | KEY_EVENT_UP | KEY_EVENT_LONG_2S | \
                                                     KEY_EVENT_DOWN | KEY_EVENT_LONG_5S)
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
static void home_async_level_adjust(ui_proc_home_t *proc, const window_evt_level_t *data)
{
    uint8_t level_max;
    int time_index;
    uint8_t i;

    EXCEPTION_CHECK_VOID(proc && proc->pump && data);
    home_sub_mode_state_init(proc, 1);
    time_index = proc->sub_mode_time_index;
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    level_max = home_level_adjust_max_get(proc->sub_mode_type);

    if (data->side == WINDOW_EVT_LEVEL_SIDE_BOTH) {
        for (i = 0; i < BREAST_PUMP_NUM; i++) {
            uint8_t level = data->level[i];
            if (level > level_max) {
                level = level_max;
            }
            proc->setting.level[i] = level;
        }
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        LOG_WM("async level adjust side=%d L=%d R=%d max=%d",
            data->side, proc->setting.level[0], proc->setting.level[1], level_max);
        show_home_both_level_adjust_ui(proc);
    } else if (data->side == WINDOW_EVT_LEVEL_SIDE_LEFT) {
        uint8_t level = data->level[0];
        if (level > level_max) {
            level = level_max;
        }
        proc->setting.level[0] = level;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        LOG_WM("async level adjust side=%d L=%d max=%d", data->side, level, level_max);
        show_home_level_adjust_ui(proc, true);
    } else {
        uint8_t level = data->level[1];
        if (level > level_max) {
            level = level_max;
        }
        proc->setting.level[1] = level;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        LOG_WM("async level adjust side=%d R=%d max=%d", data->side, level, level_max);
        show_home_level_adjust_ui(proc, false);
    }
}

static void home_async_speed_adjust(ui_proc_home_t *proc, uint8_t speed)
{
    int time_index;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    home_sub_mode_state_init(proc, 0);
    time_index = proc->sub_mode_time_index;
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    if (speed >= BREAST_PUMP_SPEED_MAX) {
        speed = BREAST_PUMP_SPEED_MAX - 1;
    }

    LOG_WM("async speed adjust speed=%d", speed);
    proc->setting.speed = speed;
    WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    show_home_speed_adjust_ui(proc);
}

static int home_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_home_t *proc = (ui_proc_home_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_HOME, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    switch (evt_data->evt_id) {
        case WINDOW_ASYNC_EVT_LEVEL_ADJUST: {
            window_evt_level_t *data = (window_evt_level_t *)evt_data->user_data;
            if (!data) {
                LOG_WM("home level evt user_data null.");
                break;
            }
            home_async_level_adjust(proc, data);
            break;
        }
        case WINDOW_ASYNC_EVT_SPEED_ADJUST: {
            window_evt_speed_t *data = (window_evt_speed_t *)evt_data->user_data;
            if (!data) {
                LOG_WM("home speed evt user_data null.");
                break;
            }
            home_async_speed_adjust(proc, data->speed);
            break;
        }
        default:
            LOG_WM("unknow async evt: %u", evt_data->evt_id);
            break;
    }
    return 0;
}

static void home_start_key_event_handler(ui_proc_home_t *proc)
{
    if (proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT ||
        proc->current_ui_type == HOME_UI_TYPE_SETTINGS_SUCCESS) {
        LOG_WM("current ui type is %d, ignore start key.", proc->current_ui_type);
        return;
    }
    home_start_pump_handler(proc);
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
    } else {
        proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
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
            /// 模式选择页短按 power：确认模式 → 成功提示，超时后启动泵奶
            home_mode_confirm_handler(proc);
            show_home_settings_success_ui(proc);
        } else {
            home_start_pump_handler(proc);
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

#if HOME_ADJUST_KEY_EXIT_PROTECT_ENABLE
static bool home_adjust_key_exit_protected(ui_proc_home_t *proc)
{
    if (get_timestamp_ms(proc->level_adjust_time) < HOME_ADJUST_KEY_EXIT_PROTECT_MS) {
        LOG_WM("adjust key exit protect %ums.", HOME_ADJUST_KEY_EXIT_PROTECT_MS);
        return true;
    }
    return false;
}
#endif

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
            if (proc->current_ui_type == HOME_UI_TYPE_SPEED_ADJUST) {
#if HOME_ADJUST_KEY_EXIT_PROTECT_ENABLE
                if (home_adjust_key_exit_protected(proc)) {
                    break;
                }
#endif
                home_speed_adjust_ui_dismiss(proc);
                break;
            }
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
            if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST &&
                proc->level_adjust_side == HOME_LEVEL_ADJUST_SIDE_RIGHT) {
#if HOME_ADJUST_KEY_EXIT_PROTECT_ENABLE
                if (home_adjust_key_exit_protected(proc)) {
                    break;
                }
#endif
                home_level_adjust_ui_dismiss(proc);
                break;
            }
            show_home_level_adjust_ui(proc, false);
            break;
        case KEY_CODE_LEFT_CONTROL:
            if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST &&
                proc->level_adjust_side == HOME_LEVEL_ADJUST_SIDE_LEFT) {
#if HOME_ADJUST_KEY_EXIT_PROTECT_ENABLE
                if (home_adjust_key_exit_protected(proc)) {
                    break;
                }
#endif
                home_level_adjust_ui_dismiss(proc);
                break;
            }
            show_home_level_adjust_ui(proc, true);
            break;
        case KEY_CODE_BOTH_CONTROL:
            if (proc->current_ui_type == HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
#if HOME_ADJUST_KEY_EXIT_PROTECT_ENABLE
                if (home_adjust_key_exit_protected(proc)) {
                    break;
                }
#endif
                home_both_level_adjust_ui_dismiss(proc);
                break;
            }
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

    LOG_WM("keyevent: %d, %d", keycode, keytype);
    /// 模式选择页倒计时中：任意按键刷新计时
    home_mode_select_idle_touch(proc);

    /// 待机页：短按 power 退出回首页，长按 power 可关机，其它按键不响应
    if (proc->current_ui_type == HOME_UI_TYPE_HOME_IDLE) {
        if (keytype == KEY_EVENT_CLICK && keycode == KEY_CODE_POWER) {
            if (proc->power_off_hold_ignore_click) {
                proc->power_off_hold_ignore_click = 0;
                LOG_WM("ignore power click after power off hold.");
                return 0;
            }
            home_idle_ui_dismiss(proc);
            return 0;
        }

        if (keytype == KEY_EVENT_LONG_2S && keycode == KEY_CODE_POWER) {
            home_power_off_hold_key_long_2s_handler(proc);
            return 0;
        }

        LOG_WM("system idle, ignore key event.");
        return 0;
    }

    if (keycode == KEY_CODE_TRAVEL_LOCK && keytype == KEY_EVENT_LONG_2S) { 
        if (proc->airplane_mode && proc->airplane_mode->hook && 
            proc->airplane_mode->hook->travel_lock_key_event_handler) {
            proc->airplane_mode->hook->travel_lock_key_event_handler(proc->airplane_mode, proc->wm);
        }
        return 0;
    }

    if (keycode == KEY_CODE_FACTORY_RESET && keytype == KEY_EVENT_LONG_5S) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_FACTORY_RESET;
        msg.union_body.extra.arg1 = 1; /// 1: factory reset confirm
        msg.union_body.extra.arg2 = ID_WINDOW_HOME;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        LOG_WM("goto factory reset");
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

    /// 关机确认页期间忽略其它 click，避免误触
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
                /// 待机页仅允许短按 power 退出；其它按键触发的 idle release 保持待机并熄灯
                if (proc->current_ui_type == HOME_UI_TYPE_HOME_IDLE) {
                    if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                        proc->led->hook->set_key_backlight(proc->led, 0);
                    }
                } else if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                    /// 已由 power 短按退出时，确保按键灯全亮（覆盖 led_hook 半亮）
                    proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
                }
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
        if (proc->extra.arg1 == HOME_SUB_UI_TYPE_LEVEL_ADJUST) {
            show_home_level_adjust_ui(proc, proc->extra.arg2 == HOME_LEVEL_ADJUST_SIDE_LEFT);
            return 0;
        }
        if (proc->extra.arg1 == HOME_SUB_UI_TYPE_BOTH_LEVEL_ADJUST) {
            show_home_both_level_adjust_ui(proc);
            return 0;
        }
        if (proc->extra.arg1 == HOME_SUB_UI_TYPE_SPEED_ADJUST) {
            show_home_speed_adjust_ui(proc);
            return 0;
        }
        if (proc->extra.arg1 == HOME_SUB_UI_TYPE_MODE_SET) {
            proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
            start_home_ui_animation(proc);
            home_mode_set_handler(proc, (uint8_t)proc->extra.arg2);
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
    proc->motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    proc->airplane_mode = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "home_screen";
    proc->receiver.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    proc->receiver.receiver_cb = on_home_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);

    /// 设置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = home_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);
    
    return ID_WINDOW_DISPLAY_OK;
}

