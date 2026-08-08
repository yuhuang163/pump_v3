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
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void switch_pump_status_handler(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM ||
        proc->current_ui_type == PUMP_UI_TYPE_WILL_FINISH ||
        proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT) {
        LOG_WM("ignore start key %d", proc->current_ui_type);
        return;
    }

#if 0
    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS) {
        LOG_WM("settings success, ignore start key.");
        return;
    }
#endif

    switch_pump_status_ui(proc);
}

static void pump_ui_nav_restore_frame(ui_proc_pump_t *proc, const pump_ui_nav_frame_t *frame)
{
    EXCEPTION_CHECK_VOID(proc && frame);
    proc->ui_mode_page_index = frame->page_index;
    proc->ui_nav.restoring = 1;
    switch (frame->ui_type) {
        case PUMP_UI_TYPE_LEVEL_ADJUST:
            show_pump_level_adjust_ui(proc,
                frame->level_side == PUMP_LEVEL_ADJUST_SIDE_LEFT);
            break;
        case PUMP_UI_TYPE_BOTH_LEVEL_ADJUST:
            show_pump_both_level_adjust_ui(proc);
            break;
        case PUMP_UI_TYPE_SPEED_ADJUST:
            show_pump_speed_adjust_ui(proc);
            break;
        case PUMP_UI_TYPE_HOME:
        default:
            show_pump_home_ui(proc);
            break;
    }
    proc->ui_nav.restoring = 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_ui_nav_push(ui_proc_pump_t *proc)
{
    pump_ui_nav_frame_t *frame;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->ui_nav.restoring) {
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_NONE) {
        return;
    }
    if (proc->ui_nav.depth >= PUMP_UI_NAV_DEPTH_MAX) {
        /* 栈满：丢掉最旧帧，整体前移 */
        memmove(&proc->ui_nav.stack[0], &proc->ui_nav.stack[1],
            sizeof(proc->ui_nav.stack[0]) * (PUMP_UI_NAV_DEPTH_MAX - 1));
        proc->ui_nav.depth = PUMP_UI_NAV_DEPTH_MAX - 1;
    }
    frame = &proc->ui_nav.stack[proc->ui_nav.depth++];
    frame->ui_type = proc->current_ui_type;
    frame->page_index = proc->ui_mode_page_index;
    frame->level_side = proc->level_adjust_side;
#if PUMP_MODE_SELECT_IDLE_EXIT_ENABLE
    /// 离开首页进子页时暂停倒计时，返回后重新计时
    proc->mode_select_idle_time = 0;
#endif
    LOG_WM("pump ui_nav push type:%d page:%d depth:%d",
        frame->ui_type, frame->page_index, proc->ui_nav.depth);
}

void pump_ui_nav_clear(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    memset(&proc->ui_nav, 0, sizeof(proc->ui_nav));
}

void pump_ui_nav_back(ui_proc_pump_t *proc, pump_ui_nav_back_e policy)
{
    pump_ui_nav_frame_t frame = {0};

    EXCEPTION_CHECK_VOID(proc);
    if (proc->ui_nav.depth > 0) {
        frame = proc->ui_nav.stack[--proc->ui_nav.depth];
    }

    LOG_WM("pump ui_nav back policy:%d prev_type:%d page:%d depth:%d",
        policy, frame.ui_type, frame.page_index, proc->ui_nav.depth);

    switch (policy) {
        case PUMP_UI_NAV_BACK_HISTORY:
            if (frame.ui_type != PUMP_UI_TYPE_NONE) {
                pump_ui_nav_restore_frame(proc, &frame);
            } else {
                show_pump_home_ui(proc);
            }
            break;
        case PUMP_UI_NAV_BACK_HOME_KEEP_PAGE:
            if (frame.ui_type != PUMP_UI_TYPE_NONE) {
                proc->ui_mode_page_index = frame.page_index;
            }
            pump_ui_nav_clear(proc);
            proc->ui_nav.restoring = 1;
            show_pump_home_ui(proc);
            proc->ui_nav.restoring = 0;
            break;
        case PUMP_UI_NAV_BACK_HOME_RESET_PAGE:
        default:
            proc->ui_mode_page_index = PUMP_MODE_PAGE_INDEX_NONE;
            pump_ui_nav_clear(proc);
            proc->ui_nav.restoring = 1;
            show_pump_home_ui(proc);
            proc->ui_nav.restoring = 0;
            break;
    }
}

