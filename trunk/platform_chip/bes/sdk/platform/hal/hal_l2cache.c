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
#include "hal_l2cache.h"

#if defined(L2CC0_BASE) && !defined(__ARM_ARCH_ISA_ARM)

#include "hal_location.h"
#include "hal_trace.h"
#include "l2c_310.h"

#ifndef L2CACHE_SIZE
#define L2CACHE_SIZE                        (256 * 1024)
#endif

#define L2CACHE_LINE_SIZE                   32

static L2C_310_TypeDef *l2cc[HAL_L2CACHE_ID_QTY] = {
#ifdef L2CC0_BASE
    (L2C_310_TypeDef *)L2CC0_BASE,
#endif
#ifdef L2CC1_BASE
    (L2C_310_TypeDef *)L2CC1_BASE,
#endif
};

#ifdef CORE_SLEEP_POWER_DOWN
SRAM_BSS_LOC
static bool cache_enabled[HAL_L2CACHE_ID_QTY];
#endif

int hal_l2cache_enable(enum HAL_L2CACHE_ID_T id)
{
    enum HAL_CMU_MOD_ID_T mod = HAL_CMU_MOD_O_L2CC0;
#ifdef L2CC1_BASE
    if (id == HAL_L2CACHE_ID_1) {
        mod = HAL_CMU_MOD_O_L2CC1;
    }
#endif
    hal_cmu_clock_enable(mod);
    hal_cmu_reset_clear(mod);
    L2C_310_Enable(l2cc[id]);
#ifdef CORE_SLEEP_POWER_DOWN
    cache_enabled[id] = true;
#endif
    return 0;
}

int hal_l2cache_disable(enum HAL_L2CACHE_ID_T id)
{
    enum HAL_CMU_MOD_ID_T mod = HAL_CMU_MOD_O_L2CC0;
#ifdef L2CC1_BASE
    if (id == HAL_L2CACHE_ID_1) {
        mod = HAL_CMU_MOD_O_L2CC1;
    }
#endif
    L2C_310_Disable(l2cc[id]);
    hal_cmu_reset_set(mod);
    hal_cmu_clock_disable(mod);
#ifdef CORE_SLEEP_POWER_DOWN
    cache_enabled[id] = false;
#endif
    return 0;
}

int hal_l2cache_enabled(enum HAL_L2CACHE_ID_T id)
{
    enum HAL_CMU_MOD_ID_T mod = HAL_CMU_MOD_O_L2CC0;
#ifdef L2CC1_BASE
    if (id == HAL_L2CACHE_ID_1) {
        mod = HAL_CMU_MOD_O_L2CC1;
    }
#endif
    if (hal_cmu_reset_get_status(mod) == HAL_CMU_RST_SET) {
        return false;
    }
    return L2C_310_Enabled(l2cc[id]);
}

int hal_l2cache_invalidate(enum HAL_L2CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    // Sync and invalidate operation is much slower than invalidate operation,
    // so sync and invalidate the whole cache only when len is much larger than cache size
    if (len > L2CACHE_SIZE * 2 - L2CACHE_LINE_SIZE) {
        L2C_310_CleanInvAllByWay(l2cc[id]);
    } else {
        // PREREQUISITE:
        // To support cache operations not aligned with cache line size,
        // all the buffer shared between processor and hardware device (e.g., DMA)
        // must be invalidated before assigning to the hardware device.
        // Otherwise the hardware device output data might be overwritten by
        // cache sync operaton.

        uint32_t offset;

        // Align start_address to cache line size
        offset = start_address & (L2CACHE_LINE_SIZE - 1);
        if (offset) {
            offset = L2CACHE_LINE_SIZE - offset;
            L2C_310_CleanInvPa(l2cc[id], (void *)start_address);
            start_address += offset;
            len -= offset;
        }
        // Align len to cache line size
        offset = len & (L2CACHE_LINE_SIZE - 1);
        if (offset) {
            len -= offset;
            L2C_310_CleanInvPa(l2cc[id], (void *)start_address + len);
        }
        while (len) {
            L2C_310_InvPa(l2cc[id], (void *)start_address);
            start_address += L2CACHE_LINE_SIZE;
            len -= L2CACHE_LINE_SIZE;
        }
    }

    return 0;
}

int hal_l2cache_invalidate_all(enum HAL_L2CACHE_ID_T id)
{
    L2C_310_InvAllByWay(l2cc[id]);
    return 0;
}

int hal_l2cache_sync_all(enum HAL_L2CACHE_ID_T id)
{
    L2C_310_CleanAllByWay(l2cc[id]);
    return 0;
}

int hal_l2cache_sync_invalidate_all(enum HAL_L2CACHE_ID_T id)
{
    L2C_310_CleanInvAllByWay(l2cc[id]);
    return 0;
}

int BOOT_TEXT_SRAM_LOC hal_l2cache_sync(enum HAL_L2CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    uint32_t offset;

    if (len > L2CACHE_SIZE - L2CACHE_LINE_SIZE) {
        L2C_310_CleanAllByWay(l2cc[id]);
    } else {
        while (len) {
            L2C_310_CleanPa(l2cc[id], (void *)start_address);
            if (len < L2CACHE_LINE_SIZE) {
                offset = len;
            } else {
                offset = L2CACHE_LINE_SIZE;
            }
            start_address += offset;
            len -= offset;
        }
    }

    return 0;
}

int hal_l2cache_sync_invalidate(enum HAL_L2CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    uint32_t offset;

    if (len > L2CACHE_SIZE - L2CACHE_LINE_SIZE) {
        L2C_310_CleanInvAllByWay(l2cc[id]);
    } else {
        while (len) {
            L2C_310_CleanInvPa(l2cc[id], (void *)start_address);
            if (len < L2CACHE_LINE_SIZE) {
                offset = len;
            } else {
                offset = L2CACHE_LINE_SIZE;
            }
            start_address += offset;
            len -= offset;
        }
    }

    return 0;
}

#ifdef CORE_SLEEP_POWER_DOWN
void hal_l2cache_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    if (mode != HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        return;
    }

    for (int id = 0; id < (int)HAL_L2CACHE_ID_QTY; id++) {
        if (cache_enabled[id]) {
            L2C_310_CleanAllByWay(l2cc[id]);
            L2C_310_Disable(l2cc[id]);
        }
    }
}

void hal_l2cache_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    if (mode != HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        return;
    }

    __DSB();
    __ISB();
    for (int id = 0; id < (int)HAL_L2CACHE_ID_QTY; id++) {
        if (cache_enabled[id]) {
            L2C_310_Enable(l2cc[id]);
        }
    }
}
#endif

#endif
