/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "led_hook_internal.h" // IWYU pragma: keep
#if IS_SUPPORT_NIGHT_LIGHT
#include "night_light_policy.h" // IWYU pragma: keep
#endif

#if IS_SUPPORT_NIGHT_LIGHT

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING
#define ADAP_DIM_NL_PWM_IDX_OFF                    0
#define ADAP_DIM_NL_PWM_IDX_L1                     25u
#define ADAP_DIM_NL_PWM_IDX_L2                     45u
#define ADAP_DIM_NL_PWM_IDX_L3                     65u
#define ADAP_DIM_NL_ADC_DUTY_KNOT_CNT              4
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint8_t s_nl_low_power_paused;
#if IS_SUPPORT_ADAPTIVE_DIMMING
static adap_dim_lux_duty_knot_t s_nl_adc_duty_knots[ADAP_DIM_NL_ADC_DUTY_KNOT_CNT] = {
    {0u, ADAP_DIM_NL_PWM_IDX_L1},
    {ADAP_DIM_LUX_THR_VDARK, ADAP_DIM_NL_PWM_IDX_L1},
    {ADAP_DIM_LUX_THR_DARK, ADAP_DIM_NL_PWM_IDX_L2},
    {ADAP_DIM_LUX_THR_MEDIUM, ADAP_DIM_NL_PWM_IDX_L3},
};
static uint8_t s_nl_duty_target = 0;
static uint8_t s_nl_duty_pushed = ADAP_DIM_INVALID_DUTY;
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void set_night_light_level_internal(led_manager_context_t *thiz, uint8_t level)
{
    if (!thiz || !thiz->priv.led_dev) {
        LOGE("night light: led_dev null");
        return;
    }
    led_status_msg_t msg = {
        .led_index = NIGHT_LIGHT_LED,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = level,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
    thiz->priv.night_light_level = level;
}

#if IS_SUPPORT_ADAPTIVE_DIMMING
static uint8_t night_light_als_duty_from_lux(uint16_t lux_avg)
{
    adaptive_dimming_lux_thr_t thr = {0};

    adaptive_dimming_get_lux_thr(&thr);
    if (lux_avg >= thr.medium) {
        return ADAP_DIM_NL_PWM_IDX_OFF;
    }
    return adaptive_dimming_duty_interpolate(lux_avg, s_nl_adc_duty_knots,
                                           ADAP_DIM_NL_ADC_DUTY_KNOT_CNT);
}

#endif

static void night_light_apply_manual_brightness(led_manager_context_t *thiz)
{
    uint8_t gear = thiz->priv.night_light_gear;

    /* 自适应关闭后开灯：L0 不是有效手动亮度，用默认档 L2 */
    if (gear == NIGHT_LIGHT_GEAR_L0) {
        gear = night_light_policy_default_gear();
    }
    set_night_light_level_internal(thiz, night_light_policy_gear_to_level(gear));
}

static void set_night_light_adaptive_dimming_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_MISC("dimming switch: %d, global adaptive: %d", onoff, thiz->priv.global_adaptive_dimming_switch);
    if (thiz->priv.global_adaptive_dimming_switch == 0 && onoff > 0) {
        return;
    }

    uint8_t v = onoff ? 1u : 0u;

    if (thiz->priv.night_light_adaptive_dimming_switch == v) {
        return;
    }

    thiz->priv.night_light_adaptive_dimming_switch = v;
    if (!thiz->priv.night_light_onoff) {
        return;
    }

    if (v) {
        #if IS_SUPPORT_ADAPTIVE_DIMMING
        s_nl_duty_pushed = ADAP_DIM_INVALID_DUTY;
        led_night_light_als_apply(thiz);
        #endif
    } else {
        night_light_apply_manual_brightness(thiz);
    }
}

static int get_night_light_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_adaptive_dimming_switch;
}

static void night_light_turn_off(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.night_light_onoff = 0;
    thiz->priv.night_light_level = 0;
#if IS_SUPPORT_ADAPTIVE_DIMMING
    s_nl_duty_pushed = ADAP_DIM_INVALID_DUTY;
#endif
    set_night_light_level_internal(thiz, 0);
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ONOFF);
}

