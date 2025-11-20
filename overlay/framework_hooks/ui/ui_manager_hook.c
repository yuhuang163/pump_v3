/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_display.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

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
static void reset_play_count_cb(ui_animation_play_t *play)
{
    play->played_count = 0;
}

static void ui_animation_play_init(struct ui_manager_context *thiz, ui_play_info_t play_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->play_animation_lock(play_info.tag);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    ani->played_count = 0;
    ani->user_data = play_info.user_data;
    ani->res_array = (uint16_t *)play_info.res_array;
    ani->res_count = play_info.res_count;
    ani->progress_cb = play_info.progress_cb;
    ani->tag = (char *)play_info.tag;
    ani->reset_play_count_cb = reset_play_count_cb;
    thiz->play_animation_unlock(play_info.tag);
}

static void lvgl_display_handler(struct ui_manager_context *thiz)
{
    thiz->play_animation_lock(NULL);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    if (ani->played_count >= ani->res_count) {
        thiz->priv.is_playing_ui_animation = false;
        if (ani->progress_cb) {
            ani->progress_cb(thiz, ani->played_count, ANI_TIMER_DRAW_FINISH);
        }
        goto HANDLER_EXIT;
    }

    lcd_tlvs_t *tlvs = NULL;
    int tlvs_num = lvgl_display_img_data_perpare(ani, &tlvs);
    ani->played_count++;
    EXCEPTION_CHECK_VOID(tlvs);
    if (tlvs_num <= 0 || tlvs_num > LCD_TLVS_DATA_TYPE_MAX_SIZE) {
        if (tlvs) {
            u_free(tlvs);
        }   
        LOGE("free tlvs failed: %d", tlvs_num);
        goto HANDLER_EXIT;
    }
    int args[] = {LCD_DISPLAY_EVENT_SRV, (uint32_t)tlvs, tlvs_num};
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)args);
    u_free(tlvs);
    
HANDLER_EXIT:
    thiz->play_animation_unlock(NULL);
}

static void on_ui_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    ui_manager_context_t *ui = context;
    EXCEPTION_CHECK_VOID(ui);
    LOG_UI("broadcast id: %d, arg1: %d, arg2: %d", id.code, arg1, arg2);
    switch (id.code) {
        case ID_BROADCAST_PM_WAKEUP_BY_USER:
            if (ui->priv.ui_backlight_val != 100) {
                ui->hook->set_ui_backlight(ui, 100);
            }
            break;
        case ID_BROADCAST_SET_BLACKLIGHT_BY_USER:
            if (ui->priv.ui_backlight_val != 100) {
                ui->hook->set_ui_backlight(ui, 100);
            }
            break;
        case ID_BROADCAST_SYS_POWER_STATE_CHANGE:
            if (arg1 == ID_BROADCAST_SYS_POWER_OFF) {
                ui->hook->set_ui_backlight(ui, 0);
            } else if (arg1 == ID_BROADCAST_SYS_POWER_ON) {
                ui->hook->set_ui_backlight(ui, 100);
            }
            break;
        default:
            break;
    }
}

static bool is_ui_backlight_on(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_BOOL(thiz);
    return thiz->priv.ui_backlight_val > 0 ? true : false;
}

static void set_ui_backlight(ui_manager_context_t *thiz, uint16_t val)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);

    if (pm->is_system_power_on() == 0) {
        LOGE("system power off, ignore set ui backlight.");
        return;
    }
    /// 设置面板背光
    if (thiz->priv.panel_dev) {
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_SET_BL, &val);
        thiz->priv.ui_backlight_val = val;
    }
    /// 设置LED背光
    if (thiz->priv.led_dev) {
        led_status_t led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = val > LED_LEVEL_MAX ? LED_LEVEL_MAX : val,
        };
        int args[] = {PUMP_LED_ALL_ID, (int)&led_status};
        thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
    }
}

static int on_post_ui_init_route(struct ui_manager_context *ctx)
{
    return 0;
}

static int on_perpare_ui_init_route(struct ui_manager_context *thiz)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_INT(pbus);
    thiz->priv.led_dev = (struct rt_device_node *)pbus->find_device(ID_DEVICE_LED);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_ui_manager_hook(ui_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __ui_manager_hook));
    ctx->hook->on_post_init_route = on_post_ui_init_route;
    ctx->hook->on_perpare_init_route = on_perpare_ui_init_route;
    ctx->hook->ui_animation_play_init = ui_animation_play_init;
    ctx->hook->lvgl_display_handler = lvgl_display_handler;
    ctx->hook->on_ui_broadcast_receiver_route = on_ui_broadcast_receiver_route;
    ctx->hook->set_ui_backlight = set_ui_backlight;
    ctx->hook->is_ui_backlight_on = is_ui_backlight_on;
}
