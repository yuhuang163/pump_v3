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
#if defined(TZ_TRIGGER) && (defined(ARM_CMSE) || defined(ARM_CMNS))

#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_tz_trigger.h"
#include CHIP_SPECIFIC_HDR(hal_tz_trigger)

extern uint32_t int_lock_smp_com(void);
extern void int_unlock_smp_com(uint32_t flags);

static const IRQn_Type rx_irq_id = TZ_TRIGGER_RX_IRQ;

static HAL_TZ_TRIGGER_HANDLER rx_irq_handler;

static bool chan_opened;

static void hal_tz_trigger_rx_irq(void)
{
    uint32_t lock = int_lock();
    //TRACE(0,"hal_tz_trigger_rx_irq id=%d,act=%d,rx_irq_handler=%p",id,NVIC_GetActive(rx_irq_id[id]),rx_irq_handler);
    if (NVIC_GetActive(rx_irq_id)) {
        hal_mcu2cp_local_irq_clear();
        int_unlock(lock);
        rx_irq_handler(NULL);
        lock = int_lock();
    }
    int_unlock(lock);
}

int hal_tz_trigger_open(HAL_TZ_TRIGGER_HANDLER rxhandler)
{
#if defined(ARM_CMSE)
    /* wait mcu init send msg addr */
    //while (hal_mcu2cp_get_send_msg_list_mcu() == NULL)
    //    osDelay(100);
    //osDelay(1000);
#endif
    uint32_t lock = int_lock();
    if (chan_opened) {
        int_unlock(lock);
        return 3;
    } else {
        hal_mcu2cp_local_irq_clear();
        NVIC_SetVector(rx_irq_id, (uint32_t)hal_tz_trigger_rx_irq);
        NVIC_SetPriority(rx_irq_id, IRQ_PRIORITY_NORMAL);
        NVIC_DisableIRQ(rx_irq_id);
    }
    chan_opened = true;
    rx_irq_handler = rxhandler;
    NVIC_EnableIRQ(rx_irq_id);
    int_unlock(lock);
    return 0;
}

int hal_tz_trigger_close(void)
{
    uint32_t lock = int_lock();

    chan_opened = false;
    rx_irq_handler = NULL;
    NVIC_DisableIRQ(rx_irq_id);
    int_unlock(lock);
    return 0;
}

int hal_tz_trigger_set(void)
{
     if (!chan_opened) {
        return 3;
    }
    //TRACE(0,"hal_tz_trigger_set id=%d",id);
    uint32_t lock = int_lock();
    hal_mcu2cp_peer_irq_set();
    int_unlock(lock);
    return 0;
}

#endif
