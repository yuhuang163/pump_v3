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
#if defined(CHIP_HAS_PSRAM) && (CHIP_PSRAM_CTRL_VER == 11)

#include "hal_psram.h"

#ifdef PSRAM_CTRL_BASE

#include "reg_psram_mc_v11.h"
#include "hal_psramip.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "psram_debug.h"

#define TX_FIFO_DEPTH                       8
#define RX_FIFO_DEPTH                       8

// #define PSRAM_AUTO_LOW_POWER
// #define PSRAM_CLOCK_GATE_ENABLE
#define PSRAM_BURST_FIXED

#define PSRAM_QUEUE_IDLE_TIME_US            (100)

enum MEMIF_CMD_T {
    MEMIF_NO_CMD        = 0x00,
    MEMIF_WRITE         = 0x01,
    MEMIF_READ          = 0x02,
    MEMIF_MRS           = 0x05,
    MEMIF_MRR           = 0x06,
    MEMIF_REF           = 0x08,
    MEMIF_SREF          = 0x09,
    MEMIF_PD            = 0x10,
    MEMIF_NOP           = 0x20,
    MEMIF_RST           = 0xFF,
    MEMIF_ZQCL          = 0x85,
    MEMIF_ZQCS          = 0x45,
    MEMIF_ZQCRST        = 0x25,
    MEMIF_START_CLOCK   = 0x40,
    MEMIF_STOP_CLOCK    = 0x80,
    MEMIF_NEW_CMD       = 0x7F,
    MEMIF_BUS_HALT      = 0xF0,
};

enum CP_FSM_STATE_T {
    CP_FSM_STATE_PD             = 2,
    CP_FSM_STATE_READY          = 4,
};

struct PSRAM_MC_T *const psram_mc[1] = {(struct PSRAM_MC_T  *)PSRAM_CTRL_BASE};
POSSIBLY_UNUSED static const uint32_t psram_run_clk = PSRAM_SPEED;

void hal_psram_mc_set_freq(enum HAL_PSRAM_ID_T id, uint32_t clk)
{
    hal_cmu_mem_set_freq(2 * clk);
}

void hal_psram_mc_init_complete(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_400 = PSRAM_ULP_MC_INIT_COMPLETE;
}

void hal_psram_mc_set_cmd(enum HAL_PSRAM_ID_T id, enum MEMIF_CMD_T cmd)
{
    psram_mc[id]->REG_004 = cmd;
}

int hal_psram_mc_busy(enum HAL_PSRAM_ID_T id)
{
    return !!(psram_mc[id]->REG_404 & PSRAM_ULP_MC_BUSY);
}

void hal_psram_mc_busy_wait(enum HAL_PSRAM_ID_T id)
{
    while (hal_psram_mc_busy(id));
}

static int hal_psram_mc_is_wb_busy(enum HAL_PSRAM_ID_T id)
{
    return !!(psram_mc[id]->REG_404 & PSRAM_ULP_MC_WB_FILL_LEVEL_MASK);
}

void hal_psram_mc_wb_busy_wait(enum HAL_PSRAM_ID_T id)
{
    while (hal_psram_mc_is_wb_busy(id));
}

bool hal_psram_mc_is_sleep(enum HAL_PSRAM_ID_T id)
{
    return !!(GET_BITFIELD(psram_mc[id]->REG_404, PSRAM_ULP_MC_CP_FSM_STATE) == CP_FSM_STATE_PD);
}

bool hal_psram_mc_is_ready(enum HAL_PSRAM_ID_T id)
{
    return !!(GET_BITFIELD(psram_mc[HAL_PSRAM_ID_0]->REG_404, PSRAM_ULP_MC_CP_FSM_STATE) == CP_FSM_STATE_READY);
}

void hal_psram_mc_xfer_addr_len(enum HAL_PSRAM_ID_T id, uint32_t addr, uint32_t len)
{
    psram_mc[id]->REG_008 = addr;
    psram_mc[id]->REG_00C = SET_BITFIELD(psram_mc[id]->REG_00C, PSRAM_ULP_MC_MGR_LEN, len);
}

void hal_psram_mc_flush_all_fifo(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_busy_wait(id);
    psram_mc[id]->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR | PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psram_mc_busy_wait(id);
}

