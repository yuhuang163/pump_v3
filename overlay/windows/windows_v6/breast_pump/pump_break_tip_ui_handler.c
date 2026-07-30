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
#define PUMP_BREAK_TIP_DISPLAY_TASK_NAME                "pump_break_tip_display"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_break_tip_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_BREAK_TIP) {
        LOG_UI("break tip ui, ignore display completed.");
        return;
    }
    LOG_WM("pump break tip timeout, goto break end ui.");
    show_pump_break_end_ui(proc);
}

static void pump_break_tip_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_break_tip_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_BREAK_TIP_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("break tip ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(pump_break_tip_display_completed_cb, proc,
            PUMP_BREAK_TIP_DISPLAY_TASK_NAME);
    }
}

static void pump_break_tip_animation_progress_cb(void *user_data, int index,
    ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_break_tip_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_break_tip_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_break_tip_prepare(ui);
}

static void pump_break_tip_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_break_tip_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_break_tip_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_BREAK_TIP) {
        return;
    }
    pump_ui_change_hdr_remove(proc);

    ui_play_info_t play_info = {
        .tag = "pump_break_tip_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_break_tip_animation_progress_cb,
        .display_prepare_cb = pump_break_tip_ui_display_prepare,
        .display_remove_cb = pump_break_tip_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_BREAK_TIP;
}

void pump_break_tip_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BREAK_TIP_DISPLAY_TASK_NAME);
}
