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

#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_thread_client.h"
#include "bth_if_evt_def.h"

static const gatt_client_callbacks_t* ui_gatt_evt_if_cbs = NULL;


static int32_t _btif_client_gatt_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    TRACE(2, "%s sub_cmd %d", __func__, sub_cmd);
    switch(sub_cmd)
    {
        case BTH_IF_CORE_COMM_RESULT_GATT_DP_CON_STATE_EVT: {
            app_gatt_dp_connection_state_evt_t *connection_state = (app_gatt_dp_connection_state_evt_t*)buf;
            if(ui_gatt_evt_if_cbs->datapath_connection_state_cb) {
                ui_gatt_evt_if_cbs->datapath_connection_state_cb(connection_state->connect_state, connection_state->conidx);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_GATT_DP_DATA_RCV_EVT: {
            app_gatt_dp_receive_data_evt_t *recv_data = (app_gatt_dp_receive_data_evt_t*)buf;
            if(ui_gatt_evt_if_cbs->datapath_receive_data_cb) {
                ui_gatt_evt_if_cbs->datapath_receive_data_cb(recv_data->data, recv_data->data_len);
            }
        }
        break;
        default:
        break;
    }

    return 0;
}

int32_t app_bth_gatt_if_client_init(void)
{
    app_bth_if_thread_client_result_register(APP_BTH_IF_USER_GATT, _btif_client_gatt_result_rsp_callback);
    return 0;
}

int32_t app_bth_gatt_if_client_open(void* ui_callback, uint8_t comm_type)
{
    TRACE(2, "[%s] callback_if_ptr=0x%p", __func__, ui_callback);
    ui_gatt_evt_if_cbs = ui_callback;
    bth_if_thread_client_open(APP_BTH_IF_USER_GATT, (uint32_t)ui_callback, comm_type);
    return 0;
}

int32_t app_bth_gatt_if_client_close(void)
{
    bth_if_thread_client_close(APP_BTH_IF_USER_GATT);
    ui_gatt_evt_if_cbs = NULL;
    return 0;
}

int32_t app_bth_gatt_if_client_reset(void)
{
    bth_if_thread_client_reset(APP_BTH_IF_USER_GATT);
    return 0;
}

int32_t app_bth_gatt_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len)
{
    bth_if_thread_client_ctrl(APP_BTH_IF_USER_GATT, ctrl_cmd, payload, payload_len);
    return 0;
}

