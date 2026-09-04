/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_popup.h"
#include "popup_sub_ui.h" // IWYU pragma: keep

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
int popup_overlay_priority_get(uint16_t popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_TRAVEL_LOCK_COUNTDOWN:
            return POPUP_OVERLAY_PRIO_TRAVEL_LOCK;
        case ID_POPUP_UI_LOW_POWER:
        case ID_POPUP_UI_WILL_POWER_OFF:
            return POPUP_OVERLAY_PRIO_LOW_POWER;
        case ID_POPUP_UI_BT_CONNECT:
        case ID_POPUP_UI_BT_DISCONNECT:
        case ID_POPUP_UI_BT_CONNECTING:
        case ID_POPUP_UI_DEVICE_BIND:
        case ID_POPUP_UI_DEVICE_UNBIND:
            return POPUP_OVERLAY_PRIO_BLE;
        default:
            return POPUP_OVERLAY_PRIO_NONE;
    }
}

bool popup_overlay_can_preempt(uint16_t current_type, uint16_t new_type)
{
    int current_prio = popup_overlay_priority_get(current_type);
    int new_prio = popup_overlay_priority_get(new_type);

    if (new_prio > current_prio) {
        return true;
    }
    if ((new_prio == current_prio) && (new_type != current_type) && (new_prio > POPUP_OVERLAY_PRIO_NONE)) {
        return true;
    }
    return false;
}

void start_popup_ui_animation(ui_proc_popup_t *proc)
{
    EXCEPTION_CHECK_VOID(proc->ui);
    switch (proc->popup_window_id) {
        case ID_POPUP_UI_TRAVEL_LOCK_COUNTDOWN:
            start_travel_lock_countdown_ui_animation(proc);
            break;
        case ID_POPUP_UI_LOW_POWER:
            start_low_power_ui_animation(proc);
            break;
        case ID_POPUP_UI_BT_CONNECT:
        case ID_POPUP_UI_BT_DISCONNECT:
        case ID_POPUP_UI_BT_CONNECTING:
        case ID_POPUP_UI_DEVICE_BIND:
        case ID_POPUP_UI_DEVICE_UNBIND:
            start_ble_status_ui_animation(proc, proc->popup_window_id);
            break;
        default:
            LOG_UI("unknow popup window id: %d, ignore.", proc->popup_window_id);
            break;
    }
}

void stop_popup_ui_animation(ui_proc_popup_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_overlay_deinit);
    ui->hook->ui_animation_overlay_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void popup_request_close_overlay(ui_proc_popup_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->wm);    
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("popup not running, ignore close.");
        return;
    }
    proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
    if (proc->extra.popup_exit_cb) {
        proc->extra.popup_exit_cb(proc->extra.user_data, 0, 0);
    }
    proc->wm->close_popup_overlay(proc->wm);
}

void popup_window_exit_handler_cb(ui_proc_popup_t *proc)
{
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POPUP_WINDOW);
    popup_request_close_overlay(proc);
}