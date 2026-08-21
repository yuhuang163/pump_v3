/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

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
static void home_ui_nav_restore_frame(ui_proc_home_t *proc, const home_ui_nav_frame_t *frame)
{
    EXCEPTION_CHECK_VOID(proc && frame);
    proc->ui_mode_page_index = frame->page_index;
    proc->ui_nav.restoring = 1;
    switch (frame->ui_type) {
        case HOME_UI_TYPE_LEVEL_ADJUST:
            show_home_level_adjust_ui(proc,
                frame->level_side == HOME_LEVEL_ADJUST_SIDE_LEFT);
            break;
        case HOME_UI_TYPE_BOTH_LEVEL_ADJUST:
            show_home_both_level_adjust_ui(proc);
            break;
        case HOME_UI_TYPE_SPEED_ADJUST:
            show_home_speed_adjust_ui(proc);
            break;
        case HOME_UI_TYPE_HOME:
        default:
            start_home_ui_animation(proc);
            break;
    }
    proc->ui_nav.restoring = 0;
}

static uint8_t home_sub_mode_level_clamp(ui_proc_home_t *proc, uint8_t mode_type)
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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_ui_nav_push(ui_proc_home_t *proc)
{
    home_ui_nav_frame_t *frame;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->ui_nav.restoring) {
        return;
    }
    if (proc->current_ui_type == HOME_UI_TYPE_NONE) {
        return;
    }
    if (proc->ui_nav.depth >= HOME_UI_NAV_DEPTH_MAX) {
        memmove(&proc->ui_nav.stack[0], &proc->ui_nav.stack[1],
            sizeof(proc->ui_nav.stack[0]) * (HOME_UI_NAV_DEPTH_MAX - 1));
        proc->ui_nav.depth = HOME_UI_NAV_DEPTH_MAX - 1;
    }
    frame = &proc->ui_nav.stack[proc->ui_nav.depth++];
    frame->ui_type = proc->current_ui_type;
    frame->page_index = proc->ui_mode_page_index;
    frame->level_side = proc->level_adjust_side;
#if HOME_MODE_SELECT_IDLE_EXIT_ENABLE
    /// 离开首页进子页时暂停倒计时，返回后重新计时
    proc->mode_select_idle_time = 0;
#endif
    LOG_WM("home ui_nav push type:%d page:%d depth:%d",
        frame->ui_type, frame->page_index, proc->ui_nav.depth);
}

void home_ui_nav_clear(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    memset(&proc->ui_nav, 0, sizeof(proc->ui_nav));
}

void home_adjust_idle_touch(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != HOME_UI_TYPE_LEVEL_ADJUST &&
        proc->current_ui_type != HOME_UI_TYPE_BOTH_LEVEL_ADJUST &&
        proc->current_ui_type != HOME_UI_TYPE_SPEED_ADJUST) {
        return;
    }
    proc->level_adjust_time = get_timestamp();
}

void home_ui_nav_back(ui_proc_home_t *proc, home_ui_nav_back_e policy)
{
    home_ui_nav_frame_t frame = {0};

    EXCEPTION_CHECK_VOID(proc);
    if (proc->ui_nav.depth > 0) {
        frame = proc->ui_nav.stack[--proc->ui_nav.depth];
    }

    LOG_WM("home ui_nav back policy:%d prev_type:%d page:%d depth:%d",
        policy, frame.ui_type, frame.page_index, proc->ui_nav.depth);

    switch (policy) {
        case HOME_UI_NAV_BACK_HISTORY:
            if (frame.ui_type != HOME_UI_TYPE_NONE) {
                home_ui_nav_restore_frame(proc, &frame);
            } else {
                start_home_ui_animation(proc);
            }
            break;
        case HOME_UI_NAV_BACK_HOME_KEEP_PAGE:
            if (frame.ui_type != HOME_UI_TYPE_NONE) {
                proc->ui_mode_page_index = frame.page_index;
            }
            home_ui_nav_clear(proc);
            proc->ui_nav.restoring = 1;
            start_home_ui_animation(proc);
            proc->ui_nav.restoring = 0;
            break;
        case HOME_UI_NAV_BACK_HOME_RESET_PAGE:
        default:
            proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
            home_ui_nav_clear(proc);
            proc->ui_nav.restoring = 1;
            start_home_ui_animation(proc);
            proc->ui_nav.restoring = 0;
            break;
    }
}

