/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_power_on_guide.h" // IWYU pragma: keep

#if IS_SUPPORT_POWER_ON_GUIDE_UI

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void power_on_guide_low_power_direct_power_off(void)
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
    LOG_WM("==========POWER_ON_GUIDE LOW_POWER POWER_OFF==========");
    msleep(50);
    for (uint8_t i = 0; i < 3; i++) {
        power_dev->ioctl(power_dev, OP_CODE_POWER_CONTROL, (void *)arg1);
        msleep(200);
        LOG_WM("power off try: %d", i);
    }
    platform_system_reboot();
}

static void power_on_guide_low_power_complete_cb(hdr_post_args_t *args)
{
    ui_proc_power_on_guide_t *proc = (ui_proc_power_on_guide_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POWER_ON_GUIDE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore low power complete.");
        return;
    }

    if (proc->current_ui_type != POWER_ON_GUIDE_UI_TYPE_LOW_POWER) {
        LOG_UI("power on guide low power ui, ignore complete.");
        return;
    }

#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
    if (proc->battery && BATTERY_IS_CHARGING(proc->battery->get_charge_status())) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
        msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_CHARGE;
        LOG_WM("charging, goto power off charge screen");
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
        msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_CHARGE;
        LOG_WM("adapter mismatch, goto power off charge screen");
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->pd_check_retry_count < POWER_ON_GUIDE_PD_CHECK_RETRY_MAX) {
        proc->pd_check_retry_count++;
        LOG_WM("pd check retry:%d", proc->pd_check_retry_count);
        POST_WM_TASK_HANDLER_DELAYED_ARG1(power_on_guide_low_power_complete_cb, POWER_ON_GUIDE_PD_CHECK_RETRY_DELAY_MS,
            proc, POWER_ON_GUIDE_LOW_POWER_TASK_NAME);
        return;
    }
#else
    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
        msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_CHARGE;
        LOG_WM("adapter mismatch, goto power off charge screen");
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#endif
    LOG_WM("power on guide low power timeout, direct power off.");
    power_on_guide_low_power_direct_power_off();
}

static void power_on_guide_low_power_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_power_on_guide_t *proc = (ui_proc_power_on_guide_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POWER_ON_GUIDE);
    ui_display_battery_low_power_process(ui);
}

static void power_on_guide_low_power_animation_progress_cb(void *user_data, int index,
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
            power_on_guide_low_power_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void power_on_guide_low_power_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_battery_low_power_prepare(ui, 0, NULL);
}

static void power_on_guide_low_power_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_battery_low_power_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_power_on_guide_low_power_ui(ui_proc_power_on_guide_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == POWER_ON_GUIDE_UI_TYPE_LOW_POWER) {
        return;
    }

    ui_play_info_t play_info = {
        .tag = "power_on_guide_low_power_screen",
        .fps = POWER_ON_GUIDE_ANIMATION_FREQ,
        .progress_cb = power_on_guide_low_power_animation_progress_cb,
        .display_prepare_cb = power_on_guide_low_power_ui_display_prepare,
        .display_remove_cb = power_on_guide_low_power_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = POWER_ON_GUIDE_UI_TYPE_LOW_POWER;
    proc->power_on_hold_holding = 0;
    proc->pd_check_retry_count = 0;
    if (proc->led && proc->led->hook && proc->led->hook->set_ui_backlight_level_rate) {
        proc->led->hook->set_ui_backlight_level_rate(proc->led, IDLE_BRIGHTNESS_MAX_VALUE);
    }
    POST_WM_TASK_HANDLER_DELAYED_ARG1(power_on_guide_low_power_complete_cb,
        POWER_ON_GUIDE_HOLD_IDLE_TIMEOUT_MS, proc, POWER_ON_GUIDE_LOW_POWER_TASK_NAME);
}

void power_on_guide_low_power_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_ON_GUIDE_LOW_POWER_TASK_NAME);
}

#endif /* IS_SUPPORT_POWER_ON_GUIDE_UI */
