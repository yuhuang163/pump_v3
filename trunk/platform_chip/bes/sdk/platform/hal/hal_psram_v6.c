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
#if defined(CHIP_HAS_PSRAM) && (CHIP_PSRAM_CTRL_VER == 6)

#include "plat_types.h"
#include "plat_addr_map.h"

#include "cmsis.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_l2cache.h"
#include "pmu.h"
#include "reg_psram_mc_v6.h"
#include "reg_psram_phy_v6.h"
#include "string.h"

#include "psram_timing.h"
#include "psram_drv.h"
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
#ifdef CORE_SLEEP_POWER_DOWN
BOOT_BSS_LOC static uint32_t psram_save_dyn_freq_sts;
#endif
BOOT_BSS_LOC static uint32_t psram_t_xphs[2];
BOOT_BSS_LOC static uint32_t psram_t_precharge[2];
BOOT_BSS_LOC static uint32_t psram_t_dvdd[2];
BOOT_BSS_LOC static uint32_t psram_t_lock[2];
#endif

enum MEMIF_CMD_T {
    MEMIF_NO_CMD        = 0x00,
    MEMIF_WRITE         = 0x01,
    MEMIF_READ          = 0x02,
    MEMIF_MRS           = 0x05,
    MEMIF_MRR           = 0x06,
    MEMIF_REF           = 0x08,
    MEMIF_SREF          = 0x09,
    MEMIF_PD            = 0x10,
    MEMIF_LOOPBACK      = 0x11,
    MEMIF_NOP           = 0x20,
    MEMIF_RST           = 0xFF,
    MEMIF_ZQCL          = 0x85,
    MEMIF_ZQCS          = 0x45,
    MEMIF_ZQCRST        = 0x25,
    MEMIF_START_CLOCK   = 0x40,
    MEMIF_STOP_CLOCK    = 0x80,
    MEMIF_NEW_CMD       = 0x7F,
};

enum PSRAM_MC_BUS_HALT_STATE {
    PSRAM_MC_AXI_BUS_FREE  = 0,
    PSRAM_MC_AXI0_BUS_HALT = 1,
    PSRAM_MC_AXI1_BUS_HALT = 2,
    PSRAM_MC_AXI_BUS_HALT  = 3,
    PSRAM_MC_AXI0_BUS_FREE = PSRAM_MC_AXI1_BUS_HALT,
    PSRAM_MC_AXI1_BUS_FREE = PSRAM_MC_AXI0_BUS_HALT,
};

struct PSRAM_MC_T  *const psram_mc[1]   = {(struct PSRAM_MC_T  *)SYS_PSRAM0_CTRL_BASE};
struct PSRAM_PHY_T  *const psram_phy[1] = {(struct PSRAM_PHY_T *)(SYS_PSRAM0_CTRL_BASE + 0x8000)};
static uint32_t psram_sleep_time;
static const uint32_t psram_run_clk = PSRAM_SPEED;

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

void hal_psram_mc_init_complete(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_400 = PSRAM_MC_INIT_COMPLETE;
}