void stop_home_common_ui(ui_proc_home_t *proc)
{
    home_pump_result_ui_exit(proc);
    home_settings_success_ui_exit(proc);
    home_power_off_hold_ui_exit(proc);
    home_level_adjust_ui_exit(proc);
    home_both_level_adjust_ui_exit(proc);
    home_speed_adjust_ui_exit(proc);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
    home_ui_nav_clear(proc);
}

void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}

#if IS_SUPPORT_LOW_POWER_FORBID_START_TIP
static uint8_t home_battery_state_is_forbid_start(ui_proc_home_t *proc)
{
    if (proc == NULL || proc->battery == NULL || proc->battery->get_battery_state == NULL) {
        return 0;
    }
    return (proc->battery->get_battery_state() == BATTERY_STATE_FORBID_START) ? 1 : 0;
}

void home_forbid_start_tip_sync(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (!home_battery_state_is_forbid_start(proc)) {
        if (proc->popup_ui_type == ID_POPUP_UI_LOW_POWER_START_TIP) {
            home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        }
        return;
    }
    if (proc->current_ui_type == HOME_UI_TYPE_POWER_OFF_HOLD) {
        return;
    }
    if (proc->current_ui_type != HOME_UI_TYPE_HOME) {
        start_home_ui_animation(proc);
        return;
    }
    home_popup_ui_type_update(proc, ID_POPUP_UI_LOW_POWER_START_TIP);
}

uint8_t home_forbid_start_key_is_blocked(ui_proc_home_t *proc, uint16_t keycode, uint16_t keytype)
{
    if (!home_battery_state_is_forbid_start(proc)) {
        return 0;
    }
    if (proc->current_ui_type == HOME_UI_TYPE_POWER_OFF_HOLD) {
        return 0;
    }
    if (keytype == KEY_EVENT_LONG_2S &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        return 0;
    }
    if (keytype == KEY_EVENT_UP &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        return 0;
    }
    return 1;
}
#endif


void home_ui_change_hdr_remove(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case HOME_UI_TYPE_PUMP_RESULT:
            home_pump_result_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SETTINGS_SUCCESS:
            home_settings_success_ui_exit(proc);
            break;
        case HOME_UI_TYPE_POWER_OFF_HOLD:
            home_power_off_hold_ui_exit(proc);
            break;
        case HOME_UI_TYPE_LEVEL_ADJUST:
            home_level_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_BOTH_LEVEL_ADJUST:
            home_both_level_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SPEED_ADJUST:
            home_speed_adjust_ui_exit(proc);
            break;
        default:
            break;
    }
}

uint8_t home_sub_mode_type_get(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->motor == NULL || proc->motor->get_runtime_mode_type == NULL) {
        return PUMP_MODE_LACTATION;
    }
    return proc->motor->get_runtime_mode_type(false);
}

void home_sub_mode_switch_clear(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->sub_mode_switch = 0;
}

void home_sub_mode_switch_tick(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->sub_mode_switch == 0) {
        return;
    }
    if (get_timestamp_ms(proc->sub_mode_switch_time) < HOME_SUB_MODE_SWITCH_TIMEOUT_MS) {
        return;
    }
    home_sub_mode_switch_clear(proc);
}

void home_sub_mode_state_init(ui_proc_home_t *proc, uint8_t clamp_level)
{
    int pump_mode = 0;
    int time_index = 0;

    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
    proc->sub_mode_time_index = time_index;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    proc->sub_mode_type = home_level_adjust_mode_type_resolve((uint8_t)pump_mode,
        home_sub_mode_type_get(proc));
    proc->sub_mode_switch = 0;
    READ_PUMP_SETTING_INFO(proc->pump, proc->sub_mode_time_index, proc->setting);
    if (clamp_level) {
        home_sub_mode_level_clamp(proc, proc->sub_mode_type);
    }
}

