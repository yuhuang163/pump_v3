/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "adaptive_dimming.h" // IWYU pragma: keep
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
#include "input_key_code.h" // IWYU pragma: keep
#include "key_input_manager_api.h" // IWYU pragma: keep
#endif

/*
* DEFINES
*****************************************************************************************
*/
#define ADAP_DIM_INVALID_DUTY                       255            /// 无效的LED亮度等级
#define ADAPTIVE_DIMMING_DEBUG_LOG                  (1)            /// 自适应背光调试日志 0: 不打印 1: 打印

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t last_lux_for_duty = 0;           /// 上一次计算duty时的lux值
static adaptive_dimming_info_t *adaptive = NULL;
static uint16_t light_sensor_data[10] = {0};
static uint8_t s_light_report_batch_count = 0;
static uint16_t s_lux_filtered = 0;
static uint8_t s_lux_filt_inited = 0;
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
/** 旋钮屏蔽剩余 MISC 次数，由 adaptive_dimming_notify_encoder_activity 置为 ADAP_DIM_ENCODER_SUPPRESS_TICKS */
static uint8_t s_encoder_suppress_ticks = 0;
#endif
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
/*
 * 光感「明显变化」稳定确认：与 duty_calculate 内 last_lux 独立；计数均在 MISC 轮询线程内推进，不用系统时间。
 */
static uint16_t s_last_applied_lux = 0;          ///< 最近一次夜灯+UI 自适应亮度已成功下发时对应的滤波后 lux；0 表示尚未有过生效输出
static uint8_t s_lux_settling = 0;               ///< 1：当前处于「相对 s_last_applied_lux 明显变化」后的待确认阶段
static uint16_t s_lux_settle_candidate = 0;      ///< 明显变化后锁定的候选 lux，与每批新值在 ±EPS 内比较
static uint16_t s_lux_stable_misc_ticks = 0;     ///< 在 settling 且与候选一致前提下，adaptive_dimming_lux_stable_misc_tick 已累加的 MISC 次数
static uint8_t s_lux_misc_stable_gate_ready = 0; ///< 1：misc 周期已达 ADAP_DIM_LUX_STABLE_MISC_TICKS，本批可在 gate 内判通过
static uint16_t s_lux_last_batch_lux = 0;        ///< 上一批已算出的 lux_for_duty，供 misc_tick 在无新批次时仍按 200ms 推进稳定计数
static uint8_t s_lux_last_batch_valid = 0;       ///< 是否已有至少一批有效 lux 写入 s_lux_last_batch_lux
#endif
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void light_sensor_report_accumulator_reset(void)
{
    s_light_report_batch_count = 0;
    memset(light_sensor_data, 0, sizeof(light_sensor_data));
}

static void on_light_sensor_data_cb(void *user_data, uint8_t *data, int data_len)
{
    EXCEPTION_CHECK_VOID(adaptive);
    EXCEPTION_CHECK_VOID(user_data);
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)data;
    if ( adaptive->is_sampling) {
        //上报
        light_sensor_data_report(data);
        return;
    }
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR
    if (adaptive->capture_data.is_acquire_complete == 0 && 
        adaptive->capture_data.index < ADAP_DIM_DATA_SIZE) {
        adaptive->capture_data.buff[adaptive->capture_data.index] = sensor_data->light_sensor_data.lux_level;
        adaptive->capture_data.index++;
        if (adaptive->capture_data.index >= ADAP_DIM_DATA_SIZE) {
            adaptive->capture_data.is_acquire_complete = 1;
            adaptive->capture_data.index = 0;
            // LOG_MISC("capture data complete");
        }
    }
#endif
}

