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

static void guide_exit_to_home(ui_proc_guide_t *proc)
{
    POST_WM_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
}

static void guide_exit_settings_reset(ui_proc_guide_t *proc)
{
    proc->setting.level[0] = BREAST_PUMP_LEVEL_1;
    proc->setting.level[1] = BREAST_PUMP_LEVEL_1;
    proc->setting.speed = BREAST_PUMP_SPEED_MID;
    if (proc->pump) {
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
    proc->mode_selected = GUIDE_MODE_SELECT_BREASTFEEDING;
}

static bool guide_exit_countdown_allowed(ui_proc_guide_t *proc)
{
    return !GUIDE_PAGE_IS_WELCOME_ANIM(proc->page_index) &&
        proc->page_index != GUIDE_PAGE_FINISH;
}

void guide_exit_countdown_popup_dismiss(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->popup_ui_type != ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN) {
        return;
    }
    LOG_WM("dismiss guide exit countdown popup, page:%d.", proc->page_index);
    proc->exit_holding = 0;
    proc->popup_ui_type = ID_POPUP_UI_NONE;
    /// 弹窗占用期间暂停了自动切页，关闭后刷新计时，避免立刻翻页
    proc->last_adjust_time = get_timestamp();
    proc->page_enter_time = get_timestamp();
}

void guide_cancel_key_down_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
#if IS_SUPPORT_GUIDE_KEY_GUIDE_CANCEL_ADVANCE
    if (proc->page_index == GUIDE_PAGE_KEY_GUIDE) {
        return;
    }
#endif
    if (proc->popup_ui_type == ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN) {
        return;
    }
    if (!guide_exit_countdown_allowed(proc)) {
        return;
    }

    LOG_WM("guide exit countdown show on cancel key down, page:%d.", proc->page_index);
    proc->exit_holding = 1;
    proc->exit_ignore_click = 0;
    proc->popup_ui_type = ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN;
}

void guide_cancel_key_up_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->popup_ui_type != ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN) {
        return;
    }
    if (!proc->exit_holding) {
        return;
    }
    LOG_WM("guide exit hold release before complete, dismiss popup.");
    guide_exit_countdown_popup_dismiss(proc);
    proc->exit_ignore_click = 1;
}

static void guide_popup_ui_finish_policy(ui_manager_context_t *ui, uint8_t popup_ui_index)
{
    ui_proc_guide_t *proc = (ui_proc_guide_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_GUIDE);
    if (popup_ui_index != ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN ||
        proc->popup_ui_type != ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN) {
        return;
    }

    LOG_WM("===> guide exit countdown complete, page:%d.", proc->page_index);
    proc->popup_ui_type = ID_POPUP_UI_NONE;
    proc->exit_holding = 0;
    proc->exit_ignore_click = 1;

    if (proc->page_index == GUIDE_PAGE_PUMP_STOP_GUIDE) {
        guide_page_advance(proc, GUIDE_PAGE_FINISH);
        return;
    }
    guide_exit_settings_reset(proc);
    guide_exit_to_home(proc);
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

    switch (next_page) {
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
#if IS_SUPPORT_GUIDE_PUMP_START
        if (proc->pump) {
            WRITE_PUMP_WORK_MODE(proc->pump, BREAST_PUMP_STIMULATION_MODE, false, false);
            WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_START, false);
        }
#endif
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
        case GUIDE_PAGE_LEVEL_LEFT_GUIDE:
        case GUIDE_PAGE_LEVEL_LEFT:
        case GUIDE_PAGE_LEVEL_RIGHT_GUIDE:
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_pump_level_page_enter_reset(proc);
            break;
        case GUIDE_PAGE_MODE_SELECT_GUIDE:
            proc->ui_mode_index = BREAST_PUMP_STIMULATION_MODE;
            break;
        case GUIDE_PAGE_RHYTHM_SELECT_GUIDE:
            proc->ui_mode_index = BREAST_PUMP_MILK_BOOST_MODE;
            break;
        case GUIDE_PAGE_MODE_SELECT:
#if IS_SUPPORT_GUIDE_PUMP_START
            if (proc->pump) {
                WRITE_PUMP_WORK_MODE(proc->pump, BREAST_PUMP_STIMULATION_MODE, false, false);
            }
#endif
            proc->mode_select_touched = 0;
            proc->mode_select_click_cnt = 0;
            break;
        case GUIDE_PAGE_RHYTHM_SELECT:
#if IS_SUPPORT_GUIDE_PUMP_START
            if (proc->pump) {
                WRITE_PUMP_WORK_MODE(proc->pump, BREAST_PUMP_MILK_BOOST_MODE, false, false);
            }
#endif
            proc->rhythm_select_touched = 0;
            proc->rhythm_select_click_cnt = 0;
            break;
        case GUIDE_PAGE_PUMP_STOP_GUIDE:
#if IS_SUPPORT_GUIDE_PUMP_START
            if (proc->pump) {
                WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_PAUSE, false);
            }
#endif
            break;
        default:
            break;
    }
    guide_blink_state_restart(proc);
    guide_led_backlight_update(proc);
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
    if (!GUIDE_PAGE_IS_GUIDE_STEP(proc->page_index)) {
        return;
    }
    if (get_timestamp_ms(proc->page_enter_time) < GUIDE_GUIDE_STEP_ADVANCE_TIME_MS) {
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_INTRO:
            guide_page_advance(proc, GUIDE_PAGE_KNOB_GUIDE);
            break;
        default:
            break;
    }
}

