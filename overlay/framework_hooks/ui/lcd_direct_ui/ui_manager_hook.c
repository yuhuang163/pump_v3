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
#define IS_PRINT_LVGL_MEM_MONITOR               1

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
#if IS_PRINT_LVGL_MEM_MONITOR
static void lvgl_mem_monitor_cb(void)
{
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);

    uint32_t used_kb = (mon.total_size - mon.free_size) / 1024;
    uint32_t free_kb = mon.free_size / 1024;
    uint32_t total_kb = mon.total_size / 1024;
    uint32_t max_used_kb = mon.max_used / 1024;
    // uint32_t biggest_free_kb = mon.free_biggest_size / 1024;
    LOG_UI("total:%lu KB, used:%lu KB (%d%%), free:%lu KB, max_used:%lu KB",
           (unsigned long)total_kb,
           (unsigned long)used_kb,
           mon.used_pct,
           (unsigned long)free_kb,
           (unsigned long)max_used_kb);
}
#endif

static void lvgl_display_handler(struct ui_manager_context *thiz)
{
    uint8_t msg = 0, is_running = 0;
    uint32_t ms_till_next = 0;
    uint16_t log_count = 0;
    MSG_QUEUE_RECV(thiz->priv.ui_hdr_thd.msg_id, &msg, OS_WAIT_FOREVER);
    LOG_UI("==>lvgl scheduler enter");
    while(1) {             
        if (thiz->priv.lv_timer == NULL ||
            lv_timer_get_paused(thiz->priv.lv_timer) == true) {
            break;
        }

        ms_till_next = lv_timer_handler();
        if (ms_till_next > 10000) {
            LOGE("time error!!!!!!!: %d", ms_till_next);
            ms_till_next = 200;
        }
        msleep(ms_till_next);
        log_count++;
        if (log_count % 200 == 0) {
            LOG_UI("lvgl scheduler:%d", log_count);
#if IS_PRINT_LVGL_MEM_MONITOR
            lvgl_mem_monitor_cb();
#endif
        }
    }
    LOG_UI("==>lvgl scheduler exit");
}

static void change_images_cb(lv_timer_t *t)
{    
    EXCEPTION_CHECK_VOID(ui_ctx);
    if (ui_ctx->priv.lv_timer == NULL || ui_ctx->priv.lv_timer != t) {
        LOG_UI("==>lv_timer is %s:%p", ui_ctx->priv.lv_timer == NULL ? "NULL" : "changed", t);
        return;
    }
    
    if (!ui_ctx->priv.is_playing_ui_animation) {
        return;
    }
    
    ui_animation_play_t *ani = &ui_ctx->priv.play_info;
    EXCEPTION_CHECK_VOID(ani->tag);
    if (ani->is_play_finished) {
        ui_ctx->priv.is_playing_ui_animation = false;
        if (ani->progress_cb) {
            ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_FINISH);
        }
        return;
    }

    if (ani->progress_cb) {
        ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_BEFORE);
        ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_AFTER);
    }
    ani->played_count++;
}

static void reset_play_count_cb(ui_animation_play_t *play)
{
    play->played_count = 0;
}

static void ui_animation_play_init(struct ui_manager_context *thiz, ui_play_info_t play_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    uint8_t msg = 0;
    uint32_t period = 1000 / play_info.fps;
    LOG_UI("==>%s, period: %d", play_info.tag, period);    
    
    if (thiz->priv.lv_timer != NULL) {
        lv_timer_t *timer_to_delete = thiz->priv.lv_timer;
        thiz->priv.lv_timer = NULL;
        lv_timer_pause(timer_to_delete);
        msleep(10);
        lv_timer_delete(timer_to_delete);
    }

    ui_animation_play_t *ani = &thiz->priv.play_info;
    if (ani->display_remove_cb) {
        lv_lock();
        ani->display_remove_cb(thiz);
        lv_unlock();
        LOG_UI("==> ui remove tag: %s", ani->tag);
        // lv_refr_now(NULL);
    } 
    thiz->play_animation_lock(play_info.tag);
    ani->played_count = 0;
    ani->user_data = play_info.user_data;
    ani->progress_cb = play_info.progress_cb;
    ani->tag = (char *)play_info.tag;
    ani->reset_play_count_cb = reset_play_count_cb;
    ani->display_prepare_cb = play_info.display_prepare_cb;
    ani->display_remove_cb = play_info.display_remove_cb;
    ani->is_play_finished = false;
    thiz->play_animation_unlock(play_info.tag);
    
    if (ani->display_prepare_cb) {
        lv_lock();
        ani->display_prepare_cb(thiz);
        lv_unlock();
    }

    if (thiz->priv.lv_timer == NULL) {
        thiz->priv.lv_timer = lv_timer_create(change_images_cb, period < 30 ? 30 : period, NULL);
        if (thiz->priv.lv_timer != NULL) {
            lv_timer_ready(thiz->priv.lv_timer);
        } else {
            LOG_UI("==>lv_timer create failed");
        }
    }
    thiz->priv.is_playing_ui_animation = true;
    LOG_UI("==>%s, lv_timer created, period: %d, timer: %p", 
        play_info.tag, period < 30 ? 30 : period, thiz->priv.lv_timer);
    MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);

}

