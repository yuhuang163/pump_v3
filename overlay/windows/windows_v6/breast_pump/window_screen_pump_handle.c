/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager.h" // IWYU pragma: keep
#include "input_key_code.h"
#include "window_screen_pump.h"// IWYU pragma: keep
#include "windows_manager.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                    pump
#define PAGE_NAME_STR                       "pump"
/** 含 L+R 组合键（KEY_CODE_FACTORY_RESET）以进入双边力度调节 */
#define PUMP_UI_KEY_CODE                    (ALL_KEY_CODE_NO_COMBINE | KEY_CODE_FACTORY_RESET)
#define PUMP_UI_KEY_TYPE                    (KEY_EVENT_CLICK | KEY_EVENT_DOWN | KEY_EVENT_UP | KEY_EVENT_LONG_2S)
#define PUMP_COMPLETE_TASK_NAME             "pump_complete"
#define PUMP_BROADCAST_TASK_NAME            "pump_broadcast"
#define PUMP_SETTINGS_SUCCESS_DISPLAY_TASK_NAME "pump_settings_success_display"
#define PUMP_BREAK_TIP_DISPLAY_TASK_NAME        "pump_break_tip_display"
#define PUMP_STOP_HOLD_COMPLETE_TASK_NAME   "pump_stop_hold_complete"
#define PUMP_BREAK_END_COMPLETE_TASK_NAME       "pump_break_end_complete"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_COMPLETE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BROADCAST_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SETTINGS_SUCCESS_DISPLAY_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BREAK_TIP_DISPLAY_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_STOP_HOLD_COMPLETE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BREAK_END_COMPLETE_TASK_NAME); \
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
static void on_breast_pump_result_cb(void *user_data, breast_pump_result_t *result)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
}

static void pump_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    int action = args->arg1;
    switch (action) {
        case PUMP_HDR_ACTION_PUMP_COMPLETE:        
            LOG_WM("pump complete, goto home window.");
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_HOME;
            msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
            break;
        default:
            LOG_WM("unknow action: %d, ignore.", action);
            break;
    }
}


static void pump_mode_key_event_cb(ui_proc_pump_t *proc, uint8_t mode_index)
{
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        proc->ui_mode_page_index = PUMP_MODE_PAGE_INDEX_NONE;
        show_pump_home_ui(proc);
    }
    pump_mode_set_handler(proc, mode_index);
}

static void pump_knob_left_event_cb(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        pump_stop_hold_dismiss(proc);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS ||
        proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM ||
        proc->current_ui_type == PUMP_UI_TYPE_BREAK_TIP) {
        show_pump_home_ui(proc);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST) {
        pump_level_adjust_handler(proc, false);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        pump_both_level_adjust_handler(proc, false);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) {
        pump_speed_adjust_handler(proc, false);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        pump_mode_adjust_handler(proc, true);
    }
}

static void pump_knob_right_event_cb(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        pump_stop_hold_dismiss(proc);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS ||
        proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM ||
        proc->current_ui_type == PUMP_UI_TYPE_BREAK_TIP) {
        show_pump_home_ui(proc);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST) {
        pump_level_adjust_handler(proc, true);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        pump_both_level_adjust_handler(proc, true);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) {
        pump_speed_adjust_handler(proc, true);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        pump_mode_adjust_handler(proc, false);
    }
}

static void pump_click_event_cb(ui_proc_pump_t *proc, uint16_t keycode)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_BREAK_END) {
        LOG_WM("pump finish/break-end ui, ignore key event.");
        return;
    }

    /* 韵律即将结束提示：仅短按 power 可退出，其它按键忽略 */
    if (proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
        if (keycode == KEY_CODE_POWER) {
            pump_will_finish_ui_dismiss(proc);
        } else {
            LOG_WM("pump will finish ui, ignore key event. code:%d", keycode);
        }
        return;
    }

    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_TIMEOUT ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_LOW_POWER) {
        LOG_WM("pump finish timeout, ignore key event.");
        return;
    }

    /* 成功页/确认页/超时页/停止确认页/力度调节页/速度调节页仅限制 start，其它按键正常响应（与 home 一致） */
    if ((proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS ||
         proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM ||
         proc->current_ui_type == PUMP_UI_TYPE_BREAK_TIP ||
         proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD ||
         proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST ||
         proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST ||
         proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) &&
        keycode == KEY_CODE_START) {
        LOG_WM("settings/confirm/break-tip/stop-hold/level-adjust/speed-adjust ui, ignore start key.");
        return;
    }

    LOG_WM("click keycode: %d, popup ui type:%d.", keycode, proc->popup_ui_type);
    switch (keycode) {
        case KEY_CODE_STIMULATION_MODE:
            if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
                pump_stop_hold_dismiss(proc);
                break;
            }
            pump_mode_key_event_cb(proc, BREAST_PUMP_STIMULATION_MODE);
            break;
        case KEY_CODE_LACTATION_MODE:
            if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
                pump_stop_hold_dismiss(proc);
                break;
            }
            pump_mode_key_event_cb(proc, BREAST_PUMP_LACTATION_MODE);
            break;
        case KEY_CODE_FREQUENCY:
            show_pump_speed_adjust_ui(proc);
            break;
        case KEY_CODE_POWER:
            pump_power_key_event_handler(proc);
            break;
        case KEY_CODE_START:
            pump_start_key_event_handler(proc);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
                show_pump_level_adjust_ui(proc, false);
            }
            break;
        case KEY_CODE_LEFT_CONTROL:
            if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
                show_pump_level_adjust_ui(proc, true);
            }
            break;
        case KEY_CODE_FACTORY_RESET:
            /* 同时短按 L+R → 双边力度调节 */
            show_pump_both_level_adjust_ui(proc);
            break;
        case KEY_CODE_KNOB_LEFT:
            pump_knob_left_event_cb(proc);
            break;
        case KEY_CODE_KNOB_RIGHT:
            pump_knob_right_event_cb(proc);
            break;
        default:
            LOG_WM("unknow keycode: %d, ignore.", keycode);
            break;
    }
}

