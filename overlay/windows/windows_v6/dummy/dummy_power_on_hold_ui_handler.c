/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_dummy.h" // IWYU pragma: keep

#if IS_SUPPORT_POWER_ON_GUIDE_UI

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_ON_HOLD_COMPLETE_TASK_NAME            "dummy_power_on_hold_complete"
#define POWER_ON_HOLD_IDLE_TASK_NAME                "dummy_power_on_hold_idle"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void dummy_power_on_hold_direct_power_off(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *power_dev;
    int arg1[] = {0};

    EXCEPTION_CHECK_VOID(pbus);
    power_dev = pbus->find_device(ID_DEVICE_POWER);
    if (power_dev == NULL) {
        LOGE("power dev not found, reboot");
        msleep(50);
        platform_system_reboot();
        return;
    }
    LOG_WM("==========POWER_ON_HOLD IDLE POWER_OFF==========");
    for (uint8_t i = 0; i < 3; i++) {
        power_dev->ioctl(power_dev, OP_CODE_POWER_CONTROL, (void *)arg1);
        msleep(200);
        LOG_WM("power off try: %d", i);
    }
    platform_system_reboot();
}

static void power_on_hold_goto_charge_screen(ui_proc_dummy_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->wm);
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
    LOG_WM("adapter mismatch, goto charge screen");
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void power_on_hold_complete_cb(hdr_post_args_t *args)
{
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_DUMMY);
    if (proc->current_ui_type != DUMMY_UI_TYPE_POWER_ON_HOLD) {
        LOG_UI("power on hold ui, ignore complete.");
        return;
    }
    proc->power_on_hold_holding = 0;
    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        power_on_hold_goto_charge_screen(proc);
        return;
    }
    LOG_WM("power on hold complete, goto logo.");
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_LOGO;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void power_on_hold_idle_power_off_cb(hdr_post_args_t *args)
{
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_DUMMY);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore idle power off.");
        return;
    }
    if (proc->current_ui_type != DUMMY_UI_TYPE_POWER_ON_HOLD) {
        return;
    }
    if (proc->power_on_hold_holding) {
        LOG_WM("holding power on, ignore idle power off.");
        return;
    }
    /// 适配器不匹配：跳转不匹配页，不自动关机
    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        power_on_hold_goto_charge_screen(proc);
        return;
    }
    LOG_WM("power on hold idle timeout, direct power off.");
    dummy_power_on_hold_direct_power_off();
}

static void power_on_hold_idle_timeout_check(ui_proc_dummy_t *proc)
{
    if (proc->power_on_hold_holding) {
        // LOG_WM("power on hold holding, ignore idle timeout check.");
        return;
    }
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff < DUMMY_POWER_ON_HOLD_IDLE_TIMEOUT_MS) {
        // LOG_WM("time diff: %dms, ignore.", time_diff);
        return;
    }
    proc->last_progress_ani_time = get_timestamp();
    POST_WM_TASK_HANDLER_NO_DELAY(power_on_hold_idle_power_off_cb, proc,
        POWER_ON_HOLD_IDLE_TASK_NAME);
}

static void power_on_hold_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_DUMMY);
    ui_display_power_on_hold_process(ui);
    power_on_hold_idle_timeout_check(proc);
}

static void power_on_hold_animation_progress_cb(void *user_data, int index,
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
            power_on_hold_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            POST_WM_TASK_HANDLER_NO_DELAY(power_on_hold_complete_cb,
                ui->priv.play_info.user_data, POWER_ON_HOLD_COMPLETE_TASK_NAME);
            break;
        default:
            break;
    }
}

static void power_on_hold_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_on_hold_prepare(ui);
}

static void power_on_hold_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_on_hold_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_dummy_power_on_hold_ui(ui_proc_dummy_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == DUMMY_UI_TYPE_POWER_ON_HOLD) {
        return;
    }

    ui_play_info_t play_info = {
        .tag = "power_on_hold_screen",
        .fps = DUMMY_ANIMATION_FREQ,
        .progress_cb = power_on_hold_animation_progress_cb,
        .display_prepare_cb = power_on_hold_ui_display_prepare,
        .display_remove_cb = power_on_hold_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = DUMMY_UI_TYPE_POWER_ON_HOLD;
    proc->power_on_hold_holding = 0;
    if (proc->led && proc->led->hook && proc->led->hook->set_ui_backlight_level_rate) {
        proc->led->hook->set_ui_backlight_level_rate(proc->led, IDLE_BRIGHTNESS_MAX_VALUE);
    }
}

void stop_dummy_ui_animation(ui_proc_dummy_t *proc)
{
    dummy_power_on_hold_ui_hdr_remove();
    EXCEPTION_CHECK_VOID(proc);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    if (proc->led && proc->led->hook && proc->led->hook->set_ui_backlight_level_rate) {
        proc->led->hook->set_ui_backlight_level_rate(proc->led, 100);
    }
    proc->last_progress_ani_time = get_timestamp();
}

void dummy_power_on_hold_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_ON_HOLD_COMPLETE_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_ON_HOLD_IDLE_TASK_NAME);
}

#endif /* IS_SUPPORT_POWER_ON_GUIDE_UI */
