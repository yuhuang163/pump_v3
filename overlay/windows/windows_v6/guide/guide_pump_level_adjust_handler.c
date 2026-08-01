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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int guide_pump_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase)
{
    uint8_t index;
    uint8_t level_max;
    uint8_t cur;

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

    if (is_increase) {
        if (cur < level_max) {
            cur++;
        }
    } else if (cur > BREAST_PUMP_LEVEL_0) {
        cur--;
    }

    proc->level_value = cur;
    proc->level_arc_level = cur;
    proc->setting.level[index] = cur;
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
