/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "ota_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define OTA_PROGRESS_TIMER_PERIOD_MS    (100)
#define IS_SUPPORT_OTA_EVENT_POLLING    (0)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ota_manager_context_t *ota_obj = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool is_ota_upgrading(void)
{
    return ota_obj->priv.ota_info.is_working;
}

static void set_ota_upgrading(bool yes)
{
    LOG_OTA("ota_is_working: %d", yes);
    ota_obj->priv.ota_info.is_working = yes;
}

static const char *get_ota_object_name(ota_object_e object)
{
    switch (object) {
        default:
            return "unknow";
        case OTA_OBJECT_MAIN_FIRMWARE:
            return "app";
        case OTA_OBJECT_IMAGE_RESOURCE:
            return "img";
        case OTA_OBJECT_TOUCH_KEY_FIRMWARE:
            return "touch-key";
    }
}

static void set_ota_upgrade_progress(uint8_t progress)
{
    LOG_OTA("ota %s progress: %d", get_ota_object_name(ota_obj->priv.ota_info.object), progress);
    ota_obj->priv.ota_info.progress = progress;
}

static void on_ota_broadcast_receive(broadcast_mask_id_t id, 
                                        void *context, 
                                        int arg1, 
                                        int arg2,
                                        broadcast_priv_data_t *pdata)
{
    if (ota_obj->hook && ota_obj->hook->on_ota_receiver_route) {
        ota_obj->hook->on_ota_receiver_route(id, ota_obj, arg1, arg2, pdata);
    }
}

static void set_ota_upgrade_state(ota_upgrade_state_e state)
{
    LOG_OTA("ota_state: %d", state);
    ota_obj->priv.ota_info.state = state;
}

static void get_ota_info(ota_info_t *info)
{
    memcpy(info, &ota_obj->priv.ota_info, sizeof(ota_info_t));
}

static void ota_upgrade_state_progress(ota_manager_context_t *ctx, ota_event_info_t *info)
{
    if (info == NULL) {
        return;
    }
    ctx->priv.ota_info.recv_size = (info->args2 > info->args1) ? info->args1 : info->args2;
    ctx->priv.ota_info.total_size = info->args1;
    uint8_t val = 0;
    if (ctx->priv.ota_info.total_size > 0) {
        val = (ctx->priv.ota_info.recv_size * 100) / ctx->priv.ota_info.total_size;
    } else {
        LOGE("ota size error:%d/%d", ctx->priv.ota_info.recv_size, ctx->priv.ota_info.total_size);
    }
    set_ota_upgrade_progress(val);
}

static void ota_upgrade_state_start(ota_manager_context_t *ctx, ota_event_info_t *info)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    ctx->priv.ota_info.object = info->object;
    ctx->priv.ota_info.recv_size = (info->args2 > info->args1) ? info->args1 : info->args2;
    ctx->priv.ota_info.total_size = info->args1;
    set_ota_upgrading(true);
    uint8_t val = 0;
    if (ctx->priv.ota_info.total_size > 0) {
        val = (ctx->priv.ota_info.recv_size * 100) / ctx->priv.ota_info.total_size;
    } else {
        LOGE("ota size error:%d/%d", ctx->priv.ota_info.recv_size, ctx->priv.ota_info.total_size);
    }
    set_ota_upgrade_progress(val);
    set_ota_upgrade_state(OTA_UPGRADE_STATE_START);
    if (wm && wm->dispatch_new_window_args && 
        wm->is_current_window && wm->is_current_window(wm, ID_WINDOW_OTA_UPGRADE) == 0) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_OTA_UPGRADE;
        msg.union_body.extra.is_bg_dispatch = 0x01;
        wm->dispatch_new_window_args(wm, &msg);
    }
    SEND_BROADCAST_ARGS1(ID_BROADCAST_OTA_STATE_CHANGE, OTA_UPGRADE_STATE_START);
}