static int adaptive_dimming_duty_calculate(uint16_t lux_avg, uint8_t *out_duty)
{
    uint8_t new_duty = 0;
    /// 限制 lux 值在有效范围内
    if (lux_avg > ADAP_DIM_LUX_MAX) {
        lux_avg = ADAP_DIM_LUX_MAX;
    }
    
    /// 计算滞后阈值，避免在临界值附近来回切换
    const uint16_t hysteresis_threshold = (lux_avg > ADAP_DIM_HYST_THR_BASE) ? 
                                         (lux_avg / ADAP_DIM_HYST_THR_RATIO) : 
                                         ADAP_DIM_HYST_THR_MIN;
    
    /// 根据 lux 值分段计算 duty（环境越亮，LED 越暗）
    if (lux_avg < ADAP_DIM_LUX_THR_VDARK) {
        new_duty = ADAP_DIM_LED_LVL_VDARK;
    } else if (lux_avg < ADAP_DIM_LUX_THR_DARK) {
        /// 线性映射: lux 10->100, lux 50->80
        new_duty = ADAP_DIM_LED_LVL_VDARK + 
                   (lux_avg - ADAP_DIM_LUX_THR_VDARK) * 
                   (ADAP_DIM_LED_LVL_DARK_MIN - ADAP_DIM_LED_LVL_VDARK) / 
                   (ADAP_DIM_LUX_THR_DARK - ADAP_DIM_LUX_THR_VDARK);
        if (new_duty > ADAP_DIM_LED_LVL_DARK_MIN) {
            new_duty = ADAP_DIM_LED_LVL_DARK_MIN;
        }
    // if (lux_avg < ADAP_DIM_LUX_THR_DARK) {
    //     new_duty = ADAP_DIM_LED_LVL_DARK_MIN;
    } else if (lux_avg < ADAP_DIM_LUX_THR_MEDIUM) {
        /// 线性映射: lux 50->80, lux 200->50
        new_duty = ADAP_DIM_LED_LVL_DARK_MIN + 
                   (lux_avg - ADAP_DIM_LUX_THR_DARK) * 
                   (ADAP_DIM_LED_LVL_MEDIUM_MIN - ADAP_DIM_LED_LVL_DARK_MIN) / 
                   (ADAP_DIM_LUX_THR_MEDIUM - ADAP_DIM_LUX_THR_DARK);
        if (new_duty > ADAP_DIM_LED_LVL_MEDIUM_MIN) {
            new_duty = ADAP_DIM_LED_LVL_MEDIUM_MIN;
        }
    } else if (lux_avg < ADAP_DIM_LUX_THR_BRIGHT) {
        /// 线性映射: lux 200->50, lux 500->20
        new_duty = ADAP_DIM_LED_LVL_MEDIUM_MIN + 
                   (lux_avg - ADAP_DIM_LUX_THR_MEDIUM) * 
                   (ADAP_DIM_LED_LVL_BRIGHT_MIN - ADAP_DIM_LED_LVL_MEDIUM_MIN) / 
                   (ADAP_DIM_LUX_THR_BRIGHT - ADAP_DIM_LUX_THR_MEDIUM);
        if (new_duty < ADAP_DIM_LED_LVL_BRIGHT_MIN) {
            new_duty = ADAP_DIM_LED_LVL_BRIGHT_MIN;
        }
    // } else if (lux_avg < ADAP_DIM_LUX_THR_VBRIGHT) {
    //     new_duty = ADAP_DIM_LED_LVL_VBRIGHT;
    } else {
        new_duty = ADAP_DIM_LED_LVL_ON;
    }
    
    /// 滞后机制：只有当 lux 值变化超过阈值时才改变亮度，避免闪烁
    if (last_lux_for_duty == 0) {
        last_lux_for_duty = lux_avg;
        *out_duty = new_duty;
        LOG_MISC("lux_avg: %d, new_duty: %d", lux_avg, new_duty);
        return 1;
    } 
    uint16_t lux_diff = (lux_avg > last_lux_for_duty) ? \
            (lux_avg - last_lux_for_duty) : (last_lux_for_duty - lux_avg);
    if (lux_diff >= hysteresis_threshold) {
        LOG_MISC("lux_diff: %d, hysteresis_threshold: %d, new_duty: %d, lux_avg: %d",
                lux_diff, hysteresis_threshold, new_duty, lux_avg);     
        last_lux_for_duty = lux_avg;
        *out_duty = new_duty;
        return 1;
    }
    return 0;
}

