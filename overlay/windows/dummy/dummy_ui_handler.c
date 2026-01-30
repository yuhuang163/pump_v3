/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_dummy.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define DUMMY_DISPLAY_TASK_NAME              "dummy_display_cmp"

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
static void dummy_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    LOG_UI("dummy play finish.\n");
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    uint8_t airplane_mode_state = 0;
    if (wm->is_current_window(wm, ID_WINDOW_DUMMY) == 0) {
        return;
    }
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_LOGO;
    wm->dispatch_new_window_args(wm, &msg);
    return;
}

static void dummy_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void dummy_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_dummy_process(ui);
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= DUMMY_ANIMATION_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(dummy_display_completed_cb, proc, DUMMY_DISPLAY_TASK_NAME);
    }
}

static void dummy_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            dummy_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            dummy_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void dummy_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_dummy_prepare(ui);
}

static void dummy_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_dummy_remove(ui);
}

static void dummy_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(DUMMY_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_dummy_night_light_onoff_ui(ui_proc_dummy_t *proc)
{
    EXCEPTION_CHECK_INT(proc->led && proc->led->hook);
    if (proc->led->hook->toggle_night_light) {
        proc->led->hook->toggle_night_light(proc->led);
    }
    return 0;
}

void start_dummy_ui_animation(ui_proc_dummy_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == DUMMY_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "dummy_screen",
        .fps = DUMMY_ANIMATION_FREQ,
        .progress_cb = dummy_animation_progress_cb,
        .display_prepare_cb = dummy_ui_display_prepare,
        .display_remove_cb = dummy_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = DUMMY_UI_TYPE_HOME;
}

void stop_dummy_ui_animation(ui_proc_dummy_t *proc)
{
    dummy_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}