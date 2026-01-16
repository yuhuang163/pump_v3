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
    EXCEPTION_CHECK_VOID(proc);
    LOG_UI("logo play finished");
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_LOGO) == 0) {
        return;
    }
    multi_func_msg_data_t msg = {0};
#if IS_SUPPORT_LVGL
    uint8_t factory_deploy_flag = 0, guide_deploy_flag = 0, airplane_mode_state = 0;
    if(proc->st) {
        proc->st->get_store_ram_val(STORE_ID_SYS_AIRPLANE_MODE_STATE, 0, 0, &airplane_mode_state);
        if (airplane_mode_state == 1) {
            LOG_WM("goto airplane mode screen");
            msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
            wm->dispatch_new_window_args(wm, &msg);
            return;
        }

        proc->st->get_store_ram_val(STORE_ID_FACTORY_DEPLOY_FLAG, 0, 0, &factory_deploy_flag);
        if(factory_deploy_flag == 0) {
            LOG_WM("goto factory mode screen");
            msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
            wm->dispatch_new_window_args(wm, &msg);
            return;
        } 
        proc->st->get_store_ram_val(STORE_ID_SYS_GUIDE_DEPLOYED, 0, 0, &guide_deploy_flag);
        if(guide_deploy_flag == 0) {
            LOG_WM("goto guide screen");
            msg.union_body.extra.window_id = ID_WINDOW_HOME;
            wm->dispatch_new_window_args(wm, &msg);
            return;
        }
    }
    LOG_WM("goto pump screen");
    msg.union_body.extra.window_id = ID_WINDOW_PUMP;
    wm->dispatch_new_window_args(wm, &msg);
#else
    uint8_t airplane_mode_state = 0;
    if(proc->st) {
        proc->st->get_store_ram_val(STORE_ID_SYS_AIRPLANE_MODE_STATE, 0, 0, &airplane_mode_state);
        if (airplane_mode_state == 1) {
            LOG_WM("goto airplane mode screen");
            msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
            wm->dispatch_new_window_args(wm, &msg);
            return;
        }
    }
    LOG_WM("goto pump screen");
    msg.union_body.extra.window_id = ID_WINDOW_PUMP;
    wm->dispatch_new_window_args(wm, &msg);
#endif
}

static void logo_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void logo_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_logo_process(ui);
    ui_proc_logo_t *proc = (ui_proc_logo_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= LOGO_ANIMATION_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(logo_display_completed_cb, proc, LOGO_DISPLAY_TASK_NAME);
    } 
}

static void logo_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            logo_ui_before_policy(ui, index);
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
    if (proc->current_ui_type == LOGO_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
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
    proc->last_progress_ani_time = get_timestamp();
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
    EXCEPTION_CHECK_VOID(proc->pm);
    while (get_timestamp_ms(0) < BOOT_LONG_PRESS_TIME) {
        if (proc->pm->is_system_power_on()) {
            break;
        }
        msleep(10);
    }
    LOG_UI("start logo ui animation.");
    show_logo_ui(proc);
}

void stop_logo_ui_animation(ui_proc_logo_t *proc)
{
    logo_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}