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
#include "hal_trace.h"
#include "hal_timer.h"
#include "cmsis.h"

#ifdef RTOS
#include "cmsis_os.h"

#define DELAY_MS_DEFAULT                       (1000)

void test_ostick_and_timestamp(void)
{
    /*
        aco_timer: ACO timer[2][0], 32kHz
        fast_timer: BTH timer[1][0], fast clk source
        sys_timer: BTH timer[0][0], 32kHz
    */
    uint32_t aco_curr_t = hal_aco_timer_get();
    uint32_t fast_curr_t = hal_fast_sys_timer_get();
    uint32_t sys_curr_t = hal_sys_timer_get();

    TRACE(3,"%s sysTick [clk src is 16kHz in FPGA] start, and osDelay[%dms]!!!", __FUNCTION__, DELAY_MS_DEFAULT);
    osDelay(DELAY_MS_DEFAULT / 2);
    
    uint32_t aco_tick = hal_aco_timer_get() - aco_curr_t;
    uint32_t fast_tick = hal_fast_sys_timer_get() - fast_curr_t;
    uint32_t sys_tick = hal_sys_timer_get() - sys_curr_t;

    TRACE(3,"%s aco_timer[%dms] fast_timer clk[%dms] sys_timer clk[%dms]", __FUNCTION__,
                    TICKS_TO_MS(aco_tick),
                    FAST_TICKS_TO_MS(fast_tick),
                    TICKS_TO_MS(sys_tick)
                );
    TRACE(3,"%s sysTick test exit!!!", __FUNCTION__);
}
#endif

