/*
* INCLUDE FILES
*****************************************************·************************************
*/
#include "root_common_api.h"
#include "ble_manager.h" // IWYU pragma: keep
#include "ble_event_process.h" // IWYU pragma: keep


/*
* DEFINES
*****************************************************************************************
*/
#define BLE_MSG_MALLOC_MIN_SIZE              (256) /// 消息分配最小大小

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ble_manager_context_t *ble_obj = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int ble_charge_state_handler(int state)
{
    return 0;
}

static void ble_pm_change_handler(int arg1)
{
    ble_manager_context_t *ctx = ble_obj;
    if (ctx->ble_is_connected()) {
        LOG_BLE("ble connected ignore");
        return;
    }

    if (arg1 == PM_BROADCAST_SYS_RESUME) {
        ble_opt_adv_id_e adv = BLE_OPT_ADV_FAST;
        LOG_BLE("ble adv fast start");
        ctx->dispatch_ble_event_wrapper(ctx, BLE_MSG_SET_ADV_START, (uint8_t *)&adv, sizeof(adv));
    } else if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
        LOG_BLE("ble adv stop");
        ble_opt_adv_id_e adv = BLE_OPT_ADV_STOP;
    }
}

static void ble_ota_upgrade_state_handler(void *user_data)
{

}

static void on_ble_broadcast_receive(broadcast_mask_id_t id, void *context, int arg1, int arg2, 
                                        broadcast_priv_data_t *pdata)
{
    uint32_t *user_data = NULL;
    ble_manager_context_t *ctx = context;
    LOG_BLE("ble broadcast rcv_id: 0x%lx", id.code);
    switch (id.code) {
        case ID_BROADCAST_BATTERY_STATE_CHANGE:
            if (arg1 == BATTERY_BROADCAST_CHARGE_STATE_CHANGE) {
                ble_charge_state_handler(arg2);
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE:
            ble_pm_change_handler(arg1);
            break;
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_BOOT_COMPLETED) {
                ble_opt_adv_id_e adv = BLE_OPT_ADV_FAST;
                LOG_BLE("ble adv fast start");
                ctx->dispatch_ble_event_wrapper(ctx, BLE_MSG_SET_ADV_START, (uint8_t *)&adv, sizeof(adv));
            }
            break;
        case ID_BROADCAST_OTA_STATE_CHANGE:
            if (arg1 == OTA_BROADCAST_STATE_CHANGE) {
                PRIV_DATA_TO_USER_DATA(user_data, pdata, uint32_t);
                ble_ota_upgrade_state_handler(user_data);
            }
            break;
       default:
           break;
    }
}

static int ble_mac_addr_get(uint8_t *mac)
{
    if (ble_obj->priv.ble_dev && ble_obj->priv.ble_dev->ioctl) {
        uint32_t args[] = {(uint32_t)mac};
        int ret = DEVICE_IOCTL_ARGS(ble_obj->priv.ble_dev, OP_CODE_BLE_MAC_ADDR_GET, args);
        if (ret == 0 && mac) {
            memcpy(ble_obj->priv.mac_addr, mac, BLE_MAC_ADDR_LEN);
            return 0;
        }
    }
    // 如果底层获取失败，尝试从缓存读取
    if (mac) {
        memcpy(mac, ble_obj->priv.mac_addr, BLE_MAC_ADDR_LEN);
    }
    return 0;
}

