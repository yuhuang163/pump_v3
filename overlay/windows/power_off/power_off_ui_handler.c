/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_power_off.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_OFF_DELAY_TIME                    (1000 * 3)
#define POWER_OFF_TASK_NAME                     "power_off_wm_task"

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
static void power_off_ui_after_policy(ui_manager_context_t *ui, int index)
{
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_off_process(ui);
}

static void power_off_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            power_off_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void power_off_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_off_prepare(ui);
}

static void power_off_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_power_off_remove(ui);
}

static void show_power_off_ui(ui_proc_power_off_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == POWER_OFF_UI_TYPE_POWER_OFF) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "power_off_screen",
        .fps = POWER_OFF_ANIMATION_FREQ,
        .progress_cb = power_off_animation_progress_cb,
        .display_prepare_cb = power_off_ui_display_prepare,
        .display_remove_cb = power_off_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = POWER_OFF_UI_TYPE_POWER_OFF;
}

static void power_off_cb(hdr_post_args_t *args)
{
    ui_proc_power_off_t *proc = (ui_proc_power_off_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POWER_OFF);
#if IS_SUPPORT_POWER_OFF_WHEN_CHARGING
    if (proc->battery) {
        int chage_state = proc->battery->get_charge_status();
        if (BATTERY_IS_CHARGING(chage_state)) {
            show_power_off_charging_ui(proc);
            return;
        } 
    }
#endif
    power_off_handle(proc);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void power_off_handle(ui_proc_power_off_t *proc)
{
    LOGW("==========POWER_OFF==========");
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    if (pbus == NULL) {
        platform_system_reboot();
    }
    rt_device_node_t *power_dev = pbus->find_device(ID_DEVICE_POWER);
    if (power_dev == NULL) {
        platform_system_reboot();
    }
    int arg1[] = {0};
    for (uint8_t i = 0; i < 3; i++) {
        power_dev->ioctl(power_dev, OP_CODE_POWER_CONTROL, (void *)arg1);
        msleep(200);
        LOG_PM("power off try: %d", i);
    }
    platform_system_reboot();
}

void start_power_off_ui_animation(ui_proc_power_off_t *proc)
{
    show_power_off_ui(proc);
    POST_TASK_HANDLER_DELAYED_ARG1(power_off_cb, POWER_OFF_DELAY_TIME, proc, POWER_OFF_TASK_NAME);
}

void stop_power_off_ui_animation(ui_proc_power_off_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_OFF_TASK_NAME);
    EXCEPTION_CHECK_VOID(proc);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}