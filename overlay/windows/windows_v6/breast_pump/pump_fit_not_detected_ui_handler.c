/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_pump.h"
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
#include "ui_page_define.h" // IWYU pragma: keep
#include "hal_led.h" // IWYU pragma: keep
#include "root_algo.h" // IWYU pragma: keep
#endif

#if IS_SUPPORT_DEVICE_AIR_PRESSURE
/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_FIT_NOT_DETECTED_BLINK_PERIOD_MS          (1000) /// 半周期 1000ms → 0.5Hz
#ifndef IS_SUPPORT_PUMP_FIT_NOT_DETECTED_ONCE
#define IS_SUPPORT_PUMP_FIT_NOT_DETECTED_ONCE          (1) /// 1: 一次吸乳仅触发一次
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_fit_not_detected_led_all_off(ui_proc_pump_t *proc)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(proc && proc->led && proc->led->set_led_level);
    if (proc->led->hook && proc->led->hook->set_key_backlight) {
        proc->led->hook->set_key_backlight(proc->led, 0);
    }
    for (i = 0; i < PUMP_LED_INDEX_MAX; i++) {
        proc->led->set_led_level(i, 0);
    }
}

static void pump_fit_not_detected_led_restore(ui_proc_pump_t *proc)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(proc && proc->led);
    if (proc->led->hook && proc->led->hook->set_key_backlight) {
        proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
    } else if (proc->led->set_led_level) {
        for (i = 0; i < PUMP_LED_INDEX_MAX; i++) {
            proc->led->set_led_level(i, LED_LEVEL_MAX);
        }
    }
    proc->pump_light_status = PUMP_LIGHT_STATUS_DEFAULT;
}

static void pump_fit_not_detected_pause_pump(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    if (!PUMP_IS_GOING(proc->pump_status)) {
        return;
    }
    WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_PAUSE, 1);
    proc->pump_status = BREAST_PUMP_PAUSE;
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
    pump_pause_idle_start(proc);
#endif
    LOG_WM("fit not detected, pause pump.");
}

static void pump_fit_not_detected_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc;

    (void)index;
    proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_fit_not_detected_process(ui);
    pump_fit_not_detected_led_tick(proc);
}

static void pump_fit_not_detected_animation_progress_cb(void *user_data, int index,
    ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;

    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_fit_not_detected_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_fit_not_detected_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;

    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_fit_not_detected_prepare(ui);
}

static void pump_fit_not_detected_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;

    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_fit_not_detected_remove(ui);
}

static bool pump_fit_not_detected_can_jump(ui_proc_pump_t *proc)
{
    if (proc == NULL || proc->pump == NULL) {
        return false;
    }
#if IS_SUPPORT_PUMP_FIT_NOT_DETECTED_ONCE
    if (proc->fit_not_detected_shown) {
        return false;
    }
#endif
    if (proc->current_ui_type != PUMP_UI_TYPE_HOME &&
        proc->current_ui_type != PUMP_UI_TYPE_LEVEL_ADJUST &&
        proc->current_ui_type != PUMP_UI_TYPE_BOTH_LEVEL_ADJUST &&
        proc->current_ui_type != PUMP_UI_TYPE_SPEED_ADJUST) {
        return false;
    }
    if (!PUMP_IS_GOING(proc->pump_status)) {
        return false;
    }
    return true;
}

static void pump_fit_not_detected_do_jump(ui_proc_pump_t *proc)
{
    if (!pump_fit_not_detected_can_jump(proc)) {
        return;
    }

    LOG_WM("not worn detected, jump to popup.");
    proc->seal_tip = PUMP_SEAL_TIP_NONE;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
    pump_fit_not_detected_pause_pump(proc);
    show_pump_fit_not_detected_ui(proc);
}

bool pump_fit_not_detected_is_not_worn(const uint8_t *weared)
{
    uint8_t i;

    if (weared == NULL) {
        return false;
    }
    for (i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (weared[i] == ALGO_WEARED_STATUS_NOT_WORN ||
            weared[i] == ALGO_WEARED_STATUS_WORN_OVT) {
            return true;
        }
    }
    return false;
}

bool pump_fit_not_detected_try_jump(ui_proc_pump_t *proc)
{
    if (!pump_fit_not_detected_can_jump(proc)) {
        return false;
    }
    pump_fit_not_detected_do_jump(proc);
    return proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_fit_not_detected_reset(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_FIT_NOT_DETECTED_TASK_NAME);
}

