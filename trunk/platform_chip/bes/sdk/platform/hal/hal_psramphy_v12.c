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
#if defined(CHIP_HAS_PSRAM) && (CHIP_PSRAM_PHY_VER == 12)

#include "plat_types.h"
#include "plat_addr_map.h"

#include "hal_timer.h"
#include "reg_psram_phy_v12.h"

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_debug.h"

enum PSRAM_PHY_LP_STATE {
    PSRAM_PHY_LP_IDLE             = 0x00,
    PSRAM_PHY_LP_DOWN_WAIT        = 0x01,
    PSRAM_PHY_LP_UP0              = 0x02,
    PSRAM_PHY_LP_DOWN             = 0x03,
    PSRAM_PHY_LP_UP               = 0x04,
    PSRAM_PHY_LP_UP3              = 0x05,
    PSRAM_PHY_LP_UP1              = 0x06,
    PSRAM_PHY_LP_UP2              = 0x07,
};

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
    uint32_t val;

    val = psram_phy[0]->REG_07C;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE, range);
    psram_phy[0]->REG_07C = val;

    val = psram_phy[0]->REG_064;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CEB1_DLY, t_div_4);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CEB2_DLY, t_div_4);

    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY, t_div_4);

    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY, t_div_4);

    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL, t_div_4);
    psram_phy[0]->REG_064 = val;
}

void hal_psram_anaphy1_set_dll_dly(uint32_t range, uint32_t t_div_4)
{
    uint32_t val;

    val = psram_phy[0]->REG_07C;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE_1, range);
    psram_phy[0]->REG_07C = val;

    val = psram_phy[0]->REG_064;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CEB1_DLY_1, t_div_4);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CEB2_DLY_1, t_div_4);
    psram_phy[0]->REG_064 = val;

    val = psram_phy[0]->REG_058;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE_1, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY_1, t_div_4);

    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE_1, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY_1, t_div_4);

    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE_1, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL_1, t_div_4);
    psram_phy[0]->REG_058 = val;
}

void hal_psram_anaphy_set_rxdqs_dly(uint32_t dly)
{
    uint32_t val;

    val = psram_phy[0]->REG_064;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL, dly);
    psram_phy[0]->REG_064 = val;
}

void hal_psram_anaphy1_set_rxdqs_dly(uint32_t dly)
{
    uint32_t val;

    val = psram_phy[0]->REG_058;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL_1, dly);
    psram_phy[0]->REG_058 = val;
}

