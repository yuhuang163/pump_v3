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
#define HOME_PUMP_LEVEL_ADJUST_DISPLAY_TASK_NAME                     "home_pump_level_adjust_display_cmp"

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
static void home_pump_level_ui_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->current_ui_type != HOME_UI_TYPE_LEVEL_ADJUST) {
        return;
    }
#if (IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST == 0)
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    uint8_t level_idx = proc->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? 0 : 1;
    if (proc->level_sync_flag && proc->level_change != 0) {
        int8_t level = proc->setting.level[level_idx] + proc->level_change ;
        if (level > BREAST_PUMP_LEVEL_15) {
            level = BREAST_PUMP_LEVEL_15;
        } else if (level < BREAST_PUMP_LEVEL_0) {
            level = 0;
        }
        proc->setting.level[level_idx] = level;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    proc->level_change = 0;
#endif
    proc->level_sync_flag = false;
    start_home_ui_animation(proc);
}

static void home_pump_level_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (popup_ui_index == ID_POPUP_UI_OP_SUCCESS_PRESET) {
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_level_ui_display_completed_cb,
            proc, HOME_PUMP_LEVEL_ADJUST_DISPLAY_TASK_NAME);
    }
}

static void home_pump_level_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    ui_display_home_pump_level_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= HOME_PUMP_LEVEL_DISPLAY_TIMEOUT) {
        proc->level_change = 0;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("level ui timeout: %d", proc->last_progress_ani_time);
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_level_ui_display_completed_cb,
            proc, HOME_PUMP_LEVEL_ADJUST_DISPLAY_TASK_NAME);
    } else if (time_diff >= HOME_PUMP_LEVEL_SYNC_TIMEOUT && proc->level_sync_flag) {
        proc->popup_ui_type = ID_POPUP_UI_OP_SUCCESS_PRESET;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("level ui sync timeout: %d", proc->last_progress_ani_time);
    }
}

static void home_pump_level_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_pump_level_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            home_pump_level_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_pump_level_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_pump_level_prepare(ui);
}

static void home_pump_level_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_pump_level_remove(ui);
}

static void show_home_pump_level_ui(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST) {
        return;
    } else {
        home_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "home_pump_level_screen",
        .fps = HOME_PUMP_LEVEL_ANIMATION_FREQ,
        .progress_cb = home_pump_level_animation_progress_cb,
        .display_prepare_cb = home_pump_level_ui_prepare,
        .display_remove_cb = home_pump_level_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = HOME_UI_TYPE_LEVEL_ADJUST;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_home_pump_level_control_type(ui_proc_home_t *proc, uint8_t control_type)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST && proc->control_type == control_type) {
        start_home_ui_animation(proc);
        return;
    }

    if (proc->pump_priming == 0) {
        proc->pump_priming = true;
        if (proc->led)  {
            proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
        }
    }

#if IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    if (proc->current_ui_type != HOME_UI_TYPE_LEVEL_ADJUST) {
        proc->level_change = 0;
        proc->level_sync_flag = false;
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    }
    if (proc->control_type != control_type) {
        proc->control_type = control_type;
    }
#else
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    if (proc->current_ui_type != HOME_UI_TYPE_LEVEL_ADJUST) {
        proc->level_change = 0;
        proc->level_sync_flag = false;
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    } 

    if (proc->control_type != control_type) {
        uint8_t level_idx = proc->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? 0 : 1;
        proc->control_type = control_type;
        if (proc->level_sync_flag && proc->level_change != 0) {
            int8_t level = proc->setting.level[level_idx] + proc->level_change ;
            if (level > BREAST_PUMP_LEVEL_15) {
                level = BREAST_PUMP_LEVEL_15;
            } else if (level < BREAST_PUMP_LEVEL_0) {
                level = BREAST_PUMP_LEVEL_0;
            }
            proc->setting.level[level_idx] = level;
            WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        }
        proc->level_change = 0;
    }
#endif
    show_home_pump_level_ui(proc);
    LOG_WM("switch home pump level adjust type: %d", control_type);
}

void switch_home_pump_level(ui_proc_home_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
#if IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    if (proc->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        if (is_increase) {
            if (proc->setting.level[0] < BREAST_PUMP_LEVEL_15) {
                proc->setting.level[0]++;
                proc->level_change = true;
            }
        } else {
            if (proc->setting.level[0] > BREAST_PUMP_LEVEL_0) {
                proc->setting.level[0]--;
                proc->level_change = true;
            }
        }
    } else {
        if (is_increase) {
            if (proc->setting.level[1] < BREAST_PUMP_LEVEL_15) {
                proc->setting.level[1]++;
                proc->level_change = true;
            }
        } else {
            if (proc->setting.level[1] > BREAST_PUMP_LEVEL_0) {
                proc->setting.level[1]--;
                proc->level_change = true;
            }
        }
    }
    if (proc->level_change) {
        proc->level_change = false;
        LOG_WM("switch level dir:%d, level:%d/%d", is_increase, proc->setting.level[0], proc->setting.level[1]);
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    proc->level_sync_flag = true;
#else
    int16_t level = 0;
    if (proc->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
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
    proc->level_sync_flag = true;
    LOG_WM("switch home pump level: %d, level_change: %d", is_increase, proc->level_change);
#endif
    show_home_pump_level_ui(proc);
}

void home_pump_level_ui_exit(ui_proc_home_t *proc)
{
#if (IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST == 0)
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_PUMP_LEVEL_ADJUST_DISPLAY_TASK_NAME);
    uint8_t level_idx = proc->control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? 0 : 1;
    if (proc->level_sync_flag && proc->level_change != 0) {
        int8_t level = proc->setting.level[level_idx] + proc->level_change ;
        if (level > BREAST_PUMP_LEVEL_15) {
            level = BREAST_PUMP_LEVEL_15;
        } else if (level < BREAST_PUMP_LEVEL_0) {
            level = 0;
        }
        proc->setting.level[level_idx] = level;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    proc->level_change = 0;
    proc->level_sync_flag = false;
#endif
}