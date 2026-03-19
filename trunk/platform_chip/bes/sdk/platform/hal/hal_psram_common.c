/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE) && (CHIP_PSRAM_CTRL_VER >= 10)

#include "plat_types.h"
#include "plat_addr_map.h"

#include "cmsis.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_l2cache.h"
#include "pmu.h"
#include "string.h"

#include "psram_drv.h"
#include "psram_debug.h"

static SRAM_BSS_LOC struct PSRAM_CFG_T psram_cfg_param;

void SRAM_TEXT_LOC hal_psram_wakeup_init(void)
{
    hal_psram_mc_clock_gate_disable(HAL_PSRAM_ID_0);

    hal_psram_phy_init(HAL_PSRAM_ID_0, &psram_cfg_param);

    hal_psram_mc_init(HAL_PSRAM_ID_0, &psram_cfg_param);

    hal_psram_mc_set_timing(HAL_PSRAM_ID_0, &psram_cfg_param, psram_cfg_param.run_freq_mhz, true);

    hal_psram_mc_init_complete(HAL_PSRAM_ID_0);

    hal_psram_mc_enable_dyn_freq(HAL_PSRAM_ID_0, &psram_cfg_param);

    hal_psram_mc_priority_config(HAL_PSRAM_ID_0);
#ifdef CHIP_BEST1502P
    if (psram_cfg_param.psram_type == PSRAM_TYPE_HYPERRAM)
        hal_psram_mc_set_cmd_nop(HAL_PSRAM_ID_0);
#endif
}

void hal_psram_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        hal_psram_phy_param_save(HAL_PSRAM_ID_0);
        hal_psram_mc_param_save(HAL_PSRAM_ID_0);
    }
#endif

    if (hal_psram_mc_is_sleep(HAL_PSRAM_ID_0) == true)
        return;

    hal_psram_mc_sleep(mode);
    psram_device_sleep(&psram_cfg_param);
    hal_psram_phy_sleep(mode);
}

void hal_psram_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD && hal_psram_mc_is_reset(HAL_PSRAM_ID_0)) {
        hal_psram_wakeup_init();
        hal_psram_mc_param_recover(HAL_PSRAM_ID_0);
        hal_psram_phy_param_recover(HAL_PSRAM_ID_0);
    }
#endif

    hal_psram_phy_wakeup(mode);
    hal_psram_mc_wakeup(mode);
}

int hal_psram_open(enum HAL_PSRAM_ID_T id, const struct PSRAM_T *psram)
{
    memset(&psram_cfg_param, 0, sizeof(psram_cfg_param));
    psram_cfg_param.psram_type = psram->psram_type[id];
    psram_cfg_param.psram_die_name = psram->psram_die_name[id];
    psram_cfg_param.psram_data_bus = psram->psram_data_bus[id];
    psram_cfg_param.phy_bypass =  psram->phy_bypass_cfg[id];
    psram_cfg_param.cfg_freq_mhz = psram->cfg_freq_mhz;
    psram_cfg_param.run_freq_mhz = psram->run_freq_mhz[id];

    hal_psram_mc_set_freq(id, psram->cfg_freq_mhz * 1000000);

    hal_psram_mc_clock_gate_disable(id);
    hal_psram_mc_auto_lowpower_disable(id);
    hal_psram_mc_snoop_disable(id);

    psram_struct_init(&psram_cfg_param);

    hal_psram_phy_init(id, &psram_cfg_param);
    PSRAM_TRACE(0, "****psram phy power up");

    hal_psram_mc_init(id, &psram_cfg_param);

    hal_psram_mc_set_default_param(id, &psram_cfg_param);

    hal_psram_mc_set_timing(id, &psram_cfg_param, psram->cfg_freq_mhz, false);

    hal_psram_mc_init_complete(id);
    PSRAM_TRACE(0, "****psram mc init complete");

    hal_psram_mc_set_cmd_nop(id);

    hal_psram_mc_set_cmd_start_clock(id);

    // 100 clc when clk =20MHz
    hal_sys_timer_delay_us(5);

    hal_psram_mc_set_cmd_stop_clock(id);

#if !defined(FPGA) && !defined(SIMU)
    hal_psram_phy_dll_dly_init(id, &psram_cfg_param);
#endif

    psram_reset(id, &psram_cfg_param);

#ifdef PSRAM_POLLING
    if (psram_die_check(id, &psram_cfg_param)) {
        return 1;
    }
#endif

    psram_mr_setting(id, &psram_cfg_param);

    hal_psram_mc_set_timing(id, &psram_cfg_param, psram->run_freq_mhz[id], true);

    // run_clk
    hal_psram_mc_set_freq(id, psram->run_freq_mhz[id] * 1000000);

    hal_psram_mc_enable_dyn_freq(id, &psram_cfg_param);

#if !defined(FPGA) && !defined(SIMU)
    hal_psram_phy_dll_dly_init(id, &psram_cfg_param);
    if (psram->phy_bypass_cfg[id] != PHY_ANA_BYPASS) {
        psram_calib(psram);
    }
#endif

    hal_psram_mc_priority_config(id);

#ifdef PSRAM_DEBUG
    psram_mr_show(id, &psram_cfg_param);
#endif

    hal_psram_mc_auto_lowpower_enable(id);
    hal_psram_mc_clock_gate_enable(id);

    return 0;
}

void hal_psram_cache_init()
{
    hal_cache_wrap_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_wrap_enable(HAL_CACHE_ID_D_CACHE);
#ifdef PSRAM_CACHE_DOUBLE_LINEFILL
    hal_cache_double_linefill_enable(HAL_CACHE_ID_D_CACHE);
#endif
}

void hal_psram_init(void)
{
    struct PSRAM_T psram;

    psram.cfg_freq_mhz = 24,
    psram.run_freq_mhz[HAL_PSRAM_ID_0] = PSRAM_SPEED,
    PSRAM_TRACE(1, "psram cfg clk = %d", psram.cfg_freq_mhz);
    PSRAM_TRACE(1, "psram run clk = %d", psram.run_freq_mhz[HAL_PSRAM_ID_0]);

    hal_psram_cache_init();
    pmu_psram_freq_config(PSRAM_SPEED * 1000000);

#ifdef PSRAM_POLLING
    do {
        hal_cmu_psram_clock_enable();

        if (psram_type_bus_init(&psram)) {
            PSRAM_ASSERT(false, "can not find the appropriate package type");
        }

        if (hal_psram_open(HAL_PSRAM_ID_0, &psram)) {
            hal_cmu_psram_clock_disable();
            PSRAM_TRACE(0, "package error, continue polling");
            continue;
        } else {
            break;
        }
     } while (1);
#else
    hal_cmu_psram_clock_enable();
    psram_type_bus_init(&psram);
    hal_psram_open(HAL_PSRAM_ID_0, &psram);
#endif
}

#endif
