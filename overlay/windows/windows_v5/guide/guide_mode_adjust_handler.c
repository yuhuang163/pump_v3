/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_guide.h"
#if IS_SUPPORT_GUIDE_PUMP_MODE_SYNC
#include "breast_pump_manager_api.h"
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t guide_normalize_common_mode(int mode)
{
    return PUMP_IS_COMMON_MODE(mode) ? (uint8_t)mode : BREAST_PUMP_STIMULATION_MODE;
}

static uint8_t guide_normalize_expert_mode(int mode)
{
    if (mode == BREAST_PUMP_PP_MILK_TRACK_MODE) {
        return BREAST_PUMP_PP_MILK_TRACK_MODE;
    }
    return BREAST_PUMP_MILK_BOOST_MODE;
}

static uint8_t guide_next_common_mode(int mode)
{
    uint8_t common = guide_normalize_common_mode(mode);
    return common < BREAST_PUMP_NORMAL_MODE_UPPER ?
        (uint8_t)(common + 1) : BREAST_PUMP_NORMAL_MODE_LOWER;
}

static uint8_t guide_next_expert_mode(int mode)
{
    mode = guide_normalize_expert_mode(mode);
    return mode == BREAST_PUMP_MILK_BOOST_MODE ?
        BREAST_PUMP_PP_MILK_TRACK_MODE : BREAST_PUMP_MILK_BOOST_MODE;
}

static uint8_t guide_cycle_pump_mode(uint8_t ui_mode_index, bool is_bf_mode)
{
    if (is_bf_mode) {
        return guide_next_expert_mode(ui_mode_index);
    }
    return guide_next_common_mode(ui_mode_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int guide_pump_mode_adjust_handler(ui_proc_guide_t *proc, bool is_bf_mode)
{
    EXCEPTION_CHECK_INT(proc);

    if (is_bf_mode) {
        if (!GUIDE_PAGE_IS_RHYTHM_INTERACTIVE(proc->page_index)) {
            return -1;
        }
    } else if (!GUIDE_PAGE_IS_MODE_INTERACTIVE(proc->page_index)) {
        return -1;
    }

    proc->ui_mode_index = guide_cycle_pump_mode(proc->ui_mode_index, is_bf_mode);
    if (is_bf_mode) {
        proc->rhythm_select_touched = 1;
        if (proc->rhythm_select_click_cnt < 0xFF) {
            proc->rhythm_select_click_cnt++;
        }
    } else {
        proc->mode_select_touched = 1;
        if (proc->mode_select_click_cnt < 0xFF) {
            proc->mode_select_click_cnt++;
        }
    }
#if IS_SUPPORT_GUIDE_PUMP_MODE_SYNC
    if (proc->pump != NULL) {
        WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_mode_index, true, 1);
    }
#endif
    proc->last_adjust_time = get_timestamp();
    return 0;
}
