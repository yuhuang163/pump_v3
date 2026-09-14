/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager.h" // IWYU pragma: keep
#include "input_key_code.h"
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"// IWYU pragma: keep
#include "windows_manager.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                    pump
#define PAGE_NAME_STR                       "pump"
/** 含 L+R 组合键（KEY_CODE_BOTH_CONTROL）：短按/长按(2~10s)均可进入或退出双边力度调节 */
#define PUMP_UI_KEY_CODE                    (ALL_KEY_CODE_NO_COMBINE | KEY_CODE_BOTH_CONTROL)
#define PUMP_UI_KEY_TYPE                    (KEY_EVENT_CLICK | KEY_EVENT_DOWN | KEY_EVENT_UP | \
                                             KEY_EVENT_LONG_2S | KEY_EVENT_LONG_3S | KEY_EVENT_LONG_4S | \
                                             KEY_EVENT_LONG_5S | KEY_EVENT_LONG_6S | KEY_EVENT_LONG_7S | \
                                             KEY_EVENT_LONG_8S | KEY_EVENT_LONG_9S | KEY_EVENT_LONG_10S)
/** L+R 长按进入：2~10s 均视为进入触发 */
#define PUMP_BOTH_CONTROL_LONG_PRESS(keytype) \
    ((keytype) == KEY_EVENT_LONG_2S || (keytype) == KEY_EVENT_LONG_3S || \
     (keytype) == KEY_EVENT_LONG_4S || (keytype) == KEY_EVENT_LONG_5S || \
     (keytype) == KEY_EVENT_LONG_6S || (keytype) == KEY_EVENT_LONG_7S || \
     (keytype) == KEY_EVENT_LONG_8S || (keytype) == KEY_EVENT_LONG_9S || \
     (keytype) == KEY_EVENT_LONG_10S)
#define PUMP_COMPLETE_TASK_NAME             "pump_complete"
#define PUMP_BROADCAST_TASK_NAME            "pump_broadcast"
#define PUMP_SETTINGS_SUCCESS_DISPLAY_TASK_NAME "pump_settings_success_display"
#define PUMP_STOP_HOLD_COMPLETE_TASK_NAME   "pump_stop_hold_complete"
#define PUMP_POWER_OFF_HOLD_COMPLETE_TASK_NAME "pump_power_off_hold_complete"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_COMPLETE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BROADCAST_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SETTINGS_SUCCESS_DISPLAY_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_STOP_HOLD_COMPLETE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_POWER_OFF_HOLD_COMPLETE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LOW_POWER_COMPLETE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_LEAK_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_FIT_NOT_DETECTED_TASK_NAME); \
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
static void pump_async_level_adjust(ui_proc_pump_t *proc, const window_evt_level_t *data)
{
    uint8_t level_max;
    int time_index;
    uint8_t i;

    EXCEPTION_CHECK_VOID(proc && proc->pump && data);
    pump_sub_mode_state_init(proc, 1);
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
        show_pump_both_level_adjust_ui(proc);
    } else if (data->side == WINDOW_EVT_LEVEL_SIDE_LEFT) {
        uint8_t level = data->level[0];
        if (level > level_max) {
            level = level_max;
        }
        proc->setting.level[0] = level;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        LOG_WM("async level adjust side=%d L=%d max=%d", data->side, level, level_max);
        show_pump_level_adjust_ui(proc, true);
    } else {
        uint8_t level = data->level[1];
        if (level > level_max) {
            level = level_max;
        }
        proc->setting.level[1] = level;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        LOG_WM("async level adjust side=%d R=%d max=%d", data->side, level, level_max);
        show_pump_level_adjust_ui(proc, false);
    }
}

static void pump_async_speed_adjust(ui_proc_pump_t *proc, uint8_t speed)
{
    int time_index;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    pump_sub_mode_state_init(proc, 0);
    time_index = proc->sub_mode_time_index;
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    if (speed >= BREAST_PUMP_SPEED_MAX) {
        speed = BREAST_PUMP_SPEED_MAX - 1;
    }

    LOG_WM("async speed adjust speed=%d", speed);
    proc->setting.speed = speed;
    WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    show_pump_speed_adjust_ui(proc);
}

