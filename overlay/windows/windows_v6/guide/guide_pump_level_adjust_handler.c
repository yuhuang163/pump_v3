/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_guide.h"
#include "home_level_adjust_ui.h" // IWYU pragma: keep

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t guide_level_side_level_max_get(const ui_proc_guide_t *proc)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;
    uint8_t mode;
    uint8_t mode_type;

    if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
        slot = (uint8_t)proc->ui_mode_index;
    }
    mode = guide_mode_switch_mode_from_slot(slot);
    mode_type = home_level_adjust_mode_type_resolve(mode, PUMP_MODE_LACTATION);
    return home_level_adjust_max_get(mode_type);
}

#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
static void guide_level_over_limit_clear(ui_proc_guide_t *proc)
{
    if (proc->level_over_limit == 0) {
        return;
    }
    proc->level_over_limit = 0;
    proc->level_arc_level = proc->level_value;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_level_page_enter_sync(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->level_adjust_side > GUIDE_LEVEL_ADJUST_SIDE_RIGHT) {
        proc->level_adjust_side = GUIDE_LEVEL_ADJUST_SIDE_LEFT;
    }
    proc->level_value = proc->setting.level[proc->level_adjust_side];
    proc->level_arc_level = proc->level_value;
    proc->level_over_limit = 0;
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
    proc->level_inc_window_base = proc->level_value;
    proc->level_inc_window_time = get_timestamp();
#endif
    proc->last_adjust_time = get_timestamp();
    proc->level_knob_touched = 0;
}

#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
void guide_pump_level_over_limit_tick(ui_proc_guide_t *proc)
{
    uint32_t elapsed;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_LEVEL_SIDE) {
        return;
    }
    if (proc->level_over_limit == 0) {
        return;
    }
    elapsed = get_timestamp_ms(proc->level_over_limit_time);
    /* 1s：进度条开始回退到实际力度 */
    if (elapsed >= GUIDE_LEVEL_OVER_LIMIT_ARC_ROLLBACK_MS &&
        proc->level_arc_level != proc->level_value) {
        proc->level_arc_level = proc->level_value;
    }
    /* 2s：过大提醒文案消失 */
    if (elapsed >= GUIDE_LEVEL_OVER_LIMIT_TIP_CLEAR_MS) {
        guide_level_over_limit_clear(proc);
    }
}
#endif

int guide_pump_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase)
{
    uint8_t index;
    uint8_t level_max;
    uint8_t cur;
    uint8_t next;

    EXCEPTION_CHECK_INT(proc);
    if (proc->page_index != GUIDE_PAGE_LEVEL_SIDE) {
        return -1;
    }

    index = proc->level_adjust_side;
    if (index > GUIDE_LEVEL_ADJUST_SIDE_RIGHT) {
        return -1;
    }

    level_max = guide_level_side_level_max_get(proc);
    cur = proc->level_value;
    proc->last_adjust_time = get_timestamp();
    proc->level_knob_touched = 1;

    if (!is_increase) {
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
        guide_level_over_limit_clear(proc);
#endif
        if (cur > BREAST_PUMP_LEVEL_0) {
            cur--;
            proc->setting.level[index] = cur;
            proc->level_value = cur;
        }
        proc->level_arc_level = proc->level_value;
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
        proc->level_inc_window_base = proc->level_value;
        proc->level_inc_window_time = get_timestamp();
#endif
        return 0;
    }

#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
    /* 加档：过大提示中仅推进圆弧，不改实际力度 */
    if (proc->level_over_limit) {
        next = proc->level_arc_level;
        if (next < level_max) {
            next++;
        }
        proc->level_arc_level = next;
        proc->level_over_limit_time = get_timestamp();
        return 0;
    }

    if (get_timestamp_ms(proc->level_inc_window_time) >= GUIDE_LEVEL_OVER_LIMIT_WINDOW_MS) {
        proc->level_inc_window_base = cur;
        proc->level_inc_window_time = get_timestamp();
    }
#endif

    next = cur;
    if (next < level_max) {
        next++;
    }

#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
    if ((next - proc->level_inc_window_base) > GUIDE_LEVEL_OVER_LIMIT_MAX_STEP) {
        proc->level_over_limit = 1;
        proc->level_over_limit_time = get_timestamp();
        proc->level_arc_level = next;
        /* 实际力度不变 */
        return 0;
    }
#endif

    if (next != cur) {
        proc->setting.level[index] = next;
        proc->level_value = next;
    }
    proc->level_arc_level = proc->level_value;
    return 0;
}

uint8_t guide_pump_level_idle_remain_sec_get(const ui_proc_guide_t *proc)
{
    (void)proc;
    return 0;
}

void guide_pump_level_idle_timeout_handler(ui_proc_guide_t *proc)
{
    (void)proc;
}