bool hal_psram_mc_is_reset(enum HAL_PSRAM_ID_T id)
{
    return !(psram_mc[id]->REG_400 & PSRAM_MC_INIT_COMPLETE);
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

enum HAL_PSRAM_ANAPHY_LOCK_RESULT_T hal_psram_phy_dll_set_range(uint32_t range, uint32_t time_out_ms)
{
    enum HAL_PSRAM_ANAPHY_LOCK_RESULT_T ret;

    hal_psram_phy_dll_reset_set_dr(0);
    hal_psram_phy_dll_clk_disable_dr(0);
    psramphy_dll_range_set(0, range);
    hal_psram_phy_dll_clk_enable_dr(0);
    hal_psram_phy_dll_reset_clear_dr(0);
    ret = psramphy_dll_wait_lock(0, time_out_ms);
    hal_psram_phy_dll_clk_rdy_dr_clear(0);
    hal_psram_phy_dll_reset_dr_clear(0);

    return ret;
}

// set better dynamically according to clk freq, and return clk_quantificat_val
uint32_t hal_psram_phy_dll_range_dyn_config(uint32_t clk)
{
    uint32_t range;
    uint32_t dly_in;
    enum HAL_PSRAM_ANAPHY_LOCK_RESULT_T val;

    range = hal_psram_phy_get_init_range(clk);

    do {
        // hal_psram_phy_dll_set_range(range, 2);
        if (range > (REG_PSRAM_DLL_RANGE_MASK >> REG_PSRAM_DLL_RANGE_SHIFT)) {
            range = (REG_PSRAM_DLL_RANGE_MASK >> REG_PSRAM_DLL_RANGE_SHIFT);
        }

        val = hal_psram_phy_dll_set_range(range, 2);
        if (val == HAL_PSRAM_ANAPHY_LOCK_TIME_OUT) {
            PSRAM_ASSERT(0, "Time out when wait anaphy  lock");
        } else if (val == HAL_PSRAM_ANAPHY_LOCK_ALL_ONE) {
            PSRAM_TRACE(0, "dll dly is all one");
            if (range < GET_BITFIELD(0xffff, REG_PSRAM_DLL_RANGE)) {
                range++;
                continue;
            }
        }

        dly_in = psramphy_dll_dly_in_get(0);
        PSRAM_TRACE(0, "##range = %d, T/2 = 0x%x", range, dly_in);
        if (dly_in > (REG0_PSRAM_TX_CEB_DLY_MASK >> REG0_PSRAM_TX_CEB_DLY_SHIFT)) {
            PSRAM_TRACE(0, "NOTE: T / 2 > 0x1f");
            if (range < (REG_PSRAM_DLL_RANGE_MASK >> REG_PSRAM_DLL_RANGE_SHIFT)) {
                range++;
                continue;
            }
        }

        break;

    } while (1);

    dly_in = MIN(dly_in,  GET_BITFIELD(0xffff, REG0_PSRAM_TX_CEB_DLY));

    return dly_in;
}

// return dly_in val
uint32_t hal_psram_anaphy_get_dll_dly_in_val(enum HAL_PSRAMPHY_ID_T phy_id)
{
    enum PSRAM_ANAPHY_ID_T id;

    if (phy_id == HAL_PSRAMPHY_ID_0) {
        id = PSRAM_ANAPHY_ID_0;
    } else {
        // ASSERT
        return 0;
    }

    return psramphy_dll_dly_in_get(id);
}

void hal_psram_anaphy_set_dly(enum HAL_PSRAMPHY_ID_T phy_id, enum HAL_PSRAM_PHY_CHANNEL_T c, enum HAL_PSRAM_DLL_DLY_T t, uint16_t dly)
{
    enum PSRAM_ANAPHY_ID_T id;
    enum PSRAM_ANAPHY_PART_T part;

    if (phy_id == HAL_PSRAMPHY_ID_0) {
        id = PSRAM_ANAPHY_ID_0;
    } else {
        // ASSERT
    }

    if (c == HAL_PSRAM_PHY_CHANNEL_0) {
        part = PSRAM_ANAPHY_PART_L;
    } else if (c == HAL_PSRAM_PHY_CHANNEL_1) {
        part = PSRAM_ANAPHY_PART_H;
    } else {
        part = PSRAM_ANAPHY_PART_ALL;
    }

    if (t & HAL_PSRAM_DLL_DLY_TX_CEB) {
        psramphy_tx_ceb_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_CLK) {
        psramphy_tx_clk_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_DQS) {
        psramphy_tx_dqs_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_DM) {
        psramphy_tx_dm_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_DQ) {
        psramphy_tx_dq_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_CMD) {
        psramphy_tx_cmd_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_RX_DQS) {
        psramphy_rx_dqs_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_RX_CLK) {
        psramphy_rx_clk_dly_set(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_RX_ADQ) {
        psramphy_rx_adq_dly_set(id, part, dly);
    }
}

void hal_psram_anaphy_set_dly_1(enum HAL_PSRAMPHY_ID_T phy_id, enum HAL_PSRAM_PHY_CHANNEL_T c, enum HAL_PSRAM_DLL_DLY_T t, uint16_t dly)
{
#ifdef PSRAM_DYN_FP_SUPPORT
    enum PSRAM_ANAPHY_ID_T id;
    enum PSRAM_ANAPHY_PART_T part;

    if (phy_id == HAL_PSRAMPHY_ID_0) {
        id = PSRAM_ANAPHY_ID_0;
    } else {
        // ASSERT
    }

    if (c == HAL_PSRAM_PHY_CHANNEL_0) {
        part = PSRAM_ANAPHY_PART_L;
    } else if (c == HAL_PSRAM_PHY_CHANNEL_1) {
        part = PSRAM_ANAPHY_PART_H;
    } else {
        part = PSRAM_ANAPHY_PART_ALL;
    }

    if (t & HAL_PSRAM_DLL_DLY_TX_CEB) {
        psramphy_tx_ceb_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_CLK) {
        psramphy_tx_clk_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_DQS) {
        psramphy_tx_dqs_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_DM) {
        psramphy_tx_dm_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_DQ) {
        psramphy_tx_dq_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_TX_CMD) {
        psramphy_tx_cmd_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_RX_DQS) {
        psramphy_rx_dqs_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_RX_CLK) {
        psramphy_rx_clk_dly_set_1(id, part, dly);
    }
    if (t & HAL_PSRAM_DLL_DLY_RX_ADQ) {
        psramphy_rx_adq_dly_set_1(id, part, dly);
    }
#endif
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
        PSRAM_PHY_CA_SEQ(psram_cfg->phy_cfg->ca_pattern) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->hyperbus_sample_lat_edge_sel, PSRAM_PHY_HYPERBUS_SAMPLE_LAT_EDGE_SEL) |
        PSRAM_PHY_HYPERBUS_CMD_DLY_CNT(psram_cfg->phy_cfg->hyperbus_cmd_dly_cnt) |
        PSRAM_PHY_PAD_EN |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->reg_ca_dq_lat_sw_en, PSRAM_PHY_REG_CA_DQ_LAT_SW_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->man_tx_wr_push_en, PSRAM_PHY_MAN_TX_WR_PUSH_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->auto_pwr_tx_wr_push_en, PSRAM_PHY_AUTO_PWR_TX_WR_PUSH_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->clk_preample_en, PSRAM_PHY_CLK_PREAMPLE_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->ca_preample_en, PSRAM_PHY_CA_PREAMPLE_EN) |
        PSRAM_PHY_HYPERBUS_TCSS(psram_cfg->phy_cfg->hyperbus_tcss) |
        PSRAM_PHY_CFG_RX_DQS_SEL(2);

    psram_phy[id]->REG_004 =
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->phy_dummy_cyc_en, PSRAM_PHY_PHY_DUMMY_CYC_EN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->phy_dummy_cyc_stop_dqs, PSRAM_PHY_PHY_DUMMY_CYC_STOP_DQS) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->idle_dq_oen, PSRAM_PHY_IDLE_DQ_OEN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->idle_dqs_oen, PSRAM_PHY_IDLE_DQS_OEN) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->dqs_dm_merge_en, PSRAM_PHY_DQS_DM_MERGE_EN)|
        PSRAM_BIT_CONFIG(!psram_cfg->phy_cfg->mrw_output_dm, PSRAM_PHY_MRW_WO_DM) |
        PSRAM_BIT_CONFIG(psram_cfg->phy_cfg->cs_pol, PSRAM_PHY_CS_POL);
}

uint32_t psram_anaphy_get_init_range(uint32_t clk_freq)
{
    uint32_t range;

    if (clk_freq <= 150000000) {
        range = 3;
    } else if (clk_freq <= 250000000) {
        range = 2;
    } else if (clk_freq <= 350000000) {
        range = 1;
    } else {
        range = 0;
    }

    return range;
}

void hal_psram_anaphy_dll_range_init(uint32_t clk_freq)
{
    uint32_t range;
    enum HAL_PSRAM_ANAPHY_LOCK_RESULT_T val;

    range = psram_anaphy_get_init_range(clk_freq);
    hal_psram_phy_dll_reset_set_dr(0);
    hal_psram_phy_dll_clk_disable_dr(0);
    psramphy_dll_range_set(0, range);
    hal_psram_phy_dll_clk_enable_dr(0);
    hal_psram_phy_dll_reset_clear_dr(0);
    val = psramphy_dll_wait_lock(0, 2);
    hal_psram_phy_dll_clk_rdy_dr_clear(0);
    hal_psram_phy_dll_reset_dr_clear(0);
    if (val != HAL_PSRAM_ANAPHY_LOCK_SUCCESS) {
        PSRAM_TRACE(0, "********anaphy dll lock failed : Ret Code = %d", val);
        while (0) {}
    }
}

/*****************************************************************************
*********************************** MC ****************************************
*****************************************************************************/
void hal_psram_mc_set_freq(enum HAL_PSRAM_ID_T id, uint32_t clk)
{
    enum HAL_CMU_FREQ_T freq;

    if (clk <= 13000000) {
        freq = HAL_CMU_FREQ_26M;
    } else if (clk <= 26000000) {
        freq = HAL_CMU_FREQ_52M;
    } else if (clk <= 52000000) {
        freq = HAL_CMU_FREQ_104M;
    } else if (clk <= 104000000) {
        freq = HAL_CMU_FREQ_208M;
    } else {
#ifdef HAL_CMU_FREQ_390M
        freq = HAL_CMU_FREQ_390M;
#else
        freq = HAL_CMU_FREQ_208M;
#endif
    }

    hal_cmu_mem_set_freq(freq);
}

void hal_psram_mc_clock_gate_disable(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_0E8 = PSRAM_MC_CACTIVE_M_MAN |
        PSRAM_MC_CACTIVE_P_MAN |
        PSRAM_MC_CACTIVE_B_MAN |
        PSRAM_MC_CACTIVE_OUT_M_MAN |
        PSRAM_MC_CACTIVE_OUT_H_MAN;
}

