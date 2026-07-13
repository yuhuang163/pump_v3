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

static void guide_exit_to_home(ui_proc_guide_t *proc)
{
    POST_WM_TASK_HANDLER_NO_DELAY(guide_display_completed_cb, proc, GUIDE_DISPLAY_TASK_NAME);
}

static void guide_blink_state_sync(ui_proc_guide_t *proc)
{
    proc->state_blink_visible =
        ((get_timestamp() / GUIDE_BLINK_PERIOD) % 2U) == 0U ? 1 : 0;
}

void guide_page_advance(ui_proc_guide_t *proc, uint16_t next_page)
{
    proc->page_index = next_page;
    proc->page_enter_time = get_timestamp();
    if (next_page >= GUIDE_PAGE_LEVEL_BOTH_GUIDE && next_page <= GUIDE_PAGE_LEVEL_RIGHT) {
        guide_pump_level_page_enter_reset(proc);
    }
    if (next_page == GUIDE_PAGE_MODE_SELECT_GUIDE) {
        proc->ui_mode_index = BREAST_PUMP_STIMULATION_MODE;
    }
    if (next_page == GUIDE_PAGE_RHYTHM_SELECT_GUIDE) {
        proc->ui_mode_index = BREAST_PUMP_MILK_BOOST_MODE;
    }
    if (next_page == GUIDE_PAGE_MODE_SELECT) {
        proc->mode_select_touched = 0;
        proc->last_adjust_time = get_timestamp();
    }
    if (next_page == GUIDE_PAGE_RHYTHM_SELECT) {
        proc->rhythm_select_touched = 0;
        proc->last_adjust_time = get_timestamp();
    }
    guide_blink_state_sync(proc);
    guide_led_backlight_update(proc);
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
        default:
            break;
    }
}

static uint32_t guide_mode_select_idle_threshold_get(ui_proc_guide_t *proc)
{
    return proc->mode_select_touched ?
        GUIDE_MODE_SELECT_MISOP_TIMEOUT_MS : GUIDE_MODE_SELECT_IDLE_TIMEOUT_MS;
}

static void guide_mode_select_idle_timeout_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_MODE_SELECT) {
        return;
    }
    if (get_timestamp_ms(proc->last_adjust_time) < guide_mode_select_idle_threshold_get(proc)) {
        return;
    }
    proc->mode_selected = GUIDE_MODE_SELECT_MODE;
    guide_page_advance(proc, GUIDE_PAGE_RHYTHM_SELECT_GUIDE);
}

static uint32_t guide_rhythm_select_idle_threshold_get(ui_proc_guide_t *proc)
{
    return proc->rhythm_select_touched ?
        GUIDE_RHYTHM_SELECT_MISOP_TIMEOUT_MS : GUIDE_RHYTHM_SELECT_IDLE_TIMEOUT_MS;
}

static void guide_rhythm_select_idle_timeout_handler(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->page_index != GUIDE_PAGE_RHYTHM_SELECT) {
        return;
    }
    if (get_timestamp_ms(proc->last_adjust_time) < guide_rhythm_select_idle_threshold_get(proc)) {
        return;
    }
    guide_page_advance(proc, GUIDE_PAGE_PUMP_START_GUIDE);
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

    if (proc->page_index == GUIDE_PAGE_WELCOME_ANIM &&
        index >= (int)BASE_GUIDE_ANIM_NUM) {
        guide_page_advance(proc, GUIDE_PAGE_INTRO);
    }

    guide_guide_step_timeout_handler(proc);
    guide_mode_select_idle_timeout_handler(proc);
    guide_rhythm_select_idle_timeout_handler(proc);
    guide_pump_level_idle_timeout_handler(proc);
    guide_finish_timeout_handler(proc);

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
            guide_page_advance(proc, GUIDE_PAGE_FUNCTION_HOME);
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
    if (GUIDE_PAGE_IS_WELCOME_ANIM(proc->page_index)) {
        return;
    }

    if (proc->page_index == GUIDE_PAGE_FINISH) {
        guide_exit_to_home(proc);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_KEY_GUIDE) {
        guide_page_advance(proc, GUIDE_PAGE_KNOB_GUIDE);
        return;
    }

    if (keycode == KEY_CODE_CANCEL) {
        if (proc->page_index == GUIDE_PAGE_PUMP_STOP_GUIDE) {
            if (keytype == KEY_EVENT_LONG_3S) {
                guide_page_advance(proc, GUIDE_PAGE_FINISH);
            }
            return;
        }
        proc->setting.level[0] = BREAST_PUMP_LEVEL_1;
        proc->setting.level[1] = BREAST_PUMP_LEVEL_1;
        proc->setting.speed = BREAST_PUMP_SPEED_MID;
        if (proc->pump) {
            WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
        }
        proc->mode_selected = GUIDE_MODE_SELECT_BREASTFEEDING;
        guide_exit_to_home(proc);
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_INTRO:
            if (keycode == KEY_CODE_START) {
                guide_page_advance(proc, GUIDE_PAGE_KEY_GUIDE);
            }
            break;
        case GUIDE_PAGE_KNOB_GUIDE:
            break;
        case GUIDE_PAGE_FUNCTION_HOME:
            if (keycode == KEY_CODE_START) {
                guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH_GUIDE);
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
            } else if (keycode == KEY_CODE_MODE) {
                guide_page_advance(proc, GUIDE_PAGE_PUMP_START_GUIDE);
            }
            break;
        case GUIDE_PAGE_PUMP_START_GUIDE:
            if (keycode == KEY_CODE_START) {
                guide_page_advance(proc, GUIDE_PAGE_PUMP_STOP_GUIDE);
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

    if (proc->page_index == GUIDE_PAGE_FINISH) {
        guide_exit_to_home(proc);
        return;
    }

    if (proc->page_index == GUIDE_PAGE_LEVEL_BOTH_GUIDE) {
        guide_page_advance(proc, GUIDE_PAGE_LEVEL_BOTH);
        return;
    }

    if (GUIDE_PAGE_IS_GUIDE_STEP(proc->page_index)) {
        guide_knob_advance_step(proc);
        return;
    }

    const bool is_increase = !is_left;
    guide_pump_level_adjust_handler(proc, is_increase);
}
