/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
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

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

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
    pump_pause_led_tick(proc);
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

    /// wear_status: 0 无效, 1 无漏气, 2 微小漏气, 3 强漏气；两侧取更差一侧
    for (i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (wear[i] > worst && wear[i] <= ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK) {
            worst = wear[i];
        }
    }

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

static void pump_seal_leak_trigger_ui_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_VOID(args);
    proc = (ui_proc_pump_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (!pump_seal_leak_can_jump(proc)) {
        return;
    }

    LOG_WM("severe leak detected, jump to leak ui.");
    proc->seal_leak_page_index = PUMP_SEAL_LEAK_PAGE_CHECK_FIT;
    pump_seal_leak_pause_pump(proc);
    show_pump_seal_leak_ui(proc);
}

static void pump_seal_leak_callback(void *user_data, algo_breast_pump_data_t *result)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;

    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (result == NULL) {
        return;
    }
    /// 检测窗口内持续刷新结果，10s 到点由 pump_seal_check_tick 消费
    proc->seal_result = pump_seal_result_from_wear(proc, result->wear_detection.result);
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
        proc->seal_leak_page_index = PUMP_SEAL_LEAK_PAGE_CHECK_FIT;
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
    LOG_WM("show pump seal leak ui, page:%d", proc->seal_leak_page_index);
}

void pump_seal_leak_ui_exit(ui_proc_pump_t *proc)
{
    (void)proc;

}

int pump_seal_leak_page_adjust(ui_proc_pump_t *proc, bool is_left)
{
    uint16_t next_index;

    EXCEPTION_CHECK_INT(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return -1;
    }
    if (is_left) {
        if (proc->seal_leak_page_index == PUMP_SEAL_LEAK_PAGE_CHECK_FIT) {
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
    if (proc == NULL || proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return false;
    }
    return proc->seal_leak_page_index < PUMP_SEAL_LEAK_PAGE_PAUSE;
}

void pump_seal_leak_resume(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->current_ui_type != PUMP_UI_TYPE_SEAL_LEAK) {
        return;
    }
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
    proc->algo->add_algo_result_listener(proc, pump_seal_leak_callback);
}

void pump_seal_leak_exit(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->algo && proc->algo->remove_algo_result_listener);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SEAL_LEAK_TASK_NAME);
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
