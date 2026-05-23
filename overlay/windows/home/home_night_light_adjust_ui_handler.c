/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_NIGHT_LIGHT_ADJUST_DISPLAY_TASK_NAME                     "home_night_light_adjust_display_cmp"

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
static void home_night_light_adjust_ui_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->current_ui_type != HOME_UI_TYPE_NIGHT_LIGHT_ADJUST) {
        return;
    }
    start_home_ui_animation(proc);
}

static void home_night_light_adjust_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (popup_ui_index == ID_POPUP_UI_OP_SUCCESS_PRESET) {
        POST_WM_TASK_HANDLER_NO_DELAY(home_night_light_adjust_ui_display_completed_cb,
            proc, HOME_NIGHT_LIGHT_ADJUST_DISPLAY_TASK_NAME);
    }
}

static void home_night_light_adjust_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_home_night_light_adjust_process(ui);
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= HOME_NIGHT_LIGHT_ADJUST_DISPLAY_TIMEOUT) {
        proc->level_change = 0;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("night light adjust ui timeout: %d", proc->last_progress_ani_time);
        POST_WM_TASK_HANDLER_NO_DELAY(home_night_light_adjust_ui_display_completed_cb,
            proc, HOME_NIGHT_LIGHT_ADJUST_DISPLAY_TASK_NAME);
    } else if (time_diff >= HOME_NIGHT_LIGHT_ADJUST_SYNC_TIMEOUT && proc->level_sync_flag) {
        proc->popup_ui_type = ID_POPUP_UI_OP_SUCCESS_PRESET;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("night light adjust ui sync timeout: %d", proc->last_progress_ani_time);
    }
}

static void home_night_light_adjust_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_night_light_adjust_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            home_night_light_adjust_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_night_light_adjust_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_night_light_adjust_prepare(ui);
}

static void home_night_light_adjust_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_night_light_adjust_remove(ui);
}



/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_home_night_light_adjust_ui(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_NIGHT_LIGHT_ADJUST) {
        return;
    } else {
        home_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "home_night_light_adjust_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_night_light_adjust_animation_progress_cb,
        .display_prepare_cb = home_night_light_adjust_ui_prepare,
        .display_remove_cb = home_night_light_adjust_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = HOME_UI_TYPE_NIGHT_LIGHT_ADJUST;
    ui->hook->ui_animation_play_init(ui, play_info);
}

void switch_home_night_light_adjust(ui_proc_home_t *proc, bool is_increase)
{
    show_home_night_light_adjust_ui(proc);
}

void home_night_light_adjust_ui_exit(ui_proc_home_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_NIGHT_LIGHT_ADJUST_DISPLAY_TASK_NAME);
}