/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "hal_transq.h"

#if defined(TRANSQ0_BASE) || defined(TRANSQ1_BASE)

#include "reg_transq.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "stdbool.h"
#include "cmsis_nvic.h"

#ifndef TRANSQ0_SLOT_NUM
#define TRANSQ0_SLOT_NUM                                32
#endif

#ifdef TRANSQ1_BASE
#ifndef TRANSQ1_SLOT_NUM
#define TRANSQ1_SLOT_NUM                                32
#endif
#endif

#define U64_STR                                         "0x%08X%08X"
#define U64_2_STR(a)                                    (uint32_t)(a >> 32), (uint32_t)a

void WEAK transq_lock_init(void)
{
    return;
}
uint32_t WEAK transq_lock(void)
{
    return int_lock();
}
void WEAK transq_unlock(uint32_t cpsr)
{
    int_unlock(cpsr);
}

// BITMAP:
// [High Priority Slots] ...... [Normal Priority Slots]
// 31 30 29 28 27 26 25  ...... 10 9 8 7 6 5 4 3 2 1 0

static struct TRANSQ_T * const transq[HAL_TRANSQ_ID_QTY] = {
    (struct TRANSQ_T *)TRANSQ0_BASE,
#ifdef TRANSQ1_BASE
    (struct TRANSQ_T *)TRANSQ1_BASE,
#endif
};

static struct TRANSQ_T * const peer_transq[HAL_TRANSQ_ID_QTY] = {
    (struct TRANSQ_T *)TRANSQ0_PEER_BASE,
#ifdef TRANSQ1_BASE
    (struct TRANSQ_T *)TRANSQ1_PEER_BASE,
#endif
};

static const enum HAL_CMU_MOD_ID_T transq_mod[HAL_TRANSQ_ID_QTY] = {
    HAL_CMU_MOD_P_TRANSQ0,
#ifdef TRANSQ1_BASE
    HAL_CMU_MOD_P_TRANSQ1,
#endif
};

static const IRQn_Type remote_irq_num[HAL_TRANSQ_ID_QTY] = {
    TRANSQ0_RMT_IRQn,
#ifdef TRANSQ1_BASE
    TRANSQ1_RMT_IRQn,
#endif
};

static const IRQn_Type local_irq_num[HAL_TRANSQ_ID_QTY] = {
    TRANSQ0_LCL_IRQn,
#ifdef TRANSQ1_BASE
    TRANSQ1_LCL_IRQn,
#endif
};

uint8_t transq_slot_num[HAL_TRANSQ_ID_QTY] = {
    TRANSQ0_SLOT_NUM,
#ifdef TRANSQ1_BASE
    TRANSQ1_SLOT_NUM,
#endif
};

static struct HAL_TRANSQ_CFG_T transq_cfg[HAL_TRANSQ_ID_QTY];

static uint8_t next_tx_slot[HAL_TRANSQ_ID_QTY][HAL_TRANSQ_PRI_QTY];

static uint8_t active_tx_slot[HAL_TRANSQ_ID_QTY][HAL_TRANSQ_PRI_QTY];

static bool tx_slot_full[HAL_TRANSQ_ID_QTY][HAL_TRANSQ_PRI_QTY];

static uint8_t next_rx_slot[HAL_TRANSQ_ID_QTY][HAL_TRANSQ_PRI_QTY];

static uint64_t rx_irq_mask[HAL_TRANSQ_ID_QTY][HAL_TRANSQ_PRI_QTY];

static uint64_t tx_irq_mask[HAL_TRANSQ_ID_QTY][HAL_TRANSQ_PRI_QTY];

static uint64_t construct_mask(enum HAL_TRANSQ_ID_T id, uint32_t lsb, uint32_t width)
{
    uint32_t i;
    uint64_t result;

    if (lsb >= transq_slot_num[id] || width == 0) {
        return 0;
    }

    result = 0;
    for (i = lsb; i < lsb + width; i++) {
        result |= ((uint64_t)1 << i);
    }

    return result;
}

