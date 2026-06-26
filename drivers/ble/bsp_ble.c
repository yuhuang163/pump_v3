/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "gatt_service.h" // IWYU pragma: keep
#include "ota_control.h" // IWYU pragma: keep
#include "ble_core_common.h" // IWYU pragma: keep
#include "bsp_ble_internal.h" // IWYU pragma: keep
#include "ble_test/ble_tests.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BLE_MTU_SIZE_DEFAULT      31 /// ATT头大小

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static ble_event_cb_func s_ble_event_callback = NULL;
static uint16_t s_ble_mtu_size = BLE_MTU_SIZE_DEFAULT;
static ota_event_cb_func s_ota_event_callback = NULL;
static uint32_t img_total_size = 0;
static uint32_t img_received_size = 0;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void add_ble_custom_service_callback(ble_event_cb_func cb)
{
    if (cb) {
        // LOG_PBUS("add ble event callback");
        s_ble_event_callback = cb;
    }
}

static void add_ble_custom_ota_event_callback(ota_event_cb_func cb)
{
    if (cb) {
        // LOG_PBUS("add ota event callback");
        s_ota_event_callback = cb;
    }
}


#if CFG_OTA_EVENT_CALLBACK
static void ota_event_router(ota_state_type_e state, void *ota_info, int arg1, int arg2)
{
    if (s_ota_event_callback == NULL) {
        LOG_PBUS("ota event callback is NULL");
    }

    if (state == OTA_STATE_FAIL_EXIT) {
        LOG_PBUS("ota fail");
        s_ota_event_callback(OTA_EVENT_OTA_FAIL, img_total_size, img_received_size);
    }
    
    if (state == OTA_STATE_UPDATE_IMG_TYPE) {
        LOG_PBUS("ota update img type: %d", arg1);
        s_ota_event_callback(OTA_EVENT_UPDATE_IMG_TYPE, img_total_size, img_received_size);
    }

    if (state == OTA_STATE_RESUME_TOTAL_SIZE) {
        img_total_size = arg1;
        LOG_PBUS("ota resume total size: %d", img_total_size);
    }

    if (state == OTA_STATE_RESUME_OFFSET_SIZE) {
        img_received_size = arg1;
        LOG_PBUS("ota resume offset size: %d", img_received_size);
    }   

    if (ota_info == NULL) {
        return;
    }

    OTA_LEGACY_PACKET_T *packet = (OTA_LEGACY_PACKET_T *)ota_info;
    switch (packet->packetType) {
        case OTA_CMD_SET_USER:
            // s_ota_event_callback(OTA_EVENT_SET_USER_TYPE, 0);
            break;
        case OTA_COMMAND_START:
            img_total_size = packet->payload.cmdOtaStart.imageSize;
            img_received_size = 0;
            LOG_PBUS("img total size:%d", img_total_size);
            s_ota_event_callback(OTA_EVENT_START_OTA, img_total_size, img_received_size);
            break;
        case OTA_COMMAND_RESUME_VERIFY:
            s_ota_event_callback(OTA_EVENT_RESUME_OTA, img_total_size, img_received_size);
            break;
        case OTA_DATA_PACKET:
            img_received_size += (arg2 - 1);
            LOG_PBUS("img size:%d/%d", img_received_size, img_total_size);
            s_ota_event_callback(OTA_EVENT_OTA_PROGRESS, img_total_size, img_received_size);
            break;
        case OTA_COMMAND_IMAGE_APPLY:
            LOG_PBUS("ota success");
            s_ota_event_callback(OTA_EVENT_OTA_SUCCESS, img_total_size, img_received_size);
            break;
    }
}
#endif

