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
static uint8_t guide_both_level_level_max_get(const ui_proc_guide_t *proc)
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
static void guide_both_level_over_limit_clear(ui_proc_guide_t *proc)
{
    if (proc->both_level_over_limit == 0) {
        return;
    }
    proc->both_level_over_limit = 0;
    proc->both_level_arc[0] = proc->both_level_value[0];
    proc->both_level_arc[1] = proc->both_level_value[1];
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_both_level_page_enter_sync(ui_proc_guide_t *proc)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(proc);
    for (i = 0; i < BREAST_PUMP_NUM; i++) {
        proc->both_level_value[i] = proc->setting.level[i];
        proc->both_level_arc[i] = proc->both_level_value[i];
    }
    proc->both_level_over_limit = 0;
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
    proc->both_level_inc_steps = 0;
    proc->both_level_inc_window_time = get_timestamp();
#endif
    proc->last_adjust_time = get_timestamp();
}

#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
void guide_pump_both_level_over_limit_tick(ui_proc_guide_t *proc)
{
    uint32_t elapsed;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_LEVEL_BOTH) {
        return;
    }
    if (proc->both_level_over_limit == 0) {
        return;
    }
    elapsed = get_timestamp_ms(proc->both_level_over_limit_time);
    /* 1s：进度条开始回退到实际力度 */
    if (elapsed >= GUIDE_LEVEL_OVER_LIMIT_ARC_ROLLBACK_MS) {
        if (proc->both_level_arc[0] != proc->both_level_value[0] ||
            proc->both_level_arc[1] != proc->both_level_value[1]) {
            proc->both_level_arc[0] = proc->both_level_value[0];
            proc->both_level_arc[1] = proc->both_level_value[1];
        }
    }
    /* 2s：过大提醒文案消失 */
    if (elapsed >= GUIDE_LEVEL_OVER_LIMIT_TIP_CLEAR_MS) {
        guide_both_level_over_limit_clear(proc);
    }
}
#endif

int guide_pump_both_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase)
{
    uint8_t level_max;
    uint8_t i;
    uint8_t next_l;
    uint8_t next_r;
    bool changed = false;

    EXCEPTION_CHECK_INT(proc);
    if (proc->page_index != GUIDE_PAGE_LEVEL_BOTH) {
        return -1;
    }

    level_max = guide_both_level_level_max_get(proc);
    proc->last_adjust_time = get_timestamp();

    if (!is_increase) {
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
        guide_both_level_over_limit_clear(proc);
#endif
        for (i = 0; i < BREAST_PUMP_NUM; i++) {
            if (proc->both_level_value[i] > BREAST_PUMP_LEVEL_0) {
                proc->both_level_value[i]--;
                proc->setting.level[i] = proc->both_level_value[i];
                changed = true;
            }
            proc->both_level_arc[i] = proc->both_level_value[i];
        }
        (void)changed;
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
        proc->both_level_inc_steps = 0;
        proc->both_level_inc_window_time = get_timestamp();
#endif
        return 0;
    }

#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
    if (proc->both_level_over_limit) {
        for (i = 0; i < BREAST_PUMP_NUM; i++) {
            if (proc->both_level_arc[i] < level_max) {
                proc->both_level_arc[i]++;
            }
        }
        proc->both_level_over_limit_time = get_timestamp();
        return 0;
    }

    if (get_timestamp_ms(proc->both_level_inc_window_time) >= GUIDE_LEVEL_OVER_LIMIT_WINDOW_MS) {
        proc->both_level_inc_steps = 0;
        proc->both_level_inc_window_time = get_timestamp();
    }
#endif

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

#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
    if ((proc->both_level_inc_steps + 1) > GUIDE_LEVEL_OVER_LIMIT_MAX_STEP) {
        proc->both_level_over_limit = 1;
        proc->both_level_over_limit_time = get_timestamp();
        proc->both_level_arc[0] = next_l;
        proc->both_level_arc[1] = next_r;
        return 0;
    }
#endif

    proc->both_level_value[0] = next_l;
    proc->both_level_value[1] = next_r;
    proc->setting.level[0] = next_l;
    proc->setting.level[1] = next_r;
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
    proc->both_level_inc_steps++;
#endif
    proc->both_level_arc[0] = proc->both_level_value[0];
    proc->both_level_arc[1] = proc->both_level_value[1];
    return 0;
}
