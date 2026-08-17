/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_system_abnormal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_OFF_HOLD_COMPLETE_TASK_NAME           "sys_abnormal_power_off_hold_complete"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void power_off_hold_complete_cb(hdr_post_args_t *args)
{
    ui_proc_system_abnormal_t *proc = (ui_proc_system_abnormal_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    if (proc->current_ui_type != SYSTEM_ABNORMAL_UI_TYPE_POWER_OFF_HOLD) {
        LOG_UI("system abnormal power off hold ui, ignore complete.");
        return;
    }
    LOG_WM("system abnormal power off hold complete, goto power off.");
    proc->power_off_hold_holding = 0;
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
    msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_MANUAL;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void power_off_hold_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_system_abnormal_t *proc = (ui_proc_system_abnormal_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    ui_display_power_off_hold_process(ui);
}

static void power_off_hold_animation_progress_cb(void *user_data, int index,
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
            power_off_hold_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            POST_WM_TASK_HANDLER_NO_DELAY(power_off_hold_complete_cb,
                ui->priv.play_info.user_data, POWER_OFF_HOLD_COMPLETE_TASK_NAME);
            break;
        default:
            break;
    }
}

static void power_off_hold_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_off_hold_prepare(ui);
}

static void power_off_hold_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_off_hold_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_system_abnormal_power_off_hold_ui(ui_proc_system_abnormal_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == SYSTEM_ABNORMAL_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }

    proc->power_off_hold_restore_ui_type = proc->current_ui_type;
    proc->last_progress_ani_time = get_timestamp();
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_OFF_HOLD_COMPLETE_TASK_NAME);

    ui_play_info_t play_info = {
        .tag = "sys_abnormal_power_off_hold_screen",
        .fps = SYSTEM_ABNORMAL_ANIMATION_FREQ,
        .progress_cb = power_off_hold_animation_progress_cb,
        .display_prepare_cb = power_off_hold_ui_display_prepare,
        .display_remove_cb = power_off_hold_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = SYSTEM_ABNORMAL_UI_TYPE_POWER_OFF_HOLD;
    proc->power_off_hold_holding = 1;
}

void system_abnormal_power_off_hold_ui_exit(ui_proc_system_abnormal_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->power_off_hold_holding = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_OFF_HOLD_COMPLETE_TASK_NAME);
}

void system_abnormal_power_off_hold_dismiss(ui_proc_system_abnormal_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != SYSTEM_ABNORMAL_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }
    LOG_WM("dismiss system abnormal power off hold, restore abnormal ui.");
    proc->power_off_hold_holding = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_OFF_HOLD_COMPLETE_TASK_NAME);

    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->current_ui_type = proc->power_off_hold_restore_ui_type;
    proc->last_progress_ani_time = get_timestamp();
    show_system_abnormal_ui(proc);
}

void system_abnormal_power_off_hold_key_long_2s_handler(ui_proc_system_abnormal_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type == SYSTEM_ABNORMAL_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }

    LOG_WM("power long 2s, show system abnormal power off hold ui.");
    show_system_abnormal_power_off_hold_ui(proc);
}

void system_abnormal_power_off_hold_key_up_handler(ui_proc_system_abnormal_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != SYSTEM_ABNORMAL_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }
    if (!proc->power_off_hold_holding) {
        return;
    }
    LOG_WM("power key up before system abnormal power off hold complete, dismiss.");
    system_abnormal_power_off_hold_dismiss(proc);
}
