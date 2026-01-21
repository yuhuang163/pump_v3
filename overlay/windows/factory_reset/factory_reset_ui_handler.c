/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "window_screen_factory_reset.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

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
    // ui_display_factory_reset_screen_prepare(ui);
}

static void factory_reset_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    // ui_display_factory_reset_screen_remove(ui);
}

static void show_factory_reset_ui(ui_proc_factory_reset_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == FACTORY_RESET_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
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
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = FACTORY_RESET_UI_TYPE_HOME;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_factory_reset_ui_animation(ui_proc_factory_reset_t *proc)
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

void stop_factory_reset_ui_animation(ui_proc_factory_reset_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    ui->animation_play_status_update(ui, false);
}