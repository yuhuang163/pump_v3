/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "gatt_service.h"
#include "factory_service.h"
#include "ble_service.h"

/*
* DEFINES
*****************************************************************************************
*/
GATT_DECL_128_LE_PRI_SERVICE(ble_factory_service,
    ROOT_FACTORY_SRV_UUID_128);

GATT_DECL_128_LE_CHAR(ble_factory_rx_character,
    ROOT_FACTORY_SRV_RX_CHAR_UUID_128,
    GATT_WR_REQ|GATT_WR_CMD,
    ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(ble_factory_rx_cudd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(ble_factory_tx_character,
    ROOT_FACTORY_SRV_TX_CHAR_UUID_128,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(ble_factory_tx_cccd,
    ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(ble_factory_tx_cudd,
    ATT_SEC_NONE);

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const char factory_tx_desc[] = "fac tx";
static const char factory_rx_desc[] = "fac rx";
static const gatt_attribute_t ble_factory_attr_list[] = {
    /* Service */
    gatt_attribute(ble_factory_service),
    /* Characteristics */
    gatt_attribute(ble_factory_rx_character),
    // gatt_attribute(ble_factory_rx_cudd),
    /* Characteristics */
    gatt_attribute(ble_factory_tx_character),
    gatt_attribute(ble_factory_tx_cccd),
    // gatt_attribute(ble_factory_tx_cudd),
};

static uint8_t notification_enabled = false;
static uint8_t conn_idx = 0;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ble_factory_service_rx_data_received(uint8_t conidx, uint16_t connhdl, const uint8_t *data, uint16_t len)
{
    // LOG_PBUS("factory service recv data length %d", len);
    ble_service_data_recv(BLE_SRV_ID_FACTORY, (uint8_t *)data, len);
    // ble_factory_service_notify_data_send((uint8_t *)data, len);
}

static void ble_factory_service_tx_ccc_changed(uint8_t conidx, uint16_t connhdl, bool notify_enabled)
{
    notification_enabled = notify_enabled;
    conn_idx = conidx;
}

static void ble_factory_service_tx_done(uint8_t conidx, uint16_t connhdl)
{
    // TODO: send tx done event
}

static void ble_factory_service_mtu_exchanged(uint8_t conidx, uint16_t connhdl, uint16_t mtu)
{
    LOG_PBUS("factory service mtu exchanged %d", mtu);
    // TODO: send mtu exchanged event
}

static void ble_factory_service_disconnected(uint8_t conidx, uint16_t connhdl)
{
    notification_enabled = false;
    // TODO: send disconnected event
}

static int ble_factory_service_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    switch (event) {
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL) {
                return false;
            }
            ble_factory_service_rx_data_received(svc->con_idx, svc->connhdl, p->value, p->value_len);
            return true;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            bool notify_enabled = false;
            if (config & GATT_CCCD_SET_NOTIFICATION) {
                notify_enabled = true;
            }
            ble_factory_service_tx_ccc_changed(svc->con_idx, svc->connhdl, notify_enabled);
            return true;
        }
        case GATT_SERV_EVENT_NTF_TX_DONE:
        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            ble_factory_service_tx_done(svc->con_idx, svc->connhdl);
            break;
        }
        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            ble_factory_service_mtu_exchanged(svc->con_idx, svc->connhdl, p->mtu);
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            ble_factory_service_disconnected(svc->con_idx, svc->connhdl);
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            if (p->descriptor == ble_factory_tx_cudd) {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)factory_tx_desc, sizeof(factory_tx_desc));
                return true;
            } else if (p->descriptor == ble_factory_rx_cudd) {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)factory_rx_desc, sizeof(factory_rx_desc));
                return true;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int ble_factory_service_notify_data_send(uint8_t* data, uint32_t len)
{
    return gatts_send_notification(gap_conn_bf(conn_idx), ble_factory_tx_character,
                data, (uint16_t)len);
}

bool ble_factory_service_notify_status_get(void)
{
    return notification_enabled;
}

void ble_factory_service_init(void)
{
    notification_enabled = false;
    gatts_register_service(ble_factory_attr_list, LENGTH_OF_ARRAY(ble_factory_attr_list), 
                ble_factory_service_callback, NULL);
}