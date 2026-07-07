/*
* INCLUDE FILES
*****************************************************************************************
*/
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
    pump_mode_adjust_exit_handler(proc);
    pump_level_adjust_exit_handler(proc);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void pump_ui_change_hdr_remove(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
}

void pump_popup_ui_type_update(ui_proc_pump_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}

void pump_sub_ui_exit_handler(ui_proc_pump_t *proc)
{
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_FINISH || pump_status == BREAST_PUMP_IDLE) {
        LOG_WM("goto home window.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
    } else {
        show_pump_home_ui(proc);
    }
}

void pump_power_key_event_handler(ui_proc_pump_t *proc)
{
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

void pump_cancel_key_event_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->exit_current_ui = true;
    int mode_adjust_result = pump_mode_adjust_exit_handler(proc);
    int level_adjust_result = pump_level_adjust_exit_handler(proc);
    if (mode_adjust_result == 0 || level_adjust_result == 0) {
        return;
    }
    LOG_WM("cancel key event, popup ui type:%d.", proc->popup_ui_type);
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        /// 触发泵奶结束埋点
        DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END, DATA_BURY_END_REASON_MANUAL, 0);
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        pump_complete_event_handler(proc);
        return;
    } else if (proc->popup_ui_type == ID_POPUP_UI_PUMP_WILL_FINISH_TIP) {
        /// 触发泵奶结束埋点
        DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END, DATA_BURY_END_REASON_MANUAL, 0);
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        pump_complete_event_handler(proc);
        return;
    }
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (PUMP_NO_FINISH(pump_status)) {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_FINISH_MANUAL);
    } else {
        LOG_WM("pump status:%d, ignore cancel key event.", pump_status);
    }
}

void pump_start_key_event_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        return;
    } else if (proc->popup_ui_type == ID_POPUP_UI_PUMP_WILL_FINISH_TIP) {
        int work_mode = 0;
        READ_PUMP_WORK_MODE(proc->pump, work_mode);
        if (PUMP_IS_COMMON_MODE(work_mode)) {
            LOG_WM("common mode, switch to lactation mode.");
        }
        WRITE_PUMP_WORK_MODE(proc->pump, BREAST_PUMP_LACTATION_MODE, true, 1);
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        return;
    }
    switch_pump_status_ui(proc);
}

void pump_mode_key_event_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_WILL_FINISH_TIP || 
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        LOG_WM("ignore mode key event.");
        return;
    }
    pump_mode_adjust_handler(proc, false);
}

void pump_breastfeeding_key_event_handler(ui_proc_pump_t *proc)
{

}

void pump_level_control_key_event_handler(ui_proc_pump_t *proc, uint8_t is_left)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_WILL_FINISH_TIP ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        LOG_WM("ignore right control key event.");
        return;
    }
    pump_level_adjust_type_update(proc, is_left);
}

void pump_knob_event_cb(ui_proc_pump_t *proc, uint8_t is_left)
{
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_TIMEOUT ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_LOW_POWER) {
        LOG_WM("pump finish timeout, ignore knob event.");
        return;
    }

#if (IS_ALLLOW_PUMP_COMPLETE_BREAK == 0)
    if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        LOG_WM("pump will finish, ignore knob event.");
        return;
    }
#endif

    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_WILL_FINISH_TIP ||
       proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        LOG_WM("ignore knob event.");
        return;
    }

    pump_level_adjust_handler(proc, is_left ? false : true);
}
