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
        case ID_POPUP_WINDOW_LOW_POWER:
            LOG_UI("start low power ui animation.");
            start_low_power_ui_animation(proc);
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