static void ota_upgrade_state_end(ota_manager_context_t *ctx, uint8_t state)
{
    set_ota_upgrading(false);
    if (0x01 == state) {
        LOG_OTA("ota result sucess");
        set_ota_upgrade_progress(100);
        set_ota_upgrade_state(OTA_UPGRADE_STATE_SUCCESS);
        SEND_BROADCAST_ARGS1(ID_BROADCAST_OTA_STATE_CHANGE, OTA_UPGRADE_STATE_SUCCESS);
    } else {
        LOG_OTA("ota result faile, reson 0x%02X", state);
        set_ota_upgrade_progress(0);
        set_ota_upgrade_state(OTA_UPGRADE_STATE_FAIL);
        SEND_BROADCAST_ARGS1(ID_BROADCAST_OTA_STATE_CHANGE, OTA_UPGRADE_STATE_FAIL);
    }
}

static void ota_upgrade_thread(void *args)
{
    ota_manager_context_t *ctx = ota_obj;
    thread_context_t *thd = args;
    multi_func_msg_data_t msg = {0};
    ota_event_type_e event;
    ota_event_info_t *ota_info;
    CREATE_MSG_QUEUE(&thd->msg_id, OTA_WORK_DISPATCH_MESQ_NUM);
    while (1) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        ota_info = &msg.union_body.ota_info;
        switch (ota_info->event) {
            case OTA_EVENT_RESUME_OTA:
            case OTA_EVENT_START_OTA:
                ota_upgrade_state_start(ctx, ota_info);
                break;
            case OTA_EVENT_OTA_PROGRESS: 
                ota_upgrade_state_progress(ctx, ota_info);
                break;
            case OTA_EVENT_OTA_FAIL: 
                ota_upgrade_state_end(ctx, false);
                break;
            case OTA_EVENT_OTA_SUCCESS:
                ota_upgrade_state_end(ctx, true);
                break;
            default:
                break;
        }
    }
}

static int dispatch_ota_event_wrapper(int event, ota_object_e object, int arg1, int arg2)
{
    multi_func_msg_data_t msg = {0};
    int ret;
    msg.union_body.ota_info.event = (ota_event_type_e)event;
    msg.union_body.ota_info.object = object;
    msg.union_body.ota_info.args1 = arg1;
    msg.union_body.ota_info.args2 = arg2;
    if (event == OTA_EVENT_OTA_PROGRESS) {
        return MSG_QUEUE_SEND(ota_obj->priv.ota_work_thd.msg_id, &msg, 0);
    }

    for (uint8_t i = 0; i < 5; i++) {
        ret = MSG_QUEUE_SEND(ota_obj->priv.ota_work_thd.msg_id, &msg, 0);
        if (ret == 0) {
            return 0;
        }
        msleep(2);
    }
    // LOGE("drop critical ota event=%d, arg1=%d", event, arg1);
    return ret;
}

static int on_post_ota_init(void *ptr)
{
    ota_manager_context_t *ctx = ptr;
    ctx->priv.ota_bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.ota_bcd->context = ctx;
    ctx->priv.ota_bcd->name = "ota_manager";
    ctx->priv.ota_bcd->id.code |= ID_BROADCAST_BLE_STATE_CHANGE;
    ctx->priv.ota_bcd->receiver_cb = on_ota_broadcast_receive;
    register_broadcast_receiver(ctx->priv.ota_bcd);
    RUN_NEW_THREAD(&ctx->priv.ota_work_thd, "ota_work_thd", 
        ota_upgrade_thread, RT_PRIO_OTA_DISPATCH_THREAD, OTA_THREAD_STACKSIZE);
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    } 
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_ota_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, ota_manager_context_t);
    ota_obj = ptr;
    ptr->is_ota_upgrading = is_ota_upgrading;
    ptr->dispatch_ota_event_wrapper = dispatch_ota_event_wrapper;
    ptr->get_ota_info = get_ota_info;
    return ADD_MODULE("ota_manager", ID_MODULE_OTA, 0, on_post_ota_init, ptr,
        list_head, sizeof(ota_manager_context_t));
}
