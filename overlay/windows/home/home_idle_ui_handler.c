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
#define HOME_IDLE_DISPLAY_TASK_NAME              "home_idle_display_cmp"

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
static void home_idle_popup_ui_update(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    switch (popup_ui_index) {
        case HOME_UI_TYPE_HOME_WILL_POWER_OFF:
            //已经显示了倒计时UI，触发关机事件
            SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_OFF);
            break;
        default:
            break;
    }
    proc->popup_ui_type = HOME_UI_TYPE_NONE;
}
static void home_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_home_screen_saver_process(ui);
}

static void home_idle_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
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
            home_idle_popup_ui_update(ui, index);
            break;
        default:
            break;
    }
}

static void home_idle_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_screen_saver_prepare(ui);
}

static void home_idle_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_screen_saver_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_home_idle_ui_animation(ui_proc_home_t *proc)
{    
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == HOME_UI_TYPE_HOME_IDLE) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "home_idle_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_idle_animation_progress_cb,
        .display_prepare_cb = home_idle_ui_display_prepare,
        .display_remove_cb = home_idle_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = HOME_UI_TYPE_HOME_IDLE;
}
