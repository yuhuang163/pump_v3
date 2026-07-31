/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_both_level_over_limit_clear(ui_proc_home_t *proc)
{
    if (proc->both_level_over_limit == 0) {
        return;
    }
    proc->both_level_over_limit = 0;
    proc->both_level_arc[0] = proc->both_level_value[0];
    proc->both_level_arc[1] = proc->both_level_value[1];
}

static void home_both_level_over_limit_tick(ui_proc_home_t *proc)
{
    if (proc->both_level_over_limit == 0) {
        return;
    }
    if (get_timestamp_ms(proc->both_level_over_limit_time) < HOME_LEVEL_OVER_LIMIT_WINDOW_MS) {
        return;
    }
    home_both_level_over_limit_clear(proc);
}

static void home_both_level_adjust_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;

    (void)index;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    home_both_level_over_limit_tick(proc);
    ui_display_home_both_level_adjust_process(ui);
}

static void home_both_level_adjust_animation_progress_cb(void *user_data, int index,
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
            home_both_level_adjust_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void home_both_level_adjust_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_both_level_adjust_prepare(ui);
}

static void home_both_level_adjust_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_both_level_adjust_remove(ui);
}

static void home_both_level_adjust_state_sync(ui_proc_home_t *proc)
{
    int pump_mode = 0;
    uint8_t level_max;
    uint8_t i;
    int time_index = 0;

    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    level_max = home_level_adjust_max_get((uint8_t)pump_mode);

    for (i = 0; i < BREAST_PUMP_NUM; i++) {
        if (proc->setting.level[i] > level_max) {
            proc->setting.level[i] = level_max;
        }
        proc->both_level_value[i] = proc->setting.level[i];
        proc->both_level_arc[i] = proc->both_level_value[i];
    }
    WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);

    proc->both_level_over_limit = 0;
    proc->both_level_inc_steps = 0;
    proc->both_level_inc_window_time = get_timestamp();
    proc->level_adjust_time = get_timestamp();
    LOG_WM("both level adjust sync mode=%d max=%d L=%d R=%d",
        pump_mode, level_max, proc->both_level_value[0], proc->both_level_value[1]);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_home_both_level_adjust_ui(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);

    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    home_both_level_adjust_state_sync(proc);

    if (proc->current_ui_type == HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return;
    }

    home_ui_change_hdr_remove(proc);
    ui_play_info_t play_info = {
        .tag = "home_both_level_adjust_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_both_level_adjust_animation_progress_cb,
        .display_prepare_cb = home_both_level_adjust_ui_prepare,
        .display_remove_cb = home_both_level_adjust_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = HOME_UI_TYPE_BOTH_LEVEL_ADJUST;
    ui->hook->ui_animation_play_init(ui, play_info);
}

int home_pump_both_level_adjust_handler(ui_proc_home_t *proc, bool is_increase)
{
    uint8_t level_max;
    uint8_t i;
    uint8_t next_l;
    uint8_t next_r;
    bool changed = false;
    int time_index = 0;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return -1;
    }

    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    level_max = home_both_level_adjust_ui_level_max_get();
    proc->level_adjust_time = get_timestamp();

    if (!is_increase) {
        home_both_level_over_limit_clear(proc);
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

    if (get_timestamp_ms(proc->both_level_inc_window_time) >= HOME_LEVEL_OVER_LIMIT_WINDOW_MS) {
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

    if ((proc->both_level_inc_steps + 1) > HOME_LEVEL_OVER_LIMIT_MAX_STEP) {
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

void home_both_level_adjust_ui_exit(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->both_level_over_limit = 0;
}

void home_both_level_adjust_ui_dismiss(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_BOTH_LEVEL_ADJUST) {
        return;
    }
    LOG_WM("dismiss home both level adjust, back to home.");
    proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
    start_home_ui_animation(proc);
}
