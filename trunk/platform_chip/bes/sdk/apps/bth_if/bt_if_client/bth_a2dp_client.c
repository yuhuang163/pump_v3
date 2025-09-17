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
#ifdef __ALSA_EN__
#include "audio_sink.h"
#endif

static const btav_source_callbacks_t* ui_a2dp_src_evt_if_cbs = NULL;

static const char* a2dp_ret2str(uint32_t sub_cmd)
{
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_A2DP_CONNECT_EVT:
            return "A2DP_CONN_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_A2DP_DISCONN_EVT:
            return "A2DP_DISC_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_A2DP_STREAM_STATUS_CHANGE_EVT:
            return "A2DP_STREAM_CHANGE_EVT";
            break;
        default:
            break;
    }
    return "UNKNOW";
}

static void a2dp_audio_stream_state_process(bt_rawaddr_t* remote_addr, uint8_t enable)
{
    if (enable) {
#ifdef __ALSA_EN__
        audio_sink_global_stream_state_set(true);
#endif

        if (ui_a2dp_src_evt_if_cbs && ui_a2dp_src_evt_if_cbs->audio_state_cb)
            ui_a2dp_src_evt_if_cbs->audio_state_cb(remote_addr, BTAV_AUDIO_STATE_STARTED);
    } else {
        if (ui_a2dp_src_evt_if_cbs && ui_a2dp_src_evt_if_cbs->audio_state_cb)
            ui_a2dp_src_evt_if_cbs->audio_state_cb(remote_addr, BTAV_AUDIO_STATE_REMOTE_SUSPEND);

#ifdef __ALSA_EN__
        audio_sink_global_stream_state_set(false);
#endif
    }
}

static int32_t _btif_client_a2dp_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    app_bth_if_payload_param_info_t *result_info = (app_bth_if_payload_param_info_t*)buf;
    TRACE(2, "[%s] sub_cmd=%s", __func__, a2dp_ret2str(sub_cmd));

    if (!result_info) {
        TRACE(1, "[%s] ERROR!!!", __func__);
        return -1;
    }
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_A2DP_CONNECT_EVT: {
            if (result_info->a2dp_conn_state_evt_info.error_code == 0x00) {
#ifdef __ALSA_EN__
                audio_sink_global_sink_type_set(AUDIO_SINK_A2DP_SOURCE);
#endif
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_A2DP_DISCONN_EVT: {
            app_bt_a2dp_disc_state_evt_t* a2dp_disc_state =
                                    (app_bt_a2dp_disc_state_evt_t*)&result_info->a2dp_disc_state_evt_info;
            a2dp_audio_stream_state_process((bt_rawaddr_t*)&a2dp_disc_state->bt_addr, false);
#ifdef __ALSA_EN__
            audio_sink_global_sink_type_set(AUDIO_SINK_ALSA);
#endif
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_A2DP_STREAM_STATUS_CHANGE_EVT: {
            app_bt_a2dp_stream_state_evt_t* a2dp_stream_state =
                                    (app_bt_a2dp_stream_state_evt_t*)&result_info->a2dp_stream_state_evt_info;
            if (a2dp_stream_state->onoff) {
                a2dp_audio_stream_state_process((bt_rawaddr_t*)&a2dp_stream_state->bt_addr, true);
            } else {
                a2dp_audio_stream_state_process((bt_rawaddr_t*)&a2dp_stream_state->bt_addr, false);
            }
        }
        break;
    }
    return 0;
}

int32_t app_bth_a2dp_if_client_init(void)
{
    app_bth_if_thread_client_result_register(APP_BTH_IF_USER_A2DP, _btif_client_a2dp_result_rsp_callback);
    return 0;
}

int32_t app_bth_a2dp_if_client_open(void* ui_callback, uint8_t comm_type)
{
    TRACE(2, "[%s] callback_if_ptr=0x%p", __func__, ui_callback);
    ui_a2dp_src_evt_if_cbs = ui_callback;
    bth_if_thread_client_open(APP_BTH_IF_USER_A2DP, (uint32_t)ui_callback, comm_type);
    return 0;
}

int32_t app_bth_a2dp_if_client_close(void)
{
    bth_if_thread_client_close(APP_BTH_IF_USER_A2DP);
    return 0;
}

int32_t app_bth_a2dp_if_client_reset(void)
{
    bth_if_thread_client_reset(APP_BTH_IF_USER_A2DP);
    return 0;
}

int32_t app_bth_a2dp_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len)
{
    bth_if_thread_client_ctrl(APP_BTH_IF_USER_A2DP, ctrl_cmd, payload, payload_len);
    return 0;
}