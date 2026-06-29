/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_DISPLAY_TASK_NAME              "home_display_cmp"

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
static bool home_ui_blink_update(ui_proc_home_t *proc)
{
    const uint8_t blink_visible =
        ((get_timestamp() / HOME_BLINK_PERIOD) % 2U) == 0U ? 1 : 0;
    if (proc->state_blink_visible == blink_visible) {
        return false;
    }
    proc->state_blink_visible = blink_visible;
    return true;
}

static bool home_ui_led_needs_update(const ui_proc_home_t *proc)
{
    const bool mode_select = proc->pump_mode_display_type != HOME_PUMP_MODE_DISPLAY_DEFAULT;
    if (mode_select) {
        return proc->mode_select_blink_visible != proc->state_blink_visible;
    }
    return proc->home_light_status != HOME_LIGHT_STATUS_DEFAULT;
}

static void home_ui_led_update(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->led);
    const bool blink_visible = proc->state_blink_visible != 0;
    const bool mode_select = proc->pump_mode_display_type != HOME_PUMP_MODE_DISPLAY_DEFAULT;

    if (mode_select) {
        if (proc->home_light_status != HOME_LIGHT_STATUS_MODE_SELECT) {
            proc->home_light_status = HOME_LIGHT_STATUS_MODE_SELECT;
        }
        if (proc->mode_select_blink_visible != blink_visible) {
            proc->led->set_led_level(PUMP_START_BL, blink_visible ? LED_LEVEL_MAX : 0);
            proc->mode_select_blink_visible = blink_visible;
        }
        return;
    }

    if (proc->home_light_status != HOME_LIGHT_STATUS_DEFAULT) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        proc->home_light_status = HOME_LIGHT_STATUS_DEFAULT;
        proc->mode_select_blink_visible = blink_visible;
    }
}

static void home_popup_ui_finish_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    LOG_WM("===> popup %d ui display complete.", index);
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
}

static void home_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    home_pump_level_adjust_timeout_handler(proc);
    home_pump_mode_adjust_timeout_handler(proc);
    const bool blink_changed = home_ui_blink_update(proc);
    ui_display_home_process(ui);
    if (blink_changed || home_ui_led_needs_update(proc)) {
        home_ui_led_update(proc);
    }
}

static void home_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            home_popup_ui_finish_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_prepare(ui);
}

static void home_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc)
{    
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);  
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    proc->exit_current_ui = false;
    if (proc->current_ui_type == HOME_UI_TYPE_HOME) {
        return;
    }
    
    if (proc->pump_priming) {
        proc->pump_priming = false;
    }

    if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
        proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
    }
    
    ui_play_info_t play_info = {
        .tag = "home_default_screen",
        .fps = HOME_ANIMATION_FREQ,
        .progress_cb = home_animation_progress_cb,
        .display_prepare_cb = home_ui_display_prepare,
        .display_remove_cb = home_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = HOME_UI_TYPE_HOME;
}