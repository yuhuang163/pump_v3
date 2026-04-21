/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_DISPLAY_TASK_NAME              "home_display_cmp"

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
static void home_popup_ui_finish_policy(ui_manager_context_t *ui, int index)
{
    LOG_WM("===> popup %d ui display complete.", index);
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
}

static void home_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_home_process(ui);
}

static void home_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            home_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_prepare(ui);
}

static void home_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc)
{    
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);  
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_HOME) {
        return;
    }
    
    if (proc->pump_priming) {
        proc->pump_priming = false;
        if (proc->led)  {
            proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        }
    }

    ui_play_info_t play_info = {
        .tag = "home_default_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_animation_progress_cb,
        .display_prepare_cb = home_ui_display_prepare,
        .display_remove_cb = home_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = HOME_UI_TYPE_HOME;
}