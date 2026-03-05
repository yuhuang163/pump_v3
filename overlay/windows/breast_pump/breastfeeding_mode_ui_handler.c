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
static void breastfeeding_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->wm);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        uint8_t do_work = proc->pump->is_on_working();
        if (do_work == 0) {
            multi_func_msg_data_t msg = {0};
            LOG_WM("goto home window.");
            msg.union_body.extra.window_id = ID_WINDOW_HOME;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
        } else {
            show_pump_home_ui(proc);
        }
    }
}

static void breastfeeding_mode_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void breastfeeding_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_breastfeeding_mode_process(ui);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_MODE_DISPLAY_TIMEOUT) {
        POST_TASK_HANDLER_NO_DELAY(breastfeeding_mode_display_completed_cb, proc, BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
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
            breastfeeding_mode_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            breastfeeding_mode_ui_after_policy(ui, index);
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
    int status = proc->pump->get_breast_pump_status();
    if (PUMP_NO_FINISH(status)) {
        LOG_UI("pump is going %d, ignore switch mode.", status); /// 泵奶进行中不支持切换追奶模式(产品需求)
        return;
    }

    uint8_t mode = proc->ui_breastfeeding_mode_index;
    if (is_increase) {
        if (mode < BREAST_PUMP_EMPTYING_BOOST_MODE) {
            mode++;
        } else {
            mode = BREAST_PUMP_CUSTOM_MODE;
        }
    } else {
        if (mode > BREAST_PUMP_CUSTOM_MODE) {
            mode--;
        } else {
            mode = BREAST_PUMP_EMPTYING_BOOST_MODE;
        }
    }
    proc->ui_breastfeeding_mode_index = mode;
    proc->pump->set_breast_pump_mode(mode, true);
    LOG_UI("switch %s mode: %d", proc->pump->get_breast_pump_mode_name(mode), mode);
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
    proc->led->set_led_blink(PUMP_BREASTFEEDING_BL, PUMP_BLINK_PERIOD, PUMP_BLINK_TIMES, LED_STATUS_ON);
    int status = proc->pump->get_breast_pump_status();
    if (proc->current_ui_type != PUMP_UI_TYPE_BREASTFEEDING_MODE && PUMP_NO_FINISH(status) == 0) {
        proc->ui_breastfeeding_mode_index = proc->pump->get_breast_pump_breastfeeding_mode();
        show_breastfeeding_mode_ui(proc);
    }
}