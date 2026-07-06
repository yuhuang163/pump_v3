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
static bool pump_ui_blink_update(ui_proc_pump_t *proc)
{
    const uint8_t blink_visible =
        ((get_timestamp() / PUMP_BLINK_PERIOD) % 2U) == 0U ? 1 : 0;
    if (proc->state_blink_visible == blink_visible) {
        return false;
    }
    proc->state_blink_visible = blink_visible;
    return true;
}

static bool pumping_ui_led_needs_update(const ui_proc_pump_t *proc)
{
    const bool is_finish_tip_popup =
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_WILL_FINISH_TIP;

    if (proc->pump_light_status == PUMP_LIGHT_STATUS_PAUSE) {
        if (proc->pump_status != BREAST_PUMP_PAUSE) {
            return true;
        }
        return proc->pump_pause_blink_state != proc->state_blink_visible;
    }
    if (proc->pump_light_status == PUMP_LIGHT_STATUS_FINISH_TIP) {
        if (!is_finish_tip_popup) {
            return true;
        }
        return proc->finish_tip_blink_visible != proc->state_blink_visible;
    }
    if (proc->pump_status == BREAST_PUMP_PAUSE) {
        return true;
    }
    if (is_finish_tip_popup) {
        return true;
    }
    return false;
}

static void pumping_ui_led_update(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->led);
    const bool blink_visible = proc->state_blink_visible != 0;
    const uint8_t popup_type = proc->popup_ui_type;
    const bool is_finish_tip_popup =
        popup_type == ID_POPUP_UI_PUMP_FINISH_MANUAL ||
        popup_type == ID_POPUP_UI_PUMP_WILL_FINISH_TIP;

    if (proc->pump_status == BREAST_PUMP_PAUSE) {
        if (proc->pump_light_status != PUMP_LIGHT_STATUS_PAUSE) {
            proc->pump_light_status = PUMP_LIGHT_STATUS_PAUSE;
        }
        if (proc->pump_pause_blink_state != blink_visible) {
            proc->led->set_led_level(PUMP_START_BL, blink_visible ? LED_LEVEL_MAX : 0);
            proc->pump_pause_blink_state = blink_visible;
        }
        return;
    }

    if (is_finish_tip_popup) {
        if (proc->pump_light_status != PUMP_LIGHT_STATUS_FINISH_TIP) {
            proc->pump_light_status = PUMP_LIGHT_STATUS_FINISH_TIP;
            proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, 0);
            proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, 0);
            proc->led->set_led_level(PUMP_MODE_BL, 0);
            proc->finish_tip_blink_visible = blink_visible;
        } else if (proc->finish_tip_blink_visible != blink_visible) {
            proc->led->set_led_level(PUMP_START_BL, blink_visible ? LED_LEVEL_MAX : 0);
            proc->led->set_led_level(PUMP_FREQ_SET_BL, blink_visible ? LED_LEVEL_MAX : 0);
            proc->finish_tip_blink_visible = blink_visible;
        }
        return;
    }

    if (proc->pump_light_status != PUMP_LIGHT_STATUS_DEFAULT) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        proc->led->set_led_level(PUMP_MODE_BL, LED_LEVEL_MAX);
        proc->led->set_led_level(PUMP_FREQ_SET_BL, LED_LEVEL_MAX);
        proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX);
        proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, LED_LEVEL_MAX);
        proc->pump_light_status = PUMP_LIGHT_STATUS_DEFAULT;
        proc->finish_tip_blink_visible = blink_visible;
        proc->pump_pause_blink_state = blink_visible;
    }
}

static void pumping_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    switch (popup_ui_index) {
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
            pump_complete_event_handler(proc);
            break;
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
            break;
        case ID_POPUP_UI_PUMP_RUNING_30M_TIP:
        case ID_POPUP_UI_PUMP_WILL_FINISH_TIP:
            break;
        default:
            break;
    }
}

static void pumping_work_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    pump_level_adjust_timeout_handler(proc);
    pump_mode_adjust_timeout_handler(proc);
    const bool blink_changed = pump_ui_blink_update(proc);
    ui_display_pump_home_screen_process(ui);
    if (blink_changed || pumping_ui_led_needs_update(proc)) {
        pumping_ui_led_update(proc);
    }
}

static void pumping_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pumping_work_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            pumping_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pumping_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_home_screen_prepare(ui);
}

static void pumping_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_home_screen_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_home_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    proc->exit_current_ui = false;
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        return;
    } else {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        pump_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "pump_home_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pumping_animation_progress_cb,
        .display_prepare_cb = pumping_ui_display_prepare,
        .display_remove_cb = pumping_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_HOME;
}

void switch_pump_status_ui(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_PAUSE || pump_status == BREAST_PUMP_IDLE || pump_status == BREAST_PUMP_FINISH) {
        int next_status = (pump_status == BREAST_PUMP_PAUSE) ? BREAST_PUMP_CONTINUE : BREAST_PUMP_START;
        ///如果是开机首次启动或者结算后的首次启动，则触发泵奶启动埋点事件
        if (BREAST_PUMP_START == next_status) {
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_START, 0, 0);
        }
        ///状态变化 触发泵奶状态切换埋点事件
        {
            int control_source = 0;
            READ_PUMP_CONTROL_SOURCE(proc->pump, control_source);
            data_bury_trigger_user_evt_t bury_evt = {
                .evt_source = (control_source == true ? DATA_BURY_EVT_SOURCE_REMOTE_APP : DATA_BURY_EVT_SOURCE_LOCAL),
                .evt_type = DATA_BURY_USER_EVT_START,
            };
            DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
            WRITE_PUMP_CONTROL_SOURCE(proc->pump, false);
        }
        WRITE_PUMP_WORK_STATUS(proc->pump, next_status, 1);
        proc->pump_status = next_status;
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        show_pump_home_ui(proc);
        pump_mode_adjust_exit_handler(proc);
        pump_level_adjust_exit_handler(proc);
        LOG_UI("pump start ui create: %d", pump_status);
    } else {
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_PAUSE, 1);
        proc->pump_status = BREAST_PUMP_PAUSE;
        ///状态变化 触发泵奶停止埋点事件
        {
            int control_source = 0;
            READ_PUMP_CONTROL_SOURCE(proc->pump, control_source);
            data_bury_trigger_user_evt_t bury_evt = {
                .evt_source = (control_source == true ? DATA_BURY_EVT_SOURCE_REMOTE_APP : DATA_BURY_EVT_SOURCE_LOCAL),
                .evt_type = DATA_BURY_USER_EVT_PAUSE,
            };
            DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
            WRITE_PUMP_CONTROL_SOURCE(proc->pump, false);
        }
        show_pump_home_ui(proc);
        pump_mode_adjust_exit_handler(proc);
        pump_level_adjust_exit_handler(proc);
        LOG_UI("pump stop ui create: %d", pump_status);
    }
}