static uint32_t get_next_rx_slot(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri, uint32_t slot)
{
    slot++;
    if (pri == HAL_TRANSQ_PRI_HIGH) {
        if (slot >= transq_slot_num[id]) {
            slot = transq_slot_num[id] - transq_cfg[id].slot.rx_num[pri];
        }
    } else {
        if (slot >= transq_cfg[id].slot.rx_num[pri]) {
            slot = 0;
        }
    }

    return slot;
}

static uint32_t get_next_tx_slot(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri, uint32_t slot)
{
    slot++;
    if (pri == HAL_TRANSQ_PRI_HIGH) {
        if (slot >= transq_slot_num[id]) {
            slot = transq_slot_num[id] - transq_cfg[id].slot.tx_num[pri];
        }
    } else {
        if (slot >= transq_cfg[id].slot.tx_num[pri]) {
            slot = 0;
        }
    }

    return slot;
}

#if defined(__ARM_ARCH_ISA_ARM) && defined(RTOS) && (!defined(KERNEL_LITEOS_A))
static void hal_transq_remote_irq_handler(int irq_num, void * irq_data)
{
    POSSIBLY_UNUSED IRQn_Type irq = (IRQn_Type)irq_num;
#else
static void hal_transq_remote_irq_handler(void)
{
    POSSIBLY_UNUSED IRQn_Type irq = NVIC_GetCurrentActiveIRQ();
#endif
    enum HAL_TRANSQ_ID_T id;
    enum HAL_TRANSQ_PRI_T pri = HAL_TRANSQ_PRI_NORMAL;
    uint64_t status;
    uint32_t mask;
    uint32_t slot;
    uint32_t lock;

#ifdef TRANSQ1_BASE
    for (id = HAL_TRANSQ_ID_0; id < HAL_TRANSQ_ID_QTY; id++) {
        if (irq == remote_irq_num[id]) {
            break;
        }
    }
    if (id >= HAL_TRANSQ_ID_QTY) {
        return;
    }
#else
    id = HAL_TRANSQ_ID_0;
#endif

    while (1) {
        status = 0;
        if (transq_slot_num[id] > 32) {
            status = peer_transq[id]->RMT_MIS_E;
            status = status << 32;
        }
        status |= peer_transq[id]->RMT_MIS;

        if (status == 0)
            break;

        lock = transq_lock();

        if (status & rx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]) {
            pri = HAL_TRANSQ_PRI_HIGH;
        } else if (status & rx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL]) {
            pri = HAL_TRANSQ_PRI_NORMAL;
        } else {
            ASSERT(false, "TRANSQ-%d: Corrupted rx mask: status=" U64_STR " rx_mask=" U64_STR " / " U64_STR "",
                id, U64_2_STR(status), U64_2_STR(rx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]), U64_2_STR(rx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL]));
        }
        slot = next_rx_slot[id][pri];
        ASSERT(slot <  transq_slot_num[id] && (status & ((uint64_t)1 << slot)),
            "TRANSQ-: Rx IRQ  status=" U64_STR " rx_mask=" U64_STR "slot=%d, irq_count:%d",
            U64_2_STR(status), U64_2_STR(rx_irq_mask[id][pri]), slot, transq_cfg[id].rx_irq_count);

        // Mask IRQ from corresponding slots
        mask = (uint32_t)rx_irq_mask[id][pri];
        peer_transq[id]->RMT_INTMASK &= ~mask;
        // Force to flush the write FIFO in the bus path
        peer_transq[id]->RMT_INTMASK;
        if (transq_slot_num[id] > 32) {
            mask = rx_irq_mask[id][pri] >> 32;
            peer_transq[id]->RMT_INTMASK_E &= ~mask;
            peer_transq[id]->RMT_INTMASK_E;
        }

        transq_unlock(lock);

        transq_cfg[id].rx_irq_count++;
        if (transq_cfg[id].rx_handler) {
            transq_cfg[id].rx_handler(pri);
        }
    }
}

