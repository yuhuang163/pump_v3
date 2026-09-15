/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" 
#include "windows_manager.h" 
#include "windows_manager_api.h"
#include "windows_map_define.h" // IWYU pragma: keep

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
static int window_exit_state_change_cb(window_lift_cycle_e new_state,
    const struct window_context *ctx, const struct window_context *new_window);
static int window_enter_state_change_cb(window_lift_cycle_e new_state,
    const struct window_context *ctx, const struct window_context *old_window);

static uint8_t get_current_window_id(void)
{
    return s_window_record.window_hdr ? s_window_record.window_hdr->window_id : ID_WINDOW_INVALID;
}

static uint8_t get_next_window_id(void)
{
    return s_window_record.next_window_id;
}

static uint8_t get_overlay_window_id(struct window_manager_context *wm)
{
    (void)wm;
    return s_window_record.base_window_hdr ? s_window_record.base_window_hdr->window_id : ID_WINDOW_INVALID;
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

static int to_valid_window_id(window_manager_context_t *wm, uint8_t window_id)
{
    if (!(window_id > ID_WINDOW_BEGIN && window_id < ID_WINDOW_MAX)) {
        LOG_WM("%s(window_id:%d)[%d~%d]", to_window_name(window_id), window_id, ID_WINDOW_BEGIN, ID_WINDOW_MAX);
        do {
            battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
            if (battery && BATTERY_IS_CHARGING(battery->get_charge_status())) {
                window_id = ID_WINDOW_CHARGE;
                break;
            }
            window_id = wm->is_window_guidance_deployed() ? ID_WINDOW_PUMP : ID_WINDOW_GUIDE;
        } while (0);
    }
    return window_id;
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

#if WM_DEFER_POPUP_OVERLAY_UI_REMOVE
static void overlay_ui_deinit(void)
{
#if IS_SUPPORT_LVGL
    ui_manager_context_t *ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    if (ui && ui->hook && ui->hook->ui_animation_overlay_deinit) {
        ui->hook->ui_animation_overlay_deinit(ui);
    }
#endif
}
#endif

static int close_popup_overlay_internal(window_manager_context_t *wm, uint8_t keep_overlay_ui)
{
    window_context_record_t *record = &s_window_record;
    if (record->base_window_context == NULL || record->base_window_hdr == NULL) {
        return -1;
    }
    window_context_handle_t *popup_hdr = record->window_hdr;
    window_context_t *popup_ctx = record->window_context;
    window_context_handle_t *base_hdr = record->base_window_hdr;
    window_context_t *base_ctx = record->base_window_context;
    /// 校验 popup 窗口是否存在且为 popup 窗口
    if (popup_hdr == NULL || popup_ctx == NULL || popup_ctx->is_popup_window == 0) {
        return -2;
    }
    LOG_WM("close overlay popup: (%s) -> base(%s), keep_ui:%d",
           to_window_name(popup_hdr->window_id),
           base_hdr ? to_window_name(base_hdr->window_id) : "NULL",
           keep_overlay_ui);
    /// 退出 popup（仅销毁 popup，不触碰 base）
    if (popup_hdr->scr_obj && popup_hdr->scr_obj->on_pause) {
        popup_hdr->scr_obj->on_pause(popup_ctx);
    }
    /// 退出 popup 窗口生命周期回调
    window_exit_state_change_cb(ID_SCR_STATE_ON_PAUSE, popup_ctx, base_ctx);
    /// 销毁 popup 窗口生命周期回调
    window_exit_state_change_cb(ID_SCR_STATE_ON_DESTORY, popup_ctx, base_ctx);
    /// 销毁 popup 窗口回调
    if (popup_hdr->scr_obj && popup_hdr->scr_obj->on_destory) {
        popup_hdr->scr_obj->on_destory();
    }
    /// 恢复 base 为当前窗口
    record->window_hdr = base_hdr;
    record->window_context = base_ctx;
    record->z_order = record->base_z_order;
#if 0
    /// 触发 base 的 resume 回调，恢复窗口刷新/任务
    window_enter_state_change_cb(ID_SCR_STATE_ON_RESUME, base_ctx, popup_ctx);
    if (base_hdr && base_hdr->scr_obj && base_hdr->scr_obj->on_resume) {
        base_hdr->scr_obj->on_resume(base_ctx);
    }
#endif
    record->base_window_hdr = NULL;
    record->base_window_context = NULL;
    record->base_z_order = 0;
#if WM_DEFER_POPUP_OVERLAY_UI_REMOVE
    if (keep_overlay_ui == 0) {
        overlay_ui_deinit();
    }
#else
    (void)keep_overlay_ui;
#endif
    return 0;
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
    int new_window_id = wm->is_valid_window_id(msg->union_body.extra.window_id);
    const char *win_name = to_window_name(new_window_id);
    bool is_bg_dispatch = (msg->union_body.extra.is_bg_dispatch == 0x01) ? true : false;
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
    if ((is_bg_dispatch) && (enter_new_window_policy(new_window_id) < 0)) {
        LOG_WM("the bg-dispatch(%s/%d) NOT allow!\n", win_name, new_window_id);
        return -3;
    }

    if (wm->is_the_same_window(wm, new_window_id)) {
        LOG_WM("thd ignore the same win: (%s/%d)", to_window_name(new_window_id), new_window_id);
        return -4;
    }

    if (new_window_id > 0) {
    try_again_dispatch:
        t0 = get_timestamp();
        LOG_WM("((bg:%d)name=%s)start", is_bg_dispatch, win_name);
        if (new_window_id != ID_WINDOW_DUMMY) {
            if (pm->is_on_suspend_gotoing()) {
                LOG_WM("pm on_suspend_gotoing, ignore: %d", new_window_id);
                return -5;
            }
        }
        
        enter_new_window_fptr proc = find_window_handle_by_id(new_window_id);
        if (proc) {
            int ret = proc(wm, new_window_id);
            if (ret == ID_WINDOW_DISPLAY_OK) {
                LOG_WM("enter_new_screen[%s]***", win_name);
            } else if (ret == ID_WINDOW_DISPLAY_TRY_HISTORY) {
                LOG_WM("%d window invalid, try history.", new_window_id);
                new_window_id = pop_window_history();
                msleep(100);
                goto try_again_dispatch;
            } else {
                LOG_WM("dispatch %d window ignore", new_window_id);
                return -6;
            }
        } else {
            LOG_WM("dispatch %d window invaild handle, ignore", new_window_id);
            return -7;
        }
        LOG_WM("dispatch %s window finish escaped: %ld ms", win_name, get_timestamp_ms(t0));
    } else {
        LOG_WM("dispatch %d window is invalid", new_window_id);
        return -8;
    }
    return 0;
}

static void window_evt_release_user_data(window_event_data_t *evt)
{
    if (evt && (evt->flags & WINDOW_EVT_FLAG_FREE_USER_DATA) && evt->user_data) {
        u_free(evt->user_data);
        evt->user_data = NULL;
    }
}

static int window_post_async_msg(window_manager_context_t *wm, multi_func_msg_data_t *msg)
{
    window_event_data_t *evt = &msg->union_body.window_evt_data;
    int window_id = wm->is_valid_window_id(evt->window_id);
    if (window_id == ID_WINDOW_INVALID) {
        LOGE("%d window is invalid", window_id);
        window_evt_release_user_data(evt);
        return -1;
    }

    window_context_t *win_ctx = wm->priv.window_private[window_id];
    if (win_ctx) {
        LOG_WM("win_ctx: %p, evt_id: %u", win_ctx, evt->evt_id);
        if (win_ctx->async_evt_handler_cb) {
            evt->window_ctx = win_ctx;
            evt->wm = wm;
            win_ctx->async_evt_handler_cb(evt);
        }
    } else {
        LOGE("win_ctx: %d not found.", window_id);
    }

    window_evt_release_user_data(evt);
    return 0;
}

static void window_post_async_key_event(multi_func_msg_data_t *msg)
{
    key_input_context_t *input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    int keycode = msg->union_body.key_event_data.keycode;
    int keytype = msg->union_body.key_event_data.keytype;
    if (input && input->dispatch_client_input_key) {
        input->dispatch_client_input_key(keycode, keytype);
    }
}

static void window_dispatch_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    window_manager_context_t *wm = CONTAINER_OF(thd, window_manager_context_t, priv.window_dispatch_thd);
    multi_func_msg_data_t *msg = &wm->priv.new_msg;
    CREATE_MSG_QUEUE(&thd->msg_id, DISP_DISPATCH_EVT_MESQ_NUM);
    task_post_process_fptr hdr_process_cb = NULL;
    REGISTER_TO_HANDLER_POST_POOL(thd->msg_id, &hdr_process_cb);
    task_post_process_fptr bcr_process_cb = NULL;
    REGISTER_TO_BROCASE_THD_POOL(thd->msg_id, &bcr_process_cb);
    for (;;) {
        thd->is_running = 0;
        MSG_QUEUE_RECV(thd->msg_id, msg, OS_WAIT_FOREVER);
        thd->is_running = get_timestamp();
        LOG_WM("recv msg: %d, window_id: %d", msg->msg_type, msg->union_body.extra.window_id);
        HANG_DETECT_LOCK(wm->priv.window_loop_detect_lock);
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
            case MSG_TYPE_CLOSE_POPUP_OVERLAY:
                WM_LOCK("--close_popup");
                close_popup_overlay_internal(wm, 0);
                WM_UNLOCK("--close_popup");
                break;
            case MSG_TYPE_HANDLER_POST_EVENT:
                /// 已在 WM 线程，勿再抢 window_switch_lock，避免与窗口切换/回调嵌套死锁
                if (hdr_process_cb) {
                    hdr_process_cb(msg, thd);
                }
                break;
            case MSG_TYPE_BROADCAST_EVENT:
                if (bcr_process_cb) {
                    bcr_process_cb(msg, thd);
                }
                break;
            default:
                break;
        }
        HANG_DETECT_UNLOCK(wm->priv.window_loop_detect_lock);
    }
}

