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
#if defined(CHIP_HAS_PSRAM) && (CHIP_PSRAM_PHY_VER == 11)

#include "hal_psram.h"

#ifdef PSRAM_CTRL_BASE

#include "plat_types.h"
#include "hal_psramip.h"
#include "reg_psram_phy_v11.h"
#include "hal_timer.h"
#include "psram_debug.h"

struct PSRAM_PHY_T  *const psram_phy[1] = {(struct PSRAM_PHY_T *)(PSRAM_CTRL_BASE + 0x8000)};

#ifdef CORE_SLEEP_POWER_DOWN
static uint32_t psramphy_save_reg_050 = 0;
#endif

void hal_psram_phy_anaphy_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    psram_phy[id]->REG_04C &= ~PSRAM_ULP_PHY_REG_PSRAM_TXDRV_MASK;
    psram_phy[id]->REG_04C |= PSRAM_ULP_PHY_REG_PSRAM_TXDRV(3);
}

void hal_psram_phy_anaphy_pwrup(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    psram_phy[id]->REG_040 |= PSRAM_ULP_PHY_ANA_PHY_PWRUP_REQ;
    hal_sys_timer_delay_us(1);
}

void hal_psram_phy_digphy_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val;

    val = psram_phy[id]->REG_004;
    val &= ~(PSRAM_ULP_PHY_CTRL_DELAY_MASK |
        PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN  |
        PSRAM_ULP_PHY_PHY_DLY_AUTO_EN |
        PSRAM_ULP_PHY_ADDR_4BYTES_EN |
        PSRAM_ULP_PHY_DQS_DM_MERGE_EN |
        PSRAM_ULP_PHY_REG_WR_WO_DM);
    val |= PSRAM_BIT_CONFIG(psram_cfg->device_cfg->addr_4bytes_en, PSRAM_ULP_PHY_ADDR_4BYTES_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->wr_dummy_cyc, PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->dqs_dm_merge_en, PSRAM_ULP_PHY_DQS_DM_MERGE_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->mrw_output_dm, PSRAM_ULP_PHY_REG_WR_WO_DM) |
        PSRAM_ULP_PHY_CTRL_DELAY(1);
    psram_phy[id]->REG_004 = val;
}

void hal_psram_phy_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    hal_psram_phy_anaphy_init(id, psram_cfg);
    hal_psram_phy_digphy_init(id, psram_cfg);
}

void hal_psram_phy_rx_timeout_enable(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_018 = PSRAM_ULP_PHY_RX_TIMEOUT_WORKAROUND_EN |
        PSRAM_ULP_PHY_RX_TIMEOUT_CNT(50);
}

void hal_psram_phy_rx_timeout_disable(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_018 = PSRAM_ULP_PHY_RX_TIMEOUT_CNT(200);
}

