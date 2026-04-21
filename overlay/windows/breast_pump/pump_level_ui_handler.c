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
static void pump_level_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    switch (popup_ui_index) {
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
            pump_complete_event_handler(proc);
            break;
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
            proc->forbid_start_pump = true;
            pump_complete_event_handler(proc);
            break;
        default:
            break;
    }
}

static void pump_level_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc && proc->wm && proc->pump);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL) {
        LOG_UI("ui type error, ignore.");
        return;
    }

    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    uint8_t level_idx = proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? 0 : 1;
    if (proc->level_change > 0) {
            proc->setting.level[level_idx] = proc->setting.level[level_idx] + proc->level_change;
            if (proc->setting.level[level_idx] >= BREAST_PUMP_LEVEL_MAX) {
                proc->setting.level[level_idx] = BREAST_PUMP_LEVEL_MAX - 1;
            }
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    } else if (proc->level_change < 0) {
        uint8_t abs_level_change = abs(proc->level_change);
        proc->setting.level[level_idx] = proc->setting.level[level_idx] < abs_level_change ?
            0 : (proc->setting.level[level_idx] - abs_level_change);
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    proc->level_change = 0;
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_FINISH || pump_status == BREAST_PUMP_IDLE) {
        LOG_WM("goto home window.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
    } else {
        show_pump_home_ui(proc);
    }
}

static void pump_level_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    ui_display_pump_level_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_LEVEL_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("level ui timeout: %d", proc->last_progress_ani_time);
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
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_level_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            pump_level_popup_ui_finish_policy(ui, index);
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
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL) {
        return;
    } else {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        pump_ui_change_hdr_remove(proc);
    }
    
    ui_play_info_t play_info = {
        .tag = "pump_level_screen",
        .fps = PUMP_LEVEL_ANIMATION_FREQ,
        .progress_cb = pump_level_animation_progress_cb,
        .display_prepare_cb = pump_level_ui_display_prepare,
        .display_remove_cb = pump_level_ui_display_remove,
        .user_data = proc,
    };
    proc->current_ui_type = PUMP_UI_TYPE_LEVEL;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_pump_control_type_ui(ui_proc_pump_t *proc, uint8_t control_type)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL && proc->info.control_type == control_type) {
        show_pump_home_ui(proc);
        return;
    }
    proc->level_change = 0;
    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL) {
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    } 
    
    if (proc->info.control_type != control_type) {
        proc->info.control_type = control_type;
        WRITE_PUMP_LEVEL_CONTROL_TYPE(proc->pump, control_type, 1);
    }
    proc->forbid_start_pump = false;
    show_pump_level_ui(proc);
    LOG_UI("switch pump control type: %d", control_type);
}

void switch_pump_level_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    int16_t level = 0;
    if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        level = proc->setting.level[0] + proc->level_change;
    } else {
        level = proc->setting.level[1] + proc->level_change;
    }

    if (is_increase) {
        if (level < BREAST_PUMP_LEVEL_15) {
            proc->level_change++;
        }
    } else {
        if (level > BREAST_PUMP_LEVEL_0) {
            proc->level_change--;
        }
    }

    show_pump_level_ui(proc);
}   

void pump_level_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LEVEL_DISPLAY_TASK_NAME);
}