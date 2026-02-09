/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#define UI_ANIMATION_STOP_TIMEOUT               (1000)
#define IS_PRINT_LVGL_MEM_MONITOR               1
#define UI_LVGL_MSG_QUEUE_NUM                   4
#define UI_BL_ON_LEVEL_MIN                      (20) /// 开灯最低亮度
#define UI_BL_ON_LEVEL_RANGE                    (50) /// 开灯调节范围大小 20 - 50

#if IS_SUPPORT_ADAPTIVE_DIMMING
#define IS_UI_BACKLIGHT_ADAPTIVE                (1)
#else
#define IS_UI_BACKLIGHT_ADAPTIVE                (0)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_manager_context_t *ui_ctx = NULL;

typedef enum {
    UI_LVGL_MSG_CMD_ANIM_INIT = 1,
    UI_LVGL_MSG_CMD_ANIM_DEINIT,
    UI_LVGL_MSG_CMD_OVERLAY_INIT,
    UI_LVGL_MSG_CMD_OVERLAY_DEINIT,
    UI_LVGL_MSG_CMD_PM_SUSPEND,
    UI_LVGL_MSG_CMD_PM_RESUME,
} ui_lvgl_msg_cmd_t;

typedef struct {
    uint8_t cmd;
    uint8_t reserved[3];
    ui_play_info_t play_info;
} ui_lvgl_msg_t;

static void change_images_cb(lv_timer_t *t);
static void reset_play_count_cb(ui_animation_play_t *play);
static void ui_pm_suspend_on_ui_thread(struct ui_manager_context *thiz);

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void set_backlight_level_internal(ui_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz && thiz->priv.panel_dev);
    uint32_t tmp[1] = {level};
    DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_SET_BL, &tmp);
    thiz->priv.ui_backlight_val = level;
    // LOG_UI("set lcd backlight:%d", level);
}

static void set_backlight_level(ui_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.ui_backlight_onoff = level > 0 ? true : false;
    set_backlight_level_internal(thiz, level);
}

static void set_backlight_onoff(ui_manager_context_t *thiz, bool onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.ui_backlight_onoff = onoff;
    LOG_UI("lcd backlight:%d", onoff);
#if (IS_UI_BACKLIGHT_ADAPTIVE == 0)
    set_lcd_backlight_level_internal(thiz, onoff ? 100 : 0);
#endif
}

#if IS_UI_BACKLIGHT_ADAPTIVE
static void ui_backlight_adaptive_thread(void *user_data)
{
    ui_manager_context_t *thiz = (ui_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.ui_backlight_onoff == false) {
        return;
    }
    set_backlight_level_internal(thiz, thiz->priv.ui_backlight_adaptive_val);
}
#endif

static void set_backlight_adaptive_level(ui_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level == 0) {
        thiz->priv.ui_backlight_adaptive_val = UI_BL_ON_LEVEL_MIN;
    } else {
        if (level > 100) {
            level = 100;
        }
        thiz->priv.ui_backlight_adaptive_val = ((level * UI_BL_ON_LEVEL_RANGE) / 100) + UI_BL_ON_LEVEL_MIN;
    }
}

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