void hal_psram_mc_write_fifo(enum HAL_PSRAM_ID_T id, uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        psram_mc[id]->REG_014 = *data++;
    }
}

uint32_t hal_psram_mc_get_tx_fifo_free_len(enum HAL_PSRAM_ID_T id)
{
    return GET_BITFIELD(psram_mc[id]->REG_404, PSRAM_ULP_MC_MGR_TXFIFO_EMPTY_CNT);
}

int hal_psram_mc_rx_fifo_empty(enum HAL_PSRAM_ID_T id)
{
    return !!(psram_mc[id]->REG_404 & PSRAM_ULP_MC_MGR_RXFIFO_R_EMPTY);
}

void hal_psram_mc_read_fifo(enum HAL_PSRAM_ID_T id, uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        *data++ = psram_mc[id]->REG_018;
    }
}

void xhal_psram_mc_read_fifo(enum HAL_PSRAM_ID_T id, uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        *data++ = psram_mc[id]->REG_018;
    }
}

void hal_psram_mc_clock_gate_disable(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_0C4 = PSRAM_ULP_MC_CACTIVE_MASK(0xff);
}

void hal_psram_mc_auto_lowpower_disable(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_024 &= ~PSRAM_ULP_MC_ENTRY_PHY_DOWN_IDLE;
    hal_psram_mc_busy_wait(id);
}

void hal_psram_mc_auto_lowpower_enable(enum HAL_PSRAM_ID_T id)
{
#ifdef PSRAM_AUTO_LOW_POWER
    psram_mc[id]->REG_0A4 = PSRAM_ULP_MC_QUEUE_IDLE_CYCLE(psram_run_clk * PSRAM_QUEUE_IDLE_TIME_US);

    psram_mc[id]->REG_024 |= PSRAM_ULP_MC_ENTRY_PHY_DOWN_IDLE;
    hal_psram_mc_busy_wait(id);
#endif
}

void hal_psram_mc_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val = 0;

    val  = psram_mc[id]->REG_000;
    val |= SET_BITFIELD(val, PSRAM_ULP_MC_CHIP_CA_PATTERN, 0x1) |
        PSRAM_BIT_CONFIG((psram_cfg->psram_type == PSRAM_TYPE_HYPERRAM), PSRAM_ULP_MC_HYPERBUS) |
        PSRAM_BIT_CONFIG((psram_cfg->psram_type == PSRAM_TYPE_HYPERRAM), PSRAM_ULP_MC_HYPERBUS_VAR_LAT) |
        PSRAM_ULP_MC_OPI_MODE;
    psram_mc[id]->REG_000 = val;

    val = psram_mc[id]->REG_0A0;
    val &= ~(PSRAM_ULP_MC_STOP_CLK_IN_NOP | PSRAM_ULP_MC_STOP_CLK_IN_TCPH);
    val |= PSRAM_BIT_CONFIG(psram_cfg->device_cfg->stop_clk_in_nop, PSRAM_ULP_MC_STOP_CLK_IN_NOP) |
        PSRAM_BIT_CONFIG(psram_cfg->device_cfg->stop_clk_in_tcph, PSRAM_ULP_MC_STOP_CLK_IN_TCPH);
    psram_mc[id]->REG_0A0 = val;

#ifdef CHIP_BEST1502P
    if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        //1502p XOPI die is connected to CE2, need to swap CE.
        psram_mc[id]->REG_0E8 = PSRAM_ULP_MC_CHIP_SEL_DR | PSRAM_ULP_MC_CHIP_SEL;
        psram_mc[id]->REG_0D4 = PSRAM_ULP_MC_CHIP_DDP | PSRAM_ULP_MC_CHIP_DDP_CLK_COMMON;
    }
#endif

    psram_mc[id]->REG_024 = PSRAM_ULP_MC_AUTOWAKEUP_EN |
        PSRAM_ULP_MC_PHY_AUTO_PD_EN |
        PSRAM_ULP_MC_MGR_AUTO_WAKEUP_EN;

    psram_mc[id]->REG_0D8 = PSRAM_ULP_MC_MRS_AS_PD_EN |
        PSRAM_ULP_MC_MRS_AS_PD_WRLAT_DR |
        PSRAM_ULP_MC_MRS_AS_PD_MR(psram_cfg->device_timing->mrs_as_pd_mr) |
        PSRAM_ULP_MC_MRS_AS_PD_MD(psram_cfg->device_timing->mrs_as_pd_md);

    val = psram_mc[id]->REG_034;
