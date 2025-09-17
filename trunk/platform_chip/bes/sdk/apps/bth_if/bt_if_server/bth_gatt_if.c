/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifdef BLE_HOST_SUPPORT
#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_thread_server.h"
#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "bth_if_cmd_def.h"
#include "bth_if_evt_def.h"

static bool btif_module_enable = false;
static uint8_t  bth_if_comm_type = BTH_IF_COMM_TYPE_RPC;
static bt_callbacks_t *bth_gatt_if_cb = NULL;
static uint8_t bth_gatt_datapath_is_connected = false;


static void btif_gatt_datapath_server_tx_done_cb(void)
{

}

static void btif_gatt_datapath_server_mtu_exchanged_done_cb(uint8_t conidx, uint16_t mtu)
{

}

static void btif_gatt_datapath_server_data_received_cb(uint8_t *p_buff, uint16_t bufLength)
{
    app_bth_if_payload_param_info_t info;
    ASSERT(bufLength <= 120, "%s datapath recv datalen %d too long", __func__, bufLength);

    info.gatt_datapath_reveive_evt_info.data_len = bufLength;
    memcpy(info.gatt_datapath_reveive_evt_info.data, p_buff, bufLength);
    if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
        bth_if_send_result_to_app(APP_BTH_IF_USER_GATT, (uint8_t*)&info.context,
                        sizeof(app_gatt_dp_receive_data_evt_t), BTH_IF_CORE_COMM_RESULT_GATT_DP_DATA_RCV_EVT);
    }

}

static void btif_gatt_datapath_server_disconnected_done_cb(uint8_t conidx)
{
    app_bth_if_payload_param_info_t info;
    bth_gatt_datapath_is_connected = false;
    TRACE(2, "%s conidx %d", __func__, conidx);
    info.gatt_datapath_connection_evt_info.conidx = conidx;
    info.gatt_datapath_connection_evt_info.connect_state = 0;
    if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
            bth_if_send_result_to_app(APP_BTH_IF_USER_GATT, (uint8_t*)&info.context,
                            sizeof(app_gatt_dp_connection_state_evt_t), BTH_IF_CORE_COMM_RESULT_GATT_DP_CON_STATE_EVT);
    }
}

static void btif_gatt_datapath_server_connected_done_cb(uint8_t conidx)
{
    app_bth_if_payload_param_info_t info;
    bth_gatt_datapath_is_connected = true;
    TRACE(2, "%s conidx %d", __func__, conidx);
    info.gatt_datapath_connection_evt_info.conidx = conidx;
    info.gatt_datapath_connection_evt_info.connect_state = 1;
    if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
            bth_if_send_result_to_app(APP_BTH_IF_USER_GATT, (uint8_t*)&info.context,
                            sizeof(app_gatt_dp_connection_state_evt_t), BTH_IF_CORE_COMM_RESULT_GATT_DP_CON_STATE_EVT);
    }
}

static void btif_gatt_datapath_event_cb(DP_EVENT_TYPE_E event_type, ble_if_app_dp_param_u*para_p)
{
    TRACE(2, "%s recv event %d", __func__, event_type);
    switch(event_type)
    {
        case DP_CONN_DONE:
        {
            uint8_t conidx = *(uint8_t*)para_p;
            btif_gatt_datapath_server_connected_done_cb(conidx);
        }
        break;
        case DP_DISCONN_DONE:
        {
            uint8_t conidx = *(uint8_t*)para_p;
            btif_gatt_datapath_server_disconnected_done_cb(conidx);
        }
        break;
        case DP_MTU_CHANGE_DONE:
        {
            app_dp_mtu_exchange_msg_t *mtu = (app_dp_mtu_exchange_msg_t *)para_p;
            btif_gatt_datapath_server_mtu_exchanged_done_cb(mtu->conidx, mtu->mtu);
        }
        break;
        case DP_TX_DONE:
            btif_gatt_datapath_server_tx_done_cb();
        break;
        case DP_DATA_RECEIVED:
        {
            app_dp_rec_data_msg_t *data = (app_dp_rec_data_msg_t *)para_p;
            btif_gatt_datapath_server_data_received_cb(data->data, data->data_len);
        }
        break;
        default:
            break;
    }
}


uint32_t app_bth_gatt_if_enable(void *callback_func_if, uint8_t comm_type)
{

    bes_ble_datapath_server_register_event_cb(btif_gatt_datapath_event_cb);
    bth_gatt_if_cb = (bt_callbacks_t *)callback_func_if;
    bth_if_comm_type = comm_type;
    btif_module_enable = true;

    //return the bluetooth interface ptr to client side
    app_bt_init_cmp_evt_t report_ret;
    report_ret.module_interface_addr = (uint32_t)NULL;
    bth_if_send_result_to_app(APP_BTH_IF_USER_GATT, (uint8_t*)&report_ret,
                                sizeof(app_bt_init_cmp_evt_t), BTH_IF_CORE_COMM_RESULT_INIT_CMP_EVT);

    return 0;
}

uint32_t app_bth_gatt_if_disable(void)
{
    btif_module_enable = false;
    bth_gatt_if_cb = NULL;
    return 0;
}

uint32_t app_bth_gatt_if_restart(void)
{
    return 0;
}

uint32_t app_bth_gatt_if_ctrl(uint32_t sub_cmd, uint8_t *ptr, uint32_t ptr_len)
{
    return 0;
}


static const app_bth_if_ipc_msg_t bth_gattif_ipc_handle_table = {
    .open           = app_bth_gatt_if_enable,
    .close          = app_bth_gatt_if_disable,
    .restart        = app_bth_gatt_if_restart,
    .ctrl           = app_bth_gatt_if_ctrl,
};

void app_bth_gatt_if_init(void)
{
    bth_if_thread_server_register(APP_BTH_IF_USER_GATT, &bth_gattif_ipc_handle_table);
}

#endif


