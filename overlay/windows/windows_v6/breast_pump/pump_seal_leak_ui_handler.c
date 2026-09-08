/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_pump.h"
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
#include "ui_page_define.h" // IWYU pragma: keep
#include "algorithm_manager_api.h" // IWYU pragma: keep
#endif

#if IS_SUPPORT_DEVICE_AIR_PRESSURE
/*
* DEFINES
*****************************************************************************************
*/
#ifndef PUMP_SEAL_LEAK_MOCK_ENABLE
#define PUMP_SEAL_LEAK_MOCK_ENABLE                      (1) /// 1: 模拟漏气结果，0: 走算法
#endif
#define PUMP_SEAL_LEAK_BLINK_PERIOD_MS                    (1000) /// 半周期 1000ms → 0.5Hz
#if PUMP_SEAL_LEAK_MOCK_ENABLE
#define PUMP_SEAL_LEAK_MOCK_INTERVAL_MS                  (5000)
#define PUMP_SEAL_LEAK_MOCK_TASK_NAME                    "pump_seal_leak_mock"
#define PUMP_SEAL_LEAK_MOCK_STEP_SEVERE                 (0)
#define PUMP_SEAL_LEAK_MOCK_STEP_SLIGHT                 (1)
#define PUMP_SEAL_LEAK_MOCK_STEP_NO_LEAK                (2)
#define PUMP_SEAL_LEAK_MOCK_STEP_INVALID                (3)
#define PUMP_SEAL_LEAK_MOCK_STEP_MAX                    (4)
#define PUMP_SEAL_LEAK_MOCK_INVALID_INTERVAL_MS          (25000)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if PUMP_SEAL_LEAK_MOCK_ENABLE
static uint8_t s_seal_leak_mock_step;
static uint8_t s_seal_leak_mock_seed;
static void pump_seal_leak_mock_tick_cb(hdr_post_args_t *args);
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void pump_seal_leak_led_all_off(ui_proc_pump_t *proc)
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

static void pump_seal_leak_led_restore(ui_proc_pump_t *proc)
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

static void pump_seal_leak_pause_pump(ui_proc_pump_t *proc)
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
    LOG_WM("severe leak, pause pump.");
}

static void pump_seal_leak_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc;

    (void)index;
    proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    ui_display_pump_seal_leak_process(ui);
    pump_seal_leak_led_tick(proc);
}

static void pump_seal_leak_animation_progress_cb(void *user_data, int index,
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
            pump_seal_leak_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_seal_leak_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;

    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_seal_leak_prepare(ui);
}

static void pump_seal_leak_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;

    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_seal_leak_remove(ui);
}

static uint8_t pump_seal_result_from_wear(ui_proc_pump_t *proc, const uint8_t *wear)
{
    uint8_t worst = ALGO_WEAR_DETECTION_RESULT_INVALID;
    uint8_t i;

    if (wear == NULL) {
        return PUMP_SEAL_RESULT_NONE;
    }

    /// leakage_status: 0 无效, 1 无漏气, 2 微小漏气, 3 强漏气；两侧取更差一侧
    for (i = 0; i < AIR_PRESSURE_NUM; i++) {
        proc->leak_channel[i] = (wear[i] == ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK);
        if (wear[i] > worst && wear[i] <= ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK) {
            worst = wear[i];
        }
    }
    
    #if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
    memcpy(proc->wear_result, wear, sizeof(proc->wear_result));
    #endif
    
    if (get_timestamp_ms(proc->seal_log_time) > 1000) {
        proc->seal_log_time = get_timestamp();
        LOG_WM("wear result: %d(%d/%d)", worst, wear[0], wear[1]);
    }

    switch (worst) {
        case ALGO_WEAR_DETECTION_RESULT_NORMAL:
            return PUMP_SEAL_RESULT_NO_LEAK;
        case ALGO_WEAR_DETECTION_RESULT_SMALL_LEAK:
            return PUMP_SEAL_RESULT_SLIGHT;
        case ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK:
            return PUMP_SEAL_RESULT_SEVERE;
        default:
            return PUMP_SEAL_RESULT_NONE;
    }
}

