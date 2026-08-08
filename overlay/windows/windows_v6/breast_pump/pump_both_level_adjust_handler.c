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
#define PUMP_BOTH_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME      "pump_both_level_adjust_idle_exit"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_both_level_over_limit_clear(ui_proc_pump_t *proc)
{
    if (proc->both_level_over_limit == 0) {
        return;
    }
    proc->both_level_over_limit = 0;
    proc->both_level_arc[0] = proc->both_level_value[0];
    proc->both_level_arc[1] = proc->both_level_value[1];
}

static void pump_both_level_over_limit_tick(ui_proc_pump_t *proc)
{
    uint32_t elapsed;

    if (proc->both_level_over_limit == 0) {
        return;
    }
    elapsed = get_timestamp_ms(proc->both_level_over_limit_time);
    /* 1s：进度条开始回退到实际力度 */
    if (elapsed >= PUMP_LEVEL_OVER_LIMIT_ARC_ROLLBACK_MS) {
        if (proc->both_level_arc[0] != proc->both_level_value[0] ||
            proc->both_level_arc[1] != proc->both_level_value[1]) {
            proc->both_level_arc[0] = proc->both_level_value[0];
            proc->both_level_arc[1] = proc->both_level_value[1];
        }
    }
    /* 2s：过大提醒文案消失 */
    if (elapsed >= PUMP_LEVEL_OVER_LIMIT_TIP_CLEAR_MS) {
        pump_both_level_over_limit_clear(proc);
    }
}

static void pump_both_level_adjust_idle_exit_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return;
    }
    if (proc->both_level_over_limit) {
        return;
    }
    LOG_UI("pump both level adjust idle timeout, silent exit");
    pump_both_level_adjust_ui_dismiss(proc);
}

static void pump_both_level_adjust_idle_timeout_tick(ui_proc_pump_t *proc)
{
    uint32_t time_diff;

    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return;
    }
    /// 力度限制态不静默退出，过大提示自行 2s 结束
    if (proc->both_level_over_limit) {
        return;
    }
    time_diff = get_timestamp_ms(proc->level_adjust_time);
    if (time_diff < PUMP_LEVEL_ADJUST_IDLE_TIMEOUT_MS) {
        return;
    }
    proc->level_adjust_time = get_timestamp();
    LOG_UI("pump both level adjust idle timeout: %d", time_diff);
    POST_WM_TASK_HANDLER_NO_DELAY(pump_both_level_adjust_idle_exit_cb, proc,
        PUMP_BOTH_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME);
}

static void pump_both_level_adjust_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;

    (void)index;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    pump_sub_mode_sync_tick(proc, PUMP_SUB_MODE_SYNC_BOTH_LEVEL);
    pump_both_level_over_limit_tick(proc);
    pump_sub_mode_switch_tick(proc);
    pump_both_level_adjust_idle_timeout_tick(proc);
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
    pump_pause_idle_timeout_handler(proc);
#endif
    ui_display_pump_both_level_adjust_process(ui);
    pump_pause_led_tick(proc);
}

static void pump_both_level_adjust_animation_progress_cb(void *user_data, int index,
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
            pump_both_level_adjust_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void pump_both_level_adjust_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_both_level_adjust_prepare(ui);
}

static void pump_both_level_adjust_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_both_level_adjust_remove(ui);
}

