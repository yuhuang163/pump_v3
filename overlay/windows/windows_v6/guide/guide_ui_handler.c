/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h"
#include "breast_pump_manager_api.h"
#include "key_input_manager_api.h" // IWYU pragma: keep
#include "resource_id_define.h" // IWYU pragma: keep
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_DISPLAY_TASK_NAME              "guide_display_cmp"
#define GUIDE_STOP_HOLD_COMPLETE_TASK_NAME   "guide_stop_hold_complete"
#define GUIDE_POWER_OFF_HOLD_COMPLETE_TASK_NAME "guide_power_off_hold_complete"

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
#if IS_SUPPORT_GUIDE_PUMP_START
    if (proc->pump) {
        int time_index = 0;
        WRITE_PUMP_WORK_MODE(proc->pump, BREAST_PUMP_MILK_BOOST_MODE, false, false);
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, false);
        proc->setting.level[0] = BREAST_PUMP_LEVEL_1;
        proc->setting.level[1] = BREAST_PUMP_LEVEL_1;
        proc->setting.speed = BREAST_PUMP_SPEED_MID;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
#endif
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

void guide_exit_to_home(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    POST_WM_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
}

static void guide_stop_hold_complete_cb(hdr_post_args_t *args)
{
    ui_proc_guide_t *proc = (ui_proc_guide_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_GUIDE);
    if (proc->page_index != GUIDE_PAGE_PUMP_FINISH_HOLD) {
        LOG_WM("guide stop hold complete ignored, page:%d.", proc->page_index);
        return;
    }
    LOG_WM("guide stop hold complete, goto pump home.");
    proc->stop_hold_holding = 0;
    proc->stop_hold_ignore_click = 1;
    guide_page_advance(proc, GUIDE_PAGE_PUMP_HOME);
}

static void guide_blink_state_restart(ui_proc_guide_t *proc)
{
    proc->blink_start_time = get_timestamp();
    proc->state_blink_visible = 1;
}

void guide_page_advance(ui_proc_guide_t *proc, uint16_t next_page)
{
    EXCEPTION_CHECK_VOID(proc);
    /// 弹窗期间禁止切页，否则新建组件会叠在弹窗之上
    if (proc->popup_ui_type != ID_POPUP_UI_NONE) {
        LOG_WM("skip page advance while popup:%d, page:%d -> %d.",
            proc->popup_ui_type, proc->page_index, next_page);
        return;
    }
    proc->page_index = next_page;
    proc->page_enter_time = get_timestamp();
    if (next_page == GUIDE_PAGE_MODE_SWITCH) {
        proc->ui_mode_index = GUIDE_MODE_SWITCH_SLOT_STIMULATION;
    }
    guide_blink_state_restart(proc);
    guide_led_backlight_update(proc);
}

int guide_mode_switch_adjust_handler(ui_proc_guide_t *proc, bool is_left)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->page_index != GUIDE_PAGE_MODE_SWITCH) {
        return -1;
    }
    if (is_left) {
        if (proc->ui_mode_index == 0) {
            return -1;
        }
        proc->ui_mode_index--;
    } else {
        if (proc->ui_mode_index + 1U >= GUIDE_MODE_SWITCH_SLOT_COUNT) {
            return -1;
        }
        proc->ui_mode_index++;
    }
    proc->mode_select_touched = 1;
    proc->last_adjust_time = get_timestamp();
    return 0;
}

static bool guide_ui_blink_update(ui_proc_guide_t *proc)
{
    const uint8_t blink_visible =
        ((get_timestamp_ms(proc->blink_start_time) / GUIDE_BLINK_PERIOD) % 2U) == 0U ? 1 : 0;
    if (proc->state_blink_visible == blink_visible) {
        return false;
    }
    proc->state_blink_visible = blink_visible;
    return true;
}

