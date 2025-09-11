/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager_api.h"
#include "windows_manager.h" 

/*
* DEFINES
*****************************************************************************************
*/
#define WM_LOCK(tag)           MUTEX_LOCK_WM((char *)(tag), wm->priv.window_switch_lock);
#define WM_UNLOCK(tag)         MUTEX_UNLOCK_WM((void *)(tag), wm->priv.window_switch_lock);

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static window_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

#if CUST_LVGL_SUPPORT
/// lvgl任务显示调度线程,所有的UI操作均在此线程完成.
static void lv_handler_thread(const void *args)
{
    thread_context_t *thd = args;
    window_manager_context_t *wm = CONTAINER_OF(thd, window_manager_context_t, priv.lv_hdr_thd);
    int msg = 0;

    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, 1, sizeof(int));
    wm->display_update_disable();

    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);

        WM_LOCK(NULL);
        lv_task_handler();
        WM_UNLOCK(NULL);

        #if DEBUG_LV_FPS == 1
        PRINTF_FPS("lv_task");
        #endif
    }
}

static void lv_tick_timer_handler(TimerHandle_t *handle)
{
    lv_tick_inc(LVGL_TIME_TICK_PERIOD);

    #if DEBUG_LV_FPS == 2
    PRINTF_FPS("lv_tick");
    #endif

    static int msg = 0;
    if (thiz->lv_hdr_thd.msg_id) {
        if (++msg % 2 == 0) {
            MSG_QUEUE_SEND(thiz->lv_hdr_thd.msg_id, &msg, 0);
        }
    }
}
#else
void window_flush_thread(const void *args)
{
    uint8_t msg;
    thread_context_t *thd = (thread_context_t *)args;
    // const window_manager_context_t *wm = find_module(ID_MODULE_WM);
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, DISP_DISPATCH_EVT_MESQ_NUM, sizeof(uint8_t));
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        // window_flush();
    }
}

int post_msg_to_flush(const char *tag, uint8_t *msg)
{
    if (!msg) {
        LOGE("##### msg is null ######");
        return -1;
    }
    MSG_QUEUE_SEND(thiz->priv.window_flush_thd.msg_id, &msg, 0);
    return 0;
}
#endif

void local_flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *color_p)
{
    uint32_t buf[] = {
        (uint32_t)thiz->priv.frame_buffer,
        (uint32_t)x1,
        (uint32_t)y1,
        (uint32_t)x2,
        (uint32_t)y2,
        (uint32_t)color_p
    };

    DEVICE_WRITE(thiz->priv.panel_dev, buf, sizeof(buf));
}

void local_clear(uint16_t color)
{
    DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_CLEAR, &color);
}

void window_flush_post_init(window_manager_context_t *wm)
{
    #if CUST_LVGL_SUPPORT
    lv_init();
    lv_port_disp_init((void *)wm);
    lv_port_indev_init((void *)wm);
    lv_animation_init();
    CREATE_TIMER(&wm->priv.lv_tick_timer, "lv_tick_timer", wm, osTimerPeriodic, lv_tick_timer_handler);
    START_TIMER(wm->priv.lv_tick_timer, LVGL_TIME_TICK_PERIOD);
    wm->priv.lv_tick_status = 1;
    RUN_NEW_THREAD(&wm->priv.lv_hdr_thd, "lv_hdr_thd", 
        lv_handler_thread, RT_PRIO_LVGL_HANDLER_THREAD, (LVGL_RENDER_THREAD_STACKSIZE));
    #else
    // window_flush_init();
    RUN_NEW_THREAD(&wm->priv.window_flush_thd, "window_flush_thd",
        window_flush_thread, RT_PRIO_LVGL_HANDLER_THREAD, LVGL_RENDER_THREAD_STACKSIZE);
    #endif //CUST_LVGL_SUPPORT
}

void window_flush_perpare_init(window_manager_context_t *wm)
{
    #if CUST_LVGL_SUPPORT
    uint32_t mem_usage = sizeof(window_manager_context_t);
    ptr->frame_buffer = u_malloz(DISPLAY_FB_SIZE * 2);
    if (ptr->frame_buffer == NULL) {
        LOGE("##### frame_buffer is null ######");
        return -1;
    }
    ptr->draw_buffer = ptr->frame_buffer + DISPLAY_FB_SIZE;
    mem_usage += DISPLAY_FB_SIZE * 2;

    #if IS_SUPPORT_OVERLAY_IMG
    for (uint8_t i = 0; i < OVERLAY_IMG_NUM_MAX; i++) {
        ptr->overlay_img_frame_buffer[i] = u_malloz(DISPLAY_FB_SIZE * 2);
        if (ptr->overlay_img_frame_buffer[i] == NULL) {
            LOGE("##### overlay_img_frame_buffer[%d] is null ######", i);
            return -1;
        }
        ptr->overlay_img_draw_buffer[i] = ptr->overlay_img_frame_buffer[i] + DISPLAY_FB_SIZE;
    }
    mem_usage += DISPLAY_FB_SIZE * 2;
    #endif //IS_SUPPORT_OVERLAY_IMG
    #endif //CUST_LVGL_SUPPORT

    /// 设置全局变量.thiz 需要提前初始化.
    thiz = wm;
}