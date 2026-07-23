/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager_api.h"
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define UI_ANIMATION_STOP_TIMEOUT               (1000)
#if IS_SUPPORT_LVGL
#define IS_PRINT_LVGL_MEM_MONITOR               1
#else
#define IS_PRINT_LVGL_MEM_MONITOR               0
#endif
#define UI_LVGL_MSG_QUEUE_NUM                   8

#ifndef UI_THREAD_DBG_ENABLE
#define UI_THREAD_DBG_ENABLE                    1
#endif

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

#if UI_THREAD_DBG_ENABLE
/* UI thread debug: step value locates where UI thread is stuck */
typedef enum {
    UI_DBG_STEP_NONE = 0,
    UI_DBG_STEP_THREAD_INIT,
    UI_DBG_STEP_THREAD_INIT_DONE,

    UI_DBG_STEP_LOOP_START = 100,
    UI_DBG_STEP_LV_TIMER_HANDLER,
    UI_DBG_STEP_LV_TIMER_HANDLER_DONE,
    UI_DBG_STEP_MSG_QUEUE_RECV,
    UI_DBG_STEP_MSG_QUEUE_RECV_TIMEOUT,
    UI_DBG_STEP_MSG_DISPATCH,
    UI_DBG_STEP_MSG_ANIM_INIT,
    UI_DBG_STEP_MSG_ANIM_DEINIT,
    UI_DBG_STEP_MSG_OVERLAY_INIT,
    UI_DBG_STEP_MSG_OVERLAY_DEINIT,
    UI_DBG_STEP_MSG_PM_SUSPEND,

    UI_DBG_STEP_CHANGE_IMAGES_ENTER = 200,
    UI_DBG_STEP_CHANGE_IMAGES_BASE_LOCK,
    UI_DBG_STEP_CHANGE_IMAGES_BASE_DRAW,
    UI_DBG_STEP_CHANGE_IMAGES_OVERLAY_LOCK,
    UI_DBG_STEP_CHANGE_IMAGES_OVERLAY_DRAW,
    UI_DBG_STEP_CHANGE_IMAGES_EXIT,

    UI_DBG_STEP_ANIM_INIT_ENTER = 300,
    UI_DBG_STEP_ANIM_INIT_DEL_TIMER,
    UI_DBG_STEP_ANIM_INIT_REMOVE_CB,
    UI_DBG_STEP_ANIM_INIT_REMOVE_CB_LV_LOCK,
    UI_DBG_STEP_ANIM_INIT_LOCK,
    UI_DBG_STEP_ANIM_INIT_PREPARE_CB,
    UI_DBG_STEP_ANIM_INIT_PREPARE_CB_LV_LOCK,
    UI_DBG_STEP_ANIM_INIT_CREATE_TIMER,
    UI_DBG_STEP_ANIM_INIT_EXIT,

    UI_DBG_STEP_ANIM_DEINIT_ENTER = 400,
    UI_DBG_STEP_ANIM_DEINIT_LOCK,
    UI_DBG_STEP_ANIM_DEINIT_DEL_TIMER,
    UI_DBG_STEP_ANIM_DEINIT_REMOVE_CB,
    UI_DBG_STEP_ANIM_DEINIT_REMOVE_CB_LV_LOCK,
    UI_DBG_STEP_ANIM_DEINIT_CLEAR,
    UI_DBG_STEP_ANIM_DEINIT_EXIT,

    UI_DBG_STEP_OVERLAY_INIT_ENTER = 500,
    UI_DBG_STEP_OVERLAY_INIT_REMOVE_CB,
    UI_DBG_STEP_OVERLAY_INIT_REMOVE_CB_LV_LOCK,
    UI_DBG_STEP_OVERLAY_INIT_LOCK,
    UI_DBG_STEP_OVERLAY_INIT_PREPARE_CB,
    UI_DBG_STEP_OVERLAY_INIT_PREPARE_CB_LV_LOCK,
    UI_DBG_STEP_OVERLAY_INIT_EXIT,

    UI_DBG_STEP_OVERLAY_DEINIT_ENTER = 600,
    UI_DBG_STEP_OVERLAY_DEINIT_LOCK,
    UI_DBG_STEP_OVERLAY_DEINIT_REMOVE_CB,
    UI_DBG_STEP_OVERLAY_DEINIT_REMOVE_CB_LV_LOCK,
    UI_DBG_STEP_OVERLAY_DEINIT_CLEAR,
    UI_DBG_STEP_OVERLAY_DEINIT_EXIT,
} ui_thread_dbg_step_e;