int hal_psram_anaphy_calib(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t * dly_div_4, uint32_t range)
{
    uint32_t val;
    uint32_t phase_check, phase_check1, phase_check2;
    uint32_t phase_check_1, phase_check1_1, phase_check2_1;
    bool phy_calib_flag, phy_calib_flag_1;

    // range = 2;
    PSRAM_TRACE(0, "psram anaphy calib range: %d", range);
    dly_div_4[0] = 0;
    dly_div_4[1] = 0;
    phase_check1 = 0;
    phase_check2 = 0;
    phase_check1_1 = 0;
    phase_check2_1 = 0;
    phy_calib_flag = true;
    phy_calib_flag_1 = true;

    psram_phy[id]->REG_024 = PSRAM_PHY_REG_TR_CLK_EN | PSRAM_PHY_REG_TR_DQS_EN;
    psram_phy[id]->REG_050 |=
        PSRAM_PHY_REG0_PSRAM_CEB1_PU | PSRAM_PHY_REG0_PSRAM_CEB1_PU_1 |
        PSRAM_PHY_REG0_PSRAM_CEB2_PU | PSRAM_PHY_REG0_PSRAM_CEB2_PU_1;

    psram_phy[id]->REG_05C |= PSRAM_PHY_REG0_PSRAM_PU | PSRAM_PHY_REG0_PSRAM_PU_1;
    psram_phy[id]->REG_05C &= ~(PSRAM_PHY_REG0_PSRAM_SLEEP_EN | PSRAM_PHY_REG0_PSRAM_SLEEP_EN_1);

    val = psram_phy[id]->REG_07C;
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S1_DELAY_RANGE, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S1_DELAY_RANGE_1, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE_1, range);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S1_DLY, 0);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S2_DLY, 0);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S1_DLY_1, 0);
    val = SET_BITFIELD(val, PSRAM_PHY_REG0_PSRAM_S2_DLY_1, 0);
    psram_phy[id]->REG_07C = val;

    for (int dly = 0; dly <= 0x7; dly++) {
        //find psram phy0 s1 delay
        if (phy_calib_flag) {
            //pull down psram phy0 cal_clk_en and div2_resetb
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
            hal_sys_timer_delay_us(1);

            //updata phy0 s1 delay
            psram_phy[id]->REG_07C = SET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S1_DLY, dly);
            hal_sys_timer_delay_us(2);

            //pull up phy0 div2 resetb/cal clk en
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
            hal_sys_timer_delay_us(2);
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
            hal_sys_timer_delay_us(2);

            phase_check = GET_BITFIELD(psram_phy[id]->REG_054, PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK);
            PSRAM_TRACE(0, "s1 phase_check: 0x%x, s1 delay: %d", phase_check, dly);

            if (phase_check1 == 0) {
                phase_check1 = phase_check;
            } else if (phase_check1 != phase_check) {
                phase_check2 = phase_check;
                phy_calib_flag = false;
            }
        }

        //find psram phy1 s1 delay
        if (phy_calib_flag_1) {
            //pull down psram phy1 cal_clk_en and div2_resetb
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN_1;
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB_1;
            hal_sys_timer_delay_us(1);

            //updata phy1 s1 delay
            psram_phy[id]->REG_07C = SET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S1_DLY_1, dly);
            hal_sys_timer_delay_us(2);

            //pull up phy1 div2 resetb/cal clk en
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_DIV2_RESETB_1;
            hal_sys_timer_delay_us(2);
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN_1;
            hal_sys_timer_delay_us(2);

            phase_check_1 = GET_BITFIELD(psram_phy[id]->REG_054, PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK_1);
            PSRAM_TRACE(0, "s1 phase_check_1: 0x%x, s1 delay: %d", phase_check_1, dly);

            if (phase_check1_1 == 0) {
                phase_check1_1 = phase_check_1;
            } else if (phase_check1_1 != phase_check_1) {
                phase_check2_1 = phase_check_1;
                phy_calib_flag_1 = false;
            }
        }

        if (!phy_calib_flag && !phy_calib_flag_1) {
            break;
        }
    }

    if (phy_calib_flag || phy_calib_flag_1) {
        PSRAM_TRACE(0, "s1 delay: the range value may be too small, range = %d", range);
        if (range < 3) {
            return 1;
        } else {
            dly_div_4[0] = 0x7;
            dly_div_4[1] = 0x7;
            PSRAM_TRACE(0, "phy0 range: 0x%x, T/4 value: %d", range, dly_div_4[0]);
            PSRAM_TRACE(0, "phy1 range: 0x%x, T/4 value: %d", range, dly_div_4[1]);
            return 0;
        }
    }

    phy_calib_flag = true;
    phy_calib_flag_1 = true;

    for (int dly = 0; dly <= 0x7; dly++) {
        //find psram phy0 s2 delay
        if (phy_calib_flag) {
            //pull down psram_cal_clk_en and psram_div2_resetb
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
            hal_sys_timer_delay_us(1);

            //updata phy0 s2 delay
            psram_phy[id]->REG_07C = SET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DLY, dly);
            hal_sys_timer_delay_us(2);

            //pull up phy0 div2_resetb/cal_clk_en
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_DIV2_RESETB;
            hal_sys_timer_delay_us(2);
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN;
            hal_sys_timer_delay_us(2);

            phase_check = GET_BITFIELD(psram_phy[id]->REG_054, PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK);
            PSRAM_TRACE(0, "s2 phase_check: 0x%x, s2 delay: %d", phase_check, dly);

            if (phase_check2 == 0) {
                phase_check2 = phase_check;
            } else if (phase_check2 != phase_check) {
                dly_div_4[0] = dly;
                phy_calib_flag = false;
            }
        }

        //find psram phy1 s2 delay
        if (phy_calib_flag_1) {
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN_1;
            psram_phy[id]->REG_060 &= ~PSRAM_PHY_REG0_PSRAM_DIV2_RESETB_1;
            hal_sys_timer_delay_us(1);

            //updata phy1 s2 delay
            psram_phy[id]->REG_07C = SET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DLY_1, dly);
            hal_sys_timer_delay_us(2);

            //pull up phy1 div2_resetb/cal_clk_en
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_DIV2_RESETB_1;
            hal_sys_timer_delay_us(2);
            psram_phy[id]->REG_060 |= PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN_1;
            hal_sys_timer_delay_us(2);

            phase_check_1 = GET_BITFIELD(psram_phy[id]->REG_054, PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK_1);
            PSRAM_TRACE(0, "s2 phase_check_1: 0x%x, s2 delay: %d", phase_check_1, dly);

            if (phase_check2_1 == 0) {
                phase_check2_1 = phase_check_1;
            } else if (phase_check2_1 != phase_check_1) {
                dly_div_4[1] = dly;
                phy_calib_flag_1 = false;
            }
        }

        if (!phy_calib_flag && !phy_calib_flag_1) {
            break;
        }
    }

    psram_phy[id]->REG_024 = 0;

    if ((dly_div_4[0] == 0) || (dly_div_4[1] == 0)) {
        if (dly_div_4[0] == 0) {
            dly_div_4[0] = 0x7;
        }

        if (dly_div_4[1] == 0) {
            dly_div_4[1] = 0x7;
        }
        return 1;
    }

    PSRAM_TRACE(0, "phy0 range: 0x%x, T/4 value: %d", range, dly_div_4[0]);
    PSRAM_TRACE(0, "phy1 range: 0x%x, T/4 value: %d", range, dly_div_4[1]);

    return 0;
}