static int ble_mac_addr_set(uint8_t *mac)
{
    uint8_t is_mac_empty = 1;

    for (uint8_t i = 0; i < BLE_MAC_ADDR_LEN; i++) {
        if (mac[i] != 0) {
            is_mac_empty = 0;
            break;
        }
    }

    memcpy(ble_obj->priv.mac_addr, mac, BLE_MAC_ADDR_LEN);
    if (!is_mac_empty) {
        LOG_BLE("set mac=%02X:%02X:%02X:%02X:%02X:%02X.\n", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

        if (ble_obj->priv.ble_dev && ble_obj->priv.ble_dev->ioctl) {
            uint32_t args[] = {BLE_MAC_ADDR_LEN, (uint32_t)mac};
            DEVICE_IOCTL_ARGS(ble_obj->priv.ble_dev, OP_CODE_BLE_MAC_ADDR_SET, args);
        }
    } else {
        LOG_BLE("mac addr is NULL.");
    }

    return 0;
}

static int ble_is_connected(void)
{
    return ble_obj->priv.ble_connect_state;
}

static int ble_disconnect_by_user(void)
{
    ble_manager_context_t *ctx = ble_obj;
    if (ctx->ble_is_connected()) {
        LOG_BLE("user ble disconnect");
        ctx->dispatch_ble_event_wrapper(ctx, BLE_MSG_PERIPH_DISCONN, (uint8_t *)ctx, 0);
    }
    return 0;
}

static int ble_bind_state_get(void)
{
    return ble_obj->priv.ble_bind_state;
}

static int ble_bind_state_set(uint8_t state)
{
    if (state != ble_obj->priv.ble_bind_state) {
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        EXCEPTION_CHECK_INT(wm);
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POPUP_WINDOW;
        msg.union_body.extra.is_bg_dispatch = 0x01;
        msg.union_body.extra.arg1 = (BLE_BIND_STATE_COMPLETE == state) ? ID_POPUP_UI_DEVICE_BIND : ID_POPUP_UI_DEVICE_UNBIND;
        msg.union_body.extra.popup_exit_time = 1000;
        wm->dispatch_new_window_args(wm, &msg);
    }
    ble_obj->priv.ble_bind_state = state;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (storage_ctx && storage_ctx->put_store_val) {
        storage_ctx->put_store_val(STORE_ID_SYS_BIND_STATUS, 0, 0, &state);
    }
    LOG_BLE("bind state: %d", state);
    return 0;
}

static void ble_adv_self_starting_handler(hdr_post_args_t *args)
{

}

static int ble_adv_stop_by_user(void)
{
    ble_manager_context_t *ctx = ble_obj;
    ble_opt_adv_id_e opt = BLE_OPT_ADV_STOP;

    LOG_BLE("ble adv stop");
    ctx->dispatch_ble_event_wrapper(ctx, BLE_MSG_SET_ADV_STOP, (uint8_t *)&opt, sizeof(opt));

    return 0;
}

static int ble_adv_start_by_user(void)
{
    ble_manager_context_t *ctx = ble_obj;
    ble_opt_adv_id_e opt = BLE_OPT_ADV_FAST;
    LOG_BLE("ble adv fast start");
    ctx->dispatch_ble_event_wrapper(ctx, BLE_MSG_SET_ADV_START, (uint8_t *)&opt, sizeof(opt));
    return 0;
}

static int ble_adv_speed_get(void)
{
    return ble_obj->priv.adv_speed_flag;
}

static int ble_adv_speed_set(int speed)
{
    ble_manager_context_t *ctx = ble_obj;
    ble_opt_adv_id_e opt = (ble_opt_adv_id_e)speed;

    LOG_BLE("adv speed: %d", speed);
    ble_obj->priv.adv_speed_flag = speed;
    return 0;
}

static int ble_adv_state_get(void)
{
    return ble_obj->priv.ble_adv_state;
}

static int ble_adv_state_set(int state)
{
    ble_obj->priv.ble_adv_state = state;
    LOG_BLE("adv state: %d", state);

    return 0;
}

static void on_connect_status_change(ble_manager_context_t *ctx, int new_status, int forbid_popup)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);

    //通知app server
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(servers_ctx && servers_ctx->hook);
    servers_publish_data_args_t args = {
        .arg1 = new_status,
        .arg2 = 0,
        .val = NULL,
    };
    servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_DISCON, &args);

    if (forbid_popup) {
        LOGW("forbid_popup");
        return;
    }

    /// 飞行模式情况下不要弹出!!
    if (ap && ap->is_on_airplane_mode()) {
        LOGW("current on_airplane_mode, ignore: %d", new_status);
        return;
    }

    /// 工厂进入休眠，断开蓝牙后不要弹出
    if (ctx->priv.fac_enter_sleep) { 
        ctx->priv.fac_enter_sleep = 0;
        LOGW("current on_factory_mode, ignore: %d", new_status);
        return;
    }

    /// 系统未准备好，忽略
    if (!(wm && wm->system_is_ready(wm))) {
        LOGW("system is not ready, ignore: %d", new_status);
        return;
    }
    LOG_BLE("new ble_connect_state: %d, show popup window.", new_status);
    SEND_BROADCAST_ARGS1(ID_BROADCAST_BLE_STATE_CHANGE, 
        new_status > 0 ? BLE_BROADCAST_CONNECTED : BLE_BROADCAST_DISCONNECT);
    if (ctx->hook && ctx->hook->on_connect_change_route) {
        ctx->hook->on_connect_change_route(ctx, new_status);
    }

    if (wm) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POPUP_WINDOW;
        msg.union_body.extra.is_bg_dispatch = 0x01;
        msg.union_body.extra.arg1 = (1 == new_status) ? ID_POPUP_UI_BT_CONNECT : ID_POPUP_UI_BT_DISCONNECT;
        msg.union_body.extra.popup_exit_time = 1000;
        wm->dispatch_new_window_args(wm, &msg);
    }
}

