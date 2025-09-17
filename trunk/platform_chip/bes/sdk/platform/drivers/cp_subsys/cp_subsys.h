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
#ifndef __CP_SUBSYS_H__
#define __CP_SUBSYS_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int (*CP_SUBSYS_RX_IRQ_HANDLER)(const void *data, unsigned int len);

typedef void (*CP_SUBSYS_TX_IRQ_HANDLER)(const void *data, unsigned int len);

int cp_subsys_open(CP_SUBSYS_RX_IRQ_HANDLER rx_hdlr, CP_SUBSYS_TX_IRQ_HANDLER tx_hdlr);

int cp_subsys_opened(void);

int cp_subsys_close(void);

int cp_subsys_send_seq(const void *data, unsigned int len, unsigned int *seq);

int cp_subsys_send(const void *data, unsigned int len);

int cp_subsys_tx_active(unsigned int seq);

int cp_subsys_msg_send(unsigned short id, unsigned short reply, const void *data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif

