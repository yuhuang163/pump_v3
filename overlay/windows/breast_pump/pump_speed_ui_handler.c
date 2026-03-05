/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "breast_pump_manager.h" // IWYU pragma: keep
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_SPEED_DISPLAY_TASK_NAME       "pump_speed_display_cmp"
#define PUMP_SPEED_SYNC_TASK_NAME          "pump_speed_sync_cmp"

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
static void pump_speed_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        show_pump_home_ui(proc);
    }
}

static void pump_ui_speed_status_syn(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        int time_index = proc->pump->get_breast_pump_step_index();
        if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
            time_index = 0;
        }
        proc->pump->set_breast_pump_setting(&proc->setting, time_index, true);
    }
}

static void pump_speed_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pump_speed_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_pump_speed_process(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_SPEED_DISPLAY_TIMEOUT) {
        POST_TASK_HANDLER_NO_DELAY(pump_speed_display_completed_cb, proc, PUMP_SPEED_DISPLAY_TASK_NAME);
    } else if (proc->speed_sync_flag && time_diff >= PUMP_SPEED_SYNC_TIMEOUT) {
        proc->speed_sync_flag = false;
        POST_TASK_HANDLER_NO_DELAY(pump_ui_speed_status_syn, proc, PUMP_SPEED_SYNC_TASK_NAME);
    } 
}

static void pump_speed_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            pump_speed_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_speed_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_speed_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_speed_prepare(ui);
}

static void pump_speed_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_speed_remove(ui);
}

static void show_pump_speed_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    } else {
        pump_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "pump_speed_screen",
        .fps = PUMP_SPEED_ANIMATION_FREQ,
        .progress_cb = pump_speed_animation_progress_cb,
        .display_prepare_cb = pump_speed_ui_display_prepare,
        .display_remove_cb = pump_speed_ui_display_remove,
        .user_data = proc,

    };
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_SPEED;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_pump_speed_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);  
    if (is_increase) {
        if (proc->setting.speed < BREAST_PUMP_SPEED_HIGH) {
            proc->setting.speed++;
        } else {
            proc->setting.speed = BREAST_PUMP_SPEED_LOW;
        }
    } else {
        if (proc->setting.speed > BREAST_PUMP_SPEED_LOW) {
            proc->setting.speed--;
        } else {
            proc->setting.speed = BREAST_PUMP_SPEED_HIGH;
        }
    }
    proc->speed_sync_flag = true;
    uint8_t mode = proc->pump->get_breast_pump_mode();
    LOG_UI("switch %s mode frequency: %d", proc->pump->get_breast_pump_mode_name(mode), proc->setting.speed);
    show_pump_speed_ui(proc);
}

void pump_speed_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SPEED_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SPEED_SYNC_TASK_NAME);
}

void pump_speed_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->led);
    proc->led->set_led_blink(PUMP_FREQ_SET_BL, PUMP_BLINK_PERIOD, PUMP_BLINK_TIMES, LED_STATUS_ON);
    if (proc->pump->is_on_working() == 0) {
        LOG_UI("pump is not working, ignore.");
        return;
    }
    uint8_t mode = proc->pump->get_breast_pump_mode();
    if (PUMP_IS_BREASTFEEDING_MODE(mode)) {
        LOG_UI("pump is breastfeeding mode, ignore.");
        return;
    }
    show_pump_speed_ui(proc);
    EXCEPTION_CHECK_VOID(proc->motor);
    int time_index = proc->pump->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    proc->pump->get_breast_pump_setting(&proc->setting, time_index);
}