static bool pump_seal_leak_can_jump(ui_proc_pump_t *proc)
{
    if (proc == NULL || proc->pump == NULL) {
        return false;
    }

#if !PUMP_SEAL_LEAK_MOCK_ENABLE
    if (proc->seal_leak_shown) {
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

static void pump_seal_leak_do_jump(ui_proc_pump_t *proc)
{
    if (!pump_seal_leak_can_jump(proc)) {
        return;
    }

    LOG_WM("severe leak detected, jump to leak ui.");
    proc->seal_tip = PUMP_SEAL_TIP_NONE;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
    proc->seal_leak_page_index = PUMP_SEAL_LEAK_PAGE_CHECK_TUBING;
    pump_seal_leak_pause_pump(proc);
    show_pump_seal_leak_ui(proc);
}

static void pump_seal_leak_trigger_ui_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    pump_seal_leak_do_jump(proc);
}

static void pump_seal_result_tip_hide_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->seal_tip == PUMP_SEAL_TIP_RESULT) {
        proc->seal_tip = PUMP_SEAL_TIP_NONE;
        LOG_WM("seal check passed tip hide.");
    }
}

static void pump_seal_result_apply_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;
    uint8_t result;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (!PUMP_IS_GOING(proc->pump_status)) {
        return;
    }

    result = proc->seal_result;
    if (proc->seal_tip == PUMP_SEAL_TIP_CHECKING && result != PUMP_SEAL_RESULT_NONE) {
        if (result == PUMP_SEAL_RESULT_NO_LEAK) {
            proc->seal_tip = PUMP_SEAL_TIP_RESULT;
            proc->seal_result_time = get_timestamp();
            REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
            POST_WM_TASK_HANDLER_DELAYED_ARG1(pump_seal_result_tip_hide_cb,
                PUMP_SEAL_RESULT_TIMEOUT_MS, proc, PUMP_SEAL_RESULT_TIP_TASK_NAME);
            LOG_WM("seal check passed, show result 3s.");
        } else {
            proc->seal_tip = PUMP_SEAL_TIP_NONE;
            REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
            LOG_WM("seal check leave checking, result:%d", result);
        }
    }

    if (result == PUMP_SEAL_RESULT_SEVERE) {
        pump_seal_leak_do_jump(proc);
    }
}

#if PUMP_SEAL_LEAK_MOCK_ENABLE
static uint8_t pump_seal_leak_mock_channel_mask(void)
{
    s_seal_leak_mock_seed = (uint8_t)(s_seal_leak_mock_seed * 33u + (uint8_t)get_timestamp());
    return (uint8_t)((s_seal_leak_mock_seed % 3u) + 1u);
}

static void pump_seal_leak_mock_fill_wear(uint8_t *wear)
{
    uint8_t leak;
    uint8_t mask;
    uint8_t i;

    switch (s_seal_leak_mock_step) {
        case PUMP_SEAL_LEAK_MOCK_STEP_SEVERE:
            leak = ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK;
            break;
        case PUMP_SEAL_LEAK_MOCK_STEP_SLIGHT:
            leak = ALGO_WEAR_DETECTION_RESULT_SMALL_LEAK;
            break;
        case PUMP_SEAL_LEAK_MOCK_STEP_INVALID:
            leak = ALGO_WEAR_DETECTION_RESULT_INVALID;
            break;
        default:
            leak = ALGO_WEAR_DETECTION_RESULT_NORMAL;
            break;
    }

    if (leak == ALGO_WEAR_DETECTION_RESULT_NORMAL ||
        leak == ALGO_WEAR_DETECTION_RESULT_INVALID) {
        for (i = 0; i < AIR_PRESSURE_NUM; i++) {
            wear[i] = leak;
        }
        return;
    }

    mask = pump_seal_leak_mock_channel_mask();
    for (i = 0; i < AIR_PRESSURE_NUM; i++) {
        wear[i] = (mask & (uint8_t)(1u << i)) ? leak : ALGO_WEAR_DETECTION_RESULT_NORMAL;
    }
}

