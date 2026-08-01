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
static void home_idle_popup_ui_finish_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    LOG_WM("===> popup %d ui display complete.", index);
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
}

static void home_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    ui_display_home_screen_saver_process(ui);
}

static void home_idle_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            home_idle_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_idle_set_key_backlight(ui_proc_home_t *proc, uint8_t level)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
        proc->led->hook->set_key_backlight(proc->led, level);
    }
}

static void home_idle_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    /// 待机页：所有按键灯熄灭（覆盖 PM idle 广播里的半亮）
    home_idle_set_key_backlight(proc, 0);
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
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_HOME_IDLE) {
        home_idle_set_key_backlight(proc, 0);
        return;
    }
    home_idle_set_key_backlight(proc, 0);
    ui_play_info_t play_info = {
        .tag = "home_idle_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_idle_animation_progress_cb,
        .display_prepare_cb = home_idle_ui_display_prepare,
        .display_remove_cb = home_idle_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = HOME_UI_TYPE_HOME_IDLE;
}

void home_idle_ui_dismiss(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME_IDLE) {
        return;
    }
    LOG_WM("dismiss home idle, back home.");
    proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
    home_ui_nav_clear(proc);
    /// start_home_ui_animation 内会将按键灯全亮
    start_home_ui_animation(proc);
}
