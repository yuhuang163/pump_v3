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
#include "plat_addr_map.h"

#ifdef SENS_CMU_BASE

#include "cmsis_nvic.h"
#include "hal_rmt_ipc_pri.h"
#include "hal_location.h"
#include "hal_ipc_notify_api.h"
#include "hal_ipc_notify.h"
#include "hal_trace.h"
#ifdef BTH_AS_MAIN_MCU
#include CHIP_SPECIFIC_HDR(reg_bthcmu)
#else
#include CHIP_SPECIFIC_HDR(reg_cmu)
#endif
#include CHIP_SPECIFIC_HDR(reg_senscmu)

#define MCU2SENS_CHAN_QTY                   (HAL_RMT_IPC_CHAN_1 + 1)

#ifndef MCU2SENS_CHAN_REC_CNT
#define MCU2SENS_CHAN_REC_CNT               3
#endif

#define MCU_CFG_PTR_LOC                     (SENS_MAILBOX_BASE + 0)
#define SENS_CFG_PTR_LOC                    (SENS_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *))

STATIC_ASSERT((2 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= SENS_MAILBOX_SIZE, "SENS_MAILBOX_SIZE too small");

#ifdef BTH_AS_MAIN_MCU
static struct BTHCMU_T * const cmu = (struct BTHCMU_T *)BTH_CMU_BASE;
#else
static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;
#endif
static struct SENSCMU_T * const senscmu = (struct SENSCMU_T *)SENS_CMU_BASE;

static const IRQn_Type rx_irq_id[MCU2SENS_CHAN_QTY] = {
    SENS2MCU_DATA_IRQn,
    SENS2MCU_DATA1_IRQn,
};

static const IRQn_Type tx_irq_id[MCU2SENS_CHAN_QTY] = {
    MCU2SENS_DONE_IRQn,
    MCU2SENS_DONE1_IRQn,
};

SYNC_FLAGS_LOC
static struct HAL_RMT_IPC_CH_CFG_T mcu2sens_chan_cfg[MCU2SENS_CHAN_QTY];
SYNC_FLAGS_LOC
static struct HAL_RMT_IPC_SEND_RECORD_T mcu2sens_send_record[MCU2SENS_CHAN_QTY * MCU2SENS_CHAN_REC_CNT];

static int hal_mcu2sens_peer_irq_set(enum HAL_RMT_IPC_CHAN_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

    if (type != HAL_RMT_IPC_IRQ_DATA_IND) {
        return 1;
    }

#ifdef CHIP_SUBSYS_SENS
    if (id == HAL_RMT_IPC_CHAN_0) {
        value = SENS_CMU_SENS2MCU_DATA_IND_SET;
    } else {
        value = SENS_CMU_SENS2MCU_DATA1_IND_SET;
    }

    senscmu->SENS2MCU_IRQ_SET = value;
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        value = CMU_MCU2SENS_DATA_IND_SET;
    } else {
        value = CMU_MCU2SENS_DATA1_IND_SET;
    }

    cmu->MCU2SENS_IRQ_SET = value;
#endif

    return 0;
}

static int hal_mcu2sens_local_irq_clear(enum HAL_RMT_IPC_CHAN_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_SENS
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2SENS_DATA_IND_CLR;
        } else {
            value = SENS_CMU_MCU2SENS_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2SENS_DATA1_IND_CLR;
        } else {
            value = SENS_CMU_MCU2SENS_DATA1_DONE_CLR;
        }
    }

    if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
        cmu->MCU2SENS_IRQ_CLR = value;
        // Flush the clear operation immediately
        cmu->MCU2SENS_IRQ_CLR;
    } else {
        senscmu->SENS2MCU_IRQ_CLR = value;
    }
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_SENS2MCU_DATA_IND_CLR;
        } else {
            value = CMU_SENS2MCU_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_SENS2MCU_DATA1_IND_CLR;
        } else {
            value = CMU_SENS2MCU_DATA1_DONE_CLR;
        }
    }

    if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
        senscmu->SENS2MCU_IRQ_CLR = value;
        // Flush the clear operation immediately
        senscmu->SENS2MCU_IRQ_CLR;
    } else {
        cmu->MCU2SENS_IRQ_CLR = value;
    }
