/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_system_abnormal.h" // IWYU pragma: keep

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
static void system_abnormal_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_system_abnormal_t *proc = (ui_proc_system_abnormal_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    if (proc->current_ui_type == SYSTEM_ABNORMAL_UI_TYPE_BATTERY_TEMP_HIGH) {
        ui_display_system_abnormal_process(ui, SYS_ABNORMAL_BATTERY_TEMP_HIGH);
    } else if (proc->current_ui_type == SYSTEM_ABNORMAL_UI_TYPE_STORAGE_FAILURE) {
        ui_display_system_abnormal_process(ui, SYS_ABNORMAL_STORAGE_FAILURE);
    } else if (proc->current_ui_type == SYSTEM_ABNORMAL_UI_TYPE_COMMUNICATION_ERROR) {
        ui_display_system_abnormal_process(ui, SYS_ABNORMAL_COMMUNICATION_ERROR);
    }
}

static void system_abnormal_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            system_abnormal_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void system_abnormal_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_system_abnormal_prepare(ui);
}

static void system_abnormal_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_system_abnormal_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_system_abnormal_ui(ui_proc_system_abnormal_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == SYSTEM_ABNORMAL_UI_TYPE_NONE) {
        LOG_WM("no abnormal type to show, ignore.");
        return;
    }

    ui_play_info_t play_info = {
        .tag = "system_abnormal_screen",
        .fps = SYSTEM_ABNORMAL_ANIMATION_FREQ,
        .progress_cb = system_abnormal_animation_progress_cb,
        .display_prepare_cb = system_abnormal_ui_display_prepare,
        .display_remove_cb = system_abnormal_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
}

void stop_system_abnormal_ui(ui_proc_system_abnormal_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->current_ui_type = SYSTEM_ABNORMAL_UI_TYPE_NONE;
}
