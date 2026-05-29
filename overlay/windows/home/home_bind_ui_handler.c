/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "windows_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_BIND_DISPLAY_TASK_NAME              "home_bind_display_cmp"

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
static void home_bind_popup_ui_finish_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    LOG_WM("===> popup %d ui display complete.", index);
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
}

static void home_bind_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    LOG_WM("home bind finish...");
    if (proc->current_ui_type == HOME_UI_TYPE_BIND_QRCODE) {
        start_home_ui_animation(proc);
    }
}

static void home_bind_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    ui_display_home_bind_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= HOME_BIND_QRCODE_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        POST_WM_TASK_HANDLER_NO_DELAY(home_bind_display_completed_cb, proc, HOME_BIND_DISPLAY_TASK_NAME);
    }
}

static void home_bind_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_bind_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            home_bind_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_bind_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_bind_prepare(ui);
}

static void home_bind_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_bind_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_home_bind_ui_animation(ui_proc_home_t *proc)
{    
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_BIND_QRCODE) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "home_bind_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_bind_animation_progress_cb,
        .display_prepare_cb = home_bind_ui_display_prepare,
        .display_remove_cb = home_bind_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = HOME_UI_TYPE_BIND_QRCODE;
}

void home_bind_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_BIND_DISPLAY_TASK_NAME);
}