/// 对多采样平均后的 lux 做 EMA；环境突变时快速跟踪，避免关灯/开灯后长时间渐变
static uint16_t adaptive_dimming_lux_filter(uint16_t raw_avg)
{
    if (!s_lux_filt_inited) {
        s_lux_filtered = raw_avg;
        s_lux_filt_inited = 1;
        return s_lux_filtered;
    }
    uint16_t diff = (raw_avg > s_lux_filtered) ? (raw_avg - s_lux_filtered)
                                               : (s_lux_filtered - raw_avg);
    if (diff >= ADAP_DIM_LUX_FAST_TRACK_DIFF) {
        s_lux_filtered = raw_avg;
    } else {
        s_lux_filtered = (uint16_t)(((uint32_t)s_lux_filtered * (uint32_t)ADAP_DIM_LUX_EMA_NUM +
                                      (uint32_t)raw_avg) /
                                     (uint32_t)ADAP_DIM_LUX_EMA_DEN);
    }
    return s_lux_filtered;
}

#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
/**
 * @brief 光感稳定 MISC 节拍（与 add_misc_task 同周期，由 polling 每轮调用；与 ADAP_DIM_LUX_STABLE_MISC_TICKS 比较，不用系统时间）。
 *
 * 在 settling 状态下，用「上一批」滤波 lux（s_lux_last_batch_lux）与候选（s_lux_settle_candidate）比较：
 * - 落在 ±ADAP_DIM_LUX_SETTLE_MATCH_EPS 内：s_lux_stable_misc_ticks 加一，达到 ADAP_DIM_LUX_STABLE_MISC_TICKS 后置 ready。
 * - 超出：候选对齐到上一批 lux，周期清零。
 * 每次调用先清 s_lux_misc_stable_gate_ready，再根据上面逻辑可能重新置 1。
 */
static void adaptive_dimming_lux_stable_misc_tick(void)
{
    s_lux_misc_stable_gate_ready = 0;
    if (!s_lux_settling || s_last_applied_lux == 0 || !s_lux_last_batch_valid) {
        return;
    }
    uint16_t dc = s_lux_last_batch_lux > s_lux_settle_candidate
                      ? (s_lux_last_batch_lux - s_lux_settle_candidate)
                      : (s_lux_settle_candidate - s_lux_last_batch_lux);
    if (dc <= ADAP_DIM_LUX_SETTLE_MATCH_EPS) {
        if (s_lux_stable_misc_ticks < 0xFFFFu) {
            s_lux_stable_misc_ticks++;
        }
        if (s_lux_stable_misc_ticks >= ADAP_DIM_LUX_STABLE_MISC_TICKS) {
            s_lux_misc_stable_gate_ready = 1u;
        }
    } else {
        s_lux_settle_candidate = s_lux_last_batch_lux;
        s_lux_stable_misc_ticks = 0;
    }
}

/**
 * @brief 光感「明显变化」门控：通过后才允许继续 duty_calculate 与下发。
 *
 * @param lux_now 本批滤波后的 lux_for_duty。
 * @return 1 可继续调光；0 本批直接放弃下发。
 *
 * - s_last_applied_lux==0：首次生效前不拦。
 * - 与 s_last_applied_lux 差值 < ADAP_DIM_LUX_SIGNIFICANT_DELTA：小变化，清 settling，直接通过。
 * - 否则进入/维持 settling：本批与候选偏差 >EPS 时更新候选并清 misc 计数；偏差 ≤EPS 时仅当 s_lux_misc_stable_gate_ready 为 1 才通过。
 */