void stop_pump_ui_animation(ui_proc_pump_t *proc)
{
    pump_rhythm_confirm_ui_exit(proc);
    pump_settings_success_ui_exit(proc);
    pump_stop_hold_ui_exit(proc);
    pump_finish_timeout_ui_exit(proc);
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
    pump_pause_timeout_ui_exit(proc);
#endif
    pump_will_finish_ui_exit(proc);
    pump_level_adjust_ui_exit(proc);
    pump_both_level_adjust_ui_exit(proc);
    pump_speed_adjust_ui_exit(proc);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
    proc->ui_mode_page_index = PUMP_MODE_PAGE_INDEX_NONE;
    pump_ui_nav_clear(proc);
}

void pump_ui_change_hdr_remove(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case PUMP_UI_TYPE_RHYTHM_CONFIRM:
            pump_rhythm_confirm_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_SETTINGS_SUCCESS:
            pump_settings_success_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_STOP_HOLD:
            pump_stop_hold_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_FINISH_TIMEOUT:
            pump_finish_timeout_ui_exit(proc);
            break;
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
        case PUMP_UI_TYPE_PAUSE_TIMEOUT:
            pump_pause_timeout_ui_exit(proc);
            break;
#endif
        case PUMP_UI_TYPE_WILL_FINISH:
            pump_will_finish_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_LEVEL_ADJUST:
            pump_level_adjust_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_BOTH_LEVEL_ADJUST:
            pump_both_level_adjust_ui_exit(proc);
            break;
        case PUMP_UI_TYPE_SPEED_ADJUST:
            pump_speed_adjust_ui_exit(proc);
            break;
        default:
            break;
    }
}

void pump_popup_ui_type_update(ui_proc_pump_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}

static bool pump_ui_blink_update(ui_proc_pump_t *proc)
{
    /// 相对半周期翻转，避免按绝对时间重算相位导致切页时抖动
    if (get_timestamp_ms(proc->blink_start_time) < PUMP_BLINK_PERIOD) {
        return false;
    }
    proc->blink_start_time = get_timestamp();
    proc->state_blink_visible = proc->state_blink_visible ? 0 : 1;
    return true;
}

static bool pump_pause_led_needs_update(const ui_proc_pump_t *proc)
{
    /* 结束类弹窗/结算页由窗口侧关灯，此处不抢控 */
    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT) {
        return false;
    }
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_LOW_POWER ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_TIMEOUT ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        return false;
    }
    if (proc->pump_status == BREAST_PUMP_PAUSE) {
        if (proc->pump_light_status != PUMP_LIGHT_STATUS_PAUSE) {
            return true;
        }
        return proc->pump_pause_blink_state != proc->state_blink_visible;
    }
    return proc->pump_light_status == PUMP_LIGHT_STATUS_PAUSE;
}

static void pump_pause_led_update(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->led);

    if (proc->current_ui_type == PUMP_UI_TYPE_PAUSE_TIMEOUT ||
        proc->current_ui_type == PUMP_UI_TYPE_FINISH_TIMEOUT) {
        return;
    }
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_LOW_POWER ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_TIMEOUT ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
        return;
    }

    if (proc->pump_status == BREAST_PUMP_PAUSE) {
        const bool entering_pause = proc->pump_light_status != PUMP_LIGHT_STATUS_PAUSE;
        if (entering_pause) {
            proc->blink_start_time = get_timestamp();
            proc->state_blink_visible = 1;
            proc->pump_light_status = PUMP_LIGHT_STATUS_PAUSE;
        }
        if (entering_pause || proc->pump_pause_blink_state != proc->state_blink_visible) {
            proc->led->set_led_level(PUMP_START_BL,
                proc->state_blink_visible ? LED_LEVEL_MAX : 0);
            proc->pump_pause_blink_state = proc->state_blink_visible;
        }
        return;
    }

    if (proc->pump_light_status == PUMP_LIGHT_STATUS_PAUSE) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        proc->pump_light_status = PUMP_LIGHT_STATUS_DEFAULT;
        proc->pump_pause_blink_state = proc->state_blink_visible;
    }
}