static void pump_both_level_adjust_state_sync(ui_proc_pump_t *proc)
{
    int pump_mode = 0;
    uint8_t i;

    pump_sub_mode_state_init(proc, 1);
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    for (i = 0; i < BREAST_PUMP_NUM; i++) {
        proc->both_level_value[i] = proc->setting.level[i];
        proc->both_level_arc[i] = proc->both_level_value[i];
    }
    proc->both_level_over_limit = 0;
    proc->both_level_inc_steps = 0;
    proc->both_level_inc_window_time = get_timestamp();
    proc->level_adjust_time = get_timestamp();
    LOG_WM("pump both level adjust sync mode=%d type=%d max=%d L=%d R=%d",
        pump_mode, proc->sub_mode_type,
        home_level_adjust_max_get(proc->sub_mode_type),
        proc->both_level_value[0], proc->both_level_value[1]);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_both_level_adjust_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);

    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    pump_both_level_adjust_state_sync(proc);

    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return;
    }

    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);
    ui_play_info_t play_info = {
        .tag = "pump_both_level_adjust_screen",
        .fps = PUMP_LEVEL_ANIMATION_FREQ,
        .progress_cb = pump_both_level_adjust_animation_progress_cb,
        .display_prepare_cb = pump_both_level_adjust_ui_prepare,
        .display_remove_cb = pump_both_level_adjust_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = PUMP_UI_TYPE_BOTH_LEVEL_ADJUST;
    ui->hook->ui_animation_play_init(ui, play_info);
}

int pump_both_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase)
{
    uint8_t level_max;
    uint8_t i;
    uint8_t next_l;
    uint8_t next_r;
    bool changed = false;
    int time_index;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return -1;
    }

    pump_sub_mode_switch_clear(proc);
    time_index = proc->sub_mode_time_index;
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    level_max = home_level_adjust_max_get(proc->sub_mode_type);
    proc->level_adjust_time = get_timestamp();

    if (!is_increase) {
        pump_both_level_over_limit_clear(proc);
        for (i = 0; i < BREAST_PUMP_NUM; i++) {
            if (proc->both_level_value[i] > BREAST_PUMP_LEVEL_0) {
                proc->both_level_value[i]--;
                proc->setting.level[i] = proc->both_level_value[i];
                changed = true;
            }
            proc->both_level_arc[i] = proc->both_level_value[i];
        }
        if (changed) {
            WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        }
        proc->both_level_inc_steps = 0;
        proc->both_level_inc_window_time = get_timestamp();
        return 0;
    }

    /* 加档：过大提示中仅推进圆弧，不改实际力度 */
    if (proc->both_level_over_limit) {
        for (i = 0; i < BREAST_PUMP_NUM; i++) {
            if (proc->both_level_arc[i] < level_max) {
                proc->both_level_arc[i]++;
            }
        }
        proc->both_level_over_limit_time = get_timestamp();
        return 0;
    }

    if (get_timestamp_ms(proc->both_level_inc_window_time) >= PUMP_LEVEL_OVER_LIMIT_WINDOW_MS) {
        proc->both_level_inc_steps = 0;
        proc->both_level_inc_window_time = get_timestamp();
    }

    next_l = proc->both_level_value[0];
    next_r = proc->both_level_value[1];
    if (next_l < level_max) {
        next_l++;
    }
    if (next_r < level_max) {
        next_r++;
    }
    if (next_l == proc->both_level_value[0] && next_r == proc->both_level_value[1]) {
        return 0;
    }

    if ((proc->both_level_inc_steps + 1) > PUMP_LEVEL_OVER_LIMIT_MAX_STEP) {
        proc->both_level_over_limit = 1;
        proc->both_level_over_limit_time = get_timestamp();
        proc->both_level_arc[0] = next_l;
        proc->both_level_arc[1] = next_r;
        return 0;
    }

    if (next_l != proc->both_level_value[0] || next_r != proc->both_level_value[1]) {
        proc->both_level_value[0] = next_l;
        proc->both_level_value[1] = next_r;
        proc->setting.level[0] = next_l;
        proc->setting.level[1] = next_r;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        proc->both_level_inc_steps++;
    }
    proc->both_level_arc[0] = proc->both_level_value[0];
    proc->both_level_arc[1] = proc->both_level_value[1];
    return 0;
}

void pump_both_level_adjust_ui_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->both_level_over_limit = 0;
    proc->sub_mode_switch = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BOTH_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME);
}

void pump_both_level_adjust_ui_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return;
    }
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BOTH_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME);
#if PUMP_ADJUST_EXIT_TO_PREV_PAGE_ENABLE
    LOG_WM("dismiss pump both level adjust, back prev page.");
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_KEEP_PAGE);
#else
    LOG_WM("dismiss pump both level adjust, back home reset page.");
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
#endif
}
