/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "led_manager.h" // IWYU pragma: keep
#if IS_SUPPORT_ADAPTIVE_DIMMING
#include "adaptive_dimming.h" // IWYU pragma: keep
#endif
#if IS_SUPPORT_NIGHT_LIGHT
#include "night_light_policy.h" // IWYU pragma: keep
#endif


/*
* LOCAL FUNCTION DEFINITIONS 
*****************************************************************************************
*/
/// 工厂模式下,屏/键/夜灯子 ALS 开关抑制
static uint8_t s_factory_als_suppress;

/*
 * 总开关两层语义：
 *
 * Flash: global_adaptive_switch — 用户/App 意图，重启后仍有效。
 * RAM:   ui/key/nl 的 *_adaptive_switch — 本机 ALS 是否跟光；可被夜灯 manual_off 等
 *        临时清 nl 子开关（不写 Flash），下次开机再由 restore_sub_switches 恢复。
 *
 * sync_sub_switches(onoff)  — 三路 RAM 子开关 = 总开关开/关。
 * set_app(onoff)            — 写 Flash + sync_sub_switches。
 * restore_sub_switches()    — 读 Flash + sync_sub_switches。
 */
static void adaptive_dimming_sync_sub_switches(led_manager_context_t *thiz, uint8_t onoff)
{
    uint8_t v = onoff ? 1u : 0u;

    if (thiz) {
        thiz->priv.key_backlight_adaptive_dimming_switch = v;
        thiz->priv.ui_backlight_adaptive_dimming_switch = v;
#if IS_SUPPORT_NIGHT_LIGHT
        thiz->priv.night_light_adaptive_dimming_switch = v;
#endif
    }
}

/// 从 Flash 恢复 global_adaptive_switch 到三路 ALS 子开关（开机、上电、产测退出）
static void adaptive_dimming_restore_sub_switches(led_manager_context_t *thiz)
{
    uint8_t g = thiz->priv.global_adaptive_dimming_switch ? 1u : 0u;

    adaptive_dimming_sync_sub_switches(thiz, g);
}
#if IS_SUPPORT_ADAPTIVE_DIMMING

#endif

static led_state_info_t s_last_reported_led_state;
static uint8_t s_last_reported_led_state_valid;

static void led_hook_fill_state_snapshot(led_manager_context_t *thiz, led_broadcast_id_e evt,
                                         led_state_info_t *info)
{
    info->evt_id = (uint8_t)evt;
#if IS_SUPPORT_NIGHT_LIGHT
    info->night_light_switch = thiz->priv.night_light_switch;
    info->night_light_gear = thiz->priv.night_light_gear;
#endif
    info->global_adaptive_dimming_switch = thiz->priv.global_adaptive_dimming_switch;
    info->screen_key_level = thiz->priv.key_light_level_max;
}

static int led_hook_led_state_snapshot_changed(const led_state_info_t *cur, const led_state_info_t *last)
{
    return cur->night_light_switch != last->night_light_switch ||
           cur->night_light_gear != last->night_light_gear ||
           cur->global_adaptive_dimming_switch != last->global_adaptive_dimming_switch ||
           cur->screen_key_level != last->screen_key_level;
}

/// 仅当 night_light_switch / gear / global_adaptive / screen_key_level 相对上次上报有变化时分发
static void led_hook_notify_state(led_manager_context_t *thiz, led_broadcast_id_e evt)
{
    led_state_info_t info = {0};

    EXCEPTION_CHECK_VOID(thiz);
    led_hook_fill_state_snapshot(thiz, evt, &info);
    if (s_last_reported_led_state_valid &&
        !led_hook_led_state_snapshot_changed(&info, &s_last_reported_led_state)) {
        return;
    }
    s_last_reported_led_state = info;
    s_last_reported_led_state_valid = 1;
    if (thiz->notify_led_state_change) {
        thiz->notify_led_state_change(&info);
    }
}

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

static void set_key_backlight_level_rate(led_manager_context_t *thiz, uint8_t rate)
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

static void key_light_adaptive_thread(void *user_data)
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
    ui_manager_context_t *ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    if (led) {
        set_key_backlight_level_rate(led, rate);
    }
    if (ui && ui->hook && ui->hook->set_ui_backlight_level_rate) {
        ui->hook->set_ui_backlight_level_rate(ui, rate);
    }
    LOG_MISC("panel backlight app rate: %d", rate);
}
#endif

