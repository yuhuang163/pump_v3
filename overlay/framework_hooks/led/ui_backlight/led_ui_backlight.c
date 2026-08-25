/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "led_hook_internal.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING
static uint8_t s_ui_duty_target = ADAP_DIM_LED_LEVEL1_MIN;
static uint8_t s_ui_duty_pushed = ADAP_DIM_INVALID_DUTY;
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void set_ui_backlight_level_internal(led_manager_context_t *thiz, uint16_t level)
{
    if (thiz->priv.panel_dev) {
        uint32_t tmp[1] = {level};
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_SET_BL, &tmp);
    }
    thiz->priv.ui_backlight_level = (uint8_t)level;
}

static void set_ui_backlight_level(led_manager_context_t *thiz, uint8_t level)
{
    uint8_t level_with_rate;

    EXCEPTION_CHECK_VOID(thiz);
    level_with_rate = level * thiz->priv.ui_backlight_level_rate / 100;
    if (level_with_rate > LCD_BACKLIGHT_LEVEL_MAX) {
        level_with_rate = LCD_BACKLIGHT_LEVEL_MAX;
    }
    set_ui_backlight_level_internal(thiz, level_with_rate);
}

static void set_ui_backlight_level_rate(led_manager_context_t *thiz, uint8_t rate)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (rate < 20 || rate > 100) {
        LOGE("set ui backlight rate error");
        return;
    }
    if (thiz->priv.ui_backlight_level_rate == rate) {
        return;
    }
    LOG_MISC("set ui backlight level rate: %d", rate);
    thiz->priv.ui_backlight_level_rate = rate;
}

#if IS_SUPPORT_ADAPTIVE_DIMMING
static void ui_backlight_adaptive_level_update(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (!thiz->hook->get_key_backlight_adaptive_dimming_switch(thiz)) {
        return;
    }
    if (level > LCD_BACKLIGHT_LEVEL_MAX) {
        level = LCD_BACKLIGHT_LEVEL_MAX;
    }
    if (thiz->priv.ui_backlight_adaptive_val == level) {
        return;
    }
    thiz->priv.ui_backlight_adaptive_val = level;
}
#endif

static int get_ui_backlight_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    return thiz->priv.ui_backlight_adaptive_dimming_switch ? 1 : 0;
}

static void set_ui_backlight_adaptive_dimming_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.ui_backlight_adaptive_dimming_switch = onoff;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING
void ui_backlight_adaptive_thread(void *user_data)
{
    led_manager_context_t *thiz = (led_manager_context_t *)user_data;
    uint8_t level_with_rate;

    EXCEPTION_CHECK_VOID(thiz);
    if (!thiz->hook->get_key_backlight_adaptive_dimming_switch(thiz)) {
        return;
    }
    level_with_rate = thiz->priv.ui_backlight_adaptive_val * thiz->priv.ui_backlight_level_rate / 100;
    if (level_with_rate > LCD_BACKLIGHT_LEVEL_MAX) {
        level_with_rate = LCD_BACKLIGHT_LEVEL_MAX;
    } else if (level_with_rate < LCD_BACKLIGHT_LEVEL_MIN) {
        level_with_rate = LCD_BACKLIGHT_LEVEL_MIN;
    }
    if (thiz->priv.ui_backlight_level == level_with_rate) {
        return;
    }
    LOG_MISC("ui bl level:%d/%d, rate: %d", level_with_rate, thiz->priv.ui_backlight_adaptive_val,
        thiz->priv.ui_backlight_level_rate);
    set_ui_backlight_level_internal(thiz, level_with_rate);
}

void led_ui_backlight_als_on_lux(uint16_t lux)
{
    s_ui_duty_target = led_key_backlight_als_duty_from_lux(lux);
}

void led_ui_backlight_als_apply(led_manager_context_t *led)
{
    if (led->hook && led->hook->get_key_backlight_adaptive_dimming_switch(led) &&
        led->hook->ui_backlight_adaptive_level_update) {
        if (s_ui_duty_target != s_ui_duty_pushed) {
            led->hook->ui_backlight_adaptive_level_update(led, s_ui_duty_target);
            s_ui_duty_pushed = s_ui_duty_target;
        }
    }
}

void led_ui_backlight_als_init(void)
{
    s_ui_duty_target = ADAP_DIM_LED_LEVEL1_MIN;
    s_ui_duty_pushed = ADAP_DIM_INVALID_DUTY;
}
#endif

void ui_backlight_on_battery_state_change(led_manager_context_t *thiz, battery_state_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz && info);
    if (info->battery_state == BATTERY_STATE_EMPTY ||
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        info->battery_state == BATTERY_STATE_FORBID_START) {
        set_ui_backlight_level_rate(thiz, FORBID_START_BRIGHTNESS_MAX_VALUE);
    } else if (info->battery_state == BATTERY_STATE_LOW_POWER_RUN) {
        set_ui_backlight_level_rate(thiz, LOW_POWER_RUN_BRIGHTNESS_MAX_VALUE);
    } else {
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        if (wm && wm->get_current_window_id() != ID_WINDOW_POWER_OFF &&
            wm->get_current_window_id() != ID_WINDOW_DUMMY &&
            wm->get_current_window_id() != ID_WINDOW_CHARGE) {
            set_ui_backlight_level_rate(thiz, 100);
        }
    }
}

void ui_backlight_on_broadcast(led_manager_context_t *thiz, broadcast_mask_id_t id, int arg1)
{
    EXCEPTION_CHECK_VOID(thiz);
    switch (id.code) {
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                set_ui_backlight_level(thiz, 0);
            } else if (arg1 == SYSTEM_BROADCAST_POWER_ON) {
                set_ui_backlight_level(thiz, LCD_BACKLIGHT_LEVEL_MAX);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                set_ui_backlight_level(thiz, 0);
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                set_ui_backlight_level(thiz, LCD_BACKLIGHT_LEVEL_MAX);
            }
            break;
        default:
            break;
    }
}

void ui_backlight_on_post_init(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
#if IS_SUPPORT_ADAPTIVE_DIMMING
    thiz->priv.ui_backlight_adaptive_dimming_switch = thiz->priv.global_adaptive_dimming_switch;
#endif
}

void led_ui_backlight_bind(led_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(ctx->hook);
    ctx->hook->set_ui_backlight_level = set_ui_backlight_level;
    ctx->hook->set_ui_backlight_level_rate = set_ui_backlight_level_rate;
    ctx->hook->ui_backlight_adaptive_level_update = ui_backlight_adaptive_level_update;
    ctx->hook->get_ui_backlight_adaptive_dimming_switch = get_ui_backlight_adaptive_dimming_switch;
    ctx->hook->set_ui_backlight_adaptive_dimming_switch = set_ui_backlight_adaptive_dimming_switch;
}