typedef struct {
    volatile uint32_t step;
    volatile uint32_t loop_count;
    volatile uint8_t last_msg_cmd;
    volatile uint8_t timer_running;
    volatile uint8_t is_playing_anim;
    volatile uint8_t is_playing_overlay;
    volatile uint32_t ms_till_next;
    volatile uint32_t anim_tag_ptr;
    volatile uint32_t overlay_tag_ptr;
    volatile uint32_t lv_timer_ptr;
} ui_thread_dbg_info_t;
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_manager_context_t *ui_ctx = NULL;
#if UI_THREAD_DBG_ENABLE
static ui_thread_dbg_info_t s_ui_thread_dbg = {0};

static void ui_dbg_update_runtime_state(struct ui_manager_context *thiz)
{
    if (thiz == NULL) {
        return;
    }
    s_ui_thread_dbg.is_playing_anim = thiz->priv.is_playing_ui_animation ? 1 : 0;
    s_ui_thread_dbg.is_playing_overlay = thiz->priv.is_playing_ui_overlay ? 1 : 0;
    s_ui_thread_dbg.lv_timer_ptr = (uint32_t)(uintptr_t)thiz->priv.lv_timer;
}

static void ui_dbg_print_status(const char *caller)
{
    LOG_UI("[UI_DBG] %s step=%lu loop=%lu msg_cmd=%u timer_run=%u anim=%u overlay=%u "
           "ms_next=%lu lv_timer=0x%08lx anim_tag=0x%08lx overlay_tag=0x%08lx",
           caller,
           (unsigned long)s_ui_thread_dbg.step,
           (unsigned long)s_ui_thread_dbg.loop_count,
           s_ui_thread_dbg.last_msg_cmd,
           s_ui_thread_dbg.timer_running,
           s_ui_thread_dbg.is_playing_anim,
           s_ui_thread_dbg.is_playing_overlay,
           (unsigned long)s_ui_thread_dbg.ms_till_next,
           (unsigned long)s_ui_thread_dbg.lv_timer_ptr,
           (unsigned long)s_ui_thread_dbg.anim_tag_ptr,
           (unsigned long)s_ui_thread_dbg.overlay_tag_ptr);
}

#define UI_DBG_SET_STEP(step_val) \
    do { s_ui_thread_dbg.step = (uint32_t)(step_val); } while (0)
#define UI_DBG_UPDATE_RUNTIME_STATE(thiz) ui_dbg_update_runtime_state(thiz)
#define UI_DBG_PRINT_STATUS(caller) ui_dbg_print_status(caller)
#define UI_DBG_SET_ANIM_TAG(tag_ptr) \
    do { s_ui_thread_dbg.anim_tag_ptr = (uint32_t)(uintptr_t)(tag_ptr); } while (0)
#define UI_DBG_SET_OVERLAY_TAG(tag_ptr) \
    do { s_ui_thread_dbg.overlay_tag_ptr = (uint32_t)(uintptr_t)(tag_ptr); } while (0)
#define UI_DBG_INC_LOOP_COUNT() \
    do { s_ui_thread_dbg.loop_count++; } while (0)
#define UI_DBG_SET_TIMER_RUNNING(v) \
    do { s_ui_thread_dbg.timer_running = (v) ? 1 : 0; } while (0)
#define UI_DBG_SET_MS_TILL_NEXT(v) \
    do { s_ui_thread_dbg.ms_till_next = (v); } while (0)
#define UI_DBG_SET_LAST_MSG_CMD(v) \
    do { s_ui_thread_dbg.last_msg_cmd = (v); } while (0)
#else
#define UI_DBG_SET_STEP(step_val)            ((void)0)
#define UI_DBG_UPDATE_RUNTIME_STATE(thiz)    ((void)0)
#define UI_DBG_PRINT_STATUS(caller)          ((void)0)
#define UI_DBG_SET_ANIM_TAG(tag_ptr)         ((void)0)
#define UI_DBG_SET_OVERLAY_TAG(tag_ptr)      ((void)0)
#define UI_DBG_INC_LOOP_COUNT()              ((void)0)
#define UI_DBG_SET_TIMER_RUNNING(v)          ((void)0)
#define UI_DBG_SET_MS_TILL_NEXT(v)           ((void)0)
#define UI_DBG_SET_LAST_MSG_CMD(v)           ((void)0)
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void reset_play_count_cb(ui_animation_play_t *play);
#if IS_SUPPORT_LVGL

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

