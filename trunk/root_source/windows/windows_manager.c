/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" 
#include "windows_manager.h" 
#include "windows_internal_api.h"

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
static window_context_record_t s_window_record = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static uint8_t get_current_window_id(void)
{
    return s_window_record.window_hdr ? s_window_record.window_hdr->window_id : 0;
}

static window_context_handle_t *dichotomy_find_window_obj(uint16_t window_id)
{
    int l = 0;
    int m = 0;
    uint16_t tmp = 0;
    uint16_t list_len = 0;
    window_context_handle_t *list;

    get_window_state_hdr_list(&list, &list_len);
    int h = list_len -1;

    while (h >= l) {
        m = (h + l) / 2;
        tmp = list[m].window_id;
        if (tmp == window_id) {
            return &list[m];
        } else if (tmp > window_id) {
            h = m - 1;
        } else if (tmp < window_id) {
            l = m + 1;
        }
    }
    LOG_WM("error! not found window_id: %d", window_id);

    return NULL;
}

static enter_new_window_fptr find_window_handle_by_id(uint8_t window_id)
{
    window_context_handle_t *tmp = dichotomy_find_window_obj(window_id);

    if (tmp) {
        return tmp->new_window_cb;
    }
    LOG_WM("error! not found func window_id : %d", window_id);

    return NULL;
}

static int find_window_z_order_by_id(int window_id)
{
    window_context_record_t *rcd = &s_window_record;
    window_context_handle_t *tmp = dichotomy_find_window_obj(window_id);

    if (tmp) {
        if (rcd->window_hdr && (rcd->window_hdr->window_id == window_id)) {
            if (rcd->z_order > WINDOW_PRIO_LOW) {
                LOG_WM("record z_order: %d", window_id);
                return rcd->z_order;
            }
        }
        return tmp->z_order;
    }
    LOG_WM("error! not found window_id: %d", window_id);

    return 0;
}

static int enter_new_window_policy(uint8_t target_window)
{
    int curr_window = get_current_window_id();
    int curr_z_order = find_window_z_order_by_id(curr_window);
    int target_z_order = find_window_z_order_by_id(target_window);

    if (target_z_order >= curr_z_order) {
        LOG_WM("allow! target_z_order:%d >= curr:%d", target_z_order, curr_z_order);
        return 0;
    }

    LOG_WM("Blocked! z_order: target(%s) < current(%s)", 
        to_window_name(target_window), to_window_name(curr_window));

    return -1;
}

static void *list_del_one_condition_cb(void *data, list_head_t *pos)
{
    window_history_node_t *node = CONTAINER_OF(pos, window_history_node_t, list);
    uint32_t dest_window_id = (uint32_t)data;

    if (node->window_id == dest_window_id) {
        LOG_WM("del: (%s)", to_window_name(node->window_id));
        return node;
    }

    return NULL;
}

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    window_history_node_t *node = CONTAINER_OF(pos, window_history_node_t, list);
    LOG_WM("clear: (%s)", to_window_name(node->window_id));
    return node;
}

static void remove_window_history(uint8_t window_id)
{
    window_manager_context_t *wm = thiz;
    list_head_t *head = &wm->priv.history_list;

    LOG_WM("window_id: %d", window_id);
    MUTEX_LOCK(wm->priv.history_lock);
    int id = window_id;
    __list_del(head, (void *)id, list_del_one_condition_cb);
    DUMP_HISTORY("*****after-remove*****", head);
    MUTEX_UNLOCK(wm->priv.history_lock);
}

static void push_window_history(uint8_t window_id)
{
    window_manager_context_t *wm = thiz;
    list_head_t *head = &wm->priv.history_list;
    window_history_node_t *node;

    if (wm->priv.go_back_history_flag) {
        LOG_WM("ignore:%d", window_id);
        return;
    }
    /// 移除相同的已经压入的窗口,避免重复压入.
    remove_window_history(window_id);

    MUTEX_LOCK(wm->priv.history_lock);
    node = u_malloz(sizeof(window_history_node_t));
    node->window_id = window_id;
    __list_add(&node->list, head);
    DUMP_HISTORY("*****after-push*****", head);
    MUTEX_UNLOCK(wm->priv.history_lock);
}