void hal_psram_anaphy_dly_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t dly_div_4)
{
    uint32_t range;
    uint32_t dly;

    range = GET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE);
    dly = GET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DLY);
    PSRAM_TRACE(0, "set psram phy0 range : %d, dly : %d, T/4 : %d", range, dly, dly_div_4);

    hal_psram_anaphy_set_dll_dly(range, dly);
}

void hal_psram_anaphy1_dly_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t dly_div_4)
{
    uint32_t range;
    uint32_t dly;

    range = GET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE_1);
    dly = GET_BITFIELD(psram_phy[id]->REG_07C, PSRAM_PHY_REG0_PSRAM_S2_DLY_1);
    PSRAM_TRACE(0, "set psram phy1 range : %d, dly : %d, T/4 : %d", range, dly, dly_div_4);

    hal_psram_anaphy1_set_dll_dly(range, dly);
}

void hal_psram_anaphy_dly_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t *dly_div_4)
{
    hal_psram_anaphy_dly_set(id, psram_cfg, dly_div_4[0]);
    hal_psram_anaphy1_dly_set(id, psram_cfg, dly_div_4[1]);
}

void hal_psram_phy_dll_dly_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val, range;
    uint32_t dly_div_4[2];

    do {
        val = GET_BITFIELD(psram_phy[id]->REG_044, PSRAM_PHY_PHY_AUTO_LP_STATE);
        if (val == PSRAM_PHY_LP_UP || val == PSRAM_PHY_LP_IDLE)
            break;
    } while (1);

    range = 0;
    for (int i = 0; i <= 3; i++) {
        if (hal_psram_anaphy_calib(id, psram_cfg, dly_div_4, range) == 0) {
            break;
        }
        range++;
    }
    hal_psram_anaphy_dly_init(id, psram_cfg, dly_div_4);
}

void hal_psram_phy_anaphy_open(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    TRACE(0, "psram_phy[id]->REG_040 = 0x%x", psram_phy[id]->REG_040);
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
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL(0) |
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL1(0);
    } else {
        psram_phy[id]->REG_040 |=
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL(4) |
            PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL1(4);
    }
    TRACE(0, "psram_phy[id]->REG_040 = 0x%x", psram_phy[id]->REG_040);

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

    psram_phy[id]->REG_048 = SET_BITFIELD(psram_phy[id]->REG_048, PSRAM_PHY_REG_T_TRANS, 8);
}

/*****************************************************************************
*********************************** hal and device ***************************
*****************************************************************************/
void hal_psram_phy_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    hal_psram_phy_anaphy_open(id, psram_cfg);
    hal_psram_phy_digphy_init(id, psram_cfg);
}

#ifdef CORE_SLEEP_POWER_DOWN
void hal_psram_phy_param_save(enum HAL_PSRAM_ID_T id)
{
}

void hal_psram_phy_param_recover(enum HAL_PSRAM_ID_T id)
{
}
#endif

void hal_psram_phy_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    // hal_cmu_psram_phy_sleep();
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

