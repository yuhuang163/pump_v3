/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "led_hook_internal.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static led_state_info_t s_last_reported_led_state;
static uint8_t s_last_reported_led_state_valid;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void led_hook_fill_state_snapshot(led_manager_context_t *thiz, led_broadcast_id_e evt,
                                         led_state_info_t *info)
{
    info->evt_id = (uint8_t)evt;
#if IS_SUPPORT_NIGHT_LIGHT
    info->night_light_onoff = thiz->priv.night_light_onoff;
    info->night_light_gear = thiz->priv.night_light_gear;
#endif
    info->global_adaptive_dimming_switch = thiz->priv.global_adaptive_dimming_switch;
    info->screen_key_level = thiz->priv.ui_backlight_level;
}

static int led_hook_led_state_snapshot_changed(const led_state_info_t *cur, const led_state_info_t *last)
{
    return cur->night_light_onoff != last->night_light_onoff ||
           cur->night_light_gear != last->night_light_gear ||
           cur->global_adaptive_dimming_switch != last->global_adaptive_dimming_switch ||
           cur->screen_key_level != last->screen_key_level;
}

/// 仅当 night_light_switch / gear / global_adaptive / screen_key_level 相对上次上报有变化时分发
void led_hook_notify_state(led_manager_context_t *thiz, led_broadcast_id_e evt)
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

/// App 设置总开关：写 Flash global_adaptive_switch，并同步三路 ALS 子开关（RAM）
static void set_global_adaptive_dimming_switch(led_manager_context_t *thiz, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (onoff != thiz->priv.global_adaptive_dimming_switch) {
        thiz->priv.global_adaptive_dimming_switch = onoff;
        LOG_MISC("global_adaptive_dimming_switch (ram): %d", thiz->priv.global_adaptive_dimming_switch);
        storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (st) {
            st->put_store_ram_val(STORE_ID_SYS_ADAPTIVE_SWITCH, thiz->priv.global_adaptive_dimming_switch, 0, NULL);
        }
        thiz->hook->set_key_backlight_adaptive_dimming_switch(thiz, thiz->priv.global_adaptive_dimming_switch);
        thiz->hook->set_ui_backlight_adaptive_dimming_switch(thiz, thiz->priv.global_adaptive_dimming_switch);
        #if IS_SUPPORT_NIGHT_LIGHT
        thiz->hook->set_night_light_adaptive_dimming_switch(thiz, thiz->priv.global_adaptive_dimming_switch);
        #endif
        led_hook_notify_state(thiz, LED_BROADCAST_NIGHT_LIGHT_ADAPTIVE);
    }
}

static int get_global_adaptive_dimming_switch(led_manager_context_t *thiz)
{
    return thiz->priv.global_adaptive_dimming_switch;
}

static void set_global_backlight_level(led_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level < 20 || level > LED_LEVEL_MAX) {
        return;
    }
    thiz->priv.global_backlight_level = level;
    thiz->hook->set_key_backlight_adaptive_dimming_switch(thiz, 0);
    thiz->hook->set_ui_backlight_adaptive_dimming_switch(thiz, 0);
    thiz->hook->set_key_backlight_level(thiz, level);
    thiz->hook->set_ui_backlight_level(thiz, level);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_BACK_LIGHT_LEVEL, thiz->priv.global_backlight_level, 0, NULL);
    }
    LOG_MISC("global backlight level: %d", thiz->priv.global_backlight_level);
}

static int get_global_backlight_level(led_manager_context_t *thiz)
{
    return thiz->priv.global_backlight_level;
}

static int on_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    led_manager_context_t *thiz = context;

    (void)arg2;
    (void)pdata;
    EXCEPTION_CHECK_INT(thiz);
    key_backlight_on_broadcast(thiz, id, arg1);
    ui_backlight_on_broadcast(thiz, id, arg1);
    return 0;
}

static void on_led_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    led_manager_context_t *thiz = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
    key_backlight_on_battery_state_change(thiz, info);
    ui_backlight_on_battery_state_change(thiz, info);
#if IS_SUPPORT_NIGHT_LIGHT
    night_light_on_battery_state_change(thiz, info);
#endif
}

static int on_post_init_route(led_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    thiz->priv.power = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    thiz->priv.key_light_level = LED_LEVEL_MAX;
    thiz->priv.key_light_level_rate = 100;
    thiz->priv.key_light_adaptive_level = LED_LEVEL_MAX;
    thiz->priv.ui_backlight_level_rate = 100;
    thiz->priv.ui_backlight_adaptive_val = LCD_BACKLIGHT_LEVEL_MAX;
    thiz->priv.global_backlight_level = LED_LEVEL_MAX;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    if (pbus) {
        thiz->priv.panel_dev = pbus->find_device(ID_DEVICE_DISPLAY_PANEL);
    }

    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        int tmp = LED_LEVEL_MAX;

        st->get_store_ram_val(STORE_ID_SYS_BACK_LIGHT_LEVEL, 0, 0, &tmp);
        if (tmp < 20 || tmp > (int)LED_LEVEL_MAX) {
            tmp = LED_LEVEL_MAX;
        }
        thiz->priv.global_backlight_level = (uint8_t)tmp;
        if (thiz->hook->set_key_backlight_level) {
            thiz->hook->set_key_backlight_level(thiz, (uint8_t)tmp);
        }
        if (thiz->hook->set_ui_backlight_level) {
            thiz->hook->set_ui_backlight_level(thiz, (uint8_t)tmp);
        }
        LOG_MISC("global backlight level: %d", thiz->priv.global_backlight_level);
#if IS_SUPPORT_ADAPTIVE_DIMMING
        st->get_store_ram_val(STORE_ID_SYS_ADAPTIVE_SWITCH, 0, 0, &tmp);
        thiz->priv.global_adaptive_dimming_switch = (uint8_t)tmp;
        LOG_MISC("global adaptive:%d", thiz->priv.global_adaptive_dimming_switch);
#endif
    }

#if IS_SUPPORT_NIGHT_LIGHT
    if (night_light_on_post_init(thiz) != 0) {
        return -1;
    }
#endif

    key_backlight_on_post_init(thiz);
    ui_backlight_on_post_init(thiz);

#if IS_SUPPORT_ADAPTIVE_DIMMING
    adaptive_dimming_init();
#endif
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    if (misc) {
        misc->add_misc_task(thiz, MISC_TASK_ID_KEY_LIGHT_ADAPTIVE, ADAP_DIM_SLEW_POLL_MS, MISC_WORK_MSG_ID_TIMER,
                            key_light_adaptive_thread);
#if IS_SUPPORT_ADAPTIVE_DIMMING
        misc->add_misc_task(thiz, MISC_TASK_ID_UI_BACKLIGHT_ADAPTIVE, 100, MISC_WORK_MSG_ID_TIMER,
                            ui_backlight_adaptive_thread);
#endif
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
    led_key_backlight_bind(ctx);
    led_ui_backlight_bind(ctx);
    #if IS_SUPPORT_NIGHT_LIGHT
    led_night_light_bind(ctx);
    #endif
    ctx->hook->set_global_adaptive_dimming_switch = set_global_adaptive_dimming_switch;
    ctx->hook->get_global_adaptive_dimming_switch = get_global_adaptive_dimming_switch;
    ctx->hook->set_global_backlight_level = set_global_backlight_level;
    ctx->hook->get_global_backlight_level = get_global_backlight_level;
}
