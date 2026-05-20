/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ota_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

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
static void ota_touch_key_event_callback(key_ota_event_e event, int arg1, int arg2)
{
    LOG_OTA("ota event: %d/%d/%d", event, arg1, arg2);
    if (event == KEY_OTA_EVENT_UPDATE_START) {
        ota_obj->dispatch_ota_event_wrapper(OTA_EVENT_START_OTA, OTA_OBJECT_TOUCH_KEY_FIRMWARE, arg1, arg2);
    } else if (event == KEY_OTA_EVENT_UPDATE_PROGRESS) {
        ota_obj->dispatch_ota_event_wrapper(OTA_EVENT_OTA_PROGRESS, OTA_OBJECT_TOUCH_KEY_FIRMWARE, arg1, arg2);
    } else if (event == KEY_OTA_EVENT_UPDATE_FAIL) {
        ota_obj->dispatch_ota_event_wrapper(OTA_EVENT_OTA_FAIL, OTA_OBJECT_TOUCH_KEY_FIRMWARE,  0, 0);
    } else if (event == KEY_OTA_EVENT_UPDATE_SUCCESS) {
        ota_obj->dispatch_ota_event_wrapper(OTA_EVENT_OTA_SUCCESS, OTA_OBJECT_TOUCH_KEY_FIRMWARE, 0, 0);
    }
}

static void ota_main_fw_event_callback(ota_event_type_e event, int arg1, int arg2)
{
    LOG_OTA("ota event: %d/%d", event, arg1, arg2);
    ota_obj->dispatch_ota_event_wrapper(event, OTA_OBJECT_MAIN_FIRMWARE, arg1, arg2);
}

static int on_broadcast_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *pdata)
{
    ota_manager_context_t *ctx = context;
    EXCEPTION_CHECK_INT(ctx);
    switch (id.code) {
        case ID_BROADCAST_BLE_STATE_CHANGE: {
            ota_info_t ota_info = {0};
            ctx->get_ota_info(&ota_info); 
            if (arg1 == BLE_BROADCAST_DISCONNECT && 
                ota_info.is_working && ota_info.object == OTA_OBJECT_MAIN_FIRMWARE) {
                ctx->dispatch_ota_event_wrapper(OTA_EVENT_OTA_FAIL, OTA_OBJECT_MAIN_FIRMWARE, 0, 0);
            }
            break;
        }
        default:
            break;
    }
    return 0;
}

static int on_post_init_route(ota_manager_context_t *thiz)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *ble_dev = pbus->find_device(ID_DEVICE_BLE);
    if (ble_dev && ble_dev->ioctl) {
        DEVICE_IOCTL_ARGS(ble_dev, OP_CODE_BLE_ADD_OTA_EVENT_CALLBACK, ota_main_fw_event_callback);
    }
    
    rt_device_node_t *key_dev = pbus->find_device(ID_DEVICE_INPUT_KEY);
    if (key_dev && key_dev->ioctl) {
        DEVICE_IOCTL_ARGS(key_dev , OP_CODE_INPUT_ADD_KEY_OTA_EVENT_CB, ota_touch_key_event_callback);
    }
    LOG_OTA("ota post init route");
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_ota_hook(ota_manager_context_t *ctx)
{
    ota_obj = ctx;
    ctx->hook = u_malloz(sizeof(struct __ota_manager_hook));
    ctx->hook->on_post_init_route = on_post_init_route;
    ctx->hook->on_ota_receiver_route = on_broadcast_receiver_route;
}

