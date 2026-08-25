/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "led_hook_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING
#define ADAP_DIM_BACK_LUX_DUTY_KNOT_CNT          6
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING
/// 与分段曲线关键点一致，段内线性插值（lux↑ → duty↑）；lux 轴由运行时阈值重建
static adap_dim_lux_duty_knot_t s_back_lux_duty_knots[ADAP_DIM_BACK_LUX_DUTY_KNOT_CNT] = {
    {0u, ADAP_DIM_LED_LEVEL1_MIN},
    {ADAP_DIM_LUX_THR_VDARK, ADAP_DIM_LED_LEVEL1_MAX},
    {ADAP_DIM_LUX_THR_DARK, ADAP_DIM_LED_LEVEL2_MAX},
    {ADAP_DIM_LUX_THR_MEDIUM, ADAP_DIM_LED_LEVEL3_MAX},
    {ADAP_DIM_LUX_THR_BRIGHT, ADAP_DIM_LED_LEVEL4_MAX},
    {ADAP_DIM_LUX_MAX, ADAP_DIM_LED_LEVEL5_MAX},
};
static uint8_t s_back_duty_target = ADAP_DIM_LED_LEVEL1_MIN;
static uint8_t s_key_duty_slew = ADAP_DIM_LED_LEVEL1_MIN;
static uint8_t s_key_duty_pushed = ADAP_DIM_INVALID_DUTY;
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void set_key_backlight(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.led_dev);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    if (pm->is_system_power_on() == 0 && level > 0) {
        LOGE("system power off, ignore set key backlight.");
        return;
    }
    led_status_msg_t msg = {
        .led_index = PUMP_LED_ALL_ID,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous = {
                .level = level > LED_LEVEL_MAX ? LED_LEVEL_MAX : level,
            },
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_key_backlight_level(led_manager_context_t *thiz, uint8_t level_max)
{
    uint8_t level_max_with_rate = level_max * thiz->priv.key_light_level_rate / 100;
    if (level_max_with_rate == thiz->priv.key_light_level_max) {
        return;
    }
    thiz->priv.key_light_level_max = level_max_with_rate;
    led_level_msg_t msg = {
        .led_index = PUMP_LED_ALL_ID,
        .level_max = thiz->priv.key_light_level_max,
    };
    if (thiz->priv.led_dev) {
        thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_LEVEL_MAX_SET, (void *)&msg);
    }
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_LEVEL);
}

static int get_key_backlight_level(led_manager_context_t *thiz)
{
    return thiz->priv.key_light_level_max;
}

static void key_light_adaptive_level_update(led_manager_context_t *thiz, uint8_t level)
{
    if (thiz->priv.key_backlight_adaptive_dimming_switch == 0) {
        return;
    }
    if (level > LED_LEVEL_MAX) {
        level = LED_LEVEL_MAX;
    }
    if (thiz->priv.key_light_adaptive_level == level) {
        return;
    }
    thiz->priv.key_light_adaptive_level = level;
}

static void set_key_backlight_adaptive_dimming_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    thiz->priv.key_backlight_adaptive_dimming_switch = onoff ? 1u : 0u;
    if (onoff == 0) {
        set_key_backlight_level_rate(thiz, 100);
    }
}

static int get_key_backlight_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    if (!thiz->priv.global_adaptive_dimming_switch) {
        return 0;
    }
    return thiz->priv.key_backlight_adaptive_dimming_switch ? 1 : 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void set_key_backlight_level_rate(led_manager_context_t *thiz, uint8_t rate)
{
    if (rate > 100) {
        rate = 100;
    }
    if (thiz->priv.key_light_level_rate == rate) {
        return;
    }
    LOG_MISC("set key backlight level rate: %d", rate);
    thiz->priv.key_light_level_rate = rate;
}

void key_backlight_on_battery_state_change(led_manager_context_t *thiz, battery_state_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(info);
    if (info->battery_state == BATTERY_STATE_EMPTY ||
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        info->battery_state == BATTERY_STATE_FORBID_START) {
        set_key_backlight_level_rate(thiz, FORBID_START_BRIGHTNESS_MAX_VALUE);
    } else if (info->battery_state == BATTERY_STATE_LOW_POWER_RUN) {
        set_key_backlight_level_rate(thiz, LOW_POWER_RUN_BRIGHTNESS_MAX_VALUE);
    } else {
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        if (pm && pm->is_pm_idle()) {
            set_key_backlight_level_rate(thiz, IDLE_BRIGHTNESS_MAX_VALUE);
        } else {
            set_key_backlight_level_rate(thiz, 100);
        }
    }
}

