/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "led_hook_internal.h" // IWYU pragma: keep
#include "light_sensor_module.h" // IWYU pragma: keep
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
#include "storage_manager_api.h" // IWYU pragma: keep
#endif
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
#include "input_key_code.h" // IWYU pragma: keep
#include "key_input_manager_api.h" // IWYU pragma: keep
#endif

/*
* DEFINES
*****************************************************************************************
*/
#define ADAPTIVE_DIMMING_DEBUG_LOG                  (0)            /// 自适应背光调试日志 0: 不打印 1: 打印

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t s_lux_thr_vdark = ADAP_DIM_LUX_THR_VDARK;
static uint16_t s_lux_thr_dark = ADAP_DIM_LUX_THR_DARK;
static uint16_t s_lux_thr_medium = ADAP_DIM_LUX_THR_MEDIUM;
static uint16_t s_lux_thr_bright = ADAP_DIM_LUX_THR_BRIGHT;

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
 * 7.2.2 滞回触发 + ADAP_DIM_LUX_DEBOUNCE_MS 去抖；基准为上次已确认 lux。
 */
static uint16_t s_last_applied_lux = 0;
static uint8_t s_lux_settling = 0;
static int8_t s_debounce_dir = 0;                ///< +1 变亮触发，-1 变暗触发
static uint16_t s_lux_settle_candidate = 0;
static uint16_t s_lux_stable_misc_ticks = 0;
static uint8_t s_lux_misc_stable_gate_ready = 0;
static uint16_t s_lux_last_batch_lux = 0;
static uint8_t s_lux_last_batch_valid = 0;
static uint16_t s_pending_lux = 0;               ///< 去抖期间跟踪的最新 lux，确认时一次映射目标
#endif

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

uint8_t adaptive_dimming_duty_interpolate(uint16_t lux, const adap_dim_lux_duty_knot_t *knots,
                                          uint8_t knot_cnt)
{
    if (knot_cnt == 0) {
        return 0;
    }
    if (lux <= knots[0].lux) {
        return knots[0].duty;
    }
    for (uint8_t i = 1; i < knot_cnt; i++) {
        if (lux <= knots[i].lux) {
            uint16_t lux0 = knots[i - 1].lux;
            uint16_t lux1 = knots[i].lux;
            uint8_t d0 = knots[i - 1].duty;
            uint8_t d1 = knots[i].duty;
            uint16_t span = lux1 - lux0;

            if (span == 0) {
                return d1;
            }
            if (d1 >= d0) {
                return (uint8_t)(d0 + (uint32_t)(lux - lux0) * (d1 - d0) / span);
            }
            return (uint8_t)(d0 - (uint32_t)(lux - lux0) * (d0 - d1) / span);
        }
    }
    return knots[knot_cnt - 1].duty;
}

uint16_t adaptive_dimming_get_filtered_lux(void)
{
    return s_lux_filtered;
}

void adaptive_dimming_get_lux_thr(adaptive_dimming_lux_thr_t *out)
{
    EXCEPTION_CHECK_VOID(out);
    out->vdark = s_lux_thr_vdark;
    out->dark = s_lux_thr_dark;
    out->medium = s_lux_thr_medium;
    out->bright = s_lux_thr_bright;
}

#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
/// 7.2.2 滞回映射表：上升阈值（lux），分段边界用运行时 s_lux_thr_*
static uint16_t adaptive_dimming_hyst_rise_thr(uint16_t ref_adc)
{
    if (ref_adc > s_lux_thr_bright) {
        return (uint16_t)((uint32_t)ref_adc * 25u / 100u);
    }
    if (ref_adc > s_lux_thr_medium) {
        return (uint16_t)((uint32_t)ref_adc * 20u / 100u);
    }
    if (ref_adc > s_lux_thr_dark) {
        return 18u;
    }
    if (ref_adc > s_lux_thr_vdark) {
        return 12u;
    }
    return 8u;
}