static void ui_animation_play_deinit(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_UI("==> ui deinit tag: %s", thiz->priv.play_info.tag);
    MSG_QUEUE_RESET(thiz->priv.ui_hdr_thd.msg_id);
    thiz->priv.is_playing_ui_animation = false;
    if (thiz->priv.lv_timer != NULL && lv_timer_get_paused(thiz->priv.lv_timer) == false) {
        lv_timer_t *timer_to_delete = thiz->priv.lv_timer;
        thiz->priv.lv_timer = NULL;  // 先清空，防止回调使用
        lv_timer_pause(timer_to_delete);
        msleep(10);
        lv_timer_delete(timer_to_delete);
        LOG_UI("==>lv_timer deleted:%p", timer_to_delete);
    }
    ui_animation_play_t *ani = &thiz->priv.play_info;
    if (ani->display_remove_cb) {
        lv_lock();
        ani->display_remove_cb(thiz);
        lv_unlock();
        // lv_refr_now(NULL);
    }
    thiz->play_animation_lock(NULL);
    memset(ani, 0, sizeof(ui_animation_play_t));
    thiz->play_animation_unlock(NULL);
}

static void ui_thread_init_route(struct ui_manager_context *thiz)
{
    CREATE_MSG_TYPE_QUEUE(&thiz->priv.ui_hdr_thd.msg_id, 1, sizeof(uint8_t));
    thiz->priv.lvgl_initialized = true;
    LOG_UI("==>lvgl init done");
}

static void set_ui_panel_backlight(ui_manager_context_t *thiz, uint16_t val)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);

    if (pm->is_system_power_on() == 0) {
        LOGE("system power off, ignore set ui backlight.");
        return;
    }

    if (thiz->priv.panel_dev) {
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_SET_BL, &val);
        thiz->priv.ui_backlight_val = val;
    }
}

static void set_ui_key_backlight(ui_manager_context_t *thiz, uint16_t val)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);

    if (pm->is_system_power_on() == 0) {
        LOGE("system power off, ignore set ui backlight.");
        return;
    }

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

static void set_led_blink(ui_manager_context_t *thiz, led_blink_info_t blink_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = blink_info.led_index,
        .led_status = {
            .mode = LED_MODE_BLINK,
            .un.blink.period = blink_info.period,
            .un.blink.continue_time = blink_info.continue_time,
            .un.blink.first_status = blink_info.first_status,
            .un.blink.finish_status = blink_info.finish_status,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_led_level(ui_manager_context_t *thiz, led_level_info_t level_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = level_info.led_index,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = level_info.level,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_led_onoff(ui_manager_context_t *thiz, led_onoff_info_t onoff_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = onoff_info.led_index,
        .led_status = {
            .mode = LED_MODE_ONOFF,
            .un.onoff.status = onoff_info.onoff,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void lcd_ready_callback(int type, int status)
{
    EXCEPTION_CHECK_VOID(ui_ctx);
    if (status) { 
        lv_refr_now(NULL);
    }
}

static void on_ui_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    ui_manager_context_t *ui = context;
    EXCEPTION_CHECK_VOID(ui);
    LOG_UI("broadcast id: %d, arg1: %d, arg2: %d", id.code, arg1, arg2);
    switch (id.code) {
        case ID_BROADCAST_PM_WAKEUP_BY_USER:
            if (ui->priv.ui_backlight_val != 100) {
                set_ui_panel_backlight(ui, 100);
            }
            break;
        case ID_BROADCAST_SET_BLACKLIGHT_BY_USER:
            if (ui->priv.ui_backlight_val != 100) {
                set_ui_key_backlight(ui, 100);
            }
            break;
        case ID_BROADCAST_SYS_POWER_STATE_CHANGE:
            if (arg1 == ID_BROADCAST_SYS_POWER_OFF) {
                set_ui_panel_backlight(ui, 0);
                set_ui_key_backlight(ui, 0);
            } else if (arg1 == ID_BROADCAST_SYS_POWER_ON) {
                set_ui_panel_backlight(ui, 100);
                set_ui_key_backlight(ui, 100);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE: 
            if (arg1 == ID_SYS_PM_SUSPEND) {
                ui->priv.is_playing_ui_animation = false;
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
    thiz->priv.lvgl_initialized = false;
    if (thiz->priv.panel_dev) {
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_ADD_READY_CB, lcd_ready_callback);
    }
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
