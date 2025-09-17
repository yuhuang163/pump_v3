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
#ifdef DSP_M55_TRC_TO_MCU

#include "plat_types.h"
#include "hal_rmt_ipc.h"
#include "hal_trace.h"
#include "rmt_trace_client.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif

#if !defined(RMT_TRC_IN_MSG_CHAN) && (defined(CHIP_BEST1600) || defined(CHIP_BEST1603))
#error "RMT_TRC_IN_MSG_CHAN should be used with DSP_M55_TRC_TO_MCU"
#endif

#ifdef RMT_TRC_IN_MSG_CHAN
#define RMT_TRC_CHAN_ID                     HAL_RMT_IPC_CHAN_0
#else
#define RMT_TRC_CHAN_ID                     HAL_RMT_IPC_CHAN_1
#endif

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

static int dsp_m55_trc_send(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

static int dsp_m55_trc_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}

static void dsp_m55_trc_tx_irq_run(void)
{
    hal_rmt_ipc_tx_irq_run(rmt_ipc_ep);
}

#ifdef USE_TRACE_ID
static const char dsp_m55_trc_tag[] = {USER_ID_CPU_DSPM,'0',};
#else
static const char dsp_m55_trc_tag[] = "DSPM/";
#endif

static const struct RMT_TRC_CLIENT_CFG_T dsp_m55_cfg = {
    .tag = dsp_m55_trc_tag,
    .send_cb = dsp_m55_trc_send,
    .tx_active_cb = dsp_m55_trc_tx_active,
    .tx_irq_run_cb = dsp_m55_trc_tx_irq_run,
};

void dsp_m55_trace_to_mcu(void)
{
    int ret;

#ifdef RMT_TRC_IN_MSG_CHAN
    ret = hal_rmt_ipc_get_ep(HAL_RMT_IPC_CORE_BTH_C0, RMT_TRC_CHAN_ID, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_get_ep failed: %d", ret);
#else
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_BTH_C0, RMT_TRC_CHAN_ID, rmt_trace_client_msg_handler, NULL, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);
#endif

    ret = rmt_trace_client_open(&dsp_m55_cfg);
    ASSERT(ret == 0, "rmt_trace_client_open failed: %d", ret);
}

#endif
