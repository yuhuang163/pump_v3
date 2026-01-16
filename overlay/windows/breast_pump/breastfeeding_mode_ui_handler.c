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
#define BREASTFEEDING_MODE_DISPLAY_TASK_NAME       "bf_mode_display_cmp"

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
static void breastfeeding_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP) == 0) {
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        show_pump_home_ui(proc);
    }
}

static void breastfeeding_mode_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void breastfeeding_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_breastfeeding_mode(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(breastfeeding_mode_display_completed_cb, proc, BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
    }
}

static void breastfeeding_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            breastfeeding_mode_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            breastfeeding_mode_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void breastfeeding_mode_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_breastfeeding_mode_prepare(ui);
}

static void breastfeeding_mode_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_breastfeeding_mode_remove(ui);
}

static void show_breastfeeding_mode_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "bf_mode_screen",
        .fps = PUMP_MODE_ANIMATION_FREQ,
        .progress_cb = breastfeeding_mode_animation_progress_cb,
        .display_prepare_cb = breastfeeding_mode_ui_display_prepare,
        .display_remove_cb = breastfeeding_mode_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_BREASTFEEDING_MODE;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_breastfeeding_mode_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump,
                        breast_pump_info_t *pump_info)
{
    if (proc->info.breastfeeding_mode < BREAST_PUMP_CUSTOM_MODE || proc->info.breastfeeding_mode >= BREAST_PUMP_LACTATION_START_MODE) {
        proc->info.breastfeeding_mode = BREAST_PUMP_CUSTOM_MODE;
    } else {
        proc->info.breastfeeding_mode++;
    }
    pump->set_breast_pump_mode(proc->info.breastfeeding_mode);
    proc->current_ui_mode = proc->info.breastfeeding_mode;
    show_breastfeeding_mode_ui(proc);
    LOG_UI("switch pump breastfeeding mode: %d, mode: %s", proc->info.breastfeeding_mode, 
        pump->get_breast_pump_mode_name(proc->info.breastfeeding_mode));
    return 0;
}

void breastfeeding_mode_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
}