void hal_psram_mc_clock_gate_enable(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_0E8 &= ~(PSRAM_MC_CACTIVE_M_MAN |
        PSRAM_MC_CACTIVE_P_MAN |
        PSRAM_MC_CACTIVE_B_MAN |
        PSRAM_MC_CACTIVE_OUT_M_MAN |
        PSRAM_MC_CACTIVE_OUT_H_MAN);
}

static int hal_psram_mc_is_wb_busy(enum HAL_PSRAM_ID_T id)
{
    return !!(psram_mc[id]->REG_404 & PSRAM_MC_WB_FILL_LEVEL_MASK);
}

void hal_psram_mc_wb_busy_wait(enum HAL_PSRAM_ID_T id)
{
    while (hal_psram_mc_is_wb_busy(id));
}

void hal_psram_mc_busy_wait(enum HAL_PSRAM_ID_T id)
{
    uint32_t val;

    do {val = psram_mc[id]->REG_404 & PSRAM_MC_BUSY;
    } while (!!val);
}

uint32_t hal_psram_mc_set_mgr_ch(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_MC_MGR_CHANEL_T c)
{
    uint32_t val0, val1;

    val0 = psram_mc[id]->REG_00C;
    val1 = GET_BITFIELD(val0, PSRAM_MC_MGR_MR_CH);
    psram_mc[id]->REG_00C = SET_BITFIELD(val0, PSRAM_MC_MGR_MR_CH, c);

    return val1;
}

void hal_psram_mc_set_cmd(enum HAL_PSRAM_ID_T id, enum MEMIF_CMD_T cmd)
{
    psram_mc[id]->REG_004 = PSRAM_MC_MGR_CMD(cmd);
    hal_psram_mc_busy_wait(id);
}

void hal_psram_mc_set_cmd_pd(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_PD);
}

void hal_psram_mc_set_cmd_nop(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_NOP);
}

void hal_psram_mc_set_cmd_start_clock(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_START_CLOCK);
}

void hal_psram_mc_set_cmd_stop_clock(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_STOP_CLOCK);
}

void hal_psram_mc_set_cmd_rst(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_RST);
}

void hal_psram_mc_set_cmd_mrs(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_MRS);
}

void hal_psram_mc_set_cmd_mrr(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_MRR);
}

bool hal_psram_mc_is_sleep(enum HAL_PSRAM_ID_T id)
{
    return !!(GET_BITFIELD(psram_mc[id]->REG_404, PSRAM_MC_CP_FSM_STATE) == PSRAM_MC_FSM_PD_MODE);
}

void hal_psram_mc_flush_all_fifo(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_busy_wait(id);
    psram_mc[id]->REG_01C = PSRAM_MC_MGR_TX_FIFO_CLR | PSRAM_MC_MGR_RX_FIFO_CLR;
    hal_psram_mc_busy_wait(id);
}

void hal_psram_mc_write_tx_fifo(enum HAL_PSRAM_ID_T id, uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        psram_mc[id]->REG_014 = PSRAM_MC_MGR_TX_FIFO(*data++);
    }
}

void hal_psram_mc_read_rx_fifo(enum HAL_PSRAM_ID_T id, uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        *data++ = psram_mc[id]->REG_018;
    }
}

uint32_t hal_psram_mc_get_tx_fifo_empty_cnt(enum HAL_PSRAM_ID_T id)
{
    return GET_BITFIELD(psram_mc[id]->REG_404, PSRAM_MC_MGR_TXFIFO_EMPTY_CNT);
}

int hal_psram_mc_get_rx_fifo_empty_cnt(enum HAL_PSRAM_ID_T id)
{
    return !!(psram_mc[id]->REG_404 & PSRAM_MC_MGR_RXFIFO_R_EMPTY);
}

void hal_psram_mc_wait_write_buffer_empty(enum HAL_PSRAM_ID_T id)
{
    uint32_t level;
    uint32_t start = hal_sys_timer_get();

    __DSB();
    do {
        level = GET_BITFIELD(psram_mc[id]->REG_404, PSRAM_MC_WB_FILL_LEVEL);
        if (level == 0) {
            break;
        }
    } while (hal_sys_timer_get() - start < MS_TO_TICKS(10));

    ASSERT(level == 0, "%s: Drain timeout: level=%u status=0x%08X", __func__, level, psram_mc[id]->REG_404);
}
void hal_psram_wait_write_buffer_empty(enum HAL_PSRAM_ID_T id) __attribute__((alias("hal_psram_mc_wait_write_buffer_empty")));

void hal_psram_mc_set_default_param(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val = 0;

    // default wl  rl
    psram_mc[id]->REG_028 =
        PSRAM_MC_WRITE_LATENCY_0(psram_cfg->device_timing->wl_def) |
        (psram_cfg->device_timing->is_mrs_wl_fixed ? PSRAM_MC_WRITE_LATENCY_MR_0(psram_cfg->device_timing->fixed_wl_mrs):
            PSRAM_MC_WRITE_LATENCY_MR_0(psram_cfg->device_timing->wl_def));

    psram_mc[id]->REG_02C =
        PSRAM_MC_READ_LATENCY_0(psram_cfg->device_timing->rl_min_def) |
        PSRAM_MC_READ_LATENCY1_0(psram_cfg->device_timing->rl_max_def +
            T_NS_TO_NCK(psram_cfg->cfg_freq_mhz * 1000000, psram_cfg->device_timing->t_dqsck_max_ns));

    psram_mc[id]->REG_224 =
        PSRAM_MC_WRITE_LATENCY_1(psram_cfg->device_timing->wl_def) |
        (psram_cfg->device_timing->is_mrs_wl_fixed ? PSRAM_MC_WRITE_LATENCY_MR_1(psram_cfg->device_timing->fixed_wl_mrs):
            PSRAM_MC_WRITE_LATENCY_MR_1(psram_cfg->device_timing->wl_def)) |
        PSRAM_MC_READ_LATENCY_1(psram_cfg->device_timing->rl_min_def) |
        PSRAM_MC_READ_LATENCY1_1(psram_cfg->device_timing->rl_max_def +
            T_NS_TO_NCK(psram_cfg->cfg_freq_mhz * 1000000, psram_cfg->device_timing->t_dqsck_max_ns));

    // t_xphs -- nop_dummy clc
    val = T_NS_TO_NCK(psram_cfg->cfg_freq_mhz, 60);
    psram_mc[id]->REG_09C = SET_BITFIELD(psram_mc[id]->REG_09C, PSRAM_MC_NOP_DMY_CYC, val);
    // ? mrs_as_pd_wrlat
    val = psram_cfg->device_timing->is_mrs_wl_fixed ? (psram_cfg->device_timing->fixed_wl_mrs) : ( psram_cfg->device_timing->wl_def);
    psram_mc[id]->REG_020 = SET_BITFIELD(psram_mc[id]->REG_020, PSRAM_MC_MRS_AS_PD_WRLAT, val);
}