static void change_images_cb(lv_timer_t *t)
{    
    UI_DBG_SET_STEP(UI_DBG_STEP_CHANGE_IMAGES_ENTER);
    EXCEPTION_CHECK_VOID(ui_ctx);
    if (ui_ctx->priv.lv_timer == NULL || ui_ctx->priv.lv_timer != t) {
        LOG_UI("==>lv_timer is %s:%p", ui_ctx->priv.lv_timer == NULL ? "NULL" : "changed", t);
        return;
    }
        
    /// base 动画先绘制
    UI_DBG_SET_STEP(UI_DBG_STEP_CHANGE_IMAGES_BASE_LOCK);
    ui_ctx->play_animation_lock(NULL);
    UI_DBG_SET_STEP(UI_DBG_STEP_CHANGE_IMAGES_BASE_DRAW);
    if (ui_ctx->priv.is_playing_ui_animation) {
        ui_animation_play_t *ani = &ui_ctx->priv.play_info;
        if (ani->tag) {
            UI_DBG_SET_ANIM_TAG(ani->tag);
            if (ani->progress_cb) {
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_BEFORE);
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_AFTER);
            }
            ani->played_count++;
        }
    }
    ui_ctx->play_animation_unlock(NULL);

    /// overlay 后绘制（保证叠加层在上面）
    UI_DBG_SET_STEP(UI_DBG_STEP_CHANGE_IMAGES_OVERLAY_LOCK);
    ui_ctx->play_animation_lock(NULL);
    UI_DBG_SET_STEP(UI_DBG_STEP_CHANGE_IMAGES_OVERLAY_DRAW);
    if (ui_ctx->priv.is_playing_ui_overlay) {
        ui_animation_play_t *ani = &ui_ctx->priv.overlay_play_info;
        if (ani->tag) {
            UI_DBG_SET_OVERLAY_TAG(ani->tag);
            if (ani->progress_cb) {
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_BEFORE);
                ani->progress_cb(ui_ctx, ani->played_count, ANI_TIMER_DRAW_AFTER);
            }
            ani->played_count++;
        }
    }
    ui_ctx->play_animation_unlock(NULL);
    UI_DBG_SET_STEP(UI_DBG_STEP_CHANGE_IMAGES_EXIT);
}

static void ui_animation_play_init_on_ui_thread(struct ui_manager_context *thiz, 
    const ui_play_info_t *play_info)
{
    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_ENTER);
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(play_info);
    EXCEPTION_CHECK_VOID(play_info->tag);
    UI_DBG_SET_ANIM_TAG(play_info->tag);

    uint32_t period = 1000;
    if (play_info->fps > 0) {
        period = 1000U / (uint32_t)play_info->fps;
    }
    uint32_t timer_period = (period < 30) ? 30 : period;

    LOG_UI("==>%s, period: %lu", play_info->tag, (unsigned long)timer_period);

    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_DEL_TIMER);
    if (thiz->priv.lv_timer != NULL) {
        lv_timer_t *timer_to_delete = thiz->priv.lv_timer;
        thiz->priv.lv_timer = NULL;
        lv_timer_pause(timer_to_delete);
        lv_timer_delete(timer_to_delete);
    }

    ui_animation_play_t *ani = &thiz->priv.play_info;
    if (ani->display_remove_cb) {
        UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_REMOVE_CB_LV_LOCK);
        lv_lock();
        UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_REMOVE_CB);
        ani->display_remove_cb(thiz);
        lv_unlock();
        LOG_UI("==> ui remove tag: %s", ani->tag);
        // lv_refr_now(NULL);
    }

    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_LOCK);
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
        UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_PREPARE_CB_LV_LOCK);
        lv_lock();
        UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_PREPARE_CB);
        ani->display_prepare_cb(thiz);
        lv_unlock();
    }

    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_CREATE_TIMER);
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
    UI_DBG_UPDATE_RUNTIME_STATE(thiz);
    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_INIT_EXIT);
    LOG_UI("==>%s, lv_timer created, period: %lu, timer: %p",
           play_info->tag, (unsigned long)timer_period, thiz->priv.lv_timer);
}