static int dispatch_ble_event_wrapper(ble_manager_context_t *ctx, uint8_t _msg_id, uint8_t *_data, int _data_len)
{
    multi_func_msg_data_t msg = { .union_body.ble_evt_type.msg_id = _msg_id, };
    if (_data_len) {
        msg.union_body.ble_evt_type.ble_data = u_malloz(_data_len > BLE_MSG_MALLOC_MIN_SIZE ? _data_len : BLE_MSG_MALLOC_MIN_SIZE);
        EXCEPTION_CHECK_INT(msg.union_body.ble_evt_type.ble_data);
        if (msg.union_body.ble_evt_type.ble_data) {
            msg.union_body.ble_evt_type.data_len = _data_len;
            memcpy(msg.union_body.ble_evt_type.ble_data, _data, _data_len);
        }
        
        if (MSG_QUEUE_SEND(ctx->priv.ble_work_thd.msg_id, &msg, 0) != 0) {
            LOGE("dispatch ble event fail. msg_id: %d", _msg_id);
            u_free(msg.union_body.ble_evt_type.ble_data);
            return -1;
        }
        return 0;
    }
    LOGE("dispatch ble event fail. msg_id: %d", _msg_id);
    return -1;
}

static void ble_manager_dispatch_thread(void *args)
{
    thread_context_t *thd = args;
    ble_manager_context_t *ctx = ble_obj;
    multi_func_msg_data_t msg = {0};
    CREATE_MSG_QUEUE(&thd->msg_id, BLE_WORK_DISPATCH_MESQ_NUM);
    while (1) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        ble_message_process_handler(ctx, &msg.union_body.ble_evt_type);
        if (msg.union_body.ble_evt_type.ble_data) {
            u_free(msg.union_body.ble_evt_type.ble_data);
        }
    }
}

static void ble_event_callback(ble_event_e event, void *param)
{
    LOG_BLE("ble event: %d", event);
    switch (event) {
        case BLE_EVENT_ADVERTISE_STARTED:
            ble_obj->priv.ble_adv_state = 1;
            break;
        case BLE_EVENT_ADVERTISE_STOPPED:
            ble_obj->priv.ble_adv_state = 0;
            break;
        case BLE_EVENT_CONN_OPENED:
        case BLE_EVENT_CONN_MTU_EXCHANGED:
        case BLE_EVENT_CONN_PARAMS_UPDATE:
        case BLE_EVENT_TX_CCC_ENABLED:
            ble_obj->dispatch_ble_event_wrapper(ble_obj, BLE_MSG_PERIPH_CONNECT_SUC, param, 1);
            break;
        case BLE_EVENT_CONN_CLOSED:
            ble_obj->dispatch_ble_event_wrapper(ble_obj, BLE_MSG_PERIPH_DISCONN_SUC, param, 1);
            break;
        case BLE_EVENT_CONN_FAILED:
            ble_obj->dispatch_ble_event_wrapper(ble_obj, BLE_MSG_PERIPH_DISCONN, param, 1);
            break;
        default:
            break;
    }
}

static void ble_data_callback(ble_service_id_e service_id, uint8_t *p_data, uint16_t size)
{
    LOG_BLE("ble data: %d", service_id);
    switch (service_id) {
        case BLE_SRV_ID_MASTER:
            ble_obj->dispatch_ble_event_wrapper(ble_obj, BLE_MSG_MASTER_RECIVE_DATA, p_data, size);
            break;
        case BLE_SRV_ID_FACTORY:
            ble_obj->dispatch_ble_event_wrapper(ble_obj, BLE_MSG_FAC_RECIVE_DATA, p_data, size);
            break;
        case BLE_SRV_ID_REMOTE:
            ble_obj->dispatch_ble_event_wrapper(ble_obj, BLE_MSG_REMOT_RECIVE_DATA, p_data, size);
            break;
        default:
            break;
    }
}

