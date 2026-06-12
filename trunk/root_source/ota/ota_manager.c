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
#define OTA_PROGRESS_TIMER_PERIOD_MS    (10)
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

static void get_ota_info(ota_info_t *info)
{
    *info = ota_obj->priv.ota_info;
}

static void on_ota_broadcast_receive(broadcast_mask_id_t id, void *context, 
                                        int arg1, int arg2, broadcast_priv_data_t *pdata)
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

static void set_ota_upgrade_progress(ota_object_e object, uint32_t total_size, uint32_t recv_size)
{
    ota_info_t *info = &ota_obj->priv.ota_info;
    info->recv_size = recv_size;
    info->total_size = total_size;
    if (info->total_size > 0) {
        info->progress = (info->recv_size * 100) / info->total_size;
    } else {
        LOGE("ota size error:%d/%d", info->recv_size, info->total_size);
    }
}

static int start_ota_upgrade(ota_object_e object, on_write_img_data_cb_ptr cb)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    ota_info_t *info = &ota_obj->priv.ota_info;
    info->object = object;
    info->state = OTA_UPGRADE_STATE_START;
    ota_obj->priv.write_img_data_cb = cb;
    set_ota_upgrading(true);
    set_ota_upgrade_state(OTA_UPGRADE_STATE_START);
    if (wm && wm->dispatch_new_window_args && 
        wm->is_current_window && wm->is_current_window(wm, ID_WINDOW_OTA_UPGRADE) == 0) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_OTA_UPGRADE;
        msg.union_body.extra.is_bg_dispatch = 0x01;
        wm->dispatch_new_window_args(wm, &msg);
    }
    SEND_BROADCAST_ARGS1(ID_BROADCAST_OTA_STATE_CHANGE, OTA_UPGRADE_STATE_START);
    return 0;
}

static void end_ota_upgrade(ota_object_e object, uint8_t state)
{
    set_ota_upgrading(false);
    ota_info_t *info = &ota_obj->priv.ota_info;
    if (state) {
        LOG_OTA("ota result sucess");
        set_ota_upgrade_progress(object, info->total_size, info->total_size);
        set_ota_upgrade_state(OTA_UPGRADE_STATE_SUCCESS);
        SEND_BROADCAST_ARGS1(ID_BROADCAST_OTA_STATE_CHANGE, OTA_UPGRADE_STATE_SUCCESS);
        
    } else {
        LOG_OTA("ota result faile, reson 0x%02X", state);
        set_ota_upgrade_progress(object, info->total_size, 0);
        set_ota_upgrade_state(OTA_UPGRADE_STATE_FAIL);
        SEND_BROADCAST_ARGS1(ID_BROADCAST_OTA_STATE_CHANGE, OTA_UPGRADE_STATE_FAIL);
    }
    ota_obj->priv.write_img_data_cb = NULL;
}

static int ota_write_image_data(ota_object_e object, uint32_t offset_addr, uint8_t *data, uint16_t data_len)
{
    if (ota_obj->priv.write_img_data_cb == NULL) {
        LOGE("img data cb is null");
        return -1;
    }
    
    if (data == NULL) {
        LOGE("img data is null");
        return -1;
    }
    
    if (data_len > OTA_BLOCK_SIZE || data_len == 0) {
        LOGE("img data size error: %d", data_len);
        return -1;
    }

    ota_image_info_t *img = u_malloz(sizeof(ota_image_info_t));
    if (img == NULL) {
        LOGE("ota image data info malloc failed");
        return -1;
    }
    memcpy(img->data, data, data_len);
    img->offset_addr = offset_addr;
    img->data_size = data_len;
    if (MSG_QUEUE_SEND(ota_obj->priv.ota_work_thd.msg_id, &img, OS_WAIT_FOREVER) != 0) {
        LOGE("ota write image data send failed");
        u_free(img);
        return -1;
    }

    return 0;
}

static void ota_upgrade_thread(void *args)
{
    ota_manager_context_t *ctx = ota_obj;
    thread_context_t *thd = args;
    ota_image_info_t *img = NULL;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, OTA_WORK_DISPATCH_MESQ_NUM, sizeof(ota_image_info_t *));
    while (1) {
        img = NULL;
        MSG_QUEUE_RECV(thd->msg_id, &img, OS_WAIT_FOREVER);
        if (img != NULL) {
            if (ota_obj->priv.write_img_data_cb && img->data_size > 0) {
                LOG_OTA("offset_addr: %d, data_size: %d", img->offset_addr, img->data_size);
                ota_obj->priv.write_img_data_cb(ctx, img->offset_addr, img->data, img->data_size);
            }
            u_free(img);
        }
    }
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
    ptr->get_ota_info = get_ota_info;
    ptr->start_ota_upgrade = start_ota_upgrade;
    ptr->set_ota_upgrade_progress = set_ota_upgrade_progress;
    ptr->end_ota_upgrade = end_ota_upgrade;
    ptr->write_image_data = ota_write_image_data;
    return ADD_MODULE("ota_manager", ID_MODULE_OTA, 0, on_post_ota_init, ptr,
        list_head, sizeof(ota_manager_context_t));
}
