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
            /// 长按 3s 圆弧闭环，跳转 home 泵奶完成页
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END, DATA_BURY_END_REASON_MANUAL, 0);
            pump_complete_event_handler(proc);
            break;
        case ID_POPUP_UI_PUMP_RUNING_30M_TIP:
            LOG_WM("pump running 30m tip popup dismissed.");
            break;
        case ID_POPUP_UI_PUMP_REST_MODE_ADJUST_TIP:
            LOG_WM("pump rest mode adjust tip popup dismissed.");
            break;
        default:
            break;
    }
}

static void pumping_work_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        pump_mode_page_index_clamp(proc);
        pump_mode_select_idle_timeout_handler(proc);
    }
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
    if (pump_pause_idle_timeout_handler(proc)) {
        return;
    }
#endif
    ui_display_pump_home_screen_process(ui);
    pump_pause_led_tick(proc);
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
    /* 不在此清 ui_mode_page_index：力度/速度调节退出需回到进入前的模式页 */
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        return;
    } else {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        pump_ui_change_hdr_remove(proc);
    }
 
    int res_theme_type = RES_THEME_TYPE_BASE;
    if (ui->priv.res) {
        res_theme_type = ui->priv.res->get_sys_theme_type();
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
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_PAUSE || pump_status == BREAST_PUMP_IDLE || pump_status == BREAST_PUMP_FINISH) {
        int next_status = (pump_status == BREAST_PUMP_PAUSE) ? BREAST_PUMP_CONTINUE : BREAST_PUMP_START;
        /// 如果是开机首次启动或者结算后的首次启动，则触发泵奶启动埋点事件
        if (BREAST_PUMP_START == next_status) {
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_START, 0, 0);
        }
        
        /// 启动时，点亮启动灯
        if (proc->led && proc->led->set_led_level) {
            proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        }

        /// 状态变化 触发泵奶状态切换埋点事件
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
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
        pump_pause_idle_stop(proc);
#endif
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        LOG_UI("pump start: %d, page index: %d", pump_status, proc->ui_mode_page_index);
        /* 首页/力度/速度调节页：start 不改 page_index，也不回退导航栈 */
        if (proc->current_ui_type != PUMP_UI_TYPE_HOME &&
            proc->current_ui_type != PUMP_UI_TYPE_LEVEL_ADJUST &&
            proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST &&
            proc->current_ui_type != PUMP_UI_TYPE_SPEED_ADJUST) {
            pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_KEEP_PAGE);
        }
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
        LOG_UI("pump pause: %d, page index: %d", pump_status, proc->ui_mode_page_index);
        /* 首页/力度/速度调节页：pause 不改 page_index，也不回退导航栈 */
        if (proc->current_ui_type != PUMP_UI_TYPE_HOME &&
            proc->current_ui_type != PUMP_UI_TYPE_LEVEL_ADJUST &&
            proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST &&
            proc->current_ui_type != PUMP_UI_TYPE_SPEED_ADJUST) {
            pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_KEEP_PAGE);
        }
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
        pump_pause_idle_start(proc);
#endif
    }
}