void key_backlight_on_broadcast(led_manager_context_t *thiz, broadcast_mask_id_t id, int arg1)
{
    EXCEPTION_CHECK_VOID(thiz);
    switch (id.code) {
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                thiz->set_led_onoff(PUMP_LED_ALL_ID, LED_STATUS_OFF);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                thiz->set_led_onoff(PUMP_LED_ALL_ID, LED_STATUS_OFF);
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0 &&
                    wm->is_current_window(wm, ID_WINDOW_GUIDE) == 0 &&
                    wm->is_current_window(wm, ID_WINDOW_POWER_OFF) == 0) {
                    set_key_backlight(thiz, LED_LEVEL_MAX);
                }
            } else if (arg1 == PM_BROADCAST_SYS_IDLE) {
                set_key_backlight_level_rate(thiz, IDLE_BRIGHTNESS_MAX_VALUE);
            } else if (arg1 == PM_BROADCAST_SYS_IDLE_RELEASE) {
                battery_manager_context_t *batt = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
                if (batt && batt->priv.battery_state == BATTERY_STATE_NONE) {
                    set_key_backlight_level_rate(thiz, 100);
                } else {
                    set_key_backlight_level_rate(thiz, LOW_POWER_RUN_BRIGHTNESS_MAX_VALUE);
                }
            }
            break;
        default:
            break;
    }
}

void key_light_adaptive_thread(void *user_data)
{
    led_manager_context_t *thiz = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.power && thiz->priv.power->is_system_power_on() == 0) {
        return;
    }
#if IS_SUPPORT_ADAPTIVE_DIMMING
    if (!thiz->hook->get_key_backlight_adaptive_dimming_switch(thiz)) {
        return;
    }
#endif
    set_key_backlight_level(thiz, thiz->priv.key_light_adaptive_level);
}

#if IS_SUPPORT_ADAPTIVE_DIMMING
void panel_backlight_apply_app_rate(uint8_t rate)
{
    if (rate < PANEL_BACKLIGHT_APP_RATE_MIN) {
        rate = PANEL_BACKLIGHT_APP_RATE_MIN;
    } else if (rate > PANEL_BACKLIGHT_APP_RATE_MAX) {
        rate = PANEL_BACKLIGHT_APP_RATE_MAX;
    }
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_BACK_LIGHT_LEVEL, rate, 0, NULL);
    }
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    if (led) {
        set_key_backlight_level_rate(led, rate);
    }
    LOG_MISC("panel backlight app rate: %d", rate);
}

uint8_t led_key_backlight_als_duty_from_lux(uint16_t lux_avg)
{
    if (lux_avg > ADAP_DIM_LUX_MAX) {
        lux_avg = ADAP_DIM_LUX_MAX;
    }
    return adaptive_dimming_duty_interpolate(lux_avg, s_back_lux_duty_knots,
                                           ADAP_DIM_BACK_LUX_DUTY_KNOT_CNT);
}

void led_key_backlight_als_rebuild_knots(void)
{
    adaptive_dimming_lux_thr_t thr = {0};

    adaptive_dimming_get_lux_thr(&thr);
    s_back_lux_duty_knots[0].lux = 0u;
    s_back_lux_duty_knots[0].duty = ADAP_DIM_LED_LEVEL1_MIN;
    s_back_lux_duty_knots[1].lux = thr.vdark;
    s_back_lux_duty_knots[1].duty = ADAP_DIM_LED_LEVEL1_MAX;
    s_back_lux_duty_knots[2].lux = thr.dark;
    s_back_lux_duty_knots[2].duty = ADAP_DIM_LED_LEVEL2_MAX;
    s_back_lux_duty_knots[3].lux = thr.medium;
    s_back_lux_duty_knots[3].duty = ADAP_DIM_LED_LEVEL3_MAX;
    s_back_lux_duty_knots[4].lux = thr.bright;
    s_back_lux_duty_knots[4].duty = ADAP_DIM_LED_LEVEL4_MAX;
    s_back_lux_duty_knots[5].lux = ADAP_DIM_LUX_MAX;
    s_back_lux_duty_knots[5].duty = ADAP_DIM_LED_LEVEL5_MAX;
}

void led_key_backlight_als_on_lux(uint16_t lux)
{
    s_back_duty_target = led_key_backlight_als_duty_from_lux(lux);
}

void led_key_backlight_als_apply(led_manager_context_t *led)
{
    s_key_duty_slew = s_back_duty_target;
    if (led->hook && led->hook->get_key_backlight_adaptive_dimming_switch(led) &&
        led->hook->key_light_adaptive_level_update &&
        s_key_duty_slew != s_key_duty_pushed) {
        led->hook->key_light_adaptive_level_update(led, s_key_duty_slew);
        s_key_duty_pushed = s_key_duty_slew;
    }
}

void led_key_backlight_als_init(void)
{
    s_back_duty_target = ADAP_DIM_LED_LEVEL1_MIN;
    s_key_duty_slew = ADAP_DIM_LED_LEVEL1_MIN;
    s_key_duty_pushed = ADAP_DIM_INVALID_DUTY;
}
#endif

void led_key_backlight_bind(led_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx && ctx->hook);
    ctx->hook->set_key_backlight = set_key_backlight;
    ctx->hook->set_key_backlight_level = set_key_backlight_level;
    ctx->hook->set_key_backlight_level_rate = set_key_backlight_level_rate;
    ctx->hook->get_key_backlight_level = get_key_backlight_level;
    ctx->hook->key_light_adaptive_level_update = key_light_adaptive_level_update;
    ctx->hook->set_key_backlight_adaptive_dimming_switch = set_key_backlight_adaptive_dimming_switch;
    ctx->hook->get_key_backlight_adaptive_dimming_switch = get_key_backlight_adaptive_dimming_switch;
}
