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
static void system_will_power_off_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void system_will_power_off_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_will_power_off_process(ui);
}

static void system_will_power_off_ui_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            system_will_power_off_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            system_will_power_off_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void system_will_power_off_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_will_power_off_prepare(ui);
}

static void system_will_power_off_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_will_power_off_remove(ui);
}

static void show_system_will_power_off_ui(ui_proc_popup_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui);
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_overlay_init);
    ui_play_info_t play_info = {
        .tag = "low_power_screen",
        .fps = POPUP_ANIMATION_FREQ,
        .progress_cb = system_will_power_off_ui_animation_progress_cb,
        .display_prepare_cb = system_will_power_off_ui_display_prepare,
        .display_remove_cb = system_will_power_off_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_overlay_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_system_will_power_off_ui_animation(ui_proc_popup_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    LOG_UI("start system will power off ui animation.");
    show_system_will_power_off_ui(proc);
}