static void on_wm_receive_internal(broadcast_mask_id_t id, void *context, int32_t arg1, 
        int32_t arg2, broadcast_priv_data_t *pdata)
{
    window_manager_context_t *wm = thiz;
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE: {
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                LOG_WM("pm state is suspend, clear window history.");
                clear_window_history();
            } 
        }
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

static bool can_refresh_overlay_popup(uint8_t window_id)
{
    window_context_t *ctx = s_window_record.window_context;

    return (window_id == ID_WINDOW_POPUP_WINDOW) && (ctx != NULL) && (ctx->is_popup_window != 0);
}

static int dispatch_overlay_popup_refresh(struct window_manager_context *wm, multi_func_msg_data_t *msg)
{
    multi_func_msg_data_t evt_msg = {0};
    window_entry_extra_t extra_copy;
    window_entry_extra_t *heap_extra = NULL;
    uint8_t window_id;
    uint32_t popup_type;
    uint32_t popup_exit_time;

    memcpy(&extra_copy, &msg->union_body.extra, sizeof(extra_copy));
    window_id = extra_copy.window_id;
    popup_type = (uint32_t)extra_copy.arg1;
    popup_exit_time = extra_copy.popup_exit_time;

    if (!wm->priv.window_dispatch_thd.msg_id) {
        LOG_WM("refresh overlay popup thd not ready, ignore type=%lu", popup_type);
        return -1;
    }

    heap_extra = (window_entry_extra_t *)u_malloz(sizeof(*heap_extra));
    if (heap_extra != NULL) {
        memcpy(heap_extra, &extra_copy, sizeof(*heap_extra));
    }

    LOG_WM("refresh overlay popup: type=%lu time=%lu extra=%p",
           popup_type, popup_exit_time, heap_extra);
    evt_msg.msg_type = MSG_TYPE_WIN_EVENT;
    evt_msg.union_body.window_evt_data.window_id = window_id;
    evt_msg.union_body.window_evt_data.evt_id = WINDOW_ASYNC_EVT_POPUP_UPDATE;
    evt_msg.union_body.window_evt_data.args1 = popup_type;
    evt_msg.union_body.window_evt_data.args2 = popup_exit_time;
    evt_msg.union_body.window_evt_data.user_data = heap_extra;
    evt_msg.union_body.window_evt_data.flags = (heap_extra != NULL) ? WINDOW_EVT_FLAG_FREE_USER_DATA : 0;
    MSG_QUEUE_SEND(wm->priv.window_dispatch_thd.msg_id, &evt_msg, 0);
    return 0;
}