#endif

    return 0;
}

static int hal_mcu2sens_local_irq_mask_set(enum HAL_RMT_IPC_CHAN_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_SENS
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_MCU2SENS_DATA_MSK_SET;
        } else {
            value = SENS_CMU_SENS2MCU_DATA_MSK_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_MCU2SENS_DATA1_MSK_SET;
        } else {
            value = SENS_CMU_SENS2MCU_DATA1_MSK_SET;
        }
    }

    senscmu->SENS2MCU_IRQ_SET = value;
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_SENS2MCU_DATA_MSK_SET;
        } else {
            value = CMU_MCU2SENS_DATA_MSK_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_SENS2MCU_DATA1_MSK_SET;
        } else {
            value = CMU_MCU2SENS_DATA1_MSK_SET;
        }
    }

    cmu->MCU2SENS_IRQ_SET = value;
#endif

    return 0;
}

static int hal_mcu2sens_local_irq_mask_clear(enum HAL_RMT_IPC_CHAN_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_SENS
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_MCU2SENS_DATA_MSK_CLR;
        } else {
            value = SENS_CMU_SENS2MCU_DATA_MSK_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_MCU2SENS_DATA1_MSK_CLR;
        } else {
            value = SENS_CMU_SENS2MCU_DATA1_MSK_CLR;
        }
    }

    senscmu->SENS2MCU_IRQ_CLR = value;
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_SENS2MCU_DATA_MSK_CLR;
        } else {
            value = CMU_MCU2SENS_DATA_MSK_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_SENS2MCU_DATA1_MSK_CLR;
        } else {
            value = CMU_MCU2SENS_DATA1_MSK_CLR;
        }
    }

    cmu->MCU2SENS_IRQ_CLR = value;
#endif

    return 0;
}

int hal_mcu2sens_rx_irq_pending(enum HAL_RMT_IPC_CHAN_T id)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_SENS
    if (id == HAL_RMT_IPC_CHAN_0) {
        value = SENS_CMU_MCU2SENS_DATA_INTR;
    } else {
        value = SENS_CMU_MCU2SENS_DATA1_INTR;
    }

    return !!(senscmu->SENS2MCU_IRQ_SET & value);
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        value = CMU_SENS2MCU_DATA_INTR;
    } else {
        value = CMU_SENS2MCU_DATA1_INTR;
    }

    return !!(cmu->MCU2SENS_IRQ_SET & value);
#endif
}

static void hal_mcu2sens_irq_init(uint32_t id)
{
#ifdef CHIP_SUBSYS_SENS
    if (id == HAL_RMT_IPC_CHAN_0) {
        senscmu->SENS2MCU_IRQ_CLR = SENS_CMU_MCU2SENS_DATA_DONE_CLR | SENS_CMU_SENS2MCU_DATA_IND_CLR;
    } else {
        senscmu->SENS2MCU_IRQ_CLR = SENS_CMU_MCU2SENS_DATA1_DONE_CLR | SENS_CMU_SENS2MCU_DATA1_IND_CLR;
    }
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        cmu->MCU2SENS_IRQ_CLR = CMU_SENS2MCU_DATA_DONE_CLR | CMU_MCU2SENS_DATA_IND_CLR;
    } else {
        cmu->MCU2SENS_IRQ_CLR = CMU_SENS2MCU_DATA1_DONE_CLR | CMU_MCU2SENS_DATA1_IND_CLR;
    }
#endif

    hal_mcu2sens_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_DATA_IND);
    hal_mcu2sens_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_DATA_DONE);
}

