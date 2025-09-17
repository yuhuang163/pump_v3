/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#if (CHIP_PSRAM_CTRL_VER == 6)

#include "plat_types.h"
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "reg_psram_mc_v6.h"
#include "reg_psram_phy_v6.h"
#include "string.h"

#include "hal_psram.h"
#include "psram_timing.h"
#include "psram_drv.h"

BOOT_DATA_LOC static struct PSRAM_MC_CFG_T  psram_mc_cfg;
BOOT_DATA_LOC static struct PSRAM_PHY_CFG_T psram_phy_cfg;
BOOT_DATA_LOC static struct PSRAM_CFG_T psram_cfg_param;
static HAL_PSRAM_WINDOW_2D_CB_T hal_psram_window_2d_cb_func = NULL;

void psram_window_2d_cb_register(enum HAL_PSRAM_ID_T id, HAL_PSRAM_WINDOW_2D_CB_T func)
{
    hal_psram_window_2d_cb_func = func;
}
void hal_psram_window_2d_cb_register(enum HAL_PSRAM_ID_T id, HAL_PSRAM_WINDOW_2D_CB_T func) __attribute__((alias("psram_window_2d_cb_register")));

bool psram_window_2d_cb_exec(void * const array, uint8_t volume, uint8_t dly_inc, int byte_pos, const struct HAL_PSRAM_RECTANGLE_T *rectangle)
{
    if (hal_psram_window_2d_cb_func != NULL) {
        hal_psram_window_2d_cb_func(array, volume + 1, dly_inc, 0, rectangle);
        return true;
    }

    return false;
}

void psram_mr_setting(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->psram_type == PSRAM_TYPE_OPI) {
        psram_opi_mr_set(id, psram_cfg);
    } else if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        psram_xopi_mr_set(id, psram_cfg);
    } else { }
}

void SRAM_TEXT_LOC psram_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk)
{
    if (psram_cfg->psram_type == PSRAM_TYPE_OPI) {
        psram_opi_mr_get(psram_cfg, reg, val, clk);
    } else if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        psram_xopi_mr_get(psram_cfg, reg, val, clk);
    } else { }
}

void psram_dll_dly_init(const struct PSRAM_T *psram)
{
    uint16_t dly_in, dly;
    struct HLA_PSRAM_DLL_DLY_VAL_T v;

    dly_in = hal_psram_anaphy_get_dll_dly_in_val(0);
    dly = MIN(dly_in / 2, 0x1f);
    memset(&v, dly, sizeof(struct HLA_PSRAM_DLL_DLY_VAL_T));

    if (psram->psram_type[HAL_PSRAM_ID_0] == PSRAM_TYPE_XOPI) {
        if (psram->psram_data_bus[HAL_PSRAM_ID_0] == PSRAM_DATA_BUS_X8) {
            psram_xopi_set_dly(0, HAL_PSRAM_PHY_CHANNEL_0, &v);
        } else {
            psram_xopi_set_dly(0, HAL_PSRAM_PHY_CHANNEL_ALL, &v);
        }
    } else if (psram->psram_type[HAL_PSRAM_ID_0] == PSRAM_TYPE_OPI) {
        if (psram->psram_data_bus[HAL_PSRAM_ID_0] == PSRAM_DATA_BUS_X8) {
            psram_opi_set_dly(0, HAL_PSRAM_PHY_CHANNEL_0, &v);
        } else {
            psram_opi_set_dly(0, HAL_PSRAM_PHY_CHANNEL_ALL, &v);
        }
    } else {
        //assert
    }
}

