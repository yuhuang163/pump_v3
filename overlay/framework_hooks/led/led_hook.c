/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_manager.h" // IWYU pragma: keep
#include "led_manager.h" // IWYU pragma: keep
#if IS_SUPPORT_ADAPTIVE_DIMMING
#include "adaptive_dimming.h" // IWYU pragma: keep
#endif
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
#include "bsp_led_internal.h" // IWYU pragma: keep
#endif

/*
* DEFINES
*****************************************************************************************
*/
#define NIGHT_LIGHT_DEFAULT_LEVEL                     5
#define NIGHT_LIGHT_ADAPTIVE_SWITCH_LEVEL             5

#define KEY_LIGHT_DEFAULT_LEVEL                       5
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
    LOG_MISC("set key backlight: %d", level);
    if (pm->is_system_power_on() == 0) {
        LOGE("system power off, ignore set ui backlight.");
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
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev,
        OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_key_backlight_rate(led_manager_context_t *thiz, uint8_t rate)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.led_dev);
    led_level_rate_msg_t msg = {
        .led_index = PUMP_LED_ALL_ID,
        .rate = rate,
    };
    thiz->priv.key_light_adaptive_cap_level = rate;
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_LEVEL_RATE_SET, (void *)&msg);
}

static uint8_t get_key_backlight_rate(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.key_light_adaptive_cap_level;
}

static void set_night_light_level_internal(led_manager_context_t *thiz, uint8_t level)
{
    thiz->priv.night_light_level = level;
    thiz->priv.night_light_onoff = (level > 0) ? 1 : 0;
    led_status_msg_t msg = {
        .led_index = NIGHT_LIGHT_LED,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = level,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_night_light_level(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level) {
        thiz->priv.night_light_setting_level = level;
    }
    set_night_light_level_internal(thiz, level);
}

static void toggle_night_light(led_manager_context_t *thiz)
{    
    uint8_t level = 0;
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.power && thiz->priv.power->is_system_power_on() == 0) {
        LOGE("system power off, ignore toggle night light.");
        return;
    }
    thiz->hook->set_night_light_adaptive_switch(thiz, 0);
    thiz->priv.night_light_onoff = thiz->priv.night_light_level > 0 ? 0 : 1;    
    if (thiz->priv.night_light_onoff > 0) {
        if (thiz->priv.night_light_adaptive_dimming > 0) {
            LOG_MISC("night light adaptive dimming, ignore toggle night light.");
            return;
        }
        if (thiz->priv.night_light_setting_level == 0) {
            thiz->priv.night_light_setting_level = NIGHT_LIGHT_DEFAULT_LEVEL;
        }
        level = thiz->priv.night_light_setting_level;
    }
    LOG_MISC("toggle night light, onoff: %d, level: %d/%d", 
        thiz->priv.night_light_onoff, thiz->priv.night_light_adaptive_level, thiz->priv.night_light_level);
    set_night_light_level_internal(thiz, level);
}

static void night_light_adaptive_thread(void *user_data)
{
    uint8_t level = 0;
    led_manager_context_t *thiz = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.power && thiz->priv.power->is_system_power_on() == 0) {
        return;
    }
    
    if (thiz->priv.night_light_adaptive_switch) {
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        if (wm && (wm->is_current_window(wm, ID_WINDOW_CHARGE) ||
            wm->is_current_window(wm, ID_WINDOW_LOGO))) {
            return;
        }

        if (thiz->priv.night_light_adaptive_dimming) {
            level = thiz->priv.night_light_adaptive_level;
            if (level == 0) {
                level = NIGHT_LIGHT_DEFAULT_LEVEL;
            }
        } else {
            if (thiz->priv.night_light_setting_level == 0) {
                thiz->priv.night_light_setting_level = NIGHT_LIGHT_ADAPTIVE_SWITCH_LEVEL;
            }
            level = thiz->priv.night_light_setting_level;
        }
        set_night_light_level_internal(thiz, level);
        return;
    }

    if (thiz->priv.night_light_onoff > 0 && thiz->priv.night_light_adaptive_dimming > 0) {
        level = thiz->priv.night_light_adaptive_level;
        if (level == 0) {
            level = NIGHT_LIGHT_DEFAULT_LEVEL; /// 如果夜灯亮度为0, 则设置为默认亮度.
        }
        set_night_light_level_internal(thiz, level);
    }
}

