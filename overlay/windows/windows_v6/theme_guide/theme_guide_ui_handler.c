/*
* INCLUDE FILES
*****************************************************************************************
*/
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
    proc->popup_ui_type = ID_POPUP_UI_NONE;
    if (proc->res) {
        proc->res->set_sys_theme_type(proc->selected_index == 0 ? 
            RES_THEME_TYPE_BASE : RES_THEME_TYPE_CUSTOM);
    }

    multi_func_msg_data_t msg = {0};
    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        LOG_WM("vbus plug in, goto charge screen");
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

#if IS_SUPPORT_GUIDE_WINDOW
    if (is_need_show_guide_window(proc->wm)) {
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#else
    if (proc->wm->is_window_guidance_deployed() == 0) {
        proc->wm->set_window_guidance_deployed_flag(true);
    }
#endif

    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_MODE_UNSELECT;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void theme_guide_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    ui_proc_theme_guide_t *proc = (ui_proc_theme_guide_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_THEME_GUIDE);
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    POST_WM_TASK_HANDLER_NO_DELAY(theme_guide_display_completed_cb, proc, THEME_GUIDE_DISPLAY_TASK_NAME);

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
            theme_guide_popup_ui_finish_policy(ui, index);
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