static void hal_mcu2sens_peer_tx_irq_set(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

    hal_mcu2sens_peer_irq_set(id, HAL_RMT_IPC_IRQ_DATA_IND);
}

static void hal_mcu2sens_local_tx_irq_clear(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

    hal_mcu2sens_local_irq_clear(id, HAL_RMT_IPC_IRQ_DATA_DONE);
}

static void hal_mcu2sens_rx_irq_suspend(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

    hal_mcu2sens_local_irq_mask_clear(id, HAL_RMT_IPC_IRQ_DATA_IND);
}

static void hal_mcu2sens_rx_irq_resume(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

    hal_mcu2sens_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_DATA_IND);
}

static void hal_mcu2sens_rx_done_id32(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

    hal_mcu2sens_local_irq_clear(id, HAL_RMT_IPC_IRQ_DATA_IND);
    hal_mcu2sens_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_DATA_IND);
}

static int hal_mcu2sens_irq_active(uint32_t id, uint32_t type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_SENS
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_MCU2SENS_DATA_INTR_MSK;
        } else {
            value = SENS_CMU_SENS2MCU_DATA_INTR_MSK;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = SENS_CMU_MCU2SENS_DATA1_INTR_MSK;
        } else {
            value = SENS_CMU_SENS2MCU_DATA1_INTR_MSK;
        }
    }

    return !!(senscmu->SENS2MCU_IRQ_SET & value);
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_SENS2MCU_DATA_INTR_MSK;
        } else {
            value = CMU_MCU2SENS_DATA_INTR_MSK;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_SENS2MCU_DATA1_INTR_MSK;
        } else {
            value = CMU_MCU2SENS_DATA1_INTR_MSK;
        }
    }

    return !!(cmu->MCU2SENS_IRQ_SET & value);
#endif
}

RMT_IPC_OPEN_CFG_QUAL
struct HAL_RMT_IPC_CFG_T mcu2sens_cfg;

static void hal_mcu2sens_rx_irq_handler(void)
{
    hal_rmt_ipc_rx_irq_handler(&mcu2sens_cfg);
}

static void hal_mcu2sens_tx_irq_handler(void)
{
    hal_rmt_ipc_tx_irq_handler(&mcu2sens_cfg);
}

#ifdef CORE_SLEEP_POWER_DOWN
static int hal_mcu2sens_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    return hal_rmt_ipc_pm_notif_handler(&mcu2sens_cfg, state);
}
#endif

// best1501: Avoid sensor hub accessing mcu flash data (for flash might be in program state)
#if defined(CHIP_BEST1501) || \
    (defined(CHIP_SUBSYS_SENS) && defined(NO_SUBSYS_FLASH_ADDR_ACCESS) && !defined(CPU_TO_DEV_ADDR_REMAP))
SRAM_RODATA_LOC
#endif
RMT_IPC_OPEN_CFG_QUAL
struct HAL_RMT_IPC_CFG_T mcu2sens_cfg = {
    .name = "MCU2SENS",
#ifdef CHIP_SUBSYS_SENS
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)MCU_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)SENS_CFG_PTR_LOC,
#else
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)SENS_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)MCU_CFG_PTR_LOC,
#endif

    .irq_init = hal_mcu2sens_irq_init,
    .peer_tx_irq_set = hal_mcu2sens_peer_tx_irq_set,
    .local_tx_irq_clear = hal_mcu2sens_local_tx_irq_clear,
    .rx_irq_suspend = hal_mcu2sens_rx_irq_suspend,
    .rx_irq_resume = hal_mcu2sens_rx_irq_resume,
    .rx_done = hal_mcu2sens_rx_done_id32,
    .irq_active = hal_mcu2sens_irq_active,
    .rx_irq_entry = hal_mcu2sens_rx_irq_handler,
    .tx_irq_entry = hal_mcu2sens_tx_irq_handler,
#ifdef CORE_SLEEP_POWER_DOWN
    .pm_notif_handler = hal_mcu2sens_pm_notif_handler,
