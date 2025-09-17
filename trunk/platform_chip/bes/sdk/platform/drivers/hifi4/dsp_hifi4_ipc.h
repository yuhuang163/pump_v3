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
#ifndef __DSP_HIFI4_IPC_H__
#define __DSP_HIFI4_IPC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_rmt_ipc.h"

int dsp_hifi4_ipc_open(HAL_RMT_IPC_RX_IRQ_HANDLER rxhandler, HAL_RMT_IPC_TX_IRQ_HANDLER txhandler);

int dsp_hifi4_ipc_close(void);

int dsp_hifi4_ipc_opened(void);

int dsp_hifi4_send_seq(const void *data, unsigned int len, unsigned int *seq);

int dsp_hifi4_send(const void *data, unsigned int len);

int dsp_hifi4_tx_active(unsigned int seq);

int dsp_hifi4_tx_irq_run(void);

#ifdef __cplusplus
}
#endif

#endif