static int hal_transq_active_tx_valid(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri)
{
    return ((active_tx_slot[id][pri] >= transq_slot_num[id]) ||
            (active_tx_slot[id][pri] != next_tx_slot[id][pri]) ||
            tx_slot_full[id][pri]);
}

#if defined(__ARM_ARCH_ISA_ARM) && defined(RTOS)
void hal_transq_local_irq_handler(int irq_num, void *irq_data)
{
    POSSIBLY_UNUSED IRQn_Type irq = (IRQn_Type)irq_num;
#else
static void hal_transq_local_irq_handler(void)
{
    POSSIBLY_UNUSED IRQn_Type irq = NVIC_GetCurrentActiveIRQ();
#endif
    enum HAL_TRANSQ_ID_T id;
    enum HAL_TRANSQ_PRI_T pri = HAL_TRANSQ_PRI_NORMAL;
    uint32_t status0 = 0;
    uint32_t status1 = 0;
    uint64_t status;
    uint32_t slot, next_slot;
    uint32_t lock;

#ifdef TRANSQ1_BASE
    for (id = HAL_TRANSQ_ID_0; id < HAL_TRANSQ_ID_QTY; id++) {
        if (irq == local_irq_num[id]) {
            break;
        }
    }
    if (id >= HAL_TRANSQ_ID_QTY) {
        return;
    }
#else
    id = HAL_TRANSQ_ID_0;
#endif

    status0 = transq[id]->LERR_MIS;
    if (transq_slot_num[id] > 32)
        status1 = transq[id]->LERR_MIS_E;

    if (status0)
        transq[id]->LERR_ISC.LERR_INTCLR = status0;
    if (status1)
        transq[id]->LERR_ISC_E.LERR_INTCLR = status1;

    if (status0 || status1) {
        ASSERT(false, "TRANSQ-%d: Tx on active slot: 0x%08x 0x%08x", id, status0, status1);
    }

    while (1) {
        status = 0;
        if (transq_slot_num[id] > 32) {
            status = transq[id]->LDONE_MIS_E;
            status = status << 32;
        }
        status |= transq[id]->LDONE_MIS;

        if (status == 0)
            break;

        if (transq_cfg[id].tx_handler) {
            lock = transq_lock();
            ASSERT(hal_transq_active_tx_valid(id, HAL_TRANSQ_PRI_HIGH), "TRANSQ-%d: Corrupted pri active tx: active=%d next=%d full=%d",
                id, active_tx_slot[id][HAL_TRANSQ_PRI_HIGH], next_tx_slot[id][HAL_TRANSQ_PRI_HIGH], tx_slot_full[id][HAL_TRANSQ_PRI_HIGH]);
            ASSERT(hal_transq_active_tx_valid(id, HAL_TRANSQ_PRI_NORMAL), "TRANSQ-%d: Corrupted active tx: active=%d next=%d full=%d",
                id, active_tx_slot[id][HAL_TRANSQ_PRI_NORMAL], next_tx_slot[id][HAL_TRANSQ_PRI_NORMAL], tx_slot_full[id][HAL_TRANSQ_PRI_NORMAL]);

            if (status & tx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]) {
                pri = HAL_TRANSQ_PRI_HIGH;
            } else if (status & tx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL]) {
                pri = HAL_TRANSQ_PRI_NORMAL;
            } else {
                ASSERT(false, "TRANSQ-%d: Corrupted tx mask: status=" U64_STR "tx_mask=" U64_STR " / " U64_STR "",
                    id, U64_2_STR(status), U64_2_STR(tx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]), U64_2_STR(tx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL]));
            }
            slot = active_tx_slot[id][pri];

            ASSERT(slot < transq_slot_num[id] && (status & ((uint64_t)1 << slot)),
                "TRANSQ-%d: Tx done IRQ when slot empty or out of order: status=" U64_STR " next=%d pri_next=%d",
                id, U64_2_STR(status), active_tx_slot[id][HAL_TRANSQ_PRI_NORMAL], active_tx_slot[id][HAL_TRANSQ_PRI_HIGH]);

            // Clear the interrupt
            if(slot < 32)
                transq[id]->LDONE_ISC.LDONE_INTCLR = (1 << slot);
            else
                transq[id]->LDONE_ISC_E.LDONE_INTCLR = (1 << (slot - 32));

            next_slot = get_next_tx_slot(id, pri, slot);

            if (!tx_slot_full[id][pri] && (next_slot == next_tx_slot[id][pri])) {
                // No tx in progress
                active_tx_slot[id][pri] = transq_slot_num[id];
            } else {
                tx_slot_full[id][pri] = false;
                if (transq_cfg[id].slot.tx_num[pri] == 1) {
                    // No tx in progress
                    active_tx_slot[id][pri] = transq_slot_num[id];
                } else {
                    // Some tx in progress
                    active_tx_slot[id][pri] = next_slot;
                }
            }
            transq_unlock(lock);
            transq_cfg[id].tx_handler(pri,
                (const uint8_t *)transq[id]->WSLOT[slot].ADDR,
                transq[id]->WSLOT[slot].LEN);
        } else {
            transq[id]->LDONE_INTMASK = 0;
            if (transq_slot_num[id] > 32)
                transq[id]->LDONE_INTMASK_E = 0;
        }
    }
}

#if defined(__ARM_ARCH_ISA_ARM) && defined(RTOS) && defined(KERNEL_LITEOS_A)
static void hal_transq_local_irq_handler_wrap(void)
{
    POSSIBLY_UNUSED IRQn_Type irq = NVIC_GetCurrentActiveIRQ();
    hal_transq_local_irq_handler(irq, NULL);
}
#endif

bool hal_transq_tx_busy(enum HAL_TRANSQ_ID_T id)
{
    bool ret;
    uint32_t lock;
    lock = int_lock();

    if ((active_tx_slot[id][HAL_TRANSQ_PRI_NORMAL] !=  transq_slot_num[id] ||
                active_tx_slot[id][HAL_TRANSQ_PRI_HIGH] !=  transq_slot_num[id])
            && (transq[id]->LDONE_MIS == 0 && (transq_slot_num[id] < 32 || transq[id]->LDONE_MIS_E == 0))) {
        ret = true;
    } else {
        ret = false;
    }

    int_unlock(lock);
    return ret;
}

enum HAL_TRANSQ_RET_T hal_transq_get_rx_status(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri, bool *ready)
{
    uint32_t lock;
    uint32_t slot;

    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }
    if (pri >= HAL_TRANSQ_PRI_QTY) {
        return HAL_TRANSQ_RET_BAD_PRI;
    }
    if (transq_cfg[id].slot.rx_num[pri] == 0) {
        return HAL_TRANSQ_RET_BAD_RX_NUM;
    }
    if (transq_cfg[id].rx_handler) {
        // Rx will be processed by IRQ handler
        return HAL_TRANSQ_RET_BAD_MODE;
    }

    lock = int_lock();

    slot = next_rx_slot[id][pri];

    if ((slot <  32) && (peer_transq[id]->RMT_ISC.RMT_RIS & (1 << slot))) {
        *ready = true;
    } else if (32 <= slot && slot < TRANSQ_SLOT_NUM_MAX && (peer_transq[id]->RMT_ISC_E.RMT_RIS & (1 << (slot - 32)))) {
        *ready = true;
    } else {
        *ready = false;
    }

    int_unlock(lock);

    return HAL_TRANSQ_RET_OK;
}