static void ui_animation_play_deinit_on_ui_thread(struct ui_manager_context *thiz)
{
    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_DEINIT_ENTER);
    EXCEPTION_CHECK_VOID(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    LOG_UI("==> ui deinit tag: %s", ani->tag);
    UI_DBG_SET_ANIM_TAG(ani->tag);
    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_DEINIT_LOCK);
    thiz->play_animation_lock(ani->tag);
    thiz->priv.is_playing_ui_animation = false;
    thiz->play_animation_unlock(ani->tag);
    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_DEINIT_DEL_TIMER);
    if (thiz->priv.lv_timer != NULL && lv_timer_get_paused(thiz->priv.lv_timer) == false) {
        lv_timer_t *timer_to_delete = thiz->priv.lv_timer;
        thiz->priv.lv_timer = NULL;  // 先清空，防止回调使用
        lv_timer_pause(timer_to_delete);
        lv_timer_delete(timer_to_delete);
        LOG_UI("==>lv_timer deleted:%p", timer_to_delete);
    }

    if (ani->display_remove_cb) {
        UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_DEINIT_REMOVE_CB_LV_LOCK);
        lv_lock();
        UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_DEINIT_REMOVE_CB);
        ani->display_remove_cb(thiz);
        lv_unlock();
    }
    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_DEINIT_CLEAR);
    thiz->play_animation_lock(NULL);
    memset(ani, 0, sizeof(ui_animation_play_t));
    thiz->play_animation_unlock(NULL);
    UI_DBG_UPDATE_RUNTIME_STATE(thiz);
    UI_DBG_SET_STEP(UI_DBG_STEP_ANIM_DEINIT_EXIT);
}

static void ui_animation_overlay_init_on_ui_thread(struct ui_manager_context *thiz,
    const ui_play_info_t *play_info)
{
    UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_INIT_ENTER);
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(play_info);
    EXCEPTION_CHECK_VOID(play_info->tag);
    UI_DBG_SET_OVERLAY_TAG(play_info->tag);

    uint32_t period = 1000;
    if (play_info->fps > 0) {
        period = 1000U / (uint32_t)play_info->fps;
    }
    uint32_t timer_period = (period < 30) ? 30 : period;
    LOG_UI("==>overlay:%s, period: %lu", play_info->tag, (unsigned long)timer_period);
    ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
    if (ani->display_remove_cb) {
        UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_INIT_REMOVE_CB_LV_LOCK);
        lv_lock();
        UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_INIT_REMOVE_CB);
        ani->display_remove_cb(thiz);
        lv_unlock();
        LOG_UI("==> overlay remove tag: %s", ani->tag);
    }
    UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_INIT_LOCK);
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
        UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_INIT_PREPARE_CB_LV_LOCK);
        lv_lock();
        UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_INIT_PREPARE_CB);
        ani->display_prepare_cb(thiz);
        lv_unlock();
    }
    thiz->priv.is_playing_ui_overlay = true;
    UI_DBG_UPDATE_RUNTIME_STATE(thiz);
    UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_INIT_EXIT);
}

static void ui_pm_suspend_on_ui_thread(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.lv_timer != NULL) {
        lv_timer_pause(thiz->priv.lv_timer);
    }
    LOG_UI("==>pm suspend, lv_timer paused");
}

static void ui_pm_resume_on_ui_thread(struct ui_manager_context *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (thiz->priv.lv_timer != NULL) {
        lv_timer_resume(thiz->priv.lv_timer);
        lv_timer_ready(thiz->priv.lv_timer);
    }
    LOG_UI("==>pm resume, lv_timer resumed");
}

