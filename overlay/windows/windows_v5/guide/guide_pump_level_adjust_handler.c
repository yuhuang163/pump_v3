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
static void guide_level_adjust_both(ui_proc_guide_t *proc, bool is_increase)
{
    if (is_increase) {
        if (proc->setting.level[0] < GUIDE_LEVEL_MAX) {
            proc->setting.level[0]++;
        }
        if (proc->setting.level[1] < GUIDE_LEVEL_MAX) {
            proc->setting.level[1]++;
        }
    } else {
        if (proc->setting.level[0] > GUIDE_LEVEL_MIN) {
            proc->setting.level[0]--;
        }
        if (proc->setting.level[1] > GUIDE_LEVEL_MIN) {
            proc->setting.level[1]--;
        }
    }
}

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

static uint32_t guide_level_idle_silent_ms_get(const ui_proc_guide_t *proc)
{
    return proc->level_knob_touched ?
        GUIDE_IDLE_SILENT_LEVEL_TOUCH_MS : GUIDE_IDLE_SILENT_FIRST_MS;
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
}

int guide_pump_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_INT(proc);

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH:
        case GUIDE_PAGE_LEVEL_LEFT:
        case GUIDE_PAGE_LEVEL_RIGHT:
            break;
        default:
            return -1;
    }

#if IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC
    if (proc->pump != NULL) {
        READ_PUMP_SETTING_INFO(proc->pump, 0, proc->setting);
    }
#endif

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH:
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

    proc->level_knob_touched = 1;
    proc->last_adjust_time = get_timestamp();
    return 0;
}

uint8_t guide_pump_level_idle_remain_sec_get(const ui_proc_guide_t *proc)
{
    if (proc == NULL || !GUIDE_PAGE_IS_LEVEL_INTERACTIVE(proc->page_index)) {
        return 0;
    }

    return guide_idle_countdown_remain_sec(get_timestamp_ms(proc->last_adjust_time),
        guide_level_idle_silent_ms_get(proc));
}

void guide_pump_level_idle_timeout_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (!GUIDE_PAGE_IS_LEVEL_INTERACTIVE(proc->page_index)) {
        return;
    }
    if (get_timestamp_ms(proc->last_adjust_time) <
        guide_idle_total_timeout_ms(guide_level_idle_silent_ms_get(proc))) {
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_BOTH:
            guide_page_advance(proc, GUIDE_PAGE_LEVEL_LEFT_GUIDE);
            break;
        case GUIDE_PAGE_LEVEL_LEFT:
            guide_page_advance(proc, GUIDE_PAGE_LEVEL_RIGHT_GUIDE);
            break;
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_page_advance(proc, GUIDE_PAGE_MODE_SELECT_GUIDE);
            break;
        default:
            break;
    }
}