void pump_pause_led_tick(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    const bool blink_changed = pump_ui_blink_update(proc);
    if (blink_changed || pump_pause_led_needs_update(proc)) {
        pump_pause_led_update(proc);
    }
}

void pump_power_key_event_handler(ui_proc_pump_t *proc)
{
    if (proc->stop_hold_ignore_click) {
        proc->stop_hold_ignore_click = 0;
        LOG_WM("ignore power click after stop hold release.");
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL_ADJUST) {
        pump_level_adjust_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_BOTH_LEVEL_ADJUST) {
        pump_both_level_adjust_ui_dismiss(proc);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED_ADJUST) {
        pump_speed_adjust_ui_dismiss(proc);
        return;
    }

    /// 确认页：再次短按 power → 落地模式并跳转成功页（超时后仅暂停态启动泵奶）
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        if (pump_mode_confirm_handler(proc) == 0) {
            show_pump_settings_success_ui(proc, true);
        }
        return;
    }

    /// 模式选择页：满足条件先弹确认页，否则直接成功页（超时后仅暂停态启动泵奶）
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        if (proc->ui_mode_page_index > PUMP_MODE_PAGE_INDEX_NONE) {
            if (pump_mode_rhythm_confirm_needed(proc) == 0) {
                show_pump_rhythm_confirm_ui(proc);
                return;
            }
            if (pump_mode_confirm_handler(proc) == 0) {
                show_pump_settings_success_ui(proc, true);
            }
            return;
        }
        switch_pump_status_handler(proc);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_SETTINGS_SUCCESS) {
        pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_STOP_HOLD) {
        pump_stop_hold_dismiss(proc);
        return;
    }

#if IS_SUPPORT_NIGHT_LIGHT
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        uint8_t battery_state = proc->battery->get_battery_state();
        if (battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
            battery_state == BATTERY_STATE_EMPTY ||
            battery_state == BATTERY_STATE_FORBID_START) {
            LOG_WM("battery state is low, ignore power key event.");
            return;
        }
        
#if IS_SUPPORT_DEVICE_NIGHT_LIGHT_TOGGLE
        if (proc->led && proc->led->hook && proc->led->hook->sync_night_light_power_key) {
            proc->led->hook->sync_night_light_power_key(proc->led);
        }
        return;
#endif
    }
#endif
    LOG_WM("ignore power key event.");
}

void pump_start_key_event_handler(ui_proc_pump_t *proc)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        pump_rhythm_confirm_start_key_handler(proc);
        return;
    }
    switch_pump_status_handler(proc);
}

void pump_start_pump_if_pause(ui_proc_pump_t *proc)
{
    int pump_status = 0;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_PAUSE) {
        switch_pump_status_ui(proc);
    }
}

uint8_t pump_sub_mode_type_get(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->motor == NULL || proc->motor->get_runtime_mode_type == NULL) {
        return PUMP_MODE_LACTATION;
    }
    return proc->motor->get_runtime_mode_type(false);
}

void pump_sub_mode_switch_clear(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->sub_mode_switch = 0;
}

void pump_sub_mode_switch_tick(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->sub_mode_switch == 0) {
        return;
    }
    if (get_timestamp_ms(proc->sub_mode_switch_time) < PUMP_SUB_MODE_SWITCH_TIMEOUT_MS) {
        return;
    }
    pump_sub_mode_switch_clear(proc);
}

