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
#define PUMP_SEAL_LEAK_MOCK_ENABLE                      (0) /// 1: 模拟漏气/未佩戴，0: 走算法
#endif
#ifndef IS_SUPPORT_PUMP_SEAL_LEAK_UI_BOTH_SIDES
#define IS_SUPPORT_PUMP_SEAL_LEAK_UI_BOTH_SIDES         (0) /// 1: 任一侧漏气按双侧展示
#endif
#ifndef IS_SUPPORT_PUMP_SEAL_SLIGHT_AS_PASS
#define IS_SUPPORT_PUMP_SEAL_SLIGHT_AS_PASS             (1) /// 1: 轻微漏气展示通过；0: 不展示
#endif
#define PUMP_SEAL_LEAK_BLINK_PERIOD_MS                  (1000) /// 半周期 1000ms → 0.5Hz
#if PUMP_SEAL_LEAK_MOCK_ENABLE
#define PUMP_SEAL_LEAK_MOCK_INJECT_DELAY_MS             (1500) /// 先展示 Checking seal... 再灌结果
#define PUMP_SEAL_LEAK_MOCK_TASK_NAME                   "pump_seal_leak_mock"
/// 暂停再启动切下一步。0-5 接续 Checking；6-9 结束检测后走吸奶动画。
#define PUMP_SEAL_LEAK_MOCK_CHK_NO_LEAK                 (0)
#define PUMP_SEAL_LEAK_MOCK_CHK_SLIGHT                  (1)
#define PUMP_SEAL_LEAK_MOCK_CHK_SEVERE_FIRST            (2)
#define PUMP_SEAL_LEAK_MOCK_CHK_SEVERE_AGAIN            (3)
#define PUMP_SEAL_LEAK_MOCK_CHK_OVT_FIRST               (4)
#define PUMP_SEAL_LEAK_MOCK_CHK_OVT_AGAIN               (5)
#define PUMP_SEAL_LEAK_MOCK_IDLE_SEVERE_AGAIN           (6)
#define PUMP_SEAL_LEAK_MOCK_IDLE_OVT_AGAIN              (7)
#define PUMP_SEAL_LEAK_MOCK_IDLE_SEVERE_FIRST           (8)
#define PUMP_SEAL_LEAK_MOCK_IDLE_OVT_FIRST              (9)
#define PUMP_SEAL_LEAK_MOCK_STEP_MAX                    (10)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if PUMP_SEAL_LEAK_MOCK_ENABLE
static uint8_t s_seal_leak_mock_step;
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

static void pump_seal_leak_channel_apply_ui_policy(ui_proc_pump_t *proc)
{
#if IS_SUPPORT_PUMP_SEAL_LEAK_UI_BOTH_SIDES
    if (proc->leak_channel[0] || proc->leak_channel[1]) {
        memset(proc->leak_channel, 1, sizeof(proc->leak_channel));
    }
#endif
    /// 锁存后仍无通道：按双侧漏气展示，避免 L/R 都未选中
    if (!proc->leak_channel[0] && !proc->leak_channel[1]) {
        memset(proc->leak_channel, 1, sizeof(proc->leak_channel));
    }
}

static void pump_seal_leak_channel_latch(ui_proc_pump_t *proc)
{
    if (proc->leak_channel_latched) {
        return;
    }
    pump_seal_leak_channel_apply_ui_policy(proc);
    proc->leak_channel_latched = 1;
    LOG_WM("leak channel latch:%d/%d", proc->leak_channel[0], proc->leak_channel[1]);
}

static void pump_seal_leak_channel_unlatch(ui_proc_pump_t *proc)
{
    proc->leak_channel_latched = 0;
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
        if (!proc->leak_channel_latched) {
            proc->leak_channel[i] = (wear[i] == ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK);
        }
        if (wear[i] > worst && wear[i] <= ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK) {
            worst = wear[i];
        }
    }
    if (!proc->leak_channel_latched && worst == ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK) {
        /// 首次强漏气即锁存，避免跳转前被后续帧覆盖成 [0,0]
        pump_seal_leak_channel_latch(proc);
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

    if (proc->seal_leak_shown) {
        return false;
    }

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
    proc->seal_result_time = 0;
    proc->seal_leak_page_index = PUMP_SEAL_LEAK_PAGE_CHECK_TUBING;
    pump_seal_leak_channel_latch(proc);
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

static void pump_seal_result_tip_hide_cb(hdr_post_args_t *args);

static void pump_seal_result_tip_show(ui_proc_pump_t *proc, uint8_t tip, uint8_t result)
{
    if (proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK ||
        proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        proc->seal_tip = PUMP_SEAL_TIP_NONE;
        return;
    }
    /// 通过/补偿只能接续 Checking seal...，吸奶动画中不得单独出现
    if ((tip == PUMP_SEAL_TIP_RESULT || tip == PUMP_SEAL_TIP_RESULT_FAIL) &&
        proc->seal_tip != PUMP_SEAL_TIP_CHECKING) {
        LOG_WM("seal tip %d ignored, not checking.", tip);
        return;
    }
    proc->seal_tip = tip;
    proc->seal_result_time = get_timestamp();
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
    POST_WM_TASK_HANDLER_DELAYED_ARG1(pump_seal_result_tip_hide_cb,
        PUMP_SEAL_RESULT_TIMEOUT_MS, proc, PUMP_SEAL_RESULT_TIP_TASK_NAME);
    LOG_WM("seal check result:%d, show tip %d 3s.", result, tip);
}

static void pump_seal_result_tip_hide_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->seal_tip != PUMP_SEAL_TIP_RESULT &&
        proc->seal_tip != PUMP_SEAL_TIP_RESULT_FAIL) {
        return;
    }
    proc->seal_tip = PUMP_SEAL_TIP_NONE;
    LOG_WM("seal check result tip hide.");
}

