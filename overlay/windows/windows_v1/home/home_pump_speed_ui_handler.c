/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_PUMP_SPEED_DISPLAY_TASK_NAME                     "home_pump_speed_display_cmp"

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
static void home_pump_speed_ui_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    EXCEPTION_CHECK_VOID(proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_SPEED_ADJUST) {
        LOG_UI("ui type error, ignore.");
        return;
    }

    if (proc->speed_sync_flag) {
        proc->speed_sync_flag = false;
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    start_home_ui_animation(proc);
}

static void home_pump_speed_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (popup_ui_index == ID_POPUP_UI_OP_SUCCESS_PRESET) {
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_speed_ui_display_completed_cb, proc, HOME_PUMP_SPEED_DISPLAY_TASK_NAME);
    }
}

static void home_pump_speed_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    ui_display_home_pump_speed_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= HOME_PUMP_SPEED_DISPLAY_TIMEOUT) {
        proc->speed_sync_flag = false;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("speed ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_speed_ui_display_completed_cb, proc, HOME_PUMP_SPEED_DISPLAY_TASK_NAME);
    } else if (time_diff >= HOME_PUMP_SPEED_SYNC_TIMEOUT && proc->speed_sync_flag) {
        proc->popup_ui_type = ID_POPUP_UI_OP_SUCCESS_PRESET;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("speed ui sync timeout: %d", time_diff);
    }
}

static void home_pump_speed_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_pump_speed_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            home_pump_speed_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_pump_speed_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_pump_speed_prepare(ui);
}

static void home_pump_speed_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_pump_speed_remove(ui);
}

static void show_home_pump_speed_ui(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_SPEED_ADJUST) {
        return;
    } else {
        home_ui_change_hdr_remove(proc);
    }

    ui_play_info_t play_info = {
        .tag = "home_pump_speed_screen",
        .fps = HOME_PUMP_SPEED_ANIMATION_FREQ,
        .progress_cb = home_pump_speed_animation_progress_cb,
        .display_prepare_cb = home_pump_speed_ui_prepare,
        .display_remove_cb = home_pump_speed_ui_remove,
        .user_data = proc,

    };
    proc->current_ui_type = HOME_UI_TYPE_SPEED_ADJUST;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_home_pump_speed(ui_proc_home_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (is_increase) {
        if (proc->setting.speed < (BREAST_PUMP_SPEED_MAX - 1)) {
            proc->setting.speed++;
        } else {
            proc->setting.speed = BREAST_PUMP_SPEED_LOW;
        }
    } else {
        if (proc->setting.speed > BREAST_PUMP_SPEED_LOW) {
            proc->setting.speed--;
        } else {
            proc->setting.speed = (BREAST_PUMP_SPEED_MAX - 1);
        }
    }
    proc->speed_sync_flag = true;
    char *mode_name = "unknow";
    READ_PUMP_WORK_MODE_NAME(proc->pump, &mode_name);
    LOG_WM("switch %s mode frequency: %d", mode_name, proc->setting.speed);
    show_home_pump_speed_ui(proc);
}

void home_pump_speed_ui_exit(ui_proc_home_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_PUMP_SPEED_DISPLAY_TASK_NAME);
}

void home_pump_speed_key_click_handler(ui_proc_home_t *proc)
{
    if (proc->current_ui_type == HOME_UI_TYPE_SPEED_ADJUST) {
        start_home_ui_animation(proc);
        return;
    } 
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting); 
    if (proc->pump_priming == 0) {
        proc->pump_priming = true;
        if (proc->led)  {
            proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
        }
    }
    show_home_pump_speed_ui(proc);
}