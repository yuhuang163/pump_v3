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
#if defined(CHIP_HAS_PSRAM) && (CHIP_PSRAM_PHY_VER == 10)

#include "plat_types.h"
#include "plat_addr_map.h"

#include "hal_timer.h"
#include "reg_psram_mc_v6.h"
#include "reg_psram_phy_v10.h"

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_debug.h"

#define PSRAM_MC_SWITCH
#define PSRAM_MC_WRITE_PRFRTCH
#define PSRAM_MC_READ_MERGE
#define PSRAM_MC_SNOOP_DISABLE

#define PSRAM_AUTO_LOW_POWER
#define PSRAM_CLOCK_GATE_ENABLE
#define PSRAM_DYN_FP_SUPPORT
#define PSRAM_QUEUE_IDLE_TIME_US            (20)

#ifdef PSRAM_DYN_FP_SUPPORT
#define PSRAM_SPEED_0                       PSRAM_SPEED
#define PSRAM_SPEED_1                       (48)
#endif


static struct PSRAM_PHY_T  *const psram_phy[1] = {(struct PSRAM_PHY_T *)(SYS_PSRAM0_CTRL_BASE + 0x8000)};

static struct PSRAM_PHY_CFG_T psram_phy_cfg = {
    .phy_pd_trans_mask            = 0,
    .reg_ca_dq_lat_sw_en          = 0,
    .man_tx_wr_push_en            = 0,
    .auto_pwr_tx_wr_push_en       = 0,
    .idle_dq_oen                  = 1,
    .idle_dqs_oen                 = 1,
};

/*****************************************************************************
********************************** PHY ***************************************
*****************************************************************************/
void hal_psram_anaphy_set_dll_dly(uint32_t range, uint32_t t_div_4)
{
    psram_phy[0]->REG_07C |=
        PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE(range) |
        PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE2(range) |
        PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE(range) |
        PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE2(range);

    psram_phy[0]->REG_05C |=
        PSRAM_PHY_REG0_PSRAM_TX_CEB1_DLY(t_div_4) |
        PSRAM_PHY_REG0_PSRAM_TX_CEB2_DLY(t_div_4) |
        PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY(t_div_4) |
        PSRAM_PHY_REG0_PSRAM_TX_CLK2_DLY(t_div_4) |
        PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE(range) |
        PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE2(range) |
        PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY(t_div_4) |
        PSRAM_PHY_REG0_PSRAM_TX_DQS2_DLY(t_div_4);

    psram_phy[0]->REG_064 |=
        PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE(range) |
        PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE2(range) |
        PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL(t_div_4) |
        PSRAM_PHY_REG0_PSRAM_RX_DELAY2_CTRL(t_div_4);
}

void hal_psram_anaphy_set_rxdqs_dly(uint32_t dly)
{
    psram_phy[0]->REG_064 |=
        PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL(dly) |
        PSRAM_PHY_REG0_PSRAM_RX_DELAY2_CTRL(dly);
}