static uint8_t pop_window_history(void)
{
    window_manager_context_t *wm = thiz;
    list_head_t *head = &wm->priv.history_list;
    window_history_node_t *node;
    uint8_t window_id = 0;

    MUTEX_LOCK(wm->priv.history_lock);
    LIST_FOR_EACH((pos), (head)) {
        node = CONTAINER_OF(pos, window_history_node_t, list);
        window_id = node->window_id;
        wm->priv.go_back_history_flag = 1;
        head->next = pos->next;
        u_free(node);
        break;
    }
    DUMP_HISTORY("*****after-pop*****", head);
    MUTEX_UNLOCK(wm->priv.history_lock);

    return to_valid_window_id(wm, window_id);
}

static void clear_window_history(void)
{
    window_manager_context_t *wm = thiz;
    list_head_t *head = &wm->priv.history_list;

    MUTEX_LOCK(wm->priv.history_lock);
    LIST_EMPTY(head, list_del_condition_cb);
    MUTEX_UNLOCK(wm->priv.history_lock);
}

static int window_switch(window_manager_context_t *wm, multi_func_msg_data_t *msg)
{
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    int new_window_id = is_valid_window_id(msg->union_body.extra.window_id);
    const char *win_name = to_window_name(new_window_id);
    bool is_bg = msg->union_body.extra.is_background == 0x01;
    uint32_t t0 = 0;

    #if IS_SUPPORT_DEVICE_FS
    if (!pbus->fs_ready()) {
        LOG_WM("fs not ready, ignore: %d", new_window_id);
        return -1;
    }
    #endif

    if (wm->priv.window_switch_forbid) {
        LOG_WM("forbid window switch, ignore");
        return -2;
    }

    /// 前台有高优先级窗口情况下，后台拉起优先级低的窗口调度行为将不会被允许.
    if ((is_bg) && (enter_new_window_policy(new_window_id) < 0)) {
        LOG_WM("the bg-dispatch(%s/%d) NOT allow!\n", win_name, new_window_id);
        return -3;
    }

    if (is_the_same_window(new_window_id)) {
        LOG_WM("thd ignore the same win: (%s/%d)", to_window_name(new_window_id), new_window_id);
        return -4;
    }

    if (new_window_id > 0) {
    try_again_dispatch:
        t0 = get_timestamp();
        LOG_WM("((bg:%d)name=%s)start", is_bg, win_name);
        if (new_window_id != ID_WINDOW_DUMMY) {
            if (pm->is_on_suspend_gotoing()) {
                LOG_WM("pm on_suspend_gotoing, ignore: %d", new_window_id);
                return -5;
            }

            if (wm->system_is_ready) {
                SEND_BROADCAST_ARGS1(ID_BROADCAST_PM_WAKEUP_BY_USER, PM_WAKEUP_REASON_ENTER_NEW_SCREEN);
            }
        }
        
        enter_new_window_fptr proc = find_window_handle_by_id(new_window_id);
        if (proc) {
            int ret = proc(wm, new_window_id);
            if (ret == ID_WINDOW_DISPLAY_OK) {
                LOG_WM("enter_new_screen[%s]***", win_name);
            } else if (ret == ID_WINDOW_DISPLAY_TRY_HISTORY) {
                LOG_WM("invalid new_window_id: %d, try history.", new_window_id);
                new_window_id = pop_window_history();
                msleep(100);
                goto try_again_dispatch;
            } else {
                LOG_WM("dispatch_window:ignore: %d", new_window_id);
                return -6;
            }
        } else {
            LOG_WM("dispatch_window:invaild handle, ignore: %d", new_window_id);
            return -7;
        }
        LOG_WM("(dispatch_window:name=%s)finish escaped: %ld ms", win_name, (get_timestamp() - t0));
    } else {
        LOG_WM("dispatch_window:invalid new_window_id: %d", new_window_id);
        return -8;
    }
    return 0;
}