static void add_ble_evt_lisenser(void *user_data, int msg_channel, on_ble_evt_data_cb_fptr cb)
{
    ble_manager_context_t *obj = ble_obj;
    list_head_t *head = &obj->priv.client_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        ble_evt_client_node_t *node = CONTAINER_OF(pos, ble_evt_client_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOGW("ignore dumplicate add.");
            MUTEX_UNLOCK(obj->priv.m_lock);
            return;
        }
    }
    ble_evt_client_node_t *node = u_malloz(sizeof(ble_evt_client_node_t));
    node->cb = cb;
    node->msg_channel = msg_channel;
    node->user_data = user_data;
    __list_add(&node->list, head);
    LOG_BLE("add: user_data: %lx, msg_channel: %d, cb: %lx", user_data, msg_channel, cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void *list_del_ble_condition_cb(void *data, list_head_t *pos)
{
    on_ble_evt_data_cb_fptr cb = (on_ble_evt_data_cb_fptr)data;
    ble_evt_client_node_t *node = CONTAINER_OF(pos, ble_evt_client_node_t, list);
    if ((node->cb) == (cb)) {
        LOG_BLE("del: user_data: %lx, msg_channel: %d, cb: %lx", node->user_data,
             node->msg_channel, node->cb);
        return node;
    }

    return NULL;
}

static void dispatch_ble_client(ble_manager_context_t *obj, int msg_channel, uint8_t *data, uint16_t size)
{
    list_head_t *head = &obj->priv.client_list;
    LOG_BLE("==> dispatch_ble_client: msg_channel=%d, data=%p, size=%d", msg_channel, data, size);
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        ble_evt_client_node_t *node = CONTAINER_OF(pos, ble_evt_client_node_t, list);
        LOG_BLE("==> dispatch_ble_client: checking node, node->msg_channel=%d", node->msg_channel);
        if (node->msg_channel == msg_channel) {
            LOG_BLE("==> dispatch_ble_client: calling callback, node->cb=%p", node->cb);
            node->cb(node->user_data, data, size);
            LOG_BLE("==> dispatch_ble_client: callback returned");
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);
    LOG_BLE("==> dispatch_ble_client: done");
}

static void remove_ble_evt_lisenser(on_ble_evt_data_cb_fptr cb)
{
    ble_manager_context_t *obj = ble_obj;
    list_head_t *head = &obj->priv.client_list;
    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, list_del_ble_condition_cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static int on_post_init(void *ptr)
{
    ble_manager_context_t *ctx = ptr;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ctx->priv.ble_dev = pbus->find_device(ID_DEVICE_BLE);
    /// 注册BLE设备事件回调
    if (ctx->priv.ble_dev && ctx->priv.ble_dev->ioctl) {
        DEVICE_IOCTL_ARGS(ctx->priv.ble_dev, OP_CODE_BLE_ADD_EVENT_CALLBACK, ble_event_callback);
    }
    /// 获取BLE MAC地址
    if (ctx->priv.ble_dev && ctx->priv.ble_dev->ioctl) {
        uint32_t args[] = {(uint32_t)ctx->priv.mac_addr};
        DEVICE_IOCTL_ARGS(ctx->priv.ble_dev, OP_CODE_BLE_MAC_ADDR_GET, args);
    }
    /// 注册BLE数据回调
    if (ctx->priv.ble_dev && ctx->priv.ble_dev->ioctl) {
        DEVICE_IOCTL_ARGS(ctx->priv.ble_dev, OP_CODE_BLE_ADD_DATA_CALLBACK, ble_data_callback);
    }
    /// 获取绑定状态
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (storage_ctx && storage_ctx->get_store_val) {
        storage_ctx->get_store_val(STORE_ID_SYS_BIND_STATUS, 0, 0, &ctx->priv.ble_bind_state);
    }

    ctx->priv.bcreceiver = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.bcreceiver->context = ctx;
    ctx->priv.bcreceiver->name = "ble_manager";
    ctx->priv.bcreceiver->id.code |= ID_BROADCAST_BATTERY_STATE_CHANGE;
    ctx->priv.bcreceiver->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.bcreceiver->id.code |= ID_BROADCAST_OTA_STATE_CHANGE;
    ctx->priv.bcreceiver->id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    ctx->priv.bcreceiver->receiver_cb = on_ble_broadcast_receive;
    register_broadcast_receiver(ctx->priv.bcreceiver);
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }
    RUN_NEW_THREAD(&ctx->priv.ble_work_thd, "ble_thd", 
        ble_manager_dispatch_thread, RT_PRIO_BLE_DISPATCH_THREAD, BLE_THREAD_STACKSIZE);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_ble_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, ble_manager_context_t);
    ptr->get_mac_addr = ble_mac_addr_get;
    ptr->set_mac_addr = ble_mac_addr_set;
    ptr->ble_is_connected = ble_is_connected;
    ptr->disconnect_bt_by_user = ble_disconnect_by_user;
    ptr->ble_bind_state_get = ble_bind_state_get;
    ptr->ble_bind_state_set = ble_bind_state_set;
    ptr->bt_adv_stop_by_user = ble_adv_stop_by_user;
    ptr->bt_adv_start_by_user = ble_adv_start_by_user;
    ptr->get_adv_speed = ble_adv_speed_get;
    ptr->set_adv_speed = ble_adv_speed_set;
    ptr->get_adv_state = ble_adv_state_get;
    ptr->set_adv_state = ble_adv_state_set;
    ptr->on_connect_status_change = on_connect_status_change;
    ptr->dispatch_ble_event_wrapper = dispatch_ble_event_wrapper;
    ptr->add_ble_evt_lisenser = add_ble_evt_lisenser;
    ptr->remove_ble_evt_lisenser = remove_ble_evt_lisenser;
    ptr->dispatch_ble_client = dispatch_ble_client;
    ble_obj = ptr;
    return ADD_MODULE("ble_module", ID_MODULE_BLE, 0, on_post_init, 
        ptr, list_head, sizeof(ble_manager_context_t));
}