static void ui_animation_play_init_on_ui_thread(struct ui_manager_context *thiz, 
    const ui_play_info_t *play_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(play_info);
    EXCEPTION_CHECK_VOID(play_info->tag);

    uint32_t period = 1000;
    if (play_info->fps > 0) {
        period = 1000U / (uint32_t)play_info->fps;
    }
    uint32_t timer_period = (period < 30) ? 30 : period;

    LOG_UI("==>%s, period: %lu", play_info->tag, (unsigned long)timer_period);

    if (thiz->priv.lv_timer != NULL) {
        lv_timer_t *timer_to_delete = thiz->priv.lv_timer;
        thiz->priv.lv_timer = NULL;
        lv_timer_pause(timer_to_delete);
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

    thiz->play_animation_lock(play_info->tag);
    ani->played_count = 0;
    ani->user_data = play_info->user_data;
    ani->progress_cb = play_info->progress_cb;
    ani->tag = (char *)play_info->tag;
    ani->reset_play_count_cb = reset_play_count_cb;
    ani->display_prepare_cb = play_info->display_prepare_cb;
    ani->display_remove_cb = play_info->display_remove_cb;
    thiz->play_animation_unlock(play_info->tag);

    if (ani->display_prepare_cb) {
        lv_lock();
        ani->display_prepare_cb(thiz);
        lv_unlock();
    }

    if (thiz->priv.lv_timer == NULL) {
        thiz->priv.lv_timer = lv_timer_create(change_images_cb, timer_period, NULL);
        if (thiz->priv.lv_timer != NULL) {
            lv_timer_ready(thiz->priv.lv_timer);
        }
        else {
            LOG_UI("==>lv_timer create failed");
        }
    }

    thiz->priv.is_playing_ui_animation = true;
    LOG_UI("==>%s, lv_timer created, period: %lu, timer: %p",
           play_info->tag, (unsigned long)timer_period, thiz->priv.lv_timer);
}

static void ui_animation_play_deinit_on_ui_thread(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    LOG_UI("==> ui deinit tag: %s", ani->tag);
    thiz->play_animation_lock(ani->tag);
    thiz->priv.is_playing_ui_animation = false;
    thiz->play_animation_unlock(ani->tag);
    if (thiz->priv.lv_timer != NULL && lv_timer_get_paused(thiz->priv.lv_timer) == false) {
        lv_timer_t *timer_to_delete = thiz->priv.lv_timer;
        thiz->priv.lv_timer = NULL;  // 先清空，防止回调使用
        lv_timer_pause(timer_to_delete);
        lv_timer_delete(timer_to_delete);
        LOG_UI("==>lv_timer deleted:%p", timer_to_delete);
    }

    if (ani->display_remove_cb) {
        lv_lock();
        ani->display_remove_cb(thiz);
        lv_unlock();
    }
    thiz->play_animation_lock(NULL);
    memset(ani, 0, sizeof(ui_animation_play_t));
    thiz->play_animation_unlock(NULL);
}

static void ui_animation_overlay_init_on_ui_thread(struct ui_manager_context *thiz,
    const ui_play_info_t *play_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(play_info);
    EXCEPTION_CHECK_VOID(play_info->tag);

    uint32_t period = 1000;
    if (play_info->fps > 0) {
        period = 1000U / (uint32_t)play_info->fps;
    }
    uint32_t timer_period = (period < 30) ? 30 : period;
    LOG_UI("==>overlay:%s, period: %lu", play_info->tag, (unsigned long)timer_period);
    ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
    if (ani->display_remove_cb) {
        lv_lock();
        ani->display_remove_cb(thiz);
        lv_unlock();
        LOG_UI("==> overlay remove tag: %s", ani->tag);
    }
    thiz->play_animation_lock(play_info->tag);
    ani->played_count = 0;
    ani->user_data = play_info->user_data;
    ani->progress_cb = play_info->progress_cb;
    ani->tag = (char *)play_info->tag;
    ani->reset_play_count_cb = reset_play_count_cb;
    ani->display_prepare_cb = play_info->display_prepare_cb;
    ani->display_remove_cb = play_info->display_remove_cb;
    thiz->play_animation_unlock(play_info->tag);

    if (ani->display_prepare_cb) {
        lv_lock();
        ani->display_prepare_cb(thiz);
        lv_unlock();
    }

    thiz->priv.is_playing_ui_overlay = true;
}

static void ui_animation_overlay_deinit_on_ui_thread(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    LOG_UI("==> overlay deinit tag: %s", thiz->priv.overlay_play_info.tag);

    ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
    thiz->play_animation_lock(ani->tag);
    thiz->priv.is_playing_ui_overlay = false;
    thiz->play_animation_unlock(ani->tag);
    if (ani->display_remove_cb) {
        lv_lock();
        ani->display_remove_cb(thiz);
        lv_unlock();
    }

    thiz->play_animation_lock(NULL);
    memset(ani, 0, sizeof(ui_animation_play_t));
    thiz->play_animation_unlock(NULL);
}

static void lvgl_display_handler(struct ui_manager_context *thiz)
{
    ui_lvgl_msg_t msg;
    uint32_t ms_till_next = 1;
    uint16_t log_count = 0;

    while (1) {
        const bool timer_running = (thiz->priv.lv_timer != NULL) &&
                                   (lv_timer_get_paused(thiz->priv.lv_timer) == false);
        if (timer_running) {
            ms_till_next = lv_timer_handler();
            if (ms_till_next > 10000) {
                LOGE("time error!!!!!!!: %lu", (unsigned long)ms_till_next);
                ms_till_next = 200;
            }
            if (ms_till_next == 0) {
                ms_till_next = 1;
            }

            log_count++;
            if (log_count % 200 == 0) {
                LOG_UI("lvgl scheduler:%d", log_count);
#if IS_PRINT_LVGL_MEM_MONITOR
                lvgl_mem_monitor_cb();
#endif
            }
        }

        os_status_e ret = MSG_QUEUE_RECV(thiz->priv.ui_hdr_thd.msg_id, &msg,
                                         timer_running ? ms_till_next : OS_WAIT_FOREVER);
        if (ret != 0) {
            continue;
        }
        switch ((ui_lvgl_msg_cmd_t)msg.cmd) {
            case UI_LVGL_MSG_CMD_ANIM_INIT:
                ui_animation_play_init_on_ui_thread(thiz, &msg.play_info);
                break;
            case UI_LVGL_MSG_CMD_ANIM_DEINIT:
                ui_animation_play_deinit_on_ui_thread(thiz);
                break;
            case UI_LVGL_MSG_CMD_OVERLAY_INIT:
                ui_animation_overlay_init_on_ui_thread(thiz, &msg.play_info);
                break;
            case UI_LVGL_MSG_CMD_OVERLAY_DEINIT:
                ui_animation_overlay_deinit_on_ui_thread(thiz);
                break;
            case UI_LVGL_MSG_CMD_PM_SUSPEND:
                ui_pm_suspend_on_ui_thread(thiz);
                break;
            default:
                break;
        }
    }
}

static void change_images_cb(lv_timer_t *t)
{    
    EXCEPTION_CHECK_VOID(ui_ctx);
    if (ui_ctx->priv.lv_timer == NULL || ui_ctx->priv.lv_timer != t) {
        LOG_UI("==>lv_timer is %s:%p", ui_ctx->priv.lv_timer == NULL ? "NULL" : "changed", t);
        return;
    }
        
    /// base 动画先绘制
    ui_ctx->play_animation_lock(NULL);
    if (ui_ctx->priv.is_playing_ui_animation) {
        ui_ctx->play_animation_unlock(NULL);
        ui_animation_play_t *ani = &ui_ctx->priv.play_info;
        if (ani->tag) {
            if (ani->progress_cb) {
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_BEFORE);
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_AFTER);
            }
            ani->played_count++;
        }
    }

    /// overlay 后绘制（保证叠加层在上面）
    ui_ctx->play_animation_lock(NULL);
    if (ui_ctx->priv.is_playing_ui_overlay) {
        ui_ctx->play_animation_unlock(NULL);
        ui_animation_play_t *ani = &ui_ctx->priv.overlay_play_info;
        if (ani->tag) {
            if (ani->progress_cb) {
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_BEFORE);
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_AFTER);
            }
            ani->played_count++;
        }
    }
}

