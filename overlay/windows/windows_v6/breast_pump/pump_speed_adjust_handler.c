/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_speed_adjust_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;

    (void)index;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    pump_sub_mode_sync_tick(proc, PUMP_SUB_MODE_SYNC_SPEED);
    pump_sub_mode_switch_tick(proc);
    ui_display_pump_speed_adjust_process(ui);
    pump_pause_led_tick(proc);
}

static void pump_speed_adjust_animation_progress_cb(void *user_data, int index,
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
            pump_speed_adjust_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void pump_speed_adjust_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_speed_adjust_prepare(ui);
}

static void pump_speed_adjust_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_speed_adjust_remove(ui);
}

static void pump_speed_adjust_state_sync(ui_proc_pump_t *proc)
{
    pump_sub_mode_state_init(proc, 0);
    if (proc->setting.speed >= BREAST_PUMP_SPEED_MAX) {
        proc->setting.speed = BREAST_PUMP_SPEED_MAX - 1;
    }
    LOG_WM("pump speed adjust sync speed=%d type=%d time=%d",
        proc->setting.speed, proc->sub_mode_type, proc->sub_mode_time_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_speed_adjust_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);

    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    pump_speed_adjust_state_sync(proc);

    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) {
        return;
    }

    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);
    ui_play_info_t play_info = {
        .tag = "pump_speed_adjust_screen",
        .fps = PUMP_SPEED_ANIMATION_FREQ,
        .progress_cb = pump_speed_adjust_animation_progress_cb,
        .display_prepare_cb = pump_speed_adjust_ui_prepare,
        .display_remove_cb = pump_speed_adjust_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = PUMP_UI_TYPE_SPEED_ADJUST;
    ui->hook->ui_animation_play_init(ui, play_info);
}

int pump_speed_adjust_handler(ui_proc_pump_t *proc, bool is_increase)
{
    int time_index;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_SPEED_ADJUST) {
        return -1;
    }

    pump_sub_mode_switch_clear(proc);
    time_index = proc->sub_mode_time_index;
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    if (is_increase) {
        if (proc->setting.speed < BREAST_PUMP_SPEED_MAX - 1) {
            proc->setting.speed++;
        }
    } else {
        if (proc->setting.speed > BREAST_PUMP_SPEED_LOW) {
            proc->setting.speed--;
        }
    }
    WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    LOG_WM("pump speed adjust -> %d time=%d", proc->setting.speed, time_index);
    return 0;
}

void pump_speed_adjust_ui_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->sub_mode_switch = 0;
}

void pump_speed_adjust_ui_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_SPEED_ADJUST) {
        return;
    }
    LOG_WM("dismiss pump speed adjust, back home keep page.");
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_KEEP_PAGE);
}