uint32_t hal_psram_phy_get_quat_t(enum HAL_PSRAM_ID_T id)
{
    uint32_t val;
    uint32_t dly = 0;
    POSSIBLY_UNUSED uint32_t dly_T_div_4 = 0;
    POSSIBLY_UNUSED uint8_t phase_check, phase_check1, phase_check2;

    phase_check1 = 0x0;
    phase_check2 = 0x0;

    psram_phy[id]->REG_024 = PSRAM_ULP_PHY_REG_TR_CLK_EN | PSRAM_ULP_PHY_REG_TR_DQS_EN;

    psram_phy[id]->REG_068 |= PSRAM_ULP_PHY_REG_PSRAM_CEB1_PU | PSRAM_ULP_PHY_REG_PSRAM_CEB2_PU;
    psram_phy[id]->REG_048 |= PSRAM_ULP_PHY_REG_PSRAM_PU;
    psram_phy[id]->REG_048 &= ~PSRAM_ULP_PHY_REG_PSRAM_SLEEP_EN;

    val = psram_phy[id]->REG_054;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_S1_DELAY_RANGE, 0);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_S2_DELAY_RANGE, 0);
    psram_phy[id]->REG_054 = val;

    psram_phy[id]->REG_054 = SET_BITFIELD(psram_phy[id]->REG_054, PSRAM_ULP_PHY_REG_PSRAM_S1_DLY, 0);
    psram_phy[id]->REG_054 = SET_BITFIELD(psram_phy[id]->REG_054, PSRAM_ULP_PHY_REG_PSRAM_S2_DLY, 0);

    do {
        psram_phy[id]->REG_04C &= ~PSRAM_ULP_PHY_REG_PSRAM_CAL_CLK_EN;
        psram_phy[id]->REG_04C &= ~PSRAM_ULP_PHY_REG_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(1);

        //updata s1/s2 delay
        psram_phy[id]->REG_054 = SET_BITFIELD(psram_phy[id]->REG_054, PSRAM_ULP_PHY_REG_PSRAM_S1_DLY, dly);

        hal_sys_timer_delay_us(2);

        psram_phy[id]->REG_04C |= PSRAM_ULP_PHY_REG_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(2);
        psram_phy[id]->REG_04C |= PSRAM_ULP_PHY_REG_PSRAM_CAL_CLK_EN;
        hal_sys_timer_delay_us(2);

        // read out tx_clk_phase_check<3:0> and tx_clk_dqs_phase_check<3:0 （should be equal)
        PSRAM_TRACE(0, "psram_phy->REG_058 = 0x%x (s1_delay = 0x%x)", psram_phy[id]->REG_058, dly);
        phase_check = GET_BITFIELD(psram_phy[id]->REG_058, PSRAM_ULP_PHY_PSRAM_TX_CLK_PHASE_CHECK);
        // ASSERT(psram_func_find_ones(phase_check) == 2, "Phase check val must have 2 ones, eg. 0110");
        if (phase_check1 == 0) {
            phase_check1 = phase_check;
        } else if (phase_check != phase_check1) {
            //assert phase_check == phase_check1 << 1
            phase_check2 = phase_check;
            break;
        }

        dly++;
        if (dly > GET_BITFIELD(PSRAM_ULP_PHY_REG_PSRAM_S1_DLY_MASK, PSRAM_ULP_PHY_REG_PSRAM_S1_DLY)) {
            break;
        }

    } while (1);
    phase_check2 = 0;
    dly = 1;
    do {
        psram_phy[id]->REG_04C &= ~PSRAM_ULP_PHY_REG_PSRAM_CAL_CLK_EN;
        psram_phy[id]->REG_04C |= PSRAM_ULP_PHY_REG_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(1);

        //updata s1/s2 delay
        psram_phy[id]->REG_054 = SET_BITFIELD(psram_phy[id]->REG_054, PSRAM_ULP_PHY_REG_PSRAM_S2_DLY, dly);

        hal_sys_timer_delay_us(2);

        psram_phy[id]->REG_04C &= ~PSRAM_ULP_PHY_REG_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(2);
        psram_phy[id]->REG_04C |= PSRAM_ULP_PHY_REG_PSRAM_DIV2_RESETB;
        hal_sys_timer_delay_us(2);
        psram_phy[id]->REG_04C |= PSRAM_ULP_PHY_REG_PSRAM_CAL_CLK_EN;
        hal_sys_timer_delay_us(2);

        // read out tx_clk_phase_check<3:0> and tx_clk_dqs_phase_check<3:0 （should be equal)
        PSRAM_TRACE(0, "psram_phy->REG_058 = 0x%x (s2_delay = 0x%x)", psram_phy[id]->REG_058, dly);
        phase_check = GET_BITFIELD(psram_phy[id]->REG_058, PSRAM_ULP_PHY_PSRAM_TX_CLK_PHASE_CHECK);
        // ASSERT(psram_func_find_ones(phase_check) == 2, "Phase check val must have 2 ones, eg. 0110");
        if (phase_check2 == 0) {
            phase_check2 = phase_check;
        } else if (phase_check != phase_check2) {
            //assert phase_check == phase_check2 << 1
            PSRAM_TRACE(0, "phase_check = %d, phase_check2 = %d", phase_check, phase_check2);
            dly_T_div_4 = dly - 1;
            break;
        }

        dly++;
        if (dly > GET_BITFIELD(PSRAM_ULP_PHY_REG_PSRAM_S2_DLY_MASK, PSRAM_ULP_PHY_REG_PSRAM_S2_DLY)) {
            dly_T_div_4 = 7;
            // TRACE_IMM(0, "3");
            break;
        }
    } while (1);
    // while(1){}
    psram_phy[id]->REG_024 = 0;
    psram_phy[id]->REG_04C |= PSRAM_ULP_PHY_REG_PSRAM_DIV2_RESETB;
    psram_phy[id]->REG_04C &= ~PSRAM_ULP_PHY_REG_PSRAM_CAL_CLK_EN;

    return dly_T_div_4;
}

void hal_psram_anaphy_set_rxdqs_dly(uint32_t dly)
{
    uint32_t val;
    val = psram_phy[0]->REG_050;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_CTRL, dly);
    psram_phy[0]->REG_050 = val;
}

void hal_psram_phy_dly_init(enum HAL_PSRAM_ID_T id, uint32_t quat_t)
{
    uint32_t val;

    // #ifdef SIMU
    #if 0
        val = psram_phy->REG_050;
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_CEB1_DLY, 0);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_CEB2_DLY, 0);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY, 0);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY, 0);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_CTRL, 0);
        psram_phy->REG_050 = val;
    #else
        // uint32_t t_div_4 = 0;
        uint32_t range = 0;

        range = GET_BITFIELD(psram_phy[id]->REG_054, PSRAM_ULP_PHY_REG_PSRAM_S2_DELAY_RANGE);

        val = psram_phy[id]->REG_050;
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_CEB_DELAY_RANGE, range);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_CEB1_DLY, quat_t);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_CEB2_DLY, quat_t);

        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DELAY_RANGE, range);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY, quat_t);

        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DELAY_RANGE, range);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY, quat_t);

        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_RANGE, range);
        hal_psram_anaphy_set_rxdqs_dly(quat_t);
        psram_phy[id]->REG_050 = val;
    #endif
}

void hal_psram_phy_dll_dly_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t quat_t;
    quat_t = hal_psram_phy_get_quat_t(id);
    hal_psram_phy_dly_init(id, quat_t);
}

#ifdef CORE_SLEEP_POWER_DOWN
void hal_psram_phy_param_save(enum HAL_PSRAM_ID_T id)
{
    psramphy_save_reg_050 = psram_phy[id]->REG_050;
}

void hal_psram_phy_param_recover(enum HAL_PSRAM_ID_T id)
{
    psram_phy[id]->REG_050 = psramphy_save_reg_050;
}
#endif

void hal_psram_phy_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
}

void hal_psram_phy_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
}

#endif

#endif
