/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_home.h"

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
static void home_ui_nav_restore_frame(ui_proc_home_t *proc, const home_ui_nav_frame_t *frame)
{
    EXCEPTION_CHECK_VOID(proc && frame);
    proc->ui_mode_page_index = frame->page_index;
    proc->ui_nav.restoring = 1;
    switch (frame->ui_type) {
        case HOME_UI_TYPE_LEVEL_ADJUST:
            show_home_level_adjust_ui(proc,
                frame->level_side == HOME_LEVEL_ADJUST_SIDE_LEFT);
            break;
        case HOME_UI_TYPE_BOTH_LEVEL_ADJUST:
            show_home_both_level_adjust_ui(proc);
            break;
        case HOME_UI_TYPE_SPEED_ADJUST:
            show_home_speed_adjust_ui(proc);
            break;
        case HOME_UI_TYPE_HOME:
        default:
            start_home_ui_animation(proc);
            break;
    }
    proc->ui_nav.restoring = 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_ui_nav_push(ui_proc_home_t *proc)
{
    home_ui_nav_frame_t *frame;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->ui_nav.restoring) {
        return;
    }
    if (proc->current_ui_type == HOME_UI_TYPE_NONE) {
        return;
    }
    if (proc->ui_nav.depth >= HOME_UI_NAV_DEPTH_MAX) {
        memmove(&proc->ui_nav.stack[0], &proc->ui_nav.stack[1],
            sizeof(proc->ui_nav.stack[0]) * (HOME_UI_NAV_DEPTH_MAX - 1));
        proc->ui_nav.depth = HOME_UI_NAV_DEPTH_MAX - 1;
    }
    frame = &proc->ui_nav.stack[proc->ui_nav.depth++];
    frame->ui_type = proc->current_ui_type;
    frame->page_index = proc->ui_mode_page_index;
    frame->level_side = proc->level_adjust_side;
    LOG_WM("home ui_nav push type:%d page:%d depth:%d",
        frame->ui_type, frame->page_index, proc->ui_nav.depth);
}

void home_ui_nav_clear(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    memset(&proc->ui_nav, 0, sizeof(proc->ui_nav));
}

void home_ui_nav_back(ui_proc_home_t *proc, home_ui_nav_back_e policy)
{
    home_ui_nav_frame_t frame = {0};

    EXCEPTION_CHECK_VOID(proc);
    if (proc->ui_nav.depth > 0) {
        frame = proc->ui_nav.stack[--proc->ui_nav.depth];
    }

    LOG_WM("home ui_nav back policy:%d prev_type:%d page:%d depth:%d",
        policy, frame.ui_type, frame.page_index, proc->ui_nav.depth);

    switch (policy) {
        case HOME_UI_NAV_BACK_HISTORY:
            if (frame.ui_type != HOME_UI_TYPE_NONE) {
                home_ui_nav_restore_frame(proc, &frame);
            } else {
                start_home_ui_animation(proc);
            }
            break;
        case HOME_UI_NAV_BACK_HOME_KEEP_PAGE:
            if (frame.ui_type != HOME_UI_TYPE_NONE) {
                proc->ui_mode_page_index = frame.page_index;
            }
            home_ui_nav_clear(proc);
            proc->ui_nav.restoring = 1;
            start_home_ui_animation(proc);
            proc->ui_nav.restoring = 0;
            break;
        case HOME_UI_NAV_BACK_HOME_RESET_PAGE:
        default:
            proc->ui_mode_page_index = HOME_MODE_PAGE_INDEX_NONE;
            home_ui_nav_clear(proc);
            proc->ui_nav.restoring = 1;
            start_home_ui_animation(proc);
            proc->ui_nav.restoring = 0;
            break;
    }
}

void stop_home_common_ui(ui_proc_home_t *proc)
{
    home_pump_result_ui_exit(proc);
    home_settings_success_ui_exit(proc);
    home_power_off_hold_ui_exit(proc);
    home_level_adjust_ui_exit(proc);
    home_both_level_adjust_ui_exit(proc);
    home_speed_adjust_ui_exit(proc);
    if (proc->led) {
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
    }
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
    home_ui_nav_clear(proc);
}

void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = type;
}


void home_ui_change_hdr_remove(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    switch (proc->current_ui_type) {
        case HOME_UI_TYPE_PUMP_RESULT:
            home_pump_result_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SETTINGS_SUCCESS:
            home_settings_success_ui_exit(proc);
            break;
        case HOME_UI_TYPE_NIGHT_LIGHT_ADJUST:
            home_night_light_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_POWER_OFF_HOLD:
            home_power_off_hold_ui_exit(proc);
            break;
        case HOME_UI_TYPE_LEVEL_ADJUST:
            home_level_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_BOTH_LEVEL_ADJUST:
            home_both_level_adjust_ui_exit(proc);
            break;
        case HOME_UI_TYPE_SPEED_ADJUST:
            home_speed_adjust_ui_exit(proc);
            break;
        default:
            break;
    }
}
