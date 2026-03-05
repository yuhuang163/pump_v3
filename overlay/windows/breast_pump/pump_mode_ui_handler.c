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
#define PUMP_MODE_DISPLAY_TASK_NAME                     "pump_mode_display_cmp"

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
static void pump_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->wm);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
        uint8_t do_work = proc->pump->is_on_working();
        if (do_work == 0) {
            multi_func_msg_data_t msg = {0};
            LOG_WM("goto home window.");
            msg.union_body.extra.window_id = ID_WINDOW_HOME;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
        } else {
            if (proc->ui_common_mode_index != proc->pump->get_breast_pump_mode()) {
                proc->popup_ui_type = PUMP_POPUP_UI_MODE_PROMPT;
            }
            show_pump_home_ui(proc);
        }
    }
}

static void pump_mode_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pump_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_pump_mode_process(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    uint8_t do_work = proc->pump->is_on_working();
    if (do_work == 0) {
        if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_MODE_DISPLAY_TIMEOUT) {
            LOG_UI("pump mode ui timeout: %d", proc->last_progress_ani_time);
            POST_TASK_HANDLER_NO_DELAY(pump_mode_display_completed_cb, proc, PUMP_MODE_DISPLAY_TASK_NAME);
        }
    } else {
        if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMPING_MODE_DISPLAY_TIMEOUT) {
            LOG_UI("pumping mode ui timeout: %d", proc->last_progress_ani_time);
            POST_TASK_HANDLER_NO_DELAY(pump_mode_display_completed_cb, proc, PUMP_MODE_DISPLAY_TASK_NAME);
        }
    }
}

static void pump_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            pump_mode_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_mode_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_mode_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_mode_prepare(ui);
}

static void pump_mode_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_mode_remove(ui);
}

static void show_pump_mode_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
        return;
    } else {
        pump_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "pump_mode_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_mode_animation_progress_cb,
        .display_prepare_cb = pump_mode_ui_display_prepare,
        .display_remove_cb = pump_mode_ui_display_remove,
        .user_data = proc,
    };
    proc->current_ui_type = PUMP_UI_TYPE_COMMON_MODE;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_pump_common_mode_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    uint8_t mode = proc->ui_common_mode_index;
    if (is_increase) {
        if (mode < BREAST_PUMP_MIX_MODE) {
            mode++;
        } else {
            mode = BREAST_PUMP_STIMULATION_MODE;
        }
    } else {
        if (mode > BREAST_PUMP_STIMULATION_MODE) {
            mode--;
        } else {
            mode = BREAST_PUMP_MIX_MODE;
        }
    }
    proc->ui_common_mode_index = mode;
    LOG_UI("switch %s mode: %d", proc->pump->get_breast_pump_mode_name(mode), mode);
    show_pump_mode_ui(proc);
}

void pump_mode_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_MODE_DISPLAY_TASK_NAME);
}

void pump_mode_key_click_handler(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->led);
    proc->led->set_led_blink(PUMP_MODE_BL, PUMP_BLINK_PERIOD, PUMP_BLINK_TIMES, LED_STATUS_ON);
    int status = proc->pump->get_breast_pump_status();
    int mode = proc->pump->get_breast_pump_mode();
    if (PUMP_NO_FINISH(status) && PUMP_IS_BREASTFEEDING_MODE(mode)) {
        LOG_UI("pump is breastfeeding mode, ignore.");
        return;
    }

    if (proc->current_ui_type != PUMP_UI_TYPE_COMMON_MODE) {
        proc->ui_common_mode_index = proc->pump->get_breast_pump_common_mode();
        show_pump_mode_ui(proc);
    } else {
        if (proc->pump->is_on_working() > 0) {
            POST_TASK_HANDLER_NO_DELAY(pump_mode_display_completed_cb, proc, PUMP_MODE_DISPLAY_TASK_NAME);
        }
    }
}