static void ui_animation_overlay_deinit_on_ui_thread(struct ui_manager_context *thiz)
{
    UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_DEINIT_ENTER);
    EXCEPTION_CHECK_VOID(thiz);
    LOG_UI("==> overlay deinit tag: %s", thiz->priv.overlay_play_info.tag);
    UI_DBG_SET_OVERLAY_TAG(thiz->priv.overlay_play_info.tag);

    ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
    UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_DEINIT_LOCK);
    thiz->play_animation_lock(ani->tag);
    thiz->priv.is_playing_ui_overlay = false;
    thiz->play_animation_unlock(ani->tag);
    if (ani->display_remove_cb) {
        UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_DEINIT_REMOVE_CB_LV_LOCK);
        lv_lock();
        UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_DEINIT_REMOVE_CB);
        ani->display_remove_cb(thiz);
        lv_unlock();
    }

    UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_DEINIT_CLEAR);
    thiz->play_animation_lock(NULL);
    memset(ani, 0, sizeof(ui_animation_play_t));
    thiz->play_animation_unlock(NULL);
    UI_DBG_UPDATE_RUNTIME_STATE(thiz);
    UI_DBG_SET_STEP(UI_DBG_STEP_OVERLAY_DEINIT_EXIT);
}

static void lvgl_display_handler(struct ui_manager_context *thiz)
{
    ui_lvgl_msg_t msg;
    uint32_t ms_till_next = 1;
    uint16_t log_count = 0;

    while (1) {
        UI_DBG_INC_LOOP_COUNT();
        UI_DBG_SET_STEP(UI_DBG_STEP_LOOP_START);
        UI_DBG_UPDATE_RUNTIME_STATE(thiz);

        const bool timer_running = (thiz->priv.lv_timer != NULL) &&
                                   (lv_timer_get_paused(thiz->priv.lv_timer) == false);
        UI_DBG_SET_TIMER_RUNNING(timer_running);
        if (timer_running) {
            UI_DBG_SET_STEP(UI_DBG_STEP_LV_TIMER_HANDLER);
            ms_till_next = lv_timer_handler();
            UI_DBG_SET_MS_TILL_NEXT(ms_till_next);
            UI_DBG_SET_STEP(UI_DBG_STEP_LV_TIMER_HANDLER_DONE);
            if (ms_till_next > 10000) {
                LOGE("time error!!!!!!!: %lu", (unsigned long)ms_till_next);
                ms_till_next = 200;
                UI_DBG_SET_MS_TILL_NEXT(ms_till_next);
            }

            if (ms_till_next == 0) {
                ms_till_next = 1;
                UI_DBG_SET_MS_TILL_NEXT(ms_till_next);
            }

            log_count++;
            if (log_count % 200 == 0) {
                LOG_UI("lvgl scheduler:%d", log_count);
#if IS_PRINT_LVGL_MEM_MONITOR
                lvgl_mem_monitor_cb();
#endif
            }
        }

        UI_DBG_SET_STEP(UI_DBG_STEP_MSG_QUEUE_RECV);
        os_status_e ret = MSG_QUEUE_RECV(thiz->priv.ui_hdr_thd.msg_id, &msg,
                                         timer_running ? ms_till_next : OS_WAIT_FOREVER);
        if (ret != 0) {
            UI_DBG_SET_STEP(UI_DBG_STEP_MSG_QUEUE_RECV_TIMEOUT);
            continue;
        }

        UI_DBG_SET_LAST_MSG_CMD(msg.cmd);
        UI_DBG_SET_STEP(UI_DBG_STEP_MSG_DISPATCH);
        switch ((ui_lvgl_msg_cmd_t)msg.cmd) {
            case UI_LVGL_MSG_CMD_ANIM_INIT:
                UI_DBG_SET_STEP(UI_DBG_STEP_MSG_ANIM_INIT);
                ui_animation_play_init_on_ui_thread(thiz, &msg.play_info);
                break;
            case UI_LVGL_MSG_CMD_ANIM_DEINIT:
                UI_DBG_SET_STEP(UI_DBG_STEP_MSG_ANIM_DEINIT);
                ui_animation_play_deinit_on_ui_thread(thiz);
                break;
            case UI_LVGL_MSG_CMD_OVERLAY_INIT:
                UI_DBG_SET_STEP(UI_DBG_STEP_MSG_OVERLAY_INIT);
                ui_animation_overlay_init_on_ui_thread(thiz, &msg.play_info);
                break;
            case UI_LVGL_MSG_CMD_OVERLAY_DEINIT:
                UI_DBG_SET_STEP(UI_DBG_STEP_MSG_OVERLAY_DEINIT);
                ui_animation_overlay_deinit_on_ui_thread(thiz);
                break;
            case UI_LVGL_MSG_CMD_PM_SUSPEND:
                UI_DBG_SET_STEP(UI_DBG_STEP_MSG_PM_SUSPEND);
                ui_pm_suspend_on_ui_thread(thiz);
                break;
            case UI_LVGL_MSG_CMD_PM_RESUME:
                ui_pm_resume_on_ui_thread(thiz);
                break;
            default:
                break;
        }
    }
}
#endif

