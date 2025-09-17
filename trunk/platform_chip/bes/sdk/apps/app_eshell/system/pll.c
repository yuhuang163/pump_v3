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
#ifdef UTILS_ESHELL_EN

#ifdef __SYS_AS_MAIN__
#if defined(CHIP_BEST1600)

#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_psramuhs.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "plat_addr_map.h"
#include "hal_location.h"
#include "eshell.h"
#include "app_eshell.h"
#include "pmu.h"

static void sys_pll_autocalib_usage(void)
{
    eshell_putstring("Usage: \r\n");
    eshell_putstring("  pll_autocalib <codec_freq>\r\n");
    eshell_putstring("  codec_freq=aud_samplerate*512\r\n");
    eshell_putstring("  eg:pll_autocalib 22579200\r\n");
}

extern void bbpll_auto_calib_ires(uint32_t codec_freq, uint32_t div);
/*
 * In order to ensure that the audio system works stably at a certain audio
 * sampling rate,it is necessary to calibrate the PLL of the system to work
 * at a value that can precisely divide the frequency corresponding to the
 * operating frequency of CODEC. Therefore, if the audio sampling rate is
 * required to be 48,000, the required AUD CODEC operating frequency is
 * 48,000 *512=24576000, and the actual operating frequency of the
 * calibrated PLL is 196608000
 */
static void sys_pll_autocalib(int argc, char *argv[])
{
    uint32_t codec_freq;
    uint16_t val;

    if (argc != 2)
        goto usage;

    codec_freq = (uint32_t)atoi(argv[1]);
    eshell_putstring("  start pll_autocalib freq/%d\r\n", codec_freq);
    bbpll_auto_calib_ires(codec_freq, CODEC_PLL_DIV);
    pmu_read(0x20C4, &val);
    eshell_putstring("  calibed,PMU_REG_RF_C4/0x20C4=0x%x\r\n", val);
    eshell_putstring("  pll_autocalib finished.\r\n");
    return;
usage:
    sys_pll_autocalib_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "pll_autocalib", "usage: pll_autocalib help", sys_pll_autocalib);

#endif
#endif
#endif