static void guide_guide_step_timeout_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (!GUIDE_PAGE_IS_GUIDE_STEP(proc->page_index) &&
        proc->page_index != GUIDE_PAGE_PUMP_HOME_GUIDE &&
        proc->page_index != GUIDE_PAGE_PUMP_PAUSE &&
        proc->page_index != GUIDE_PAGE_PUMP_HOME &&
        proc->page_index != GUIDE_PAGE_MODE_HOME &&
        proc->page_index != GUIDE_PAGE_LEVEL_SIDE_DONE &&
        proc->page_index != GUIDE_PAGE_LEVEL_BOTH_DONE &&
        proc->page_index != GUIDE_PAGE_SPEED_DONE) {
        return;
    }

    const uint32_t advance_ms = (proc->page_index == GUIDE_PAGE_PUMP_HOME_GUIDE)
        ? GUIDE_PUMP_HOME_GUIDE_ADVANCE_TIME_MS
        : GUIDE_GUIDE_STEP_ADVANCE_TIME_MS;
    if (get_timestamp_ms(proc->page_enter_time) < advance_ms) {
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_INTRO:
            guide_page_advance(proc, GUIDE_PAGE_PUMP_START_GUIDE);
            break;
        case GUIDE_PAGE_PUMP_HOME_GUIDE:
            guide_page_advance(proc, GUIDE_PAGE_PUMP_PAUSE_GUIDE);
            break;
        case GUIDE_PAGE_PUMP_PAUSE:
            guide_page_advance(proc, GUIDE_PAGE_PUMP_FINISH_GUIDE);
            break;
        case GUIDE_PAGE_PUMP_HOME:
            guide_page_advance(proc, GUIDE_PAGE_MODE_SWITCH_GUIDE);
            break;
        case GUIDE_PAGE_MODE_HOME:
            guide_page_advance(proc, GUIDE_PAGE_LEVEL_SIDE_GUIDE);
            break;
        case GUIDE_PAGE_LEVEL_SIDE_DONE:
            guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_GUIDE);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_DONE:
            guide_page_advance(proc, GUIDE_PAGE_SPEED_GUIDE);
            break;
        case GUIDE_PAGE_SPEED_DONE:
            guide_page_advance(proc, GUIDE_PAGE_FINISH);
            break;
        default:
            break;
    }
}

static void guide_finish_timeout_handler(ui_proc_guide_t *proc)
{
    (void)proc;
}

static void guide_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_guide_t *proc = (ui_proc_guide_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_GUIDE);

    /// 弹窗期间不跑自动切页逻辑，避免组件重建盖住弹窗
    if (proc->popup_ui_type == ID_POPUP_UI_NONE) {
        if (proc->page_index == GUIDE_PAGE_WELCOME_ANIM) {
            int res_theme_type = RES_THEME_TYPE_BASE;
            if (proc->ui && proc->ui->priv.res) {
                res_theme_type = proc->ui->priv.res->get_sys_theme_type();
            }
            if (res_theme_type == RES_THEME_TYPE_CUSTOM &&
                index >= (int)CUSTOM_GUIDE_ANIM_NUM) {
                // guide_page_advance(proc, GUIDE_PAGE_INTRO);
                guide_page_advance(proc, GUIDE_PAGE_PUMP_START_GUIDE);
            }
        }

        guide_guide_step_timeout_handler(proc);
        guide_pump_level_idle_timeout_handler(proc);
        guide_pump_both_level_over_limit_tick(proc);
        guide_finish_timeout_handler(proc);
    }

    const bool blink_changed = guide_ui_blink_update(proc);
    if (blink_changed && GUIDE_PAGE_IS_LED_BLINK(proc->page_index)) {
        guide_led_backlight_blink_tick(proc);
    }
    if (proc->page_index == GUIDE_PAGE_WELCOME_ANIM) {
        guide_led_backlight_update(proc);
    }

    ui_display_guide_process(ui);
}

static void guide_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    ui_proc_guide_t *proc;
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
            proc = (ui_proc_guide_t *)ui->priv.play_info.user_data;
            if (proc != NULL && proc->page_index == GUIDE_PAGE_PUMP_FINISH_HOLD) {
                POST_WM_TASK_HANDLER_NO_DELAY(guide_stop_hold_complete_cb, proc,
                    GUIDE_STOP_HOLD_COMPLETE_TASK_NAME);
            }
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
    REMOVE_TASK_HANDLER_DIRECTLY(GUIDE_STOP_HOLD_COMPLETE_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(GUIDE_POWER_OFF_HOLD_COMPLETE_TASK_NAME);
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

void guide_stop_hold_key_long_2s_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_PUMP_FINISH_GUIDE) {
        return;
    }
    if (proc->popup_ui_type != ID_POPUP_UI_NONE) {
        return;
    }
    LOG_WM("power long 2s, show guide pump finish hold.");
    proc->stop_hold_holding = 1;
    proc->stop_hold_ignore_click = 0;
    guide_page_advance(proc, GUIDE_PAGE_PUMP_FINISH_HOLD);
}

void guide_stop_hold_key_up_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_PUMP_FINISH_HOLD) {
        /// 长按已完成时松手：LONG_2S 后通常不再上报 CLICK，
        /// 需在 UP 时清掉 ignore，避免第 10 页首次短按被吞掉。
        if (proc->stop_hold_ignore_click) {
            proc->stop_hold_ignore_click = 0;
            LOG_WM("clear stop hold ignore click on power up after complete.");
        }
        return;
    }
    if (!proc->stop_hold_holding) {
        return;
    }
    LOG_WM("power key up before guide stop hold complete, back to finish guide.");
    proc->stop_hold_holding = 0;
    proc->stop_hold_ignore_click = 1;
    REMOVE_TASK_HANDLER_DIRECTLY(GUIDE_STOP_HOLD_COMPLETE_TASK_NAME);
    guide_page_advance(proc, GUIDE_PAGE_PUMP_FINISH_GUIDE);
}

