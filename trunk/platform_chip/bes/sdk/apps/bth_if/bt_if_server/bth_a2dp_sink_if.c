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
#include "bth_if_thread_server.h"
#include "bluetooth_bt_api.h"
#include "bth_if_cmd_def.h"
#include "bth_if_evt_def.h"
#include "cmsis_os.h"
//#include "app_bt_stream.h"
#include "kfifo.h"

//TODO: use BTH Adapter APIs to replace it.
WEAK uint8_t bt_get_sbc_sample_rate(void)
{
    return 0;
}

WEAK void rmt_a2dp_sink_dev_onoff_callback_register(int (*onoff_cb)(bool))
{

}

WEAK void rmt_a2dp_sink_dev_data_ind_callback_register(int (*data_ind_cb)(uint8_t *buffer, uint32_t size))
{

}

WEAK void rmt_a2dp_sink_dev_vol_set_callback_register(int (*vol_cb)(uint8_t vol))
{

}

static struct kfifo a2dp_sink_packet_fifo;
#define A2DP_PACKET_BUFFER_SIZE (16*1024)
static uint8_t packet_data_cache[A2DP_PACKET_BUFFER_SIZE];

POSSIBLY_UNUSED static int btif_a2dp_sink_evt_customif_cb(uint8_t  event, const uint32_t *param)
{
    TRACE(2, "[%s] event=%d", __func__, event);

    return 0;
}

uint32_t app_bth_a2dp_sink_if_enable(void *callback_func_if, uint8_t comm_type)
{
    return 0;
}

static int app_bth_nontify_rmt_device_a2dp_sink_onoff(bool onoff)
{
    app_bt_a2dp_sink_param_t param;
    param.onoff_param.onoff = onoff;

    if(onoff)
    {
        kfifo_init(&a2dp_sink_packet_fifo, packet_data_cache, A2DP_PACKET_BUFFER_SIZE);
        //kfifo_put(&a2dp_sink_packet_fifo, "sbc", 4);
        param.onoff_param.codec_type = bes_aud_bt->aud_get_a2dp_codec_type(bes_aud_bt->aud_get_curr_a2dp_device());
        param.onoff_param.sample_rate = (uint32_t)bt_get_sbc_sample_rate();     //bt_export_get_a2dp_sample_rate
        param.onoff_param.kfifo_addr  = (uint32_t)(&a2dp_sink_packet_fifo);
        bth_if_send_result_to_app(APP_BTH_IF_USER_A2DP_SINK, (uint8_t*)(&param), sizeof(app_bt_a2dp_sink_param_t), BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_START);
    }
    else
    {
        bth_if_send_result_to_app(APP_BTH_IF_USER_A2DP_SINK, (uint8_t*)(&param), sizeof(app_bt_a2dp_sink_param_t), BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_STOP);
    }
    return 0;
}

static int app_bth_notify_rmt_device_a2dp_data_ind(uint8_t *buf, uint32_t len)
{
    if(kfifo_get_free_space(&a2dp_sink_packet_fifo)>=len)
    {
        kfifo_put(&a2dp_sink_packet_fifo, buf, len);
    }
    else
    {
        TRACE(0, "%s kfifo overflow!!!", __func__);
        return -1;
    }
    return 0;
}

static int app_bth_notify_rmt_device_a2dp_volume_set(uint8_t vol)
{
    app_bt_a2dp_sink_param_t param;
    param.vol_set.volume_level = vol;
    bth_if_send_result_to_app(APP_BTH_IF_USER_A2DP_SINK, (uint8_t*)(&param), sizeof(app_bt_a2dp_sink_param_t), BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_VOL_SET);
    return 0;
}

uint32_t app_bth_a2dp_sink_if_disable(void)
{
    return 0;
}

uint32_t app_bth_a2dp_sink_if_restart(void)
{
    return 0;
}

uint32_t app_bth_a2dp_sink_if_ctrl(uint32_t sub_cmd, uint8_t *ptr, uint32_t ptr_len)
{

    switch(sub_cmd)
    {
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_START:

        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_STOP:

        break;
        default:
        break;
    }

    return 0;
}

static const app_bth_if_ipc_msg_t bth_a2dp_sink_btif_ipc_handle_table = {
    .open           = app_bth_a2dp_sink_if_enable,
    .close          = app_bth_a2dp_sink_if_disable,
    .restart        = app_bth_a2dp_sink_if_restart,
    .ctrl           = app_bth_a2dp_sink_if_ctrl,
};

void app_bth_a2dp_sink_if_init(void)
{
    bth_if_thread_server_register(APP_BTH_IF_USER_A2DP_SINK, &bth_a2dp_sink_btif_ipc_handle_table);
    rmt_a2dp_sink_dev_onoff_callback_register(app_bth_nontify_rmt_device_a2dp_sink_onoff);
    rmt_a2dp_sink_dev_data_ind_callback_register(app_bth_notify_rmt_device_a2dp_data_ind);
    rmt_a2dp_sink_dev_vol_set_callback_register(app_bth_notify_rmt_device_a2dp_volume_set);
}