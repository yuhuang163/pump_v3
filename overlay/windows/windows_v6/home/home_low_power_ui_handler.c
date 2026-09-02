/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_LOW_POWER_DISPLAY_TIMEOUT_MS               (3000)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_low_power_back_home(ui_proc_home_t *proc)
{
    home_ui_nav_back(proc, HOME_UI_NAV_BACK_HOME_RESET_PAGE);
}

static void home_low_power_complete_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->current_ui_type != HOME_UI_TYPE_LOW_POWER) {
        LOG_UI("low power ui, ignore complete.");
        return;
    }
    LOG_WM("home low power countdown complete, back to home.");
    home_low_power_back_home(proc);
}

static void home_low_power_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    ui_display_battery_low_power_process(ui);
}

static void home_low_power_animation_progress_cb(void *user_data, int index,
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
            home_low_power_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_low_power_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_battery_low_power_prepare(ui, 1, HOME_LOW_POWER_PAGE_TEXT);
}

static void home_low_power_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_battery_low_power_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_home_low_power_ui(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_LOW_POWER) {
        return;
    }
    home_ui_change_hdr_remove(proc);

    proc->current_ui_type = HOME_UI_TYPE_LOW_POWER;
    ui_play_info_t play_info = {
        .tag = "home_low_power_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_low_power_animation_progress_cb,
        .display_prepare_cb = home_low_power_ui_display_prepare,
        .display_remove_cb = home_low_power_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    POST_WM_TASK_HANDLER_DELAYED_ARG1(home_low_power_complete_cb,
        HOME_LOW_POWER_DISPLAY_TIMEOUT_MS, proc, HOME_LOW_POWER_COMPLETE_TASK_NAME);
}

void home_low_power_ui_exit(ui_proc_home_t *proc)
{
    (void)proc;
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_LOW_POWER_COMPLETE_TASK_NAME);
}

void home_low_power_ui_touch(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_LOW_POWER) {
        return;
    }
    LOG_WM("home low power ui key, refresh countdown.");
    ui_display_battery_low_power_restart_countdown();
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_LOW_POWER_COMPLETE_TASK_NAME);
    POST_WM_TASK_HANDLER_DELAYED_ARG1(home_low_power_complete_cb,
        HOME_LOW_POWER_DISPLAY_TIMEOUT_MS, proc, HOME_LOW_POWER_COMPLETE_TASK_NAME);
}

void home_low_power_ui_prepare_action(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_LOW_POWER) {
        return;
    }
    LOG_WM("prepare home low power for action, skip home flash.");
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_LOW_POWER_COMPLETE_TASK_NAME);
    proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
    home_ui_nav_clear(proc);
    /// 逻辑切到 HOME，不播放首页动画，由后续功能页直接接管画面
    proc->current_ui_type = HOME_UI_TYPE_HOME;
}

void home_low_power_ui_dismiss(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_LOW_POWER) {
        return;
    }
    LOG_WM("dismiss home low power ui by key, back to home.");
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_LOW_POWER_COMPLETE_TASK_NAME);
    home_low_power_back_home(proc);
}
