/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_LEVEL_DISPLAY_TASK_NAME       "pump_level_display_cmp"

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
static void pump_level_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL) {
        show_pump_home_ui(proc);
    }
}

static void pump_level_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pump_level_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    ui_display_pump_level_process(ui);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(pump_level_display_completed_cb, proc, PUMP_LEVEL_DISPLAY_TASK_NAME);
    }
}

static void pump_level_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            pump_level_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_level_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_level_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_level_prepare(ui);
}

static void pump_level_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_level_remove(ui);
}

static void show_pump_level_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "pump_level_screen",
        .fps = PUMP_LEVEL_ANIMATION_FREQ,
        .progress_cb = pump_level_animation_progress_cb,
        .display_prepare_cb = pump_level_ui_display_prepare,
        .display_remove_cb = pump_level_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_LEVEL;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_pump_control_type_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, uint8_t control_type)
{
    if (control_type >= BREAST_PUMP_CONTROL_TYPE_MAX) {
        LOGE("switch pump control type: %d, out of range", control_type);
        return -1;
    }
    
    if (proc->info.control_type != control_type) {
        proc->info.control_type = control_type;
        pump->set_breast_pump_control_type(control_type);
    }
    show_pump_level_ui(proc);
    return 0;
}

int switch_pump_level_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump,
                    breast_pump_info_t *pump_info, uint8_t is_left)
{
    storage_manager_context_t *st = pump->priv.st;
    EXCEPTION_CHECK_INT(st);
    EXCEPTION_CHECK_INT(pump->priv.motor);
    int time_index = pump->priv.motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    pump->get_breast_pump_setting(&proc->setting, time_index);
    if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        if (is_left) {
            if (proc->setting.level[0] > 0) {
                proc->setting.level[0]--;
            }
        } else {
            if (proc->setting.level[0] < BREAST_PUMP_LEVEL_15) {
                proc->setting.level[0]++;
            }
        }
    } else if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
        if (is_left) {
            if (proc->setting.level[1] > 0) {
                proc->setting.level[1]--;
            }
        } else {
            if (proc->setting.level[1] < BREAST_PUMP_LEVEL_15) {
                proc->setting.level[1]++;
            }
        }
    } else if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_BILATERAL) {
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            if (is_left) {
                if (proc->setting.level[i] > 0) {
                    proc->setting.level[i]--;
                }
            } else {
                if (proc->setting.level[i] < BREAST_PUMP_LEVEL_15) {
                    proc->setting.level[i]++;
                }
            }
        }
    }
    pump->set_breast_pump_setting(&proc->setting, time_index);
    store_pump_setting_value_t store_pump_setting_value = {0};
    store_pump_setting_value.is_custom_trial_mode = pump->priv.m_info.is_custom_trial_mode;
    store_pump_setting_value.setting = &proc->setting;
    st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, pump_info->mode, time_index, &store_pump_setting_value);
    show_pump_level_ui(proc);
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        LOG_UI("switch pump %d level: %d, mode: %s", i, proc->setting.level[i],
             pump->get_breast_pump_mode_name(pump_info->mode));
    } 
    
    return 0;
}   

void pump_level_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LEVEL_DISPLAY_TASK_NAME);
}