static void set_backlight_level_internal(ui_manager_context_t *thiz, uint16_t level)
{
    if (thiz->priv.panel_dev) {
        uint32_t tmp[1] = {level};
        DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_SET_BL, &tmp);
    }
    thiz->priv.ui_backlight_level = level;
}

static void set_backlight_level(ui_manager_context_t *thiz, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    uint8_t level_with_rate = level * thiz->priv.ui_backlight_level_rate / 100;
    if (level_with_rate > LCD_BACKLIGHT_LEVEL_MAX) {
        level_with_rate = LCD_BACKLIGHT_LEVEL_MAX;
    }
    set_backlight_level_internal(thiz, level_with_rate);
}

static void set_backlight_level_rate(ui_manager_context_t *thiz, uint8_t rate)
{
    if (rate < 20 || rate > 100) {
        LOGE("set ui backlight rate error");
        return;
    }

    if (thiz->priv.ui_backlight_level_rate == rate) {
        return;
    }
    LOG_UI("set ui backlight level rate: %d", rate);
    thiz->priv.ui_backlight_level_rate = rate;
}

static uint8_t get_backlight_level_rate(ui_manager_context_t *thiz)
{
    return thiz->priv.ui_backlight_level_rate;
}

#if IS_UI_BACKLIGHT_ADAPTIVE
static void ui_backlight_adaptive_thread(void *user_data)
{
    ui_manager_context_t *thiz = (ui_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(thiz);
#if IS_SUPPORT_ADAPTIVE_DIMMING
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);    
    EXCEPTION_CHECK_VOID(led);
    // 按键和屏背光自适应开关共用，底层有分开实现
    if (!led->hook->get_key_backlight_adaptive_dimming_switch(led)) {
        return;
    }
#endif
    if (thiz->priv.ui_backlight_level == 0) {
        return;
    }

    uint8_t level_with_rate = thiz->priv.ui_backlight_adaptive_val * thiz->priv.ui_backlight_level_rate / 100;
    if (level_with_rate > LCD_BACKLIGHT_LEVEL_MAX) {
        level_with_rate = LCD_BACKLIGHT_LEVEL_MAX;
    } else if (level_with_rate < LCD_BACKLIGHT_LEVEL_MIN) {
        level_with_rate = LCD_BACKLIGHT_LEVEL_MIN;
    }

    if (thiz->priv.ui_backlight_level == level_with_rate) {
        return;
    }
    LOG_UI("level:%d/%d, rate: %d", level_with_rate, thiz->priv.ui_backlight_adaptive_val,
        thiz->priv.ui_backlight_level_rate);
    set_backlight_level_internal(thiz, level_with_rate);
}

static void ui_backlight_adaptive_level_update(ui_manager_context_t *thiz, uint8_t level)
{
    #if IS_SUPPORT_ADAPTIVE_DIMMING
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);    
    EXCEPTION_CHECK_VOID(led);
    // 按键和屏背光自适应开关共用，底层有分开实现
    if (!led->hook->get_key_backlight_adaptive_dimming_switch(led)) {
        return;
    }