static void guide_finish_timeout_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_FINISH) {
        return;
    }
    if (get_timestamp_ms(proc->page_enter_time) < GUIDE_FINISH_IDLE_TIMEOUT_MS) {
        return;
    }
    guide_exit_to_home(proc);
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
                guide_page_advance(proc, GUIDE_PAGE_KNOB_GUIDE);
            }
        }

        guide_guide_step_timeout_handler(proc);
        guide_pump_level_idle_timeout_handler(proc);
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
            guide_popup_ui_finish_policy(ui, index);
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

static void guide_knob_advance_step(ui_proc_guide_t *proc)
{
    switch (proc->page_index) {
        case GUIDE_PAGE_KNOB_GUIDE:
            guide_page_advance(proc, GUIDE_PAGE_KEY_GUIDE);
            break;
        default:
            break;
    }
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

void guide_key_event_handler(ui_proc_guide_t *proc, uint16_t keycode, uint16_t keytype)
{
    EXCEPTION_CHECK_VOID(proc);
    (void)keytype;
    if (GUIDE_PAGE_IS_WELCOME_ANIM(proc->page_index)) {
        return;
    }

    if (keycode == KEY_CODE_CANCEL && proc->exit_ignore_click) {
        proc->exit_ignore_click = 0;
        LOG_WM("ignore cancel click after guide exit countdown release.");
        return;
    }

    if (proc->popup_ui_type == ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN) {
        guide_exit_countdown_popup_dismiss(proc);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_FINISH) {
        guide_exit_to_home(proc);
        return;
    }

    if (keycode == KEY_CODE_CANCEL) {
#if IS_SUPPORT_GUIDE_KEY_GUIDE_CANCEL_ADVANCE
        if (proc->page_index == GUIDE_PAGE_KEY_GUIDE) {
            guide_page_advance(proc, GUIDE_PAGE_FUNCTION_HOME);
            return;
        }
#endif
        /// 退出倒计时由 DOWN/UP 处理，短按 CLICK 忽略
        return;
    }

    if (proc->page_index == GUIDE_PAGE_KEY_GUIDE && keycode != KEY_CODE_POWER) {
        guide_page_advance(proc, GUIDE_PAGE_FUNCTION_HOME);
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_INTRO:
            if (keycode == KEY_CODE_START) {
                guide_page_advance(proc, GUIDE_PAGE_KNOB_GUIDE);
            }
            break;
        case GUIDE_PAGE_KNOB_GUIDE:
            break;
        case GUIDE_PAGE_FUNCTION_HOME:
            if (keycode == KEY_CODE_START) {
                guide_page_advance(proc, GUIDE_PAGE_MODE_SELECT_GUIDE);
            }
            break;
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
            break;
        case GUIDE_PAGE_LEVEL_LEFT_GUIDE:
            if (keycode == KEY_CODE_LEFT_CONTROL) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_LEFT);
            }
            break;
        case GUIDE_PAGE_LEVEL_RIGHT_GUIDE:
            if (keycode == KEY_CODE_RIGHT_CONTROL) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_RIGHT);
            }
            break;
        case GUIDE_PAGE_MODE_SELECT_GUIDE:
            if (keycode == KEY_CODE_MODE) {
                guide_page_advance(proc, GUIDE_PAGE_MODE_SELECT);
            }
            break;
        case GUIDE_PAGE_MODE_SELECT:
            if (keycode == KEY_CODE_MODE) {
                guide_pump_mode_adjust_handler(proc, false);
                if (proc->mode_select_click_cnt >= GUIDE_MODE_SELECT_ADVANCE_CLICK_COUNT) {
                    proc->mode_selected = GUIDE_MODE_SELECT_MODE;
                    guide_page_advance(proc, GUIDE_PAGE_RHYTHM_SELECT_GUIDE);
                }
            } else if (keycode == KEY_CODE_BREASTFEEDING) {
                proc->mode_selected = GUIDE_MODE_SELECT_BREASTFEEDING;
                guide_page_advance(proc, GUIDE_PAGE_RHYTHM_SELECT_GUIDE);
            }
            break;
        case GUIDE_PAGE_RHYTHM_SELECT_GUIDE:
            if (keycode == KEY_CODE_BREASTFEEDING) {
                guide_page_advance(proc, GUIDE_PAGE_RHYTHM_SELECT);
            }
            break;
        case GUIDE_PAGE_RHYTHM_SELECT:
            if (keycode == KEY_CODE_BREASTFEEDING) {
                guide_pump_mode_adjust_handler(proc, true);
                if (proc->rhythm_select_click_cnt >= GUIDE_RHYTHM_SELECT_ADVANCE_CLICK_COUNT) {
                    guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_GUIDE);
                }
            } else if (keycode == KEY_CODE_MODE) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_GUIDE);
            }
            break;
        default:
            if (GUIDE_PAGE_IS_GUIDE_STEP(proc->page_index)) {
                guide_knob_advance_step(proc);
            }
            break;
    }
}

void guide_knob_event_handler(ui_proc_guide_t *proc, bool is_left)
{
    EXCEPTION_CHECK_VOID(proc);
    if (GUIDE_PAGE_IS_WELCOME_ANIM(proc->page_index) ||
        proc->page_index == GUIDE_PAGE_INTRO) {
        return;
    }

    if (proc->popup_ui_type == ID_POPUP_UI_GUIDE_EXIT_COUNTDOWN) {
        guide_exit_countdown_popup_dismiss(proc);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_FINISH) {
        guide_exit_to_home(proc);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_LEVEL_BOTH_GUIDE) {
        guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_INCREASE);
        return;
    }

    if (GUIDE_PAGE_IS_GUIDE_STEP(proc->page_index)) {
        guide_knob_advance_step(proc);
        return;
    }

    const bool is_increase = !is_left;
    guide_pump_level_adjust_handler(proc, is_increase);
}
