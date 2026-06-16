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
#define BREASTFEEDING_MODE_DISPLAY_TASK_NAME       "bf_mode_display_cmp"
#define BREASTFEEDING_MODE_SYNC_TASK_NAME          "bf_mode_page_sync"

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
static void breastfeeding_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type != PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        LOG_UI("ui type error, ignore.");
        return;
    }
    pump_sub_ui_exit_handler(proc);
}

static void breastfeeding_mode_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
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
            proc->forbid_start_pump = true;
            pump_complete_event_handler(proc);
            break;
        case ID_POPUP_UI_OP_SUCCESS:
            POST_WM_TASK_HANDLER_NO_DELAY(breastfeeding_mode_display_completed_cb, 
                proc, BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
            break;
        default:
            break;
    }
}

static void breastfeeding_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_breastfeeding_mode_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (proc->exit_current_ui || time_diff >= PUMP_MODE_DISPLAY_TIMEOUT) {
        proc->exit_current_ui = false;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("mode ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(breastfeeding_mode_display_completed_cb, proc, BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
    }
}

static void breastfeeding_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            breastfeeding_mode_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            breastfeeding_mode_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void breastfeeding_mode_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_breastfeeding_mode_prepare(ui);
}

static void breastfeeding_mode_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_breastfeeding_mode_remove(ui);
}

static void show_breastfeeding_mode_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init); 
    proc->last_progress_ani_time = get_timestamp();  
    if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        return;
    } else {
        pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
        pump_ui_change_hdr_remove(proc);
    }
    
    ui_play_info_t play_info = {
        .tag = "bf_mode_screen",
        .fps = PUMP_MODE_ANIMATION_FREQ,
        .progress_cb = breastfeeding_mode_animation_progress_cb,
        .display_prepare_cb = breastfeeding_mode_ui_display_prepare,
        .display_remove_cb = breastfeeding_mode_ui_display_remove,
        .user_data = proc,
    };
    proc->current_ui_type = PUMP_UI_TYPE_BREASTFEEDING_MODE;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void breastfeeding_mode_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (proc->current_ui_type != PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        int mode_index = 0;
        READ_PUMP_EXPERT_MODE(proc->pump, false, mode_index);
        proc->ui_bf_mode_index = PUMP_IS_BREASTFEEDING_MODE(mode_index) == 0 ? 
            BREAST_PUMP_EXPERT_MODE_DEFAULT : mode_index;
        READ_PUMP_WORK_MODE(proc->pump, mode_index);
        if (proc->ui_bf_mode_index != mode_index) {
            WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_bf_mode_index, true, 1);
            char *mode_name = "unknow";
            READ_PUMP_MODE_NAME(proc->pump, proc->ui_bf_mode_index, &mode_name);
            LOG_UI("switch %s mode: %d", mode_name, proc->ui_bf_mode_index);
        }
    } else {
        uint8_t mode_from = proc->ui_bf_mode_index;
        uint8_t mode = mode_from;
        int activate_status = 0;
        READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
        uint8_t mode_upper_boundary = activate_status > 0 ? 
            BREAST_PUMP_CUSTOM_MODE : (BREAST_PUMP_CUSTOM_MODE - 1); /// 获取韵律模式上限边界
        mode = mode < mode_upper_boundary ? (mode + 1) : BREAST_PUMP_EXPERT_MODE_LOWER;
        if (mode_from != mode) {
            data_bury_trigger_manual_adjust_t bury_adj = {
                .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
                .evt_type = DATA_BURY_ADJUST_PUMP_MODE,
                .adjust_from = mode_from,
                .adjust_to = mode,
            };
            DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST, 0, 0, &bury_adj);
        }
        proc->ui_bf_mode_index = mode;
        WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_bf_mode_index, true, 1);
        char *mode_name = "unknow";
        READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
        LOG_UI("switch %s rhythm: %d", mode_name, mode); /// 切换韵律模式
    }
    proc->exit_current_ui = false;
    proc->forbid_start_pump = false;
    show_breastfeeding_mode_ui(proc);  
}

void breastfeeding_mode_ui_exit(ui_proc_pump_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(BREASTFEEDING_MODE_SYNC_TASK_NAME);
}