static void pump_seal_result_apply_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;
    uint8_t result;
    bool checking;
    bool worn_ovt;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (!PUMP_IS_GOING(proc->pump_status)) {
        return;
    }
    if (proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK ||
        proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        proc->seal_tip = PUMP_SEAL_TIP_NONE;
        return;
    }

    result = proc->seal_result;
    checking = (proc->seal_tip == PUMP_SEAL_TIP_CHECKING);
    worn_ovt = pump_fit_not_detected_is_not_worn(proc->weared_status);

    /// 检测中：超时/大漏优先于通过；补偿 tip 仅检测中。检测外超时已展示过则继续看大漏。
    if (worn_ovt) {
        if (pump_fit_not_detected_try_jump(proc)) {
            LOG_WM("seal resolve: fit not detected.");
            return;
        }
        if (checking) {
            pump_seal_result_tip_show(proc, PUMP_SEAL_TIP_RESULT_FAIL, result);
            LOG_WM("seal resolve: worn ovt already shown, compensating.");
            return;
        }
    }
    if (result == PUMP_SEAL_RESULT_SEVERE) {
        if (pump_seal_leak_can_jump(proc)) {
            pump_seal_leak_do_jump(proc);
            LOG_WM("seal resolve: severe leak ui.");
            return;
        }
        if (checking) {
            pump_seal_result_tip_show(proc, PUMP_SEAL_TIP_RESULT_FAIL, result);
            LOG_WM("seal resolve: severe already shown, compensating.");
        } else {
            pump_seal_leak_channel_unlatch(proc);
        }
        return;
    }
    if (!checking || result == PUMP_SEAL_RESULT_NONE) {
        return;
    }
#if !IS_SUPPORT_PUMP_SEAL_SLIGHT_AS_PASS
    if (result == PUMP_SEAL_RESULT_SLIGHT) {
        proc->seal_tip = PUMP_SEAL_TIP_NONE;
        proc->seal_result_time = 0;
        REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
        LOG_WM("seal resolve: slight leak, skip pass.");
        return;
    }
#endif
    pump_seal_result_tip_show(proc, PUMP_SEAL_TIP_RESULT, result);
    LOG_WM("seal resolve: pass.");
}

static void pump_seal_ingest_algo(ui_proc_pump_t *proc, const uint8_t *leakage, const uint8_t *weared)
{
    bool wear_changed;
    uint8_t new_result;

    EXCEPTION_CHECK_VOID(proc && leakage && weared);
    wear_changed = (memcmp(proc->weared_status, weared, sizeof(proc->weared_status)) != 0);
    memcpy(proc->weared_status, weared, sizeof(proc->weared_status));
    new_result = pump_seal_result_from_wear(proc, leakage);
    if (new_result == proc->seal_result && !wear_changed) {
        /// CHECKING 仅在已有有效结果时允许同值再投，避免 NONE 帧按算法频率刷 WM 任务
        if (proc->seal_tip != PUMP_SEAL_TIP_CHECKING ||
            new_result == PUMP_SEAL_RESULT_NONE) {
            return;
        }
    }
    proc->seal_result = new_result;
    LOG_WM("seal ingest:%d weared:%d/%d tip:%d shown:%d/%d", new_result,
        weared[0], weared[1], proc->seal_tip,
        proc->fit_not_detected_shown, proc->seal_leak_shown);
    POST_WM_TASK_HANDLER_NO_DELAY(pump_seal_result_apply_cb, proc, PUMP_SEAL_LEAK_TASK_NAME);
}

