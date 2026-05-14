/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "window_screen_factory_reset.h" // IWYU pragma: keep

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

static void show_factory_reset_ui(ui_proc_factory_reset_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == FACTORY_RESET_UI_TYPE_FACTORY_RESET) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "factory_reset_screen",
        .fps = FACTORY_RESET_ANIMATION_FREQ,
        .progress_cb = factory_reset_animation_progress_cb,
        .display_prepare_cb = factory_reset_ui_display_prepare,
        .display_remove_cb = factory_reset_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = FACTORY_RESET_UI_TYPE_FACTORY_RESET;
}

static void start_factory_reset_ui_animation(ui_proc_factory_reset_t *proc)
{
    EXCEPTION_CHECK_VOID(proc->pm);
    LOG_UI("start factory reset ui animation.");
    show_factory_reset_ui(proc);
}

static void factory_reset_led_blink_policy(ui_proc_factory_reset_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->led);
    LOG_UI("factory reset led blink policy");
    proc->led->set_led_blink(PUMP_LED_ALL_ID, LED_MEDIUM_BLINK_TIME, 0, LED_STATUS_OFF);
}

static void factory_reset_cb(hdr_post_args_t *args)
{
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_FACTORY_RESET);
    if (proc->led && proc->led->hook && proc->led->hook->set_key_light_adaptive_dimming) {
        proc->led->hook->set_key_light_adaptive_dimming(proc->led, 0);
    }
    factory_reset_led_blink_policy(proc);
    start_factory_reset_ui_animation(proc);
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
void factory_reset_handle(ui_proc_factory_reset_t *proc)
{
    POST_TASK_HANDLER_NO_DELAY(factory_reset_cb, proc, FACTORY_RESET_TASK_NAME);
}

void factory_reset_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(FACTORY_RESET_TASK_NAME);
}