static int dispatch_new_window_args(struct window_manager_context *wm, multi_func_msg_data_t *msg)
{
    uint8_t window_id = msg->union_body.extra.window_id;

    if (wm->is_the_same_window(wm, window_id)) {
        if (can_refresh_overlay_popup(window_id)) {
            return dispatch_overlay_popup_refresh(wm, msg);
        }
        LOG_WM("ignore the same scr: (%s/%d)", to_window_name(window_id), window_id);
        return -1;
    }

    bool is_bg_dispatch = (msg->union_body.extra.is_bg_dispatch == 0x01) ? true : false;
    if (!is_bg_dispatch || (enter_new_window_policy(window_id) == 0)) {
        LOG_WM("dispatch-UI(bg: %d)(scr: %s/%d) allow.\n", is_bg_dispatch, to_window_name(window_id), window_id);
        if (wm->priv.window_dispatch_thd.msg_id) {
            msg->msg_type = MSG_TYPE_WIN_DISPATCH;
            // LOG_WM("send msg: %d, window_id: %d", msg->msg_type, msg->union_body.extra.window_id);
            MSG_QUEUE_SEND(wm->priv.window_dispatch_thd.msg_id, msg, 0);
            return 0;
        }
        LOG_WM("dispatch-UI(bg: %d)(scr: %s/%d) thd not ready!\n", is_bg_dispatch, to_window_name(window_id), window_id);
        return -1;
    }
    LOG_WM("dispatch-UI(bg: %d)(scr: %s/%d) NOT allow!\n", is_bg_dispatch, to_window_name(window_id), window_id);

    return -1;
}

