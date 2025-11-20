/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" 
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_play_animation_lock(const char *tag)
{
    if (tag) {
        LOG_UI("=>Take: %s.", tag);
    }
    MUTEX_LOCK(thiz->priv.ui_draw_lock);
}

static void ui_play_animation_unlock(const char *tag)
{
    MUTEX_UNLOCK(thiz->priv.ui_draw_lock);
    if (tag) {
        LOG_UI("=>Give: %s.", tag);
    }
}

static int post_msg_to_flush(const char *tag, multi_func_msg_data_t *msg)
{
    EXCEPTION_CHECK_INT(msg);
    return MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void animation_play_status_update(ui_manager_context_t *thiz, bool flag)
{
    EXCEPTION_CHECK_VOID(thiz);
    thiz->priv.is_playing_ui_animation = flag;
}

static void lv_tick_timer_handler(void *args)
{
   uint8_t msg = 0;
   MSG_QUEUE_SEND(thiz->priv.ui_hdr_thd.msg_id, &msg, 0);
}

static void ui_handler_thread(const void *args)
{
    uint8_t msg = 0;
    thread_context_t *thd = (thread_context_t *)args;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, DISP_DISPATCH_EVT_MESQ_NUM, sizeof(uint8_t));
    while (1) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        if (thiz->hook && thiz->hook->lvgl_display_handler) {
            thiz->hook->lvgl_display_handler(thiz);
        }
    }
}

static void on_ui_broadcast_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    ui_manager_context_t *ui = (ui_manager_context_t *)context;
    if (ui->hook && ui->hook->on_ui_broadcast_receiver_route) {
        ui->hook->on_ui_broadcast_receiver_route(id, context, arg1, arg2, pdata);
    }
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE: 
            if (arg1 == ID_SYS_PM_SUSPEND) {
                ui->priv.is_playing_ui_animation = false;
                STOP_TIMER(ui->priv.lv_tick_timer);
                ui->priv.lv_tick_timer_running = false;
            }
            else {
                STOP_TIMER(ui->priv.lv_tick_timer);
                START_TIMER(ui->priv.lv_tick_timer, LVGL_TIME_TICK_PERIOD);
                ui->priv.lv_tick_timer_running = true;
            }
            break;

        default:
            break;
    }
}

static int on_post_ui_init(void *self)
{
    ui_manager_context_t *ui = (ui_manager_context_t *)self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    if (pbus) {
        thiz->priv.panel_dev = pbus->find_device(ID_DEVICE_DISPLAY_PANEL);
    }
    /// 调用外部钩子.
    if (thiz->hook->on_post_init_route) {
        thiz->hook->on_post_init_route(thiz);
    }
    /// 广播监听PM状态改变.
    ui->priv.m_bcd.context = ui;
    ui->priv.m_bcd.name = "ui_manager";
    ui->priv.m_bcd.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ui->priv.m_bcd.id.code |= ID_BROADCAST_PM_WAKEUP_BY_USER;
    ui->priv.m_bcd.id.code |= ID_BROADCAST_SET_BLACKLIGHT_BY_USER;
    ui->priv.m_bcd.id.code |= ID_BROADCAST_SYS_POWER_STATE_CHANGE;
    ui->priv.m_bcd.receiver_cb = on_ui_broadcast_receiver;
    register_broadcast_receiver(&ui->priv.m_bcd);
    CREATE_TIMER(&ui->priv.lv_tick_timer, "lv_tick_timer", ui, 
        OS_TIMER_PERIODIC, lv_tick_timer_handler);
    START_TIMER(ui->priv.lv_tick_timer, LVGL_TIME_TICK_PERIOD);
    RUN_NEW_THREAD(&thiz->priv.ui_hdr_thd, "ui_hdr_thd", 
        ui_handler_thread, RT_PRIO_LVGL_HANDLER_THREAD, (LVGL_RENDER_THREAD_STACKSIZE));
    return 0;
}

static int on_perpare_ui_init(void *self)
{
    thiz = self;
    if (thiz->hook->on_perpare_init_route) {
        thiz->hook->on_perpare_init_route(thiz);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
/// 显示管理外部注册接口.
void *register_module_for_ui_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, ui_manager_context_t);
    MUTEX_INIT(&ptr->priv.ui_draw_lock, ui_draw_lock);
    ptr->animation_play_status_update = animation_play_status_update;
    ptr->play_animation_lock = ui_play_animation_lock;
    ptr->play_animation_unlock = ui_play_animation_unlock;
    thiz = ptr;
    return ADD_MODULE("ui", ID_MODULE_UI, on_perpare_ui_init, 
        on_post_ui_init, ptr, list_head, sizeof(ui_manager_context_t));
}
