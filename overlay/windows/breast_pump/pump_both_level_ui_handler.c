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
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    switch (popup_ui_index) {
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
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

static void pump_both_level_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    EXCEPTION_CHECK_VOID(proc->pump);
    if (proc->popup_ui_type == ID_POPUP_UI_LEVEL_ADJUST_WARNING) {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    }

    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL) {
        LOG_UI("ui type error, ignore.");
        return;
    }

#if (IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST == 0)
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    if (proc->both_level_sync_flag && proc->both_level_change != 0) {
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            int8_t level = proc->setting.level[i] + proc->both_level_change ;
            if (level > BREAST_PUMP_LEVEL_15) {
                level = BREAST_PUMP_LEVEL_15;
            } else if (level < 0) {
                level = 0;
            }
            proc->setting.level[i] = level;
        }
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    proc->both_level_change = 0;
#endif
    proc->both_level_sync_flag = false;
    pump_sub_ui_exit_handler(proc);
}

static void pump_both_level_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_both_level_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_LEVEL_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("both level ui timeout: %d", proc->last_progress_ani_time);
        POST_WM_TASK_HANDLER_NO_DELAY(pump_both_level_display_completed_cb, proc,
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
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL) {
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
    proc->current_ui_type = PUMP_UI_TYPE_BOTH_LEVEL;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_pump_both_level_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
#if IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST
    int16_t left_level = proc->setting.level[0] + proc->both_level_change;
    int16_t right_level = proc->setting.level[1] + proc->both_level_change;
    pump_setting_t setting;
    if (is_increase) {
        if (left_level < BREAST_PUMP_LEVEL_15 || right_level < BREAST_PUMP_LEVEL_15) {
            proc->both_level_change++;
        } else {
            return;
        }
    } else {
        if (left_level > BREAST_PUMP_LEVEL_0 || right_level > BREAST_PUMP_LEVEL_0) {
            proc->both_level_change--;
        } else {
            return;
        }
    }

    left_level = proc->setting.level[0] + proc->both_level_change;
    if (left_level < BREAST_PUMP_LEVEL_0) {
        setting.level[0] = BREAST_PUMP_LEVEL_0;
    } else if (left_level > BREAST_PUMP_LEVEL_15) {
        setting.level[0] = BREAST_PUMP_LEVEL_15;
    } else {
        setting.level[0] = left_level;
    }
    right_level = proc->setting.level[1] + proc->both_level_change;
    if (right_level < BREAST_PUMP_LEVEL_0) {
        setting.level[1] = BREAST_PUMP_LEVEL_0;
    } else if (right_level > BREAST_PUMP_LEVEL_15) {
        setting.level[1] = BREAST_PUMP_LEVEL_15;
    } else {
        setting.level[1] = right_level;
    }
    setting.speed = proc->setting.speed;
    WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, setting);
#else
    int16_t left_level = proc->setting.level[0] + proc->both_level_change;
    int16_t right_level = proc->setting.level[1] + proc->both_level_change;
    if (is_increase) {
        if (left_level < BREAST_PUMP_LEVEL_15 || right_level < BREAST_PUMP_LEVEL_15) {
            proc->both_level_change++;
        }
    } else {
        if (left_level > BREAST_PUMP_LEVEL_0 || right_level > BREAST_PUMP_LEVEL_0) {
            proc->both_level_change--;
        }
    }
#endif
    proc->both_level_sync_flag = true;
    show_pump_both_level_ui(proc);
}   


void switch_pump_both_control_type_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
#if IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST
    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL) {
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
        proc->both_level_change = 0;
        switch_pump_both_level_ui(proc, is_increase);
    } 
#else
    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL) {
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
        proc->both_level_change = 0;
        if (is_increase) {
            for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
                if (proc->setting.level[i] < BREAST_PUMP_LEVEL_15) {
                    proc->both_level_change = 1;
                    proc->both_level_sync_flag = true;
                    break;
                }
            }
        } else {
            for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
                if (proc->setting.level[i] > BREAST_PUMP_LEVEL_0) {
                    proc->both_level_change = -1;
                    proc->both_level_sync_flag = true;
                    break;
                }
            }
        }
    } 
#endif
    proc->forbid_start_pump = false;
    show_pump_both_level_ui(proc);
    LOG_UI("switch pump both control type");
}

void pump_both_level_ui_exit(ui_proc_pump_t *proc)
{
#if (IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST == 0)
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BOTH_LEVEL_DISPLAY_TASK_NAME);
    if (proc->both_level_sync_flag && proc->both_level_change != 0) {
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            int8_t level = proc->setting.level[i] + proc->both_level_change ;
            if (level > BREAST_PUMP_LEVEL_15) {
                level = BREAST_PUMP_LEVEL_15;
            } else if (level < 0) {
                level = 0;
            }
            proc->setting.level[i] = level;
        }
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    proc->both_level_change = 0;
    proc->both_level_sync_flag = false;
#endif
}