static int adaptive_dimming_lux_stable_gate_pass(uint16_t lux_now)
{
    if (s_last_applied_lux == 0) {
        return 1;
    }
    uint16_t diff_from_applied = lux_now > s_last_applied_lux ? (lux_now - s_last_applied_lux)
                                                              : (s_last_applied_lux - lux_now);
    if (diff_from_applied < ADAP_DIM_LUX_SIGNIFICANT_DELTA) {
        s_lux_settling = 0;
        s_lux_stable_misc_ticks = 0;
        s_lux_misc_stable_gate_ready = 0;
        return 1;
    }
    if (!s_lux_settling) {
        s_lux_settling = 1;
        s_lux_settle_candidate = lux_now;
        s_lux_stable_misc_ticks = 0;
        s_lux_misc_stable_gate_ready = 0;
        return 0;
    }
    uint16_t diff_cand = lux_now > s_lux_settle_candidate ? (lux_now - s_lux_settle_candidate)
                                                          : (s_lux_settle_candidate - lux_now);
    if (diff_cand > ADAP_DIM_LUX_SETTLE_MATCH_EPS) {
        s_lux_settle_candidate = lux_now;
        s_lux_stable_misc_ticks = 0;
        s_lux_misc_stable_gate_ready = 0;
        return 0;
    }
    return s_lux_misc_stable_gate_ready ? 1 : 0;
}
#endif /* ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE */

#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
void adaptive_dimming_notify_encoder_activity(void)
{
    s_encoder_suppress_ticks = ADAP_DIM_ENCODER_SUPPRESS_TICKS;
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    s_lux_settling = 0;
    s_lux_stable_misc_ticks = 0;
    s_lux_misc_stable_gate_ready = 0;
#endif
}

static int on_key_input_event_cb(void *user_data, uint16_t keycode, uint16_t keytype)
{
    EXCEPTION_CHECK_INT(user_data);
    (void)keycode;
    (void)keytype;
    adaptive_dimming_notify_encoder_activity();
    return 0;
}
#endif /* ADAP_DIM_USE_ENCODER_KNOB_SHIELD */

static void adaptive_dimming_polling_thread(void *user_data)
{
    led_manager_context_t *led = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(led);
    EXCEPTION_CHECK_VOID(adaptive);
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
    if (s_encoder_suppress_ticks > 0) {
        s_encoder_suppress_ticks--;
        return;
    }
#endif
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    adaptive_dimming_lux_stable_misc_tick();
#endif
    if (adaptive->capture_data.is_acquire_complete == 0) {
        return;
    }
    uint32_t lux_sum = 0, lux_avg = 0;
    for (int i = 0; i < ADAP_DIM_DATA_SIZE; i++) {
        lux_sum += adaptive->capture_data.buff[i];
    }
    adaptive->capture_data.is_acquire_complete = 0; /// 采集完成标志清零
    lux_avg = lux_sum / ADAP_DIM_DATA_SIZE;

    uint16_t lux_for_duty = adaptive_dimming_lux_filter((uint16_t)lux_avg);
#if ADAPTIVE_DIMMING_DEBUG_LOG
    LOG_MISC("lux raw_avg: %lu, filt: %u", (unsigned long)lux_avg, (unsigned)lux_for_duty);
#endif
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    /* 更新「上一批」供 misc_tick 使用，再判明显变化门控；通过后才进入 duty_calculate */
    s_lux_last_batch_lux = lux_for_duty;
    s_lux_last_batch_valid = 1;
    if (!adaptive_dimming_lux_stable_gate_pass(lux_for_duty)) {
        return;
    }
#endif

    uint8_t refresh_duty = 0;
    if (adaptive_dimming_duty_calculate(lux_for_duty, &refresh_duty) == 0) {
        return;
    }

#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
    if (led->hook && led->hook->key_light_adaptive_level_update) {
        led->hook->key_light_adaptive_level_update(led, refresh_duty);
    }
#endif

#if IS_SUPPORT_NIGHT_LIGHT
    if (led->hook && led->hook->night_light_adaptive_level_update) {
        led->hook->night_light_adaptive_level_update(led, refresh_duty);
    }
#endif

    ui_manager_context_t *ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    if (ui && ui->hook) {
        ui->hook->set_ui_backlight_adaptive_level(ui, refresh_duty * 100);
    }
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    /* 下发成功：记录用于「明显变化」比较的基准 lux，并清稳定状态机，便于下一轮从小变化或重新 settling 开始 */
    s_last_applied_lux = lux_for_duty;
    s_lux_settling = 0;
    s_lux_stable_misc_ticks = 0;
    s_lux_misc_stable_gate_ready = 0;
#endif
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void adaptive_dimming_init(void)
{
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
    s_encoder_suppress_ticks = 0;
#endif
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    s_last_applied_lux = 0;
    s_lux_settling = 0;
    s_lux_settle_candidate = 0;
    s_lux_stable_misc_ticks = 0;
    s_lux_misc_stable_gate_ready = 0;
    s_lux_last_batch_lux = 0;
    s_lux_last_batch_valid = 0;
#endif
    adaptive = (adaptive_dimming_info_t *)u_malloz(sizeof(adaptive_dimming_info_t));
    EXCEPTION_CHECK_VOID(adaptive);
    adaptive->is_sampling = 0;
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_VOID(misc);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led);
    misc->add_misc_task(led, MISC_TASK_ID_ADAPTIVE_DIMMING, ADAP_DIM_MISC_POLL_MS, MISC_WORK_MSG_ID_TIMER,
                        adaptive_dimming_polling_thread);
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    EXCEPTION_CHECK_VOID(sensor);
    sensor->add_sensor_evt_lisenser(sensor, LIGHT_SENSOR_EVT_POLLING, 1, on_light_sensor_data_cb);
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
    key_input_context_t *input_key_ctx = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(input_key_ctx);
    input_key_ctx->add_keyevent_handler("adaptive_dimming", input_key_ctx,
                                        KEY_CODE_KNOB_LEFT | KEY_CODE_KNOB_RIGHT,
                                        0xff,
                                        on_key_input_event_cb, KEY_HANDLER_SHIELD_LEVEL_1);
#endif
}