enum HAL_TRANSQ_RET_T hal_transq_get_tx_status(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri, bool *done)
{
    uint32_t lock;
    uint32_t slot, next_slot;

    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }
    if (pri >= HAL_TRANSQ_PRI_QTY) {
        return HAL_TRANSQ_RET_BAD_PRI;
    }
    if (transq_cfg[id].slot.tx_num[pri] == 0) {
        return HAL_TRANSQ_RET_BAD_TX_NUM;
    }
    if (transq_cfg[id].tx_handler) {
        // Tx done will be processed by IRQ handler
        return HAL_TRANSQ_RET_BAD_MODE;
    }

    lock = int_lock();

    slot = active_tx_slot[id][pri];

    ASSERT(hal_transq_active_tx_valid(id, pri), "TRANSQ-%d: Corrupted active tx: pri=%d active=%d next=%d full=%d",
        id, pri, active_tx_slot[id][pri], next_tx_slot[id][pri], tx_slot_full[id][pri]);

    if (slot < 32 && (transq[id]->LDONE_ISC.LDONE_RIS & (1 << slot))) {
        *done = true;

        // Clear the interrupt
        transq[id]->LDONE_ISC.LDONE_INTCLR = (1 << slot);
    } else if (32 <= slot && slot < TRANSQ_SLOT_NUM_MAX && (transq[id]->LDONE_ISC_E.LDONE_RIS & (1 << (slot - 32)))) {
        *done = true;

        // Clear the interrupt
        transq[id]->LDONE_ISC_E.LDONE_INTCLR = (1 << (slot - 32));
    } else {
        *done = false;
    }

    if (*done == true) {
        next_slot = get_next_tx_slot(id, pri, slot);

        if (!tx_slot_full[id][pri] && (next_slot == next_tx_slot[id][pri])) {
            // No tx in progress
            active_tx_slot[id][pri] = transq_slot_num[id];
        } else {
            tx_slot_full[id][pri] = false;
            if (transq_cfg[id].slot.tx_num[pri] == 1) {
                // No tx in progress
                active_tx_slot[id][pri] = transq_slot_num[id];
            } else {
                // Some tx in progress
                active_tx_slot[id][pri] = next_slot;
            }
        }
    }
    int_unlock(lock);

    return HAL_TRANSQ_RET_OK;
}

enum HAL_TRANSQ_RET_T hal_transq_rx_first(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri, const uint8_t **data, uint32_t *len)
{
    enum HAL_TRANSQ_RET_T ret;
    uint32_t slot;

    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }
    if (pri >= HAL_TRANSQ_PRI_QTY) {
        return HAL_TRANSQ_RET_BAD_PRI;
    }
    if (transq_cfg[id].slot.rx_num[pri] == 0) {
        return HAL_TRANSQ_RET_BAD_RX_NUM;
    }

    slot = next_rx_slot[id][pri];
    do {
        if (slot <  transq_slot_num[id]) {
            uint32_t rmt_ris;
            rmt_ris = (slot < 32) ? peer_transq[id]->RMT_ISC.RMT_RIS & (1 << slot) :
                            peer_transq[id]->RMT_ISC_E.RMT_RIS & (1 << (slot - 32));
            if (rmt_ris) {
                // Msg available
                ret = HAL_TRANSQ_RET_OK;
                if (data)
                    *data = (const uint8_t *)peer_transq[id]->RSLOT[slot].ADDR;
                if (len)
                    *len = peer_transq[id]->RSLOT[slot].LEN;
                break;
            }
        }

        // No msg. Re-enable IRQ
        ret = HAL_TRANSQ_RET_RX_EMPTY;
        if (data) {
            *data = NULL;
        }
        if (len) {
            *len = 0;
        }

        transq_cfg[id].rx_irq_count = 0;

        uint32_t mask;
        mask = (uint32_t)rx_irq_mask[id][pri];
        peer_transq[id]->RMT_INTMASK |= mask;
        if (transq_slot_num[id] > 32) {
            mask = rx_irq_mask[id][pri] >> 32;
            peer_transq[id]->RMT_INTMASK_E |= mask;
        }
    } while (0);

    return ret;
}

