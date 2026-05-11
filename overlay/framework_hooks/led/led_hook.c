/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "led_manager.h" // IWYU pragma: keep
#if IS_SUPPORT_ADAPTIVE_DIMMING
#include "adaptive_dimming.h" // IWYU pragma: keep
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
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_key_backlight_level_rate(led_manager_context_t *thiz, uint8_t rate)
{
    if (rate < 20 || rate > 100) {
        LOGE("set key backlight rate error");
        return;
    }
    thiz->priv.key_light_level_rate = rate;
}

static uint8_t get_key_backlight_level_rate(led_manager_context_t *thiz)
{
    return thiz->priv.key_light_level_rate;
}

static void set_key_backlight_level_max(led_manager_context_t *thiz, uint8_t level_max)
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
        LOG_MISC("level rate:%d, max:%d/%d", thiz->priv.key_light_level_rate, level_max_with_rate, level_max);
        thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_LEVEL_MAX_SET, (void *)&msg);
    }
}

#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
static void key_light_adaptive_thread(void *user_data)
{
    led_manager_context_t *thiz = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);

    if (thiz->priv.power && thiz->priv.power->is_system_power_on() == 0) {
        return;
    }
    set_key_backlight_level_max(thiz, thiz->priv.key_light_adaptive_level);
}

static void key_light_adaptive_level_update(led_manager_context_t *thiz, uint8_t level)
{
    if (level > LED_LEVEL_MAX) {
        level = LED_LEVEL_MAX;
    }

    if (thiz->priv.key_light_adaptive_level == level) {
        return;
    }
    thiz->priv.key_light_adaptive_level = level;
    LOG_MISC("level update: %d", thiz->priv.key_light_adaptive_level);
}
#endif

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
    if (thiz->priv.night_light_adaptive_level == level) {
        return;
    }
    LOG_MISC("level update: %d", level);
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
    return thiz->priv.night_light_adaptive_switch;
}

static void set_night_light_adaptive_dimming(led_manager_context_t *thiz, uint8_t onoff)
{
    thiz->priv.night_light_adaptive_dimming = onoff;
}

static int get_night_light_adaptive_dimming(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_adaptive_dimming;
}

static int get_night_light_level(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_level;
}

static int get_night_light_onoff(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_onoff;
}

static int on_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    led_manager_context_t *thiz = context;
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
                window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0 &&
                    wm->is_current_window(wm, ID_WINDOW_GUIDE) == 0) {
                    set_key_backlight(thiz, LED_LEVEL_MAX);
                }
            } else if (arg1 == PM_BROADCAST_SYS_IDLE) {
                set_key_backlight_level_rate(thiz, 50);
            } else if (arg1 == PM_BROADCAST_SYS_IDLE_RELEASE) {
                battery_manager_context_t *batt = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
                if (batt && batt->priv.battery_state == BATTERY_STATE_NONE) {
                    set_key_backlight_level_rate(thiz, 100);
                } else {
                    set_key_backlight_level_rate(thiz, 50);
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

static void on_led_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    led_manager_context_t *thiz = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
    if (info->battery_state == BATTERY_STATE_EMPTY || 
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        info->battery_state == BATTERY_STATE_FORBID_START ||
        info->battery_state == BATTERY_STATE_LOW_POWER_RUN) {
        if (get_night_light_onoff(thiz) > 0) {
            toggle_night_light(thiz);
        }

        if (get_key_backlight_level_rate(thiz) > 50) {
            set_key_backlight_level_rate(thiz, 50);
        }
    } else {
        if (get_key_backlight_level_rate(thiz) < 50) {
            power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
            if (pm && pm->is_pm_idle() == 0) {
                set_key_backlight_level_rate(thiz, 100);
            } else {
                set_key_backlight_level_rate(thiz, 50);
            }
        }
    }
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

    thiz->priv.key_light_level_max = LED_LEVEL_MAX;
    thiz->priv.key_light_level_rate = 100;
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
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    if (battery) {
        battery->add_battery_state_cb(thiz, on_led_battery_state_change_cb);
    }
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
    ctx->hook->key_light_adaptive_level_update = key_light_adaptive_level_update;
#endif
}