void psram_anaphy_calib(const struct PSRAM_T *psram)
{
    if (psram->psram_type[HAL_PSRAM_ID_0] == PSRAM_TYPE_OPI) {
        if (psram->psram_data_bus[HAL_PSRAM_ID_0] == PSRAM_DATA_BUS_X8) {
            psram_opi_calib(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_0, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
        } else {
            psram_opi_calib(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_ALL, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
        }
    } else if (psram->psram_type[HAL_PSRAM_ID_0] == PSRAM_TYPE_XOPI) {
        if (psram->psram_data_bus[HAL_PSRAM_ID_0] == PSRAM_DATA_BUS_X8) {
            psram_xopi_calib(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_0, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
        } else {
            psram_xopi_calib(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_ALL, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
        }
    } else {
        ///assert
    }
}

void psram_anaphy_open(const struct PSRAM_T *psram)
{
    if (psram->phy_bypass_cfg[HAL_PSRAM_ID_0] == PHY_ANA_BYPASS) {
        hal_cmu_psram_phy_bypass_cfg();
        psramphy_ana_bypass_bg_config(PSRAM_ANAPHY_ID_0);
    } else {
        hal_cmu_psram_phy_init(1);
        // psram_anaphy_open_dr(id);
        hal_psram_anaphy_open_auto();
        psramphy_bg_config(PSRAM_ANAPHY_ID_0);
    }
}

void psram_open(enum HAL_PSRAM_ID_T id, const struct PSRAM_T *psram)
{
    memset(&psram_mc_cfg, 0, sizeof(psram_mc_cfg));
    memset(&psram_phy_cfg, 0, sizeof(psram_phy_cfg));
    memset(&psram_cfg_param, 0, sizeof(psram_cfg_param));

    psram_cfg_param.psram_type = psram->psram_type[id];
    psram_cfg_param.psram_data_bus = psram->psram_data_bus[id];
    psram_cfg_param.phy_bypass =  psram->phy_bypass_cfg[id];
    psram_cfg_param.cfg_freq_mhz = psram->cfg_freq_mhz;
    psram_cfg_param.run_freq_mhz = psram->run_freq_mhz[id];

    psram_struct_init(&psram_mc_cfg, &psram_phy_cfg, &psram_cfg_param);

#if (!defined(FPGA) && !defined(NO_ISPI))
    psram_anaphy_open(psram);
    hal_sys_timer_delay_us(1000);
    hal_psram_anaphy_dll_range_init(psram->cfg_freq_mhz * 1000000);

    psram_dll_dly_init(psram);
#endif

    hal_psram_phy_init(id, &psram_cfg_param);

    hal_psram_mc_init(id, &psram_cfg_param);

    hal_psram_mc_set_default_param(id, &psram_cfg_param);

    hal_psram_mc_set_timing(id, &psram_cfg_param, psram->cfg_freq_mhz, false);

    hal_psram_mc_init_complete(id);

    hal_psram_mc_set_cmd_nop(id);

    hal_psram_mc_set_cmd_start_clock(id);

    // 100 clc when clk =20MHz
    hal_sys_timer_delay_us(5);

    hal_psram_mc_set_cmd_stop_clock(id);

    hal_psram_mc_set_cmd_rst(id);

    psram_mr_setting(id, &psram_cfg_param);

    hal_psram_mc_set_timing(id, &psram_cfg_param, psram->run_freq_mhz[id], true);

    hal_psram_mc_enable_dyn_freq(id, &psram_cfg_param);

#ifdef PSRAM_DEBUG
    /// INFO: just for debug
    uint32_t md;
    PSRAM_TRACE(0, "Psram%d MR as follow", HAL_PSRAM_ID_0);
    md = hal_psram_device_mr_read(HAL_PSRAM_ID_0, 8);
    PSRAM_TRACE(0, "****MR8 = 0x%x", md);
    md = hal_psram_device_mr_read(HAL_PSRAM_ID_0, 0);
    PSRAM_TRACE(0, "****MR0 = 0x%x", md);
    md = hal_psram_device_mr_read(HAL_PSRAM_ID_0, 4);
    PSRAM_TRACE(0, "****MR4 = 0x%x", md);
#endif
}

void psram_struct_init(struct PSRAM_MC_CFG_T *mc_cfg, struct PSRAM_PHY_CFG_T *phy_cfg, struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->psram_type == PSRAM_TYPE_OPI) {
        PSRAM_TRACE(0, "**PSRAM_TYPE_OPI");
        psram_opi_struct_init(mc_cfg, phy_cfg, psram_cfg);
    }
    else if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        PSRAM_TRACE(0, "**PSRAM_TYPE_XOPI");
        psram_xopi_struct_init(mc_cfg, phy_cfg, psram_cfg);
    }
    else{}
}

void psram_calib(const struct PSRAM_T *psram)
{
    psram_anaphy_calib(psram);
}

static void psram_type_bus_init(struct PSRAM_T *psram)
{
    uint32_t type, bus;

    type = PSRAM_TYPE_XOPI;
    bus = PSRAM_DATA_BUS_2X16;

#if defined(PSRAM_CLASSIFY_XOPI)
    type = PSRAM_TYPE_XOPI;
#elif defined(PSRAM_CLASSIFY_HYPERBUS)
    type = PSRAM_TYPE_HYPERRAM;
#elif defined(PSRAM_CLASSIFY_OPI)
    type = PSRAM_TYPE_OPI;
#endif

#if defined(PSRAM_X8_MODE)
    bus = PSRAM_DATA_BUS_X8;
#elif defined(PSRAM_X16_MODE)
    bus = PSRAM_DATA_BUS_X16;
#elif defined(PSRAM_DUAL_8BIT)
    bus = PSRAM_DATA_BUS_2X8;
#elif defined(PSRAM_2X16_MODE)
    bus = PSRAM_DATA_BUS_2X16;
#endif

    psram->psram_type[HAL_PSRAM_ID_0] = type;
    psram->psram_data_bus[HAL_PSRAM_ID_0] = bus;
    PSRAM_TRACE(1, "psram cfg type = %d, data bus = %d", type, bus);
}

void psram_init(void)
{
    struct PSRAM_T psram;

    pmu_psram_freq_config(PSRAM_SPEED * 1000000);

    hal_psc_mem_enable();
    hal_cmu_psram_clock_enable();

    psram.cfg_freq_mhz = 24,
    psram.run_freq_mhz[HAL_PSRAM_ID_0] = PSRAM_SPEED,
    psram.psram_type[HAL_PSRAM_ID_0]= PSRAM_TYPE_XOPI,
    psram.psram_data_bus[HAL_PSRAM_ID_0] = PSRAM_DATA_BUS_2X16,
#if (defined(FPGA) || defined(NO_ISPI))
    psram.phy_bypass_cfg[HAL_PSRAM_ID_0] = PHY_ANA_BYPASS,
#else
    psram.phy_bypass_cfg[HAL_PSRAM_ID_0] = PHY_P2S_BYPASS,
#endif

    psram_type_bus_init(&psram);
    PSRAM_TRACE(1, "psram cfg clk = %d", psram.cfg_freq_mhz);
    PSRAM_TRACE(1, "psram run clk = %d", psram.run_freq_mhz[HAL_PSRAM_ID_0]);

    hal_psram_mc_clock_gate_disable(HAL_PSRAM_ID_0);

    PSRAM_TRACE(1,"PSRM0 Init ");
    // cfg_clK
    hal_psram_mc_set_freq(HAL_PSRAM_ID_0, psram.cfg_freq_mhz * 1000000);

    psram_open(HAL_PSRAM_ID_0, &psram);

    // run_clk
    hal_psram_mc_set_freq(HAL_PSRAM_ID_0, psram.run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);

#if (!defined(FPGA) && !defined(NO_ISPI))
    if (psram.phy_bypass_cfg[HAL_PSRAM_ID_0] != PHY_ANA_BYPASS) {
        psram_calib(&psram);
        // psram_dll_dly_init(&psram);
    }
#endif

    hal_psram_mc_priority_config(HAL_PSRAM_ID_0);
}

void SRAM_TEXT_LOC psram_wakeup_init(void)
{
    hal_cmu_psram_clock_disable();

    hal_cmu_psram_clock_enable();

    hal_psram_mc_clock_gate_disable(HAL_PSRAM_ID_0);

    hal_psram_phy_init(HAL_PSRAM_ID_0, &psram_cfg_param);

    hal_psram_mc_init(HAL_PSRAM_ID_0, &psram_cfg_param);

    hal_psram_mc_set_timing(HAL_PSRAM_ID_0, &psram_cfg_param, psram_cfg_param.run_freq_mhz, true);

    hal_psram_mc_init_complete(HAL_PSRAM_ID_0);

    hal_psram_mc_enable_dyn_freq(HAL_PSRAM_ID_0, &psram_cfg_param);

    hal_psram_mc_priority_config(HAL_PSRAM_ID_0);
}

#endif
