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
#include "hwtest.h"
#ifdef SPI_ROM_ONLY
#include "pmu_rom.h"
#else
#include "hal_analogif.h"
#include "pmu.h"
#endif

// EFUSE_TEST
void efuse_test(void)
{
    unsigned int i;
    unsigned short val;

    for (i = 0; i < 16; i++) {
#ifdef SPI_ROM_ONLY
        pmu_rom_get_efuse(i, &val);
#else
        pmu_get_efuse((enum PMU_EFUSE_PAGE_T)i, &val);
#endif
        TRACE(2,"Efuse %d: 0x%04x", i, val);
    }
}


