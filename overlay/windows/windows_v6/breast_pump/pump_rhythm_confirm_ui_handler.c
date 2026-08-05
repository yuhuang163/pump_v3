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
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_rhythm_confirm_ui_after_policy(ui_manager_context_t *ui, int index)
{
    (void)index;
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_rhythm_confirm_process(ui);
}

static void pump_rhythm_confirm_animation_progress_cb(void *user_data, int index,
    ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_rhythm_confirm_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_rhythm_confirm_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_rhythm_confirm_prepare(ui);
}

static void pump_rhythm_confirm_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_rhythm_confirm_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_rhythm_confirm_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_RHYTHM_CONFIRM) {
        return;
    }
    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);

    ui_play_info_t play_info = {
        .tag = "pump_rhythm_confirm_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_rhythm_confirm_animation_progress_cb,
        .display_prepare_cb = pump_rhythm_confirm_ui_display_prepare,
        .display_remove_cb = pump_rhythm_confirm_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_RHYTHM_CONFIRM;
#if (PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME == PUMP_RHYTHM_CONFIRM_SCHEME_1)
    {
        int work_mode = 0;

        /// 按当前工作模式区分：模式→韵律 / 韵律→韵律，各自记一次
        READ_PUMP_WORK_MODE(proc->pump, work_mode);
        if (PUMP_IS_BREASTFEEDING_MODE(work_mode)) {
            proc->rhythm_confirm_shown |= PUMP_RHYTHM_CONFIRM_SHOWN_RHYTHM_TO_RHYTHM;
        } else {
            proc->rhythm_confirm_shown |= PUMP_RHYTHM_CONFIRM_SHOWN_MODE_TO_RHYTHM;
        }
    }
#endif
}

void pump_rhythm_confirm_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;
}