void hal_psram_mc_set_timing(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t freq_mhz, bool config_latency)
{
    POSSIBLY_UNUSED uint32_t item;
    uint32_t i;
    uint32_t val;

    for (i = 0; i < psram_cfg->device_timing->ac_timing_count; i++) {
        if ((psram_cfg->device_timing->ac_timing[i].freq_mhz >= freq_mhz) &&
            (psram_cfg->device_timing->ac_timing[i].freq_mhz < freq_mhz + 20)) {
            item = i;
            break;
        }
    }

    /// TODO: Aserrt
    if (i >= psram_cfg->device_timing->ac_timing_count) {
        PSRAM_ASSERT(0, "Param error");
    }

    if (config_latency) {
        ///TODO: move to func  latency_config();
        psram_mc[id]->REG_028 =
            PSRAM_MC_WRITE_LATENCY_0(psram_cfg->device_timing->ac_timing[item].wl) |
            (psram_cfg->device_timing->is_mrs_wl_fixed ? PSRAM_MC_WRITE_LATENCY_MR_0(psram_cfg->device_timing->fixed_wl_mrs) :
                PSRAM_MC_WRITE_LATENCY_MR_0(psram_cfg->device_timing->ac_timing[item].wl));

        psram_mc[id]->REG_02C =
            PSRAM_MC_READ_LATENCY_0(psram_cfg->device_timing->ac_timing[item].rl_min) |
            PSRAM_MC_READ_LATENCY1_0(psram_cfg->device_timing->ac_timing[item].rl_max +
                psram_cfg->device_timing->ac_timing[item].t_dqsck);

        psram_mc[id]->REG_224 =
            PSRAM_MC_WRITE_LATENCY_1(psram_cfg->device_timing->ac_timing[item].wl) |
           (psram_cfg->device_timing->is_mrs_wl_fixed ? PSRAM_MC_WRITE_LATENCY_MR_1(psram_cfg->device_timing->fixed_wl_mrs) :
                PSRAM_MC_WRITE_LATENCY_MR_1(psram_cfg->device_timing->ac_timing[item].wl)) |
            PSRAM_MC_READ_LATENCY_1(psram_cfg->device_timing->ac_timing[item].rl_min) |
            PSRAM_MC_READ_LATENCY1_1(psram_cfg->device_timing->ac_timing[item].rl_max + psram_cfg->device_timing->ac_timing[item].t_dqsck);

        // t_xphs -- nop_dummy clc
        val = psram_cfg->device_timing->ac_timing[item].t_xphs;  //? tcph
        psram_mc[id]->REG_09C = SET_BITFIELD(psram_mc[id]->REG_09C, PSRAM_MC_NOP_DMY_CYC, val);

        // ? mrs_as_pd_wrlat
        val = psram_cfg->device_timing->is_mrs_wl_fixed ? (psram_cfg->device_timing->fixed_wl_mrs) : (psram_cfg->device_timing->ac_timing[item].wl);
        psram_mc[id]->REG_020 = SET_BITFIELD(psram_mc[id]->REG_020, PSRAM_MC_MRS_AS_PD_WRLAT, val);
    }

    psram_mc[id]->REG_04C =
        PSRAM_MC_T_REFI_0(psram_cfg->device_timing->ac_timing[item].t_refi) |
        PSRAM_MC_NUM_OF_BURST_RFS(0);

    psram_mc[id]->REG_050 =
        PSRAM_MC_T_RC_0(psram_cfg->device_timing->ac_timing[item].t_rc);

    psram_mc[id]->REG_054 =
        PSRAM_MC_T_RFC_0(psram_cfg->device_timing->ac_timing[item].t_rfc);

    psram_mc[id]->REG_058 =
        PSRAM_MC_T_CPHR_0(psram_cfg->device_timing->ac_timing[item].t_cphr);

    psram_mc[id]->REG_05C =
        PSRAM_MC_T_CPHR_AP_0(psram_cfg->device_timing->ac_timing[item].t_cphr);

    psram_mc[id]->REG_060 =
        PSRAM_MC_T_CPHW_0(psram_cfg->device_timing->ac_timing[item].t_cphw);

    psram_mc[id]->REG_064 =
        PSRAM_MC_T_CPHW_AP_0(psram_cfg->device_timing->ac_timing[item].t_cphw);

    psram_mc[id]->REG_068 =
        PSRAM_MC_T_MRR_0(psram_cfg->device_timing->ac_timing[item].t_mrr);

    psram_mc[id]->REG_06C =
        PSRAM_MC_T_MRS_0(psram_cfg->device_timing->ac_timing[item].t_mrs);

    psram_mc[id]->REG_070 =
        PSRAM_MC_T_CEM_0(psram_cfg->device_timing->ac_timing[item].t_cem);

    psram_mc[id]->REG_074 =
        PSRAM_MC_T_RST_0(psram_cfg->device_timing->ac_timing[item].t_rst);

    psram_mc[id]->REG_078 =
        PSRAM_MC_T_SRF_0(psram_cfg->device_timing->ac_timing[item].t_srf);

    psram_mc[id]->REG_07C =
        PSRAM_MC_T_XSR_0(psram_cfg->device_timing->ac_timing[item].t_xsr);

    psram_mc[id]->REG_080 =
        PSRAM_MC_T_HS_0(psram_cfg->device_timing->ac_timing[item].t_hs);

    psram_mc[id]->REG_084 =
        PSRAM_MC_T_XPHS_0(psram_cfg->device_timing->ac_timing[item].t_xphs);

    psram_mc[id]->REG_088 =
        PSRAM_MC_T_XHS_0(psram_cfg->device_timing->ac_timing[item].t_xhs);

    psram_mc[id]->REG_08C =
        PSRAM_MC_T_ZQCAL_0(psram_cfg->device_timing->ac_timing[item].t_zacal);

    psram_mc[id]->REG_090 =
        PSRAM_MC_T_ZQCRST_0(psram_cfg->device_timing->ac_timing[item].t_zqcrst);

    psram_mc[id]->REG_094 =
        PSRAM_MC_T_XCKD_0(psram_cfg->device_timing->ac_timing[item].t_xckd);

    psram_mc[id]->REG_098 =
        PSRAM_MC_T_ECKD_0(psram_cfg->device_timing->ac_timing[item].t_eckd);

    psram_phy[id]->REG_008 =
        PSRAM_PHY_T_WPST(psram_cfg->device_timing->ac_timing[item].t_wpst);

    psram_phy[id]->REG_048 =
        SET_BITFIELD(psram_phy[id]->REG_048, PSRAM_PHY_REG_T_PRECHARGE, psram_cfg->device_timing->ac_timing[item].t_precharge);

    psram_t_dvdd[0] = psram_phy[id]->REG_04C =
        PSRAM_PHY_REG_T_PU_DVDD(psram_cfg->device_timing->ac_timing[item].t_dvdd) |
        PSRAM_PHY_REG_T_PU_AVDDQ(psram_cfg->device_timing->ac_timing[item].t_avddq);

    psram_phy[id]->REG_050 =
        SET_BITFIELD(psram_phy[id]->REG_050, PSRAM_PHY_REG_T_LOCK, psram_cfg->device_timing->ac_timing[item].t_lock);

    psram_t_precharge[0] = psram_cfg->device_timing->ac_timing[item].t_precharge;
    psram_t_lock[0] = psram_cfg->device_timing->ac_timing[item].t_lock;
    psram_t_xphs[0] = psram_cfg->device_timing->ac_timing[item].t_xphs;
}