#ifdef PSRAM_BURST_FIXED
    //3: fixed 128B  2: fixed 64B  1: fix 32B
    val = SET_BITFIELD(val, PSRAM_ULP_MC_BURST_LENGTH, 2);
#else
    val = SET_BITFIELD(val, PSRAM_ULP_MC_BURST_LENGTH, 5);
#endif
    val = SET_BITFIELD(val, PSRAM_ULP_MC_PAGE_BOUNDARY, psram_cfg->device_cfg->page_boundary);
    psram_mc[id]->REG_034 = val;

    psram_mc[id]->REG_038 = PSRAM_BIT_CONFIG(0, PSRAM_ULP_MC_BUS_WIDTH);

    psram_mc[id]->REG_03C = PSRAM_ULP_MC_HIGH_PRI_LEVEL(7);

    psram_mc[id]->REG_040 = PSRAM_ULP_MC_WRAP_CRT_RET_EN;

#ifdef PSRAM_BURST_FIXED
    psram_mc[id]->REG_040 = 0;
    psram_mc[id]->REG_044 = PSRAM_ULP_MC_BUFFERABLE_WB_EN | PSRAM_ULP_MC_RD_CANCEL_EN;
#else
    psram_mc[id]->REG_044 = 0;
#endif

    psram_mc[id]->REG_048 = PSRAM_ULP_MC_FRE_RATIO(0);

    psram_mc[id]->REG_010 = PSRAM_ULP_MC_MGR_WSTRB(0xF0);
}

void hal_psram_set_clk_freq(enum HAL_PSRAM_ID_T id, uint32_t clk)
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

void hal_psram_mc_snoop_disable(enum HAL_PSRAM_ID_T id)
{
    psram_mc[id]->REG_044 |= PSRAM_ULP_MC_SNP_DISABLE;
}

void hal_psram_mc_set_cmd_nop(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_NOP);
}

void hal_psram_mc_set_cmd_pd(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_PD);
}

void hal_psram_mc_set_cmd_halt(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_BUS_HALT);
}

void hal_psram_mc_wl_get(enum HAL_PSRAM_ID_T id, uint32_t *val)
{
    *val = psram_mc[id]->REG_028;
}

void hal_psram_mc_wl_set(enum HAL_PSRAM_ID_T id, uint32_t val)
{
    psram_mc[id]->REG_028 = val;
}

void hal_psram_mc_var_lat_en(enum HAL_PSRAM_ID_T id, bool v)
{
    uint32_t bit_file;
    volatile uint32_t * p_reg;

    bit_file = PSRAM_ULP_MC_HYPERBUS_VAR_LAT;
    p_reg = &psram_mc[id]->REG_000;

    if (v) {
        *p_reg |= bit_file;
    } else {
        *p_reg &= ~bit_file;
    }
}

void hal_psram_mc_set_default_param(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->device_timing->is_mrs_wl_fixed) {
        psram_mc[id]->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(psram_cfg->device_timing->fixed_wl_mrs);
    } else {
        psram_mc[id]->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(psram_cfg->device_timing->wl_def);
    }

    psram_mc[id]->REG_02C = PSRAM_ULP_MC_READ_LATENCY(psram_cfg->device_timing->rl_min_def);
}