static void window_scheduling_start(void)
{
    window_manager_context_t *wm = thiz;
#if IS_SUPPORT_MODULE_RES
    resource_manager_context_t *res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    if (res) {
        LOG_WM("wait res ready");
        res->wait_init_completed(res);
    }
#endif
    wm->window_boot_completed(wm);
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
        return 0;
    }

    if (msg->msg_type == MSG_TYPE_WIN_EVENT || msg->msg_type == 0) {
        window_evt_release_user_data(&msg->union_body.window_evt_data);
    }
    return -1;
}

static int post_window_event(const char *tag, uint8_t window_id, uint32_t evt_id,
    void *user_data, uint8_t flags)
{
    multi_func_msg_data_t msg = {0};

    msg.msg_type = MSG_TYPE_WIN_EVENT;
    msg.union_body.window_evt_data.window_id = window_id;
    msg.union_body.window_evt_data.evt_id = evt_id;
    msg.union_body.window_evt_data.user_data = user_data;
    msg.union_body.window_evt_data.flags = flags;
    return post_async_msg_to_window(tag, &msg);
}

static bool wait_window_ready(int wait_dummy)
{
    uint16_t retry_times = 0;
    /// BEGIN窗口下未唤醒，发广播接收不到，DUMMY窗口下未跑完唤醒流程，发广播会唤醒异常
    while (thiz->is_current_window(thiz, ID_WINDOW_BEGIN) || (wait_dummy && thiz->is_current_window(thiz, ID_WINDOW_DUMMY))) {
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
    window_manager_context_t *wm = thiz;
    if (new_state == ID_SCR_STATE_ON_CREATE) {
        wm->set_window_private_context(wm, ctx->new_window_id, (void *)ctx);
    }

    if (ctx->evt_cb_entry) {
        return ctx->evt_cb_entry(new_state, ctx, old_window);
    }

    return 0;
}

static int window_exit_state_change_cb(window_lift_cycle_e new_state, 
    const struct window_context *ctx, const struct window_context *new_window) 
{
    window_manager_context_t *wm = thiz;
    uint8_t old_window_id = ctx->new_window_id;
    int ret = 0;

    if (ctx->evt_cb_exit) {
        ret = ctx->evt_cb_exit(new_state, ctx, new_window);
    }

    if (new_state == ID_SCR_STATE_ON_DESTORY) {
        if (ret == 0) {
            wm->set_window_private_context(wm, old_window_id, NULL);
        } else {
            LOGW("#window_private[%d] NOT restore!", old_window_id);
        }
    }
    return ret;
}

static int dispatch_new_window(window_context_t *ctx)
{
    window_context_handle_t *next = NULL;
    window_manager_context_t *wm = thiz;
    window_context_record_t *record = &s_window_record;
    static uint32_t counter = 0;
#if IS_SUPPORT_LVGL
    ui_manager_context_t *ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    if (ui && ui->is_lvgl_initialized) {
        uint32_t wait_count = 0;
        while (!ui->is_lvgl_initialized(ui) && wait_count < 100) {
            LOG_WM("waiting for LVGL initialization... (%lu)", wait_count);
            msleep(10);
            wait_count++;
        }
        if (!ui->is_lvgl_initialized(ui)) {
            LOG_WM("ERROR: LVGL initialization timeout, window creation aborted!");
            return -99;
        }
    }
#endif
    /// 查找窗口对象
    next = dichotomy_find_window_obj(ctx->new_window_id);
    if (!next) {
        LOG_WM("not found window_id: %d", ctx->new_window_id);
        return -1; 
    }
    /// 校验是否为同一个窗口
    if (wm->is_the_same_window(wm, ctx->new_window_id)) {
        LOG_WM("ignore the same screen: %s", to_window_name(ctx->new_window_id)); /// 忽略同一个窗口
        return -1; /// 忽略同一个窗口
    }
    uint32_t t0 = get_timestamp();   /// 获取当前时间戳
#if WM_DEFER_POPUP_OVERLAY_UI_REMOVE
    uint8_t defer_overlay_ui = 0;
#endif
    if (record->base_window_context != NULL && ctx->is_popup_window == 0) {
#if WM_DEFER_POPUP_OVERLAY_UI_REMOVE
        defer_overlay_ui = 1;
        close_popup_overlay_internal(wm, 1);   /// 窗口按原顺序关，LVGL 层延后拆
        if (wm->is_the_same_window(wm, ctx->new_window_id)) {
            overlay_ui_deinit();
            LOG_WM("ignore the same screen after closing overlay: %s", to_window_name(ctx->new_window_id));
            return -1;
        }
#else
        close_popup_overlay_internal(wm, 0);
        if (wm->is_the_same_window(wm, ctx->new_window_id)) {
            LOG_WM("ignore the same screen after closing overlay: %s", to_window_name(ctx->new_window_id));
            return -1;
        }
#endif
    }
    /// overlay popup：不销毁 base window，仅创建 popup 叠加在上层
    if (ctx->is_popup_window) {
        if (record->base_window_context != NULL) {
            close_popup_overlay_internal(wm, 0); /// 已存在 overlay popup，先关闭旧 popup，再创建新 popup（单层 overlay）
        }

        if (record->window_hdr == NULL || record->window_context == NULL) {
            LOG_WM("no base window present, fallback to normal switch for popup");
            return -1;
        } 
        LOG_WM("overlay popup start: (%s) over base(%s), counter=[%lu]",
                to_window_name(next->window_id),
                to_window_name(record->window_hdr->window_id),
                (++counter));
        record->base_window_hdr = record->window_hdr;
        record->base_window_context = record->window_context;
        record->base_z_order = record->z_order;
        record->z_order = (ctx->z_order > WINDOW_PRIO_LOW) ? ctx->z_order : next->z_order; /// 更新当前窗口的层级优先级（popup 在顶层）
        window_enter_state_change_cb(ID_SCR_STATE_ON_CREATE, ctx, record->base_window_context); /// 进入 popup（不 pause/destory base）
        if (next->scr_obj && next->scr_obj->on_create) {
            next->scr_obj->on_create(ctx);
        }
        window_enter_state_change_cb(ID_SCR_STATE_ON_RESUME, ctx, record->base_window_context);/// 恢复 popup 窗口生命周期回调
        if (next->scr_obj && next->scr_obj->on_resume) {
            next->scr_obj->on_resume(ctx);
        }
        record->window_hdr = next;
        record->window_context = ctx;
        LOG_WM("overlay popup finish: (%s/%d/z_order:%d), escaped: %ld ms",
                to_window_name(next->window_id), next->window_id, record->z_order, get_timestamp_ms(t0));
        return 0;
    }
    LOG_WM("switch_windows start: (%s), new_window_id: %d", to_window_name(ctx->new_window_id), (ctx->new_window_id));
    if (record->window_hdr != NULL) {
        LOG_WM("on_pause old scr_obj: (%s)", to_window_name(record->window_hdr->window_id));        
        if (record->window_hdr->scr_obj->on_pause) {
            record->window_hdr->scr_obj->on_pause(record->window_context);
        }
        window_exit_state_change_cb(ID_SCR_STATE_ON_PAUSE, record->window_context, ctx);/// 退出当前窗口生命周期回调
    }
   
    record->next_window_id = next->window_id; /// 更新即将进入的新窗口id
    record->z_order = (ctx->z_order > WINDOW_PRIO_LOW) ? ctx->z_order : next->z_order;  /// 更新当前窗口的层级优先级.
    LOG_WM("create new scr_obj:(%s),counter=[%lu]", to_window_name(next->window_id), (++counter));
    window_enter_state_change_cb(ID_SCR_STATE_ON_CREATE, ctx, record->window_context); /// 进入新窗口生命周期回调
    if (next->scr_obj->on_create) {
        next->scr_obj->on_create(ctx);   /// 创建新窗口回调
    }
    window_enter_state_change_cb(ID_SCR_STATE_ON_RESUME, ctx, record->window_context); /// 恢复当前窗口生命周期回调
    if (next->scr_obj->on_resume) {
        next->scr_obj->on_resume(ctx);   /// 恢复当前窗口回调
    }

    if (record->window_hdr != NULL && record->window_context != NULL) {
        if (record->window_context->not_push_history) {
            LOG_WM("not_push_history flag: %s", to_window_name(record->window_hdr->window_id)); /// 不压入历史栈
        } else if (ctx->new_window_id != ID_WINDOW_DUMMY) {
            push_window_history(record->window_hdr->window_id);   /// 压入历史栈
        }
        wm->clear_window_go_back_history_flag(wm);   /// 清除Flag.
        LOG_WM("destory old scr_obj: (%s)", to_window_name(record->window_hdr->window_id));
        window_exit_state_change_cb(ID_SCR_STATE_ON_DESTORY, record->window_context, ctx);   /// 销毁当前窗口生命周期回调
        if (record->window_hdr->scr_obj->on_destory) {
            record->window_hdr->scr_obj->on_destory();   /// 销毁当前窗口回调
        }
    } else {
        LOG_WM("Not found prev scr!"); /// 未找到前一个窗口
    }
    record->window_hdr = next; /// 更新当前窗口记录
    record->window_context = ctx;   /// 更新当前窗口上下文
#if WM_DEFER_POPUP_OVERLAY_UI_REMOVE
    if (defer_overlay_ui) {
        overlay_ui_deinit();
    }
#endif
    LOGE("switch_windows finish: (%s/%d/z_order:%d), escaped: %ld ms", 
        to_window_name(next->window_id), next->window_id, record->z_order, get_timestamp_ms(t0));
    return 0;
}

static int go_back_to_history_wrapper(void *wm, window_entry_extra_t *__extra)
{
    window_manager_context_t *_wm = wm;
    multi_func_msg_data_t msg = {0};

    memcpy(&msg.union_body.extra, (__extra), sizeof(window_entry_extra_t));
    msg.union_body.extra.window_id = pop_window_history();

    if (get_current_window_id() == msg.union_body.extra.window_id) {
        msg.union_body.extra.window_id = pop_window_history();
    }

    LOG_WM("go back to history: %s\n", to_window_name(msg.union_body.extra.window_id));
    _wm->dispatch_new_window_args(_wm, &(msg));

    return 0;
}

static int close_popup_overlay_wrapper(struct window_manager_context *wm)
{
    EXCEPTION_CHECK_INT(wm);
    if (!wm->priv.window_dispatch_thd.msg_id) {
        MUTEX_LOCK(wm->priv.window_switch_lock);
        int ret = close_popup_overlay_internal(wm, 0);
        MUTEX_UNLOCK(wm->priv.window_switch_lock);
        return ret;
    }
    multi_func_msg_data_t msg = {0};
    msg.msg_type = MSG_TYPE_CLOSE_POPUP_OVERLAY;
    post_async_msg_to_window("close_popup_overlay", &msg);
    return 0;
}

static int on_post_wm_init(void *ptr)
{
    window_manager_context_t *wm = ptr;
    /// 广播监听PM状态改变.
    wm->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    wm->priv.bcd->context = wm;
    wm->priv.bcd->name = "window_manager";
    wm->priv.bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    wm->priv.bcd->id.code |= ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE;
    wm->priv.bcd->receiver_cb = on_wm_receive_internal;
    register_broadcast_receiver(wm->priv.bcd);
    RUN_NEW_THREAD(&wm->priv.window_dispatch_thd, "window_dispatch_thd", 
        window_dispatch_thread, RT_PRIO_WINDOW_DISPATCH_THREAD, WM_THREAD_STACKSIZE);

    return 0;
}

static int on_perpare_wm_init(void *self)
{
    window_manager_context_t *wm = self;
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
/// 显示管理外部注册接口.
void *register_module_for_window_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, window_manager_context_t);
    ptr->dispatch_new_window = dispatch_new_window;
    ptr->window_scheduling_start = window_scheduling_start;
    ptr->post_msg_to_window = post_async_msg_to_window;
    ptr->post_window_event = post_window_event;
    ptr->wait_window_ready = wait_window_ready;
    ptr->to_window_name = to_window_name;
    ptr->get_current_window_id = get_current_window_id;
    ptr->get_next_window_id = get_next_window_id;
    ptr->get_overlay_window_id = get_overlay_window_id;
    ptr->dispatch_new_window_args = dispatch_new_window_args;
    ptr->go_back_to_history = go_back_to_history_wrapper;
    ptr->close_popup_overlay = close_popup_overlay_wrapper;
    register_window_state_module(ptr);

    USE_INIT_LIST_HEAD(&ptr->priv.history_list);
    MUTEX_INIT(&ptr->priv.history_lock, history_stack_lock);
    MUTEX_INIT(&ptr->priv.window_switch_lock, window_switch_lock);
    HANG_DETECT_LOCK_INIT(&ptr->priv.window_loop_detect_lock, window_loop_detect_lock,
                          HANG_DETECT_TIMEOUT_S);
    thiz = ptr;

    return ADD_MODULE("wm", ID_MODULE_WM, on_perpare_wm_init, 
        on_post_wm_init, ptr, list_head, sizeof(window_manager_context_t));
}