static void night_light_turn_on(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.night_light_onoff = 1;
    if (thiz->priv.night_light_adaptive_dimming_switch) {
        #if IS_SUPPORT_ADAPTIVE_DIMMING
        s_nl_duty_pushed = ADAP_DIM_INVALID_DUTY;
        led_night_light_als_apply(thiz);
        #endif
        led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ONOFF);
        return;
    }
    night_light_apply_manual_brightness(thiz);
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ONOFF);
}

static void set_night_light_level(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level > NIGHT_LIGHT_LEVEL_MAX) {
        level = NIGHT_LIGHT_LEVEL_MAX;
    }
    /* ALS 把亮度调到 L0(0) 只改 PWM，逻辑开关保持开 */
    if (thiz->priv.night_light_adaptive_dimming_switch == 0 && level == 0) {
        thiz->priv.night_light_onoff = level > 0 ? 1 : 0;
    }
    set_night_light_level_internal(thiz, level);
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_LEVEL);
}

static void set_night_light_gear(led_manager_context_t *thiz, uint8_t gear)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (gear >= NIGHT_LIGHT_GEAR_MAX) {
        gear = NIGHT_LIGHT_GEAR_MAX - 1;
    }

    LOG_MISC("set_night_light_gear: %d", gear);

    if (thiz->priv.night_light_gear != gear) {
        thiz->priv.night_light_gear = gear;
        storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (st) {
            st->put_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_GEAR, thiz->priv.night_light_gear, 0, NULL);
        }
    }
#if IS_SUPPORT_ADAPTIVE_DIMMING
    /// 用户明确选档：关夜灯 ALS。档位未变也要关，否则环境亮时 PWM 仍停在 0
    set_night_light_adaptive_dimming_switch(thiz, 0);
#endif
    /* 协议/UI 选 L0：灭灯。勿走 apply_manual（那里会把 L0 抬成默认 L2） */
    if (gear == NIGHT_LIGHT_GEAR_L0) {
        if (thiz->priv.night_light_onoff) {
            night_light_turn_off(thiz);
        } else {
            set_night_light_level_internal(thiz, NIGHT_LIGHT_LEVEL_OFF);
        }
        led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_LEVEL);
        return;
    }
    if (thiz->priv.night_light_onoff) {
        night_light_apply_manual_brightness(thiz);
    }
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_LEVEL);
}

static int get_night_light_gear(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_gear;
}

static int set_night_light_onoff(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_INT(thiz);
    if (thiz->priv.power && thiz->priv.power->is_system_power_on() == 0) {
        return -1;
    }
    
    LOG_MISC("set_night_light_onoff: %d", onoff);
    onoff = onoff ? 1u : 0u;
    if (thiz->priv.night_light_onoff == onoff) {
        return 0;
    }

    if (onoff) {
        night_light_turn_on(thiz);
    } else {
        night_light_turn_off(thiz);
    }
    return 0;
}

static int get_night_light_level(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_level;
}

static int get_night_light_onoff(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_onoff;
}

static void set_night_light_startup_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    uint8_t v = onoff ? 1u : 0u;

    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.night_light_startup_switch == v) {
        return;
    }
    thiz->priv.night_light_startup_switch = v;
    LOG_MISC("night_light_startup_switch (ram): %d", thiz->priv.night_light_startup_switch);

    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_STARTUP_SWITCH, thiz->priv.night_light_startup_switch, 0, NULL);
    }
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ONOFF);
}

static int get_night_light_startup_switch(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.night_light_startup_switch ? 1 : 0;
}

static void night_light_on_low_power_hold(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    s_nl_low_power_paused = 1;
    LOG_MISC("low power hold");
    if (get_night_light_onoff(thiz) > 0) {
        night_light_turn_off(thiz);
    }
}

