/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define AIRPLANE_MODE_EXIT_TASK_NAME            "airplane_mode_exit_task"
#define AIRPLANE_MODE_POWER_OFF_TASK_NAME       "airplane_mode_power_off"

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
static void airplane_mode_exit_cb(hdr_post_args_t *args)
{
    ui_manager_context_t *ui = args->user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_AIRPLANE_MODE);
    EXCEPTION_CHECK_VOID(proc->airplane);
    if (proc->popup_ui_type != ID_POPUP_UI_AIRPLANE_MODE_PROMPT) {
        LOG_WM("popup ui type is not airplane mode prompt, ignore");
        return;
    }
    proc->airplane->exit(AIRPLANE_MODE_EXIT);
    multi_func_msg_data_t msg = {0};
    if (proc->battery && proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        LOG_WM("goto charge screen");
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
     
    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void airplane_mode_power_off_cb(hdr_post_args_t *args)
{
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_AIRPLANE_MODE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore power off.");
        return;
    }
    LOG_WM("airplane mode timeout, goto power off.");
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
    msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_AUTO;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void airplane_mode_power_off_idle_timeout_tick(ui_proc_airplane_mode_t *proc)
{
    uint32_t time_diff;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        return;
    }
    if (proc->power_off_idle_time == 0) {
        proc->power_off_idle_time = get_timestamp();
        return;
    }
    time_diff = get_timestamp_ms(proc->power_off_idle_time);
    if (time_diff < AIRPLANE_MODE_TIMEOUT_TIME) {
        return;
    }
    proc->power_off_idle_time = get_timestamp();
    LOG_WM("airplane mode idle timeout: %u", time_diff);
    POST_WM_TASK_HANDLER_NO_DELAY(airplane_mode_power_off_cb, proc,
        AIRPLANE_MODE_POWER_OFF_TASK_NAME);
}

static void airplane_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_AIRPLANE_MODE);
    (void)index;
    airplane_mode_power_off_idle_timeout_tick(proc);
    ui_display_airplane_mode_process(ui);
}

static void airplane_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            airplane_mode_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH: {
            ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)ui->priv.play_info.user_data;
            if (proc && get_timestamp_ms(proc->last_progress_ani_time) >= AIRPLANE_MODE_DISPLAY_TIMEOUT) {
                proc->last_progress_ani_time = get_timestamp();
                POST_WM_TASK_HANDLER_NO_DELAY(airplane_mode_exit_cb, ui, AIRPLANE_MODE_EXIT_TASK_NAME);
            }
            break;
        }
        default:
            break;
    }
}

static void airplane_mode_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_airplane_mode_prepare(ui);
}

static void airplane_mode_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_airplane_mode_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_airplane_mode_ui(ui_proc_airplane_mode_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    proc->power_off_idle_time = get_timestamp();
    if (proc->current_ui_type == AIRPLANE_MODE_UI_TYPE_HOME) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "airplane_mode_screen",
        .fps = AIRPLANE_MODE_ANIMATION_FREQ,
        .progress_cb = airplane_mode_animation_progress_cb,
        .display_prepare_cb = airplane_mode_ui_display_prepare,
        .display_remove_cb = airplane_mode_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = AIRPLANE_MODE_UI_TYPE_HOME;
}

void stop_airplane_mode_ui(ui_proc_airplane_mode_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void airplane_mode_led_policy(ui_proc_airplane_mode_t *proc, uint8_t status)
{
    uint8_t control_type = 0;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->led);
    if (status) {
        if (proc->led->hook && proc->led->hook->set_key_backlight ) {
            proc->led->hook->set_key_backlight(proc->led, 0);
        }
        proc->led->set_led_level(PUMP_MODE_BL, LED_LEVEL_MAX);
        proc->led->set_led_level(PUMP_FREQ_SET_BL, LED_LEVEL_MAX);

    } else {
        if (proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, 100);
        }
    }
}

void airplane_mode_pump_policy(ui_proc_airplane_mode_t *proc)
{
    // EXCEPTION_CHECK_VOID(proc);
    // EXCEPTION_CHECK_VOID(proc->pump);
    // int is_pump_working = 0; 
    // READ_PUMP_IS_WORKING(proc->pump, is_pump_working);
    // if (is_pump_working > 0) {
    //     WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, 1);
    // }
}

void airplane_mode_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(AIRPLANE_MODE_EXIT_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(AIRPLANE_MODE_POWER_OFF_TASK_NAME);
}

