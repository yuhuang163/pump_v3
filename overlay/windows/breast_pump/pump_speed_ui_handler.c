/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "breast_pump_manager.h" 
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_SPEED_DISPLAY_TASK_NAME       "pump_speed_display_cmp"

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

static void pump_speed_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pump_speed_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_pump_speed_process(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(pump_speed_display_completed_cb, proc, PUMP_SPEED_DISPLAY_TASK_NAME);
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
int switch_pump_frequency_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump,
                        breast_pump_info_t *pump_info)
{
    struct storage_manager_context *st = pump->priv.st;
    EXCEPTION_CHECK_INT(st);
    EXCEPTION_CHECK_INT(pump->priv.motor);
    EXCEPTION_CHECK_INT(proc);
    int time_index = pump->priv.motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }

    if (!BREAST_PUMP_IS_COMMON_MODE(pump_info->mode)) {
        LOG_UI("pump is not common mode, ignore.");
        return 0;
    } 
    pump->get_breast_pump_setting(&proc->setting, time_index);
    if (proc->setting.speed < BREAST_PUMP_SPEED_HIGH) {
        proc->setting.speed++;
    } else {
        proc->setting.speed = BREAST_PUMP_SPEED_LOW;
    }
    pump->set_breast_pump_setting(&proc->setting, time_index);
    store_pump_setting_value_t store_pump_setting_value = {0};
    store_pump_setting_value.is_custom_trial_mode = pump->priv.m_info.is_custom_trial_mode;
    store_pump_setting_value.setting = &proc->setting;
    st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, pump_info->mode, time_index, &store_pump_setting_value);
    show_pump_speed_ui(proc);
    LOG_UI("switch pump frequency: %d, mode: %s", proc->setting.speed,
         pump->get_breast_pump_mode_name(pump_info->mode));

    return 0;
}

void pump_speed_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SPEED_DISPLAY_TASK_NAME);
}