static void reset_play_count_cb(ui_animation_play_t *play)
{
    play->played_count = 0;
}

static void ui_animation_play_init(struct ui_manager_context *thiz, ui_play_info_t play_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    ui_lvgl_msg_t msg = {0};
    msg.cmd = UI_LVGL_MSG_CMD_ANIM_INIT;
    msg.play_info = play_info; /* copy params to UI thread */
    MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void ui_animation_play_deinit(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ui_lvgl_msg_t msg = {0};
    msg.cmd = UI_LVGL_MSG_CMD_ANIM_DEINIT;
    MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void ui_animation_overlay_init(struct ui_manager_context *thiz, ui_play_info_t play_info)
{
    EXCEPTION_CHECK_VOID(thiz);
    ui_lvgl_msg_t msg = {0};
    msg.cmd = UI_LVGL_MSG_CMD_OVERLAY_INIT;
    msg.play_info = play_info; /* copy params to UI thread */
    MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void ui_animation_overlay_deinit(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ui_lvgl_msg_t msg = {0};
    msg.cmd = UI_LVGL_MSG_CMD_OVERLAY_DEINIT;
    MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void ui_pm_suspend_on_ui_thread(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.is_playing_ui_animation = false;
    thiz->priv.is_playing_ui_overlay = false;
    if (thiz->priv.lv_timer != NULL) {
        lv_timer_t *timer_to_delete = thiz->priv.lv_timer;
        thiz->priv.lv_timer = NULL;
        lv_timer_pause(timer_to_delete);
        lv_timer_delete(timer_to_delete);
        LOG_UI("==>pm suspend: lv_timer deleted:%p", timer_to_delete);
    }
}

static void ui_thread_init_route(struct ui_manager_context *thiz)
{
    CREATE_MSG_TYPE_QUEUE(&thiz->priv.ui_hdr_thd.msg_id, UI_LVGL_MSG_QUEUE_NUM, sizeof(ui_lvgl_msg_t));
    thiz->priv.lvgl_initialized = true;
    LOG_UI("==>lvgl init done");
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
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                set_backlight_onoff(ui, false);
            } else if (arg1 == SYSTEM_BROADCAST_POWER_ON) {
                set_backlight_onoff(ui, true);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE: 
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                set_backlight_onoff(ui, false);
                ui_lvgl_msg_t msg = {0};
                msg.cmd = UI_LVGL_MSG_CMD_PM_SUSPEND;
                MSG_QUEUE_SEND(ui->priv.ui_hdr_thd.msg_id, &msg, 0);
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                set_backlight_onoff(ui, true);
            }
            break;
        default:
            break;
    }
}

static int on_post_ui_init_route(struct ui_manager_context *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
#if IS_UI_BACKLIGHT_ADAPTIVE
    ctx->priv.ui_backlight_adaptive_val = UI_BL_ON_LEVEL_MIN;
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_INT(misc);
    misc->add_misc_task(ctx, MISC_TASK_ID_UI_BACKLIGHT_ADAPTIVE, 100, MISC_WORK_MSG_ID_TIMER,
        ui_backlight_adaptive_thread);
#endif
    return 0;
}

static int on_perpare_ui_init_route(struct ui_manager_context *thiz)
{    
    thiz->priv.lvgl_initialized = false;
    if (thiz->priv.panel_dev) {
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_ADD_READY_CB,
            lcd_ready_callback);
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
    ctx->hook->ui_animation_overlay_init = ui_animation_overlay_init;
    ctx->hook->ui_animation_overlay_deinit = ui_animation_overlay_deinit;
    ctx->hook->on_ui_broadcast_receiver_route = on_ui_broadcast_receiver_route;
    ctx->hook->ui_thread_init_route = ui_thread_init_route;
    ctx->hook->lvgl_display_handler = lvgl_display_handler;
    ctx->hook->set_ui_backlight_onoff = set_backlight_onoff;
    ctx->hook->set_ui_backlight_level = set_backlight_level;
    ctx->hook->set_ui_backlight_adaptive_level = set_backlight_adaptive_level;
}
