/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_home.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_NORMAL_MODE_DISPLAY_TASK_NAME                     "home_normal_mode_display_cmp"

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
static void home_pump_normal_mode_ui_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (proc->current_ui_type == HOME_UI_TYPE_NORMAL_MODE_SELECT) {
        start_home_ui_animation(proc);
    }
}

static void home_pump_normal_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_home_t *proc = (ui_proc_home_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_HOME);
    ui_display_home_pump_normal_mode_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
#if V3_UX_VERSION == V3_UX_V1
    if (time_diff >= HOME_MODE_SELECT_DISPLAY_TIME) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("normal mode ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_normal_mode_ui_display_completed_cb, 
            proc, HOME_NORMAL_MODE_DISPLAY_TASK_NAME);
    }
#else
    if (proc->exit_current_ui || time_diff >= HOME_MODE_SELECT_DISPLAY_TIME) {
        proc->exit_current_ui = false;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("normal mode ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(home_pump_normal_mode_ui_display_completed_cb, 
            proc, HOME_NORMAL_MODE_DISPLAY_TASK_NAME);
    }
#endif
}

static void home_pump_normal_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            home_pump_normal_mode_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void home_pump_normal_mode_ui_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_pump_normal_mode_prepare(ui);
}

static void home_pump_normal_mode_ui_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_home_pump_normal_mode_remove(ui);
}

static void show_home_pump_normal_mode_ui(ui_proc_home_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    home_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == HOME_UI_TYPE_NORMAL_MODE_SELECT) {
        return;
    } else {
        home_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "home_pump_normal_mode_screen",
        .fps = HOME_PUMP_MODE_ANIMATION_FREQ,
        .progress_cb = home_pump_normal_mode_animation_progress_cb,
        .display_prepare_cb = home_pump_normal_mode_ui_prepare,
        .display_remove_cb = home_pump_normal_mode_ui_remove,
        .user_data = proc,
    };
    proc->current_ui_type = HOME_UI_TYPE_NORMAL_MODE_SELECT;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if V3_UX_VERSION == V3_UX_V1
void switch_home_pump_normal_mode(ui_proc_home_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    uint8_t mode = proc->ui_normal_mode_index;
    if (is_increase) {
        if (mode < BREAST_PUMP_NORMAL_MODE_UPPER) {
            mode++;
        } else {
            mode = BREAST_PUMP_NORMAL_MODE_LOWER;
        }
    } else {
        if (mode > BREAST_PUMP_NORMAL_MODE_LOWER) {
            mode--;
        } else {
            mode = BREAST_PUMP_NORMAL_MODE_UPPER;
        }
    }
    proc->ui_normal_mode_index = mode;
    char *mode_name = "unknow";
    READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
    LOG_UI("switch %s mode: %d", mode_name, mode);
    show_home_pump_normal_mode_ui(proc);
}

void home_pump_normal_mode_key_click_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (proc->current_ui_type == HOME_UI_TYPE_NORMAL_MODE_SELECT) {
        start_home_ui_animation(proc);
        return;
    } 
    int mode_index = 0;
    READ_PUMP_COMMON_MODE(proc->pump, false, mode_index);
    proc->ui_normal_mode_index = mode_index;
    if (PUMP_IS_COMMON_MODE(mode_index) == 0) {
        proc->ui_normal_mode_index = BREAST_PUMP_NORMAL_MODE_DEFAULT;
    } 
    if (proc->pump_priming == 0) {
        proc->pump_priming = true;
        if (proc->led)  {
            proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
        }
    }
    show_home_pump_normal_mode_ui(proc);
}
#else
void home_pump_normal_mode_key_click_handler(ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (proc->current_ui_type != HOME_UI_TYPE_NORMAL_MODE_SELECT) {
        int mode_index = 0;
        READ_PUMP_COMMON_MODE(proc->pump, false, mode_index);
        proc->ui_normal_mode_index = PUMP_IS_COMMON_MODE(mode_index) == 0 ?
                BREAST_PUMP_NORMAL_MODE_DEFAULT : mode_index;
        READ_PUMP_WORK_MODE(proc->pump, mode_index);
        if (proc->ui_normal_mode_index != mode_index) {
            WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_normal_mode_index, true, 1);
            char *mode_name = "unknow";
            READ_PUMP_MODE_NAME(proc->pump, proc->ui_normal_mode_index, &mode_name);
            LOG_UI("switch %s mode: %d", mode_name, proc->ui_normal_mode_index);
        }
        if (proc->pump_priming == 0) {
            proc->pump_priming = true;
            if (proc->led)  {
                proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
            }
        }
    } else {
        uint8_t mode = proc->ui_normal_mode_index;
        mode = mode < BREAST_PUMP_NORMAL_MODE_UPPER ? (mode + 1) : BREAST_PUMP_NORMAL_MODE_LOWER;
        proc->ui_normal_mode_index = mode;
        WRITE_PUMP_WORK_MODE(proc->pump, proc->ui_normal_mode_index, true, 1);
        char *mode_name = "unknow";
        READ_PUMP_MODE_NAME(proc->pump, mode, &mode_name);
        LOG_UI("switch %s mode: %d", mode_name, mode);
    }
    proc->exit_current_ui = false;
    show_home_pump_normal_mode_ui(proc);
}
#endif

void home_pump_normal_mode_ui_exit(ui_proc_home_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_NORMAL_MODE_DISPLAY_TASK_NAME);
}