#if PUMP_SEAL_LEAK_MOCK_ENABLE
static void pump_seal_leak_mock_fill(uint8_t leak, bool worn_ovt, uint8_t *leakage, uint8_t *weared)
{
    uint8_t i;

    for (i = 0; i < AIR_PRESSURE_NUM; i++) {
        leakage[i] = leak;
        weared[i] = worn_ovt ? ALGO_WEARED_STATUS_WORN_OVT : ALGO_WEARED_STATUS_WORN;
    }
}

static const char *pump_seal_leak_mock_expect(uint8_t step)
{
    switch (step) {
        case PUMP_SEAL_LEAK_MOCK_CHK_NO_LEAK:
            return "1.1 checking no_leak -> pass";
        case PUMP_SEAL_LEAK_MOCK_CHK_SLIGHT:
#if IS_SUPPORT_PUMP_SEAL_SLIGHT_AS_PASS
            return "1.1 checking slight -> pass";
#else
            return "1.1 checking slight -> skip pass";
#endif
        case PUMP_SEAL_LEAK_MOCK_CHK_SEVERE_FIRST:
            return "1.2 checking severe first -> leak ui";
        case PUMP_SEAL_LEAK_MOCK_CHK_SEVERE_AGAIN:
            return "1.2 checking severe shown -> compensate";
        case PUMP_SEAL_LEAK_MOCK_CHK_OVT_FIRST:
            return "1.3 checking ovt first -> fit ui";
        case PUMP_SEAL_LEAK_MOCK_CHK_OVT_AGAIN:
            return "1.3 checking ovt shown -> compensate";
        case PUMP_SEAL_LEAK_MOCK_IDLE_SEVERE_AGAIN:
            return "2.1 idle severe shown -> nop";
        case PUMP_SEAL_LEAK_MOCK_IDLE_OVT_AGAIN:
            return "2.2 idle ovt shown -> nop";
        case PUMP_SEAL_LEAK_MOCK_IDLE_SEVERE_FIRST:
            return "2.1 idle severe first -> leak ui";
        case PUMP_SEAL_LEAK_MOCK_IDLE_OVT_FIRST:
            return "2.2 idle ovt first -> fit ui";
        default:
            return "unknown";
    }
}

static void pump_seal_leak_mock_prepare(ui_proc_pump_t *proc, uint8_t step,
    uint8_t *leakage, uint8_t *weared)
{
    /// 0-5：保持启动时的 Checking；6-9：结束检测，回到吸奶动画再灌结果
    if (step >= PUMP_SEAL_LEAK_MOCK_IDLE_SEVERE_AGAIN) {
        pump_seal_check_stop(proc);
    }

    switch (step) {
        case PUMP_SEAL_LEAK_MOCK_CHK_NO_LEAK:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_NORMAL, false, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_CHK_SLIGHT:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_SMALL_LEAK, false, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_CHK_SEVERE_FIRST:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK, false, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_CHK_SEVERE_AGAIN:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK, false, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_CHK_OVT_FIRST:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_NORMAL, true, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_CHK_OVT_AGAIN:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_NORMAL, true, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_IDLE_SEVERE_AGAIN:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK, false, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_IDLE_OVT_AGAIN:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_NORMAL, true, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_IDLE_SEVERE_FIRST:
            proc->seal_leak_shown = 0;
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK, false, leakage, weared);
            break;
        case PUMP_SEAL_LEAK_MOCK_IDLE_OVT_FIRST:
            proc->fit_not_detected_shown = 0;
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_NORMAL, true, leakage, weared);
            break;
        default:
            pump_seal_leak_mock_fill(ALGO_WEAR_DETECTION_RESULT_NORMAL, false, leakage, weared);
            break;
    }
}

static void pump_seal_leak_mock_schedule(ui_proc_pump_t *proc)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_LEAK_MOCK_TASK_NAME);
    POST_WM_TASK_HANDLER_DELAYED_ARG1(pump_seal_leak_mock_tick_cb,
        PUMP_SEAL_LEAK_MOCK_INJECT_DELAY_MS, proc, PUMP_SEAL_LEAK_MOCK_TASK_NAME);
}