static void pump_seal_leak_mock_schedule(ui_proc_pump_t *proc)
{
    uint32_t interval_ms = (s_seal_leak_mock_step == PUMP_SEAL_LEAK_MOCK_STEP_INVALID) ?
        PUMP_SEAL_LEAK_MOCK_INVALID_INTERVAL_MS : PUMP_SEAL_LEAK_MOCK_INTERVAL_MS;

    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_LEAK_MOCK_TASK_NAME);
    POST_WM_TASK_HANDLER_DELAYED_ARG1(pump_seal_leak_mock_tick_cb,
        interval_ms, proc, PUMP_SEAL_LEAK_MOCK_TASK_NAME);
}

static void pump_seal_leak_mock_tick_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;
    uint8_t wear[AIR_PRESSURE_NUM];
    uint8_t new_result;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    pump_seal_leak_mock_schedule(proc);

    if (!PUMP_IS_GOING(proc->pump_status) ||
        proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK ||
        proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        return;
    }

    pump_seal_leak_mock_fill_wear(wear);
    new_result = pump_seal_result_from_wear(proc, wear);
    pump_fit_not_detected_on_wear(proc, wear);
    s_seal_leak_mock_step = (uint8_t)((s_seal_leak_mock_step + 1u) % PUMP_SEAL_LEAK_MOCK_STEP_MAX);
    if (new_result == proc->seal_result) {
        return;
    }
    proc->seal_result = new_result;
    LOG_WM("seal mock result: %d(%d/%d)", new_result, wear[0], wear[1]);
    POST_WM_TASK_HANDLER_NO_DELAY(pump_seal_result_apply_cb, proc, PUMP_SEAL_LEAK_TASK_NAME);
}
#endif

static void pump_seal_leak_callback(void *user_data, algo_breast_pump_data_t *result)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;
#if !PUMP_SEAL_LEAK_MOCK_ENABLE
    uint8_t new_result;
#endif

    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
#if PUMP_SEAL_LEAK_MOCK_ENABLE
    (void)result;
    return;
#else
    if (result == NULL || !PUMP_IS_GOING(proc->pump_status)) {
        return;
    }

    new_result = pump_seal_result_from_wear(proc, result->wear_detection.result);
    pump_fit_not_detected_on_wear(proc, result->wear_detection.result);
    if (new_result == proc->seal_result) {
        return;
    }
    proc->seal_result = new_result;
    LOG_WM("seal result: %d(%d/%d)", new_result, result->wear_detection.result[0], result->wear_detection.result[1]);
    POST_WM_TASK_HANDLER_NO_DELAY(pump_seal_result_apply_cb, proc, PUMP_SEAL_LEAK_TASK_NAME);
#endif
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_seal_leak_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui;

    EXCEPTION_CHECK_VOID(proc);
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK) {
        return;
    }
    if (proc->seal_leak_page_index >= PUMP_SEAL_LEAK_PAGE_MAX) {
        proc->seal_leak_page_index = PUMP_SEAL_LEAK_PAGE_CHECK_TUBING;
    }
    pump_ui_nav_push(proc);
    pump_ui_change_hdr_remove(proc);

    ui_play_info_t play_info = {
        .tag = "pump_seal_leak_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .progress_cb = pump_seal_leak_animation_progress_cb,
        .display_prepare_cb = pump_seal_leak_ui_display_prepare,
        .display_remove_cb = pump_seal_leak_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PUMP_UI_TYPE_SEAL_LEAK;
    proc->seal_leak_shown = 1;
    proc->pump_light_status = PUMP_LIGHT_STATUS_DEFAULT;
    pump_seal_leak_led_tick(proc);
    LOG_WM("show pump seal leak ui, page:%d", proc->seal_leak_page_index);
}

void pump_seal_leak_ui_exit(ui_proc_pump_t *proc)
{
    if (proc != NULL && proc->pump_light_status == PUMP_LIGHT_STATUS_SEAL_LEAK) {
        pump_seal_leak_led_restore(proc);
    }
}

int pump_seal_leak_page_adjust(ui_proc_pump_t *proc, bool is_left)
{
    uint16_t next_index;

    EXCEPTION_CHECK_INT(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return -1;
    }
    if (is_left) {
        if (proc->seal_leak_page_index == PUMP_SEAL_LEAK_PAGE_CHECK_TUBING) {
            return -1;
        }
        next_index = (uint16_t)(proc->seal_leak_page_index - 1);
    } else {
        if (proc->seal_leak_page_index >= (PUMP_SEAL_LEAK_PAGE_MAX - 1)) {
            return -1;
        }
        next_index = (uint16_t)(proc->seal_leak_page_index + 1);
    }
    proc->seal_leak_page_index = next_index;
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
    pump_pause_idle_touch(proc);
#endif
    LOG_WM("seal leak page -> %d", next_index);
    return 0;
}

