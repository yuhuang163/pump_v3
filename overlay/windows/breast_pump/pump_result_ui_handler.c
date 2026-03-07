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
#define PUMP_COMPLETE_DISPLAY_TASK_NAME         "pump_complete_display"

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
static void pump_result_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        multi_func_msg_data_t msg = {0};
        LOG_WM("goto home window.");
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
    }
}

static void pump_result_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pump_result_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_pump_complete_tip_process(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_RESULT_DISPLAY_TIMEOUT) {
        POST_TASK_HANDLER_NO_DELAY(pump_result_display_completed_cb, proc, PUMP_COMPLETE_DISPLAY_TASK_NAME);
    }
}

static void pump_result_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            pump_result_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_result_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_result_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_complete_tip_prepare(ui);
}

static void pump_result_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_complete_tip_remove(ui);
}

static void show_pump_result_loop(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    } else {
        pump_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "pump_result_screen",
        .fps = PUMP_RESULT_ANIMATION_FREQ,
        .progress_cb = pump_result_animation_progress_cb,
        .display_prepare_cb = pump_result_ui_display_prepare,
        .display_remove_cb = pump_result_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_RESULT;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_result_ui(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->led);
    int pump_status = proc->pump->get_breast_pump_status_async();
    if (PUMP_NO_FINISH(pump_status)) {
        LOG_UI("=>set pump finish%d", pump_status);
        proc->led->set_led_level(PUMP_FREQ_SET_BL, 100);
        proc->pump->set_breast_pump_status(BREAST_PUMP_FINISH);
    } else {
        LOG_UI("pump status: %s", proc->pump->get_breast_pump_status_name(pump_status));
    }
    show_pump_result_loop(proc);
}

void pump_result_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_COMPLETE_DISPLAY_TASK_NAME);
}