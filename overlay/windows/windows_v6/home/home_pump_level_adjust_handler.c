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
static void home_level_over_limit_clear(ui_proc_home_t *proc)
{
    if (proc->level_over_limit == 0) {
        return;
    }
    proc->level_over_limit = 0;
    proc->level_arc_level = proc->level_value;
}

static void home_level_over_limit_tick(ui_proc_home_t *proc)
{
    if (proc->level_over_limit == 0) {
        return;
    }
    if (get_timestamp_ms(proc->level_over_limit_time) < HOME_LEVEL_OVER_LIMIT_WINDOW_MS) {
        return;
    }
    home_level_over_limit_clear(proc);
}

static void home_level_adjust_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;

    (void)index;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    home_level_over_limit_tick(proc);
    ui_display_home_level_adjust_process(ui);
}

static void home_level_adjust_animation_progress_cb(void *user_data, int index,
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
            home_level_adjust_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void home_level_adjust_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_level_adjust_prepare(ui);
}

static void home_level_adjust_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_level_adjust_remove(ui);
}

static void home_level_adjust_state_sync(ui_proc_home_t *proc, uint8_t is_left)
{
    uint8_t side = is_left ? HOME_LEVEL_ADJUST_SIDE_LEFT : HOME_LEVEL_ADJUST_SIDE_RIGHT;
    uint8_t index = side;
    int pump_mode = 0;
    uint8_t level_max;
    int time_index = 0;

    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    level_max = home_level_adjust_max_get((uint8_t)pump_mode);
    if (proc->setting.level[index] > level_max) {
        proc->setting.level[index] = level_max;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }

    proc->level_adjust_side = side;
    proc->level_value = proc->setting.level[index];
    proc->level_arc_level = proc->level_value;
    proc->level_over_limit = 0;
    proc->level_inc_window_base = proc->level_value;
    proc->level_inc_window_time = get_timestamp();
    proc->level_adjust_time = get_timestamp();
    LOG_WM("level adjust sync side=%d mode=%d max=%d level=%d",
        side, pump_mode, level_max, proc->level_value);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_home_level_adjust_ui(ui_proc_home_t *proc, uint8_t is_left)
{
    ui_manager_context_t *ui;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);

    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    home_level_adjust_state_sync(proc, is_left);

    if (proc->current_ui_type == HOME_UI_TYPE_LEVEL_ADJUST) {
        /* 同页切换左右侧，仅刷新状态 */
        return;
    }

    home_ui_change_hdr_remove(proc);
    ui_play_info_t play_info = {
        .tag = "home_level_adjust_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_level_adjust_animation_progress_cb,
        .display_prepare_cb = home_level_adjust_ui_prepare,
        .display_remove_cb = home_level_adjust_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = HOME_UI_TYPE_LEVEL_ADJUST;
    ui->hook->ui_animation_play_init(ui, play_info);
}

int home_pump_level_adjust_handler(ui_proc_home_t *proc, bool is_increase)
{
    uint8_t index;
    uint8_t level_max;
    uint8_t cur;
    uint8_t next;
    int time_index = 0;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_LEVEL_ADJUST) {
        return -1;
    }

    index = proc->level_adjust_side;
    if (index >= BREAST_PUMP_NUM) {
        return -1;
    }

    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    level_max = home_level_adjust_ui_level_max_get();
    cur = proc->level_value;
    proc->level_adjust_time = get_timestamp();

    if (!is_increase) {
        home_level_over_limit_clear(proc);
        if (cur > BREAST_PUMP_LEVEL_0) {
            cur--;
            proc->setting.level[index] = cur;
            WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
            proc->level_value = cur;
        }
        proc->level_arc_level = proc->level_value;
        proc->level_inc_window_base = proc->level_value;
        proc->level_inc_window_time = get_timestamp();
        return 0;
    }

    /* 加档：先刷新过大提示倒计时（若已在提示中） */
    if (proc->level_over_limit) {
        next = proc->level_arc_level;
        if (next < level_max) {
            next++;
        }
        proc->level_arc_level = next;
        proc->level_over_limit_time = get_timestamp();
        return 0;
    }

    if (get_timestamp_ms(proc->level_inc_window_time) >= HOME_LEVEL_OVER_LIMIT_WINDOW_MS) {
        proc->level_inc_window_base = cur;
        proc->level_inc_window_time = get_timestamp();
    }

    next = cur;
    if (next < level_max) {
        next++;
    }

    if ((next - proc->level_inc_window_base) > HOME_LEVEL_OVER_LIMIT_MAX_STEP) {
        proc->level_over_limit = 1;
        proc->level_over_limit_time = get_timestamp();
        proc->level_arc_level = next;
        /* 实际力度不变 */
        return 0;
    }

    if (next != cur) {
        proc->setting.level[index] = next;
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
        proc->level_value = next;
    }
    proc->level_arc_level = proc->level_value;
    return 0;
}

void home_level_adjust_ui_exit(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->level_over_limit = 0;
}

void home_level_adjust_ui_dismiss(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_LEVEL_ADJUST) {
        return;
    }
    LOG_WM("dismiss home level adjust, back to home.");
    proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
    start_home_ui_animation(proc);
}