void hal_psram_mc_priority_config(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_0B0 = PSRAM_MC_TURN_LIMIT(8);

    psram_mc[id]->REG_288 =
        PSRAM_MC_QOS0_PRIORITY(0) |
        PSRAM_MC_QOS1_PRIORITY(1) |
        PSRAM_MC_QOS2_PRIORITY(2) |
        PSRAM_MC_QOS3_PRIORITY(3) |
        PSRAM_MC_QOS4_PRIORITY(4) |
        PSRAM_MC_QOS5_PRIORITY(5) |
        PSRAM_MC_QOS6_PRIORITY(6) |
        PSRAM_MC_QOS7_PRIORITY(7);
    psram_mc[id]->REG_290 =
        PSRAM_MC_WRITE_FILL_PRIORITY_1_8THS(1) |
        PSRAM_MC_WRITE_FILL_PRIORITY_2_8THS(2) |
        PSRAM_MC_WRITE_FILL_PRIORITY_3_8THS(3) |
        PSRAM_MC_WRITE_FILL_PRIORITY_4_8THS(4) |
        PSRAM_MC_WRITE_FILL_PRIORITY_5_8THS(5) |
        PSRAM_MC_WRITE_FILL_PRIORITY_6_8THS(6) |
        PSRAM_MC_WRITE_FILL_PRIORITY_7_8THS(7);
    psram_mc[id]->REG_298 =
        PSRAM_MC_READ_FILL_PRIORITY_1_8THS(1) |
        PSRAM_MC_READ_FILL_PRIORITY_2_8THS(2) |
        PSRAM_MC_READ_FILL_PRIORITY_3_8THS(3) |
        PSRAM_MC_READ_FILL_PRIORITY_4_8THS(4) |
        PSRAM_MC_READ_FILL_PRIORITY_5_8THS(5) |
        PSRAM_MC_READ_FILL_PRIORITY_6_8THS(6) |
        PSRAM_MC_READ_FILL_PRIORITY_7_8THS(7);

    psram_mc[id]->REG_2A0 =
        PSRAM_MC_WRITE_THRESHOLD_ENABLE |
        PSRAM_MC_READ_THRESHOLD_ENABLE;

#if 0
    psram_mc[id]->REG_2A4 =
        PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_0(7) |
        PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_1(0) |
        PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_0(7) |
        PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_1(0);

    psram_mc[id]->REG_2A8 =
        PSRAM_MC_WVQOS0_DR_EN | PSRAM_MC_WVQOS0(7) |
        PSRAM_MC_WVQOS1_DR_EN | PSRAM_MC_WVQOS1(0) |
        PSRAM_MC_RVQOS0_DR_EN | PSRAM_MC_RVQOS0(7) |
        PSRAM_MC_RVQOS1_DR_EN | PSRAM_MC_RVQOS1(0);
#endif
}

void hal_psram_mc_auto_lowpower_enable(enum HAL_PSRAM_ID_T id)
{
    // low power config
    psram_mc[id]->REG_024 =
        PSRAM_BIT_CONFIG(1, PSRAM_MC_AUTOWAKEUP_EN) |    //1
        PSRAM_BIT_CONFIG(1, PSRAM_MC_ENTRY_SELF_REFRESH_IDLE) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_ENTRY_SLEEP_IDLE) | //1
        PSRAM_BIT_CONFIG(0, PSRAM_MC_SELF_REFRESH_LP_EN) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_BURST_REFRESH_EN) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_REFRESH_MODE) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_MAN_CKE_EN);

    psram_mc[id]->REG_0A4 = PSRAM_MC_QUEUE_IDLE_CYCLE(psram_run_clk * PSRAM_QUEUE_IDLE_TIME_US / 2);
}

void hal_psram_mc_auto_lowpower_disable(enum HAL_PSRAM_ID_T id)
{
    uint32_t val;

    // low power config
    psram_mc[id]->REG_024 =
        PSRAM_BIT_CONFIG(1, PSRAM_MC_AUTOWAKEUP_EN) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_ENTRY_SELF_REFRESH_IDLE) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_ENTRY_SLEEP_IDLE) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_SELF_REFRESH_LP_EN) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_BURST_REFRESH_EN) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_REFRESH_MODE) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_MAN_CKE_EN);

        // psram_mc[id]->REG_0A4 = PSRAM_MC_QUEUE_IDLE_CYCLE(/*psram->run_freq_mhz*/ 1000 / 2);
    do {
        val = GET_BITFIELD(psram_mc[HAL_PSRAM_ID_0]->REG_404, PSRAM_MC_CP_FSM_STATE);
    } while (val != PSRAM_MC_FSM_IDLE_MODE);
}