#if IS_SUPPORT_NIGHT_LIGHT
static void night_light_turn_off(led_manager_context_t *thiz);
static void night_light_turn_on(led_manager_context_t *thiz);
static void set_night_light_adaptive_dimming_switch(led_manager_context_t *thiz, uint8_t onoff);
static int get_night_light_switch(led_manager_context_t *thiz);
static void set_night_light_switch(led_manager_context_t *thiz, uint8_t onoff);
static int get_night_light_onoff(led_manager_context_t *thiz);
static int toggle_night_light(led_manager_context_t *thiz);

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

static void set_night_light_level(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_MISC("nl set req: level=%u, priv=%u, gear=%u, als=%u",
        (unsigned)level,
        (unsigned)thiz->priv.night_light_level,
        (unsigned)thiz->priv.night_light_gear,
        (unsigned)thiz->priv.night_light_adaptive_dimming_switch);


    if (level > NIGHT_LIGHT_LEVEL_MAX) {
        level = NIGHT_LIGHT_LEVEL_MAX;
    }

    if (level == 0) {
        /* ALS 推送 0（亮环境/目标未就绪）仅关 PWM，勿走 turn_off 清 gear/onoff */
        if (thiz->priv.night_light_adaptive_dimming_switch && thiz->priv.night_light_onoff) {
            thiz->priv.night_light_level = 0;
            set_night_light_level_internal(thiz, 0);
            return;
        }
        night_light_turn_off(thiz);
        LOG_MISC("nl set1 skip: req=%u, priv=%u (no bsp)", (unsigned)level, (unsigned)thiz->priv.night_light_level);
        return;
    }

    thiz->priv.night_light_onoff = 1;
    if (thiz->priv.night_light_level == level) {
        set_night_light_level_internal(thiz, level);
        LOG_MISC("nl set2 skip: req=%u, priv=%u (bsp refresh)", (unsigned)level, (unsigned)thiz->priv.night_light_level);
        return;
    }
    set_night_light_level_internal(thiz, level);

    uint8_t gear = night_light_policy_level_to_gear(level);
    if (thiz->priv.night_light_gear != gear) {
        thiz->priv.night_light_gear = gear;
        led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_LEVEL);
    }
}

//  夜灯亮度关闭，并不是夜灯功能完全关闭
static void night_light_turn_off(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.night_light_onoff = 0;
    thiz->priv.night_light_level = 0;
#if IS_SUPPORT_ADAPTIVE_DIMMING
    adaptive_dimming_on_night_light_close();
#endif
    set_night_light_level_internal(thiz, 0);
}

/// APP手动关闭夜灯
void night_light_on_app_manual_off(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
#if IS_SUPPORT_ADAPTIVE_DIMMING
    set_night_light_adaptive_dimming_switch(thiz, 0);
#endif
    night_light_turn_off(thiz);
}

static void night_light_turn_on(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    uint8_t gear = thiz->priv.night_light_gear;
    
    thiz->priv.night_light_onoff = 1;
    thiz->priv.night_light_switch = 1;

#if IS_SUPPORT_ADAPTIVE_DIMMING
    if (thiz->priv.night_light_adaptive_dimming_switch) {
        adaptive_dimming_on_night_light_open(thiz);
        return;
    }
#endif
    set_night_light_level(thiz, night_light_policy_gear_to_level(gear));
}

/// 设置夜灯档位，APP及设备端调档时调用
static void set_night_light_gear(led_manager_context_t *thiz, uint8_t gear)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (gear >= NIGHT_LIGHT_GEAR_MAX) {
        gear = NIGHT_LIGHT_GEAR_MAX - 1;
    }

    if (gear == NIGHT_LIGHT_GEAR_L0) {
        thiz->priv.night_light_gear = NIGHT_LIGHT_GEAR_L0;
        thiz->priv.night_light_switch = 0;
        night_light_turn_off(thiz);
        led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_LEVEL);
        return;
    }

    if (thiz->priv.night_light_gear == gear) {
        return;
    }
    thiz->priv.night_light_gear = gear;
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_GEAR, thiz->priv.night_light_gear, 0, NULL);
    }
#if IS_SUPPORT_ADAPTIVE_DIMMING
    set_night_light_adaptive_dimming_switch(thiz, 0);
#endif
    set_night_light_level(thiz, night_light_policy_gear_to_level(gear));
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_LEVEL);
}

static int get_night_light_gear(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_gear;
}

/// 切换夜灯亮灭，上层调用（设备端 UI 暂关时仍保留供 App/低电等路径使用）
static int toggle_night_light(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    if (thiz->priv.power && thiz->priv.power->is_system_power_on() == 0) {
        return -1;
    }
    if (thiz->priv.night_light_onoff) {
        night_light_turn_off(thiz);
    } else {
        night_light_turn_on(thiz);
    }
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ONOFF);
    return thiz->priv.night_light_onoff ? 1 : 0;
}

