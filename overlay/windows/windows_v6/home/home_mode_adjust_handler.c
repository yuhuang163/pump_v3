/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t home_mode_page_index_max(int activate_status)
{
    return activate_status > 0 ?
            HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT :
            HOME_MODE_PAGE_INDEX_POWER_PUMPING_SELECT;
}

static uint8_t home_mode_from_page_index(uint16_t page_index)
{
    switch (page_index) {
        case HOME_MODE_PAGE_INDEX_STIMULATION_SELECT:
            return BREAST_PUMP_STIMULATION_MODE;
        case HOME_MODE_PAGE_INDEX_EXPRESSION_SELECT:
            return BREAST_PUMP_LACTATION_MODE;
        case HOME_MODE_PAGE_INDEX_MILK_BOOST_SELECT:
            return BREAST_PUMP_MILK_BOOST_MODE;
        case HOME_MODE_PAGE_INDEX_POWER_PUMPING_SELECT:
            return BREAST_PUMP_PP_MILK_TRACK_MODE;
        case HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT:
            return BREAST_PUMP_CUSTOM_MODE;
        default:
            return BREAST_PUMP_STIMULATION_MODE;
    }
}

static void home_mode_apply_page_index(ui_proc_home_t *proc, uint16_t page_index)
{
    if (proc->ui_mode_page_index == page_index) {
        return;
    }

    proc->ui_mode_page_index = page_index;
#if HOME_MODE_SELECT_IDLE_EXIT_ENABLE
    if (page_index == HOME_MODE_PAGE_INDEX_NONE) {
        proc->mode_select_idle_time = 0;
    } else {
        proc->mode_select_idle_time = get_timestamp();
    }
#endif
    if (page_index == HOME_MODE_PAGE_INDEX_NONE) {
        LOG_UI("home page_index -> HOME");
        return;
    }

    LOG_UI("home page_index -> %d (preview)", page_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_mode_select_idle_touch(ui_proc_home_t *proc)
{
#if HOME_MODE_SELECT_IDLE_EXIT_ENABLE
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        return;
    }
    if (proc->ui_mode_page_index == HOME_MODE_PAGE_INDEX_NONE) {
        return;
    }
    proc->mode_select_idle_time = get_timestamp();
#else
    (void)proc;
#endif
}

void home_mode_select_idle_timeout_handler(ui_proc_home_t *proc)
{
#if HOME_MODE_SELECT_IDLE_EXIT_ENABLE
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        return;
    }
    if (proc->ui_mode_page_index == HOME_MODE_PAGE_INDEX_NONE) {
        return;
    }
    /// 从子页 KEEP_PAGE 回首页等场景：计时未启动则先启动
    if (proc->mode_select_idle_time == 0) {
        proc->mode_select_idle_time = get_timestamp();
        return;
    }
    if (get_timestamp_ms(proc->mode_select_idle_time) < HOME_MODE_SELECT_IDLE_TIMEOUT_MS) {
        return;
    }
    /// 静默退出预览：仅复位 page_index，不调用 confirm/set，不更换工作模式
    LOG_UI("home mode select idle timeout, silent exit (no mode change)");
    home_mode_apply_page_index(proc, HOME_MODE_PAGE_INDEX_NONE);
#else
    (void)proc;
#endif
}

void home_mode_page_index_clamp(ui_proc_home_t *proc)
{
    int activate_status = 0;
    uint16_t page_max;
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    page_max = home_mode_page_index_max(activate_status);
    if (proc->ui_mode_page_index > page_max) {
        home_mode_apply_page_index(proc, page_max);
    }
}

int home_mode_set_handler(ui_proc_home_t *proc, uint8_t mode_index)
{
    char *mode_name = "unknow";
    int work_mode = 0;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    READ_PUMP_WORK_MODE(proc->pump, work_mode);
    if (work_mode != mode_index) {
        data_bury_trigger_manual_adjust_t bury_adj = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_ADJUST_PUMP_MODE,
            .adjust_from = work_mode,
            .adjust_to = mode_index,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
        WRITE_PUMP_WORK_MODE(proc->pump, mode_index, true, 1);
    }
    READ_PUMP_MODE_NAME(proc->pump, mode_index, &mode_name);
    LOG_UI("home set %s mode:%d", mode_name, mode_index);
    return 0;
}

int home_mode_confirm_handler(ui_proc_home_t *proc)
{
    uint8_t mode_index;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        return -1;
    }
    if (proc->ui_mode_page_index == HOME_MODE_PAGE_INDEX_NONE) {
        return -1;
    }

    mode_index = home_mode_from_page_index(proc->ui_mode_page_index);
    LOG_UI("home confirm page_index:%d", proc->ui_mode_page_index);
    return home_mode_set_handler(proc, mode_index);
}

int home_mode_adjust_handler(ui_proc_home_t *proc, bool is_left)
{
    int activate_status = 0;
    uint16_t page_max;
    uint16_t next_index;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        return -1;
    }

    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    page_max = home_mode_page_index_max(activate_status);

    /// 个性化失活时，若当前停在个性化页则先收束到右转边界
    if (proc->ui_mode_page_index > page_max) {
        home_mode_apply_page_index(proc, page_max);
        return 0;
    }

    if (is_left) {
        /// page_index 为 0（主页）时不支持左旋
        if (proc->ui_mode_page_index == HOME_MODE_PAGE_INDEX_NONE) {
            return -1;
        }
        next_index = (uint16_t)(proc->ui_mode_page_index - 1);
    } else {
        if (proc->ui_mode_page_index >= page_max) {
            return -1;
        }
        next_index = (uint16_t)(proc->ui_mode_page_index + 1);
    }
    home_mode_apply_page_index(proc, next_index);

    return 0;
}
