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
#if defined(CHIP_HAS_PSRAM) && (CHIP_PSRAM_PHY_VER == 6)

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
#include "reg_psram_phy_v6.h"
#include "string.h"
#include "psram_drv.h"
#include "psram_debug.h"
#include "hal_psramip.h"

static struct PSRAM_PHY_CFG_T psram_phy_cfg = {
    .phy_pd_trans_mask            = 0,
    .reg_ca_dq_lat_sw_en          = 0,
    .man_tx_wr_push_en            = 0,
    .auto_pwr_tx_wr_push_en       = 0,
    .idle_dq_oen                  = 1,
    .idle_dqs_oen                 = 1,
};

struct PSRAM_PHY_T  *const psram_phy[1] = {(struct PSRAM_PHY_T *)(SYS_PSRAM0_CTRL_BASE + 0x8000)};
// static const uint32_t psram_run_clk = PSRAM_SPEED;

/*****************************************************************************
********************************** PHY ***************************************
*****************************************************************************/
void hal_psram_phy_dll_reset_set_dr(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_050 |= PSRAM_PHY_REG_PSRAM_DLL_RESETB_DR;
    psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG_PSRAM_DLL_RESETB;
}

void hal_psram_phy_dll_reset_clear_dr(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_050 |= PSRAM_PHY_REG_PSRAM_DLL_RESETB_DR;
    psram_phy[id]->REG_058 |= PSRAM_PHY_REG_PSRAM_DLL_RESETB;
}

void hal_psram_phy_dll_clk_enable_dr(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_050 |= PSRAM_PHY_REG_PSRAM_DLL_CK_RDY_DR;
    psram_phy[id]->REG_058 |= PSRAM_PHY_REG_PSRAM_DLL_CK_RDY;
}

void hal_psram_phy_dll_clk_disable_dr(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_050 |= PSRAM_PHY_REG_PSRAM_DLL_CK_RDY_DR;
    psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG_PSRAM_DLL_CK_RDY;
}

void hal_psram_phy_dll_clk_rdy_dr_clear(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_050 &= ~PSRAM_PHY_REG_PSRAM_DLL_CK_RDY_DR;
}

void hal_psram_phy_dll_reset_dr_clear(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_050 &= ~PSRAM_PHY_REG_PSRAM_DLL_RESETB_DR;
}


uint32_t hal_psram_phy_get_init_range(uint32_t clk)
{
    uint32_t range;

    if (clk <= 150000000) {
        range = 3;
    } else if (clk <= 250000000) {
        range = 3;
    } else if (clk <= 350000000) {
        range = 1;
    } else {
        range = 0;
    }
    return range;
}

void hal_psram_phy_dll_wait_lock(void)
{

}

void hal_psram_anaphy_open_auto(void)
{
    psram_phy[0]->REG_040 =
        PSRAM_BIT_CONFIG(0, PSRAM_PHY_PHY_P2S_BYPASS) |
        PSRAM_BIT_CONFIG(0, PSRAM_PHY_PHY_ANA_BYPASS) |
        PSRAM_PHY_PHY_BYPASS_AUTO_LP_EN |
        PSRAM_PHY_MC_IDLE_AUTO_LP_EN;

    psram_phy[0]->REG_050 =
        PSRAM_BIT_CONFIG(0, PSRAM_PHY_REG_PSRAM_PHY_BYPASS_DR) |
        PSRAM_BIT_CONFIG(0,PSRAM_PHY_REG_PHY_PD_TRANS_MASK);
}

void hal_psram_phy_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    psram_cfg->phy_cfg = &psram_phy_cfg;

    psram_phy[id]->REG_040 =
        PSRAM_BIT_CONFIG(psram_cfg->phy_bypass == PHY_P2S_BYPASS, PSRAM_PHY_PHY_P2S_BYPASS) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_bypass == PHY_ANA_BYPASS, PSRAM_PHY_PHY_ANA_BYPASS) |
        PSRAM_PHY_PHY_BYPASS_AUTO_LP_EN |
        PSRAM_PHY_MC_IDLE_AUTO_LP_EN;

    psram_phy[id]->REG_050 =
        PSRAM_BIT_CONFIG(psram_cfg->phy_bypass == PHY_ANA_BYPASS, PSRAM_PHY_REG_PSRAM_PHY_BYPASS_DR) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->phy_pd_trans_mask,PSRAM_PHY_REG_PHY_PD_TRANS_MASK);

    psram_phy[id]->REG_020 =
        PSRAM_PHY_PAD_RSTN |
        PSRAM_PHY_CA_SEQ(psram_cfg->device_cfg->device_pattern) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->hyperbus_sample_lat_edge_sel, PSRAM_PHY_HYPERBUS_SAMPLE_LAT_EDGE_SEL) |
        PSRAM_PHY_HYPERBUS_CMD_DLY_CNT(psram_cfg->device_cfg->hyperbus_cmd_dly_cnt) |
        PSRAM_PHY_PAD_EN |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->reg_ca_dq_lat_sw_en, PSRAM_PHY_REG_CA_DQ_LAT_SW_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->man_tx_wr_push_en, PSRAM_PHY_MAN_TX_WR_PUSH_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->auto_pwr_tx_wr_push_en, PSRAM_PHY_AUTO_PWR_TX_WR_PUSH_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->clk_preample_en, PSRAM_PHY_CLK_PREAMPLE_EN) |
        PSRAM_PHY_HYPERBUS_TCSS(psram_cfg->device_cfg->hyperbus_tcss) |
        PSRAM_PHY_CFG_RX_DQS_SEL(2);

    psram_phy[id]->REG_004 =
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->phy_dummy_cyc_en, PSRAM_PHY_PHY_DUMMY_CYC_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->phy_dummy_cyc_stop_dqs, PSRAM_PHY_PHY_DUMMY_CYC_STOP_DQS) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->idle_dq_oen, PSRAM_PHY_IDLE_DQ_OEN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->idle_dqs_oen, PSRAM_PHY_IDLE_DQS_OEN) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->dqs_dm_merge_en, PSRAM_PHY_DQS_DM_MERGE_EN)|
        PSRAM_BIT_CONFIG(!psram_cfg->device_cfg->mrw_output_dm, PSRAM_PHY_MRW_WO_DM) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->cs_pol, PSRAM_PHY_CS_POL);
}

void hal_psram_phy_rx_timeout_enable(enum HAL_PSRAM_ID_T id)
{
}

void hal_psram_phy_rx_timeout_disable(enum HAL_PSRAM_ID_T id)
{
}


#endif
