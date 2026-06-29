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
static void sys_error_ui_display_complete(ui_manager_context_t *ui, int index)
{
    EXCEPTION_CHECK_VOID(ui);
    ui_proc_popup_t *proc = (ui_proc_popup_t *)ui->priv.overlay_play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POPUP_WINDOW);
    popup_window_exit_handler_cb(proc);
}

static void sys_error_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_popup_t *proc = (ui_proc_popup_t *)ui->priv.overlay_play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POPUP_WINDOW);
    popup_window_ui_process(ui, proc->current_ui_type);
}

static void sys_error_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;

    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            sys_error_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            sys_error_ui_display_complete(ui, index);
            break;
        default:
            break;
    }
}

static void sys_error_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_proc_popup_t *proc = (ui_proc_popup_t *)ui->priv.overlay_play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    popup_window_ui_prepare(ui, proc->current_ui_type, proc->extra.popup_exit_time);
}

static void sys_error_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    popup_window_ui_remove(ui);
}

static void show_sys_error_ui(ui_proc_popup_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui);
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_overlay_init);
    ui_play_info_t play_info = {
        .tag = "sys_error",
        .fps = POPUP_ANIMATION_FREQ,
        .progress_cb = sys_error_animation_progress_cb,
        .display_prepare_cb = sys_error_ui_display_prepare,
        .display_remove_cb = sys_error_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_overlay_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_sys_error_ui_animation(ui_proc_popup_t *proc, uint16_t popup_ui_type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->current_ui_type = popup_ui_type;
    show_sys_error_ui(proc);
}
