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
#ifdef BTH_TRC_TO_SYS

#include "plat_types.h"
#include "hal_rmt_ipc.h"
#include "hal_trace.h"
#include "rmt_trace_client.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif

#if defined(CHIP_BEST1600) || defined(CHIP_BEST1603)
#ifndef RMT_TRC_IN_MSG_CHAN
#error "RMT_TRC_IN_MSG_CHAN should be used with BTH_TRC_TO_SYS"
#endif
#endif

#ifdef RMT_TRC_IN_MSG_CHAN
#define RMT_TRC_CHAN_ID                     HAL_RMT_IPC_CHAN_0
#else
#define RMT_TRC_CHAN_ID                     HAL_RMT_IPC_CHAN_1
#endif

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

static int bth_trc_send(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

static int bth_trc_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}

static void bth_trc_tx_irq_run(void)
{
    hal_rmt_ipc_tx_irq_run(rmt_ipc_ep);
}

#ifdef USE_TRACE_ID
static const char bth_trc_tag[] = {USER_ID_CPU_BTH0,'0',};
#else
static const char bth_trc_tag[] = "BTH /";
#endif

static const struct RMT_TRC_CLIENT_CFG_T bth_cfg = {
    .tag = bth_trc_tag,
    .send_cb = bth_trc_send,
    .tx_active_cb = bth_trc_tx_active,
    .tx_irq_run_cb = bth_trc_tx_irq_run,
};

void bth_trace_to_sys(void)
{
    int ret;

#ifdef RMT_TRC_IN_MSG_CHAN
    ret = hal_rmt_ipc_get_ep(HAL_RMT_IPC_CORE_SYS_M55C0, RMT_TRC_CHAN_ID, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_get_ep failed: %d", ret);
#else
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SYS_M55C0, RMT_TRC_CHAN_ID, rmt_trace_client_msg_handler, NULL, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);
#endif

    ret = rmt_trace_client_open(&bth_cfg);
    ASSERT(ret == 0, "rmt_trace_client_open failed: %d", ret);
}

#endif