#endif
    if (level > LCD_BACKLIGHT_LEVEL_MAX) {
        level = LCD_BACKLIGHT_LEVEL_MAX;
    }
    
    if (thiz->priv.ui_backlight_adaptive_val == level) {
        return;
    }

    thiz->priv.ui_backlight_adaptive_val = level;
    // LOG_UI("level update: %d", level);
}
#endif

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
    thiz->play_animation_lock(NULL);
    thiz->priv.is_playing_ui_animation = false;
    thiz->priv.play_info.progress_cb = NULL;
    thiz->priv.play_info.user_data = NULL;
    thiz->play_animation_unlock(NULL);

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
    /* 不在调用线程抢 ui_draw_lock，实际清理在 UI 线程 OVERLAY_DEINIT 中完成 */
    thiz->priv.is_playing_ui_overlay = false;
    thiz->priv.overlay_play_info.progress_cb = NULL;
    thiz->priv.overlay_play_info.user_data = NULL;

    ui_lvgl_msg_t msg = {0};
    msg.cmd = UI_LVGL_MSG_CMD_OVERLAY_DEINIT;
    MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void ui_thread_init_route(struct ui_manager_context *thiz)
{
    UI_DBG_SET_STEP(UI_DBG_STEP_THREAD_INIT);
    CREATE_MSG_TYPE_QUEUE(&thiz->priv.ui_hdr_thd.msg_id, UI_LVGL_MSG_QUEUE_NUM, sizeof(ui_lvgl_msg_t));
    thiz->priv.lvgl_initialized = true;
#if IS_SUPPORT_LVGL
    ui_snap_shot_perpare(thiz);
#endif
    UI_DBG_SET_STEP(UI_DBG_STEP_THREAD_INIT_DONE);
    LOG_UI("==>lvgl init done");
}

static void lcd_ready_callback(int type, int status)
{
    EXCEPTION_CHECK_VOID(ui_ctx);
    if (status) { 
#if IS_SUPPORT_LVGL
        lv_refr_now(NULL);
#endif
    }
}

static void on_ui_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1,
    int arg2, broadcast_priv_data_t *pdata)
{
    ui_manager_context_t *ui = context;
    EXCEPTION_CHECK_VOID(ui);
    LOG_UI("broadcast id: %d, arg1: %d, arg2: %d", id.code, arg1, arg2);
    switch (id.code) {
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                set_backlight_level(ui, 0);
            } else if (arg1 == SYSTEM_BROADCAST_POWER_ON) {
                set_backlight_level(ui, LCD_BACKLIGHT_LEVEL_MAX);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE: 
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                set_backlight_level(ui, 0);
                ui_lvgl_msg_t msg = {0};
                msg.cmd = UI_LVGL_MSG_CMD_PM_SUSPEND;
                MSG_QUEUE_SEND(ui->priv.ui_hdr_thd.msg_id, &msg, 0);
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                set_backlight_level(ui, LCD_BACKLIGHT_LEVEL_MAX);
                ui_lvgl_msg_t msg = {0};
                msg.cmd = UI_LVGL_MSG_CMD_PM_RESUME;
                MSG_QUEUE_SEND(ui->priv.ui_hdr_thd.msg_id, &msg, 0);
            }
            break;
        default:
            break;
    }
}

static void on_ui_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_manager_context_t *ui = (ui_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(ui);
    UI_DBG_PRINT_STATUS("battery_cb");
    if (info->battery_state == BATTERY_STATE_EMPTY || 
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        info->battery_state == BATTERY_STATE_FORBID_START ||
        info->battery_state == BATTERY_STATE_LOW_POWER_RUN) {
        set_backlight_level_rate(ui, 50);
    } else {
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        if (wm && wm->get_current_window_id() != ID_WINDOW_POWER_OFF) {
            set_backlight_level_rate(ui, 100);
        }
    }
    LOG_UI("batt state: %d, bg rate: %d", info->battery_state, ui->priv.ui_backlight_level_rate);
}

static int on_post_ui_init_route(struct ui_manager_context *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    ctx->priv.ui_backlight_level_rate = 100;
#if IS_UI_BACKLIGHT_ADAPTIVE
    ctx->priv.ui_backlight_adaptive_val = LCD_BACKLIGHT_LEVEL_MIN;
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_INT(misc);
    misc->add_misc_task(ctx, MISC_TASK_ID_UI_BACKLIGHT_ADAPTIVE, 100, MISC_WORK_MSG_ID_TIMER,
        ui_backlight_adaptive_thread);
#endif
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    ADD_BATTERY_STATE_CB(battery, ctx, on_ui_battery_state_change_cb);
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
#if IS_SUPPORT_LVGL
    ctx->hook->lvgl_display_handler = lvgl_display_handler;
#endif
    ctx->hook->set_ui_backlight_level = set_backlight_level;
    ctx->hook->set_ui_backlight_level_rate = set_backlight_level_rate;
#if IS_UI_BACKLIGHT_ADAPTIVE
    ctx->hook->ui_backlight_adaptive_level_update = ui_backlight_adaptive_level_update;
#endif
}
