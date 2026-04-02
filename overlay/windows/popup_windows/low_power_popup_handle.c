/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_popup.h"

/*
* DEFINES
*****************************************************************************************
*/
#define LOW_POWER_POP_DISPLAY_COMPLETE_TASK_NAME        "low_power_power_off_task"

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
static void low_power_pop_display_complete_cb(hdr_post_args_t *args)
{
    ui_proc_popup_t *proc = (ui_proc_popup_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POPUP_WINDOW);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    pm->system_power_control(false);
}

static void low_power_pop_display_complete(ui_manager_context_t *ui, int index)
{
    EXCEPTION_CHECK_VOID(ui);
    ui_proc_popup_t *proc = (ui_proc_popup_t *)ui->priv.overlay_play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    EXCEPTION_CHECK_VOID(battery);
    if (battery->is_low_battery() == 0) {
        LOGW("low power pop display complete, but not low battery, ignore.");
        if (proc->wm->close_popup_overlay) {
            proc->wm->close_popup_overlay(proc->wm, &proc->extra);
        }
        return;
    }
    POST_TASK_HANDLER_NO_DELAY(low_power_pop_display_complete_cb, proc, LOW_POWER_POP_DISPLAY_COMPLETE_TASK_NAME);
}

static void low_power_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void low_power_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_low_power_popup_process(ui);
}

static void low_power_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            low_power_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            low_power_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            low_power_pop_display_complete(ui, index);
            break;
        default:
            break;
    }
}

static void low_power_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_low_power_popup_prepare(ui);
}

static void low_power_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_low_power_popup_remove(ui);
}

static void show_low_power_ui(ui_proc_popup_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui);
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_overlay_init);
    ui_play_info_t play_info = {
        .tag = "low_power_pop_screen",
        .fps = POPUP_ANIMATION_FREQ,
        .progress_cb = low_power_animation_progress_cb,
        .display_prepare_cb = low_power_ui_display_prepare,
        .display_remove_cb = low_power_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_overlay_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_low_power_ui_animation(ui_proc_popup_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    LOG_UI("start low power pop ui animation.");
    show_low_power_ui(proc);
}