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
    return ota_obj->priv.is_working;
}

static void set_ota_upgrading(bool yes)
{
    LOG_OTA("ota_is_working: %d", yes);
    ota_obj->priv.is_working = yes;
}

static uint8_t get_ota_upgrade_progress(void)
{
    return ota_obj->priv.ota_progress;
}

static void set_ota_upgrade_progress(uint8_t progress)
{
    LOG_OTA("ota_progress: %d", progress);
    ota_obj->priv.ota_progress = progress;
}

static void on_ota_broadcast_receive(broadcast_mask_id_t id, 
                                        void *context, 
                                        int arg1, 
                                        int arg2,
                                        broadcast_priv_data_t *pdata)
{
    ota_manager_context_t *ctx = context;

    LOG_OTA("ota_broadcast rcv_id: 0x%lx", id.code);

    switch (id.code) {
        case ID_BROADCAST_BLE_CONNECT_STATE_CHANGE:
            break;

        default:
            LOGE("not support broadcast id");
            break;
    }
}

static void ota_upgrade_state_start(ota_manager_context_t *ctx)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);

    set_ota_upgrading(true);
    set_ota_upgrade_progress(0);

    if (wm && wm->dispatch_new_window_args) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_OTA_UPGRADE;
        msg.union_body.extra.is_background = 0x1;
        wm->dispatch_new_window_args(wm, &msg);
    }
}

static void ota_upgrade_state_progress(ota_manager_context_t *ctx, uint8_t progress)
{
    uint8_t val = progress > 100 ? 100 : progress;
    set_ota_upgrade_progress(val);
}

static void ota_upgrade_state_end(ota_manager_context_t *ctx, uint8_t state)
{
    ota_obj->priv.is_working = false;
    /// @todo OTA 页面如果需要结果，可发广播
    if (0x01 == state) {
        LOG_OTA("ota result sucess");
        set_ota_upgrade_progress(100);
    } else {
        LOG_OTA("ota result faile, reson 0x%02X", state);
        set_ota_upgrade_progress(0);
    }
}

static ota_upgrade_state_e get_ota_upgrade_state(void)
{
    return ota_obj->priv.ota_state;
}

static void set_ota_upgrade_state(ota_upgrade_state_e state)
{
    LOG_OTA("ota_state: %d", state);
    ota_obj->priv.ota_state = state;
}

static void ota_upgrade_thread(void *args)
{
    ota_manager_context_t *ctx = ota_obj;
    thread_context_t *thd = args;
    multi_func_msg_data_t msg = {0};
    ota_event_type_e event;

    CREATE_MSG_QUEUE(&thd->msg_id, OTA_WORK_DISPATCH_MESQ_NUM);
    while (1) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);

        event = msg.union_body.ota_evt_type.event;
        // LOG_OTA("ota event:%d", event);
        switch (event) {
            case OTA_EVENT_RESUME_OTA:
            case OTA_EVENT_START_OTA:
                ota_upgrade_state_start(ctx);
                break;

            case OTA_EVENT_OTA_PROGRESS: {
                uint8_t progress = msg.union_body.ota_evt_type.sub_code;
                LOG_OTA("ota progress %d", progress);
                ota_upgrade_state_progress(ctx, progress);
                break;
            }

            case OTA_EVENT_OTA_FAIL: {
                ota_upgrade_state_end(ctx, 0);
                break;
            }

            case OTA_EVENT_OTA_SUCCESS: {
                ota_upgrade_state_end(ctx, 0);
                break;
            }

            default:
                break;
        }
    }
}

static int dispatch_ota_event_wrapper(void *context, int event, uint32_t sub_code)
{
    ota_manager_context_t *ota_ctx = context;
    multi_func_msg_data_t msg = {0};
    msg.union_body.ota_evt_type.event = (ota_event_type_e)event;
    msg.union_body.ota_evt_type.sub_code = sub_code;

    return MSG_QUEUE_SEND(ota_ctx->priv.ota_work_thd.msg_id, &msg, 0);
}

static void ota_event_callback(ota_event_type_e event, int args)
{
    LOG_OTA("ota event router: %d, %d", event, args);
    dispatch_ota_event_wrapper(ota_obj, event, args);
}

static int on_post_ota_init(void *ptr)
{
    ota_manager_context_t *ctx = ptr;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ctx->priv.ble_dev = pbus->find_device(ID_DEVICE_BLE);
    if (ctx->priv.ble_dev && ctx->priv.ble_dev->ioctl) {
        DEVICE_IOCTL_ARGS(ctx->priv.ble_dev, OP_CODE_BLE_ADD_OTA_EVENT_CALLBACK, ota_event_callback);
    }
    ctx->priv.ota_bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.ota_bcd->context = ctx;
    ctx->priv.ota_bcd->name = "ota_manager";
    ctx->priv.ota_bcd->id.code |= ID_BROADCAST_BLE_CONNECT_STATE_CHANGE;
    ctx->priv.ota_bcd->receiver_cb = on_ota_broadcast_receive;
    register_broadcast_receiver(ctx->priv.ota_bcd);
    RUN_NEW_THREAD(&ctx->priv.ota_work_thd, "ota_work_thd", 
        ota_upgrade_thread, RT_PRIO_OTA_DISPATCH_THREAD, OTA_THREAD_STACKSIZE);
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
    ptr->set_ota_upgrading = set_ota_upgrading;
    ptr->get_ota_upgrade_progress = get_ota_upgrade_progress;
    ptr->set_ota_upgrade_progress = set_ota_upgrade_progress;
    ptr->get_ota_upgrade_state = get_ota_upgrade_state;
    ptr->set_ota_upgrade_state = set_ota_upgrade_state;

    return ADD_MODULE("ota_manager", ID_MODULE_OTA, 0, on_post_ota_init, ptr, list_head, sizeof(ota_manager_context_t));
}