static void set_night_light_adaptive_dimming_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.night_light_adaptive_dimming_switch = onoff ? 1u : 0u;
    LOG_MISC("night_light_adaptive_dimming_switch (ram): %d", thiz->priv.night_light_adaptive_dimming_switch);

    /// TODO: 后续考虑根据全局自适应开关来调节夜灯自适应开关
}

static int get_night_light_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    if (s_factory_als_suppress) {
        return 0;
    }
    if (!thiz->priv.global_adaptive_dimming_switch) {
        return 0;
    }
    if (!thiz->priv.night_light_switch) {
        return 0;
    }
    return thiz->priv.night_light_adaptive_dimming_switch ? 1 : 0;
}

static void set_night_light_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    uint8_t v = onoff ? 1u : 0u;

    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.night_light_switch == v) {
        return;
    }
    thiz->priv.night_light_switch = v;
    LOG_MISC("set_night_light_switch (ram): %d", thiz->priv.night_light_switch);
#if IS_SUPPORT_ADAPTIVE_DIMMING
    if (v) {
        adaptive_dimming_on_night_light_switch_restored(thiz);
    }
#endif
    led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ONOFF);
}

static int get_night_light_switch(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_switch ? 1 : 0;
}

static int get_night_light_level(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_level;
}

static int get_night_light_onoff(led_manager_context_t *thiz)
{
    return thiz->priv.night_light_onoff;
}

/// 吸乳页电源键：先改 switch，再按需 toggle 亮灭
static int sync_night_light_power_key(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    if (get_night_light_switch(thiz)) {
        set_night_light_switch(thiz, 0);
        if (get_night_light_onoff(thiz)) {
            return toggle_night_light(thiz);
        }
        return 0;
    }
    set_night_light_switch(thiz, 1);
    if (!get_night_light_onoff(thiz)) {
        return toggle_night_light(thiz);
    }
    return get_night_light_onoff(thiz) ? 1 : 0;
}

static int night_light_on_post_init(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    
    thiz->priv.night_light_switch = 0;
    thiz->priv.night_light_onoff = 0;
    thiz->priv.night_light_gear = NIGHT_LIGHT_GEAR_L0;
    thiz->priv.night_light_level = 0;

    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        int tmp = NIGHT_LIGHT_GEAR_L2;

        st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_GEAR, 0, 0, &tmp);
        if (tmp >= NIGHT_LIGHT_GEAR_MAX) {
            tmp = NIGHT_LIGHT_GEAR_L2;
        }
        thiz->priv.night_light_gear = (uint8_t)tmp;
    }

    // TODO: 后续考虑使用夜灯硬件强制关闭
    // night_light_hw_force_off(thiz);
    return 0;
}
#endif /* IS_SUPPORT_NIGHT_LIGHT */

static int get_ui_backlight_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    if (s_factory_als_suppress) {
        return 0;
    }
    if (!thiz->priv.global_adaptive_dimming_switch) {
        return 0;
    }
    return thiz->priv.ui_backlight_adaptive_dimming_switch ? 1 : 0;
}

static int get_key_backlight_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    if (s_factory_als_suppress) {
        return 0;
    }
    if (!thiz->priv.global_adaptive_dimming_switch) {
        return 0;
    }
    return thiz->priv.key_backlight_adaptive_dimming_switch ? 1 : 0;
}

/// App 设置总开关：写 Flash global_adaptive_switch，并同步三路 ALS 子开关（RAM）
static void set_global_adaptive_dimming_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (onoff != thiz->priv.global_adaptive_dimming_switch)
    {
        thiz->priv.global_adaptive_dimming_switch = onoff ? 1u : 0u;
        LOG_MISC("global_adaptive_dimming_switch (ram): %d", thiz->priv.global_adaptive_dimming_switch);
        storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (st) {
            st->put_store_ram_val(STORE_ID_SYS_ADAPTIVE_SWITCH, thiz->priv.global_adaptive_dimming_switch, 0, NULL);
        }
        adaptive_dimming_sync_sub_switches(thiz, thiz->priv.global_adaptive_dimming_switch);
        led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ADAPTIVE);
    }
}

static int get_global_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    return thiz->priv.global_adaptive_dimming_switch;
}

/// 工厂模式下,屏/键/夜灯子 ALS 开关抑制
static void set_adaptive_dimming_fct_suppress(led_manager_context_t *thiz, uint8_t onoff)
{
    s_factory_als_suppress = onoff ? 1u : 0u;
}

