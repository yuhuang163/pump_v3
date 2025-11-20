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
#include "plat_addr_map.h"

#if defined(CP_AS_SUBSYS) || defined(CHIP_ROLE_CP)

#include "cmsis_nvic.h"
#include "hal_rmt_ipc_pri.h"
#include "hal_trace.h"
#include "hal_location.h"
#ifdef CHIP_SUBSYS_SENS
#include CHIP_SPECIFIC_HDR(reg_senscmu)
#elif defined(CHIP_SUBSYS_BTH)
#include CHIP_SPECIFIC_HDR(reg_bthcmu)
#else
#include CHIP_SPECIFIC_HDR(reg_cmu)
#endif

#ifdef CMU_MCU2CP_DATA_IND_SET

#define MCU2CP_CHAN_QTY                     (HAL_RMT_IPC_CHAN_1 + 1)

#ifndef MCU2CP_CHAN_REC_CNT
#define MCU2CP_CHAN_REC_CNT                 3
#endif

#define MCU_CFG_PTR_LOC                     (CP_SUBSYS_MAILBOX_BASE + 0)
#define CP_CFG_PTR_LOC                      (CP_SUBSYS_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *))

STATIC_ASSERT((2 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= CP_SUBSYS_MAILBOX_SIZE, "CP_SUBSYS_MAILBOX_SIZE too small");

#ifdef CHIP_SUBSYS_SENS
static struct SENSCMU_T * const cmu = (struct SENSCMU_T *)SENS_CMU_BASE;
#elif defined(CHIP_SUBSYS_BTH)
static struct BTHCMU_T * const cmu = (struct BTHCMU_T *)BTH_CMU_BASE;
#else
static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;
#endif

static const IRQn_Type rx_irq_id[MCU2CP_CHAN_QTY] = {
    CP2MCU_DATA_IRQn,
    CP2MCU_DATA1_IRQn,
};

static const IRQn_Type tx_irq_id[MCU2CP_CHAN_QTY] = {
    MCU2CP_DONE_IRQn,
    MCU2CP_DONE1_IRQn,
};

SYNC_FLAGS_LOC
static struct HAL_RMT_IPC_CH_CFG_T mcu2cp_chan_cfg[MCU2CP_CHAN_QTY];
SYNC_FLAGS_LOC
static struct HAL_RMT_IPC_SEND_RECORD_T mcu2cp_send_record[MCU2CP_CHAN_QTY * MCU2CP_CHAN_REC_CNT];

static int hal_mcu2cp_peer_irq_set(enum HAL_RMT_IPC_CHAN_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_ROLE_CP
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA_IND_SET;
        } else {
            value = CMU_MCU2CP_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA1_IND_SET;
        } else {
            value = CMU_MCU2CP_DATA1_DONE_SET;
        }
    }

    cmu->CP2MCU_IRQ_SET = value;
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA_IND_SET;
        } else {
            value = CMU_CP2MCU_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA1_IND_SET;
        } else {
            value = CMU_CP2MCU_DATA1_DONE_SET;
        }
    }

    cmu->MCU2CP_IRQ_SET = value;
#endif

    return 0;
}

static int hal_mcu2cp_local_irq_clear(enum HAL_RMT_IPC_CHAN_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_ROLE_CP
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA_IND_CLR;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            // MCU2CP_DATA_DONE means DATA_DONE signal is sent from MCU to CP
            value = CMU_MCU2CP_DATA_DONE_CLR;
#else
            // CP2MCU_DATA_DONE means the msg sent from CP to MCU has been done
            value = CMU_CP2MCU_DATA_DONE_CLR;
#endif
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA1_IND_CLR;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            value = CMU_MCU2CP_DATA1_DONE_CLR;
#else
            value = CMU_CP2MCU_DATA1_DONE_CLR;
#endif
        }
    }

#ifdef CP_IPC_MODE_CROSS_DOMAIN
    if (type == HAL_RMT_IPC_IRQ_DATA_DONE) {
        cmu->CP2MCU_IRQ_CLR = value;
    } else
#endif
    {
        cmu->MCU2CP_IRQ_CLR = value;
    }
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA_IND_CLR;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            value = CMU_CP2MCU_DATA_DONE_CLR;
#else
            value = CMU_MCU2CP_DATA_DONE_CLR;
#endif
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA1_IND_CLR;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            value = CMU_CP2MCU_DATA1_DONE_CLR;
#else
            value = CMU_MCU2CP_DATA1_DONE_CLR;
#endif
        }
    }

#ifdef CP_IPC_MODE_CROSS_DOMAIN
    if (type == HAL_RMT_IPC_IRQ_DATA_DONE) {
        cmu->MCU2CP_IRQ_CLR = value;
    } else
#endif
    {
        cmu->CP2MCU_IRQ_CLR = value;
    }
#endif

    cmu->CP2MCU_IRQ_CLR;
    __DSB();

    return 0;
}

#ifndef CP_IPC_MODE_CROSS_DOMAIN
static int hal_mcu2cp_local_irq_set(enum HAL_RMT_IPC_CHAN_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_ROLE_CP
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA_IND_SET;
        } else {
            value = CMU_CP2MCU_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA1_IND_SET;
        } else {
            value = CMU_CP2MCU_DATA1_DONE_SET;
        }
    }

    cmu->MCU2CP_IRQ_SET = value;
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA_IND_SET;
        } else {
            value = CMU_MCU2CP_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA1_IND_SET;
        } else {
            value = CMU_MCU2CP_DATA1_DONE_SET;
        }
    }

    cmu->CP2MCU_IRQ_SET = value;
#endif

    return 0;
}
#endif

int hal_mcu2cp_rx_irq_pending(enum HAL_RMT_IPC_CHAN_T id)
{
    uint32_t value;

#ifdef CHIP_ROLE_CP
    if (id == HAL_RMT_IPC_CHAN_0) {
        value = CMU_MCU2CP_DATA_IND_SET;
    } else {
        value = CMU_MCU2CP_DATA1_IND_SET;
    }

    return !!(cmu->MCU2CP_IRQ_SET & value);
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        value = CMU_CP2MCU_DATA_IND_SET;
    } else {
        value = CMU_CP2MCU_DATA1_IND_SET;
    }

    return !!(cmu->CP2MCU_IRQ_SET & value);
#endif
}

static void hal_mcu2cp_irq_init(uint32_t id)
{
#ifdef CHIP_ROLE_CP
    if (id == HAL_RMT_IPC_CHAN_0) {
        cmu->CP2MCU_IRQ_CLR = CMU_MCU2CP_DATA_DONE_CLR | CMU_CP2MCU_DATA_IND_CLR;
    } else {
        cmu->CP2MCU_IRQ_CLR = CMU_MCU2CP_DATA1_DONE_CLR | CMU_CP2MCU_DATA1_IND_CLR;
    }
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        cmu->MCU2CP_IRQ_CLR = CMU_CP2MCU_DATA_DONE_CLR | CMU_MCU2CP_DATA_IND_CLR;
    } else {
        cmu->MCU2CP_IRQ_CLR = CMU_CP2MCU_DATA1_DONE_CLR | CMU_MCU2CP_DATA1_IND_CLR;
    }
#endif

    cmu->CP2MCU_IRQ_CLR;
    __DSB();
}

static void hal_mcu2cp_peer_tx_irq_set(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

    hal_mcu2cp_peer_irq_set(id, HAL_RMT_IPC_IRQ_DATA_IND);
}

static void hal_mcu2cp_local_tx_irq_clear(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

    hal_mcu2cp_local_irq_clear(id, HAL_RMT_IPC_IRQ_DATA_DONE);
}

static void hal_mcu2cp_rx_irq_suspend(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

#ifdef CP_IPC_MODE_CROSS_DOMAIN
    NVIC_DisableIRQ(rx_irq_id[id]);
#else
    hal_mcu2cp_local_irq_clear(id, HAL_RMT_IPC_IRQ_DATA_IND);
#endif
}

static void hal_mcu2cp_rx_irq_resume(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

#ifdef CP_IPC_MODE_CROSS_DOMAIN
    NVIC_EnableIRQ(rx_irq_id[id]);
#else
    hal_mcu2cp_local_irq_set(id, HAL_RMT_IPC_IRQ_DATA_IND);
#endif
}

static void hal_mcu2cp_rx_done_id32(uint32_t id32)
{
    enum HAL_RMT_IPC_CHAN_T id = (enum HAL_RMT_IPC_CHAN_T)id32;

#ifdef CP_IPC_MODE_CROSS_DOMAIN
    hal_mcu2cp_local_irq_clear(id, HAL_RMT_IPC_IRQ_DATA_IND);
    NVIC_EnableIRQ(rx_irq_id[id]);
#else
    hal_mcu2cp_peer_irq_set(id, HAL_RMT_IPC_IRQ_DATA_DONE);
#endif
}

static int hal_mcu2cp_irq_active(uint32_t id, uint32_t type)
{
    uint32_t value;

#ifdef CHIP_ROLE_CP
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA_IND_SET;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            value = CMU_MCU2CP_DATA_DONE_SET;
#else
            value = CMU_CP2MCU_DATA_DONE_SET;
#endif
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_MCU2CP_DATA1_IND_SET;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            value = CMU_MCU2CP_DATA1_DONE_SET;
#else
            value = CMU_CP2MCU_DATA1_DONE_SET;
#endif
        }
    }

#ifdef CP_IPC_MODE_CROSS_DOMAIN
    if (type == HAL_RMT_IPC_IRQ_DATA_DONE) {
        return !!(cmu->CP2MCU_IRQ_SET & value);
    }
#endif
    return !!(cmu->MCU2CP_IRQ_SET & value);
#else
    if (id == HAL_RMT_IPC_CHAN_0) {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA_IND_SET;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            value = CMU_CP2MCU_DATA_DONE_SET;
#else
            value = CMU_MCU2CP_DATA_DONE_SET;
#endif
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_DATA_IND) {
            value = CMU_CP2MCU_DATA1_IND_SET;
        } else {
#ifdef CP_IPC_MODE_CROSS_DOMAIN
            value = CMU_CP2MCU_DATA1_DONE_SET;
#else
            value = CMU_MCU2CP_DATA1_DONE_SET;
#endif
        }
    }

#ifdef CP_IPC_MODE_CROSS_DOMAIN
    if (type == HAL_RMT_IPC_IRQ_DATA_DONE) {
        return !!(cmu->MCU2CP_IRQ_SET & value);
    }
#endif
    return !!(cmu->CP2MCU_IRQ_SET & value);
#endif
}

struct HAL_RMT_IPC_CFG_T mcu2cp_cfg;

static void hal_mcu2cp_rx_irq_handler(void)
{
    hal_rmt_ipc_rx_irq_handler(&mcu2cp_cfg);
}

static void hal_mcu2cp_tx_irq_handler(void)
{
    hal_rmt_ipc_tx_irq_handler(&mcu2cp_cfg);
}

// CP might NOT be allowed to access flash
struct HAL_RMT_IPC_CFG_T mcu2cp_cfg = {
    .name = "MCU2CP",
#ifdef CHIP_ROLE_CP
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)MCU_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)CP_CFG_PTR_LOC,
#else
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)CP_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)MCU_CFG_PTR_LOC,
#endif

    .irq_init = hal_mcu2cp_irq_init,
    .peer_tx_irq_set = hal_mcu2cp_peer_tx_irq_set,
    .local_tx_irq_clear = hal_mcu2cp_local_tx_irq_clear,
    .rx_irq_suspend = hal_mcu2cp_rx_irq_suspend,
    .rx_irq_resume = hal_mcu2cp_rx_irq_resume,
    .rx_done = hal_mcu2cp_rx_done_id32,
    .irq_active = hal_mcu2cp_irq_active,
    .rx_irq_entry = hal_mcu2cp_rx_irq_handler,
    .tx_irq_entry = hal_mcu2cp_tx_irq_handler,
#ifdef CORE_SLEEP_POWER_DOWN
    .pm_notif_handler = NULL,
#endif

    .chan_num = MCU2CP_CHAN_QTY,
    .rec_num_per_chan = ARRAY_SIZE(mcu2cp_send_record) / MCU2CP_CHAN_QTY,
    .chan_cfg = &mcu2cp_chan_cfg[0],
    .send_record = &mcu2cp_send_record[0],
    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],
    .wake_lock_en = false,
#ifdef OPENAMP_ENABLE
    .openamp_rsc_start = (uint32_t)__resource_table_start__ ,
    .openamp_rsc_end = (uint32_t)__resource_table_end__,
#endif
};

#endif
#endif