void home_sub_mode_sync_tick(ui_proc_home_t *proc, home_sub_mode_sync_e sync)
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
        home_sub_mode_type_get(proc));

    if (proc->sub_mode_time_index != time_index) {
        if (proc->sub_mode_type != 0xFF) {
            switched = 1;
            LOG_WM("home sub mode time %d->%d sync=%d",
                proc->sub_mode_time_index, time_index, sync);
        }
        proc->sub_mode_time_index = time_index;
    }

    if (proc->sub_mode_type != mode_type) {
        if (proc->sub_mode_type != 0xFF) {
            switched = 1;
            proc->sub_mode_switch = 1;
            proc->sub_mode_switch_time = get_timestamp();
            LOG_WM("home sub mode type %d->%d sync=%d",
                proc->sub_mode_type, mode_type, sync);
        }
        proc->sub_mode_type = mode_type;
    }

    if (sync == HOME_SUB_MODE_SYNC_LEVEL || sync == HOME_SUB_MODE_SYNC_BOTH_LEVEL) {
        home_sub_mode_level_clamp(proc, mode_type);
    } else if (sync == HOME_SUB_MODE_SYNC_SPEED) {
        uint8_t old_speed = proc->setting.speed;

        /* 与力度页一致：每帧回读，APP 改速度后调节页才能刷新 */
        READ_PUMP_SETTING_INFO(proc->pump, proc->sub_mode_time_index, proc->setting);
        if (proc->setting.speed >= BREAST_PUMP_SPEED_MAX) {
            proc->setting.speed = BREAST_PUMP_SPEED_MAX - 1;
        }
        if (old_speed != proc->setting.speed) {
            proc->level_adjust_time = get_timestamp();
        }
    }

    if (switched) {
        if (sync == HOME_SUB_MODE_SYNC_LEVEL) {
            if (proc->level_over_limit) {
                proc->level_over_limit = 0;
                proc->level_arc_level = proc->level_value;
            }
        } else if (sync == HOME_SUB_MODE_SYNC_BOTH_LEVEL) {
            if (proc->both_level_over_limit) {
                proc->both_level_over_limit = 0;
                proc->both_level_arc[0] = proc->both_level_value[0];
                proc->both_level_arc[1] = proc->both_level_value[1];
            }
        }
    }

    if (sync == HOME_SUB_MODE_SYNC_LEVEL) {
        index = proc->level_adjust_side;
        if (index < BREAST_PUMP_NUM && proc->level_over_limit == 0) {
            proc->level_value = proc->setting.level[index];
            proc->level_arc_level = proc->level_value;
        }
    } else if (sync == HOME_SUB_MODE_SYNC_BOTH_LEVEL) {
        if (proc->both_level_over_limit == 0) {
            for (i = 0; i < BREAST_PUMP_NUM; i++) {
                proc->both_level_value[i] = proc->setting.level[i];
                proc->both_level_arc[i] = proc->both_level_value[i];
            }
        }
    }
}

int home_start_pump_handler(ui_proc_home_t *proc)
{
    return home_start_pump_with_sub_ui(proc, PUMP_SUB_UI_TYPE_NONE);
}

int home_start_pump_with_sub_ui(ui_proc_home_t *proc, pump_sub_ui_type_e sub_ui)
{
    uint8_t battery_state = BATTERY_STATE_NONE;

    EXCEPTION_CHECK_INT(proc);
    if (proc->battery) {
        battery_state = proc->battery->get_battery_state();
    }

    if (battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        battery_state == BATTERY_STATE_EMPTY ||
        battery_state == BATTERY_STATE_FORBID_START) {
        LOG_WM("battery state is low, ignore key event.");
        return -1;
    }

    if (proc->wm) {
        multi_func_msg_data_t msg = {0};
        LOG_WM("goto pump window, sub_ui:%d.", sub_ui);
        msg.union_body.extra.window_id = ID_WINDOW_PUMP;
        msg.union_body.extra.arg1 = (int)sub_ui;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return 0;
    }
    return -1;
}