/// 7.2.2 滞回映射表：下降阈值（lux）
static uint16_t adaptive_dimming_hyst_fall_thr(uint16_t ref_adc)
{
    if (ref_adc > s_lux_thr_bright) {
        return (uint16_t)((uint32_t)ref_adc * 15u / 100u);
    }
    if (ref_adc > s_lux_thr_medium) {
        return (uint16_t)((uint32_t)ref_adc * 12u / 100u);
    }
    if (ref_adc > s_lux_thr_dark) {
        return 8u;
    }
    if (ref_adc > s_lux_thr_vdark) {
        return 5u;
    }
    return 3u;
}

/**
 * @return 1 当前 lux 相对 anchor 已超过滞回（需去抖）；0 在滞回带内。
 * @param out_dir +1 变亮，-1 变暗
 */
static int adaptive_dimming_lux_exceeds_hyst(uint16_t lux_now, uint16_t anchor, int *out_dir)
{
    if (lux_now > anchor) {
        uint16_t rise = adaptive_dimming_hyst_rise_thr(anchor);
        if ((uint16_t)(lux_now - anchor) >= rise) {
            *out_dir = 1;
            return 1;
        }
    } else if (lux_now < anchor) {
        uint16_t fall = adaptive_dimming_hyst_fall_thr(anchor);
        if ((uint16_t)(anchor - lux_now) >= fall) {
            *out_dir = -1;
            return 1;
        }
    }
    return 0;
}

static void adaptive_dimming_debounce_reset(void)
{
    s_lux_settling = 0;
    s_debounce_dir = 0;
    s_lux_stable_misc_ticks = 0;
    s_lux_misc_stable_gate_ready = 0;
}
#endif /* ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE */

/// 一次确认：按当前 lux 更新各灯目标 duty
static void adaptive_dimming_confirm_targets(uint16_t lux_avg)
{
    led_key_backlight_als_on_lux(lux_avg);
    led_ui_backlight_als_on_lux(lux_avg);
#if IS_SUPPORT_NIGHT_LIGHT
    led_night_light_als_on_lux(lux_avg);
#endif
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    s_last_applied_lux = lux_avg;
    s_pending_lux = 0;
    adaptive_dimming_debounce_reset();
#endif
}