enum HAL_TRANSQ_RET_T hal_transq_rx_next(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri, const uint8_t **data, uint32_t *len)
{
    enum HAL_TRANSQ_RET_T ret;
    uint32_t slot;

    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }
    if (pri >= HAL_TRANSQ_PRI_QTY) {
        return HAL_TRANSQ_RET_BAD_PRI;
    }
    if (transq_cfg[id].slot.rx_num[pri] == 0) {
        return HAL_TRANSQ_RET_BAD_RX_NUM;
    }

    ret = HAL_TRANSQ_RET_OK;

    slot = next_rx_slot[id][pri];

    do {
        if (slot <  transq_slot_num[id]) {
            uint32_t rmt_ris;
            rmt_ris = (slot < 32) ? peer_transq[id]->RMT_ISC.RMT_RIS & (1 << slot) :
                            peer_transq[id]->RMT_ISC_E.RMT_RIS & (1 << (slot -32));
            if (rmt_ris) {
                // Clear cur IRQ
                if (slot < 32)
                    peer_transq[id]->RMT_ISC.RMT_INTCLR = (1 << slot);
                else
                    peer_transq[id]->RMT_ISC_E.RMT_INTCLR = (1 << (slot - 32));

                // Update next_rx_slot
                slot = get_next_rx_slot(id, pri, slot);

                next_rx_slot[id][pri] = slot;

                rmt_ris = (slot < 32) ? peer_transq[id]->RMT_ISC.RMT_RIS & (1 << slot) :
                                peer_transq[id]->RMT_ISC_E.RMT_RIS & (1 << (slot -32));
                if (rmt_ris) {
                    // Next msg available
                    if (data)
                        *data = (const uint8_t *)peer_transq[id]->RSLOT[slot].ADDR;
                    if (len)
                        *len = peer_transq[id]->RSLOT[slot].LEN;
                    break;
                }
            }
        }
        ret = HAL_TRANSQ_RET_RX_EMPTY;
    } while (0);

    if (ret == HAL_TRANSQ_RET_RX_EMPTY) {
        if (data) {
            *data = NULL;
        }
        if (len) {
            *len = 0;
        }

        transq_cfg[id].rx_irq_count = 0;
        // Re-enable IRQ
        uint32_t mask;
        mask = (uint32_t)rx_irq_mask[id][pri];
        peer_transq[id]->RMT_INTMASK |= mask;
        if (transq_slot_num[id] > 32) {
            mask = rx_irq_mask[id][pri] >> 32;
            peer_transq[id]->RMT_INTMASK_E |= mask;
        }
    }

    return ret;
}

enum HAL_TRANSQ_RET_T hal_transq_tx(enum HAL_TRANSQ_ID_T id, enum HAL_TRANSQ_PRI_T pri, const uint8_t *data, uint32_t len)
{
    enum HAL_TRANSQ_RET_T ret;
    uint32_t lock;
    uint32_t slot;

    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }
    if (pri >= HAL_TRANSQ_PRI_QTY) {
        return HAL_TRANSQ_RET_BAD_PRI;
    }
    if (transq_cfg[id].slot.tx_num[pri] == 0) {
        return HAL_TRANSQ_RET_BAD_TX_NUM;
    }

    lock = transq_lock();

    if (tx_slot_full[id][pri]) {
        ret = HAL_TRANSQ_RET_TX_FULL;
    } else {
        ret = HAL_TRANSQ_RET_OK;

        slot = next_tx_slot[id][pri];
        transq[id]->WSLOT[slot].ADDR = (uint32_t)data;
        transq[id]->WSLOT[slot].LEN = len;
        if(slot < 32) {
            transq[id]->RMT_INTSET = (1 << slot);
        } else {
            transq[id]->RMT_INTSET_E = (1 << (slot - 32));
        }

        // Update active_tx_slot if this is the only tx in progress
        if (active_tx_slot[id][pri] >= transq_slot_num[id]) {
            active_tx_slot[id][pri] = slot;
        }

        // Update next_tx_slot
        next_tx_slot[id][pri] = get_next_tx_slot(id, pri, slot);

        if (next_tx_slot[id][pri] == active_tx_slot[id][pri]) {
            tx_slot_full[id][pri] = true;
        }
    }
    transq_unlock(lock);

    return ret;
}

