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
#define PUMP_BOTH_LEVEL_DISPLAY_TASK_NAME       "pump_both_level_display_cmp"

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
static void pump_both_level_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
}

static void pump_both_level_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc && proc->wm && proc->pump);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->popup_ui_type == ID_POPUP_UI_LEVEL_ADJUST_WARNING) {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    }

    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL) {
        LOG_UI("ui type error, ignore.");
        return;
    }

    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    if (proc->level_change > 0) {
        for (int i = 0; i < BREAST_PUMP_NUM; i++) {
            proc->setting.level[i] = proc->setting.level[i] + proc->level_change;
            if (proc->setting.level[i] >= BREAST_PUMP_LEVEL_MAX) {
                proc->setting.level[i] = BREAST_PUMP_LEVEL_MAX - 1;
            }
        }
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    } else if (proc->level_change < 0) {
        uint8_t abs_level_change = abs(proc->level_change);
        for (int i = 0; i < BREAST_PUMP_NUM; i++) {
            proc->setting.level[i] = proc->setting.level[i] < abs_level_change ?
                0 : (proc->setting.level[i] - abs_level_change);
        }
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

static void pump_both_level_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    ui_display_pump_both_level_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_LEVEL_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("both level ui timeout: %d", proc->last_progress_ani_time);
        POST_TASK_HANDLER_NO_DELAY(pump_both_level_display_completed_cb, proc,
            PUMP_BOTH_LEVEL_DISPLAY_TASK_NAME);
    }


}

static void pump_both_level_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_both_level_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            pump_both_level_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_both_level_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_both_level_prepare(ui);
}

static void pump_both_level_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_both_level_remove(ui);
}

static void show_pump_both_level_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    } else {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        pump_ui_change_hdr_remove(proc);
    }
    
    ui_play_info_t play_info = {
        .tag = "pump_both_level_screen",
        .fps = PUMP_LEVEL_ANIMATION_FREQ,
        .progress_cb = pump_both_level_animation_progress_cb,
        .display_prepare_cb = pump_both_level_ui_display_prepare,
        .display_remove_cb = pump_both_level_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_BOTH_LEVEL;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_pump_both_control_type_ui(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    proc->level_change = 0;
    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL) {
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    } 
    WRITE_PUMP_LEVEL_CONTROL_TYPE(proc->pump, BREAST_PUMP_CONTROL_TYPE_BILATERAL, 1);
    proc->forbid_start_pump = false;
    show_pump_both_level_ui(proc);
    LOG_UI("switch pump both control type");
}

void switch_pump_both_level_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int16_t left_level = proc->setting.level[0] + proc->level_change;
    int16_t right_level = proc->setting.level[1] + proc->level_change;
    if (is_increase) {
        if (left_level < BREAST_PUMP_LEVEL_15 || right_level < BREAST_PUMP_LEVEL_15) {
            proc->level_change++;
        }
    } else {
        if (left_level > BREAST_PUMP_LEVEL_0 || right_level > BREAST_PUMP_LEVEL_0) {
            proc->level_change--;
        }
    }
    show_pump_both_level_ui(proc);
}   

void pump_both_level_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BOTH_LEVEL_DISPLAY_TASK_NAME);
}