void hal_psram_mc_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    psram_mc[id]->REG_038 = PSRAM_BIT_CONFIG(0, PSRAM_MC_BUS_WIDTH);
    psram_mc[id]->REG_048 = PSRAM_MC_FRE_RATIO(1);
    psram_mc[id]->REG_000 =
        PSRAM_BIT_CONFIG(psram_cfg->psram_type > PSRAM_TYPE_HYPERRAM, PSRAM_MC_CHIP_TYPE) |
        PSRAM_BIT_CONFIG(psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X8, PSRAM_MC_CHIP_BIT) |
        PSRAM_BIT_CONFIG(psram_cfg->psram_data_bus >= PSRAM_DATA_BUS_X16, PSRAM_MC_CHIP_X16) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_CHIP_SWITCH) |
        PSRAM_BIT_CONFIG(psram_cfg->mc_cfg->chip_cmd_4bit, PSRAM_MC_CHIP_CMD_4BIT) |
        PSRAM_BIT_CONFIG((psram_cfg->psram_type == PSRAM_TYPE_HYPERRAM), PSRAM_MC_HYPERBUS) |
        PSRAM_BIT_CONFIG(psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16, PSRAM_MC_CHIP_DDP) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_CHIP_DDP_CLK_COMMON) |
        PSRAM_MC_CHIP_CA_MAP(psram_cfg->mc_cfg->chip_ca_map) |
        PSRAM_MC_OPI_MODE | PSRAM_MC_DTR_MODE(7) |
        PSRAM_MC_CHIP_CS0_REGION(3); // 3:32MB

    psram_mc[id]->REG_140 =
        PSRAM_MC_CMD_TABLE_ARRAY_RD(psram_cfg->cmd->cmd_array_rd) |
        PSRAM_MC_CMD_TABLE_ARRAY_WR(psram_cfg->cmd->cmd_array_wr) |
        PSRAM_MC_CMD_TABLE_REG_RD(psram_cfg->cmd->cmd_reg_rd) |
        PSRAM_MC_CMD_TABLE_REG_WR(psram_cfg->cmd->cmd_reg_wr);

    psram_mc[id]->REG_150 =
        PSRAM_MC_CMD_TABLE_AUTO_REFR(psram_cfg->cmd->cmd_auto_refr) |
        PSRAM_MC_CMD_TABLE_SELF_REFR(psram_cfg->cmd->cmd_self_refr) |
        PSRAM_MC_CMD_TABLE_HSLP_ENTRY(psram_cfg->cmd->cmd_hslp_entry) |
        PSRAM_MC_CMD_TABLE_GLBRST(psram_cfg->cmd->cmd_glbrst);

    psram_mc[id]->REG_160 =
        PSRAM_MC_CMD_TABLE_NOP(psram_cfg->cmd->cmd_nop);

    psram_mc[id]->REG_020 =
        PSRAM_BIT_CONFIG(psram_cfg->device_timing->is_mrs_wl_fixed, PSRAM_MC_MRS_AS_PD_WRLAT_DR) | // ?
        PSRAM_BIT_CONFIG(psram_cfg->device_timing->mrs_as_pd_en, PSRAM_MC_MRS_AS_PD_EN) |
        PSRAM_MC_MRS_AS_PD_MR(psram_cfg->device_timing->mrs_as_pd_mr) |
        PSRAM_MC_MRS_AS_PD_MD(psram_cfg->device_timing->mrs_as_pd_md);

    psram_mc[id]->REG_030 =
        psram_cfg->mc_cfg->prefetch_width == PSRAM_PREFETCH_WIDTD_16N ? PSRAM_MC_PREFETCH_16N :
        psram_cfg->mc_cfg->prefetch_width == PSRAM_PREFETCH_WIDTD_32N ? PSRAM_MC_PREFETCH_32N : 0;

    psram_mc[id]->REG_034 =
        PSRAM_MC_BURST_LENGTH_0(psram_cfg->mc_cfg->burst_length) |
        PSRAM_MC_PAGE_BOUNDARY(psram_cfg->mc_cfg->page_boundary);

    psram_mc[id]->REG_040 =
        PSRAM_BIT_CONFIG(psram_cfg->mc_cfg->cp_wrap_en, PSRAM_MC_CP_WRAP_EN_0) |
        PSRAM_BIT_CONFIG(psram_cfg->mc_cfg->auto_precharge, PSRAM_MC_AUTO_PRECHARGE);

    psram_mc[id]->REG_044 =
        PSRAM_BIT_CONFIG(0, PSRAM_MC_SNP_DISABLE) |
        PSRAM_BIT_CONFIG(psram_cfg->mc_cfg->wb_bypass, PSRAM_MC_WB_BYPASS);

    psram_mc[id]->REG_0E0 =
        PSRAM_BIT_CONFIG(1, PSRAM_MC_RD_DF_EN) |
        PSRAM_BIT_CONFIG(1, PSRAM_MC_RD_DF_DEC_EN) |
        PSRAM_BIT_CONFIG(1, PSRAM_MC_WR_DM_EN) |
        PSRAM_BIT_CONFIG(1, PSRAM_MC_WR_DM_DEC_EN) |
        PSRAM_BIT_CONFIG(1, PSRAM_MC_RD_RIB_FIFO_POP_MERGE) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_PREFETCH_ENABLE);

    psram_mc[id]->REG_09C =
        PSRAM_MC_WR_DMY_CYC(psram_cfg->mc_cfg->wr_dummy_cyc) |
        PSRAM_MC_CMD_DMY_CYC(psram_cfg->mc_cfg->cmd_dummy_cyc);

    psram_mc[id]->REG_0A0 =
        PSRAM_BIT_CONFIG(psram_cfg->mc_cfg->stop_clk_in_nop, PSRAM_MC_STOP_CLK_IN_NOP) |
        PSRAM_BIT_CONFIG(psram_cfg->mc_cfg->stop_clk_in_tcph, PSRAM_MC_STOP_CLK_IN_TCPH);

    // mgr_len_seq0
    psram_mc[id]->REG_00C =
        PSRAM_MC_MGR_LEN(1) |
        PSRAM_MC_MGR_LEN_BYPASS |
        PSRAM_MC_MRS_AS_PD_LEN(1) |
        PSRAM_MC_MGR_MR_CH(1);

    // mgr_wstrb_seq0
    psram_mc[id]->REG_010 = PSRAM_MC_MGR_WSTRB(0xfffe);

    if (psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16) {
        hal_psram_mc_set_mgr_ch(id, HAL_PSRAM_MC_MGR_CHANEL_ALL);
    }
}