static int window_post_async_msg(window_manager_context_t *wm, multi_func_msg_data_t *msg)
{
    int window_id = is_valid_window_id(msg->union_body.window_evt_data.window_id);

    if (window_id == ID_WINDOW_INVALID) {
        LOGE("ID_WINDOW_INVALID: %d", window_id);
        return -1;
    }

    window_context_t *win_ctx = wm->priv.window_private[window_id];
    if (win_ctx) {
        LOG_WM("win_ctx: %p", win_ctx);
        if (win_ctx->async_evt_handler_cb) {
            msg->union_body.window_evt_data.window_ctx = win_ctx;
            win_ctx->async_evt_handler_cb(&msg->union_body.window_evt_data);
        }
    } else {
        LOGE("win_ctx: %d not found.", window_id);
    }

    return 0;
}

static void window_post_async_key_event(multi_func_msg_data_t *msg)
{
    key_input_context_t *input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    int keycode = msg->union_body.key_event_data.keycode;
    int keytype = msg->union_body.key_event_data.keytype;
    if (input->dispatch_client_input_key) {
        input->dispatch_client_input_key(keycode, keytype);
    }
}

static void window_dispatch_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    window_manager_context_t *wm = CONTAINER_OF(thd, window_manager_context_t, priv.window_dispatch_thd);
    multi_func_msg_data_t *msg = &wm->priv.new_msg;
    CREATE_MSG_QUEUE(&thd->msg_id, DISP_DISPATCH_EVT_MESQ_NUM);
    task_post_process_fptr *hdr_process_cb = NULL;
    REGISTER_TO_HANDLER_POST_POOL(thd->msg_id, &hdr_process_cb);
    task_post_process_fptr *bcr_process_cb = NULL;
    REGISTER_TO_BROCASE_THD_POOL(thd->msg_id, &bcr_process_cb);

    for (;;) {
        thd->is_running = 0;
        MSG_QUEUE_RECV(thd->msg_id, msg, OS_WAIT_FOREVER);
        thd->is_running = get_timestamp();

        switch (msg->msg_type) {
            case MSG_TYPE_WIN_DISPATCH:
                WM_LOCK("--switch");
                window_switch(wm, msg);
                WM_UNLOCK("--switch");
                break;
            case MSG_TYPE_WIN_EVENT:
                WM_LOCK("--async");
                window_post_async_msg(wm, msg);
                WM_UNLOCK("--async");
                break;
            case MSG_TYPE_INPUT_KEY_EVENT:
                WM_LOCK("--input");
                window_post_async_key_event(msg);
                WM_UNLOCK("--input");
                break;
            case MSG_TYPE_HANDLER_POST_EVENT:
                WM_LOCK("--hdr");
                if (hdr_process_cb) {
                    (*hdr_process_cb)(msg, thd);
                }
                WM_UNLOCK("--hdr");
                break;
            case MSG_TYPE_BROADCAST_EVENT:
                WM_LOCK("--bcr");
                if (bcr_process_cb) {
                    (*bcr_process_cb)(msg, thd);
                }
                WM_UNLOCK("--bcr");
                break;
            default:
                break;
        }
    }
}

static void wm_post_hdr_cb(hdr_post_args_t *args)
{
    window_manager_context_t *wm = args->user_data;
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);

    LOG_WM("task name: %s, arg2: %ld, arg1: %ld", args->name, args->arg2, args->arg1);

    if (args->arg2 == ACTION_SET_BL) {
        int counter = 0;
        int val = args->arg1;

        if (val > 0) {
            #if IS_SUPPORT_AIRPLANE_MODE
            airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AP_MODE);
            if (ap && ap->is_on_airplane_mode()) {
                LOG_WM("is_on_airplane_mode not turn on bl");
                return;
            }
            #endif
            
            do {
                LOG_WM("wait display_ready ready: %s, %d", args->name, counter);
                if (wm->priv.display_ready == 1) {
                    break;
                }
                msleep(100);
            } while (++counter < 20);

        } else {
            wm->priv.display_ready = 0;
            LOG_WM("display_ready reset 0");
        }

        if (pm->pm_get_wakeup_reason() == PM_WAKEUP_REASON_IOT) {
            LOG_WM("PM_WAKEUP_REASON_IOT not turn on bl!");
            return;
        }
        wm->set_backlight(val);
    } else if (args->arg2 == ACTION_STOP_LV_TICK_TIMER) {
    #if CUST_LVGL_SUPPORT
        if (pm->is_pm_suspend()) {
            wm->lv_tick_status = 0;
            LOG_WM("=====lv_tick_status: %d=====", wm->lv_tick_status);
            STOP_TIMER(wm->lv_tick_timer);
        }
    #endif
    } else if (args->arg2 == ACTION_BOOT_COMPLETED) {
        LOG_WM("====make up a cut if need====");
        window_boot_completed();
    }
}