/// 对批均值做滤波：变暗直接跟批均值；变亮保留 EMA + 快跟，避免亮环境抖动。
static uint16_t adaptive_dimming_lux_filter(uint16_t raw_avg)
{
    if (!s_lux_filt_inited) {
        s_lux_filtered = raw_avg;
        s_lux_filt_inited = 1;
        return s_lux_filtered;
    }
#if ADAP_DIM_DIM_LUX_FAST_PATH
    if (raw_avg < s_lux_filtered) {
        s_lux_filtered = raw_avg;
        return s_lux_filtered;
    }
#endif
    uint16_t diff = raw_avg - s_lux_filtered;
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
static void adaptive_dimming_lux_stable_misc_tick(void)
{
    s_lux_misc_stable_gate_ready = 0;
    if (!s_lux_settling || s_last_applied_lux == 0 || !s_lux_last_batch_valid) {
        return;
    }
    uint16_t dc = s_lux_last_batch_lux > s_lux_settle_candidate ? (s_lux_last_batch_lux - s_lux_settle_candidate)
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
 * 超滞回启动 ADAP_DIM_LUX_DEBOUNCE_MS 去抖；期间 s_pending_lux 跟最新 lux；
 * 期满用 pending 一次定目标（非链式多次去抖）。
 */
static int adaptive_dimming_lux_stable_gate_pass(uint16_t lux_now)
{
    if (s_last_applied_lux == 0) {
        s_pending_lux = lux_now;
        return 1;
    }

    int dir = 0;
    int exceeded = adaptive_dimming_lux_exceeds_hyst(lux_now, s_last_applied_lux, &dir);

    if (!s_lux_settling) {
        if (!exceeded) {
            return 0;
        }
        s_lux_settling = 1;
        s_debounce_dir = (int8_t)dir;
        s_lux_settle_candidate = lux_now;
        s_pending_lux = lux_now;
        s_lux_stable_misc_ticks = 0;
        s_lux_misc_stable_gate_ready = 0;
        return 0;
    }

    s_pending_lux = lux_now;

    if (!exceeded) {
        adaptive_dimming_debounce_reset();
        s_pending_lux = 0;
        return 0;
    }

    if (dir != (int)s_debounce_dir) {
        s_debounce_dir = (int8_t)dir;
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
    adaptive_dimming_debounce_reset();
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

    if (adaptive->capture_data.is_acquire_complete) {
        uint32_t lux_sum = 0;
        uint16_t lux_for_duty;

        for (int i = 0; i < ADAP_DIM_DATA_SIZE; i++) {
            lux_sum += adaptive->capture_data.buff[i];
        }
        adaptive->capture_data.is_acquire_complete = 0;
        lux_for_duty = adaptive_dimming_lux_filter((uint16_t)(lux_sum / ADAP_DIM_DATA_SIZE));
#if ADAPTIVE_DIMMING_DEBUG_LOG
        LOG_MISC("lux raw_avg: %lu, filt: %u", (unsigned long)(lux_sum / ADAP_DIM_DATA_SIZE),
                 (unsigned)lux_for_duty);
#endif
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
        s_lux_last_batch_lux = lux_for_duty;
        s_lux_last_batch_valid = 1;
        if (adaptive_dimming_lux_stable_gate_pass(lux_for_duty)) {
            uint16_t lux_confirm = s_pending_lux ? s_pending_lux : lux_for_duty;
            /* 变暗确认：lux 不得高于上次已确认值或本批滤波值，避免去抖期尖峰把 target 拉回亮档 */
            if (s_debounce_dir < 0) {
                if (lux_for_duty < lux_confirm) {
                    lux_confirm = lux_for_duty;
                }
                if (s_last_applied_lux != 0u && lux_confirm > s_last_applied_lux) {
                    lux_confirm = s_last_applied_lux;
                }
            }

            adaptive_dimming_confirm_targets(lux_confirm);
        }
#else
        adaptive_dimming_confirm_targets(lux_for_duty);
#endif
    }

}

/// 快周期：分发到各灯 ALS 输出（屏/夜灯 BSP 爬坡）
static void adaptive_dimming_slew_thread(void *user_data)
{
    led_manager_context_t *led = (led_manager_context_t *)user_data;

    EXCEPTION_CHECK_VOID(led);
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
    if (s_encoder_suppress_ticks > 0) {
        return;
    }
#endif
    led_key_backlight_als_apply(led);
    led_ui_backlight_als_apply(led);
#if IS_SUPPORT_NIGHT_LIGHT
    led_night_light_als_apply(led);
#endif
}

#define LIGHT_SENSOR_USER_CALIB_INVALID_MARK      ((int32_t)0xFFFFFFFF)

static int light_sensor_calib_thr_in_range(int32_t v)
{
    return (v > 0) && (v <= (int32_t)ADAP_DIM_LUX_MAX);
}

static int light_sensor_calib_tbl_is_invalid(const int32_t *tbl, uint8_t num)
{
    for (uint8_t i = 0; i < num; i++) {
        if (tbl[i] == 0 || tbl[i] == LIGHT_SENSOR_USER_CALIB_INVALID_MARK ||
            !light_sensor_calib_thr_in_range(tbl[i])) {
            return 1;
        }
    }
    return 0;
}

static int adaptive_dimming_lux_thr_is_monotonic(void)
{
    return (s_lux_thr_vdark < s_lux_thr_dark) &&
           (s_lux_thr_dark < s_lux_thr_medium) &&
           (s_lux_thr_medium < s_lux_thr_bright) &&
           (s_lux_thr_bright <= ADAP_DIM_LUX_MAX);
}

static void adaptive_dimming_lux_knots_rebuild(void)
{
    led_key_backlight_als_rebuild_knots();
#if IS_SUPPORT_NIGHT_LIGHT
    led_night_light_als_rebuild_knots();
#endif
}

/// 阈值变更后：重建 knots、复位去抖；滤波已就绪则按当前 lux 重算目标
static void adaptive_dimming_lux_thr_on_changed(void)
{
    adaptive_dimming_lux_knots_rebuild();
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    s_pending_lux = 0;
    adaptive_dimming_debounce_reset();
#endif
    if (s_lux_filt_inited) {
        adaptive_dimming_confirm_targets(s_lux_filtered);
    }
}

static void adaptive_dimming_lux_thr_load_defaults(void)
{
    s_lux_thr_vdark = ADAP_DIM_LUX_THR_VDARK;
    s_lux_thr_dark = ADAP_DIM_LUX_THR_DARK;
    s_lux_thr_medium = ADAP_DIM_LUX_THR_MEDIUM;
    s_lux_thr_bright = ADAP_DIM_LUX_THR_BRIGHT;
    adaptive_dimming_lux_thr_on_changed();
}

static void adaptive_dimming_lux_thr_apply(const int32_t *calib)
{
    if (calib == NULL ||
        light_sensor_calib_tbl_is_invalid(calib, ADAP_DIM_LUX_CALIB_THR_POINTS)) {
        adaptive_dimming_lux_thr_load_defaults();
        LOG_MISC("light sensor calib invalid, use default lux thr");
        return;
    }
    s_lux_thr_vdark = (uint16_t)calib[0];
    s_lux_thr_dark = (uint16_t)calib[1];
    s_lux_thr_medium = (uint16_t)calib[2];
    s_lux_thr_bright = (uint16_t)calib[3];
    if (!adaptive_dimming_lux_thr_is_monotonic()) {
        adaptive_dimming_lux_thr_load_defaults();
        LOG_MISC("light sensor calib not monotonic, use default lux thr");
        return;
    }
    adaptive_dimming_lux_thr_on_changed();
    LOG_MISC("lux thr from factory: vdark=%u dark=%u medium=%u bright=%u",
             s_lux_thr_vdark, s_lux_thr_dark, s_lux_thr_medium, s_lux_thr_bright);
}

int adaptive_dimming_lux_thr_reload_from_factory(void)
{
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    EXCEPTION_CHECK_INT(sensor);
    light_sensor_context_t *light_sensor =
        (light_sensor_context_t *)sensor->find_sensor(ID_SENSOR_LIGHT_SENSOR);
    EXCEPTION_CHECK_INT(light_sensor);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_INT(st);
    st->get_store_ram_val(STORE_ID_FACTORY_LIGHT_SENSOR_CALIB_DATA, 0, 0, light_sensor->priv.calib_tbl);
    adaptive_dimming_lux_thr_apply(light_sensor->priv.calib_tbl);
    return 0;
#else
    adaptive_dimming_lux_thr_load_defaults();
    return 0;
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
    led_key_backlight_als_init();
    led_ui_backlight_als_init();
#if IS_SUPPORT_NIGHT_LIGHT
    led_night_light_als_init();
#endif
#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
    s_last_applied_lux = 0;
    s_pending_lux = 0;
    s_debounce_dir = 0;
    adaptive_dimming_debounce_reset();
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
    misc->add_misc_task(led, MISC_TASK_ID_ADAPTIVE_DIMMING_SLEW, ADAP_DIM_SLEW_POLL_MS, MISC_WORK_MSG_ID_TIMER,
                        adaptive_dimming_slew_thread);
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    EXCEPTION_CHECK_VOID(sensor);
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    /// calib[0..3] 分段 lux 阈值（1..ADAP_DIM_LUX_MAX），calib[4] 预留
    light_sensor_context_t *light_sensor = (light_sensor_context_t *)sensor->find_sensor(ID_SENSOR_LIGHT_SENSOR);
    EXCEPTION_CHECK_VOID(light_sensor);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    st->get_store_ram_val(STORE_ID_FACTORY_LIGHT_SENSOR_CALIB_DATA, 0, 0, light_sensor->priv.calib_tbl);
    adaptive_dimming_lux_thr_apply(light_sensor->priv.calib_tbl);
#else
    adaptive_dimming_lux_thr_load_defaults();
#endif
    sensor->add_sensor_evt_lisenser(sensor, LIGHT_SENSOR_EVT_POLLING, 1, on_light_sensor_data_cb);
#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
    key_input_context_t *input_key_ctx = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    ADD_KEYEVENT_CB(input_key_ctx, "adaptive_dimming", input_key_ctx, KEY_CODE_KNOB_LEFT | KEY_CODE_KNOB_RIGHT,
                    KEY_EVENT_CLICK, on_key_input_event_cb);
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
