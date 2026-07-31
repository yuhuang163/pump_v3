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
#define PUMP_BREAK_TIP_DISPLAY_TASK_NAME                "pump_break_tip_display"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_break_tip_pause(ui_proc_pump_t *proc)
{
    int pump_status = 0;
    int control_source = 0;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_PAUSE || pump_status == BREAST_PUMP_IDLE ||
        pump_status == BREAST_PUMP_FINISH) {
        return;
    }

    WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_PAUSE, 1);
    proc->pump_status = BREAST_PUMP_PAUSE;
    READ_PUMP_CONTROL_SOURCE(proc->pump, control_source);
    data_bury_trigger_user_evt_t bury_evt = {
        .evt_source = (control_source == true ? DATA_BURY_EVT_SOURCE_REMOTE_APP :
            DATA_BURY_EVT_SOURCE_LOCAL),
        .evt_type = DATA_BURY_USER_EVT_PAUSE,
    };
    DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
    WRITE_PUMP_CONTROL_SOURCE(proc->pump, false);
    LOG_UI("pump break tip, pause pump. status:%d", pump_status);
}

static void pump_break_tip_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_BREAK_TIP) {
        LOG_UI("break tip ui, ignore display completed.");
        return;
    }
    LOG_WM("pump break tip timeout, back to previous page.");
    pump_break_tip_ui_dismiss(proc);
}

static void pump_break_tip_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_break_tip_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_BREAK_TIP_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("break tip ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(pump_break_tip_display_completed_cb, proc,
            PUMP_BREAK_TIP_DISPLAY_TASK_NAME);
    }
}

static void pump_break_tip_animation_progress_cb(void *user_data, int index,
    ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_break_tip_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_break_tip_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_break_tip_prepare(ui);
}

static void pump_break_tip_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_break_tip_remove(ui);
}

static void pump_break_tip_restore_prev_ui(ui_proc_pump_t *proc)
{
    uint16_t prev_ui = proc->break_tip_prev_ui_type;

    proc->break_tip_prev_ui_type = PUMP_UI_TYPE_NONE;
    switch (prev_ui) {
        case PUMP_UI_TYPE_LEVEL_ADJUST:
            show_pump_level_adjust_ui(proc,
                proc->level_adjust_side == PUMP_LEVEL_ADJUST_SIDE_LEFT);
            break;
        case PUMP_UI_TYPE_BOTH_LEVEL_ADJUST:
            show_pump_both_level_adjust_ui(proc);
            break;
        case PUMP_UI_TYPE_SPEED_ADJUST:
            show_pump_speed_adjust_ui(proc);
            break;
        case PUMP_UI_TYPE_RHYTHM_CONFIRM:
            show_pump_rhythm_confirm_ui(proc);
            break;
        case PUMP_UI_TYPE_SETTINGS_SUCCESS:
            show_pump_settings_success_ui(proc);
            break;
        case PUMP_UI_TYPE_HOME:
        default:
            show_pump_home_ui(proc);
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_break_tip_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_BREAK_TIP) {
        return;
    }
    proc->break_tip_prev_ui_type = proc->current_ui_type;
    pump_ui_change_hdr_remove(proc);
    pump_break_tip_pause(proc);

    ui_play_info_t play_info = {
        .tag = "pump_break_tip_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_break_tip_animation_progress_cb,
        .display_prepare_cb = pump_break_tip_ui_display_prepare,
        .display_remove_cb = pump_break_tip_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_BREAK_TIP;
}

void pump_break_tip_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BREAK_TIP_DISPLAY_TASK_NAME);
}

void pump_break_tip_ui_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_BREAK_TIP) {
        return;
    }
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BREAK_TIP_DISPLAY_TASK_NAME);
    LOG_WM("pump break tip dismiss, restore prev ui:%d", proc->break_tip_prev_ui_type);
    pump_break_tip_restore_prev_ui(proc);
}