int hal_psram_anaphy_calib(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val;
    uint32_t dly_div_4;
    uint32_t phase_check, phase_check1, phase_check2;

    dly_div_4 = 0;
    phase_check1 = 0;
    phase_check2 = 0;

    psram_phy[id]->REG_024 = PSRAM_PHY_REG_TR_CLK_EN | PSRAM_PHY_REG_TR_DQS_EN;
    psram_phy[id]->REG_050 =
        PSRAM_PHY_REG_PSRAM_CEB1_PU | PSRAM_PHY_REG_PSRAM_CEB2_PU |
        PSRAM_PHY_REG_PSRAM_PU | PSRAM_PHY_REG_PSRAM_PU2;

    // psram_phy[id]->REG_050 &= ~(PSRAM_PHY_REG_PSRAM_SLEEP_EN | PSRAM_PHY_REG_PSRAM_SLEEP_EN2);

    val = psram_phy[id]->REG_07C;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S1_DELAY_RANGE, 2);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE, 2);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S1_DLY, 0);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S2_DLY, 0);
    psram_phy[id]->REG_07C = val;

    for (int dly = 0; dly <= 0xf; dly++) {

        psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
        psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(1);

        val = psram_phy[id]->REG_07C;
        val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S1_DLY, dly);
        psram_phy[id]->REG_07C = val;
        // hal_sys_timer_delay_us(4);

        // psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(2);
        psram_phy[id]->REG_058 |= PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(2);
        psram_phy[id]->REG_058 |= PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
        hal_sys_timer_delay_us(2);

        PSRAM_TRACE(0, "s1 delay : %d", dly);
        phase_check = GET_BITFIELD(psram_phy[id]->REG_054, PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK);
        PSRAM_TRACE(0, "s1 phase_check : 0x%x", phase_check);

        if (phase_check1 == 0) {
            phase_check1 = phase_check;
        } else if (phase_check1 != phase_check) {
            phase_check2 = phase_check;
            break;
        }
    }

    for (int dly = 0; dly <= 0xf; dly++) {

        psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
        psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(1);

        val = psram_phy[id]->REG_07C;
        val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S2_DLY, dly);
        psram_phy[id]->REG_07C = val;
        hal_sys_timer_delay_us(2);

        // psram_phy[id]->REG_058 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
        // hal_sys_timer_delay_us(2);
        psram_phy[id]->REG_058 |= PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(2);
        psram_phy[id]->REG_058 |= PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
        hal_sys_timer_delay_us(2);

        PSRAM_TRACE(0, "s2 delay : %d", dly);
        phase_check = GET_BITFIELD(psram_phy[id]->REG_054, PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK);
        PSRAM_TRACE(0, "s2 phase_check : 0x%x", phase_check);

        if (phase_check2 == 0) {
            phase_check2 = phase_check;
        } else if (phase_check2 != phase_check) {
            dly_div_4 = dly;
            break;
        }
    }

    if (dly_div_4 == 0) {
        dly_div_4 = 0xF;
    }

    psram_phy[id]->REG_024 = 0;

    return dly_div_4;

}

void hal_psram_anaphy_dly_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t range;
    uint32_t dly;

    range = GET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE);
    dly = GET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DLY);
    PSRAM_TRACE(0, "psram phy range : %d, T/4 : %d", range, dly);

    hal_psram_anaphy_set_dll_dly(range, dly);
}

void hal_psram_phy_dll_dly_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val;

    do {
        val = GET_BITFIELD(psram_phy[id]->REG_044, PSRAM_PHY_PHY_AUTO_LP_STATE);
    } while (val != 0x7);

    hal_psram_anaphy_calib(id, psram_cfg);
    hal_psram_anaphy_dly_init(id, psram_cfg);
}

void hal_psram_phy_anaphy_open(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    psram_phy[id]->REG_040 =
        PSRAM_BIT_CONFIG(psram_cfg->phy_bypass == PHY_P2S_BYPASS, PSRAM_PHY_PHY_P2S_BYPASS) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_bypass == PHY_ANA_BYPASS, PSRAM_PHY_PHY_ANA_BYPASS) |
        PSRAM_PHY_PHY_BYPASS_AUTO_LP_EN |
        PSRAM_PHY_MC_IDLE_AUTO_LP_EN;
}

void hal_psram_phy_digphy_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    psram_cfg->phy_cfg = &psram_phy_cfg;

    if (psram_cfg->phy_bypass == PHY_ANA_BYPASS) {
        psram_phy[id]->REG_040 |=
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL(1) |
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL1(1);
    } else {
        psram_phy[id]->REG_040 |=
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL(4) |
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL1(4);
    }

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
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->device_preample_en, PSRAM_PHY_CA_PREAMPLE_EN) |
        PSRAM_PHY_HYPERBUS_TCSS(psram_cfg->device_cfg->hyperbus_tcss);

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

/*****************************************************************************
*********************************** hal and device ***************************
*****************************************************************************/
void hal_psram_phy_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    hal_psram_phy_anaphy_open(id, psram_cfg);
    hal_psram_phy_digphy_init(id, psram_cfg);
}

void hal_psram_phy_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    hal_cmu_psram_phy_sleep();
}

void hal_psram_phy_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    // int dual;
    // if (hal_psram_mc_is_reset(HAL_PSRAM_ID_0)) {
    //     dual = 1;
    // } else if (psram_mc[0]->REG_000 & (PSRAM_MC_CHIP_BIT | PSRAM_MC_CHIP_X16)) {
    //     dual = 1;
    // } else {
    //     dual = 0;
    // }

    // hal_cmu_psram_phy_wakeup(dual);
}
#endif