/// 接受PM状态切换广播，执行相关状态切换.
static void on_wm_receive_internal(broadcast_mask_id_t id, 
    void *context, int32_t arg1, int32_t arg2, broadcast_priv_data_t *pdata)
{
    window_manager_context_t *wm = thiz;
    
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            {
                int pm_state = arg1;
                if (pm_state == ID_SYS_PM_SUSPEND) {
                    LOG_WM("pm_state : ID_SYS_PM_SUSPEND");
                    clear_window_history();
                #if CUST_LVGL_SUPPORT
                    POST_TASK_HANDLER_DELAYED_ARG5(wm_post_hdr_cb, 
                        100, -1, ACTION_STOP_LV_TICK_TIMER, wm, "lv_tick_stop");
                #endif
                } else {
                    LOG_WM("pm_state : ID_SYS_PM_RESUME");
                    POST_TASK_HANDLER_DELAYED_ARG5(wm_post_hdr_cb, 0, 100, ACTION_SET_BL, thiz, "boot_set_bl-ON2");
                #if CUST_LVGL_SUPPORT
                    STOP_TIMER(wm->lv_tick_timer);
                    START_TIMER(wm->lv_tick_timer, LVGL_TIME_TICK_PERIOD);
                    wm->lv_tick_status = 1;
                    LOG_WM("=====lv_tick_status: %d", wm->lv_tick_status);
                #endif
                }
            }
            break;

        case ID_BROADCAST_PM_WAKEUP_BY_USER:
        case ID_BROADCAST_SET_BLACKLIGHT_BY_USER:
            LOG_WM("SET_BLACKLIGHT_BY_USER");
            POST_TASK_HANDLER_DELAYED_ARG5(wm_post_hdr_cb, 0, 100, ACTION_SET_BL, thiz, "boot_set_bl-ON3");
            break;

        case ID_BROADCAST_SYS_SHUTDOWN:
            LOG_WM("ID_BROADCAST_SYS_SHUTDOWN set_backlight 0");
            wm->set_backlight(0);
            break;
            
        #if IS_SUPPORT_AIRPLANE_MODE
        case ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE:
            LOG_WM("received ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE, arg1: %ld.", arg1);
            if (SETTINGS_AIRPLANE_MODE_ENTER == arg1) {
                clear_window_history();
            }
            break;
            #endif

        default:
            break;
    }
}

/// UI窗口调度分发接口.
static int dispatch_new_window_args(struct window_manager_context *wm, multi_func_msg_data_t *msg)
{
    uint8_t window_id = msg->union_body.extra.window_id;

    if (is_the_same_window(window_id)) {
        LOG_WM("ignore the same scr: (%s/%d)", to_window_name(window_id), window_id);
        return -1;
    }

    int s_bg = (msg->union_body.extra.is_background == 0x01);
    if (!s_bg || (enter_new_window_policy(window_id) == 0)) {
        LOG_WM("dispatch-UI(bg: %d)(scr: %s/%d) allow.\n", s_bg, to_window_name(window_id), window_id);
        if (wm->priv.window_dispatch_thd.msg_id) {
            msg->msg_type = MSG_TYPE_WIN_DISPATCH;
            MSG_QUEUE_SEND(wm->priv.window_dispatch_thd.msg_id, msg, 0);
            return 0;
        }
    }
    LOG_WM("dispatch-UI(bg: %d)(scr: %s/%d) NOT allow!\n", s_bg, to_window_name(window_id), window_id);

    return -1;
}

