/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager_api.h"
#include "input_key_code.h"
#include "persist_storage.h"
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_DISPLAY_TASK_NAME              "guide_display_cmp"

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
static void guide_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_guide_t *proc = (ui_proc_guide_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_GUIDE);
    proc->wm->set_window_guidance_deployed_flag(true);
    multi_func_msg_data_t msg = {0};
    LOG_WM("goto home window.");
    if (proc->mode_selected != GUIDE_MODE_SELECT_NONE) {
        if (proc->pump) {
            WRITE_PUMP_WORK_MODE(proc->pump, proc->mode_selected == GUIDE_MODE_SELECT_BREASTFEEDING ? 
                BREAST_PUMP_MILK_BOOST_MODE : BREAST_PUMP_STIMULATION_MODE, true, false);
        }
        msg.union_body.extra.arg1 = proc->mode_selected == GUIDE_MODE_SELECT_BREASTFEEDING ? 
            HOME_SUB_UI_TYPE_PUMP_RHYTHM_ADJUST : HOME_SUB_UI_TYPE_PUMP_MODE_ADJUST;
    } else {
        msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_MODE_UNSELECT;
    }
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void guide_level_adjust_both(ui_proc_guide_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    READ_PUMP_SETTING_INFO(proc->pump, 0, proc->setting);
    if (is_increase) {
        if (proc->setting.level[0] < GUIDE_LEVEL_MAX) {
            proc->setting.level[0]++;
        }
        if (proc->setting.level[1] < GUIDE_LEVEL_MAX) {
            proc->setting.level[1]++;
        }
    } else {
        if (proc->setting.level[0] > GUIDE_LEVEL_MIN) {
            proc->setting.level[0]--;
        }
        if (proc->setting.level[1] > GUIDE_LEVEL_MIN) {
            proc->setting.level[1]--;
        }
    }
    WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
    proc->last_adjust_time = get_timestamp();
}

static void guide_level_adjust_side(ui_proc_guide_t *proc, bool is_left, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    const uint8_t index = is_left ? 0 : 1;
    READ_PUMP_SETTING_INFO(proc->pump, 0, proc->setting);
    if (is_increase) {
        if (proc->setting.level[index] < GUIDE_LEVEL_MAX) {
            proc->setting.level[index]++;
        }
    } else if (proc->setting.level[index] > GUIDE_LEVEL_MIN) {
        proc->setting.level[index]--;
    }
    WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
    /// 力度调节后，展示页面停留200ms，然后进入下一个页面.
    if (get_timestamp_ms(proc->last_adjust_time) > GUIDE_LEVEL_ADJUST_IDLE_SHOW_TIME_MS) {
        proc->last_adjust_time = get_timestamp() - 
            (GUIDE_LEVEL_ADJUST_IDLE_TIMEOUT_MS - GUIDE_LEVEL_ADJUST_IDLE_SHOW_TIME_MS);
    }
}

static bool guide_ui_blink_update(ui_proc_guide_t *proc)
{
    const uint8_t blink_visible =
        ((get_timestamp() / GUIDE_BLINK_PERIOD) % 2U) == 0U ? 1 : 0;
    if (proc->state_blink_visible == blink_visible) {
        return false;
    }
    proc->state_blink_visible = blink_visible;
    return true;
}

static void guide_idle_timeout_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_PUMP_LEVEL_LEFT_ADJUST &&
        proc->page_index != GUIDE_PAGE_PUMP_LEVEL_RIGHT_ADJUST) {
        return;
    }
    if (get_timestamp_ms(proc->last_adjust_time) < GUIDE_LEVEL_ADJUST_IDLE_TIMEOUT_MS) {
        return;
    }
    /// 引导页面选中模式时，亮灯.
    if (proc->led) {
        proc->led->set_led_level(PUMP_MODE_BL, LED_LEVEL_MAX);
        proc->led->set_led_level(PUMP_BREASTFEEDING_BL, LED_LEVEL_MAX);
    }
    proc->page_index = GUIDE_PAGE_PUMP_MODE_SELECT;
    proc->last_adjust_time = get_timestamp();
}

static void guide_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_guide_t *proc = (ui_proc_guide_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_GUIDE);
    guide_idle_timeout_handler(proc);
    guide_ui_blink_update(proc);
    ui_display_guide_process(ui);
}

static void guide_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            guide_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void guide_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_guide_prepare(ui);
}

static void guide_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_guide_remove(ui);
}

static void guide_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(GUIDE_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_guide_ui_animation(ui_proc_guide_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == GUIDE_UI_TYPE_HOME) {
        return;
    }
    ui_play_info_t play_info = {
        .tag = "guide_screen",
        .fps = GUIDE_ANIMATION_FREQ,
        .progress_cb = guide_animation_progress_cb,
        .display_prepare_cb = guide_ui_display_prepare,
        .display_remove_cb = guide_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = GUIDE_UI_TYPE_HOME;
}

void stop_guide_ui_animation(ui_proc_guide_t *proc)
{
    guide_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

void guide_key_event_handler(ui_proc_guide_t *proc, uint16_t keycode)
{
    EXCEPTION_CHECK_VOID(proc);
    if (keycode == KEY_CODE_CANCEL) {
        proc->setting.level[0] = BREAST_PUMP_LEVEL_1;
        proc->setting.level[1] = BREAST_PUMP_LEVEL_1;
        proc->setting.speed = BREAST_PUMP_SPEED_MID;
        if (proc->pump) {
            WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
        }
        proc->mode_selected = GUIDE_MODE_SELECT_BREASTFEEDING;
        POST_WM_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
        return;
    } 

    switch (proc->page_index) {
        case GUIDE_PAGE_PUMP_LEVEL_SINGLE_SELECT:
            if (keycode == KEY_CODE_LEFT_CONTROL) {
                proc->page_index = GUIDE_PAGE_PUMP_LEVEL_LEFT_ADJUST;
                proc->last_adjust_time = get_timestamp();
            } else if (keycode == KEY_CODE_RIGHT_CONTROL) {
                proc->page_index = GUIDE_PAGE_PUMP_LEVEL_RIGHT_ADJUST;
                proc->last_adjust_time = get_timestamp();
            }
            break;
        case GUIDE_PAGE_PUMP_MODE_SELECT: 
            if (keycode == KEY_CODE_MODE) {
                proc->mode_selected = GUIDE_MODE_SELECT_MODE;
                POST_WM_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
            } else if (keycode == KEY_CODE_BREASTFEEDING) {
                proc->mode_selected = GUIDE_MODE_SELECT_BREASTFEEDING;
                POST_WM_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
            }
            break;
        case GUIDE_PAGE_PUMP_MODE_UNSELECT:
            break;
        default:
            break;
    }
}

void guide_knob_event_handler(ui_proc_guide_t *proc, bool is_left)
{
    EXCEPTION_CHECK_VOID(proc);
    const bool is_increase = !is_left;

    switch (proc->page_index) {
        case GUIDE_PAGE_PUMP_LEVEL_DOUBLE_ADJUST:
            guide_level_adjust_both(proc, is_increase);
            proc->page_index = GUIDE_PAGE_PUMP_LEVEL_SINGLE_SELECT;
            if (proc->led) {
                proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX);
                proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, LED_LEVEL_MAX);
            }
            break;
        case GUIDE_PAGE_PUMP_LEVEL_LEFT_ADJUST:
            guide_level_adjust_side(proc, true, is_increase);
            break;
        case GUIDE_PAGE_PUMP_LEVEL_RIGHT_ADJUST:
            guide_level_adjust_side(proc, false, is_increase);
            break;
        default:
            break;
    }
}
