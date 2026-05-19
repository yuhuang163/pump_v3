/*
* INCLUDE FILES
*****************************************************************************************
*/ 
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define OTA_DISPLAY_TASK_NAME              "ota_display_cmp"

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
static void ota_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_ota_t *proc = (ui_proc_ota_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_OTA_UPGRADE);
    EXCEPTION_CHECK_VOID(proc->wm);
    multi_func_msg_data_t msg = {0};
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
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }    
    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void ota_display_completed_policy(ui_manager_context_t *ui)
{
    EXCEPTION_CHECK_VOID(ui);
    ui_proc_ota_t *proc = (ui_proc_ota_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != OTA_UI_TYPE_UPDATE) {
        return;
    }
    proc->current_ui_type = OTA_UI_TYPE_NONE;
    POST_WM_TASK_HANDLER_NO_DELAY(ota_display_completed_cb, proc, OTA_DISPLAY_TASK_NAME);
}

static void ota_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_ota_process(ui);
}

static void ota_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            ota_display_completed_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            ota_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void ota_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_ota_prepare(ui);
}

static void ota_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_ota_remove(ui);
}

static void show_ota_ui(ui_proc_ota_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == OTA_UI_TYPE_UPDATE) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "ota_screen",
        .fps = OTA_ANIMATION_FREQ,
        .progress_cb = ota_animation_progress_cb,
        .display_prepare_cb = ota_ui_display_prepare,
        .display_remove_cb = ota_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = OTA_UI_TYPE_UPDATE;
}

static void ota_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(OTA_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_ota_ui_animation(ui_proc_ota_t *proc)
{
    EXCEPTION_CHECK_VOID(proc->pm);
    while (get_timestamp_ms(0) < BOOT_LONG_PRESS_TIME) {
        if (proc->pm->is_system_power_on()) {
            break;
        }
        msleep(10);
    }
    LOG_UI("start ota ui animation.");
    show_ota_ui(proc);
}

void stop_ota_ui_animation(ui_proc_ota_t *proc)
{
    ota_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}