#endif

    .chan_num = MCU2SENS_CHAN_QTY,
    .rec_num_per_chan = ARRAY_SIZE(mcu2sens_send_record) / MCU2SENS_CHAN_QTY,
    .chan_cfg = &mcu2sens_chan_cfg[0],
    .send_record = &mcu2sens_send_record[0],
    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],
    .wake_lock_en = true,
#ifdef OPENAMP_ENABLE
    .openamp_rsc_start = (uint32_t)__resource_table_start__,
    .openamp_rsc_end = (uint32_t)__resource_table_end__,
#endif
};

static struct HAL_IPC_NOTIFY_CH_CFG_T mcu2sens_ipc_notify_chan_cfg[MCU2SENS_CHAN_QTY];
static void hal_mcu2sens_ipc_notify_rx_irq_handler(void);
static void hal_mcu2sens_ipc_notify_tx_irq_handler(void);
#ifdef CORE_SLEEP_POWER_DOWN
static int hal_mcu2sens_ipc_pm_notif_handler(enum HAL_PM_STATE_T state);
#endif

static const struct HAL_IPC_NOTIFY_CFG_T mcu2sens_ipc_notify_cfg = {
    .name = "MCU2SENS",
    .core = HAL_IPC_NOTIFY_CORE_SYS_SENS,

    .irq_init = hal_mcu2sens_irq_init,
    .peer_tx_irq_set = hal_mcu2sens_peer_tx_irq_set,
    .local_tx_irq_clear = hal_mcu2sens_local_tx_irq_clear,

    .rx_done = hal_mcu2sens_rx_done_id32,
    .irq_active = hal_mcu2sens_irq_active,
    .rx_irq_entry = hal_mcu2sens_ipc_notify_rx_irq_handler,
    .tx_irq_entry = hal_mcu2sens_ipc_notify_tx_irq_handler,
    .chan_num = MCU2SENS_CHAN_QTY,
    .chan_cfg = &mcu2sens_ipc_notify_chan_cfg[0],
#ifdef CORE_SLEEP_POWER_DOWN
    .pm_notif_handler = hal_mcu2sens_ipc_pm_notif_handler,
#endif

    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],

};

static void hal_mcu2sens_ipc_notify_rx_irq_handler(void)
{
    hal_ipc_notify_internal_rx_irq_handler(&mcu2sens_ipc_notify_cfg);
}

static void hal_mcu2sens_ipc_notify_tx_irq_handler(void)
{
    hal_ipc_notify_internal_tx_irq_handler(&mcu2sens_ipc_notify_cfg);
}

#ifdef CORE_SLEEP_POWER_DOWN
static int hal_mcu2sens_ipc_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    return hal_ipc_notify_pm_notif_handler(&mcu2sens_ipc_notify_cfg, state);
}
#endif

int hal_mcu2sens_ipc_notify_open(enum HAL_RMT_IPC_CHAN_T id, HAL_MCU2SENS_NOTIFY_IRQ_HANDLER rxhandler, HAL_MCU2SENS_NOTIFY_IRQ_HANDLER txhandler)
{
    return hal_ipc_notify_internal_open(&mcu2sens_ipc_notify_cfg, id, rxhandler, txhandler);
}

int hal_mcu2sens_ipc_notify_start_recv(enum HAL_RMT_IPC_CHAN_T id)
{
    return hal_ipc_notify_internal_start_recv(&mcu2sens_ipc_notify_cfg, id);
}

int hal_mcu2sens_ipc_notify_interrupt_core(enum HAL_RMT_IPC_CHAN_T id)
{
    return hal_ipc_notify_internal_peer_irq_set(&mcu2sens_ipc_notify_cfg, (uint32_t)id);
}

int hal_mcu2sens_ipc_notify_close(enum HAL_RMT_IPC_CHAN_T id)
{
    return hal_ipc_notify_internal_close(&mcu2sens_ipc_notify_cfg, (uint32_t)id);
}

#endif