void hal_psram_mc_set_timing(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t freq_mhz, bool config_latency)
{
    POSSIBLY_UNUSED uint32_t item = 0;
    uint32_t i;

    for (i = 0; i < psram_cfg->device_timing->ac_timing_count; i++) {
        if ((freq_mhz <= 26) && (psram_cfg->device_timing->ac_timing[i].freq_mhz > 26)) {
            item = i - 1;
            break;
        } else if ((freq_mhz <= 52) && (psram_cfg->device_timing->ac_timing[i].freq_mhz > 52)) {
            item = i - 1;
            break;
        } else if (psram_cfg->device_timing->ac_timing[i].freq_mhz == 200) {
            item = i;
            break;
        }
    }

    if (config_latency) {
        psram_mc[id]->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(psram_cfg->device_timing->ac_timing[item].wl);
        psram_mc[id]->REG_02C = PSRAM_ULP_MC_READ_LATENCY(psram_cfg->device_timing->ac_timing[item].rl_min);

        if (psram_cfg->device_timing->is_mrs_wl_fixed) {
            psram_mc[id]->REG_0D8 = SET_BITFIELD(psram_mc[id]->REG_0D8, PSRAM_ULP_MC_MRS_AS_PD_WRLAT, psram_cfg->device_timing->fixed_wl_mrs);
        } else {
            psram_mc[id]->REG_0D8 = SET_BITFIELD(psram_mc[id]->REG_0D8, PSRAM_ULP_MC_MRS_AS_PD_WRLAT, psram_cfg->device_timing->ac_timing[item].wl);
        }
    }

    psram_mc[id]->REG_058 = PSRAM_ULP_MC_T_CPHR(psram_cfg->device_timing->ac_timing[item].t_cphr);
    psram_mc[id]->REG_068 = PSRAM_ULP_MC_T_MRR(psram_cfg->device_timing->ac_timing[item].t_mrr);

    psram_mc[id]->REG_060 = PSRAM_ULP_MC_T_CPHW(psram_cfg->device_timing->ac_timing[item].t_cphw);
    psram_mc[id]->REG_06C = 20;

    psram_mc[id]->REG_050 = PSRAM_ULP_MC_T_RC(psram_cfg->device_timing->ac_timing[item].t_rc);
    psram_mc[id]->REG_070 = PSRAM_ULP_MC_T_CEM(psram_cfg->device_timing->ac_timing[item].t_cem);
    psram_mc[id]->REG_074 = PSRAM_ULP_MC_T_RST(psram_cfg->device_timing->ac_timing[item].t_rst);
    psram_mc[id]->REG_080 = PSRAM_ULP_MC_T_HS(psram_cfg->device_timing->ac_timing[item].t_hs);
    psram_mc[id]->REG_084 = PSRAM_ULP_MC_T_XPHS(psram_cfg->device_timing->ac_timing[item].t_xphs);
    psram_mc[id]->REG_088 = PSRAM_ULP_MC_T_XHS(psram_cfg->device_timing->ac_timing[item].t_xhs);
    psram_mc[id]->REG_09C = PSRAM_ULP_MC_WR_DMY_CYC(psram_cfg->device_cfg->wr_dummy_cyc);

    psram_mc[id]->REG_0A0 |= PSRAM_ULP_MC_NOP_DMY_CYC(psram_cfg->device_timing->ac_timing[item].t_xphs);
}

void hal_psram_device_mr_write(enum HAL_PSRAM_ID_T id, uint32_t mr, uint32_t val)
{
    hal_psram_mc_flush_all_fifo(id);
    hal_psram_mc_write_fifo(id, &val, 1);
    hal_psram_mc_xfer_addr_len(id, mr, 1);
    hal_psram_mc_set_cmd(id, MEMIF_MRS);
    while (hal_psram_mc_get_tx_fifo_free_len(id) != TX_FIFO_DEPTH);
    hal_psram_mc_busy_wait(id);
}

uint32_t hal_psram_device_mr_read(enum HAL_PSRAM_ID_T id, uint32_t mr)
{
    uint32_t val;
    hal_psram_mc_flush_all_fifo(id);
    hal_psram_mc_xfer_addr_len(id, mr, 1);
    hal_psram_mc_set_cmd(id, MEMIF_MRR);
    while (hal_psram_mc_rx_fifo_empty(id));
    hal_psram_mc_read_fifo(id, &val, 1);
    return val;
}

void hal_psram_mc_set_cmd_rst(enum HAL_PSRAM_ID_T id)
{
    hal_psram_mc_set_cmd(id, MEMIF_RST);
}

