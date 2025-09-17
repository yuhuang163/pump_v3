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
#ifdef SENS_TRC_TO_MCU

#include "plat_types.h"
#include "hal_rmt_ipc.h"
#include "hal_trace.h"
#include "rmt_trace_client.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

static int sens_trc_send(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

static int sens_trc_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}

static void sens_trc_tx_irq_run(void)
{
    hal_rmt_ipc_tx_irq_run(rmt_ipc_ep);
}

#ifdef USE_TRACE_ID
static const char sens_trc_tag[] = {USER_ID_CPU_SENS0,'0',};
#else
static const char sens_trc_tag[] = "SENS/";
#endif

static const struct RMT_TRC_CLIENT_CFG_T sens_cfg = {
    .tag = sens_trc_tag,
    .send_cb = sens_trc_send,
    .tx_active_cb = sens_trc_tx_active,
    .tx_irq_run_cb = sens_trc_tx_irq_run,
};

void sens_trace_to_mcu(void)
{
    int ret;

    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_MCU, HAL_RMT_IPC_CHAN_1, rmt_trace_client_msg_handler, NULL, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

    ret = rmt_trace_client_open(&sens_cfg);
    ASSERT(ret == 0, "rmt_trace_client_open failed: %d", ret);
}

#endif