#ifdef PSRAM_DYN_FP_SUPPORT
void hal_psram_mc_set_timing_1(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t freq_mhz)
{
    POSSIBLY_UNUSED uint32_t item;
    uint32_t i;

    for (i = 0; i < psram_cfg->device_timing->ac_timing_count; i++) {
        if ((psram_cfg->device_timing->ac_timing[i].freq_mhz >= freq_mhz) &&
            (psram_cfg->device_timing->ac_timing[i].freq_mhz < freq_mhz + 20)) {
            item = i;
            break;
        }
    }

    /// TODO: Aserrt
    if (i >= psram_cfg->device_timing->ac_timing_count) {
        PSRAM_ASSERT(0, "Param error");
    }

    //set fp1 MR
    psram_mc[id]->REG_224 =
        PSRAM_MC_WRITE_LATENCY_1(psram_cfg->device_timing->ac_timing[item].wl) |
        (psram_cfg->device_timing->is_mrs_wl_fixed ? PSRAM_MC_WRITE_LATENCY_MR_1(psram_cfg->device_timing->fixed_wl_mrs) :
            PSRAM_MC_WRITE_LATENCY_MR_1(psram_cfg->device_timing->ac_timing[item].wl)) |
        PSRAM_MC_READ_LATENCY_1(psram_cfg->device_timing->ac_timing[item].rl_min) |
        PSRAM_MC_READ_LATENCY1_1(psram_cfg->device_timing->ac_timing[item].rl_max + psram_cfg->device_timing->ac_timing[item].t_dqsck);

    //set fp1 wrap and burst length
    psram_mc[id]->REG_228 =
        PSRAM_BIT_CONFIG(psram_cfg->mc_cfg->cp_wrap_en, PSRAM_MC_CP_WRAP_EN_1) |
        PSRAM_MC_BURST_LENGTH_1(psram_cfg->mc_cfg->burst_length);

    //set fp1 mc timing
    psram_mc[id]->REG_22C =
        PSRAM_MC_T_REFI_1(psram_cfg->device_timing->ac_timing[item].t_refi);

    psram_mc[id]->REG_230 =
        PSRAM_MC_T_RC_1(psram_cfg->device_timing->ac_timing[item].t_rc);

    psram_mc[id]->REG_234 =
        PSRAM_MC_T_RFC_1(psram_cfg->device_timing->ac_timing[item].t_rfc);

    psram_mc[id]->REG_238 =
        PSRAM_MC_T_CPHR_1(psram_cfg->device_timing->ac_timing[item].t_cphr);

    psram_mc[id]->REG_23C =
        PSRAM_MC_T_CPHR_AP_1(psram_cfg->device_timing->ac_timing[item].t_cphr);

    psram_mc[id]->REG_240 =
        PSRAM_MC_T_CPHW_1(psram_cfg->device_timing->ac_timing[item].t_cphw);

    psram_mc[id]->REG_244 =
        PSRAM_MC_T_CPHW_AP_1(psram_cfg->device_timing->ac_timing[item].t_cphw);

    psram_mc[id]->REG_248 =
        PSRAM_MC_T_MRR_1(psram_cfg->device_timing->ac_timing[item].t_mrr);

    psram_mc[id]->REG_24C =
        PSRAM_MC_T_MRS_1(psram_cfg->device_timing->ac_timing[item].t_mrs);

    psram_mc[id]->REG_250 =
        PSRAM_MC_T_CEM_1(psram_cfg->device_timing->ac_timing[item].t_cem);

    psram_mc[id]->REG_254 =
        PSRAM_MC_T_RST_1(psram_cfg->device_timing->ac_timing[item].t_rst);

    psram_mc[id]->REG_258 =
        PSRAM_MC_T_SRF_1(psram_cfg->device_timing->ac_timing[item].t_srf);

    psram_mc[id]->REG_25C =
        PSRAM_MC_T_XSR_1(psram_cfg->device_timing->ac_timing[item].t_xsr);

    psram_mc[id]->REG_260 =
        PSRAM_MC_T_HS_1(psram_cfg->device_timing->ac_timing[item].t_hs);

    psram_mc[id]->REG_264 =
        PSRAM_MC_T_XPHS_1(psram_cfg->device_timing->ac_timing[item].t_xphs);

    psram_mc[id]->REG_268 =
        PSRAM_MC_T_XHS_1(psram_cfg->device_timing->ac_timing[item].t_xhs);

    psram_mc[id]->REG_26C =
        PSRAM_MC_T_ZQCAL_1(psram_cfg->device_timing->ac_timing[item].t_zacal);

    psram_mc[id]->REG_270 =
        PSRAM_MC_T_ZQCRST_1(psram_cfg->device_timing->ac_timing[item].t_zqcrst);

    psram_mc[id]->REG_274 =
        PSRAM_MC_T_XCKD_1(psram_cfg->device_timing->ac_timing[item].t_xckd);

    psram_mc[id]->REG_278 =
        PSRAM_MC_T_ECKD_1(psram_cfg->device_timing->ac_timing[item].t_eckd);

    psram_t_dvdd[1] =
        PSRAM_PHY_REG_T_PU_DVDD(psram_cfg->device_timing->ac_timing[item].t_dvdd) |
        PSRAM_PHY_REG_T_PU_AVDDQ(psram_cfg->device_timing->ac_timing[item].t_avddq);

    psram_t_precharge[1] = psram_cfg->device_timing->ac_timing[item].t_precharge;
    psram_t_lock[1] = psram_cfg->device_timing->ac_timing[item].t_lock;
    psram_t_xphs[1] = psram_cfg->device_timing->ac_timing[item].t_xphs;
}

void hal_psram_set_timing_0(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_DYN_FREQ_ID_T freq_id, struct PSRAM_CFG_T *psram_cfg, uint32_t clk)
{
    uint8_t reg[3] = {0};
    uint8_t val[3] = {0};

    psram_mr_get(psram_cfg, reg, val, clk);

    if (freq_id == HAL_PSRAM_DYN_FREQ_ID_0) {
        psram_mc[id]->REG_204 =
            PSRAM_MC_DYN_FREQ_MR0_0(reg[0]) | PSRAM_MC_DYN_FREQ_MR1_0(reg[1]) | PSRAM_MC_DYN_FREQ_MR2_0(reg[2]);
        psram_mc[id]->REG_20C =
            PSRAM_MC_DYN_FREQ_MD0_0(val[0]) | PSRAM_MC_DYN_FREQ_MD1_0(val[1]) | PSRAM_MC_DYN_FREQ_MD2_0(val[2]);
    } else {
        psram_mc[id]->REG_214 =
            PSRAM_MC_DYN_FREQ_MR0_1(reg[0]) | PSRAM_MC_DYN_FREQ_MR1_1(reg[1]) | PSRAM_MC_DYN_FREQ_MR2_1(reg[2]);
        psram_mc[id]->REG_21C =
            PSRAM_MC_DYN_FREQ_MD0_1(val[0]) | PSRAM_MC_DYN_FREQ_MD1_1(val[1]) | PSRAM_MC_DYN_FREQ_MD2_1(val[2]);
    }
}

void hal_psram_set_dyn_fp_timing(enum HAL_PSRAM_DYN_FREQ_ID_T freq_id, enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_09C = SET_BITFIELD(psram_mc[id]->REG_09C, PSRAM_MC_NOP_DMY_CYC, psram_t_xphs[freq_id]);
    psram_phy[id]->REG_04C = psram_t_dvdd[freq_id];
    psram_phy[id]->REG_048 = SET_BITFIELD(psram_phy[id]->REG_048, PSRAM_PHY_REG_T_PRECHARGE, psram_t_precharge[freq_id]);
    psram_phy[id]->REG_050 = SET_BITFIELD(psram_phy[id]->REG_050, PSRAM_PHY_REG_T_LOCK, psram_t_lock[freq_id]);
}

void hal_psram_dyn_freq_select(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_DYN_FREQ_ID_T freq_id, uint32_t sleep)
{
    if (!!(psram_mc[id]->REG_200 & PSRAM_MC_DYN_FREQ_STS) == freq_id) {
        return;
    }

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_disable(id);
#endif

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_disable(id);
#endif

    __DSB();

    if (sleep == 0) {
        hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
#ifdef L2CC0_BASE
        if (hal_l2cache_enabled(HAL_L2CACHE_ID_0)) {
            hal_l2cache_sync_all(HAL_L2CACHE_ID_0);
        }
#endif
    }

    hal_psram_mc_wb_busy_wait(id);
    hal_psram_mc_busy_wait(id);

    psram_mc[id]->REG_19C |= PSRAM_MC_SW_BUS_HALT_EN;
    while (GET_BITFIELD(psram_mc[id]->REG_19C, PSRAM_MC_SW_BUS_HALT_STS) != PSRAM_MC_AXI_BUS_HALT);

    hal_psram_mc_wb_busy_wait(id);
    hal_psram_mc_busy_wait(id);

    if (freq_id == HAL_PSRAM_DYN_FREQ_ID_0) {
        psram_mc[id]->REG_200 &= ~PSRAM_MC_DYN_FREQ_SET_POINT;
        hal_sys_timer_delay_us(10);
        while (psram_mc[id]->REG_200 & PSRAM_MC_DYN_FREQ_STS);

        hal_psram_mc_set_freq(id, PSRAM_SPEED_0 * 1000000);
        hal_sys_timer_delay_us(10);
    } else {
        psram_mc[id]->REG_200 |= PSRAM_MC_DYN_FREQ_SET_POINT;
        hal_sys_timer_delay_us(10);
        while (!(psram_mc[id]->REG_200 & PSRAM_MC_DYN_FREQ_STS));

        hal_psram_mc_set_freq(id, PSRAM_SPEED_1 * 1000000);
        hal_sys_timer_delay_us(10);
    }

    hal_psram_set_dyn_fp_timing(freq_id, id);

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_enable(id);
#endif

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_enable(id);
#endif

    psram_mc[id]->REG_19C &= ~PSRAM_MC_SW_BUS_HALT_EN;
}
#endif

/*****************************************************************************
*********************************** hal and device ***************************
*****************************************************************************/
void hal_psram_device_mr_write(enum HAL_PSRAM_ID_T id, uint32_t mr, uint32_t val)
{
    hal_psram_mc_flush_all_fifo(id);
    hal_psram_mc_write_tx_fifo(id, &val, 1);
    psram_mc[id]->REG_010 = PSRAM_MC_MGR_WSTRB(0xfffe);
    psram_mc[id]->REG_008 = PSRAM_MC_MGR_ADDR(mr);
    hal_psram_mc_set_cmd_mrs(id);
    while(hal_psram_mc_get_tx_fifo_empty_cnt(id) != 8) {}
    hal_psram_mc_busy_wait(id);
    //for some mrs cost so much time
    hal_sys_timer_delay_us(10);
}

uint32_t hal_psram_device_mr_read(enum HAL_PSRAM_ID_T id, uint32_t mr)
{
    uint32_t val = 0;

    hal_psram_mc_flush_all_fifo(id);
    psram_mc[id]->REG_008 = PSRAM_MC_MGR_ADDR(mr);
    hal_psram_mc_set_cmd_mrr(id);
    while (hal_psram_mc_get_rx_fifo_empty_cnt(id) == 1) {}
    hal_psram_mc_read_rx_fifo(id, &val, 1);

    return val;
}

void hal_psram_init(void)
{
    hal_cache_wrap_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_wrap_enable(HAL_CACHE_ID_D_CACHE);

    psram_init();

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_enable(HAL_PSRAM_ID_0);
#endif

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_enable(HAL_PSRAM_ID_0);
#endif
}

void hal_psram_mc_enable_dyn_freq(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
#ifdef PSRAM_DYN_FP_SUPPORT
    psram_mc[id]->REG_200 = PSRAM_MC_DYN_FREQ_MR_VLD(0x7);
    hal_psram_set_timing_0(id, HAL_PSRAM_DYN_FREQ_ID_0, psram_cfg, psram_run_clk);
    hal_psram_mc_set_timing_1(id, psram_cfg, PSRAM_SPEED_1);
    hal_psram_set_timing_0(id, HAL_PSRAM_DYN_FREQ_ID_1, psram_cfg, PSRAM_SPEED_1);
#endif
}

void hal_psram_phy_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    hal_cmu_psram_phy_sleep();
}

void hal_psram_phy_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    int dual;
    if (hal_psram_mc_is_reset(HAL_PSRAM_ID_0)) {
        dual = 1;
    } else if (psram_mc[0]->REG_000 & (PSRAM_MC_CHIP_BIT | PSRAM_MC_CHIP_X16)) {
        dual = 1;
    } else {
        dual = 0;
    }

    hal_cmu_psram_phy_wakeup(dual);
}

void hal_psram_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    uint32_t val;
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
#ifdef PSRAM_DYN_FP_SUPPORT
        psram_save_dyn_freq_sts = psram_mc[HAL_PSRAM_ID_0]->REG_200;
#endif
    }
#endif

    if (hal_psram_mc_is_sleep(HAL_PSRAM_ID_0) == true)
        return;

    hal_psram_mc_wb_busy_wait(HAL_PSRAM_ID_0);
    hal_psram_mc_busy_wait(HAL_PSRAM_ID_0);

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_disable(HAL_PSRAM_ID_0);
#endif

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_disable(HAL_PSRAM_ID_0);
    // psram auto wakeup need manual send pd cmd
    hal_psram_mc_set_cmd_pd(HAL_PSRAM_ID_0);
#else
    psram_mc[HAL_PSRAM_ID_0]->REG_024 =
        PSRAM_BIT_CONFIG(1, PSRAM_MC_AUTOWAKEUP_EN) |    //1
        PSRAM_BIT_CONFIG(0, PSRAM_MC_ENTRY_SELF_REFRESH_IDLE) |
        PSRAM_BIT_CONFIG(1, PSRAM_MC_ENTRY_SLEEP_IDLE);
#endif

    do {
        val = GET_BITFIELD(psram_mc[HAL_PSRAM_ID_0]->REG_404, PSRAM_MC_CP_FSM_STATE);
    } while (val != PSRAM_MC_FSM_PD_MODE);

    hal_sys_timer_delay_us(10);

    psram_sleep_time = hal_sys_timer_get();
}

void hal_psram_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    if (0) {
#ifdef CORE_SLEEP_POWER_DOWN
    } else if ((mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) && hal_psram_mc_is_reset(HAL_PSRAM_ID_0)) {
        /// auto_pd --> auto_sr
        uint32_t val;

        psram_wakeup_init();

        val = psram_mc[HAL_PSRAM_ID_0]->REG_080;
        if (hal_sys_timer_get() - psram_sleep_time >= US_TO_TICKS(150) + 1) {
            psram_mc[HAL_PSRAM_ID_0]->REG_080 = PSRAM_MC_T_HS_0(0);
        }

        hal_psram_mc_set_cmd_nop(HAL_PSRAM_ID_0);

        psram_mc[HAL_PSRAM_ID_0]->REG_080 = val;
        hal_sys_timer_delay_us(200);
#endif
    } else {

    }

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_enable(HAL_PSRAM_ID_0);
#else
    uint32_t status;

    psram_mc[HAL_PSRAM_ID_0]->REG_024 =
        PSRAM_BIT_CONFIG(1, PSRAM_MC_AUTOWAKEUP_EN) |    //1
        PSRAM_BIT_CONFIG(0, PSRAM_MC_ENTRY_SELF_REFRESH_IDLE) |
        PSRAM_BIT_CONFIG(0, PSRAM_MC_ENTRY_SLEEP_IDLE);

    do {
        status = GET_BITFIELD(psram_mc[HAL_PSRAM_ID_0]->REG_404, PSRAM_MC_CP_FSM_STATE);
    } while (status != PSRAM_MC_FSM_IDLE_MODE);
#endif

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_enable(HAL_PSRAM_ID_0);
#endif

#if defined(PSRAM_DYN_FP_SUPPORT) && defined(CORE_SLEEP_POWER_DOWN)
    if (!!(psram_save_dyn_freq_sts & PSRAM_MC_DYN_FREQ_STS)) {
        hal_psram_dyn_freq_select(HAL_PSRAM_ID_0, HAL_PSRAM_DYN_FREQ_ID_1, 1);
    }
#endif
}

void hal_psram_lowpower_enable(enum HAL_PSRAM_ID_T id)
{
#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_enable(id);
#endif

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_enable(id);
#endif
}

void hal_psram_lowpower_disable(enum HAL_PSRAM_ID_T id)
{
#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_disable(id);
#endif

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_disable(id);
#endif
}

#endif

