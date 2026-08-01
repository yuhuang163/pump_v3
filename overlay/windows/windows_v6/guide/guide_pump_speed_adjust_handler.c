/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_guide.h"

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_speed_page_enter_sync(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    /// 力度沿用第十八页退出前的 setting.level / both_level_value
    if (proc->setting.speed >= BREAST_PUMP_SPEED_MAX) {
        proc->setting.speed = (BREAST_PUMP_SPEED_MAX - 1);
    }
    proc->last_adjust_time = get_timestamp();
}

int guide_pump_speed_adjust_handler(ui_proc_guide_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->page_index != GUIDE_PAGE_SPEED) {
        return -1;
    }

    proc->last_adjust_time = get_timestamp();
    if (is_increase) {
        if (proc->setting.speed < (BREAST_PUMP_SPEED_MAX - 1)) {
            proc->setting.speed++;
        }
    } else if (proc->setting.speed > BREAST_PUMP_SPEED_LOW) {
        proc->setting.speed--;
    }
    return 0;
}