static int on_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    led_manager_context_t *thiz = context;
    EXCEPTION_CHECK_INT(thiz);
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
#if IS_SUPPORT_NIGHT_LIGHT
        if (get_night_light_onoff(thiz) > 0) {
            toggle_night_light(thiz);
        }
#endif
        set_key_backlight_level_rate(thiz, 50);
    } else {
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        if (pm && pm->is_pm_idle()) {
            set_key_backlight_level_rate(thiz, 50);
        } else {
            set_key_backlight_level_rate(thiz, 100);
        }
    }
}

static int on_post_init_route(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    thiz->priv.power = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    thiz->priv.key_light_level_max = LED_LEVEL_MAX;
    thiz->priv.key_light_level_rate = 100;

    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        int tmp = 0;
#if IS_SUPPORT_ADAPTIVE_DIMMING
        tmp = PANEL_BACKLIGHT_APP_RATE_DEFAULT;
        st->get_store_ram_val(STORE_ID_SYS_BACK_LIGHT_LEVEL, 0, 0, &tmp);
        if (tmp < (int)PANEL_BACKLIGHT_APP_RATE_MIN || tmp > (int)PANEL_BACKLIGHT_APP_RATE_MAX) {
            tmp = PANEL_BACKLIGHT_APP_RATE_DEFAULT;
        }
        panel_backlight_apply_app_rate((uint8_t)tmp);

        /// 获取全局自适应开关
        st->get_store_ram_val(STORE_ID_SYS_ADAPTIVE_SWITCH, 0, 0, &tmp);
        thiz->priv.global_adaptive_dimming_switch = (uint8_t)tmp;
        adaptive_dimming_restore_sub_switches(thiz);
#endif
    }

#if IS_SUPPORT_NIGHT_LIGHT
    if (night_light_on_post_init(thiz) != 0) {
        return -1;
    }
#endif

#if IS_SUPPORT_ADAPTIVE_DIMMING
    adaptive_dimming_init();
#endif
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    if (misc) {
        misc->add_misc_task(thiz, MISC_TASK_ID_KEY_LIGHT_ADAPTIVE, ADAP_DIM_SLEW_POLL_MS, MISC_WORK_MSG_ID_TIMER,
                            key_light_adaptive_thread);
    }
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    ADD_BATTERY_STATE_CB(battery, thiz, on_led_battery_state_change_cb);
    return 0;
}

void register_led_hook(led_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __led_manager_hook));
    ctx->hook->on_post_init_route = on_post_init_route;
    ctx->hook->on_broadcast_receiver_route = on_broadcast_receiver_route;
#if IS_SUPPORT_NIGHT_LIGHT
    ctx->hook->get_night_light_onoff = get_night_light_onoff;
    ctx->hook->toggle_night_light = toggle_night_light;
    ctx->hook->set_night_light_level = set_night_light_level;
    ctx->hook->get_night_light_level = get_night_light_level;
    ctx->hook->set_night_light_gear = set_night_light_gear;
    ctx->hook->get_night_light_gear = get_night_light_gear;
    ctx->hook->set_night_light_switch = set_night_light_switch;
    ctx->hook->get_night_light_switch = get_night_light_switch;
    ctx->hook->set_night_light_adaptive_dimming_switch = set_night_light_adaptive_dimming_switch;
    ctx->hook->get_night_light_adaptive_dimming_switch = get_night_light_adaptive_dimming_switch;
    ctx->hook->sync_night_light_power_key = sync_night_light_power_key;
#endif
    ctx->hook->set_key_backlight = set_key_backlight;
    ctx->hook->set_key_backlight_level = set_key_backlight_level;
    ctx->hook->get_key_backlight_level = get_key_backlight_level;
    ctx->hook->key_light_adaptive_level_update = key_light_adaptive_level_update;
    ctx->hook->set_key_backlight_adaptive_dimming_switch = set_key_backlight_adaptive_dimming_switch;
    ctx->hook->set_global_adaptive_dimming_switch = set_global_adaptive_dimming_switch;
    ctx->hook->get_global_adaptive_dimming_switch = get_global_adaptive_dimming_switch;
    ctx->hook->set_adaptive_dimming_fct_suppress = set_adaptive_dimming_fct_suppress;
    ctx->hook->get_ui_backlight_adaptive_dimming_switch = get_ui_backlight_adaptive_dimming_switch;
    ctx->hook->get_key_backlight_adaptive_dimming_switch = get_key_backlight_adaptive_dimming_switch;
}
