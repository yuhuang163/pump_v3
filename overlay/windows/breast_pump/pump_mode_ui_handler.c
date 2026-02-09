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
#define PUMP_MODE_DISPLAY_TASK_NAME                     "pump_mode_display_cmp"

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
static void pump_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
        show_pump_home_ui(proc);
    }
}

static void pump_mode_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pump_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_pump_mode_process(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(pump_mode_display_completed_cb, proc, PUMP_MODE_DISPLAY_TASK_NAME);
    }
}

static void pump_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            pump_mode_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_mode_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_mode_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_mode_prepare(ui);
}

static void pump_mode_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_mode_remove(ui);
}

static void show_pump_mode_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "pump_mode_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_mode_animation_progress_cb,
        .display_prepare_cb = pump_mode_ui_display_prepare,
        .display_remove_cb = pump_mode_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_COMMON_MODE;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_pump_common_mode_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, 
                                breast_pump_info_t *pump_info)
{
    if (proc->info.common_mode < BREAST_PUMP_MIX_MODE) {
        proc->info.common_mode++;
    } else {
        proc->info.common_mode = BREAST_PUMP_STIMULATION_MODE;
    }
    pump->set_breast_pump_mode(proc->info.common_mode, 1);
    proc->current_ui_mode = proc->info.common_mode;
    show_pump_mode_ui(proc);
    LOG_UI("switch pump common mode: %d, mode: %s", proc->info.common_mode, pump->get_breast_pump_mode_name(proc->info.common_mode));
    return 0;
}

void pump_mode_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_MODE_DISPLAY_TASK_NAME);
}