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
#ifndef __CP_SUBSYS_CORE_H__
#define __CP_SUBSYS_CORE_H__

enum CORE_IRQ_HDLR_ID_T {
    CORE_IRQ_HDLR_ID_0,
    CORE_IRQ_HDLR_ID_1,

    CORE_IRQ_HDLR_ID_QTY
};

#define CORE_IRQ_HDLR_PRIO_HIGH CORE_IRQ_HDLR_ID_0
#define CORE_IRQ_HDLR_PRIO_LOW  CORE_IRQ_HDLR_ID_1

typedef unsigned int (*cp_subsys_core_rx_irq_handler_t)(const void*, unsigned int);
typedef void (*cp_subsys_core_tx_done_irq_handler_t)(const void*, unsigned int);
typedef void (*cp_subsys_core_set_sys_freq_cb)(unsigned int);

void cp_subsys_core_register_rx_irq_handler(enum CORE_IRQ_HDLR_ID_T id, cp_subsys_core_rx_irq_handler_t irqHandler);
void cp_subsys_core_register_tx_done_irq_handler(enum CORE_IRQ_HDLR_ID_T id, cp_subsys_core_tx_done_irq_handler_t irqHandler);
void cp_subsys_core_register_sys_freq_cb(cp_subsys_core_set_sys_freq_cb cb);

#endif

