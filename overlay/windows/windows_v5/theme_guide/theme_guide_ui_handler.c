/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h"
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_theme_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define THEME_GUIDE_DISPLAY_TASK_NAME              "theme_guide_display_cmp"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void theme_guide_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_theme_guide_t *proc = (ui_proc_theme_guide_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_THEME_GUIDE);
    multi_func_msg_data_t msg = {0};

    if (proc->storage) {
        proc->storage->put_store_ram_val(STORE_ID_SYS_UI_STYLE, proc->selected_index, 0, NULL);
    }

#if IS_SUPPORT_GUIDE_WINDOW
    if (proc->wm->is_window_guidance_deployed() == 0) {
        LOG_WM("theme guide confirmed, goto guide window.");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#endif

    LOG_WM("theme guide confirmed, goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void theme_guide_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_theme_guide_t *proc = (ui_proc_theme_guide_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_THEME_GUIDE);
    ui_display_theme_guide_process(ui);
}

static void theme_guide_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            theme_guide_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void theme_guide_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_theme_guide_prepare(ui);
}

static void theme_guide_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_theme_guide_remove(ui);
}

static void theme_guide_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(THEME_GUIDE_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_theme_guide_ui_animation(ui_proc_theme_guide_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == THEME_GUIDE_UI_TYPE_HOME) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "theme_guide_screen",
        .fps = THEME_GUIDE_ANIMATION_FREQ,
        .progress_cb = theme_guide_animation_progress_cb,
        .display_prepare_cb = theme_guide_ui_display_prepare,
        .display_remove_cb = theme_guide_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = THEME_GUIDE_UI_TYPE_HOME;
}

void stop_theme_guide_ui_animation(ui_proc_theme_guide_t *proc)
{
    theme_guide_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void theme_guide_key_event_handler(ui_proc_theme_guide_t *proc, uint16_t keycode)
{
    if (keycode != KEY_CODE_START || proc->is_operated == 0) {
        return;
    }
    POST_WM_TASK_HANDLER_NO_DELAY(theme_guide_display_completed_cb, proc, THEME_GUIDE_DISPLAY_TASK_NAME);
}

void theme_guide_knob_event_handler(ui_proc_theme_guide_t *proc, bool is_left)
{
    proc->is_operated = 1;
    if (is_left) {
        if (proc->selected_index > THEME_GUIDE_SELECT_SIMPLE) {
            proc->selected_index--;
        }
    } else if (proc->selected_index + 1 < THEME_GUIDE_SELECT_MAX) {
        proc->selected_index++;
    }
}