void pump_fit_not_detected_on_wear(ui_proc_pump_t *proc, const uint8_t *weared)
{
    uint8_t i;
    uint8_t worn_ovt = 0;

    EXCEPTION_CHECK_VOID(proc);
    if (weared == NULL || proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        return;
    }

    if (!pump_fit_not_detected_can_jump(proc)) {
        return;
    }

    if (get_timestamp_ms(proc->seal_log_time) > 1000) {
        LOG_WM("wear result: %d/%d", weared[0], weared[1]);
    }

    /// 检测中超时立即跳未佩戴页；NOT_WORN 等漏气结果出来后由 seal resolve 按优先级处理
    for (i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (weared[i] == ALGO_WEARED_STATUS_WORN_OVT) {
            worn_ovt = 1;
            break;
        }
    }

    if (!worn_ovt) {
        return;
    }
    // LOG_WM("weared status: %d/%d, jump fit not detected.", weared[0], weared[1]);
    
    pump_fit_not_detected_do_jump(proc);
}

void pump_fit_not_detected_led_tick(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->led && proc->led->set_led_level);
    if (proc->current_ui_type != PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        return;
    }

    if (proc->pump_light_status != PUMP_LIGHT_STATUS_FIT_NOT_DETECTED) {
        pump_fit_not_detected_led_all_off(proc);
        proc->pump_light_status = PUMP_LIGHT_STATUS_FIT_NOT_DETECTED;
#if IS_SUPPORT_PUMP_START_LED_BREATH
        EXCEPTION_CHECK_VOID(proc->led->set_led_breath);
        proc->led->set_led_breath(PUMP_START_BL, 0, 0);
#else
        proc->blink_start_time = get_timestamp();
        proc->state_blink_visible = 1;
        proc->pump_pause_blink_state = 0xFF;
        proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        proc->pump_pause_blink_state = proc->state_blink_visible;
#endif
        return;
    }

#if IS_SUPPORT_PUMP_START_LED_BREATH
    return;
#else
    if (get_timestamp_ms(proc->blink_start_time) < PUMP_FIT_NOT_DETECTED_BLINK_PERIOD_MS) {
        return;
    }
    proc->blink_start_time = get_timestamp();
    proc->state_blink_visible = proc->state_blink_visible ? 0 : 1;
    proc->led->set_led_level(PUMP_START_BL,
        proc->state_blink_visible ? LED_LEVEL_MAX : 0);
    proc->pump_pause_blink_state = proc->state_blink_visible;
#endif
}

void show_pump_fit_not_detected_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->servers && proc->servers->set_app_ctrl_block) {
        proc->servers->set_app_ctrl_block(SERVERS_APP_CTRL_BLOCK_FIT_NOT_DETECTED);
    }
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        return;
    }
    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);

    ui_play_info_t play_info = {
        .tag = "pump_fit_not_detected_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_fit_not_detected_animation_progress_cb,
        .display_prepare_cb = pump_fit_not_detected_ui_display_prepare,
        .display_remove_cb = pump_fit_not_detected_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_FIT_NOT_DETECTED;
#if IS_SUPPORT_PUMP_FIT_NOT_DETECTED_ONCE
    proc->fit_not_detected_shown = 1;
#endif
    proc->pump_light_status = PUMP_LIGHT_STATUS_DEFAULT;
    pump_fit_not_detected_led_tick(proc);
    LOG_WM("show pump fit not detected ui.");
}

void pump_fit_not_detected_ui_exit(ui_proc_pump_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_FIT_NOT_DETECTED_TASK_NAME);
    if (proc && proc->servers && proc->servers->clear_app_ctrl_block) {
        proc->servers->clear_app_ctrl_block(SERVERS_APP_CTRL_BLOCK_FIT_NOT_DETECTED);
    }
    if (proc != NULL && proc->pump_light_status == PUMP_LIGHT_STATUS_FIT_NOT_DETECTED) {
        pump_fit_not_detected_led_restore(proc);
    }
}

void pump_fit_not_detected_resume(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->servers && proc->servers->clear_app_ctrl_block) {
        proc->servers->clear_app_ctrl_block(SERVERS_APP_CTRL_BLOCK_FIT_NOT_DETECTED);
    }
    if (proc->current_ui_type != PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        return;
    }
    pump_fit_not_detected_reset(proc);
    pump_fit_not_detected_led_restore(proc);
    pump_start_pump_keep_ui(proc);
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
}

#else /* !IS_SUPPORT_DEVICE_AIR_PRESSURE */

void pump_fit_not_detected_reset(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_fit_not_detected_on_wear(ui_proc_pump_t *proc, const uint8_t *weared)
{
    (void)proc;
    (void)weared;
}

void pump_fit_not_detected_led_tick(ui_proc_pump_t *proc)
{
    (void)proc;
}

void show_pump_fit_not_detected_ui(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_fit_not_detected_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_fit_not_detected_resume(ui_proc_pump_t *proc)
{
    (void)proc;
}

bool pump_fit_not_detected_is_not_worn(const uint8_t *weared)
{
    (void)weared;
    return false;
}

bool pump_fit_not_detected_try_jump(ui_proc_pump_t *proc)
{
    (void)proc;
    return false;
}

#endif /* IS_SUPPORT_DEVICE_AIR_PRESSURE */
