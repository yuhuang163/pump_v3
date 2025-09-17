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
#ifdef TEST_FREQ_SW_TIME
#include "hal_trace.h"
#include "cmsis_os.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"


#define DNAME(val)      (#val)
static char * freq_str_table[] = {
    DNAME(HAL_CMU_FREQ_32K),
    DNAME(HAL_CMU_FREQ_24M),
    DNAME(HAL_CMU_FREQ_48M),
    DNAME(HAL_CMU_FREQ_96M),
    DNAME(HAL_CMU_FREQ_144M),
    DNAME(HAL_CMU_FREQ_192M),
};

static enum HAL_CMU_FREQ_T freq_table[] = {
    HAL_CMU_FREQ_32K,
    HAL_CMU_FREQ_24M,
    HAL_CMU_FREQ_48M,
    HAL_CMU_FREQ_96M,
    HAL_CMU_FREQ_144M,
    HAL_CMU_FREQ_192M
};


void test_freq_sw_time(void)
{
    uint32_t time0, time1;
    TRACE(2,"%s start.", __FUNCTION__);
    for (uint32_t i = 0; i < ARRAY_SIZE(freq_table); i++) {
        for (uint32_t j = 0; j < ARRAY_SIZE(freq_table); j++) {
            hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, freq_table[i]);
            osDelay(10);
            time0 = hal_fast_sys_timer_get();
            hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, freq_table[j]);
            time1 = hal_fast_sys_timer_get();
            TRACE(0, "switch    %-17s to %-17s, time cost %4d us", freq_str_table[i], freq_str_table[j], FAST_TICKS_TO_US(time1-time0));
        }
        
    }


}
#endif