static int pump_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_PUMP, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    switch (evt_data->evt_id) {
        case WINDOW_ASYNC_EVT_LEVEL_ADJUST: {
            window_evt_level_t *data = (window_evt_level_t *)evt_data->user_data;
            if (!data) {
                LOG_WM("pump level evt user_data null.");
                break;
            }
            pump_async_level_adjust(proc, data);
            break;
        }
        case WINDOW_ASYNC_EVT_SPEED_ADJUST: {
            window_evt_speed_t *data = (window_evt_speed_t *)evt_data->user_data;
            if (!data) {
                LOG_WM("pump speed evt user_data null.");
                break;
            }
            pump_async_speed_adjust(proc, data->speed);
            break;
        }
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
        case WINDOW_ASYNC_EVT_PUMP_PAUSE_SETTLE: {
            LOG_WM("pm suspend, trigger pump pause settle.");
            pump_pause_settle_trigger(proc);
            break;
        }
#endif
        default:
            LOG_WM("unknow async evt: %u", evt_data->evt_id);
            break;
    }
    return 0;
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
        case PUMP_HDR_ACTION_PUMP_WILL_FINISH_EXIT:
            if (proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
                LOG_WM("pump will finish, exit and show home.");
                show_pump_home_ui(proc);
            }
            break;
        default:
            LOG_WM("unknow action: %d, ignore.", action);
            break;
    }
}

static void on_breast_pump_result_cb(void *user_data, breast_pump_result_t *result)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    
    if (proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
        if (result->working_mode != proc->will_finish_mode || result->work_valid_time == 0) {
            POST_WM_TASK_HANDLER_NO_DELAY_ARG1(pump_ui_common_cb, proc, PUMP_HDR_ACTION_PUMP_WILL_FINISH_EXIT,
                PUMP_COMPLETE_TASK_NAME);
        }
    }
}

static void pump_mode_key_event_cb(ui_proc_pump_t *proc, uint8_t mode_index)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        pump_stop_hold_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM ||
        proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS) {
        /* 退出确认/成功页后按按键模式落地，不沿用当前预览页 */
        pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
    } else if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        /* 从力度/速度等子页回首页时清导航栈，避免残留 page_index 被 start 误恢复 */
        pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
    } else {
        proc->ui_mode_page_index = PUMP_MODE_PAGE_INDEX_NONE;
    }
    pump_mode_set_handler(proc, mode_index);
#if PUMP_PAUSE_MODE_KEY_START_ENABLE
    {
        int pump_status = 0;

        READ_PUMP_WORK_STATUS(proc->pump, pump_status);
        if (pump_status == BREAST_PUMP_PAUSE) {
            LOG_WM("pause + mode key, switch mode and start pump. mode:%d", mode_index);
            pump_start_key_event_handler(proc);
        }
    }
#endif
}

static void pump_knob_left_event_cb(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        pump_stop_hold_dismiss(proc);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS) {
        pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        /* 取消确认：回到进入前的模式选择页 */
        pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_KEEP_PAGE);
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
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK) {
        pump_seal_leak_page_adjust(proc, true);
    }
}

static void pump_knob_right_event_cb(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        pump_stop_hold_dismiss(proc);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS) {
        pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        /* 取消确认：回到进入前的模式选择页 */
        pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_KEEP_PAGE);
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
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK) {
        pump_seal_leak_page_adjust(proc, false);
    }
}

#if PUMP_ADJUST_KEY_EXIT_PROTECT_ENABLE
static bool pump_adjust_key_exit_protected(ui_proc_pump_t *proc)
{
    if (get_timestamp_ms(proc->adjust_enter_time) < PUMP_ADJUST_KEY_EXIT_PROTECT_MS) {
        LOG_WM("adjust key exit protect %ums.", PUMP_ADJUST_KEY_EXIT_PROTECT_MS);
        return true;
    }
    return false;
}
#endif

/** 休息不可调节提示：短按 Stimulation/Expression/Start/Pause 或右旋编码器时关闭弹窗 */
static void pump_rest_mode_adjust_tip_dismiss_by_key(ui_proc_pump_t *proc, uint16_t keycode)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->popup_ui_type != ID_POPUP_UI_PUMP_REST_MODE_ADJUST_TIP) {
        return;
    }
    switch (keycode) {
        case KEY_CODE_STIMULATION_MODE:
        case KEY_CODE_LACTATION_MODE:
        case KEY_CODE_START:
        case KEY_CODE_KNOB_RIGHT:
        case KEY_CODE_POWER:
        case KEY_CODE_KNOB_LEFT:
            pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
            LOG_WM("rest mode adjust tip dismissed by key:%d", keycode);
            break;
        default:
            break;
    }
}

