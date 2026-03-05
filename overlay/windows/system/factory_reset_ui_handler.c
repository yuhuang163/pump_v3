/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "window_screen_system.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define FACTORY_RESET_DELAY_TIME                    (1000 * 3)
#define FACTORY_RESET_TASK_NAME                     "factory_reset_task"
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
static void factory_reset_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void factory_reset_ui_after_policy(ui_manager_context_t *ui, int index)
{
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_process(ui);
}

static void factory_reset_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            factory_reset_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            factory_reset_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void factory_reset_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_prepare(ui);
}

static void factory_reset_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_factory_reset_remove(ui);
}

static void show_factory_reset_ui(ui_proc_system_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == SYSTEM_UI_TYPE_FACTORY_RESET) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "factory_reset_screen",
        .fps = SYSTEM_ANIMATION_FREQ,
        .progress_cb = factory_reset_animation_progress_cb,
        .display_prepare_cb = factory_reset_ui_display_prepare,
        .display_remove_cb = factory_reset_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = SYSTEM_UI_TYPE_FACTORY_RESET;
}

static void start_factory_reset_ui_animation(ui_proc_system_t *proc)
{
    EXCEPTION_CHECK_VOID(proc->pm);
    while (get_timestamp_ms(0) < BOOT_LONG_PRESS_TIME) {
        if (proc->pm->is_system_power_on()) {
            break;
        }
        msleep(10);
    }
    LOG_UI("start factory reset ui animation.");
    show_factory_reset_ui(proc);
}

static void factory_reset_led_blink_policy(led_manager_context_t *led)
{
    EXCEPTION_CHECK_VOID(led);
    LOG_UI("factory reset led blink policy");
    led->set_led_blink(PUMP_LED_ALL_ID, 500, 0, LED_STATUS_OFF);
}

static void factory_reset_cb(hdr_post_args_t *args)
{
    ui_proc_system_t *proc = (ui_proc_system_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_SYSTEM);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    pm->acquire_wakelock("reboot", 10);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    LOGW("==========RESET_AND_REBOOT==========");
    st->factory_reset(RESET_AND_REBOOT);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void system_factory_reset_handle(ui_proc_system_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    factory_reset_led_blink_policy(proc->led);
    start_factory_reset_ui_animation(proc);
    if (proc->pump && proc->pump->is_on_working()) {
        LOG_WM("factory reset, stop breast pump work.");
        proc->pump->set_breast_pump_status(BREAST_PUMP_IDLE);
    }
    POST_TASK_HANDLER_DELAYED_ARG1(factory_reset_cb, FACTORY_RESET_DELAY_TIME, proc, FACTORY_RESET_TASK_NAME);
}

void factory_reset_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(FACTORY_RESET_TASK_NAME);
}