static void night_light_resume_after_power_on(led_manager_context_t *thiz)
{
    uint8_t level = 0;
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.night_light_adaptive_switch) {
        if (thiz->priv.night_light_adaptive_dimming) {
            level = thiz->priv.night_light_adaptive_level;
            if (level == 0) {
                level = NIGHT_LIGHT_DEFAULT_LEVEL;
            }
        } else {
            level = thiz->priv.night_light_setting_level;
            if (level == 0) {
                level = NIGHT_LIGHT_ADAPTIVE_SWITCH_LEVEL;
            }
        }
        set_night_light_level_internal(thiz, level);
        return;
    }

    if (thiz->priv.night_light_onoff > 0) {
        if (thiz->priv.night_light_adaptive_dimming > 0) {
            level = thiz->priv.night_light_adaptive_level;
        } else {
            level = thiz->priv.night_light_setting_level;
        }
        if (level == 0) {
            level = NIGHT_LIGHT_DEFAULT_LEVEL;
        }
        set_night_light_level_internal(thiz, level);
    }
}

static void night_light_adaptive_level_update(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_MISC("adaptive level update: %d", level);
    thiz->priv.night_light_adaptive_level = level;
}

static void set_night_light_adaptive_switch(led_manager_context_t *thiz, uint8_t switch_onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.night_light_adaptive_switch = switch_onoff;
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_ADAPTIVE_SWITCH, thiz->priv.night_light_adaptive_switch, 0, NULL);
    }
}

static int get_night_light_adaptive_switch(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.night_light_adaptive_switch;
}

static void set_night_light_adaptive_dimming(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.night_light_adaptive_dimming = onoff;
}

static int get_night_light_adaptive_dimming(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.night_light_adaptive_dimming;
}

static int get_night_light_level(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.night_light_level;
}

static int get_night_light_onoff(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.night_light_onoff;
}

#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE

static uint8_t key_light_adaptive_cap_level_get(led_manager_context_t *thiz)
{
    uint8_t cap = thiz->priv.key_light_adaptive_cap_level;
    if (cap == 0 || cap > LED_LEVEL_MAX) {
        cap = LED_LEVEL_MAX;
    }
    return cap;
}

static uint8_t key_light_adaptive_level_clamp(led_manager_context_t *thiz, uint8_t level)
{
    uint8_t cap = key_light_adaptive_cap_level_get(thiz);
    return (level > cap) ? cap : level;
}

static void key_light_adaptive_thread(void *user_data)
{
    led_manager_context_t *thiz = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.power && thiz->priv.power->is_system_power_on() == 0) {
        return;
    }
    if (!thiz->priv.key_light_adaptive_dimming) {
        return;
    }
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE)) {
        return;
    }
#if (IS_SUPPORT_LOGO_BG_COLOR_TEST == 0 || IS_SUPPORT_LOGO_KEY_LIGHT_ADAPTIVE_TEST == 0)
    if (wm && wm->is_current_window(wm, ID_WINDOW_LOGO)) {
        return;
    }
#endif
    uint8_t level = thiz->priv.key_light_adaptive_level;
    if (level == 0) {
        level = KEY_LIGHT_DEFAULT_LEVEL;
    }
    level = key_light_adaptive_level_clamp(thiz, level);
    bsp_led_key_adaptive_duty_set(level);
}

static void set_key_light_level(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_MISC("set key light level: %d", level);
    thiz->priv.key_light_level = level;
    thiz->priv.key_light_adaptive_dimming = 0;
    bsp_led_key_adaptive_duty_snap(LED_LEVEL_MAX);
}

static void key_light_adaptive_level_update(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_MISC("key light adaptive level update: %d", level);
    thiz->priv.key_light_adaptive_level = level;
    if (!thiz->priv.key_light_adaptive_dimming) {
        return;
    }
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE)) {
        return;
    }
#if (IS_SUPPORT_LOGO_BG_COLOR_TEST == 0 || IS_SUPPORT_LOGO_KEY_LIGHT_ADAPTIVE_TEST == 0)
    if (wm && wm->is_current_window(wm, ID_WINDOW_LOGO)) {
        return;
    }
#endif
    level = key_light_adaptive_level_clamp(thiz, level);
    bsp_led_key_adaptive_duty_set(level);
}

static void set_key_light_adaptive_dimming(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.key_light_adaptive_dimming = onoff;
    if (onoff) {
        uint8_t lv = thiz->priv.key_light_adaptive_level;
        if (lv == 0) {
            lv = KEY_LIGHT_DEFAULT_LEVEL;
        }
        lv = key_light_adaptive_level_clamp(thiz, lv);
        bsp_led_key_adaptive_duty_set(lv);
    } else {
        bsp_led_key_adaptive_duty_snap(LED_LEVEL_MAX);
    }
}

static int get_key_light_adaptive_dimming(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.key_light_adaptive_dimming;
}

static int get_key_light_level(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.key_light_level;
}

static int get_key_light_onoff(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.key_light_onoff;
}
#endif