/// 开机动画.
static void play_boot_animation(void)
{
    window_manager_context_t *wm = thiz;
    do {
        if (wm->priv.window_dispatch_thd.msg_id) {
            break;
        }
        LOG_WM("wait window_dispatch_thd ready..");
        msleep(50);
    } while (1);

    #if IS_SUPPORT_MODULE_RES
    if (wm->priv.res) {
        LOG_WM("wait res ready");
        wm->priv.res->wait_init_completed(wm->priv.res);

    }
    #endif
    
    POST_TASK_HANDLER_DELAYED_ARG5(wm_post_hdr_cb, 500, 100, ACTION_SET_BL,  thiz, "boot_set_bl-ON0");
    multi_func_msg_data_t __msg = {0};
    __msg.union_body.extra.window_id = ID_WINDOW_LOGO;
    LOG_WM("goto logo screen");
    dispatch_new_window_args(wm, &__msg);
    POST_TASK_HANDLER_DELAYED_ARG5(wm_post_hdr_cb, 2000, 0, ACTION_BOOT_COMPLETED,  wm, "boot_completed#task");
}

static int post_async_msg_to_window(const char *tag, multi_func_msg_data_t *msg)
{
    if (!msg) {
        LOGE("null err");   
        return -1;
    }

    if (thiz->priv.window_dispatch_thd.msg_id) {
        if (msg->msg_type == 0) {
            msg->msg_type = MSG_TYPE_WIN_EVENT;
            LOG_WM("def type");
        }

        LOG_WM("tag:%s, msg_type: (%d)", tag, msg->msg_type);
        MSG_QUEUE_SEND(thiz->priv.window_dispatch_thd.msg_id, msg, 0);
    }
    return 0;
}

static void display_device_ready_callback(int type, int status)
{
    thiz->priv.display_ready = 1;
    LOG_WM("display_ready set 1");
    if (type == 1) {
        POST_TASK_HANDLER_DELAYED_ARG5(wm_post_hdr_cb,  0, 100, ACTION_SET_BL, thiz, "boot_set_bl-ON1");
    } 
}

static bool wait_window_ready(int wait_dummy)
{
    uint16_t retry_times = 0;
    /// BEGIN窗口下未唤醒，发广播接收不到，DUMMY窗口下未跑完唤醒流程，发广播会唤醒异常
    while (is_current_window(ID_WINDOW_BEGIN) || (wait_dummy && is_current_window(ID_WINDOW_DUMMY))) {
        if (retry_times > 200) {
            LOGE("wait system ready timeout!");
            return false;
        }
        
        if (retry_times % 25 == 0) {
            LOGW("wait system ready(%d), current(%d)...", retry_times, get_current_window_id());
        }
        retry_times ++;
        msleep(20);
    }

    return true;
}

static int window_enter_state_change_cb(window_lift_cycle_e new_state, 
    const struct window_context *ctx, const struct window_context *old_window) 
{
    window_manager_context_t *wm = ctx->wm;
    if (new_state == ID_SCR_STATE_ON_CREATE) {
        set_window_private_context(ctx->new_window_id, (void *)ctx);
    }

    if (ctx->evt_cb_entry) {
        return ctx->evt_cb_entry(new_state, ctx, old_window);
    }

    return 0;
}

static int window_exit_state_change_cb(window_lift_cycle_e new_state, 
    const struct window_context *ctx, const struct window_context *new_window) 
{
    window_manager_context_t *wm = ctx->wm;
    uint8_t old_window_id = ctx->new_window_id;
    int ret = 0;

    if (ctx->evt_cb_exit) {
        ret = ctx->evt_cb_exit(new_state, ctx, new_window);
    }

    if (new_state == ID_SCR_STATE_ON_DESTORY) {
        if (ret == 0) {
            set_window_private_context(old_window_id, NULL);
        } else {
            LOGW("#window_private[%d] NOT restore!", old_window_id);
        }
    }

    return ret;
}

