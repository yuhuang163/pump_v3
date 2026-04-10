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
static void pumping_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    switch (popup_ui_index) {
        case PUMP_POPUP_UI_PUMP_FINISH_TIMEOUT:
        case PUMP_POPUP_UI_PUMP_FINISH_MANUAL:
            pump_complete_event_handler(proc);
            break;
        case PUMP_POPUP_UI_PUMP_PAUSE_TIMEOUT:
        case PUMP_POPUP_UI_PUMP_PAUSE_MANUAL:
            break;
        case PUMP_POPUP_UI_OPERATION_SUCCESS:
        case PUMP_POPUP_UI_OPERATION_FAILED: {
            int pump_status = 0;
            READ_PUMP_WORK_STATUS(proc->pump, pump_status);
            LOG_UI("pump status: %d", pump_status);
            if (pump_status == BREAST_PUMP_PAUSE) {
                pump_popup_ui_type_update(proc, PUMP_POPUP_UI_PUMP_PAUSE_MANUAL);
                return;
            }
            break;
        }
    }
    pump_popup_ui_type_update(proc, PUMP_POPUP_UI_NONE);
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
    if (proc->home_popup_ui_type == PUMP_POPUP_UI_OPERATION_SUCCESS) {
        pump_popup_ui_type_update(proc, proc->home_popup_ui_type);
        proc->home_popup_ui_type = PUMP_POPUP_UI_NONE;
    }
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
    proc->goto_pump_home_ui= false;
    if (proc->popup_ui_type == PUMP_POPUP_UI_OPERATION_FAILED ||
        proc->popup_ui_type == PUMP_POPUP_UI_LEVEL_ADJUST_TOO_LARGE) {
        pump_popup_ui_type_update(proc, PUMP_POPUP_UI_NONE);
    }
    
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
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_PAUSE || pump_status == BREAST_PUMP_IDLE || pump_status == BREAST_PUMP_FINISH) {
        if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
            WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_common_mode_index, true, 1);
        } else if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
            WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_breastfeeding_mode_index, true, 1);
        }
        int next_status = (pump_status == BREAST_PUMP_PAUSE) ? BREAST_PUMP_CONTINUE : BREAST_PUMP_START;    
        WRITE_PUMP_WORK_STATUS(proc->pump, next_status, 1);
        if (proc->led)  {
            proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        }
        pump_popup_ui_type_update(proc, PUMP_POPUP_UI_NONE);
        show_pump_home_ui(proc);
        LOG_UI("pump start ui create: %d", pump_status);
    } else {
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_PAUSE, 1);
        if (proc->led)  {
            proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
        }
        show_pump_home_ui(proc);
        pump_popup_ui_type_update(proc, PUMP_POPUP_UI_PUMP_PAUSE_MANUAL);
        LOG_UI("pump stop ui create: %d", pump_status);
    }
}