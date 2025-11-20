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
#include "bth_a2dp_sink.h"
#include "hal_sysfreq.h"
#include "kfifo.h"

static const btav_source_callbacks_t* ui_a2dp_src_evt_if_cbs = NULL;
static a2dp_sink_send_msg_callback a2dp_sink_msg_send_cb = NULL ;

static struct kfifo *p_a2dp_sink_packet_fifo;

static int32_t _btif_client_a2dp_sink_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    app_bt_a2dp_sink_param_t *result_info = (app_bt_a2dp_sink_param_t*)buf;
    a2dp_sink_msg_t msg;
    TRACE(2, "[%s] sub_cmd=%d", __func__, sub_cmd);

    memcpy((void*)&msg.param, (void*)result_info, sizeof(app_bt_a2dp_sink_param_t));
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_START: {
            p_a2dp_sink_packet_fifo = (struct kfifo *)result_info->onoff_param.kfifo_addr;
            if(a2dp_sink_msg_send_cb!=NULL){
                msg.msg_type = A2DP_SINK_MSG_START;
                a2dp_sink_msg_send_cb(&msg);
            }
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_VOL_SET: {
            if(a2dp_sink_msg_send_cb!=NULL){
                msg.msg_type = A2DP_SINK_MSG_VOL_SET;
                a2dp_sink_msg_send_cb(&msg);
            }
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_STOP: {
            if(a2dp_sink_msg_send_cb!=NULL){
                msg.msg_type = A2DP_SINK_MSG_STOP;
                a2dp_sink_msg_send_cb(&msg);
            }
        }
        break;
        default:
        break;
    }
    return 0;
}

int32_t app_bth_a2dp_sink_if_client_init(void)
{
    app_bth_if_thread_client_result_register(APP_BTH_IF_USER_A2DP_SINK, _btif_client_a2dp_sink_result_rsp_callback);
    return 0;
}

int32_t app_bth_a2dp_sink_if_client_open(void* ui_callback, uint8_t comm_type)
{
    TRACE(2, "[%s] callback_if_ptr=0x%p", __func__, ui_callback);
    ui_a2dp_src_evt_if_cbs = ui_callback;
    bth_if_thread_client_open(APP_BTH_IF_USER_A2DP_SINK, (uint32_t)ui_callback, comm_type);
    return 0;
}

int32_t app_bth_a2dp_sink_if_client_close(void)
{
    bth_if_thread_client_close(APP_BTH_IF_USER_A2DP_SINK);
    return 0;
}

int32_t app_bth_a2dp_sink_if_client_reset(void)
{
    bth_if_thread_client_reset(APP_BTH_IF_USER_A2DP_SINK);
    return 0;
}

int32_t app_bth_a2dp_sink_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len)
{
    bth_if_thread_client_ctrl(APP_BTH_IF_USER_A2DP_SINK, ctrl_cmd, payload, payload_len);
    return 0;
}

void app_bth_a2dp_sink_msg_send_cb_register(a2dp_sink_send_msg_callback func)
{
    a2dp_sink_msg_send_cb = func;
}

int a2dp_sink_get_packet_data(uint8_t *pdata, uint32_t len)
{
    return kfifo_get(p_a2dp_sink_packet_fifo, pdata, len);
}

uint32_t a2dp_sink_get_fifo_len(void)
{
    return kfifo_len(p_a2dp_sink_packet_fifo);
}