static int on_post_wm_init(void *ptr)
{
    window_manager_context_t *wm = ptr;
    /// 广播监听PM状态改变.
    wm->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    wm->priv.bcd->context = wm;
    wm->priv.bcd->name = "window_manager";
    wm->priv.bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    wm->priv.bcd->id.code |= ID_BROADCAST_PM_WAKEUP_BY_USER;
    wm->priv.bcd->id.code |= ID_BROADCAST_SET_BLACKLIGHT_BY_USER;
    wm->priv.bcd->id.code |= ID_BROADCAST_SYS_SHUTDOWN;
    wm->priv.bcd->id.code |= ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE;
    wm->priv.bcd->receiver_cb = on_wm_receive_internal;
    register_broadcast_receiver(wm->priv.bcd);
    /// 初始化显示相关.
    window_flush_post_init(wm);
    RUN_NEW_THREAD(&wm->priv.window_dispatch_thd, "window_dispatch_thd", 
        window_dispatch_thread, RT_PRIO_WINDOW_DISPATCH_THREAD, WM_THREAD_STACKSIZE);

    return 0;
}

static int on_perpare_wm_init(void *self)
{
    window_manager_context_t *wm = self;
    
    #if IS_SUPPORT_MODULE_RES
    wm->priv.res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    #endif
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    wm->priv.panel_dev = pbus->find_device(ID_DEVICE_DISPLAY_PANEL);
    DEVICE_IOCTL_ARGS_NO_RET(wm->priv.panel_dev, OP_CODE_PANEL_ADD_READY_CB, display_device_ready_callback);
    /// 初始化显示相关.
    window_flush_perpare_init(wm);
    /// 初始化窗口状态相关.
    on_perpare_window_state_init(wm);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int dispatch_new_window(window_context_t *ctx)
{
    window_context_handle_t *next = NULL;
    window_manager_context_t *wm = ctx->wm;
    window_context_record_t *record = &s_window_record;
    static uint32_t counter = 0;

    next = dichotomy_find_window_obj(ctx->new_window_id);
    if (!next) {
        LOG_WM("not found window_id: %d", ctx->new_window_id);
        return -1;
    }

    if (is_the_same_window(ctx->new_window_id)) {
        LOG_WM("ignore the same screen: %s", to_window_name(ctx->new_window_id));
        return -1;
    }

    uint32_t t0 = get_timestamp();
    LOG_WM("LOG_KEYSTEP switch_windows start: (%s), new_window_id: %d", to_window_name(ctx->new_window_id), (ctx->new_window_id));

    /// 先停掉上一个 scr_obj
    if (record->window_hdr != NULL) {
        LOG_WM("on_pause old scr_obj: (%s)", to_window_name(record->window_hdr->window_id));
        #if IS_SUPPORT_LED_FESTOON_POLICY
        if (wm->led_ctx->stop_play_led_policy_if_need) {
            wm->led_ctx->stop_play_led_policy_if_need();
        }
        #endif
        
        if (record->window_hdr->scr_obj->on_pause) {
            record->window_hdr->scr_obj->on_pause(record->window_context);
        }
        window_exit_state_change_cb(ID_SCR_STATE_ON_PAUSE, record->window_context, ctx);
    }
    /// 更新当前窗口的层级优先级.
    record->z_order = (ctx->z_order > WINDOW_PRIO_LOW) ? ctx->z_order : next->z_order;
    /// 再进入新 scr_obj
    LOG_WM("create new scr_obj:(%s),counter=[%lu]", to_window_name(next->window_id), (++counter));
    window_enter_state_change_cb(ID_SCR_STATE_ON_CREATE, ctx, record->window_context);
    if (next->scr_obj->on_create) {
        next->scr_obj->on_create(ctx);
    }
    
    window_enter_state_change_cb(ID_SCR_STATE_ON_RESUME, ctx, record->window_context);
    if (next->scr_obj->on_resume) {
        next->scr_obj->on_resume(ctx);
    }
    
    /// 释放旧 scr_obj
    if (record->window_hdr != NULL && record->window_context != NULL) {
        if (record->window_context->not_push_history) {
            LOG_WM("not_push_history flag: %s", to_window_name(record->window_hdr->window_id));
        } else if (ctx->new_window_id != ID_WINDOW_DUMMY) {
            push_window_history(record->window_hdr->window_id);
        }
        /// 清除Flag.
        clear_window_go_back_history_flag();
        LOG_WM("destory old scr_obj: (%s)", to_window_name(record->window_hdr->window_id));
        window_exit_state_change_cb(ID_SCR_STATE_ON_DESTORY, record->window_context, ctx);
        if (record->window_hdr->scr_obj->on_destory) {
            record->window_hdr->scr_obj->on_destory();
        }
    } else {
        LOG_WM("Not found prev scr!");
    }
    
    record->window_hdr = next;
    record->window_context = ctx;
    LOGE("LOG_KEYSTEP switch_windows finish: (%s/%d/z_order:%d), escaped: %ld ms", 
        to_window_name(next->window_id), next->window_id, record->z_order, (get_timestamp() - t0));

    return 0;
}


void window_dispatch_lock(const char *tag)
{
    if (tag) {
        LOG_WM("=>Take: %s.", tag);
    }
    MUTEX_LOCK(thiz->priv.ui_draw_lock);
}

void window_dispatch_unlock(const char *tag)
{
    MUTEX_UNLOCK(thiz->priv.ui_draw_lock);

    if (tag) {
        LOG_WM("=>Give: %s.", tag);
    }
}

int go_back_to_history_wrapper(void *wm, window_entry_extra_t *__extra)
{
    window_manager_context_t *_wm = wm;
    multi_func_msg_data_t msg = {0};

    memcpy(&msg.union_body.extra, (__extra), sizeof(window_entry_extra_t));
    msg.union_body.extra.window_id = pop_window_history();

    if (get_current_window_id() == msg.union_body.extra.window_id) {
        msg.union_body.extra.window_id = pop_window_history();
    }

    LOG_WM("GO_BACK_TO_HISTORY: %s\n", to_window_name(msg.union_body.extra.window_id));
    _wm->dispatch_new_window_args(_wm, &(msg));

    return 0;
}

uint8_t to_valid_window_id(void *disp, uint8_t window_id)
{
    if (!(window_id > ID_WINDOW_BEGIN && window_id < ID_WINDOW_MAX)) {
        window_manager_context_t *wm = disp;
        misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
        LOG_WM("%s(window_id:%d)[%d~%d]", to_window_name(window_id), window_id, ID_WINDOW_BEGIN, ID_WINDOW_MAX);
        do {
            if (misc->is_charge_plug_in()) {
                window_id = ID_WINDOW_CHARGE;
                break;
            }
            window_id = is_window_deployed() ? ID_WINDOW_BREATH_PUMP : ID_WINDOW_GUIDE;
        } while (0);
    }
    return window_id;
}

/// 显示管理外部注册接口.
void *register_module_for_window_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, window_manager_context_t);

    ptr->set_backlight = set_window_backlight;
    ptr->set_is_restting_device = set_system_is_restting_device;
    ptr->set_window_switch_forbid = set_window_switch_forbid;
    ptr->system_is_ready = window_system_is_ready;
    ptr->play_boot_animation = play_boot_animation;
    ptr->post_msg_to_window = post_async_msg_to_window;
    ptr->wait_window_ready = wait_window_ready;
    ptr->is_on_window_dispatch = is_on_window_dispatch;
    ptr->is_current_window = is_current_window;
    ptr->to_window_name = to_window_name;
    ptr->get_current_window_id = get_current_window_id;
    ptr->dispatch_new_window_args = dispatch_new_window_args;

    INIT_LIST_HEAD(&ptr->priv.history_list);
    MUTEX_INIT(&ptr->priv.history_lock, history_stack_lock);
    MUTEX_INIT(&ptr->priv.window_switch_lock, window_switch_lock);
    MUTEX_INIT(&ptr->priv.ui_draw_lock, ui_update_draw_lock);
    thiz = ptr;

    return ADD_MODULE("wm", ID_MODULE_WM, on_perpare_wm_init, 
        on_post_wm_init, ptr, list_head, sizeof(window_manager_context_t));
}
