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
#define HOME_SPEED_ADJUST_IDLE_EXIT_TASK_NAME           "home_speed_adjust_idle_exit"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_speed_adjust_idle_exit_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->current_ui_type != HOME_UI_TYPE_SPEED_ADJUST) {
        return;
    }
    LOG_UI("home speed adjust idle timeout, silent exit");
    home_speed_adjust_ui_dismiss(proc);
}

static void home_speed_adjust_idle_timeout_tick(ui_proc_home_t *proc)
{
    uint32_t time_diff;

    if (proc->current_ui_type != HOME_UI_TYPE_SPEED_ADJUST) {
        return;
    }
    time_diff = get_timestamp_ms(proc->level_adjust_time);
    if (time_diff < HOME_SPEED_ADJUST_IDLE_TIMEOUT_MS) {
        return;
    }
    proc->level_adjust_time = get_timestamp();
    LOG_UI("home speed adjust idle timeout: %d", time_diff);
    POST_WM_TASK_HANDLER_NO_DELAY(home_speed_adjust_idle_exit_cb, proc,
        HOME_SPEED_ADJUST_IDLE_EXIT_TASK_NAME);
}

static void home_speed_adjust_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;

    (void)index;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    home_sub_mode_sync_tick(proc, HOME_SUB_MODE_SYNC_SPEED);
    home_sub_mode_switch_tick(proc);
    home_speed_adjust_idle_timeout_tick(proc);
    ui_display_home_speed_adjust_process(ui);
}

static void home_speed_adjust_animation_progress_cb(void *user_data, int index,
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
            home_speed_adjust_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void home_speed_adjust_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_speed_adjust_prepare(ui);
}

static void home_speed_adjust_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_speed_adjust_remove(ui);
}

static void home_speed_adjust_state_sync(ui_proc_home_t *proc)
{
    home_sub_mode_state_init(proc, 0);
    if (proc->setting.speed >= BREAST_PUMP_SPEED_MAX) {
        proc->setting.speed = (BREAST_PUMP_SPEED_MAX - 1);
    }
    proc->level_adjust_time = get_timestamp();
    LOG_WM("speed adjust sync speed=%d type=%d time=%d",
        proc->setting.speed, proc->sub_mode_type, proc->sub_mode_time_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_home_speed_adjust_ui(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    home_speed_adjust_state_sync(proc);
    if (proc->current_ui_type == HOME_UI_TYPE_SPEED_ADJUST) {
        return;
    }

    home_ui_nav_push(proc);
    home_ui_change_hdr_remove(proc);
    ui_play_info_t play_info = {
        .tag = "home_speed_adjust_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_speed_adjust_animation_progress_cb,
        .display_prepare_cb = home_speed_adjust_ui_prepare,
        .display_remove_cb = home_speed_adjust_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = HOME_UI_TYPE_SPEED_ADJUST;
    proc->adjust_enter_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
}

int home_pump_speed_adjust_handler(ui_proc_home_t *proc, bool is_increase)
{
    int time_index;
    uint8_t speed_before;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_SPEED_ADJUST) {
        return -1;
    }

    home_sub_mode_switch_clear(proc);
    time_index = proc->sub_mode_time_index;
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    speed_before = proc->setting.speed;
    proc->level_adjust_time = get_timestamp();
    if (is_increase) {
        if (proc->setting.speed < (BREAST_PUMP_SPEED_MAX - 1)) {
            proc->setting.speed++;
        }
    } else {
        if (proc->setting.speed > BREAST_PUMP_SPEED_LOW) {
            proc->setting.speed--;
        }
    }
    if (proc->setting.speed != speed_before) {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_FREQUENCY,
            .adjust_from = speed_before,
            .adjust_to = proc->setting.speed,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
    }
    WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    LOG_WM("home speed adjust -> %d time=%d", proc->setting.speed, time_index);
    return 0;
}

void home_speed_adjust_ui_exit(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->sub_mode_switch = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_SPEED_ADJUST_IDLE_EXIT_TASK_NAME);
}

void home_speed_adjust_ui_dismiss(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_SPEED_ADJUST) {
        return;
    }
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_SPEED_ADJUST_IDLE_EXIT_TASK_NAME);
#if HOME_ADJUST_EXIT_TO_PREV_PAGE_ENABLE
    LOG_WM("dismiss home speed adjust, back prev page.");
    home_ui_nav_back(proc, HOME_UI_NAV_BACK_HOME_KEEP_PAGE);
#else
    LOG_WM("dismiss home speed adjust, back home reset page.");
    home_ui_nav_back(proc, HOME_UI_NAV_BACK_HOME_RESET_PAGE);
#endif
}