/** 双边力度：短按/长按共用，未在页进入、已在页退出；返回是否已处理 */
static bool pump_both_control_key_event(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
#if PUMP_ADJUST_KEY_EXIT_PROTECT_ENABLE
        if (pump_adjust_key_exit_protected(proc)) {
            return false;
        }
#endif
        pump_both_level_adjust_ui_dismiss(proc);
        return true;
    }

    if (pump_rest_mode_blocks_adjust(proc)) {
        pump_show_rest_mode_adjust_tip(proc);
        return true;
    }
    show_pump_both_level_adjust_ui(proc);
    return true;
}

static void pump_click_event_cb(ui_proc_pump_t *proc, uint16_t keycode)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_CHARGE_ERROR ||
        proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT) {
        LOG_WM("pump finish ui, ignore key event.");
        return;
    }

    pump_rest_mode_adjust_tip_dismiss_by_key(proc, keycode);

    LOG_WM("click keycode: %d, popup ui type:%d.", keycode, proc->popup_ui_type);
    switch (keycode) {
        case KEY_CODE_STIMULATION_MODE:
            pump_mode_key_event_cb(proc, BREAST_PUMP_STIMULATION_MODE);
            break;
        case KEY_CODE_LACTATION_MODE:
            pump_mode_key_event_cb(proc, BREAST_PUMP_LACTATION_MODE);
            break;
        case KEY_CODE_FREQUENCY:
            if (proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) {
#if PUMP_ADJUST_KEY_EXIT_PROTECT_ENABLE
                if (pump_adjust_key_exit_protected(proc)) {
                    break;
                }
#endif
                pump_speed_adjust_ui_dismiss(proc);
                break;
            }
            if (pump_rest_mode_blocks_adjust(proc)) {
                pump_show_rest_mode_adjust_tip(proc);
                break;
            }
            show_pump_speed_adjust_ui(proc);
            break;
        case KEY_CODE_POWER:
            pump_power_key_event_handler(proc);
            break;
        case KEY_CODE_START:
            pump_start_key_event_handler(proc);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST &&
                proc->level_adjust_side == PUMP_LEVEL_ADJUST_SIDE_RIGHT) {
#if PUMP_ADJUST_KEY_EXIT_PROTECT_ENABLE
                if (pump_adjust_key_exit_protected(proc)) {
                    break;
                }
#endif
                pump_level_adjust_ui_dismiss(proc);
                break;
            }
            if (pump_rest_mode_blocks_adjust(proc)) {
                pump_show_rest_mode_adjust_tip(proc);
                break;
            }
            show_pump_level_adjust_ui(proc, false);
            break;
        case KEY_CODE_LEFT_CONTROL:
            if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST &&
                proc->level_adjust_side == PUMP_LEVEL_ADJUST_SIDE_LEFT) {
#if PUMP_ADJUST_KEY_EXIT_PROTECT_ENABLE
                if (pump_adjust_key_exit_protected(proc)) {
                    break;
                }
#endif
                pump_level_adjust_ui_dismiss(proc);
                break;
            }
            if (pump_rest_mode_blocks_adjust(proc)) {
                pump_show_rest_mode_adjust_tip(proc);
                break;
            }
            show_pump_level_adjust_ui(proc, true);
            break;
        case KEY_CODE_BOTH_CONTROL:
            pump_both_control_key_event(proc);
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
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_PUMP, 0);
    EXCEPTION_CHECK_INT(proc->pump);
    if (proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_CHARGE_ERROR) {
        LOG_WM("pump finish ui, ignore key event. type:%d code:%d", keytype, keycode);
        return 0;
    }