void hal_psram_mc_wait_write_buffer_empty(enum HAL_PSRAM_ID_T id)
{
    uint32_t level;
    uint32_t start = hal_sys_timer_get();

    __DSB();
    do {
        level = GET_BITFIELD(psram_mc[id]->REG_404, PSRAM_ULP_MC_WB_FILL_LEVEL);
        if (level == 0) {
            break;
        }
    } while (hal_sys_timer_get() - start < MS_TO_TICKS(10));

    // ASSERT(level == 0, "%s: Drain timeout: level=%u status=0x%08X", __func__, level, psram_mc[id]->REG_404);
}
void hal_psram_wait_write_buffer_empty(enum HAL_PSRAM_ID_T id) __attribute__((alias("hal_psram_mc_wait_write_buffer_empty")));

uint32_t hal_psram_mc_set_mgr_ch(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_MC_MGR_CHANEL_T c)
{
    return 0Xfa11;
}

void hal_psram_mc_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    uint32_t val;

    hal_psram_mc_wb_busy_wait(HAL_PSRAM_ID_0);
    hal_psram_mc_busy_wait(HAL_PSRAM_ID_0);

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_disable(HAL_PSRAM_ID_0);
#endif

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_disable(HAL_PSRAM_ID_0);
#endif

    do {
        val = GET_BITFIELD(psram_mc[HAL_PSRAM_ID_0]->REG_404, PSRAM_ULP_MC_CP_FSM_STATE);
    } while (val != CP_FSM_STATE_READY);
}

void hal_psram_mc_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_mc_auto_lowpower_enable(HAL_PSRAM_ID_0);
#endif

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_mc_clock_gate_enable(HAL_PSRAM_ID_0);
#endif
}

void hal_psram_mc_clock_gate_enable(enum HAL_PSRAM_ID_T id)
{
#ifdef PSRAM_CLOCK_GATE_ENABLE
    psram_mc[id]->REG_0C4 = PSRAM_ULP_MC_CACTIVE_MASK(0);
#endif
}

uint32_t hal_psram_mc_mrs_pre_operation(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t wl;

    hal_psram_mc_wl_get(id, &wl);

    if (psram_cfg->device_timing->is_mrs_wl_fixed) {
        hal_psram_mc_wl_set(id, psram_cfg->device_timing->fixed_wl_mrs);
        hal_psram_mc_busy_wait(id);
    }

    return wl;
}

void hal_psram_mc_mrs_post_operation(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg, uint32_t wl)
{
    if (psram_cfg->device_timing->is_mrs_wl_fixed) {
        hal_psram_mc_wl_set(id, wl);
        hal_psram_mc_busy_wait(id);
    }
}

bool hal_psram_mc_is_reset(enum HAL_PSRAM_ID_T id)
{
    return !(psram_mc[id]->REG_400 & PSRAM_ULP_MC_INIT_COMPLETE);
}

void hal_psram_mc_set_cmd_start_clock(enum HAL_PSRAM_ID_T id)
{
}

void hal_psram_mc_set_cmd_stop_clock(enum HAL_PSRAM_ID_T id)
{
}

void hal_psram_mc_enable_dyn_freq(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
}

static void hal_psram_change_speed(enum HAL_PSRAM_ID_T id, uint32_t psram_speed)
{
    uint32_t lock;

    lock = int_lock();

    hal_psram_mc_set_cmd_halt(id);
    __DSB();
    hal_psram_mc_busy_wait(id);

    pmu_psram_freq_config(psram_speed);
    hal_psram_mc_set_freq(id, psram_speed);

    hal_psram_mc_set_cmd_nop(id);

    int_unlock(lock);
}

void hal_psram_dyn_freq_select(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_DYN_FREQ_ID_T freq_id, uint32_t sleep)
{
    if (freq_id == HAL_PSRAM_DYN_FREQ_ID_0) {
        hal_psram_change_speed(id, 200 * 1000000);
    } else {
        hal_psram_change_speed(id, 96 * 1000000);
    }
}

void hal_psram_mc_priority_config(enum HAL_PSRAM_ID_T id)
{
}

#ifdef CORE_SLEEP_POWER_DOWN
void hal_psram_mc_param_save(enum HAL_PSRAM_ID_T id)
{
}

void hal_psram_mc_param_recover(enum HAL_PSRAM_ID_T id)
{
}
#endif

#endif

#endif
