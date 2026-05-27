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

static void airplane_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_AIRPLANE_MODE);
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
        case ANI_POPUP_DISPLAY_FINISH:
            POST_WM_TASK_HANDLER_NO_DELAY(airplane_mode_exit_cb, ui, AIRPLANE_MODE_EXIT_TASK_NAME);
            break;
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
#if V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4
        proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX);
#else
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
#endif
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
}