static int on_pump_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_pump_t *proc = user_data;
    WINDOW_CURRENT_ID_GUARD_INT(proc, ID_WINDOW_PUMP, 0);
    EXCEPTION_CHECK_INT(proc->pump);
    if (proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_BREAK_END) {
        LOG_WM("pump finish/break-end ui, ignore key event. type:%d code:%d", keytype, keycode);
        return 0;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
        if (keytype == KEY_EVENT_CLICK && keycode == KEY_CODE_POWER) {
            pump_click_event_cb(proc, keycode);
        } else {
            LOG_WM("pump will finish ui, ignore key event. type:%d code:%d", keytype, keycode);
        }
        return 0;
    }
    if (keytype == KEY_EVENT_LONG_2S && keycode == KEY_CODE_POWER) {
        pump_stop_hold_key_long_2s_handler(proc);
    } else if (keytype == KEY_EVENT_UP && keycode == KEY_CODE_POWER) {
        pump_stop_hold_key_up_handler(proc);
    } else if (keytype == KEY_EVENT_CLICK) {
        pump_click_event_cb(proc, keycode);
    }
    return 0;
}

static void pump_broadcast_receive_ui_cb(hdr_post_args_t *args)
{
    window_broadcast_args_t *bcd_args = (window_broadcast_args_t *)args->user_data;
    EXCEPTION_CHECK_VOID(bcd_args);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)bcd_args->context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    EXCEPTION_CHECK_VOID(proc->pump);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    LOG_WM("code:%lx, arg1:%d, arg2:%d, status:%d, mode:%d",
            bcd_args->code, bcd_args->arg1, bcd_args->arg2, pump_status, pump_mode);
    switch (bcd_args->code) {
        case ID_BROADCAST_BREAST_PUMP_STATE_CHANGE: {
            int state = bcd_args->arg1;
            if (state == BREAST_PUMP_BROADCAST_COMPLETED) {
                if (proc->current_ui_type != PUMP_UI_TYPE_FINISH_TIMEOUT &&
                    proc->current_ui_type != PUMP_UI_TYPE_BREAK_END &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_TIMEOUT &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_LOW_POWER &&
                    proc->pump_will_finish && PUMP_NO_FINISH(pump_status)) {
                    multi_func_msg_data_t msg = {0};
                    msg.union_body.extra.window_id = ID_WINDOW_HOME;
                    msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT;
                    proc->wm->dispatch_new_window_args(proc->wm, &msg);
                    LOG_WM("pump finish, goto home window.");
                }
            } else if (state == BREAST_PUMP_BROADCAST_TIMEOUT) {
                if (proc->current_ui_type != PUMP_UI_TYPE_FINISH_TIMEOUT &&
                    proc->current_ui_type != PUMP_UI_TYPE_BREAK_END &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_TIMEOUT &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_LOW_POWER &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_MANUAL) {
                    show_pump_finish_timeout_ui(proc);
                    if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                        proc->led->hook->set_key_backlight(proc->led, 0);
                    }
                    LOG_WM("pump timeout, show finish timeout ui.");
                }
            } else if (state == BREAST_PUMP_BROADCAST_APP_COMPLETED) {
                multi_func_msg_data_t msg = {0};
                msg.union_body.extra.window_id = ID_WINDOW_HOME;
                msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT;
                proc->wm->dispatch_new_window_args(proc->wm, &msg);
                LOG_WM("pump app complete, post pump app complete task.");
            } else if (state == BREAST_PUMP_BROADCAST_PUMP_ERROR) {
                LOG_WM("pump error, post pump error task.");
            } else if (state == BREAST_PUMP_BROADCAST_AUTO_PAUSE_REQUEST) {
                pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_PAUSE_TIMEOUT);
            } else if (state == BREAST_PUMP_BROADCAST_RUNNING_30M_TIP) {
                show_pump_break_tip_ui(proc);
            } else if (state == BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH_TIP) {
                if (PUMP_IS_BREASTFEEDING_MODE(pump_mode)) {
                    show_pump_will_finish_ui(proc);
                    LOG_WM("pump will finish tip, show will finish ui. mode:%d", pump_mode);
                } else {
                    LOG_WM("pump will finish tip, not in rhythm mode, ignore. mode:%d", pump_mode);
                }
            }
            break;
        }
        default:
            break;
    }
}