bool pump_seal_leak_restrict_keys(const ui_proc_pump_t *proc)
{
    return proc != NULL && proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK;
}

void pump_seal_leak_power_start_click(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return;
    }
    if (proc->seal_leak_page_index < PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE) {
        pump_seal_leak_page_adjust(proc, false);
        return;
    }
    pump_seal_leak_resume(proc);
}

void pump_seal_leak_led_tick(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->led && proc->led->set_led_level);
    if (proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return;
    }

    if (proc->pump_light_status != PUMP_LIGHT_STATUS_SEAL_LEAK) {
        pump_seal_leak_led_all_off(proc);
        proc->pump_light_status = PUMP_LIGHT_STATUS_SEAL_LEAK;
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
    if (get_timestamp_ms(proc->blink_start_time) < PUMP_SEAL_LEAK_BLINK_PERIOD_MS) {
        return;
    }
    proc->blink_start_time = get_timestamp();
    proc->state_blink_visible = proc->state_blink_visible ? 0 : 1;
    proc->led->set_led_level(PUMP_START_BL,
        proc->state_blink_visible ? LED_LEVEL_MAX : 0);
    proc->pump_pause_blink_state = proc->state_blink_visible;
#endif
}

void pump_seal_leak_resume(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return;
    }
    pump_seal_leak_led_restore(proc);
    pump_start_pump_keep_ui(proc);
    pump_ui_nav_back(proc, PUMP_UI_NAV_BACK_HOME_RESET_PAGE);
}

void pump_seal_leak_jump_severe(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    POST_WM_TASK_HANDLER_NO_DELAY(pump_seal_leak_trigger_ui_cb, proc, PUMP_SEAL_LEAK_TASK_NAME);
}

void pump_seal_leak_init(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->algo && proc->algo->add_algo_result_listener);
    proc->seal_result = PUMP_SEAL_RESULT_NONE;
    memset(proc->leak_channel, 0, sizeof(proc->leak_channel));
    pump_fit_not_detected_reset(proc);
    proc->algo->add_algo_result_listener(proc, pump_seal_leak_callback);
#if PUMP_SEAL_LEAK_MOCK_ENABLE
    s_seal_leak_mock_step = PUMP_SEAL_LEAK_MOCK_STEP_SEVERE;
    s_seal_leak_mock_seed = (uint8_t)get_timestamp();
    pump_seal_leak_mock_schedule(proc);
    LOG_WM("seal leak mock enable, interval:%dms", PUMP_SEAL_LEAK_MOCK_INTERVAL_MS);
#endif
}

void pump_seal_leak_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->algo && proc->algo->remove_algo_result_listener);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_LEAK_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
    pump_fit_not_detected_reset(proc);
#if PUMP_SEAL_LEAK_MOCK_ENABLE
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_LEAK_MOCK_TASK_NAME);
#endif
    proc->algo->remove_algo_result_listener(pump_seal_leak_callback);
}

#else /* !IS_SUPPORT_DEVICE_AIR_PRESSURE：v3 无漏气检测 */

void show_pump_seal_leak_ui(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_seal_leak_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;
}

int pump_seal_leak_page_adjust(ui_proc_pump_t *proc, bool is_left)
{
    (void)proc;
    (void)is_left;
    return -1;
}

void pump_seal_leak_resume(ui_proc_pump_t *proc)
{
    (void)proc;
}

bool pump_seal_leak_restrict_keys(const ui_proc_pump_t *proc)
{
    (void)proc;
    return false;
}

void pump_seal_leak_power_start_click(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_seal_leak_led_tick(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_seal_leak_jump_severe(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_seal_leak_init(ui_proc_pump_t *proc)
{
    (void)proc;
}

void pump_seal_leak_exit(ui_proc_pump_t *proc)
{
    (void)proc;
}

#endif /* IS_SUPPORT_DEVICE_AIR_PRESSURE */
