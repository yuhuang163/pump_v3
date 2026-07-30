/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void stop_pump_ui_animation(ui_proc_pump_t *proc)
{
    pump_rhythm_confirm_ui_exit(proc);
    pump_settings_success_ui_exit(proc);
    pump_break_tip_ui_exit(proc);
    pump_break_end_ui_exit(proc);
    pump_stop_hold_ui_exit(proc);
    pump_finish_timeout_ui_exit(proc);
    pump_will_finish_ui_exit(proc);
    pump_level_adjust_ui_exit(proc);
    pump_both_level_adjust_ui_exit(proc);
    pump_speed_adjust_ui_exit(proc);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
    proc->ui_mode_page_index = PUMP_MODE_PAGE_INDEX_NONE;
}

void pump_ui_change_hdr_remove(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case PUMP_UI_TYPE_RHYTHM_CONFIRM:
            pump_rhythm_confirm_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_SETTINGS_SUCCESS:
            pump_settings_success_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_BREAK_TIP:
            pump_break_tip_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_BREAK_END:
            pump_break_end_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_STOP_HOLD:
            pump_stop_hold_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_FINISH_TIMEOUT:
            pump_finish_timeout_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_WILL_FINISH:
            pump_will_finish_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_LEVEL_ADJUST:
            pump_level_adjust_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_BOTH_LEVEL_ADJUST:
            pump_both_level_adjust_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_SPEED_ADJUST:
            pump_speed_adjust_ui_exit(proc);
            break;
        default:
            break;
    }
}

void pump_popup_ui_type_update(ui_proc_pump_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}

void pump_power_key_event_handler(ui_proc_pump_t *proc)
{
    if (proc->stop_hold_ignore_click) {
        proc->stop_hold_ignore_click = 0;
        LOG_WM("ignore power click after stop hold release.");
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST) {
        pump_level_adjust_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        pump_both_level_adjust_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) {
        pump_speed_adjust_ui_dismiss(proc);
        return;
    }

    /// 确认页：再次短按 power → 落地模式并跳转成功页
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        if (pump_mode_confirm_handler(proc) == 0) {
            show_pump_settings_success_ui(proc);
        }
        return;
    }

    /// 模式选择页：满足条件先弹确认页，否则直接成功页
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME &&
        proc->ui_mode_page_index > PUMP_MODE_PAGE_INDEX_NONE) {
        if (pump_mode_rhythm_confirm_needed(proc) == 0) {
            show_pump_rhythm_confirm_ui(proc);
            return;
        }
        if (pump_mode_confirm_handler(proc) == 0) {
            show_pump_settings_success_ui(proc);
        }
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS) {
        show_pump_home_ui(proc);
        return;
    }

    /// 休息轻提醒页：短按 power → 回泵奶首页
    if (proc->current_ui_type == PUMP_UI_TYPE_BREAK_TIP) {
        show_pump_home_ui(proc);
        return;
    }

    /// 韵律即将结束提示：短按 power → 回泵奶首页
    if (proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH) {
        pump_will_finish_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        pump_stop_hold_dismiss(proc);
        return;
    }

#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        uint8_t battery_state = proc->battery->get_battery_state();
        if (battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
            battery_state == BATTERY_STATE_EMPTY ||
            battery_state == BATTERY_STATE_FORBID_START) {
            LOG_WM("battery state is low, ignore power key event.");
            return;
        }
        
#if IS_SUPPORT_DEVICE_NIGHT_LIGHT_TOGGLE
        if (proc->led && proc->led->hook && proc->led->hook->sync_night_light_power_key) {
            proc->led->hook->sync_night_light_power_key(proc->led);
        }
        return;
#endif
    }
#endif
    LOG_WM("ignore power key event.");
}

void pump_start_key_event_handler(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS ||
        proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM ||
        proc->current_ui_type == PUMP_UI_TYPE_BREAK_TIP ||
        proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH ||
        proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD ||
        proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST ||
        proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST ||
        proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) {
        LOG_WM("settings/confirm/break-tip/will-finish/stop-hold/level-adjust/speed-adjust ui, ignore start key.");
        return;
    }
    switch_pump_status_ui(proc);
}
