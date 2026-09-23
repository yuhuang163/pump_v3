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
#define PUMP_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME           "pump_level_adjust_idle_exit"

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_level_over_limit_clear(ui_proc_pump_t *proc)
{
    if (proc->level_over_limit == 0) {
        return;
    }
    proc->level_over_limit = 0;
    proc->level_arc_level = proc->level_value;
}

static void pump_level_over_limit_tick(ui_proc_pump_t *proc)
{
    uint32_t elapsed;

    if (proc->level_over_limit == 0) {
        return;
    }
    elapsed = get_timestamp_ms(proc->level_over_limit_time);
    /* 1s：进度条开始回退到实际力度 */
    if (elapsed >= PUMP_LEVEL_OVER_LIMIT_ARC_ROLLBACK_MS &&
        proc->level_arc_level != proc->level_value) {
        proc->level_arc_level = proc->level_value;
    }
    /* 2s：过大提醒文案消失 */
    if (elapsed >= PUMP_LEVEL_OVER_LIMIT_TIP_CLEAR_MS) {
        pump_level_over_limit_clear(proc);
    }
}

static void pump_level_adjust_idle_exit_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL_ADJUST) {
        return;
    }
    if (proc->level_over_limit) {
        return;
    }
    LOG_UI("pump level adjust idle timeout, silent exit");
    pump_level_adjust_ui_dismiss(proc);
}

static void pump_level_adjust_idle_timeout_tick(ui_proc_pump_t *proc)
{
    uint32_t time_diff;

    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL_ADJUST) {
        return;
    }
    /// 力度限制态不静默退出，过大提示自行 2s 结束
    if (proc->level_over_limit) {
        return;
    }
    time_diff = get_timestamp_ms(proc->level_adjust_time);
    if (time_diff < PUMP_LEVEL_ADJUST_IDLE_TIMEOUT_MS) {
        return;
    }
    proc->level_adjust_time = get_timestamp();
    LOG_UI("pump level adjust idle timeout: %d", time_diff);
    POST_WM_TASK_HANDLER_NO_DELAY(pump_level_adjust_idle_exit_cb, proc,
        PUMP_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME);
}

static void pump_level_adjust_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;

    (void)index;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    pump_sub_mode_sync_tick(proc, PUMP_SUB_MODE_SYNC_LEVEL);
    if (pump_adjust_exit_if_rest_mode(proc)) {
        return;
    }
    pump_level_over_limit_tick(proc);
    pump_sub_mode_switch_tick(proc);
    pump_level_adjust_idle_timeout_tick(proc);
    ui_display_pump_level_adjust_process(ui);
    pump_pause_led_tick(proc);
}

static void pump_level_adjust_animation_progress_cb(void *user_data, int index,
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
            pump_level_adjust_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void pump_level_adjust_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_level_adjust_prepare(ui);
}

static void pump_level_adjust_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_level_adjust_remove(ui);
}

static void pump_level_adjust_state_sync(ui_proc_pump_t *proc, uint8_t is_left)
{
    uint8_t side = is_left ? PUMP_LEVEL_ADJUST_SIDE_LEFT : PUMP_LEVEL_ADJUST_SIDE_RIGHT;
    uint8_t index = side;
    int pump_mode = 0;

    pump_sub_mode_state_init(proc, 1);
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    proc->level_adjust_side = side;
    proc->level_value = proc->setting.level[index];
    proc->level_arc_level = proc->level_value;
    proc->level_over_limit = 0;
    proc->level_inc_window_base = proc->level_value;
    proc->level_inc_window_time = get_timestamp();
    proc->level_adjust_time = get_timestamp();
    LOG_WM("pump level adjust sync side=%d mode=%d type=%d max=%d level=%d",
        side, pump_mode, proc->sub_mode_type,
        home_level_adjust_max_get(proc->sub_mode_type), proc->level_value);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_level_adjust_ui(ui_proc_pump_t *proc, uint8_t is_left)
{
    ui_manager_context_t *ui;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);

    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    pump_level_adjust_state_sync(proc, is_left);

    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST) {
        /* 同页切换左右侧，仅刷新状态 */
        return;
    }

    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);
    ui_play_info_t play_info = {
        .tag = "pump_level_adjust_screen",
        .fps = PUMP_LEVEL_ANIMATION_FREQ,
        .progress_cb = pump_level_adjust_animation_progress_cb,
        .display_prepare_cb = pump_level_adjust_ui_prepare,
        .display_remove_cb = pump_level_adjust_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = PUMP_UI_TYPE_LEVEL_ADJUST;
    proc->adjust_enter_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
}

static void pump_level_adjust_bury_report(ui_proc_pump_t *proc, const pump_setting_t *before)
{
    data_bury_trigger_manual_adjust_t bury_adj = {
        .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
        .evt_type = DATA_BURY_ADJUST_SUCTION_LEVEL,
        .adjust_from_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(before->level[0], before->level[1]),
        .adjust_to_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(proc->setting.level[0], proc->setting.level[1]),
    };
    if (bury_adj.adjust_from_desktop != bury_adj.adjust_to_desktop) {
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
    }
}

int pump_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase)
{
    uint8_t index;
    uint8_t level_max;
    uint8_t cur;
    uint8_t next;
    int time_index;
    pump_setting_t setting_before;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL_ADJUST) {
        return -1;
    }
    if (pump_adjust_exit_if_rest_mode(proc)) {
        return -1;
    }

    index = proc->level_adjust_side;
    if (index >= BREAST_PUMP_NUM) {
        return -1;
    }

    pump_sub_mode_switch_clear(proc);
    time_index = proc->sub_mode_time_index;
    READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    setting_before = proc->setting;
    level_max = home_level_adjust_max_get(proc->sub_mode_type);
    cur = proc->level_value;
    proc->level_adjust_time = get_timestamp();

    if (!is_increase) {
        pump_level_over_limit_clear(proc);
        if (cur > BREAST_PUMP_LEVEL_0) {
            cur--;
            proc->setting.level[index] = cur;
            WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
            proc->level_value = cur;
            pump_level_adjust_bury_report(proc, &setting_before);
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

    if (get_timestamp_ms(proc->level_inc_window_time) >= PUMP_LEVEL_OVER_LIMIT_WINDOW_MS) {
        proc->level_inc_window_base = cur;
        proc->level_inc_window_time = get_timestamp();
    }

    next = cur;
    if (next < level_max) {
        next++;
    }

    if ((next - proc->level_inc_window_base) > PUMP_LEVEL_OVER_LIMIT_MAX_STEP) {
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
        pump_level_adjust_bury_report(proc, &setting_before);
    }
    proc->level_arc_level = proc->level_value;
    return 0;
}

void pump_level_adjust_ui_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->level_over_limit = 0;
    proc->sub_mode_switch = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME);
}

void pump_level_adjust_ui_dismiss(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL_ADJUST) {
        return;
    }
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LEVEL_ADJUST_IDLE_EXIT_TASK_NAME);
#if PUMP_ADJUST_EXIT_TO_PREV_PAGE_ENABLE
    LOG_WM("dismiss pump level adjust, back prev page.");
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_KEEP_PAGE);
#else
    LOG_WM("dismiss pump level adjust, back home reset page.");
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
#endif
}
