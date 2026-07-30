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
#define PUMP_SPEED_SYNC_TASK_NAME          "pump_speed_sync"

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
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    EXCEPTION_CHECK_VOID(proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_SPEED) {
        LOG_UI("ui type error, ignore.");
        return;
    }
    pump_sub_ui_exit_handler(proc);
}

static void pump_speed_sync(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    EXCEPTION_CHECK_VOID(proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_SPEED) {
        LOG_UI("ui type error, ignore.");
        return;
    }

    if (proc->speed_sync_flag) {
        proc->speed_sync_flag = false;
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
}

static void pump_speed_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
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
        case ID_POPUP_UI_OP_SUCCESS:
            POST_WM_TASK_HANDLER_NO_DELAY(pump_speed_display_completed_cb, 
                proc, PUMP_SPEED_DISPLAY_TASK_NAME);
            break;
        default:
            break;
    }
}

static void pump_speed_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_speed_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_SPEED_DISPLAY_TIMEOUT) {
        proc->speed_sync_flag = false;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("speed ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(pump_speed_display_completed_cb, proc, PUMP_SPEED_DISPLAY_TASK_NAME);
    } else {
        if (proc->goto_pump_home_ui) {
            proc->goto_pump_home_ui = false;
            proc->last_progress_ani_time = get_timestamp();
            proc->popup_ui_type = ID_POPUP_UI_OP_SUCCESS;
            POST_WM_TASK_HANDLER_NO_DELAY(pump_speed_sync, proc, PUMP_SPEED_SYNC_TASK_NAME);
            LOG_UI("goto pump speed upate: %d", get_timestamp());
        }
    }
}

static void pump_speed_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_speed_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            pump_speed_popup_ui_finish_policy(ui, index);
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
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        return;
    } else {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
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
    proc->current_ui_type = PUMP_UI_TYPE_SPEED;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_pump_speed_ui(ui_proc_pump_t *proc, bool is_increase)
{
    uint8_t speed_before;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    speed_before = proc->setting.speed;
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
    {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_FREQUENCY,
            .adjust_from = speed_before,
            .adjust_to = proc->setting.speed,
        };

        if (bury_adj.adjust_from != bury_adj.adjust_to) {
            DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
        }
    }
    proc->speed_sync_flag = true;
    char *mode_name = "unknow";
    READ_PUMP_WORK_MODE_NAME(proc->pump, &mode_name);
    LOG_UI("switch %s mode frequency: %d", mode_name, proc->setting.speed);
    show_pump_speed_ui(proc);
}

void pump_speed_ui_exit(ui_proc_pump_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SPEED_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SPEED_SYNC_TASK_NAME);
}

void pump_speed_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        pump_sub_ui_exit_handler(proc);
        return;
    } 
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting); 
    proc->forbid_start_pump = false;
    show_pump_speed_ui(proc);
}