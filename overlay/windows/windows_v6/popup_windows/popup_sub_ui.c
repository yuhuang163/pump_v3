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