void guide_key_event_handler(ui_proc_guide_t *proc, uint16_t keycode, uint16_t keytype)
{
    EXCEPTION_CHECK_VOID(proc);
    (void)keytype;
    if (GUIDE_PAGE_IS_WELCOME_ANIM(proc->page_index)) {
        return;
    }

    if (keycode == KEY_CODE_POWER && proc->stop_hold_ignore_click) {
        proc->stop_hold_ignore_click = 0;
        LOG_WM("ignore power click after guide stop hold release.");
        return;
    }
    if (keycode == KEY_CODE_POWER && proc->power_off_hold_ignore_click) {
        proc->power_off_hold_ignore_click = 0;
        LOG_WM("ignore power click after guide power off hold.");
        return;
    }

    if (proc->page_index == GUIDE_PAGE_PUMP_FINISH_HOLD) {
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_QRCODE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_INTRO);
            }
            break;
        case GUIDE_PAGE_INTRO:
            if (keycode == KEY_CODE_START) {
                guide_page_advance(proc, GUIDE_PAGE_PUMP_START_GUIDE);
            }
            break;
        case GUIDE_PAGE_PUMP_START_GUIDE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_PUMP_HOME_GUIDE);
            }
            break;
        case GUIDE_PAGE_PUMP_PAUSE_GUIDE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_PUMP_PAUSE);
            }
            break;
        case GUIDE_PAGE_MODE_SWITCH_GUIDE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_SIDE_GUIDE);
            }
            break;
        case GUIDE_PAGE_MODE_SWITCH:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_MODE_HOME);
            }
            break;
        case GUIDE_PAGE_LEVEL_SIDE_GUIDE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_GUIDE);
            } else if (keycode == KEY_CODE_LEFT_CONTROL || keycode == KEY_CODE_RIGHT_CONTROL) {
                proc->level_adjust_side = (keycode == KEY_CODE_LEFT_CONTROL) ?
                    GUIDE_LEVEL_ADJUST_SIDE_LEFT : GUIDE_LEVEL_ADJUST_SIDE_RIGHT;
                proc->level_value = proc->setting.level[proc->level_adjust_side];
                proc->level_arc_level = proc->level_value;
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_SIDE);
            }
            break;
        case GUIDE_PAGE_LEVEL_SIDE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_SIDE_DONE);
            }
            break;
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_SPEED_GUIDE);
            } else if (keycode == KEY_CODE_BOTH_CONTROL) {
                guide_pump_both_level_page_enter_sync(proc);
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH);
            }
            break;
        case GUIDE_PAGE_LEVEL_BOTH:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_DONE);
            }
            break;
        case GUIDE_PAGE_SPEED_GUIDE:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_FINISH);
            } else if (keycode == KEY_CODE_FREQUENCY) {
                guide_pump_speed_page_enter_sync(proc);
                guide_page_advance(proc, GUIDE_PAGE_SPEED);
            }
            break;
        case GUIDE_PAGE_SPEED:
            if (keycode == KEY_CODE_POWER) {
                guide_page_advance(proc, GUIDE_PAGE_SPEED_DONE);
            }
            break;
        case GUIDE_PAGE_FINISH:
            if (keycode == KEY_CODE_POWER) {
                guide_exit_to_home(proc);
            }
            break;
        default:
            break;
    }
}

void guide_knob_event_handler(ui_proc_guide_t *proc, bool is_left)
{
    EXCEPTION_CHECK_VOID(proc);
    if (GUIDE_PAGE_IS_WELCOME_ANIM(proc->page_index) ||
        proc->page_index == GUIDE_PAGE_QRCODE ||
        proc->page_index == GUIDE_PAGE_INTRO) {
        return;
    }

    if (proc->page_index == GUIDE_PAGE_MODE_SWITCH_GUIDE) {
        if (!is_left) {
            guide_page_advance(proc, GUIDE_PAGE_MODE_SWITCH);
        }
        return;
    }

    if (proc->page_index == GUIDE_PAGE_MODE_SWITCH) {
        guide_mode_switch_adjust_handler(proc, is_left);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_LEVEL_SIDE) {
        guide_pump_level_adjust_handler(proc, !is_left);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_LEVEL_BOTH) {
        guide_pump_both_level_adjust_handler(proc, !is_left);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_SPEED) {
        guide_pump_speed_adjust_handler(proc, !is_left);
        return;
    }

    (void)is_left;
}
