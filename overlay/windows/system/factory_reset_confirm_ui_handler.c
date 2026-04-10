/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h"
#include "key_input_manager_api.h"
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "window_screen_system.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define FACTORY_RESET_CONFIRM_TASK_NAME             "factory_reset_confirm_task"

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
static void factory_reset_confirm_ui_finish_policy(ui_manager_context_t *ui, int index)
{
    EXCEPTION_CHECK_VOID(ui);
    ui_proc_system_t *proc = (ui_proc_system_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    system_factory_reset_handle(proc);
}

static void factory_reset_confirm_ui_after_policy(ui_manager_context_t *ui, int index)
{
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_confirm_process(ui);
}

static void factory_reset_confirm_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            factory_reset_confirm_ui_finish_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            factory_reset_confirm_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void factory_reset_confirm_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_confirm_prepare(ui);
}

static void factory_reset_confirm_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_confirm_remove(ui);
}

static void show_factory_reset_confirm_ui(ui_proc_system_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == SYSTEM_UI_TYPE_FACTORY_RESET_CONFIRM) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "factory_reset_confirm_screen",
        .fps = SYSTEM_ANIMATION_FREQ,
        .progress_cb = factory_reset_confirm_animation_progress_cb,
        .display_prepare_cb = factory_reset_confirm_ui_display_prepare,
        .display_remove_cb = factory_reset_confirm_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = SYSTEM_UI_TYPE_FACTORY_RESET_CONFIRM;
}

static void factory_reset_confirm_keyevent_cb(hdr_post_args_t *args)
{
    ui_proc_system_t *proc = (ui_proc_system_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_SYSTEM);
    multi_func_msg_data_t msg = {0};
    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void system_factory_reset_confirm_handle(ui_proc_system_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    show_factory_reset_confirm_ui(proc);
}

void system_factory_reset_confirm_keyevent_handler(ui_proc_system_t *proc, uint16_t keycode, uint16_t keytype)
{
    EXCEPTION_CHECK_VOID(proc);
    if (keycode != KEY_CODE_POWER) {
        return;
    }

    if (keytype == KEY_EVENT_LONG_2S) {
        proc->display_fac_rst_confirm_progress = true;
    } else if (keytype == KEY_EVENT_CLICK) {
        POST_TASK_HANDLER_NO_DELAY(factory_reset_confirm_keyevent_cb, proc,
            FACTORY_RESET_CONFIRM_TASK_NAME);
        proc->display_fac_rst_confirm_progress = false;
    } else if (keytype == KEY_EVENT_UP) {
        proc->display_fac_rst_confirm_progress = false;
    }
}

void factory_reset_confirm_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(FACTORY_RESET_CONFIRM_TASK_NAME);
}