static uint8_t pump_sub_mode_level_clamp(ui_proc_pump_t *proc, uint8_t mode_type)
{
    uint8_t level_max = home_level_adjust_max_get(mode_type);
    uint8_t i;
    bool changed = false;

    READ_PUMP_SETTING_INFO(proc->pump, proc->sub_mode_time_index, proc->setting);
    for (i = 0; i < BREAST_PUMP_NUM; i++) {
        if (proc->setting.level[i] > level_max) {
            proc->setting.level[i] = level_max;
            changed = true;
        }
    }
    if (changed) {
        WRITE_PUMP_SETTING_INFO(proc->pump, proc->sub_mode_time_index, true, proc->setting);
    }
    return level_max;
}

void pump_sub_mode_state_init(ui_proc_pump_t *proc, uint8_t clamp_level)
{
    int pump_mode = 0;
    int time_index = 0;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    proc->sub_mode_time_index = time_index;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    proc->sub_mode_type = home_level_adjust_mode_type_resolve((uint8_t)pump_mode,
        pump_sub_mode_type_get(proc));
    proc->sub_mode_switch = 0;
    READ_PUMP_SETTING_INFO(proc->pump, proc->sub_mode_time_index, proc->setting);
    if (clamp_level) {
        pump_sub_mode_level_clamp(proc, proc->sub_mode_type);
    }
}

void pump_sub_mode_sync_tick(ui_proc_pump_t *proc, pump_sub_mode_sync_e sync)
{
    int pump_mode = 0;
    int time_index = 0;
    uint8_t mode_type;
    uint8_t switched = 0;
    uint8_t i;
    uint8_t index;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    mode_type = home_level_adjust_mode_type_resolve((uint8_t)pump_mode,
        pump_sub_mode_type_get(proc));

    if (proc->sub_mode_time_index != time_index) {
        if (proc->sub_mode_type != 0xFF) {
            switched = 1;
            LOG_WM("pump sub mode time %d->%d sync=%d",
                proc->sub_mode_time_index, time_index, sync);
        }
        proc->sub_mode_time_index = time_index;
    }

    if (proc->sub_mode_type != mode_type) {
        if (proc->sub_mode_type != 0xFF) {
            switched = 1;
            proc->sub_mode_switch = 1;
            proc->sub_mode_switch_time = get_timestamp();
            LOG_WM("pump sub mode type %d->%d sync=%d",
                proc->sub_mode_type, mode_type, sync);
        }
        proc->sub_mode_type = mode_type;
    }

    if (sync == PUMP_SUB_MODE_SYNC_LEVEL || sync == PUMP_SUB_MODE_SYNC_BOTH_LEVEL) {
        pump_sub_mode_level_clamp(proc, mode_type);
    } else if (sync == PUMP_SUB_MODE_SYNC_SPEED && switched) {
        READ_PUMP_SETTING_INFO(proc->pump, proc->sub_mode_time_index, proc->setting);
    }

    if (switched) {
        if (sync == PUMP_SUB_MODE_SYNC_LEVEL) {
            if (proc->level_over_limit) {
                proc->level_over_limit = 0;
                proc->level_arc_level = proc->level_value;
            }
        } else if (sync == PUMP_SUB_MODE_SYNC_BOTH_LEVEL) {
            if (proc->both_level_over_limit) {
                proc->both_level_over_limit = 0;
                proc->both_level_arc[0] = proc->both_level_value[0];
                proc->both_level_arc[1] = proc->both_level_value[1];
            }
        }
    }

    if (sync == PUMP_SUB_MODE_SYNC_LEVEL) {
        index = proc->level_adjust_side;
        if (index < BREAST_PUMP_NUM && proc->level_over_limit == 0) {
            proc->level_value = proc->setting.level[index];
            proc->level_arc_level = proc->level_value;
        }
    } else if (sync == PUMP_SUB_MODE_SYNC_BOTH_LEVEL) {
        if (proc->both_level_over_limit == 0) {
            for (i = 0; i < BREAST_PUMP_NUM; i++) {
                proc->both_level_value[i] = proc->setting.level[i];
                proc->both_level_arc[i] = proc->both_level_value[i];
            }
        }
    }
}
