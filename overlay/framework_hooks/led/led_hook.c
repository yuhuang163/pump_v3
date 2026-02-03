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

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const uint8_t key_led_index[] = {
    PUMP_START_BL,            /// 启动背光灯
    PUMP_MODE_BL,             /// 模式背光灯
    PUMP_BREASTFEEDING_BL,    /// 追奶背光灯
};

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
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    LOG_MISC("set key backlight: %d", level);
    if (pm->is_system_power_on() == 0) {
        LOGE("system power off, ignore set ui backlight.");
        return;
    }
    EXCEPTION_CHECK_VOID(thiz->priv.led_dev);
    led_status_msg_t msg = {
        .led_index = PUMP_LED_ALL_ID,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = 0,
        },
    };

    if (level == 0) {
        thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
    } else {
        if (level > LED_LEVEL_MAX) {
            level = LED_LEVEL_MAX;
        }
        breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
        EXCEPTION_CHECK_VOID(pump);
        uint8_t control_type = pump->get_breast_pump_control_type();
        for (int i = 0; i < LENGTH_OF_ARRAY(key_led_index); i++) {
            msg.led_index = key_led_index[i];
            msg.led_status.un.luminous.level = level;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        }

        if (control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
            msg.led_index = PUMP_RIGHT_CONTROL_BL;
            msg.led_status.un.luminous.level = level / 2;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
            msg.led_index = PUMP_LEFT_CONTROL_BL;
            msg.led_status.un.luminous.level = level;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);

        } else if (control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
            msg.led_index = PUMP_LEFT_CONTROL_BL;
            msg.led_status.un.luminous.level = level / 2;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
            msg.led_index = PUMP_RIGHT_CONTROL_BL;
            msg.led_status.un.luminous.level = level;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        } else {
            msg.led_index = PUMP_LEFT_CONTROL_BL;
            msg.led_status.un.luminous.level = level;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
            msg.led_index = PUMP_RIGHT_CONTROL_BL;
            msg.led_status.un.luminous.level = level;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        }
        uint8_t mode = pump->get_breast_pump_mode();
        LOG_MISC("mode: %d, is common mode: %d", mode, BREAST_PUMP_IS_COMMON_MODE(mode));
        if (BREAST_PUMP_IS_COMMON_MODE(mode)) {
            msg.led_index = PUMP_FREQ_SET_BL;
            msg.led_status.un.luminous.level = level;
            thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        }
    }
    LOG_MISC("set ui key backlight: %d", level);
}

static void toggle_night_light(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    if (pm->is_system_power_on() == 0) {
        LOGE("system power off, ignore adaptive level update.");
        return;
    }

    thiz->priv.night_light_onoff = thiz->priv.night_light_onoff > 0 ? 0 : 1;
    if (thiz->priv.night_light_adaptive_dimming > 0 && thiz->priv.night_light_onoff > 0) {
        LOG_MISC("night light is adaptive dimming, ignore.");
        return;
    } else {
        LOG_MISC("onoff:%d, level:%d, dimming:%d", thiz->priv.night_light_onoff, 
            thiz->priv.night_light_level, thiz->priv.night_light_adaptive_dimming);
    }
    
    led_status_msg_t msg = {
        .led_index = NIGHT_LIGHT_LED,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = thiz->priv.night_light_onoff ? (LED_LEVEL_MAX / 2) : 0,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_night_light_level(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    /// 如果当前窗口为充电窗口, 禁止夜灯自适应开关.
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) && thiz->priv.night_light_onoff == 0) {
        LOG_MISC("in charge window, night light is off, ignore set night light level.");
        return;
    }
    thiz->priv.night_light_onoff = level > 0 ? 1 : 0;
    thiz->priv.night_light_level = level;
    led_status_msg_t msg = {
        .led_index = NIGHT_LIGHT_LED,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = thiz->priv.night_light_level,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void night_light_adaptive_level_update(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    if (pm->is_system_power_on() == 0) {
        LOGE("system power off, ignore adaptive level update.");
        return;
    }
    /// 夜灯开关打开, 自适应调光开关打开.
    if (thiz->priv.night_light_onoff > 0 && thiz->priv.night_light_adaptive_dimming > 0) {
        thiz->priv.night_light_level = (level < 10) ? 10 : level;  /// 自适应调光开关打开, 最低亮度为10%.
        set_night_light_level(thiz, thiz->priv.night_light_level);
    }
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

static int on_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    led_manager_context_t *thiz = context;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(thiz);
    switch (id.code) {
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                set_key_backlight(thiz, 0);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE: 
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                set_key_backlight(thiz, 0);
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0) {
                    set_key_backlight(thiz, 100); /// 充电窗口退出, 恢复背光灯.
                }
            } else if (arg1 == PM_BROADCAST_SYS_IDLE) {
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0) {
                    set_key_backlight(thiz, 50); /// 充电窗口退出, 恢复背光灯.
                }
            } else if (arg1 == PM_BROADCAST_SYS_IDLE_RELEASE) {
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0) {
                    set_key_backlight(thiz, 100); /// 充电窗口退出, 恢复背光灯.
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
    thiz->priv.night_light_level = LED_LEVEL_MAX / 2;
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        int tmp = 0;
        st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_ADAPTIVE_SWITCH, 0, 0, &tmp);
        thiz->priv.night_light_adaptive_switch = tmp;
        thiz->priv.night_light_onoff = tmp > 0 ? 1 : 0;
        LOG_MISC("night light adaptive switch: %d", thiz->priv.night_light_adaptive_switch);
    }
#if IS_SUPPORT_ADAPTIVE_DIMMING
    adaptive_dimming_init();
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
    ctx->hook->get_night_light_level = get_night_light_level;
    ctx->hook->get_night_light_onoff = get_night_light_onoff;
}

