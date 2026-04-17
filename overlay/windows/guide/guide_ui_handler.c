/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h"
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
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_GUIDE);
    proc->wm->set_window_guidance_deployed_flag(true);
    multi_func_msg_data_t msg = {0};
    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;    
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void guide_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_guide_process(ui);
}

static void guide_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
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
    if (proc->current_ui_type == GUIDE_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
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
    proc->last_progress_ani_time = get_timestamp();
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

void guide_key_event_handler_v1(ui_proc_guide_t *proc, uint16_t keycode)
{
    if (keycode != KEY_CODE_START) {
        return;
    }

    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index >= GUIDE_PAGE_MAX - 1) {
        POST_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
        return;
    }
    LOG_WM("guide page index: %d", proc->page_index);
    proc->page_index++;
}

void guide_key_event_handler_v2(ui_proc_guide_t *proc, uint16_t keycode)
{
    uint8_t page_index = proc->page_index;
    if (keycode == KEY_CODE_POWER) {
        POST_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
        return;
    }
    switch (page_index) {
        case GUIDE_PAGE_HOME:
            if (keycode == KEY_CODE_KNOB_RIGHT) {
                proc->page_index = GUIDE_PAGE_START_PUMP;
                if (proc->led) {         
                    proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
                }
            } else {
                // guide_popup_ui_type_update(proc, GUIDE_POPUP_UI_OPERATION_FAILED);
            }
            break;
        case GUIDE_PAGE_START_PUMP:
            if (keycode == KEY_CODE_START) {
                proc->page_index = GUIDE_PAGE_SETTING_PUMP;
                if (proc->led) {
                    if (proc->led->hook && proc->led->hook->set_key_backlight) {
                        proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
                    }
                    proc->led->set_led_blink(PUMP_MODE_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
                    proc->led->set_led_blink(PUMP_LEFT_CONTROL_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
                    proc->led->set_led_blink(PUMP_RIGHT_CONTROL_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
                }
            } else {
                // guide_popup_ui_type_update(proc, GUIDE_POPUP_UI_OPERATION_FAILED);
            }
            break;
        case GUIDE_PAGE_SETTING_PUMP:
            if (keycode == KEY_CODE_LEFT_CONTROL || keycode == KEY_CODE_RIGHT_CONTROL ||
                keycode == KEY_CODE_MODE || keycode == KEY_CODE_FREQUENCY || keycode == KEY_CODE_BREASTFEEDING) {
                proc->page_index = GUIDE_PAGE_STOP_PUMP;
                if (proc->led) {
                    if (proc->led->hook && proc->led->hook->set_key_backlight) {
                        proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
                    }
                    proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
                }
            } else {
                // guide_popup_ui_type_update(proc, GUIDE_POPUP_UI_OPERATION_FAILED);
            }
            break;
        case GUIDE_PAGE_PUMP_MODE_SELECT:
            if (keycode == KEY_CODE_KNOB_RIGHT) {
                proc->page_index = GUIDE_PAGE_BIND_GUIDE;
                if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                    proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
                }
            } else {
                // guide_popup_ui_type_update(proc, GUIDE_POPUP_UI_OPERATION_FAILED);
            }
            break;
        default:
            break;
    }
}

void guide_key_3s_press_handler_v2(ui_proc_guide_t *proc, uint16_t keycode)
{
    if (keycode != KEY_CODE_START) {
        return;
    }
    /// 只有在停止泵奶页面时，才允许3秒长按启动键进入泵奶模式选择页面
    if (proc->page_index != GUIDE_PAGE_STOP_PUMP) {
        return;
    }
    proc->page_index = GUIDE_PAGE_PUMP_MODE_SELECT;
    if (proc->led) {
        if (proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
        }
    }
}

void guide_knob_left_event_handler(ui_proc_guide_t *proc)
{
   switch (proc->page_index) {
        case GUIDE_PAGE_START_PUMP:
            proc->page_index = GUIDE_PAGE_HOME;
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
            }
            break;
        case GUIDE_PAGE_SETTING_PUMP:
            proc->page_index = GUIDE_PAGE_START_PUMP;
            if (proc->led) {       
                if (proc->led->hook && proc->led->hook->set_key_backlight) {
                    proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
                }
                proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
            }
            break;
        case GUIDE_PAGE_STOP_PUMP:
            proc->page_index = GUIDE_PAGE_SETTING_PUMP;
            if (proc->led) {
                if (proc->led->hook && proc->led->hook->set_key_backlight) {
                    proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
                }
                proc->led->set_led_blink(PUMP_MODE_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
                proc->led->set_led_blink(PUMP_LEFT_CONTROL_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
                proc->led->set_led_blink(PUMP_RIGHT_CONTROL_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
            }
            break;
        case GUIDE_PAGE_PUMP_MODE_SELECT:
            proc->page_index = GUIDE_PAGE_STOP_PUMP;
            if (proc->led) {
                if (proc->led->hook && proc->led->hook->set_key_backlight) {
                    proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
                }
                proc->led->set_led_blink(PUMP_START_BL, LED_SLOW_BLINK_TIME, 0, LED_STATUS_OFF);
            }
            break;
        case GUIDE_PAGE_BIND_GUIDE:
            proc->page_index = GUIDE_PAGE_PUMP_MODE_SELECT;
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
            }
            break;
        default:
            // guide_popup_ui_type_update(proc, GUIDE_POPUP_UI_OPERATION_FAILED);
            break;
   }
}