#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
#if PUMP_PAUSE_TIMEOUT_SCHEME == PUMP_PAUSE_TIMEOUT_SCHEME_KEY_EXIT
    /// 方案2：任意可响应按键先退出结算页，再执行对应功能（保持暂停态）
    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT) {
        if (keytype != KEY_EVENT_CLICK &&
            !(keytype == KEY_EVENT_LONG_2S &&
              (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) &&
            !(keytype == KEY_EVENT_UP &&
              (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) &&
            !(keytype == KEY_EVENT_LONG_2S && keycode == KEY_CODE_TRAVEL_LOCK) &&
            !(PUMP_BOTH_CONTROL_LONG_PRESS(keytype) && keycode == KEY_CODE_BOTH_CONTROL)) {
            LOG_WM("pump pause timeout ui, ignore key event. type:%d code:%d",
                keytype, keycode);
            return 0;
        }
        LOG_WM("pump pause timeout ui key exit, type:%d code:%d", keytype, keycode);
        pump_pause_timeout_ui_dismiss(proc);
    }
#else
    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT) {
        LOG_WM("pump pause timeout ui, ignore key event. type:%d code:%d",
            keytype, keycode);
        return 0;
    }
#endif
#endif

    if (proc->current_ui_type == PUMP_UI_TYPE_LOW_POWER) {
        if (keytype != KEY_EVENT_CLICK &&
            !(keytype == KEY_EVENT_LONG_2S &&
              (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) &&
            !(keytype == KEY_EVENT_UP &&
              (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) &&
            !(keytype == KEY_EVENT_LONG_2S && keycode == KEY_CODE_TRAVEL_LOCK) &&
            !(PUMP_BOTH_CONTROL_LONG_PRESS(keytype) && keycode == KEY_CODE_BOTH_CONTROL)) {
            LOG_WM("pump low power ui, ignore key event. type:%d code:%d",
                keytype, keycode);
            return 0;
        }
        LOG_WM("pump low power ui key exit, type:%d code:%d", keytype, keycode);
        pump_low_power_ui_dismiss(proc);
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        if (!((keycode == KEY_CODE_POWER || keycode == KEY_CODE_START) &&
            (keytype == KEY_EVENT_CLICK || keytype == KEY_EVENT_LONG_2S ||
             keytype == KEY_EVENT_UP))) {
            LOG_WM("fit not detected ui, ignore key event. type:%d code:%d",
                keytype, keycode);
            return 0;
        }
    }

    if (pump_seal_leak_restrict_keys(proc)) {
        bool leak_key_ok = false;

        if ((keycode == KEY_CODE_POWER || keycode == KEY_CODE_START) &&
            (keytype == KEY_EVENT_CLICK || keytype == KEY_EVENT_LONG_2S ||
             keytype == KEY_EVENT_UP)) {
            leak_key_ok = true;
        } else if ((keycode == KEY_CODE_KNOB_LEFT || keycode == KEY_CODE_KNOB_RIGHT) &&
            keytype == KEY_EVENT_CLICK) {
            leak_key_ok = true;
        }
        if (!leak_key_ok) {
            LOG_WM("seal leak ui, ignore key event. type:%d code:%d",
                keytype, keycode);
            return 0;
        }
    }

    if (keycode == KEY_CODE_TRAVEL_LOCK && keytype == KEY_EVENT_LONG_2S) {
        if (proc->airplane_mode && proc->airplane_mode->hook && 
            proc->airplane_mode->hook->travel_lock_key_event_handler) {
            proc->airplane_mode->hook->travel_lock_key_event_handler(proc->airplane_mode, proc->wm);
        }
        return 0;
    }

    /// 模式选择页倒计时中：任意按键刷新计时
    pump_mode_select_idle_touch(proc);
    /// 力度/速度调节页倒计时中：任意按键刷新计时（含 Start/Pause 启停后留在调节页）
    pump_adjust_idle_touch(proc);
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
    /// 暂停无操作倒计时：任意按键刷新计时
    pump_pause_idle_touch(proc);
#endif

#if PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE
    /// 韵律确认页：任意按键刷新静默返回计时
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        proc->last_progress_ani_time = get_timestamp();
    }
#endif
    
    /// 韵律即将结束提示：仅短按 power 可退出，其它按键/长按/抬起均忽略
    if (proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
        if (keytype == KEY_EVENT_CLICK && keycode == KEY_CODE_POWER) {
            pump_will_finish_ui_dismiss(proc);
        } else {
            LOG_WM("pump will finish ui, ignore key event");
        }
        return 0;
    }

    /// L+R 按下/抬起：重置本次长按会话，避免同一次按住的 LONG_3S~10S 再次切换进出
    if ((keytype == KEY_EVENT_DOWN || keytype == KEY_EVENT_UP) &&
        keycode == KEY_CODE_BOTH_CONTROL) {
        proc->both_control_long_acted = 0;
        return 0;
    }

    if (keytype == KEY_EVENT_LONG_2S &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        pump_power_off_hold_key_long_2s_handler(proc);
        return 0;
    }

    if (keytype == KEY_EVENT_UP &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        pump_power_off_hold_key_up_handler(proc);
        return 0;
    }

    if (PUMP_BOTH_CONTROL_LONG_PRESS(keytype) && keycode == KEY_CODE_BOTH_CONTROL) {
        /// 长按 L+R（2~10s）：与短按一致，进入/退出双边力度调节
        if (proc->current_ui_type == PUMP_UI_TYPE_POWER_OFF_HOLD) {
            return 0;
        }
        if (proc->both_control_long_acted == 0 &&
            pump_both_control_key_event(proc)) {
            proc->both_control_long_acted = 1;
        }
        return 0;
    } 

    if (keytype == KEY_EVENT_CLICK) {
        /// 关机确认页期间忽略其它 click，避免误触
        if (proc->current_ui_type == PUMP_UI_TYPE_POWER_OFF_HOLD) {
            return 0;
        }
        pump_click_event_cb(proc, keycode);
        return 0;
    }    
    return 0;
}

static void pump_runing_30m_tip_pause(ui_proc_pump_t *proc)
{
    int pump_status = 0;
    int control_source = 0;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_PAUSE || pump_status == BREAST_PUMP_IDLE ||
        pump_status == BREAST_PUMP_FINISH) {
        return;
    }

    WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_PAUSE, 1);
    proc->pump_status = BREAST_PUMP_PAUSE;
    READ_PUMP_CONTROL_SOURCE(proc->pump, control_source);
    data_bury_trigger_user_evt_t bury_evt = {
        .evt_source = (control_source == true ? DATA_BURY_EVT_SOURCE_REMOTE_APP :
            DATA_BURY_EVT_SOURCE_LOCAL),
        .evt_type = DATA_BURY_USER_EVT_PAUSE,
    };
    DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
    WRITE_PUMP_CONTROL_SOURCE(proc->pump, false);
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
    pump_pause_idle_start(proc);
