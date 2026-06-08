/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_power_off.h" // IWYU pragma: keep

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
static void charge_adapter_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_power_off_t *proc = (ui_proc_power_off_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_POWER_OFF);
    ui_display_charge_adapter_process(ui);
}

static void charge_adapter_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            charge_adapter_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void charge_adapter_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_charge_adapter_screen_prepare(ui);
}

static void charge_adapter_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_charge_adapter_screen_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_power_off_charge_error_ui(ui_proc_power_off_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == POWER_OFF_UI_TYPE_CHARGE_ERROR) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "power_off_charge_error_screen",
        .fps = POWER_OFF_ANIMATION_FREQ,
        .progress_cb = charge_adapter_progress_cb,
        .display_prepare_cb = charge_adapter_ui_display_prepare,
        .display_remove_cb = charge_adapter_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = POWER_OFF_UI_TYPE_CHARGE_ERROR;
}