static void on_pump_ui_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
     broadcast_priv_data_t *pdata)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
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
    POST_WM_TASK_HANDLER_NO_DELAY(pump_broadcast_receive_ui_cb, &proc->broadcast_args, PUMP_BROADCAST_TASK_NAME);
}

static void remove_breast_pump_result_cb(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status > BREAST_PUMP_IDLE && pump_status < BREAST_PUMP_FINISH) {
        LOG_WM("next wm:%d pump status:%d => goto finish", proc->wm->get_next_window_id(), pump_status);
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, 1);
    } 
    LOG_WM("==>remove breast_pump_result_listener");
    proc->pump->remove_breast_pump_result_listener(on_breast_pump_result_cb);
}

static void on_pump_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    
    if (info->charge_status == CHARGE_STATE_ERROR) {
        LOG_WM("charge status error");
        if (proc->current_ui_type != PUMP_UI_TYPE_FINISH_TIMEOUT &&
            proc->current_ui_type != PUMP_UI_TYPE_BREAK_END &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_TIMEOUT &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_LOW_POWER &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_MANUAL) {
            pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR);
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, 0);
            }
            return;
        }
    } else {
        if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR) {
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
            }
            pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        }
    }

#if IS_SUPPORT_LOW_POWER_AUTO_STOP_PUMP
    if (info->battery_state == BATTERY_STATE_EMPTY || 
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN || 
        info->battery_state == BATTERY_STATE_FORBID_START) {
        if (proc->current_ui_type != PUMP_UI_TYPE_FINISH_TIMEOUT &&
            proc->current_ui_type != PUMP_UI_TYPE_BREAK_END &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_TIMEOUT &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_LOW_POWER &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_MANUAL) {
            pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_FINISH_LOW_POWER);
            return;
        }
    }
#endif
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_pump_t *proc = CONTAINER_OF(thiz, ui_proc_pump_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    /// 处理窗口进入事件.
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        ADD_KEYEVENT_CB(proc->key, "pump_wm", proc, PUMP_UI_KEY_CODE, PUMP_UI_KEY_TYPE,
            on_pump_ui_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_pump_window_battery_state_change_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        proc->current_ui_type = PUMP_UI_TYPE_NONE;
        int mode_index = 0;
        READ_PUMP_WORK_MODE(proc->pump, mode_index);
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
            #if !IS_SUPPORT_PUMP_IN_MODE_SWITCH_RHYTHM
            proc->led->set_led_level(PUMP_BREASTFEEDING_BL, 0);
            #endif
        }
        switch_pump_status_ui(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_pump_t *proc = CONTAINER_OF(thiz, ui_proc_pump_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_pump_ui_animation(proc); 
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            unregister_broadcast_receiver(&proc->receiver);
            remove_breast_pump_result_cb(proc);
            REMOVE_KEYEVENT_CB(proc->key, on_pump_ui_keyevent_hdr);
            REMOVE_BATTERY_STATE_CB(proc->battery, on_pump_window_battery_state_change_cb);
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
    ui_proc_pump_t *proc = u_malloz(sizeof(ui_proc_pump_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    /// 设置窗口标识.
    window_entry_extra_t *extra = (window_entry_extra_t *)wm->get_window_entry_extra(wm);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    LOG_WM("window entry extra arg1: %d.", extra->arg1);
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "pump_screen";
    proc->receiver.id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    proc->receiver.receiver_cb = on_pump_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    /// 配置泵奶管理器.
    if (proc->pump) {
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_IDLE, 1);
        proc->pump->add_breast_pump_result_listener(proc, on_breast_pump_result_cb);
    }
    /// 设置窗口上下文.
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

void pump_complete_event_handler(ui_proc_pump_t *proc)
{
    proc->pump_will_finish = false;
    POST_WM_TASK_HANDLER_NO_DELAY_ARG1(pump_ui_common_cb, proc, PUMP_HDR_ACTION_PUMP_COMPLETE,
        PUMP_COMPLETE_TASK_NAME);
}