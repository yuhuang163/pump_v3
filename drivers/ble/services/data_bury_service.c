/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "bsp_common.h"
#include "gatt_service.h"
#include "data_bury_service.h"
#include "ble_service.h"
#include "bsp_ble_internal.h"

/*
 * DEFINES
 *****************************************************************************************
 */
GATT_DECL_128_LE_PRI_SERVICE(ble_data_bury_service, ROOT_DATA_BURY_SRV_UUID_128);

GATT_DECL_128_LE_CHAR(ble_data_bury_write_character, ROOT_DATA_BURY_SRV_WRITE_CHAR_UUID_128, GATT_WR_CMD,
                      ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(ble_data_bury_write_cudd, ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(ble_data_bury_notify_character, ROOT_DATA_BURY_SRV_NOTIFY_CHAR_UUID_128, GATT_NTF_PROP,
                      ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(ble_data_bury_notify_cccd, ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(ble_data_bury_notify_cudd, ATT_SEC_NONE);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const char data_bury_notify_desc[] = "data bury notify";
static const char data_bury_write_desc[]  = "data bury write";

static const gatt_attribute_t ble_data_bury_attr_list[] = {
    gatt_attribute(ble_data_bury_service),
    gatt_attribute(ble_data_bury_write_character),
    gatt_attribute(ble_data_bury_notify_character),
    gatt_attribute(ble_data_bury_notify_cccd),
};

static uint8_t s_notification_enabled = false;
static uint8_t s_conn_idx             = 0;

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static void ble_data_bury_service_write_received(uint8_t conidx, uint16_t connhdl, const uint8_t *data, uint16_t len)
{
    (void)connhdl;
    ble_service_data_recv(BLE_SRV_ID_DATA_BURY, (uint8_t *)data, len);
}

static void ble_data_bury_service_notify_ccc_changed(uint8_t conidx, uint16_t connhdl, bool notify_enabled)
{
    s_notification_enabled = notify_enabled;
    s_conn_idx             = conidx;
    ble_custom_tx_ccc_changed(conidx, connhdl, notify_enabled);
}

static void ble_data_bury_service_notify_done(uint8_t conidx, uint16_t connhdl)
{
    (void)conidx;
    (void)connhdl;
}

static void ble_data_bury_service_mtu_exchanged(uint8_t conidx, uint16_t connhdl, uint16_t mtu)
{
    (void)conidx;
    (void)connhdl;
    LOG_PBUS("data bury service mtu %d", mtu);
}

static void ble_data_bury_service_disconnected(uint8_t conidx, uint16_t connhdl)
{
    (void)conidx;
    (void)connhdl;
    s_notification_enabled = false;
}

static int ble_data_bury_service_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    switch (event) {
    case GATT_SERV_EVENT_CHAR_WRITE: {
        gatt_server_char_write_t *p = param.char_write;
        if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL) {
            return false;
        }
        ble_data_bury_service_write_received(svc->con_idx, svc->connhdl, p->value, p->value_len);
        return true;
    }
    case GATT_SERV_EVENT_DESC_WRITE: {
        gatt_server_desc_write_t *p = param.desc_write;
        uint16_t config           = CO_COMBINE_UINT16_LE(p->value);
        bool notify_enabled       = false;
        if (config & GATT_CCCD_SET_NOTIFICATION) {
            notify_enabled = true;
        }
        ble_data_bury_service_notify_ccc_changed(svc->con_idx, svc->connhdl, notify_enabled);
        return true;
    }
    case GATT_SERV_EVENT_NTF_TX_DONE:
    case GATT_SERV_EVENT_INDICATE_CFM:
        ble_data_bury_service_notify_done(svc->con_idx, svc->connhdl);
        break;
    case GATT_SERV_EVENT_MTU_CHANGED: {
        gatt_server_mtu_changed_t *p = param.mtu_changed;
        ble_data_bury_service_mtu_exchanged(svc->con_idx, svc->connhdl, p->mtu);
        break;
    }
    case GATT_SERV_EVENT_CONN_CLOSED:
        ble_data_bury_service_disconnected(svc->con_idx, svc->connhdl);
        break;
    case GATT_SERV_EVENT_DESC_READ: {
        gatt_server_desc_read_t *p = param.desc_read;
        if (p->descriptor == ble_data_bury_notify_cudd) {
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)data_bury_notify_desc, sizeof(data_bury_notify_desc));
            return true;
        }
        if (p->descriptor == ble_data_bury_write_cudd) {
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)data_bury_write_desc, sizeof(data_bury_write_desc));
            return true;
        }
        break;
    }
    default:
        break;
    }
    return 0;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int ble_data_bury_service_notify_data_send(uint8_t *data, uint32_t len)
{
    return gatts_send_notification(gap_conn_bf(s_conn_idx), ble_data_bury_notify_character, data, (uint16_t)len);
}

bool ble_data_bury_service_notify_status_get(void)
{
    return s_notification_enabled;
}

void ble_data_bury_service_init(void)
{
    s_notification_enabled = false;
    gatts_register_service(ble_data_bury_attr_list, LENGTH_OF_ARRAY(ble_data_bury_attr_list),
                           ble_data_bury_service_callback, NULL);
}