static void pump_seal_leak_mock_tick_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;
    uint8_t leakage[AIR_PRESSURE_NUM];
    uint8_t weared[AIR_PRESSURE_NUM];
    uint8_t step;
    bool checking_step;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);

    if (!PUMP_IS_GOING(proc->pump_status) ||
        proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK ||
        proc->current_ui_type == PUMP_UI_TYPE_FIT_NOT_DETECTED) {
        LOG_WM("[seal mock] wait step=%d/%d expect:%s ui=%d going=%d (pause/start to continue)",
            s_seal_leak_mock_step, PUMP_SEAL_LEAK_MOCK_STEP_MAX - 1,
            pump_seal_leak_mock_expect(s_seal_leak_mock_step),
            proc->current_ui_type, PUMP_IS_GOING(proc->pump_status));
        return;
    }

    step = s_seal_leak_mock_step;
    checking_step = (step <= PUMP_SEAL_LEAK_MOCK_CHK_OVT_AGAIN);
    if (checking_step && proc->seal_tip != PUMP_SEAL_TIP_CHECKING) {
        LOG_WM("[seal mock] skip step=%d, not checking (tip=%d)", step, proc->seal_tip);
        return;
    }

    pump_seal_leak_mock_prepare(proc, step, leakage, weared);
    LOG_WM("[seal mock] step=%d/%d expect:%s leak:%d/%d weared:%d/%d shown:%d/%d tip:%d",
        step, PUMP_SEAL_LEAK_MOCK_STEP_MAX - 1, pump_seal_leak_mock_expect(step),
        leakage[0], leakage[1], weared[0], weared[1],
        proc->fit_not_detected_shown, proc->seal_leak_shown, proc->seal_tip);
    s_seal_leak_mock_step = (uint8_t)((step + 1u) % PUMP_SEAL_LEAK_MOCK_STEP_MAX);
    if (s_seal_leak_mock_step == 0) {
        LOG_WM("[seal mock] round done, pause/start again for step 0");
    }
    pump_seal_ingest_algo(proc, leakage, weared);
}
#endif

void pump_seal_leak_mock_on_check_start(ui_proc_pump_t *proc)
{
#if PUMP_SEAL_LEAK_MOCK_ENABLE
    EXCEPTION_CHECK_VOID(proc);
    LOG_WM("[seal mock] start -> next inject step=%d/%d expect:%s in %dms",
        s_seal_leak_mock_step, PUMP_SEAL_LEAK_MOCK_STEP_MAX - 1,
        pump_seal_leak_mock_expect(s_seal_leak_mock_step),
        PUMP_SEAL_LEAK_MOCK_INJECT_DELAY_MS);
    pump_seal_leak_mock_schedule(proc);
#else
    (void)proc;
#endif
}

static void pump_seal_leak_callback(void *user_data, algo_breast_pump_data_t *result)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;

    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
#if PUMP_SEAL_LEAK_MOCK_ENABLE
    (void)result;
    return;
#else
    if (result == NULL || !PUMP_IS_GOING(proc->pump_status)) {
        return;
    }

    pump_seal_ingest_algo(proc, result->wear_detection.result,
        result->wear_detection.wear_result);
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
    proc->seal_tip = PUMP_SEAL_TIP_NONE;
    proc->seal_result_time = 0;
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_RESULT_TIP_TASK_NAME);
    if (proc->servers && proc->servers->set_app_ctrl_block) {
        proc->servers->set_app_ctrl_block(SERVERS_APP_CTRL_BLOCK_SEAL_LEAK);
    }
    ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
    if (proc->current_ui_type == PUMP_UI_TYPE_SEAL_LEAK) {
        return;
    }
    pump_seal_leak_channel_latch(proc);
    proc->seal_leak_page_index = PUMP_SEAL_LEAK_PAGE_CHECK_TUBING;
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
    if (proc && proc->servers && proc->servers->clear_app_ctrl_block) {
        proc->servers->clear_app_ctrl_block(SERVERS_APP_CTRL_BLOCK_SEAL_LEAK);
    }
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
    if (proc->servers && proc->servers->clear_app_ctrl_block) {
        proc->servers->clear_app_ctrl_block(SERVERS_APP_CTRL_BLOCK_SEAL_LEAK);
    }
    if (proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return;
    }
    pump_seal_leak_led_restore(proc);
    pump_seal_leak_channel_unlatch(proc);
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
    pump_seal_leak_channel_unlatch(proc);
    pump_fit_not_detected_reset(proc);
    proc->algo->add_algo_result_listener(proc, pump_seal_leak_callback);
#if PUMP_SEAL_LEAK_MOCK_ENABLE
    s_seal_leak_mock_step = PUMP_SEAL_LEAK_MOCK_CHK_NO_LEAK;
    LOG_WM("[seal mock] enable, pause then start to next step, grep '[seal mock]'");
#endif
}

void pump_seal_leak_exit(ui_proc_pump_t *proc)
{
    /// 窗口销毁时统一清除阻断门控，避免残留导致 App 控制被永久阻断
    if (proc && proc->servers && proc->servers->clear_app_ctrl_block) {
        proc->servers->clear_app_ctrl_block(SERVERS_APP_CTRL_BLOCK_SEAL_LEAK
            | SERVERS_APP_CTRL_BLOCK_FIT_NOT_DETECTED);
    }
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

void pump_seal_leak_mock_on_check_start(ui_proc_pump_t *proc)
{
    (void)proc;
}

#endif /* IS_SUPPORT_DEVICE_AIR_PRESSURE */