enum HAL_TRANSQ_RET_T hal_transq_update_num(enum HAL_TRANSQ_ID_T id, const struct HAL_TRANSQ_SLOT_NUM_T *slot)
{
    uint32_t tx_mask;

    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }
    if (slot == NULL) {
        return HAL_TRANSQ_RET_BAD_SLOT;
    }
    if (slot->tx_num[HAL_TRANSQ_PRI_NORMAL] + slot->tx_num[HAL_TRANSQ_PRI_HIGH] > transq_slot_num[id]) {
        return HAL_TRANSQ_RET_BAD_TX_NUM;
    }
    if (slot->rx_num[HAL_TRANSQ_PRI_NORMAL] + slot->rx_num[HAL_TRANSQ_PRI_HIGH] > transq_slot_num[id]) {
        return HAL_TRANSQ_RET_BAD_RX_NUM;
    }

    transq_cfg[id].slot = *slot;

    rx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL] = construct_mask(id, 0, slot->rx_num[HAL_TRANSQ_PRI_NORMAL]);
    rx_irq_mask[id][HAL_TRANSQ_PRI_HIGH] = construct_mask(id, transq_slot_num[id] - slot->rx_num[HAL_TRANSQ_PRI_HIGH], slot->rx_num[HAL_TRANSQ_PRI_HIGH]);

    tx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL] = construct_mask(id, 0, slot->tx_num[HAL_TRANSQ_PRI_NORMAL]);
    tx_irq_mask[id][HAL_TRANSQ_PRI_HIGH] = construct_mask(id, transq_slot_num[id] - slot->tx_num[HAL_TRANSQ_PRI_HIGH], slot->tx_num[HAL_TRANSQ_PRI_HIGH]);

    tx_mask = (uint32_t)(tx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL] | tx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]);

    transq[id]->RMT_INTMASK = tx_mask;
    transq[id]->LERR_INTMASK = tx_mask;

    if (transq_cfg[id].tx_handler) {
        transq[id]->LDONE_INTMASK = tx_mask;
    } else {
        transq[id]->LDONE_INTMASK = 0;
    }
    if (transq_slot_num[id] > 32) {
        tx_mask = (tx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL] | tx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]) >> 32;
        transq[id]->RMT_INTMASK_E = tx_mask;
        transq[id]->LERR_INTMASK_E = tx_mask;
        if (transq_cfg[id].tx_handler) {
            transq[id]->LDONE_INTMASK_E = tx_mask;
        } else {
            transq[id]->LDONE_INTMASK_E = 0;
        }
    }

    TRACE(1, "transq:%d, tx_handler%p, rx_mask:" U64_STR "," U64_STR "\r\n",
        id, transq_cfg[id].tx_handler,
        U64_2_STR(rx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL]), U64_2_STR(rx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]));
    TRACE(1, "tx_mask" U64_STR "," U64_STR" \r\n",
        U64_2_STR(tx_irq_mask[id][HAL_TRANSQ_PRI_NORMAL]), U64_2_STR(tx_irq_mask[id][HAL_TRANSQ_PRI_HIGH]));

    return HAL_TRANSQ_RET_OK;
}

