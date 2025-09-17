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
#ifdef PVT_TEST
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_psc.h"
#include "hal_sysfreq.h"

// make -j T=noapp_test BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CURRENT_TEST=1 CHIP=best1603 DEBUG_PORT=1 BTH_USE_SYS_PERIPH=1 MCU_I2C_SLAVE=1 PVT_TEST=1

#define PVT_TEST_CNT                                            10
#define PVT_TEST_TIME_ONE_SHOT_US                               99

struct PVT_CNT_BUF_AVG_T {
    uint32_t sys_cmu;
    uint32_t bth_cmu;
    uint32_t btc_cmu;
    uint32_t sens_cmu;
    uint32_t aon_cmu;
};

static uint32_t pvt_cnt_buf[PVT_TEST_CNT] = {0, };
static struct PVT_CNT_BUF_AVG_T pvt_avg = {0, };

static uint32_t pvt_test_cnt_avg_get(void)
{
    uint32_t pvg_cnt_total = 0;
    uint32_t pvg_cnt_avg = 0;

    for (int i = 0; i < PVT_TEST_CNT; i++) {
        pvg_cnt_total += pvt_cnt_buf[i];
    }
    pvg_cnt_avg = (pvg_cnt_total / PVT_TEST_CNT);

    return pvg_cnt_avg;
}

static void pvt_test_btc_power_on(void)
{
    static bool opened = false;

    if (!opened) {
        hal_psc_bt_enable();
        hal_cmu_bt_clock_enable();
        hal_cmu_bt_sys_reset_clear();
        opened = !opened;
    }
}

static void pvt_test_all_domain(uint32_t time_us)
{
    int i;

    pvt_test_btc_power_on();

    // BTH
    for (i = 0; i < PVT_TEST_CNT; i++) {
        hal_cmu_bth_pvt_enable(time_us);
        hal_sys_timer_delay_us(time_us + 1);
        pvt_cnt_buf[i] = hal_cmu_bth_pvt_cnt_get();
        hal_cmu_bth_pvt_disable();
    }
    pvt_avg.bth_cmu = pvt_test_cnt_avg_get();

    // SYS
    for (i = 0; i < PVT_TEST_CNT; i++) {
        hal_cmu_sys_pvt_enable(time_us);
        hal_sys_timer_delay_us(time_us + 1);
        pvt_cnt_buf[i] = hal_cmu_sys_pvt_cnt_get();
        hal_cmu_sys_pvt_disable();
    }
    pvt_avg.sys_cmu = pvt_test_cnt_avg_get();

    // BTC
    for (i = 0; i < PVT_TEST_CNT; i++) {
        hal_cmu_btc_pvt_enable(time_us);
        hal_sys_timer_delay_us(time_us + 1);
        pvt_cnt_buf[i] = hal_cmu_btc_pvt_cnt_get();
        hal_cmu_btc_pvt_disable();
    }
    pvt_avg.btc_cmu = pvt_test_cnt_avg_get();

    // SENS
    for (i = 0; i < PVT_TEST_CNT; i++) {
        hal_cmu_sens_pvt_enable(time_us);
        hal_sys_timer_delay_us(time_us + 1);
        pvt_cnt_buf[i] = hal_cmu_sens_pvt_cnt_get();
        hal_cmu_sens_pvt_disable();
    }
    pvt_avg.sens_cmu = pvt_test_cnt_avg_get();

    // AON
    for (i = 0; i < PVT_TEST_CNT; i++) {
        hal_cmu_aon_pvt_enable(time_us);
        hal_sys_timer_delay_us(time_us + 1);
        pvt_cnt_buf[i] = hal_cmu_aon_pvt_cnt_get();
        hal_cmu_aon_pvt_disable();
    }
    pvt_avg.aon_cmu = pvt_test_cnt_avg_get();

    TRACE(0, "[PVT SYS ] cnt=%d/%dus", pvt_avg.sys_cmu, time_us + 1);
    TRACE(0, "[PVT BTH ] cnt=%d/%dus", pvt_avg.bth_cmu, time_us + 1);
    TRACE(0, "[PVT BTC ] cnt=%d/%dus", pvt_avg.btc_cmu, time_us + 1);
    TRACE(0, "[PVT SENS] cnt=%d/%dus", pvt_avg.sens_cmu, time_us + 1);
    TRACE(0, "[PVT AON ] cnt=%d/%dus", pvt_avg.aon_cmu, time_us + 1);
}

void pvt_test(void)
{
    uint32_t test_cnt = 0;
    bool high_freq = false;

    TRACE(0, "PVT TEST START");

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
    TRACE(0, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));
    while(1) {
        if (high_freq) {
            hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
        } else {
            hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
        }
        high_freq = !high_freq;
        TRACE(0, "%s cnt=%d CPU freq: %u", __func__, test_cnt++, hal_sys_timer_calc_cpu_freq(5, 0));
        pvt_test_all_domain(PVT_TEST_TIME_ONE_SHOT_US);
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }
}
#endif
