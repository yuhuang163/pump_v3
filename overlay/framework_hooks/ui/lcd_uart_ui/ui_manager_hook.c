/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#define UI_ANIMATION_STOP_TIMEOUT               (1000)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_manager_context_t *ui_ctx = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void lvgl_display_handler(struct ui_manager_context *thiz)
{
    uint8_t msg = 0;
    MSG_QUEUE_RECV(thiz->priv.ui_hdr_thd.msg_id, &msg, OS_WAIT_FOREVER);
    thiz->play_animation_lock(NULL);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    if (ani->is_play_finished) {
        thiz->priv.is_playing_ui_animation = false;
        if (ani->progress_cb) {
            ani->progress_cb(thiz, ani->played_count, ANI_TIMER_DRAW_FINISH);
        }
        goto HANDLER_EXIT;
    }
    
    if (ani->progress_cb) {
        ani->progress_cb(thiz, ani->played_count, ANI_TIMER_DRAW_BEFORE);
        ani->progress_cb(thiz, ani->played_count, ANI_TIMER_DRAW_AFTER);
    }
    ani->played_count++;
HANDLER_EXIT:
    thiz->play_animation_unlock(NULL);
}

static void lv_tick_timer_handler(void *args)
{
   uint8_t msg = 0;
   ui_manager_context_t *ui_ctx = (ui_manager_context_t *)args;
   MSG_QUEUE_SEND(ui_ctx->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void reset_play_count_cb(ui_animation_play_t *play)
{
    play->played_count = 0;
}

static void ui_animation_play_init(struct ui_manager_context *thiz, ui_play_info_t play_info)
{
    uint8_t msg = 0;
    EXCEPTION_CHECK_VOID(thiz);
    thiz->play_animation_lock(play_info.tag);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    if (ani->display_remove_cb) {
        ani->display_remove_cb(thiz);
    }
    ani->played_count = 0;
    ani->user_data = play_info.user_data;
    ani->progress_cb = play_info.progress_cb;
    ani->tag = (char *)play_info.tag;
    ani->reset_play_count_cb = reset_play_count_cb;
    ani->display_prepare_cb = play_info.display_prepare_cb;
    ani->display_remove_cb = play_info.display_remove_cb;
    ani->is_play_finished = false;
    if (ani->display_prepare_cb) {
        ani->display_prepare_cb(thiz);
    }
    thiz->play_animation_unlock(play_info.tag);
}

static void ui_animation_play_deinit(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->play_animation_lock(NULL);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    if (ani->display_remove_cb) {
        ani->display_remove_cb(thiz);
    }
    memset(ani, 0, sizeof(ui_animation_play_t));
    LOG_UI("ui_animation_play_deinit");
    thiz->play_animation_unlock(NULL);
}


static void ui_thread_init_route(struct ui_manager_context *thiz)
{
    CREATE_MSG_TYPE_QUEUE(&thiz->priv.ui_hdr_thd.msg_id, DISP_DISPATCH_EVT_MESQ_NUM, sizeof(uint8_t));
    CREATE_TIMER(&thiz->priv.lv_timer, "lv_timer", thiz, OS_TIMER_PERIODIC, lv_tick_timer_handler);
    START_TIMER(thiz->priv.lv_timer, LVGL_TIME_TICK_PERIOD);
}

static void set_ui_backlight(ui_manager_context_t *thiz, uint16_t val)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);

    if (pm->is_system_power_on() == 0) {
        LOGW("system power off, ignore set ui backlight.");
        return;
    }
    /// 设置面板背光
    if (thiz->priv.panel_dev) {
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_SET_BL, &val);
        thiz->priv.ui_backlight_val = val;
    }
    /// 设置LED背光
    if (thiz->priv.led_dev) {
        led_status_msg_t msg = {
            .led_index = PUMP_LED_ALL_ID,
            .led_status = {
                .mode = LED_MODE_LEVEL,
                .un.luminous.level = val > LED_LEVEL_MAX ? LED_LEVEL_MAX : val,
            },
        };
        thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
    }
}

static void set_led_blink(ui_manager_context_t *thiz, uint8_t led_index, uint16_t period,
    uint16_t blink_times, uint8_t finish_status)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = led_index,
        .led_status = {
            .mode = LED_MODE_BLINK,
            .un.blink.period = period,
            .un.blink.times = blink_times,
            .un.blink.finish_status = finish_status,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_led_level(ui_manager_context_t *thiz, uint8_t led_index, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = led_index,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = level,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_led_onoff(ui_manager_context_t *thiz, uint8_t led_index, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = led_index,
        .led_status = {
            .mode = LED_MODE_ONOFF,
            .un.onoff.status = onoff,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void on_ui_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    ui_manager_context_t *ui = context;
    EXCEPTION_CHECK_VOID(ui);
    LOG_UI("broadcast id: %d, arg1: %d, arg2: %d", id.code, arg1, arg2);
    switch (id.code) {
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                set_ui_backlight(ui, 0);
            } else if (arg1 == SYSTEM_BROADCAST_POWER_ON) {
                set_ui_backlight(ui, 100);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE: 
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                ui->priv.is_playing_ui_animation = false;
                STOP_TIMER(ui->priv.lv_timer);
                ui->priv.lv_tick_timer_running = false;
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                STOP_TIMER(ui->priv.lv_timer);
                START_TIMER(ui->priv.lv_timer, LVGL_TIME_TICK_PERIOD);
                ui->priv.lv_tick_timer_running = true;
            }
            break;

        default:
            break;
    }
}

static int on_post_ui_init_route(struct ui_manager_context *ctx)
{
    return 0;
}

static int on_perpare_ui_init_route(struct ui_manager_context *thiz)
{ 
    ui_ctx = thiz;
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
    ctx->hook->ui_animation_play_deinit = ui_animation_play_deinit;
    ctx->hook->ui_thread_init_route = ui_thread_init_route;
    ctx->hook->lvgl_display_handler = lvgl_display_handler;
    ctx->hook->on_ui_broadcast_receiver_route = on_ui_broadcast_receiver_route;
    ctx->hook->set_led_blink = set_led_blink;
    ctx->hook->set_led_level = set_led_level;
    ctx->hook->set_led_onoff = set_led_onoff;
}
