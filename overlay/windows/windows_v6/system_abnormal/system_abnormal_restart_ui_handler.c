/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_system_abnormal.h"

#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD

/*
* DEFINES
*****************************************************************************************
*/
#define SYSTEM_ABNORMAL_RESTART_REBOOT_DELAY_MS         (1000)
#define SYSTEM_ABNORMAL_RESTART_REBOOT_TASK_NAME        "sys_abnormal_restart_reboot"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void system_abnormal_restart_reboot_cb(hdr_post_args_t *args)
{
    ui_proc_system_abnormal_t *proc = (ui_proc_system_abnormal_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    if (proc->current_ui_type != SYSTEM_ABNORMAL_UI_TYPE_RESTARTING) {
        LOG_WM("system abnormal restart ui, ignore reboot.");
        return;
    }
    LOG_WM("system abnormal restart page done, reboot.");
    msleep(20);
    platform_system_reboot();
}

static void system_abnormal_restart_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_system_abnormal_t *proc = (ui_proc_system_abnormal_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    ui_display_restart_process(ui);
}

static void system_abnormal_restart_animation_progress_cb(void *user_data, int index,
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
            system_abnormal_restart_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void system_abnormal_restart_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_restart_prepare(ui);
}

static void system_abnormal_restart_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_restart_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_system_abnormal_restart_ui(ui_proc_system_abnormal_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == SYSTEM_ABNORMAL_UI_TYPE_RESTARTING) {
        return;
    }

    LOG_WM("show system abnormal restart ui.");
    proc->restart_hold_holding = 0;
    proc->last_progress_ani_time = get_timestamp();
    REMOVE_TASK_HANDLER_DIRECTLY(SYSTEM_ABNORMAL_RESTART_REBOOT_TASK_NAME);

    ui_play_info_t play_info = {
        .tag = "restart_screen",
        .fps = SYSTEM_ABNORMAL_ANIMATION_FREQ,
        .progress_cb = system_abnormal_restart_animation_progress_cb,
        .display_prepare_cb = system_abnormal_restart_ui_display_prepare,
        .display_remove_cb = system_abnormal_restart_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = SYSTEM_ABNORMAL_UI_TYPE_RESTARTING;
    POST_TASK_HANDLER_DELAYED_ARG1(system_abnormal_restart_reboot_cb,
        SYSTEM_ABNORMAL_RESTART_REBOOT_DELAY_MS, proc, SYSTEM_ABNORMAL_RESTART_REBOOT_TASK_NAME);
}

void system_abnormal_restart_ui_exit(ui_proc_system_abnormal_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->restart_hold_holding = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(SYSTEM_ABNORMAL_RESTART_REBOOT_TASK_NAME);
}

#endif /* IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD */
