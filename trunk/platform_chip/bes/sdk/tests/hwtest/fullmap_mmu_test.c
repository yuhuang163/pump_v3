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
#ifdef FULLMAP_MMU_TEST

#include "string.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_fullmap_mmu.h"

#if 1
    #define FULLMAP_MMU_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#else
    #define FULLMAP_MMU_TRACE(n, s, ...)
#endif

#if 1
    #define FULLMAP_MMU_DELAY_MS(ms)        hal_sys_timer_delay(MS_TO_TICKS(ms))
#else
    #define FULLMAP_MMU_DELAY_MS(ms)
#endif

void fullmap_mmu_test(void)
{
    struct HAL_FULLMAP_MMU_CONFIG_T mmu_cfg;

    FULLMAP_MMU_TRACE(0, "  ");
    FULLMAP_MMU_TRACE(0, "------fullmap mmu test started------");
    FULLMAP_MMU_TRACE(0, "  ");

    memset(&mmu_cfg, 0, sizeof(mmu_cfg));
    mmu_cfg.over_provison_mode = false;
    if (mmu_cfg.over_provison_mode == true) {
        mmu_cfg.lut_sel = HAL_FULLMAP_MMU_LUT_SEL_LOW_32M;
    }
    mmu_cfg.trans_error_en = true;
    mmu_cfg.virtual_write_en = true;
    mmu_cfg.virtual_read_en = true;
    mmu_cfg.low_power_mode = true;
    mmu_cfg.check_en = true;

    hal_fullmap_mmu_open(HAL_FULLMAP_MMU_ID_0, &mmu_cfg);
    FULLMAP_MMU_TRACE(0, "fullmap mmu has been opened");
    FULLMAP_MMU_TRACE(0, "psram read and write test is performed later");
    FULLMAP_MMU_TRACE(0, "  ");
}

#endif