static int on_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    led_manager_context_t *thiz = context;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(thiz);
    switch (id.code) {
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                thiz->set_led_onoff(PUMP_LED_ALL_ID, LED_STATUS_OFF);
            } else if (arg1 == SYSTEM_BROADCAST_POWER_ON) {
                night_light_resume_after_power_on(thiz);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE: 
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                thiz->set_led_onoff(PUMP_LED_ALL_ID, LED_STATUS_OFF);
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                set_key_backlight_rate(thiz, 100);
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0 &&
                    wm->is_current_window(wm, ID_WINDOW_GUIDE) == 0) {
                    set_key_backlight(thiz, 100);
                }
            } else if (arg1 == PM_BROADCAST_SYS_IDLE) {
                set_key_backlight_rate(thiz, 50);
            } else if (arg1 == PM_BROADCAST_SYS_IDLE_RELEASE) {
                battery_manager_context_t *batt = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
                if (batt && batt->priv.battery_state == BATTERY_STATE_NONE) {
                    set_key_backlight_rate(thiz, 100);
                } else {
                    LOG_MISC("low power, ignore key backlight rate update.");
                }
            }
            break;
        case ID_BROADCAST_BATTERY_STATE_CHANGE:
            if (arg1 == BATTERY_BROADCAST_LOW_POWER_SHUTDOWN ||
                arg1 == BATTERY_BROADCAST_LOW_POWER_RUN) {
                if (get_night_light_onoff(thiz) > 0) {
                    toggle_night_light(thiz);
                }

                if (get_key_backlight_rate(thiz) > 50) {
                    set_key_backlight_rate(thiz, 50);
                }
            
                if (get_key_light_adaptive_dimming(thiz) > 0) {
                    set_key_light_adaptive_dimming(thiz, 0);
                }
            } else if (arg1 == BATTERY_BROADCAST_CAPACITY_CHANGE) {
                if (get_key_backlight_rate(thiz) < 50) {
                    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
                    if (pm && pm->is_pm_idle() == 0) {
                        set_key_backlight_rate(thiz, 100);
                    } else {
                        LOG_MISC("pm idle, ignore key backlight rate update.");
                    }
                }

                if (get_key_light_adaptive_dimming(thiz) == 0) {
                    set_key_light_adaptive_dimming(thiz, true);
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

static int on_post_init_route(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    thiz->priv.night_light_adaptive_dimming = true;
    thiz->priv.night_light_setting_level = LED_LEVEL_MAX / 2;
    thiz->priv.night_light_level = 0;
    thiz->priv.power = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        int tmp = 0;
        st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_ADAPTIVE_SWITCH, 0, 0, &tmp);
        thiz->priv.night_light_adaptive_switch = tmp;
        LOG_MISC("night light adaptive switch: %d", thiz->priv.night_light_adaptive_switch);
    }
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
    thiz->priv.key_light_adaptive_dimming = true;
    thiz->priv.key_light_level = 0;
    thiz->priv.key_light_onoff = 0;
    thiz->priv.key_light_adaptive_cap_level = LED_LEVEL_MAX;
#endif
#if IS_SUPPORT_ADAPTIVE_DIMMING
    adaptive_dimming_init();
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_INT(misc);
    misc->add_misc_task(thiz, MISC_TASK_ID_NIGHT_LIGHT_ADAPTIVE, 100, MISC_WORK_MSG_ID_TIMER,
        night_light_adaptive_thread);
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
    misc->add_misc_task(thiz, MISC_TASK_ID_KEY_LIGHT_ADAPTIVE, 100, MISC_WORK_MSG_ID_TIMER,
        key_light_adaptive_thread);
#endif
#endif
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_led_hook(led_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __led_manager_hook));
    ctx->hook->on_post_init_route = on_post_init_route;
    ctx->hook->on_broadcast_receiver_route = on_broadcast_receiver_route;
    ctx->hook->toggle_night_light = toggle_night_light;
    ctx->hook->set_night_light_level = set_night_light_level;
    ctx->hook->night_light_adaptive_level_update = night_light_adaptive_level_update;
    ctx->hook->set_night_light_adaptive_switch = set_night_light_adaptive_switch;
    ctx->hook->get_night_light_adaptive_switch = get_night_light_adaptive_switch;
    ctx->hook->set_night_light_adaptive_dimming = set_night_light_adaptive_dimming;
    ctx->hook->get_night_light_adaptive_dimming = get_night_light_adaptive_dimming;
    ctx->hook->set_key_backlight = set_key_backlight;
    ctx->hook->get_night_light_onoff = get_night_light_onoff;
    ctx->hook->get_night_light_level = get_night_light_level;
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
    ctx->hook->set_key_light_level = set_key_light_level;
    ctx->hook->key_light_adaptive_level_update = key_light_adaptive_level_update;
    ctx->hook->set_key_light_adaptive_dimming = set_key_light_adaptive_dimming;
    ctx->hook->get_key_light_adaptive_dimming = get_key_light_adaptive_dimming;
    ctx->hook->get_key_light_level = get_key_light_level;
    ctx->hook->get_key_light_onoff = get_key_light_onoff;
#endif
}

