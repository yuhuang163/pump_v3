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
#ifdef PSRAMUHS_WINDOW_TEST

#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "psramphy.h"

#ifndef PSRAMUHS_ENABLE
#error "PSRAMUHS_ENABLE not defined"
#endif

extern void hal_psramuhs_snoop_disable();

void psramuhs_window_test(void)
{
    TRACE(0, "  ");
#if defined(CHIP_BEST1501)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    while (1) {
        TRACE(0, "***1501 chip does not support this test example temporarily");
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_390M);//The minimum frequency is 390M,2021-08-28
#endif
    TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    TRACE(0, "------psramuhs window test started------");
    TRACE(0, "At present, only 2003 chips, other chips to be supported");
    TRACE(0, "  ");

    hal_psramuhs_snoop_disable();
    while (1) {
        psramphy_calib(PSRAMUHS_SPEED * 1000 * 1000);
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
}

#endif

