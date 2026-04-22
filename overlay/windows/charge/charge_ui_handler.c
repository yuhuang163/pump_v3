/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

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
static void charging_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_charge_screen_process(ui);
}

static void charging_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            charging_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void charging_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_charge_screen_prepare(ui);
}

static void charging_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_charge_screen_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_charging_ui(ui_proc_charge_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == CHARGE_UI_TYPE_CHARGING) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "charge_screen",
        .fps = CHARGE_ANIMATION_FREQ,
        .progress_cb = charging_animation_progress_cb,
        .display_prepare_cb = charging_ui_display_prepare,
        .display_remove_cb = charging_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = CHARGE_UI_TYPE_CHARGING;
}

int switch_charge_night_light_onoff_ui(ui_proc_charge_t *proc)
{
    EXCEPTION_CHECK_INT(proc->led && proc->led->hook);
    if (proc->led->hook->toggle_night_light) {
        proc->led->hook->toggle_night_light(proc->led);
    }
    return 0;
}
