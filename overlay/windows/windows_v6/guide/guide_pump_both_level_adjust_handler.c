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

static void guide_both_level_over_limit_clear(ui_proc_guide_t *proc)
{
    if (proc->both_level_over_limit == 0) {
        return;
    }
    proc->both_level_over_limit = 0;
    proc->both_level_arc[0] = proc->both_level_value[0];
    proc->both_level_arc[1] = proc->both_level_value[1];
}

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
    proc->both_level_inc_steps = 0;
    proc->both_level_inc_window_time = get_timestamp();
    proc->last_adjust_time = get_timestamp();
}

void guide_pump_both_level_over_limit_tick(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_LEVEL_BOTH) {
        return;
    }
    if (proc->both_level_over_limit == 0) {
        return;
    }
    if (get_timestamp_ms(proc->both_level_over_limit_time) < GUIDE_LEVEL_OVER_LIMIT_WINDOW_MS) {
        return;
    }
    guide_both_level_over_limit_clear(proc);
}

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
        guide_both_level_over_limit_clear(proc);
        for (i = 0; i < BREAST_PUMP_NUM; i++) {
            if (proc->both_level_value[i] > BREAST_PUMP_LEVEL_0) {
                proc->both_level_value[i]--;
                proc->setting.level[i] = proc->both_level_value[i];
                changed = true;
            }
            proc->both_level_arc[i] = proc->both_level_value[i];
        }
        (void)changed;
        proc->both_level_inc_steps = 0;
        proc->both_level_inc_window_time = get_timestamp();
        return 0;
    }

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

    if ((proc->both_level_inc_steps + 1) > GUIDE_LEVEL_OVER_LIMIT_MAX_STEP) {
        proc->both_level_over_limit = 1;
        proc->both_level_over_limit_time = get_timestamp();
        proc->both_level_arc[0] = next_l;
        proc->both_level_arc[1] = next_r;
        return 0;
    }

    proc->both_level_value[0] = next_l;
    proc->both_level_value[1] = next_r;
    proc->setting.level[0] = next_l;
    proc->setting.level[1] = next_r;
    proc->both_level_inc_steps++;
    proc->both_level_arc[0] = proc->both_level_value[0];
    proc->both_level_arc[1] = proc->both_level_value[1];
    return 0;
}
