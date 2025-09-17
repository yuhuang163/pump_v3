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
#ifdef PSRAM_DUAL_PORT_TEST

#include "plat_addr_map.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_mcu2cp.h"
#include "system_cp.h"
#include "hwtest.h"

static void CP_TEXT_SRAM_LOC NOINLINE cp_main_loop(void)
{
    hal_mcu2cp_open_cp(HAL_RMT_IPC_CHAN_0, NULL, NULL, true);

    hal_sys_timer_delay(MS_TO_TICKS(1000));
    psramuhs_test();
    while (1) {
        TRACE(0, "------------core1 mcu is running");
        hal_sys_timer_delay(MS_TO_TICKS(100));
    }
}

static void cp_start(void)
{
    system_cp_init(true);
    cp_main_loop();
}

void psram_dual_port_test(void)
{
    TRACE(0, "  ");
#if defined(CHIP_BEST1501)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    while (1) {
        TRACE(0, "***1501 chip does not support this test example");
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_390M);//The minimum frequency is 390M,2021-08-28
#endif
    TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    TRACE(0, "------psram dual port test started------");
    TRACE(0, "------psramulp runs on core0 and takes port 0");
    TRACE(0, "------psramuhs runs on core1 and takes port 1");
    TRACE(0, "  ");

    hal_cmu_cp_enable(RAMCP_BASE + RAMCP_SIZE, (uint32_t)cp_start);
    hal_mcu2cp_open_mcu(HAL_RMT_IPC_CHAN_0, NULL, NULL, true);

    psram_test();
    while (1) {
        TRACE(0, "------------core0 mcu is running");
        hal_sys_timer_delay(MS_TO_TICKS(100));
    }
}

#endif

