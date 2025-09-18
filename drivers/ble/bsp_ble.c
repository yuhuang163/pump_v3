/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "gatt_service.h" // IWYU pragma: keep
#include "ble_core_common.h" // IWYU pragma: keep
#include "besbt.h"
#include "bsp_ble_internal.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static ble_event_cb_func s_ble_event_callback = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ble_custom_mac_addr_set(uint8_t *mac_addr)
{
    bt_set_ble_local_address(mac_addr);
}

static void add_ble_custom_service_callback(ble_event_cb_func cb)
{
    if (cb) {
        s_ble_event_callback = cb;
    }
}

static int ble_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int ble_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int ble_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int ble_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    uint32_t *tmp = (uint32_t *)buf;
    uint8_t srv_id = tmp[0];
    uint8_t bind_state = tmp[1];
    uint8_t *p_data = (uint8_t *)tmp[2];
    uint16_t size = tmp[3];
    LOG_PBUS("write ble data");
    return ble_service_data_send(srv_id, p_data, size, bind_state);
}

static int ble_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *tmp = (uint32_t *)args;

    switch (op_code) {
        case OP_CODE_BLE_ADV_OPEN:
            ble_custom_gap_advs_start();
            break;

        case OP_CODE_BLE_ADV_CLOSE:
            ble_custom_gap_advs_stop();
            break;

        case OP_CODE_BLE_DISCONNECT: {
            ble_custom_gap_disconnect();
            break;
        }

        case OP_CODE_BLE_MAC_ADDR_SET: {
            uint8_t *mac_addr = (uint8_t *)tmp[0];
            ble_custom_mac_addr_set(mac_addr);
            break;
        }

        case OP_CODE_BLE_ADD_EVENT_CALLBACK: {
            ble_event_cb_func cb = (ble_event_cb_func)args;
            add_ble_custom_service_callback(cb);
            break;
        }

        case OP_CODE_BLE_ADD_DATA_CALLBACK: {
            ble_data_cb_func cb = (ble_data_cb_func)args;
            add_ble_custom_data_callback(cb);
            break;
        }

        default:
            LOGE("ble err op_code: %lx", op_code);
            break;
    }

    return 0;
}

static int ble_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int ble_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int ble_custom_event_callback(uintptr_t connhdl, gap_adv_event_t event, gap_adv_callback_param_t param)
{
    LOGI("ble conn index: %d, event: %d", connhdl, event);
    if (!s_ble_event_callback) {
        return 0;
    }
    switch ((int)event) {
        case GAP_ADV_EVENT_STARTED:
            s_ble_event_callback(BLE_EVENT_ADVERTISE_STARTED, NULL);
            break;
        case GAP_ADV_EVENT_STOPPED:
            s_ble_event_callback(BLE_EVENT_ADVERTISE_STOPPED, NULL);
            break;
        case GAP_ADV_EVENT_CONN_OPENED:
            s_ble_event_callback(BLE_EVENT_CONN_OPENED, NULL);
            break;
        case GAP_ADV_EVENT_CONN_CLOSED:
            s_ble_event_callback(BLE_EVENT_CONN_CLOSED, NULL);
            break;
        case GAP_ADV_EVENT_CONN_FAILED:
            s_ble_event_callback(BLE_EVENT_CONN_FAILED, NULL);
            break;
        case GAP_ADV_EVENT_CONN_MTU_EXCHANGED:
            s_ble_event_callback(BLE_EVENT_CONN_MTU_EXCHANGED, NULL);
            break;
        case GAP_ADV_EVENT_USER_CONFIRM:
            s_ble_event_callback(BLE_EVENT_USER_CONFIRM, NULL);
            break;
        case GAP_ADV_EVENT_CONN_ENCRYPTED:
            s_ble_event_callback(BLE_EVENT_CONN_ENCRYPTED, NULL);
            break;
        case GAP_ADV_EVENT_CONN_UPDATE_REQ:
            s_ble_event_callback(BLE_EVENT_CONN_UPDATE_REQ, NULL);
            break;
        case GAP_ADV_EVENT_CONN_PARAMS_UPDATE:
            s_ble_event_callback(BLE_EVENT_CONN_PARAMS_UPDATE, NULL);
            break;
        default:
            break;
    }
    return 0;
}

void ble_custom_service_init(void)
{
    ble_master_service_init();
    ble_factory_service_init();
    ble_remot_service_init();
}

void *register_device_for_ble(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "ble_device";
    node->id = ID_DEVICE_BLE;
    node->open = ble_dev_open;
    node->close = ble_dev_close;
    node->suspend = ble_dev_suspend;
    node->resume = ble_dev_resume;
    node->read = ble_dev_read;
    node->write = ble_dev_write;
    node->ioctl = ble_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}