static int ble_dev_open(struct rt_device_node *dev)
{
#if CFG_OTA_EVENT_CALLBACK
    ota_event_router_register(ota_event_router);
#endif
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
    ble_service_send_data_t *tmp = (ble_service_send_data_t *)buf;
    uint8_t srv_id = tmp->srv_id;
    uint8_t bind_state = tmp->bind_state;
    uint8_t *p_data = tmp->p_data;

    // LOG_PBUS("write ble data");
    return ble_service_data_send(srv_id, p_data, n_bytes, bind_state);
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
            uint8_t *mac_addr = (uint8_t *)tmp[1];
            if (mac_addr) {
                return ble_custom_gap_mac_addr_set(mac_addr);
            }
            break;
        }

        case OP_CODE_BLE_MAC_ADDR_GET: {
            uint8_t *get_mac_addr = (uint8_t *)tmp[0];
            if (get_mac_addr) {
                return ble_custom_gap_mac_addr_get(get_mac_addr);
            }
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

        case OP_CODE_BLE_ADD_OTA_EVENT_CALLBACK: {
            ota_event_cb_func cb = (ota_event_cb_func)args;
            add_ble_custom_ota_event_callback(cb);
            break;
        }
        case OP_CODE_BLE_TEST_ENTER_SIGNALINGTEST_MODE: {
            ble_test_enter_signalingtest_mode();
            break;
        }
        case OP_CODE_BLE_TEST_ENTER_NOSIGNALINGTEST_MODE: {
            ble_test_enter_nosignalingtest_mode();
            break;
        }

        case OP_CODE_BLE_TEST_XTAL_CALIB_PROC_START: {
            ble_test_xtal_calib_proc_start();
            break;
        }
        case OP_CODE_BLE_TEST_XTAL_CALIB_SET: {
            uint32_t fcap = *(uint32_t *)args;
            ble_test_xtal_calib_set(fcap);
            break;
        }
        case OP_CODE_BLE_TEST_XTAL_CALIB_GET: {
            uint32_t *fcap = (uint32_t *)args;
            ble_test_xtal_calib_get(fcap);
            break;
        }
        case OP_CODE_BLE_TEST_XTAL_CALIB_END: {
            ble_test_xtal_calib_end();
            break;
        }
        case OP_CODE_BLE_TEST_GET_RSSI_VALUE: {
            int32_t *rssi= (int32_t *)args;
            ble_test_get_rssi_value(rssi);
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

static void ble_invoke_event_callback(ble_event_e event, uintptr_t connhdl,
                                      const gap_adv_callback_param_t *gap_param)
{
    ble_hal_event_param_t hal_param = {0};

    if (!s_ble_event_callback) {
        return;
    }

    if (gap_param) {
        if (gap_param->conn_opened) {
            hal_param.connhdl = gap_param->conn_opened->connhdl;
            hal_param.error_code = gap_param->conn_opened->error_code;
            hal_param.con_idx = gap_param->conn_opened->con_idx;
        } else if (gap_param->conn_closed) {
            hal_param.connhdl = gap_param->conn_closed->connhdl;
            hal_param.error_code = gap_param->conn_closed->error_code;
            hal_param.con_idx = gap_param->conn_closed->con_idx;
        } else if (gap_param->conn_failed) {
            hal_param.error_code = gap_param->conn_failed->error_code;
        } else if (gap_param->mtu_exchanged) {
            hal_param.connhdl = gap_param->mtu_exchanged->connhdl;
            hal_param.error_code = gap_param->mtu_exchanged->error_code;
            hal_param.con_idx = gap_param->mtu_exchanged->con_idx;
            hal_param.mtu = gap_param->mtu_exchanged->mtu;
        } else if (gap_param->conn_params_update) {
            hal_param.connhdl = gap_param->conn_params_update->connhdl;
            hal_param.error_code = gap_param->conn_params_update->error_code;
            hal_param.con_idx = gap_param->conn_params_update->con_idx;
        }
    }
    if (hal_param.connhdl == 0u) {
        hal_param.connhdl = (uint16_t)connhdl;
    }
    if (connhdl == 0u) {
        connhdl = hal_param.connhdl;
    }

    switch (event) {
    case BLE_EVENT_ADVERTISE_STARTED:
    case BLE_EVENT_ADVERTISE_STOPPED:
    case BLE_EVENT_USER_CONFIRM:
    case BLE_EVENT_CONN_ENCRYPTED:
    case BLE_EVENT_CONN_UPDATE_REQ:
        s_ble_event_callback(event, connhdl, NULL);
        break;
    default:
        s_ble_event_callback(event, connhdl, &hal_param);
        break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int ble_custom_event_callback(uintptr_t connhdl, gap_adv_event_t event, gap_adv_callback_param_t param)
{
    LOG_PBUS("ble conn index: %d, event: %d", connhdl, event);
    switch ((int)event) {
        case GAP_ADV_EVENT_STARTED:
            ble_invoke_event_callback(BLE_EVENT_ADVERTISE_STARTED, connhdl, NULL);
            break;
        case GAP_ADV_EVENT_STOPPED:
            ble_invoke_event_callback(BLE_EVENT_ADVERTISE_STOPPED, connhdl, NULL);
            break;
        case GAP_ADV_EVENT_CONN_OPENED:
            ble_invoke_event_callback(BLE_EVENT_CONN_OPENED, connhdl, &param);
            break;
        case GAP_ADV_EVENT_CONN_CLOSED:
            s_ble_mtu_size = BLE_MTU_SIZE_DEFAULT;
            ble_invoke_event_callback(BLE_EVENT_CONN_CLOSED, connhdl, &param);
            break;
        case GAP_ADV_EVENT_CONN_FAILED:
            ble_invoke_event_callback(BLE_EVENT_CONN_FAILED, connhdl, &param);
            break;
        case GAP_ADV_EVENT_CONN_MTU_EXCHANGED:
            if (param.mtu_exchanged) {
                s_ble_mtu_size = param.mtu_exchanged->mtu;
            }
            LOG_PBUS("ble mtu size: %d", s_ble_mtu_size);
            ble_invoke_event_callback(BLE_EVENT_CONN_MTU_EXCHANGED, connhdl, &param);
            break;
        case GAP_ADV_EVENT_USER_CONFIRM:
            ble_invoke_event_callback(BLE_EVENT_USER_CONFIRM, connhdl, NULL);
            break;
        case GAP_ADV_EVENT_CONN_ENCRYPTED:
            ble_invoke_event_callback(BLE_EVENT_CONN_ENCRYPTED, connhdl, &param);
            break;
        case GAP_ADV_EVENT_CONN_UPDATE_REQ:
            ble_invoke_event_callback(BLE_EVENT_CONN_UPDATE_REQ, connhdl, NULL);
            break;
        case GAP_ADV_EVENT_CONN_PARAMS_UPDATE:
            ble_invoke_event_callback(BLE_EVENT_CONN_PARAMS_UPDATE, connhdl, &param);
            break;
        default:
            break;
    }
    return 0;
}

void ble_custom_tx_ccc_changed(uint8_t conidx, uint16_t connhdl, bool notify_enabled, uint8_t service_id)
{
    LOG_PBUS("ble tx ccc changed %d, service_id=%d", notify_enabled, service_id);
    if (!s_ble_event_callback) {
        return;
    }
    if (notify_enabled) {
        ble_hal_event_param_t hal_param = {
            .connhdl = connhdl,
            .service_id = service_id,
        };
        s_ble_event_callback(BLE_EVENT_TX_CCC_ENABLED, (uintptr_t)connhdl, &hal_param);
    } else {
        ble_hal_event_param_t hal_param = {
            .connhdl = connhdl,
            .service_id = service_id,
        };
        s_ble_event_callback(BLE_EVENT_TX_CCC_DISABLED, (uintptr_t)connhdl, &hal_param);
    }
}

uint16_t ble_custom_mtu_size_get(void)
{
    return s_ble_mtu_size;
}

void ble_custom_service_init(void)
{
    ble_master_service_init();
    ble_factory_service_init();
    ble_remot_service_init();
    ble_data_bury_service_init();
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