void light_sensor_sampling_start(void)
{
    EXCEPTION_CHECK_VOID(adaptive);
    light_sensor_report_accumulator_reset();
    adaptive->is_sampling = 1;
    LOG_MISC("light sensor sampling start");
}

void light_sensor_sampling_stop(void)
{
    EXCEPTION_CHECK_VOID(adaptive);
    adaptive->is_sampling = 0;
    light_sensor_report_accumulator_reset();
    LOG_MISC("light sensor sampling stop");
}

bool light_sensor_sampling_is_sampling(void)
{
    EXCEPTION_CHECK_BOOL(adaptive);
    return adaptive->is_sampling;
}

bool light_sensor_change_sampling_interval(uint16_t interval)
{
    EXCEPTION_CHECK_BOOL(adaptive);
    if (interval < 10) {
        LOG_MISC("sampling interval error");
        return 0;
    }
    light_sensor_sampling_stop();
    /// 设置采样间隔
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    EXCEPTION_CHECK_BOOL(sensor);
    light_sensor_context_t *light_sensor = (light_sensor_context_t *)sensor->find_sensor(ID_SENSOR_LIGHT_SENSOR);
    EXCEPTION_CHECK_BOOL(light_sensor);
    light_sensor->set_sampling_interval(interval);
    light_sensor_sampling_start();
    return true;
}

bool light_sensor_data_report(uint8_t *data)
{
    if (!data) {
        return false;
    }
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)data;

    if (s_light_report_batch_count >= 10) {
        s_light_report_batch_count = 0;
    }

    light_sensor_data[s_light_report_batch_count] = sensor_data->light_sensor_data.lux_level;
    s_light_report_batch_count++;

    if (s_light_report_batch_count < 10) {
        return true;
    }

    servers_manager_context_t *servers_ctx =
        (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    if (!servers_ctx || !servers_ctx->hook) {
        LOG_EXCP("light_sensor_data_report: servers_ctx or hook null");
        s_light_report_batch_count = 0;
        return false;
    }

    servers_publish_data_args_t args = {
        .arg1 = 10,
        .arg2 = 0,
        .val = light_sensor_data,
    };
    int ret = servers_ctx->hook->factory_server_data_publish(
        SERVERS_PUBLISH_DATA_LIGHT_SENSOR_SAMPLING, &args);
    s_light_report_batch_count = 0;
    return ret == 0;
}
