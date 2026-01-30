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
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP) == 0) {
        return;
    }
    
    if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        show_pump_home_ui(proc);
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
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
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
    }
    ui_play_info_t play_info = {
        .tag = "pump_result_screen",
        .fps = PUMP_RESULT_ANIMATION_FREQ,
        .progress_cb = pump_result_animation_progress_cb,
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
int switch_pump_status_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, int status)
{
    if (status == BREAST_PUMP_PAUSE) {
        LOG_UI("pump status:PAUSE => set_breast_pump_status:CONTINUE");
        pump->set_breast_pump_status(BREAST_PUMP_CONTINUE);
    } else if (status == BREAST_PUMP_IDLE || status == BREAST_PUMP_FINISH) {
        LOG_UI("first enter: ->current_ui_mode: %d", proc->current_ui_mode);
        pump->set_breast_pump_status(BREAST_PUMP_START);
    } else {
        LOG_UI("pump status:%d => set_breast_pump_status:PAUSE", pump->get_breast_pump_status_name(status));
        pump->set_breast_pump_status(BREAST_PUMP_PAUSE);
    }
    show_pump_home_ui(proc);
    return 0;
}

void show_pump_progress_time_policy(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump)
{
    EXCEPTION_CHECK_VOID(pump);
    int pump_status = pump->get_breast_pump_status_async();
    LOG_UI("pump status: %s", pump->get_breast_pump_status_name(pump_status));
    if (PUMP_IS_GOING(pump_status)) {
        LOG_UI("=>set_breast_pump_status:FINISH");
        pump->set_breast_pump_status(BREAST_PUMP_FINISH);
    }

    if (IS_PUMPTIME_LESS_THEN_30S(&proc->pump_result)) {
        LOG_UI("PUMPTIME < 30s, do not loop show!");
    } else {
        show_pump_result_loop(proc);
    }
}

void pump_result_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_COMPLETE_DISPLAY_TASK_NAME);
}
