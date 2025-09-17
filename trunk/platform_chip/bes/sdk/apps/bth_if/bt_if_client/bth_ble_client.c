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
#include "bth_walkie_talkie_stream.h"

static const ble_client_callbacks_t* ui_ble_evt_if_cbs = NULL;
static ble_client_walkie_talkie_callback_t *walkie_talkie_cb = NULL;


void btif_client_walkie_talkie_start_capture(void)
{
    TRACE(1, "%s", __func__);
#ifdef BLE_WALKIE_TALKIE
    wt_audio_capture_stream_start();
#endif
}

void btif_client_walkie_talkie_stop_capture(void)
{
    TRACE(1, "%s", __func__);
#ifdef BLE_WALKIE_TALKIE
    wt_audio_capture_stream_stop();
#endif
}

void btif_client_register_walkie_talkie_callback(ble_client_walkie_talkie_callback_t *cb)
{
    TRACE(2, "%s cb %p", __func__, cb);
    walkie_talkie_cb = cb;
}

static int32_t _btif_client_ble_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    TRACE(2, "%s sub_cmd %d", __func__, sub_cmd);
    app_bth_if_payload_param_info_t *result_info = (app_bth_if_payload_param_info_t*)buf;

    switch(sub_cmd)
    {
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_STREAM_EVT:
            if(result_info->ble_walkie_talkie_recv_stream_evt_info.state)
            {
                //start stream
                if(walkie_talkie_cb && walkie_talkie_cb->ble_client_walkie_talkie_playback_stream_start_cb)
                {
                    walkie_talkie_cb->ble_client_walkie_talkie_playback_stream_start_cb(
                            result_info->ble_walkie_talkie_recv_stream_evt_info.device_id,
                            result_info->ble_walkie_talkie_recv_stream_evt_info.kfifo);
                }
            }
            else
            {
                //stop stream
                if(walkie_talkie_cb && walkie_talkie_cb->ble_client_walkie_talkie_playback_stream_stop_cb)
                {
                    walkie_talkie_cb->ble_client_walkie_talkie_playback_stream_stop_cb(
                        result_info->ble_walkie_talkie_recv_stream_evt_info.device_id);
                }
            }
            if(ui_ble_evt_if_cbs && ui_ble_evt_if_cbs->walkie_talkie_receive_stream_state_cb)
            {
                ui_ble_evt_if_cbs->walkie_talkie_receive_stream_state_cb(result_info->ble_walkie_talkie_recv_stream_evt_info.device_id,
                                   result_info->ble_walkie_talkie_recv_stream_evt_info.state);
            }
        break;

        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_DATA_EVT:
            if(walkie_talkie_cb && walkie_talkie_cb->ble_client_walkie_talkie_playback_cb)
            {
                walkie_talkie_cb->ble_client_walkie_talkie_playback_cb(result_info->ble_walkie_talkie_recv_data_evt_info.device_id);
            }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_NABOR_CHANGED_EVT:
            TRACE(2, "%s nebor count %d", __func__, result_info->ble_walkie_talkie_nabor_changed_evt_info.nebor_count);
            if(ui_ble_evt_if_cbs && ui_ble_evt_if_cbs->walkie_talkie_nebor_changed_cb)
            {
                ui_ble_evt_if_cbs->walkie_talkie_nebor_changed_cb(result_info->ble_walkie_talkie_nabor_changed_evt_info.nebor_count);
            }
        break;
        default:
            break;
    }
    return 0;
}

int32_t app_bth_ble_if_client_init(void)
{
    app_bth_if_thread_client_result_register(APP_BTH_IF_USER_BLE, _btif_client_ble_result_rsp_callback);
    return 0;
}

int32_t app_bth_ble_if_client_open(void* ui_callback, uint8_t comm_type)
{
    TRACE(2, "[%s] callback_if_ptr=0x%p", __func__, ui_callback);
    ui_ble_evt_if_cbs = ui_callback;
    bth_if_thread_client_open(APP_BTH_IF_USER_BLE, (uint32_t)ui_callback, comm_type);
    return 0;
}

int32_t app_bth_ble_if_client_close(void)
{
    bth_if_thread_client_close(APP_BTH_IF_USER_BLE);
    ui_ble_evt_if_cbs = NULL;
    return 0;
}

int32_t app_bth_ble_if_client_reset(void)
{
    bth_if_thread_client_reset(APP_BTH_IF_USER_BLE);
    return 0;
}

int32_t app_bth_ble_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len)
{
    bth_if_thread_client_ctrl(APP_BTH_IF_USER_BLE, ctrl_cmd, payload, payload_len);
    return 0;
}