static void night_light_on_low_power_release(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_MISC("release");
    if (!s_nl_low_power_paused) {
        return;
    }
    s_nl_low_power_paused = 0;
    if (thiz->priv.night_light_onoff == 0) {
        night_light_turn_on(thiz);
    }
    set_night_light_adaptive_dimming_switch(thiz, 1);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING
void led_night_light_als_rebuild_knots(void)
{
    adaptive_dimming_lux_thr_t thr = {0};

    adaptive_dimming_get_lux_thr(&thr);
    s_nl_adc_duty_knots[0].lux = 0u;
    s_nl_adc_duty_knots[0].duty = ADAP_DIM_NL_PWM_IDX_L1;
    s_nl_adc_duty_knots[1].lux = thr.vdark;
    s_nl_adc_duty_knots[1].duty = ADAP_DIM_NL_PWM_IDX_L1;
    s_nl_adc_duty_knots[2].lux = thr.dark;
    s_nl_adc_duty_knots[2].duty = ADAP_DIM_NL_PWM_IDX_L2;
    s_nl_adc_duty_knots[3].lux = thr.medium;
    s_nl_adc_duty_knots[3].duty = ADAP_DIM_NL_PWM_IDX_L3;
}

void led_night_light_als_on_lux(uint16_t lux)
{
    int duty = night_light_als_duty_from_lux(lux);
    if (s_nl_duty_target != duty) {
        adaptive_dimming_lux_thr_t thr = {0};
        adaptive_dimming_get_lux_thr(&thr);
        LOG_MISC("led_night_light_als_on_lux: %d -> %d, lux: %d vdark: %d dark: %d medium: %d bright: %d", 
            s_nl_duty_target, duty, lux, thr.vdark, thr.dark, thr.medium, thr.bright);
        s_nl_duty_target = duty;
    }
}

void led_night_light_als_apply(led_manager_context_t *led)
{
    EXCEPTION_CHECK_VOID(led);
    /* 关灯时空跑：只更新 target，不写 PWM */
    if (!led->priv.night_light_onoff || led->priv.night_light_adaptive_dimming_switch == 0) {
        s_nl_duty_pushed = ADAP_DIM_INVALID_DUTY;
        return;
    }

    if (s_nl_duty_target == s_nl_duty_pushed) {
        return;
    }
    LOG_MISC("led_night_light_als_apply: %d", s_nl_duty_target);
    set_night_light_level_internal(led, s_nl_duty_target);
    s_nl_duty_pushed = s_nl_duty_target;
}

void led_night_light_als_init(void)
{
    s_nl_duty_target = 0;
    s_nl_duty_pushed = ADAP_DIM_INVALID_DUTY;
}
#endif /* IS_SUPPORT_ADAPTIVE_DIMMING */

void night_light_on_battery_state_change(led_manager_context_t *thiz, battery_state_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(info);
    if (info->battery_state == BATTERY_STATE_EMPTY ||
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        info->battery_state == BATTERY_STATE_FORBID_START) {
        night_light_on_low_power_hold(thiz);
    } else if (info->battery_state != BATTERY_STATE_LOW_POWER_RUN) {
        night_light_on_low_power_release(thiz);
    }
}

int night_light_on_post_init(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    thiz->priv.night_light_onoff = 0;
    thiz->priv.night_light_gear = NIGHT_LIGHT_GEAR_L0;
    thiz->priv.night_light_level = 0;
    thiz->priv.night_light_startup_switch = 1;

    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        int tmp = NIGHT_LIGHT_GEAR_L2;

        st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_GEAR, 0, 0, &tmp);
        if (tmp >= NIGHT_LIGHT_GEAR_MAX) {
            tmp = NIGHT_LIGHT_GEAR_L2;
        }
        thiz->priv.night_light_gear = (uint8_t)tmp;

        tmp = 1;
        st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_STARTUP_SWITCH, 0, 0, &tmp);
        thiz->priv.night_light_startup_switch = tmp ? 1u : 0u;
    }
    return 0;
}

void led_night_light_bind(led_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx && ctx->hook);
    ctx->hook->get_night_light_onoff = get_night_light_onoff;
    ctx->hook->set_night_light_onoff = set_night_light_onoff;
    ctx->hook->set_night_light_level = set_night_light_level;
    ctx->hook->get_night_light_level = get_night_light_level;
    ctx->hook->set_night_light_gear = set_night_light_gear;
    ctx->hook->get_night_light_gear = get_night_light_gear;
    ctx->hook->set_night_light_startup_switch = set_night_light_startup_switch;
    ctx->hook->get_night_light_startup_switch = get_night_light_startup_switch;
    ctx->hook->set_night_light_adaptive_dimming_switch = set_night_light_adaptive_dimming_switch;
    ctx->hook->get_night_light_adaptive_dimming_switch = get_night_light_adaptive_dimming_switch;
}

#endif /* IS_SUPPORT_NIGHT_LIGHT */
