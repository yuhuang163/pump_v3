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

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t pump_mode_page_index_max(int activate_status)
{
    return activate_status > 0 ?
            PUMP_MODE_PAGE_INDEX_CUSTOMIZE_SELECT :
            PUMP_MODE_PAGE_INDEX_POWER_PUMPING_SELECT;
}

static uint8_t pump_mode_from_page_index(uint16_t page_index)
{
    switch (page_index) {
        case PUMP_MODE_PAGE_INDEX_STIMULATION_SELECT:
            return BREAST_PUMP_STIMULATION_MODE;
        case PUMP_MODE_PAGE_INDEX_EXPRESSION_SELECT:
            return BREAST_PUMP_LACTATION_MODE;
        case PUMP_MODE_PAGE_INDEX_MILK_BOOST_SELECT:
            return BREAST_PUMP_MILK_BOOST_MODE;
        case PUMP_MODE_PAGE_INDEX_POWER_PUMPING_SELECT:
            return BREAST_PUMP_PP_MILK_TRACK_MODE;
        case PUMP_MODE_PAGE_INDEX_CUSTOMIZE_SELECT:
            return BREAST_PUMP_CUSTOM_MODE;
        default:
            return BREAST_PUMP_STIMULATION_MODE;
    }
}

static bool pump_mode_is_program_target(uint8_t mode_index)
{
    return mode_index == BREAST_PUMP_MILK_BOOST_MODE ||
           mode_index == BREAST_PUMP_CUSTOM_MODE ||
           mode_index == BREAST_PUMP_PP_MILK_TRACK_MODE;
}

static void pump_mode_apply_page_index(ui_proc_pump_t *proc, uint16_t page_index)
{
    if (proc->ui_mode_page_index == page_index) {
        return;
    }

    proc->ui_mode_page_index = page_index;
#if PUMP_MODE_SELECT_IDLE_EXIT_ENABLE
    if (page_index == PUMP_MODE_PAGE_INDEX_NONE) {
        proc->mode_select_idle_time = 0;
    } else {
        proc->mode_select_idle_time = get_timestamp();
    }
#endif
    if (page_index == PUMP_MODE_PAGE_INDEX_NONE) {
        LOG_UI("pump page_index -> HOME");
        return;
    }

    LOG_UI("pump page_index -> %d (preview)", page_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_mode_select_idle_touch(ui_proc_pump_t *proc)
{
#if PUMP_MODE_SELECT_IDLE_EXIT_ENABLE
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        return;
    }
    if (proc->ui_mode_page_index == PUMP_MODE_PAGE_INDEX_NONE) {
        return;
    }
    proc->mode_select_idle_time = get_timestamp();
#else
    (void)proc;
#endif
}

void pump_mode_select_idle_timeout_handler(ui_proc_pump_t *proc)
{
#if PUMP_MODE_SELECT_IDLE_EXIT_ENABLE
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        return;
    }
    if (proc->ui_mode_page_index == PUMP_MODE_PAGE_INDEX_NONE) {
        return;
    }
    /// 从子页 KEEP_PAGE 回首页等场景：计时未启动则先启动
    if (proc->mode_select_idle_time == 0) {
        proc->mode_select_idle_time = get_timestamp();
        return;
    }
    if (get_timestamp_ms(proc->mode_select_idle_time) < PUMP_MODE_SELECT_IDLE_TIMEOUT_MS) {
        return;
    }
    /// 静默退出预览：仅复位 page_index，不调用 confirm/set，不更换工作模式
    LOG_UI("pump mode select idle timeout, silent exit (no mode change)");
    pump_mode_apply_page_index(proc, PUMP_MODE_PAGE_INDEX_NONE);
#else
    (void)proc;
#endif
}

void pump_mode_page_index_clamp(ui_proc_pump_t *proc)
{
    int activate_status = 0;
    uint16_t page_max;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    page_max = pump_mode_page_index_max(activate_status);
    if (proc->ui_mode_page_index > page_max) {
        pump_mode_apply_page_index(proc, page_max);
    }
}

int pump_mode_set_handler(ui_proc_pump_t *proc, uint8_t mode_index)
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
        /// 切换到韵律
        if (pump_mode_is_program_target(mode_index)) {
            servers_manager_context_t *srv =
                (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
            if (srv && srv->dispatch_servers_event_wrapper) {
                srv->dispatch_servers_event_wrapper(srv, SERVERS_PUBLISH_DATA_RHYTHM_SWITCHED_REPORT);
            }
        }
    }
    READ_PUMP_MODE_NAME(proc->pump, mode_index, &mode_name);
    LOG_UI("pump set %s mode:%d", mode_name, mode_index);
    return 0;
}

int pump_mode_confirm_handler(ui_proc_pump_t *proc)
{
    uint8_t mode_index;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME &&
        proc->current_ui_type != PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        return -1;
    }
    if (proc->ui_mode_page_index == PUMP_MODE_PAGE_INDEX_NONE) {
        return -1;
    }

    mode_index = pump_mode_from_page_index(proc->ui_mode_page_index);
    LOG_UI("pump confirm page_index:%d", proc->ui_mode_page_index);
    return pump_mode_set_handler(proc, mode_index);
}

int pump_mode_rhythm_confirm_needed(ui_proc_pump_t *proc)
{
    uint8_t target_mode;
    int work_mode = 0;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        return -1;
    }
    if (proc->ui_mode_page_index == PUMP_MODE_PAGE_INDEX_NONE) {
        return -1;
    }

    target_mode = pump_mode_from_page_index(proc->ui_mode_page_index);
    if (!pump_mode_is_program_target(target_mode)) {
        return -1;
    }

    READ_PUMP_WORK_MODE(proc->pump, work_mode);
    if ((uint8_t)work_mode == target_mode) {
        return -1;
    }

#if (PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME == PUMP_RHYTHM_CONFIRM_SCHEME_1)
    /// 方案1：切到韵律（模式→韵律 / 韵律→韵律），本次泵奶首次才确认
    if (proc->rhythm_confirm_shown != 0) {
        return -1;
    }
#elif (PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME == PUMP_RHYTHM_CONFIRM_SCHEME_2)
    /// 方案2：目标为韵律模式即确认
#else
#error "PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME must be PUMP_RHYTHM_CONFIRM_SCHEME_1 or _2"
#endif

    return 0;
}

int pump_mode_adjust_handler(ui_proc_pump_t *proc, bool is_left)
{
    int activate_status = 0;
    uint16_t page_max;
    uint16_t next_index;

    EXCEPTION_CHECK_INT(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME) {
        return -1;
    }

    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    page_max = pump_mode_page_index_max(activate_status);

    /// 个性化失活时，若当前停在个性化页则先收束到右转边界
    if (proc->ui_mode_page_index > page_max) {
        pump_mode_apply_page_index(proc, page_max);
        return 0;
    }

    if (is_left) {
        /// page_index 为 0（主页）时不支持左旋
        if (proc->ui_mode_page_index == PUMP_MODE_PAGE_INDEX_NONE) {
            return -1;
        }
        next_index = (uint16_t)(proc->ui_mode_page_index - 1);
    } else {
        if (proc->ui_mode_page_index >= page_max) {
            return -1;
        }
        next_index = (uint16_t)(proc->ui_mode_page_index + 1);
    }
    pump_mode_apply_page_index(proc, next_index);

    return 0;
}
