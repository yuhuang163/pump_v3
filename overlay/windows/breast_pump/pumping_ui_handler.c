/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_log_define.h"
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
static void pumping_work_ui_popup_ui_update(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    switch (popup_ui_index) {
        case PUMP_POPUP_UI_MODE_PROMPT:
            if (proc->pump->is_on_working()) {
                proc->pump->set_breast_pump_mode(proc->ui_common_mode_index, true); /// 更改泵奶模块模式
            }
            break;
        case PUMP_POPUP_UI_PUMP_WILL_FINISH_TIP:
            pump_complete_event_handler(proc);
            break;
        case PUMP_POPUP_UI_PUMP_PAUSE_PROMPT:
            break;
    }
    proc->popup_ui_type = PUMP_POPUP_UI_NONE;
}

static void pumping_work_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pumping_work_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_pump_home_screen_process(ui);
    ui->priv.play_info.reset_play_count_cb(&ui->priv.play_info);
}

static void pumping_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            pumping_work_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            pumping_work_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            pumping_work_ui_popup_ui_update(ui, index);
            break;
        default:
            break;
    }
}

static void pumping_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
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
    if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    } else {
        pump_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "home_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pumping_animation_progress_cb,
        .display_prepare_cb = pumping_ui_display_prepare,
        .display_remove_cb = pumping_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_HOME;
}

void switch_pump_status_ui(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->led);
    proc->led->set_led_blink(PUMP_START_BL, PUMP_BLINK_PERIOD, 
        PUMP_BLINK_TIMES, LED_STATUS_ON); /// 按键灯闪烁
    int status = proc->pump->get_breast_pump_status();
    if (status == BREAST_PUMP_PAUSE || status == BREAST_PUMP_IDLE || status == BREAST_PUMP_FINISH) {
        if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
            proc->pump->set_breast_pump_mode(proc->ui_common_mode_index, true);
        } else if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
            proc->pump->set_breast_pump_mode(proc->ui_breastfeeding_mode_index, true);
       
        }
        uint8_t mode = proc->pump->get_breast_pump_mode();
        if (PUMP_IS_COMMON_MODE(mode)) {
            proc->led->set_led_level(PUMP_FREQ_SET_BL, 100);
        } else {
            // proc->led->set_led_level(PUMP_FREQ_SET_BL, 0);
        }
        proc->pump->set_breast_pump_status(status == BREAST_PUMP_PAUSE ? \
            BREAST_PUMP_CONTINUE : BREAST_PUMP_START);
        proc->popup_ui_type = PUMP_POPUP_UI_NONE;
        show_pump_home_ui(proc);
        LOG_UI("pump start ui create: %d", status);
    } else {
        proc->pump->set_breast_pump_status(BREAST_PUMP_PAUSE);
        show_pump_home_ui(proc);
        proc->led->set_led_level(PUMP_FREQ_SET_BL, 100);
        proc->popup_ui_type = PUMP_POPUP_UI_PUMP_PAUSE_PROMPT;
        LOG_UI("pump stop ui create: %d", status);
    }
}