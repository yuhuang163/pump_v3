/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_guide.h"
#if IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC
#include "breast_pump_manager_api.h"
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_level_adjust_side(ui_proc_guide_t *proc, uint8_t index, bool is_increase)
{
    if (is_increase) {
        if (proc->setting.level[index] < GUIDE_LEVEL_MAX) {
            proc->setting.level[index]++;
        }
    } else if (proc->setting.level[index] > GUIDE_LEVEL_MIN) {
        proc->setting.level[index]--;
    }
}

static void guide_level_adjust_both(ui_proc_guide_t *proc, bool is_increase)
{
    guide_level_adjust_side(proc, 0, is_increase);
    guide_level_adjust_side(proc, 1, is_increase);
}

static bool guide_level_side_countdown_locked(const ui_proc_guide_t *proc)
{
    return proc->level_knob_ready_time != 0;
}

static void guide_level_side_countdown_lock_if_ready(ui_proc_guide_t *proc)
{
    if (!proc->level_knob_touched || guide_level_side_countdown_locked(proc)) {
        return;
    }
    if (get_timestamp_ms(proc->last_adjust_time) >= GUIDE_LEVEL_KNOB_ADVANCE_MS) {
        /// 结算完成，锁定倒计时起点，后续旋转不再刷新
        proc->level_knob_ready_time = 1;
    }
}

static void guide_level_side_advance_timer_sync(ui_proc_guide_t *proc)
{
    proc->level_knob_touched = 1;
    if (guide_level_side_countdown_locked(proc)) {
        return;
    }
}

static bool guide_level_both_settling(const ui_proc_guide_t *proc)
{
    return proc->level_knob_ready_time != 0;
}

static bool guide_level_both_advance_target_reached(const ui_proc_guide_t *proc)
{
    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
            return proc->setting.level[1] >= GUIDE_LEVEL_BOTH_INCREASE_TARGET;
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
            return proc->setting.level[0] <= GUIDE_LEVEL_BOTH_DECREASE_TARGET;
        default:
            return false;
    }
}

static void guide_level_both_settlement_start_if_ready(ui_proc_guide_t *proc)
{
    if (!guide_level_both_advance_target_reached(proc) || guide_level_both_settling(proc)) {
        return;
    }
    proc->level_knob_touched = 1;
    proc->level_knob_ready_time = get_timestamp();
}

static void guide_level_both_advance_if_settled(ui_proc_guide_t *proc)
{
    if (!guide_level_both_settling(proc)) {
        return;
    }
    if (get_timestamp_ms(proc->level_knob_ready_time) < GUIDE_LEVEL_BOTH_SETTLE_MS) {
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
            guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_DECREASE);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
            guide_page_advance(proc, GUIDE_PAGE_LEVEL_LEFT_GUIDE);
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_level_page_enter_reset(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->last_adjust_time = get_timestamp();
    proc->level_knob_touched = 0;
    proc->level_knob_ready_time = 0;
    guide_level_both_settlement_start_if_ready(proc);
}

int guide_pump_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_INT(proc);

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
            if (guide_level_both_settling(proc)) {
                return -1;
            }
            break;
        case GUIDE_PAGE_LEVEL_LEFT:
        case GUIDE_PAGE_LEVEL_RIGHT:
            break;
        default:
            return -1;
    }

    if (GUIDE_PAGE_IS_LEVEL_SIDE(proc->page_index)) {
        guide_level_side_countdown_lock_if_ready(proc);
    }

#if IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC
    if (proc->pump != NULL) {
        READ_PUMP_SETTING_INFO(proc->pump, 0, proc->setting);
    }
#endif

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
            guide_level_adjust_both(proc, is_increase);
            break;
        case GUIDE_PAGE_LEVEL_LEFT:
            guide_level_adjust_side(proc, 0, is_increase);
            break;
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_level_adjust_side(proc, 1, is_increase);
            break;
        default:
            return -1;
    }

#if IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC
    if (proc->pump != NULL) {
        WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
    }
#endif

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
            guide_level_both_settlement_start_if_ready(proc);
            break;
        default:
            guide_level_side_advance_timer_sync(proc);
            break;
    }
    if ((proc->page_index == GUIDE_PAGE_LEVEL_BOTH_INCREASE ||
         proc->page_index == GUIDE_PAGE_LEVEL_BOTH_DECREASE) &&
        guide_level_both_settling(proc)) {
        return 0;
    }
    if (!GUIDE_PAGE_IS_LEVEL_SIDE(proc->page_index) || !guide_level_side_countdown_locked(proc)) {
        proc->last_adjust_time = get_timestamp();
    }
    return 0;
}

uint8_t guide_pump_level_idle_remain_sec_get(const ui_proc_guide_t *proc)
{
    (void)proc;
    /// 左/右力度页跳转倒计时不在 UI 展示，仅用于后台计时切页
    return 0;
}

void guide_pump_level_idle_timeout_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (!GUIDE_PAGE_IS_LEVEL_INTERACTIVE(proc->page_index)) {
        return;
    }
    if (!proc->level_knob_touched) {
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
            guide_level_both_advance_if_settled(proc);
            return;
        case GUIDE_PAGE_LEVEL_LEFT:
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_level_side_countdown_lock_if_ready(proc);
            if (get_timestamp_ms(proc->last_adjust_time) <
                GUIDE_LEVEL_KNOB_ADVANCE_MS + GUIDE_LEVEL_SIDE_COUNTDOWN_MS) {
                return;
            }
            break;
        default:
            return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_LEFT:
            guide_page_advance(proc, GUIDE_PAGE_LEVEL_RIGHT_GUIDE);
            break;
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_page_advance(proc, GUIDE_PAGE_PUMP_STOP_GUIDE);
            break;
        default:
            break;
    }
}