enum HAL_TRANSQ_RET_T hal_transq_open(enum HAL_TRANSQ_ID_T id, const struct HAL_TRANSQ_CFG_T *cfg)
{
    const struct HAL_TRANSQ_SLOT_NUM_T *slot;
    uint32_t ctrl;
    enum HAL_TRANSQ_RET_T ret;

    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }
    if (cfg == NULL) {
        return HAL_TRANSQ_RET_BAD_CFG;
    }

    hal_cmu_clock_enable(transq_mod[id]);
    hal_cmu_reset_clear(transq_mod[id]);

    transq_cfg[id] = *cfg;

    slot = &cfg->slot;

    ret = hal_transq_update_num(id, slot);
    if (ret) {
        return ret;
    }

    next_tx_slot[id][HAL_TRANSQ_PRI_NORMAL] = slot->tx_num[HAL_TRANSQ_PRI_NORMAL] ? 0 : transq_slot_num[id];
    active_tx_slot[id][HAL_TRANSQ_PRI_NORMAL] = transq_slot_num[id];
    tx_slot_full[id][HAL_TRANSQ_PRI_NORMAL] = false;
    next_rx_slot[id][HAL_TRANSQ_PRI_NORMAL] = slot->rx_num[HAL_TRANSQ_PRI_NORMAL] ? 0 : transq_slot_num[id];

    next_tx_slot[id][HAL_TRANSQ_PRI_HIGH] = slot->tx_num[HAL_TRANSQ_PRI_HIGH] ?
        (transq_slot_num[id] - slot->tx_num[HAL_TRANSQ_PRI_HIGH]) : transq_slot_num[id];
    active_tx_slot[id][HAL_TRANSQ_PRI_HIGH] =  transq_slot_num[id];
    tx_slot_full[id][HAL_TRANSQ_PRI_HIGH] = false;
    next_rx_slot[id][HAL_TRANSQ_PRI_HIGH] = slot->rx_num[HAL_TRANSQ_PRI_HIGH] ?
        (transq_slot_num[id] - slot->rx_num[HAL_TRANSQ_PRI_HIGH]) : transq_slot_num[id];

    transq[id]->RMT_ISC.RMT_INTCLR = ~0UL;
    transq[id]->LDONE_ISC.LDONE_INTCLR = ~0UL;
    transq[id]->LERR_ISC.LERR_INTCLR = ~0UL;
    if (transq_slot_num[id] > 32) {
        transq[id]->RMT_ISC_E.RMT_INTCLR = ~0UL;
        transq[id]->LDONE_ISC_E.LDONE_INTCLR = ~0UL;
        transq[id]->LERR_ISC_E.LERR_INTCLR = ~0UL;
    }
    ctrl = CTRL_REMOTE_IRQ_EN | CTRL_LOCAL_ERR_IRQ_EN;
    if (cfg->tx_handler) {
        ctrl |= CTRL_LOCAL_DONE_IRQ_EN;
    }
    transq[id]->CTRL = ctrl;

    if (cfg->rx_handler) {
        NVIC_SetVector(remote_irq_num[id], (uint32_t)hal_transq_remote_irq_handler);
        NVIC_SetPriority(remote_irq_num[id], IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(remote_irq_num[id]);
        NVIC_EnableIRQ(remote_irq_num[id]);
    }

#ifdef KERNEL_LITEOS_A
    NVIC_SetVector(local_irq_num[id], (uint32_t)hal_transq_local_irq_handler_wrap);
#else
    NVIC_SetVector(local_irq_num[id], (uint32_t)hal_transq_local_irq_handler);
#endif
    NVIC_SetPriority(local_irq_num[id], IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(local_irq_num[id]);
    NVIC_EnableIRQ(local_irq_num[id]);

    transq_lock_init();

    return HAL_TRANSQ_RET_OK;
}

enum HAL_TRANSQ_RET_T hal_transq_close(enum HAL_TRANSQ_ID_T id)
{
    if (id >= HAL_TRANSQ_ID_QTY) {
        return HAL_TRANSQ_RET_BAD_ID;
    }

    NVIC_DisableIRQ(remote_irq_num[id]);
    NVIC_DisableIRQ(local_irq_num[id]);

    transq[id]->CTRL = 0;

    hal_cmu_reset_set(transq_mod[id]);
    hal_cmu_clock_disable(transq_mod[id]);

    return HAL_TRANSQ_RET_OK;
}

enum HAL_TRANSQ_RET_T hal_transq_flush(enum HAL_TRANSQ_ID_T id)
{
    transq[id]->RMT_ISC.RMT_INTCLR = ~0UL;
    if (transq_slot_num[id] > 32) {
        transq[id]->RMT_ISC_E.RMT_INTCLR = ~0UL;
    }

    return HAL_TRANSQ_RET_OK;
}

uint8_t hal_transq_get_rx_irq_count(enum HAL_TRANSQ_ID_T id)
{
    return transq_cfg[id].rx_irq_count;
}

#endif