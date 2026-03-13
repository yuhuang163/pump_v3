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
#define NIGHT_LIGHT_DEFAULT_LEVEL                     5
#define NIGHT_LIGHT_ADAPTIVE_SWITCH_LEVEL             5

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
    msg.led_status.un.luminous.level = level;
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_night_light_level_internal(led_manager_context_t *thiz, uint8_t level)
{
    thiz->priv.night_light_level = level;
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
    thiz->priv.night_light_adaptive_switch = 0;   /// 手动开关后，临时关闭夜灯自适应开关，重启恢复
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
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0 && wm->is_current_window(wm, ID_WINDOW_GUIDE) == 0) {
                    set_key_backlight(thiz, 100); /// 充电窗口退出, 恢复背光灯.
                }
            } else if (arg1 == PM_BROADCAST_SYS_IDLE) {
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0 && wm->is_current_window(wm, ID_WINDOW_GUIDE) == 0) {
                    set_key_backlight(thiz, 50); /// 充电窗口退出, 恢复背光灯.
                }
            } else if (arg1 == PM_BROADCAST_SYS_IDLE_RELEASE) {
                if (wm && wm->is_current_window(wm, ID_WINDOW_CHARGE) == 0 && wm->is_current_window(wm, ID_WINDOW_GUIDE) == 0) {
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
#if IS_SUPPORT_ADAPTIVE_DIMMING
    adaptive_dimming_init();
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_INT(misc);
    misc->add_misc_task(thiz, MISC_TASK_ID_NIGHT_LIGHT_ADAPTIVE, 100, MISC_WORK_MSG_ID_TIMER,
        night_light_adaptive_thread);
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

