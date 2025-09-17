/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

#include "plat_types.h"
#include "dsp_hifi4_ipc.h"
#include "hal_trace.h"
#if defined(DSP_HIFI4_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
#include "rx_dsp_hifi4_trc.h"
#endif

#define IPC_CHAN_ID                         HAL_RMT_IPC_CHAN_0

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

#if defined(DSP_HIFI4_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
static HAL_RMT_IPC_RX_IRQ_HANDLER app_rx_hdlr;

static unsigned int general_rx_handler(const void *data, unsigned int len)
{
    if (dsp_hifi4_trace_rx_handler(data, len)) {
        return len;
    }

    if (app_rx_hdlr) {
        return app_rx_hdlr(data, len);
    }

    return len;
}
#endif

int dsp_hifi4_ipc_open(HAL_RMT_IPC_RX_IRQ_HANDLER rxhandler, HAL_RMT_IPC_TX_IRQ_HANDLER txhandler)
{
    int ret;

#if defined(DSP_HIFI4_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
    app_rx_hdlr = rxhandler;
    rxhandler = general_rx_handler;
#endif
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SYS_HIFIC0, IPC_CHAN_ID, rxhandler, txhandler, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

    return 0;
}

int dsp_hifi4_ipc_close(void)
{
    hal_rmt_ipc_stop_recv(rmt_ipc_ep);
    hal_rmt_ipc_close(rmt_ipc_ep);
    return 0;
}

int dsp_hifi4_ipc_opened(void)
{
    return hal_rmt_ipc_opened(rmt_ipc_ep);
}

int dsp_hifi4_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int dsp_hifi4_send(const void *data, unsigned int len)
{
    return hal_rmt_ipc_send(rmt_ipc_ep, data, len);
}

int dsp_hifi4_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}

int dsp_hifi4_tx_irq_run(void)
{
    return hal_rmt_ipc_tx_irq_run(rmt_ipc_ep);
}

#endif
