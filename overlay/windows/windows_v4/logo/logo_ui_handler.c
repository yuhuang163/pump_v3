/*
* INCLUDE FILES
*****************************************************************************************
*/ 
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_logo.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define LOGO_DISPLAY_TASK_NAME              "logo_display_cmp"

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
static void logo_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_logo_t *proc = (ui_proc_logo_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_LOGO);
    LOG_UI("logo play finished");
    multi_func_msg_data_t msg = {0};
#if IS_SUPPORT_LVGL
    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->wm->is_window_factory_test_deployed() == 0) {
        LOG_WM("goto factory test screen");
        msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        LOG_WM("vbus plug in, goto charge screen");
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->airplane && proc->airplane->is_on_airplane_mode()) {
        LOG_WM("goto airplane mode screen");
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->wm->is_window_guidance_deployed() == 0) {
        #if 0
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
        #else
        proc->wm->set_window_guidance_deployed_flag(true);
        #endif
    }    
    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
#else
    if (proc->airplane && proc->airplane->is_on_airplane_mode()) {
        LOG_WM("goto airplane mode screen");
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
    LOG_WM("goto pump screen");
    msg.union_body.extra.window_id = ID_WINDOW_PUMP;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
#endif
}

static void logo_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_logo_t *proc = (ui_proc_logo_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_LOGO);
    ui_display_logo_process(ui);
#if (IS_SUPPORT_LOGO_UI_TEST == 0)
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= LOGO_ANIMATION_DISPLAY_TIME) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("logo ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(logo_display_completed_cb, proc, LOGO_DISPLAY_TASK_NAME);
    } 
#endif
}

static void logo_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            logo_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void logo_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_logo_prepare(ui);
}

static void logo_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_logo_remove(ui);
}

static void show_logo_ui(ui_proc_logo_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == LOGO_UI_TYPE_HOME) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "logo_screen",
        .fps = LOGO_ANIMATION_FREQ,
        .progress_cb = logo_animation_progress_cb,
        .display_prepare_cb = logo_ui_display_prepare,
        .display_remove_cb = logo_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = LOGO_UI_TYPE_HOME;
}

static void logo_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(LOGO_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_logo_ui_animation(ui_proc_logo_t *proc)
{
#if IS_SUPPORT_MODULE_RES
    resource_manager_context_t *res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    if (res) {
        LOG_WM("wait res ready");
        res->wait_init_completed(res);
    }
#endif
    LOG_UI("start logo ui animation.");
    show_logo_ui(proc);
}

void stop_logo_ui_animation(ui_proc_logo_t *proc)
{
    logo_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}