/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"
#include "windows_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_PUMP_RESULT_DISPLAY_TASK_NAME         "home_pump_result_display"

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
static void home_pump_result_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->current_ui_type != HOME_UI_TYPE_PUMP_RESULT) {
        LOG_UI("pump result ui, ignore display completed.");
        return;
    }
    
    if (proc->battery && proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        LOG_WM("charge error, goto charge screen");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    start_home_ui_animation(proc);
}

static void home_pump_result_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    ui_display_pump_complete_tip_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
#if V3_UX_VERSION == V3_UX_V1
    if (time_diff >= PUMP_RESULT_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("result ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_result_display_completed_cb, proc, HOME_PUMP_RESULT_DISPLAY_TASK_NAME);
    }
#else
    if (proc->exit_current_ui || time_diff >= PUMP_RESULT_DISPLAY_TIMEOUT) {
        proc->exit_current_ui = false;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("result ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_result_display_completed_cb, proc, HOME_PUMP_RESULT_DISPLAY_TASK_NAME);
    }
#endif
}

static void home_pump_result_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_pump_result_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_pump_result_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_complete_tip_prepare(ui);
}

static void home_pump_result_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_complete_tip_remove(ui);
}

static void show_home_pump_result_loop(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == HOME_UI_TYPE_PUMP_RESULT) {
        return;
    } else {
        home_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "home_pump_result_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_pump_result_animation_progress_cb,
        .display_prepare_cb = home_pump_result_ui_display_prepare,
        .display_remove_cb = home_pump_result_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = HOME_UI_TYPE_PUMP_RESULT;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_home_pump_result_ui(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (PUMP_NO_FINISH(pump_status)) {
        LOG_UI("=>set pump finish%d", pump_status);
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, 1);
    } else {
        char *state_name = "unknow";
        READ_PUMP_STATE_NAME(proc->pump, &state_name);
        LOG_UI("pump status: %s", state_name);
    }

    if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
        proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
    }

    #if V3_UX_VERSION == V3_UX_V2
    proc->exit_current_ui = false;
    #endif
    show_home_pump_result_loop(proc);
}

void home_pump_result_ui_exit(ui_proc_home_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_PUMP_RESULT_DISPLAY_TASK_NAME);
}