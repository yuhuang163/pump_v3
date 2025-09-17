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
#ifdef CP_SUBSYS_TRC_TO_MCU

#include "plat_types.h"
#include "hal_rmt_ipc.h"
#include "hal_trace.h"
#include "rmt_trace_client.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif
#include "hwtimer_list.h"

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

static int cp_subsys_trc_send(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

static void cp_subsys_trc_tx_irq_run(void)
{
    hal_rmt_ipc_tx_irq_run(rmt_ipc_ep);
}

#ifdef USE_TRACE_ID
static const char cp_subsys_trc_tag[] = {TAG_VAL_CPU_MCU0,'0',};
#else
static const char cp_subsys_trc_tag[] = "CP_S/";
#endif

static const struct RMT_TRC_CLIENT_CFG_T cp_subsys_cfg = {
    .tag = cp_subsys_trc_tag,
    .send_cb = cp_subsys_trc_send,
    .tx_irq_run_cb = cp_subsys_trc_tx_irq_run,
};

void cp_subsys_trace_to_mcu(void)
{
    int ret;

    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_CORE0, HAL_RMT_IPC_CHAN_1, rmt_trace_client_rx_handler, rmt_trace_client_tx_handler, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

    ret = rmt_trace_client_open(&cp_subsys_cfg);
    ASSERT(ret == 0, "rmt_trace_client_open failed: %d", ret);
}

#endif

