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
static void breastfeeding_mode_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    LOG_WM("===> popup %d ui display complete.", popup_ui_index);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    pump_popup_ui_type_update(proc, PUMP_POPUP_UI_NONE);
}

static void breastfeeding_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->wm);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        int is_pump_working = 0; 
        READ_PUMP_IS_WORKING(proc->pump, is_pump_working);
        if (is_pump_working == 0) {
            multi_func_msg_data_t msg = {0};
            LOG_WM("goto home window.");
            msg.union_body.extra.window_id = ID_WINDOW_HOME;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
        } else {
            if (proc->mode_sync_flag) {
                proc->mode_sync_flag = false;
                WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_breastfeeding_mode_index, true, 1);
                proc->home_popup_ui_type = PUMP_POPUP_UI_OPERATION_SUCCESS;
            }
            show_pump_home_ui(proc);
        }
    }
}

static void breastfeeding_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_breastfeeding_mode_process(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_MODE_DISPLAY_TIMEOUT) {
        proc->last_progress_ani_time = get_timestamp();
        POST_TASK_HANDLER_NO_DELAY(breastfeeding_mode_display_completed_cb, proc, BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
    } else {
        if (proc->goto_pump_home_ui) {
            proc->goto_pump_home_ui = false;
            POST_TASK_HANDLER_NO_DELAY(breastfeeding_mode_display_completed_cb, proc, BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
        }
    }
}

static void breastfeeding_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
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
    if (proc->popup_ui_type == PUMP_POPUP_UI_OPERATION_SUCCESS ||
        proc->popup_ui_type == PUMP_POPUP_UI_OPERATION_FAILED ||
        proc->popup_ui_type == PUMP_POPUP_UI_LEVEL_ADJUST_TOO_LARGE) {
        pump_popup_ui_type_update(proc, PUMP_POPUP_UI_NONE);
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        return;
    } else {
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
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_BREASTFEEDING_MODE;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_breastfeeding_mode_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    if (PUMP_NO_FINISH(pump_status) && PUMP_IS_BREASTFEEDING_MODE(pump_mode)) {
        LOG_UI("pump is going %d, ignore switch mode.", pump_status, pump_mode); /// 泵奶进行中不支持切换追奶模式(产品需求)
        return;
    }

    uint8_t mode = proc->ui_breastfeeding_mode_index;
    int activate_status = 0;
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    uint8_t mode_upper_boundary = activate_status > 0 ? BREAST_PUMP_CUSTOM_MODE : BREAST_PUMP_EMPTYING_BOOST_MODE;
    if (is_increase) {
        if (mode < mode_upper_boundary) {
            mode++;
        } else {
            mode = BREAST_PUMP_REGULAR_PUMPING_MODE;
        }
    } else {
        if (mode > BREAST_PUMP_REGULAR_PUMPING_MODE) {
            mode--;
        } else {
            mode = mode_upper_boundary;
        }
    }
    proc->mode_sync_flag = true;
    proc->ui_breastfeeding_mode_index = mode;
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, mode);
    show_breastfeeding_mode_ui(proc);
}

void breastfeeding_mode_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
}

void breastfeeding_mode_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->led);
    int mode_index = 0;
    READ_PUMP_EXPERT_MODE(proc->pump, false, mode_index);
    proc->ui_breastfeeding_mode_index = mode_index;
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status == BREAST_PUMP_PAUSE) {
        pump_popup_ui_type_update(proc, PUMP_POPUP_UI_OPERATION_FAILED);
        LOG_WM("pump is pause, ignore.");
        return;
    }

    if (PUMP_IS_BREASTFEEDING_MODE(pump_mode) && (pump_status == BREAST_PUMP_START || pump_status == BREAST_PUMP_CONTINUE)) {
        pump_popup_ui_type_update(proc, PUMP_POPUP_UI_OPERATION_FAILED);
        LOG_WM("mode %d/%d/%d is working, ignore.", pump_mode, mode_index, pump_status);
        return;
    }
    proc->mode_sync_flag = true;
    show_breastfeeding_mode_ui(proc);   
}