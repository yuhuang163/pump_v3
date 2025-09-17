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
#include <stdio.h>
#include <stdarg.h>
#include "plat_types.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "eshell.h"
#include "app_eshell.h"
#include "app_utils.h"
#include "pmu.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"

#define FREQ_32K    HAL_CMU_FREQ_32K

#if defined(CHIP_BEST1502X) || defined(CHIP_BEST1503)
#define FREQ_13M    HAL_CMU_FREQ_13M
#define FREQ_26M    HAL_CMU_FREQ_26M
#define FREQ_52M    HAL_CMU_FREQ_52M
#define FREQ_78M    HAL_CMU_FREQ_78M
#define FREQ_104M   HAL_CMU_FREQ_104M
#define FREQ_208M   HAL_CMU_FREQ_208M
#endif

#if defined(CHIP_BEST1600) || defined(CHIP_BEST1700)
#define FREQ_12M    HAL_CMU_FREQ_12M
#define FREQ_24M    HAL_CMU_FREQ_24M
#define FREQ_48M    HAL_CMU_FREQ_48M
#define FREQ_72M    HAL_CMU_FREQ_72M
#define FREQ_96M    HAL_CMU_FREQ_96M
#define FREQ_104M   HAL_CMU_FREQ_104M
#define FREQ_192M   HAL_CMU_FREQ_192M
#define FREQ_208M   HAL_CMU_FREQ_208M
#endif

static uint32_t cpu_freq_get(void)
{
    enum HAL_CMU_FREQ_T freq = hal_sysfreq_get();
    uint32_t val = 0;

    if (freq == FREQ_32K)
        val = 32000;
#ifdef FREQ_6M
    else if (freq == FREQ_6M)
        val = 6000000;
#endif
#ifdef FREQ_12M
    else if (freq == FREQ_12M)
        val = 12000000;
#endif
#ifdef FREQ_13M
    else if (freq == FREQ_13M)
        val = 13000000;
#endif
#ifdef FREQ_24M
    else if (freq == FREQ_24M)
        val = 24000000;
#endif
#ifdef FREQ_26M
    else if (freq == FREQ_26M)
        val = 26000000;
#endif
#ifdef FREQ_48M
    else if (freq == FREQ_48M)
        val = 48000000;
#endif
#ifdef FREQ_52M
    else if (freq == FREQ_52M)
        val = 52000000;
#endif
#ifdef FREQ_72M
    else if (freq == FREQ_72M)
        val = 72000000;
#endif
#ifdef FREQ_78M
    else if (freq == FREQ_78M)
        val = 78000000;
#endif
#ifdef FREQ_96M
    else if (freq == FREQ_96M)
        val = 96000000;
#endif
#ifdef FREQ_104M
    else if (freq == FREQ_104M)
        val = 104000000;
#endif
#ifdef FREQ_192M
    else if (freq == FREQ_192M)
        val = 192000000;
#endif
#ifdef FREQ_208M
    else if (freq == FREQ_208M)
        val = 208000000;
#endif
    else
        val = 0;

    return val;
}

static void cpu_freq_set(uint32_t freq_val)
{
    enum HAL_CMU_FREQ_T freq = FREQ_32K;

    switch (freq_val) {
        case 32000:
            freq = FREQ_32K;
            break;
#ifdef FREQ_24M
        case 24000000:
            freq = FREQ_24M;
            break;
#endif
#ifdef FREQ_26M
        case 26000000:
            freq = FREQ_26M;
            break;
#endif
#ifdef FREQ_48M
        case 48000000:
            freq = FREQ_48M;
            break;
#endif
#ifdef FREQ_52M
        case 52000000:
            freq = FREQ_52M;
            break;
#endif
#ifdef FREQ_78M
        case 78000000:
            freq = FREQ_78M;
            break;
#endif
#ifdef FREQ_96M
        case 96000000:
            freq = FREQ_96M;
            break;
#endif
#ifdef FREQ_104M
        case 104000000:
            freq = FREQ_104M;
            break;
#endif
#ifdef FREQ_192M
        case 192000000:
            freq = FREQ_192M;
            break;
#endif
#ifdef FREQ_208M
        case 208000000:
            freq = FREQ_208M;
            break;
#endif
        default:
            break;
    }

    hal_sysfreq_req(APP_SYSFREQ_USER_ESHELL, freq);
}

static void sysfreq_cmd_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  sysfreq get\r\n");
    eshell_putstring("  sysfreq set <32K|24M|26M|48M|52M|78M|96M|104M|192M|208M>\r\n");
}

static void do_sysfreq_cmd(int argc, char *argv[])
{
    uint32_t freq;

    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "get", 3) == 0) {
        freq = cpu_freq_get();
        eshell_putstring("CPU current freq: %u\r\n", freq);

    } else if (strncmp(argv[1], "set", 3) == 0) {

        if (argc < 3)
            goto usage;

        if (strncmp(argv[2], "32K", 3) == 0)
            freq = 32000;
        else if (strncmp(argv[2], "24M", 3) == 0)
            freq = 24000000;
        else if (strncmp(argv[2], "26M", 3) == 0)
            freq = 26000000;
        else if (strncmp(argv[2], "48M", 3) == 0)
            freq = 48000000;
        else if (strncmp(argv[2], "52M", 3) == 0)
            freq = 52000000;
        else if (strncmp(argv[2], "78M", 3) == 0)
            freq = 78000000;
        else if (strncmp(argv[2], "96M", 3) == 0)
            freq = 96000000;
        else if (strncmp(argv[2], "104M", 4) == 0)
            freq = 104000000;
        else if (strncmp(argv[2], "192M", 4) == 0)
            freq = 192000000;
        else if (strncmp(argv[2], "208M", 4) == 0)
            freq = 208000000;
        else
            goto usage;

        if (freq > 1000000)
            eshell_putstring("Set CPU freq to %uMhz\r\n",
                    freq / 1000 / 1000);
        else
            eshell_putstring("Set CPU freq to %uKhz\r\n",
                    freq / 1000);
        cpu_freq_set(freq);

    } else {
        goto usage;
    }

    freq = hal_sys_timer_calc_cpu_freq(5, 0);
    eshell_putstring("CPU real freq: %u\n", freq);
    return;
usage:
    sysfreq_cmd_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "sysfreq", "Set and get CPU frequency", do_sysfreq_cmd);

#endif // UTILS_ESHELL_EN
