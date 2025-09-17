/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#if !(defined(CHIP_ROLE_CP) || defined(CHIP_SUBSYS_SENS) || (defined(CHIP_SUBSYS_BTH) ^ defined(BTH_AS_MAIN_MCU)))
#ifdef DSP_HIFI4_TRC_TO_MCU

#include "plat_types.h"
#include "rx_dsp_hifi4_trc.h"
#include "hal_rmt_ipc.h"
#include "hal_trace.h"
#include "rmt_trace_server.h"
#include "string.h"

#if !defined(RMT_TRC_IN_MSG_CHAN) && defined(BTH_AS_MAIN_MCU) && (defined(CHIP_BEST1600) || defined(CHIP_BEST1603))
#error "RMT_TRC_IN_MSG_CHAN should be used with DSP_HIFI4_TRC_TO_MCU when BTH_AS_MAIN_MCU=1"
#endif

#ifdef RMT_TRC_IN_MSG_CHAN
#define RMT_TRC_CHAN_ID                     HAL_RMT_IPC_CHAN_0
#else
#define RMT_TRC_CHAN_ID                     HAL_RMT_IPC_CHAN_1
#endif

static struct RMT_TRC_SERVER_CFG_T server_cfg;
static const char *trc_name = "DSP_HIFI4";
static HAL_RMT_IPC_EP_T rmt_ipc_ep;

static int dsp_hifi4_trace_send(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

static void dsp_hifi4_trace_server_dump(void)
{
    rmt_trace_server_dump(&server_cfg);
}

unsigned int dsp_hifi4_trace_rx_handler(const void *data, unsigned int len)
{
    return rmt_trace_server_msg_handler(&server_cfg, data, len);
}

void dsp_hifi4_trace_server_open(void)
{
    int ret;

    memset(&server_cfg, 0, sizeof(server_cfg));
    server_cfg.name = trc_name;
    server_cfg.first_msg = true;
    server_cfg.send_cb = dsp_hifi4_trace_send;
#ifdef RMT_TRC_IN_MSG_CHAN
    server_cfg.in_msg_chan = true;

    ret = hal_rmt_ipc_get_ep(HAL_RMT_IPC_CORE_SYS_HIFIC0, RMT_TRC_CHAN_ID, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_get_ep failed: %d", ret);
#else
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SYS_HIFIC0, RMT_TRC_CHAN_ID, dsp_hifi4_trace_rx_handler, NULL, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);
#endif

    rmt_trace_server_dump_register(dsp_hifi4_trace_server_dump);

    TRACE(0, "Start to rx dsp_hifi4 trace");
}

void dsp_hifi4_trace_server_close(void)
{
    rmt_trace_server_dump_deregister(dsp_hifi4_trace_server_dump);
#ifndef RMT_TRC_IN_MSG_CHAN
    hal_rmt_ipc_stop_recv(rmt_ipc_ep);
    hal_rmt_ipc_close(rmt_ipc_ep);
#endif

    TRACE(0, "Stop rx dsp_hifi4 trace");
}

#endif
#endif