#endif
    LOG_WM("pump running 30m tip, pause pump. status:%d", pump_status);
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
                if (proc->battery &&
                    proc->battery->get_battery_state() == BATTERY_STATE_LOW_POWER_SHUTDOWN) {
                    LOG_WM("low power shutdown, ignore pump complete goto home.");
                    break;
                }
                if (proc->current_ui_type != PUMP_UI_TYPE_FINISH_TIMEOUT &&
                    proc->current_ui_type != PUMP_UI_TYPE_FINISH_CHARGE_ERROR &&
                    proc->current_ui_type != PUMP_UI_TYPE_PAUSE_TIMEOUT &&
                    proc->current_ui_type != PUMP_UI_TYPE_LOW_POWER) {
                    multi_func_msg_data_t msg = {0};
                    msg.union_body.extra.window_id = ID_WINDOW_HOME;
                    msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT;
                    proc->wm->dispatch_new_window_args(proc->wm, &msg);
                    LOG_WM("pump finish, goto home window.");
                }
            } else if (state == BREAST_PUMP_BROADCAST_TIMEOUT) {
                if (proc->current_ui_type != PUMP_UI_TYPE_FINISH_TIMEOUT &&
                    proc->current_ui_type != PUMP_UI_TYPE_FINISH_CHARGE_ERROR &&
                    proc->current_ui_type != PUMP_UI_TYPE_PAUSE_TIMEOUT &&
                    proc->current_ui_type != PUMP_UI_TYPE_LOW_POWER) {
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
                    pump_pause_idle_stop(proc);
#endif
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
                proc->pump_status = BREAST_PUMP_PAUSE;
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
                pump_pause_idle_start(proc);
#endif
                pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_PAUSE_TIMEOUT);
            } else if (state == BREAST_PUMP_BROADCAST_RUNNING_30M_TIP) {
                pump_runing_30m_tip_pause(proc);
                pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
                pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_RUNING_30M_TIP);
                LOG_WM("pump running 30m tip popup.");
            } else if (state == BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH_TIP) {
                if (PUMP_IS_BREASTFEEDING_MODE(pump_mode)) {
                    show_pump_will_finish_ui(proc);
                    LOG_WM("pump will finish tip, mode:%d", pump_mode);
                }
            } else if (state == BREAST_PUMP_BROADCAST_STATE_CHANGE) {
                /// 只补按键没走到的缺口：泵已在跑、窗口还停在暂停。
                /// 按键启停会先改 proc->pump_status 再发同样广播，缓存已对齐则这里不动。
                if (PUMP_IS_GOING(pump_status) && proc->pump_status == BREAST_PUMP_PAUSE) {
                    proc->pump_status = pump_status;
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
                    pump_pause_idle_stop(proc);
#endif
                    if (proc->led && proc->led->set_led_level) {
                        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
                    }
                    LOG_WM("pump ui sync running, status:%d", pump_status);
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
            proc->current_ui_type != PUMP_UI_TYPE_PAUSE_TIMEOUT &&
            proc->current_ui_type != PUMP_UI_TYPE_FINISH_CHARGE_ERROR) {
            show_pump_finish_charge_error_ui(proc);
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, 0);
            }
        }
        return;
    } else {
        if (proc->current_ui_type == PUMP_UI_TYPE_FINISH_CHARGE_ERROR) {
            LOG_WM("charge adapter recovered, dismiss charge error ui.");
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
            }
            pump_finish_charge_error_ui_dismiss(proc);
        }
    }

    if (info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN &&
        !BATTERY_IS_CHARGING(info->charge_status) &&
        proc->current_ui_type != PUMP_UI_TYPE_POWER_OFF_HOLD) {
        LOG_WM("low power shutdown, goto power off.");
        if (proc->pump) {
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END,
                DATA_BURY_END_REASON_LOW_BATTERY, 0);
            WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, 1);
            proc->pump_status = BREAST_PUMP_FINISH;
        }
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
        msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_LOW_POWER;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (info->battery_state == BATTERY_STATE_FORBID_START) {
        if (proc->low_power_ui_shown == 0 &&
            proc->current_ui_type != PUMP_UI_TYPE_FINISH_TIMEOUT &&
            proc->current_ui_type != PUMP_UI_TYPE_FINISH_CHARGE_ERROR &&
            proc->current_ui_type != PUMP_UI_TYPE_PAUSE_TIMEOUT &&
            proc->current_ui_type != PUMP_UI_TYPE_LOW_POWER) {
            proc->low_power_ui_shown = 1;
            show_pump_low_power_ui(proc);
            return;
        }
    } else {
        proc->low_power_ui_shown = 0;
    }
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
        pump_seal_leak_init(proc);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        proc->current_ui_type = PUMP_UI_TYPE_NONE;
        if (proc->pump) {
            WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_IDLE, 1);
            proc->pump->add_breast_pump_result_listener(proc, on_breast_pump_result_cb);
        }
        int mode_index = 0;
        READ_PUMP_WORK_MODE(proc->pump, mode_index);
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
            #if !IS_SUPPORT_PUMP_IN_MODE_SWITCH_RHYTHM
            proc->led->set_led_level(PUMP_BREASTFEEDING_BL, 0);
            #endif
        }
        /// home 模式确认进入：先 Success 1.5s，再回泵奶首页
        if (proc->extra.arg1 == PUMP_SUB_UI_TYPE_SETTINGS_SUCCESS) {
            pump_start_pump_keep_ui(proc);
            show_pump_settings_success_ui(proc, false);
        } else {
            switch_pump_status_ui(proc);
        }
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
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
            pump_pause_idle_stop(proc);
#endif
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            pump_seal_leak_exit(proc);
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
    memcpy(&proc->extra, extra, sizeof(window_entry_extra_t));
    proc->margic = UI_WINDOW_MARGIC_CODE;
    LOG_WM("window entry extra arg1: %d.", proc->extra.arg1);
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
    proc->airplane_mode = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "pump_screen";
    proc->receiver.id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    proc->receiver.receiver_cb = on_pump_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);

    /// 设置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = pump_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);
    
    return ID_WINDOW_DISPLAY_OK;
}

void pump_complete_event_handler(ui_proc_pump_t *proc)
{
    POST_WM_TASK_HANDLER_NO_DELAY_ARG1(pump_ui_common_cb, proc, PUMP_HDR_ACTION_PUMP_COMPLETE,
        PUMP_COMPLETE_TASK_NAME);
}