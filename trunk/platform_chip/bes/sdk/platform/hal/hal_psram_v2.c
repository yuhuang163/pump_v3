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
#if defined(CHIP_HAS_PSRAM) && \
    (CHIP_PSRAM_CTRL_VER == 2 || CHIP_PSRAM_CTRL_VER == 3 || CHIP_PSRAM_CTRL_VER == 5 || \
    CHIP_PSRAM_CTRL_VER == 7 || CHIP_PSRAM_CTRL_VER == 8)

#include "plat_types.h"
#include "plat_addr_map.h"

#ifdef PSRAM_CTRL_BASE

#include "cmsis.h"
#include "hal_location.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "pmu.h"
#include "reg_psram_mc_v2.h"
#include "reg_psram_phy_v2.h"
#include "string.h"
#ifdef CHIP_BEST1502X
#include "analog.h"
#endif

#define PSRAM_RESET
//#define PSRAM_DUAL_8BIT
#ifdef CHIP_BEST2003
    #ifndef PSRAM_APS3208K
        #define PSRAM_WRAP_ENABLE
    #endif
#endif

#if (CHIP_PSRAM_CTRL_VER >= 5)
#define SLEEP_WITH_DIGPHY
// #define PSRAM_AUTO_LOW_POWER
// #define PSRAM_CLOCK_GATE_ENABLE
//#define PSRAM_BURST_FIXED
#endif

#ifdef PSRAM_AUTO_LOW_POWER
#ifdef PSRAM_WINDOW_TEST
#undef PSRAM_AUTO_LOW_POWER
#else
#ifndef SLEEP_WITH_DIGPHY
#define SLEEP_WITH_DIGPHY
#endif
#define PSRAM_QUEUE_IDLE_TIME_US            (100)
#endif
#endif

#ifdef PSRAM_BURST_FIXED
#ifdef PSRAM_WRAP_ENABLE
#undef PSRAM_WRAP_ENABLE
#endif
#endif

#ifndef PSRAM_PAGE_SIZE
#ifdef PSRAM_XCCELA_MODE
// APS128xxN APS256xxN APS512xxN
#define PSRAM_PAGE_SIZE                     2048
#else /* PSRAM_XCCELA_MODE */
// APS3208K APS6408L
#define PSRAM_PAGE_SIZE                     1024
#endif /* PSRAM_XCCELA_MODE */
#endif

#ifdef PSRAM_DEBUG
    #define PSRAM_TRACE TRACE_IMM
#else
    #define PSRAM_TRACE(...)
#endif

#ifdef PSRAM_DEBUG
#define PSRAM_TRACENOCRLF_NOTS          TRACE_NOCRLF_NOTS
#define PSRAM_TRACE_NOTS                TRACE_NOTS
#define PSRAM_TRACE_LINEFEED            TRACE_OUTPUT_LINEFEED
#else
#define PSRAM_TRACENOCRLF_NOTS(...)
#define PSRAM_TRACE_NOTS(...)
#define PSRAM_TRACE_LINEFEED()
#endif


#define TX_FIFO_DEPTH                       8
#define RX_FIFO_DEPTH                       8

#define HAL_PSRAM_SWAP8BITS(val)            (((val & 0xff00) >> 8) | ((val & 0xff) << 8))
#define HAL_PSRAM_WR_TIMEOUT_US             (1000)
#define HAL_PSRAM_DATA_READ_TIME_OUT        (-1)
#define HAL_PSRAM_MRR_READ_TIME_OUT         (-2)
#define HAL_PSRAM_DATA_WRITE_TIME_OUT       (-3)
#define HAL_PSRAM_DATA_MRS_TIME_OUT         (-4)

// MR0
#define MR0_DRIVE_STR_SHIFT                 0
#define MR0_DRIVE_STR_MASK                  (0x3 << MR0_DRIVE_STR_SHIFT)
#define MR0_DRIVE_STR(n)                    BITFIELD_VAL(MR0_DRIVE_STR, n)
#define MR0_READ_LATENCY_SHIFT              2
#define MR0_READ_LATENCY_MASK               (0x7 << MR0_READ_LATENCY_SHIFT)
#define MR0_READ_LATENCY(n)                 BITFIELD_VAL(MR0_READ_LATENCY, n)
#define MR0_LT                              (1 << 5)
#define MR0_FIXED_00_SHIFT                  6
#define MR0_FIXED_00_MASK                   (0x3 << MR0_FIXED_00_SHIFT)
#define MR0_FIXED_00(n)                     BITFIELD_VAL(MR0_FIXED_00, n)

// MR1
#define MR1_VENDOR_ID_SHIFT                 0
#define MR1_VENDOR_ID_MASK                  (0x1F << MR1_VENDOR_ID_SHIFT)
#define MR1_VENDOR_ID(n)                    BITFIELD_VAL(MR1_VENDOR_ID, n)
#define MR1_DENSITY_SHIFT                   5
#define MR1_DENSITY_MASK                    (0x3 << MR1_DENSITY_SHIFT)
#define MR1_DENSITY(n)                      BITFIELD_VAL(MR1_DENSITY, n)
#define MR1_ULP                             (1 << 7)

// MR2
#define MR2_VENDOR_ID_SHIFT                 0
#define MR2_VENDOR_ID_MASK                  (0x7 << MR2_VENDOR_ID_SHIFT)
#define MR2_VENDOR_ID(n)                    BITFIELD_VAL(MR2_VENDOR_ID, n)
#define MR2_DEV_ID_SHIFT                    3
#define MR2_DEV_ID_MASK                     (0x3 << MR2_DEV_ID_SHIFT)
#define MR2_DEV_ID(n)                       BITFIELD_VAL(MR2_DEV_ID, n)
#define MR2_RSVD                            (1 << 5)
#define MR2_FIXED_1                         (1 << 6)
#define MR2_GB                              (1 << 7)

// MR4
#define MR4_PASR_SHIFT                      0
#define MR4_PASR_MASK                       (0x7 << MR4_PASR_SHIFT)
#define MR4_PASR(n)                         BITFIELD_VAL(MR4_PASR, n)
#define MR4_RF_RATE_HALF                    (3)
#define MR4_RF_RATE_1X                      (1)
#define MR4_RF_RATE_4X                      (0)
#define MR4_RF_RATE_SHIFT                   (3)
#define MR4_RF_RATE_MASK                    (0x3 << MR4_RF_RATE_SHIFT)
#define MR4_RF_RATE(n)                      BITFIELD_VAL(MR4_RF_RATE, n)
#define MR4_WRITE_LATENCY_SHIFT             5
#define MR4_WRITE_LATENCY_MASK              (0x7 << MR4_WRITE_LATENCY_SHIFT)
#define MR4_WRITE_LATENCY(n)                BITFIELD_VAL(MR4_WRITE_LATENCY, n)

// MR6
#define MR6_RSVD_SHIFT                      0
#define MR6_RSVD_MASK                       (0xF << MR6_RSVD_SHIFT)
#define MR6_RSVD(n)                         BITFIELD_VAL(MR6_RSVD, n)
#define MR6_HALF_SLEEP_SHIFT                4
#define MR6_HALF_SLEEP_MASK                 (0xF << MR6_HALF_SLEEP_SHIFT)
#define MR6_HALF_SLEEP(n)                   BITFIELD_VAL(MR6_HALF_SLEEP, n)

// MR8
#define MR8_BL_SHIFT                        0
#define MR8_BL_MASK                         (0x3 << MR8_BL_SHIFT)
#define MR8_BL(n)                           BITFIELD_VAL(MR8_BL, n)
#define MR8_BT                              (1 << 2)
#define MR8_FIXED_0                         (1 << 3)
#define MR8_RSVD_SHIFT                      4
#define MR8_RSVD_MASK                       (0x7 << MR8_RSVD_SHIFT)
#define MR8_RSVD(n)                         BITFIELD_VAL(MR8_RSVD, n)
#define MR8_FIXED_00                        (1 << 7)

enum PSRAM_CMD_T {
    PSRAM_CMD_SYNC_READ     = 0x00,
    PSRAM_CMD_SYNC_WRITE    = 0x80,
    PSRAM_CMD_4BYTE_READ    = 0x3F,
    PSRAM_CMD_4BYTE_WRITE   = 0xBF,
    PSRAM_CMD_REG_READ      = 0x40,
    PSRAM_CMD_REG_WRITE     = 0xC0,
    PSRAM_CMD_GLOBAL_RESET  = 0xFF,
};

enum CP_FSM_STATE_T {
    CP_FSM_STATE_SELF_REFRESH   = 1,
    CP_FSM_STATE_PD             = 2,
    CP_FSM_STATE_READY          = 4,
};

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
};

static struct PSRAM_MC_T *const psram_mc = (struct PSRAM_MC_T *)PSRAM_CTRL_BASE;
static struct PSRAM_PHY_T *const psram_phy = (struct PSRAM_PHY_T *)(PSRAM_CTRL_BASE + 0x8000);

static bool phy_bypass = 0;

#ifdef CORE_SLEEP_POWER_DOWN
uint32_t psramphy_save_reg_054 = 0;
uint32_t psramphy_save_range = 0;
#endif

#ifdef FPGA
    static const uint32_t psram_cfg_clk = 20 * 1000 * 1000;
    static const uint32_t psram_run_clk = 20 * 1000 * 1000;
#else /*FPGA*/
    #if (PSRAM_SPEED != 0)
        static const uint32_t psram_cfg_clk = 48 * 1000 * 1000;
        static const uint32_t psram_run_clk = PSRAM_SPEED * 1000 * 1000;
    #else
        #error "invalid PSRAM_SPEED"
    #endif
#endif /*FPGA*/

static HAL_PSRAM_WINDOW_2D_CB_T hal_psram_window_2d_cb_func = NULL;

static void hal_psram_phy_set_range(uint32_t range);

void hal_psram_window_2d_cb_register(enum HAL_PSRAM_ID_T id, HAL_PSRAM_WINDOW_2D_CB_T func)
{
    hal_psram_window_2d_cb_func = func;
}

static uint32_t hal_psram_phy_read_reg(__I uint32_t *addr)
{
#if defined(CHIP_BEST2003) || defined(CHIP_BEST2006)
    *addr;
#endif
    return *addr;
}

static void psram_chip_timing_config(uint32_t clk, bool psram_first);

int hal_psramip_mc_busy(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_BUSY);
}

static int hal_psramip_wb_busy(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_WB_FILL_LEVEL_MASK);
}

int hal_psramip_mc_in_sleep(void)
{
    return GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_CP_FSM_STATE) == CP_FSM_STATE_PD;
}

int hal_psramip_rx_fifo_empty(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_MGR_RXFIFO_R_EMPTY);
}

int hal_psramip_tx_fifo_full(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_MGR_TXFIFO_W_FULL);
}

uint32_t hal_psramip_get_rx_fifo_len(void)
{
    return GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_MGR_RXFIFO_FULL_CNT);
}

uint32_t hal_psramip_get_tx_fifo_free_len(void)
{
    return GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_MGR_TXFIFO_EMPTY_CNT);
}

void hal_psramip_mc_busy_wait(void)
{
    while (hal_psramip_mc_busy());
}

void hal_psramip_wb_busy_wait(void)
{
    while (hal_psramip_wb_busy());
}

void hal_psramip_flush_tx_fifo(void)
{
    hal_psramip_mc_busy_wait();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR;
    hal_psramip_mc_busy_wait();
}

void hal_psramip_flush_rx_fifo(void)
{
    hal_psramip_mc_busy_wait();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psramip_mc_busy_wait();
}

void hal_psramip_flush_all_fifo(void)
{
    hal_psramip_mc_busy_wait();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR | PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psramip_mc_busy_wait();
}

void hal_psramip_xfer_addr_len(uint32_t addr, uint32_t len)
{
    psram_mc->REG_008 = addr;
    psram_mc->REG_00C = len;
}

void hal_psramip_write_fifo(uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        psram_mc->REG_014 = *data++;
    }
}

void hal_psramip_read_fifo(uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        *data++ = psram_mc->REG_018;
    }
}

void hal_psramip_set_reg_data_mask(void)
{
#ifdef PSRAM_DUAL_8BIT
    psram_mc->REG_010 = 0xFC;
#else
    psram_mc->REG_010 = 0xFE;
#endif
}

void hal_psramip_set_mem_data_mask(void)
{
    psram_mc->REG_010 = 0;
}

void hal_psramip_set_cmd(enum MEMIF_CMD_T cmd)
{
    psram_mc->REG_004 = cmd;
}

POSSIBLY_UNUSED static bool hal_psramip_is_mc_reset(void)
{
    return !(psram_mc->REG_400 & PSRAM_ULP_MC_INIT_COMPLETE);
}

POSSIBLY_UNUSED static bool hal_psramip_is_auto_lp_support(void)
{
    bool ret;

    ret = false;
#if (CHIP_PSRAM_CTRL_VER >= 5)
    if (0) {
#ifdef CHIP_BEST1502X
    } else if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2) {
#else
    } else {
#endif
        ret = true;
    }
#endif

    return ret;
}

POSSIBLY_UNUSED static bool hal_psramip_is_dig_pd_support(void)
{
    return hal_psramip_is_auto_lp_support();
}

POSSIBLY_UNUSED static bool hal_psramip_is_fixburst_support(void)
{
    return hal_psramip_is_auto_lp_support();
}

static void hal_psramip_phy_ceb_pu_set(enum HAL_PSRAM_PHY_CHANNEL_T c)
{
#if (CHIP_PSRAM_CTRL_VER >= 3)
#if (defined(CHIP_BEST2003) || defined(CHIP_BEST2006) || defined(CHIP_BEST1502X))
#ifdef CHIP_BEST1502X
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2) {
        psram_mc ->REG_200 |= PSRAM_ULP_MC_PHY_REG_CEB_PU;
#else
    if (pmu_get_chip_model() >= CHIP_MODEL_2006) {
        psram_mc ->REG_200 |= PSRAM_ULP_MC_PHY_REG_CEB_PU_2006;
    } else if (pmu_get_chip_model() >= CHIP_MODEL_2003I) {
        psram_mc ->REG_200 |= PSRAM_ULP_MC_PHY_REG_CEB_PU_2003;
#endif
    } else {
        // Do not support yet
    }
#else
    uint32_t val;

    if (c & HAL_PSRAM_PHY_CHANNEL_0) {
        val = hal_psram_phy_read_reg(&psram_phy ->REG_068);
        val |= PSRAM_ULP_PHY_REG_CEB1_PU;
        psram_phy ->REG_068 = val;
    }
    if (c & HAL_PSRAM_PHY_CHANNEL_1) {
        val = hal_psram_phy_read_reg(&psram_phy ->REG_068);
        val |= PSRAM_ULP_PHY_REG_CEB2_PU;
        psram_phy ->REG_068 = val;
    }
#endif
#endif
}

POSSIBLY_UNUSED static void hal_psramip_phy_ceb_pu_clr(enum HAL_PSRAM_PHY_CHANNEL_T c)
{
#if (CHIP_PSRAM_CTRL_VER >= 3)
#if (defined(CHIP_BEST2003) || defined(CHIP_BEST2006) || defined(CHIP_BEST1502X))
#ifdef CHIP_BEST1502X
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2) {
        psram_mc ->REG_200 &= ~PSRAM_ULP_MC_PHY_REG_CEB_PU;
#else
    if (pmu_get_chip_model() >= CHIP_MODEL_2006) {
        psram_mc ->REG_200 &= ~PSRAM_ULP_MC_PHY_REG_CEB_PU_2006;
    } else if (pmu_get_chip_model() >= CHIP_MODEL_2003I) {
        psram_mc ->REG_200 &= ~PSRAM_ULP_MC_PHY_REG_CEB_PU_2003;
#endif
    } else {
        // Do not support yet
    }
#else
    uint32_t val;

    if (c & HAL_PSRAM_PHY_CHANNEL_0) {
        val = hal_psram_phy_read_reg(&psram_phy ->REG_068);
        val &= ~PSRAM_ULP_PHY_REG_CEB1_PU;
        psram_phy ->REG_068 = val;
    }
    if (c & HAL_PSRAM_PHY_CHANNEL_1) {
        val = hal_psram_phy_read_reg(&psram_phy ->REG_068);
        val &= ~PSRAM_ULP_PHY_REG_CEB2_PU;
        psram_phy ->REG_068 = val;
    }
#endif
#endif
}

POSSIBLY_UNUSED static void hal_psramip_hyperbus_var_lat_en(bool v)
{
    POSSIBLY_UNUSED uint32_t bit_file;
    POSSIBLY_UNUSED volatile uint32_t * p_reg;

#if (CHIP_PSRAM_CTRL_VER >= 3)
#if (defined(CHIP_BEST2003) || defined(CHIP_BEST1502X))
#ifdef CHIP_BEST1502X
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2) {
#elif defined(CHIP_BEST2003)
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_8) {
#endif
        bit_file = PSRAM_ULP_MC_HYPERBUS_LC_DETECT_DISABLE;
    } else {
        bit_file = 1 << 0;
    }
    p_reg = &psram_mc ->REG_200;
#else
    bit_file = PSRAM_ULP_MC_HYPERBUS_VAR_LAT;
    p_reg = &psram_mc ->REG_000;
#endif

    if (v) {
        *p_reg |= bit_file;
    } else {
        *p_reg &= ~bit_file;
    }
#endif
}

POSSIBLY_UNUSED static void hal_psram_phy_auto_pd_en(bool v)
{
    POSSIBLY_UNUSED volatile uint32_t *reg;
    POSSIBLY_UNUSED uint32_t bit_field;

#if (CHIP_PSRAM_CTRL_VER >= 7)
    reg = &psram_mc ->REG_024;
    bit_field = PSRAM_ULP_MC_PHY_AUTO_PD_EN;
#if (CHIP_PSRAM_CTRL_VER >= 8)
    // only phy down (no MIMF_PD CMD), when in auto low power
    bit_field |= PSRAM_ULP_MC_ENTRY_PHY_DOWN_IDLE;
#endif
#elif (CHIP_PSRAM_CTRL_VER == 5)
    reg = &psram_mc ->REG_200;
    bit_field = PSRAM_ULP_MC_AUTO_PD_ENTER_EN;
#else
    return;
#endif

    if (v) {
        *reg |= bit_field;
    } else {
        *reg &= ~bit_field;
    }
}

POSSIBLY_UNUSED void psram_read_reg(uint32_t reg, uint32_t *val)
{
    hal_psramip_flush_all_fifo();
    hal_psramip_xfer_addr_len(reg, 1);
    hal_psramip_set_cmd(MEMIF_MRR);
    while (hal_psramip_rx_fifo_empty());
    hal_psramip_read_fifo(val, 1);
}

void hal_psram_dump_mr(enum HAL_PSRAM_ID_T id)
{
    uint32_t reg, val = 0;
    uint8_t real_bit = 0;

    TRACE_IMM(0, "MR Dump:");
    for (reg = 0; reg <= 8; reg++) {
        if (5 <= reg && reg <= 7) {
            continue;
        }
        psram_read_reg(reg, &val);
        real_bit = val & 0xff;
        TRACE_IMM(0, "MR%u: 0x%02x", reg, real_bit);
    }
}

static void psram_send_cmd_reg(enum MEMIF_CMD_T cmd, uint32_t reg, uint32_t val)
{
#ifdef PSRAM_DUAL_8BIT
    val &= 0xFF;
    val |= (val << 8);
#endif
    hal_psramip_flush_all_fifo();
    //hal_psramip_set_reg_data_mask();
    hal_psramip_write_fifo(&val, 1);
    hal_psramip_xfer_addr_len(reg, 1);
    hal_psramip_set_cmd(cmd);
    while (hal_psramip_get_tx_fifo_free_len() != TX_FIFO_DEPTH);
    hal_psramip_mc_busy_wait();
    //hal_psramip_set_mem_data_mask();
}

static void psram_write_reg(uint32_t reg, uint32_t val)
{
    psram_send_cmd_reg(MEMIF_MRS, reg, val);
}

static void psram_single_cmd(enum MEMIF_CMD_T cmd)
{
    hal_psramip_flush_all_fifo();
    hal_psramip_set_cmd(cmd);
    hal_psramip_mc_busy_wait();
}

int hal_psram_mgr_read_4byte(uint32_t addr, uint32_t *val)
{
    int ret;
    uint32_t t, v;

    ret = 0;

    hal_psramip_flush_all_fifo();
    hal_psramip_xfer_addr_len(addr & 0x3ffffff, 1); //64MB MAX
    hal_psramip_set_cmd(MEMIF_READ);
    t = hal_fast_sys_timer_get();
    while (hal_psramip_rx_fifo_empty()) {
        if (FAST_TICKS_TO_US(hal_fast_sys_timer_get() - t) > HAL_PSRAM_WR_TIMEOUT_US) {
            v = hal_psram_phy_read_reg(&psram_phy->REG_004);
            v |= PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = v;
            while (hal_psramip_mc_busy()) { }
            v &= ~PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = v;
            hal_psramip_flush_all_fifo();
            ret = HAL_PSRAM_DATA_READ_TIME_OUT;
            break;
        }
    }
    if (!ret) {
        hal_psramip_read_fifo(val, 1);
    }

    return ret;
}

int hal_psram_mgr_write_4byte(uint32_t addr, uint32_t val)
{
    int ret;
    uint32_t t, v;

    ret = 0;
    hal_psramip_flush_all_fifo();
    hal_psramip_write_fifo(&val, 1);
    hal_psramip_xfer_addr_len(addr, 1);
    hal_psramip_set_cmd(MEMIF_WRITE);
    t = hal_fast_sys_timer_get();
    while (hal_psramip_get_tx_fifo_free_len() != TX_FIFO_DEPTH) {
        if (FAST_TICKS_TO_US(hal_fast_sys_timer_get() - t) > HAL_PSRAM_WR_TIMEOUT_US) {
            v = hal_psram_phy_read_reg(&psram_phy->REG_004);
            v |= PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = v;
            while (hal_psramip_mc_busy()) {}
            v &= ~PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = v;
            hal_psramip_flush_all_fifo();
            ret = HAL_PSRAM_DATA_WRITE_TIME_OUT;
            break;
        }
    }
    hal_psramip_mc_busy_wait();

    return ret;
}

static POSSIBLY_UNUSED void psram_reset(void)
{
    psram_single_cmd(MEMIF_RST);
#ifdef PSRAM_APS3208K
    hal_sys_timer_delay_us(1);
    psram_single_cmd(MEMIF_RST);
#endif
}

static void psram_set_timing(uint32_t clk)
{
    uint32_t reg;
    uint32_t val;
#ifdef PSRAM_XCCELA_MODE
    reg = 8;
#ifdef PSRAM_WRAP_ENABLE
    // Wrap 32
    val = MR8_BL(1);
#else
    // Wrap 1k
    val = MR8_BL(0x3);
#endif
    psram_write_reg(reg, val);

    reg = 0;
    if (clk <= 66000000) {
        val = 0;
    } else if (clk <= 109000000) {
        val = 1;
    } else if (clk <= 133000000) {
        val = 2;
    } else if (clk <= 166000000) {
        val = 3;
    } else {
        val = 4;
    }
    // Latency type: Variable
    val = MR0_DRIVE_STR(3) | MR0_READ_LATENCY(val);
    psram_write_reg(reg, val);

    reg = 4;
    if (clk <= 66000000) {
        val = 0;
    } else if (clk <= 109000000) {
        val = 4;
    } else if (clk <= 133000000) {
        val = 2;
    } else if (clk <= 166000000) {
        val = 6;
    } else {
        val = 1;
    }
    //Enable 0.5x Refresh for reducing current consumption
    val = MR4_PASR(0) | MR4_WRITE_LATENCY(val) | MR4_RF_RATE(MR4_RF_RATE_HALF);
    psram_write_reg(reg, val);
#elif defined(PSRAM_HYPERBUS_MODE)
    int lc = psram_mc->REG_028;
    hal_psramip_hyperbus_var_lat_en(false);
    psram_mc->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(1);
    __DSB();
    hal_psramip_mc_busy_wait();

    reg = 0x1000;
    if (clk <= 85000000) {
            val = 0xe;
        } else if (clk <= 104000000) {
            val = 0xf;
        } else if (clk <= 133000000) {
            val = 0x0;
        } else if (clk <= 166000000) {
            val = 0x1;
        } else  {
            val = 0x2;
        }
    val = (val << 4) | (0x8f07);

    psram_write_reg(reg, HAL_PSRAM_SWAP8BITS(val));
    hal_psramip_mc_busy_wait();
    hal_psramip_hyperbus_var_lat_en(true);
    psram_mc->REG_028 = lc;
    __DSB();
#else
#ifndef PSRAM_APS3208K
    reg = 8;
#ifdef PSRAM_WRAP_ENABLE
    // Wrap 32
    val = MR8_BL(1);
#else
    // Wrap 1k
    val = MR8_BL(0x3);
#endif
    psram_write_reg(reg, val);
#endif /* PSRAM_APS3208K */
    reg = 0;
    if (clk <= 66000000) {
        val = 2;
    } else if (clk <= 109000000) {
        val = 3;
    } else if (clk <= 133000000) {
        val = 4;
    } else if (clk <= 166000000) {
        val = 5;
    } else {
        val = 6;
    }
    // Latency type: Variable
    val = MR0_DRIVE_STR(3) | MR0_READ_LATENCY(val);
    psram_write_reg(reg, val);

    reg = 4;
    if (clk <= 166000000) {
        val = 0;
    } else {
        val = 4;
    }
    //Enables Slow Refresh when temperature allows for reducing current consumption
    val = MR4_PASR(0) | MR4_WRITE_LATENCY(val) | MR4_RF_RATE(MR4_RF_RATE_1X);
    psram_write_reg(reg, val);
#endif /*PSRAM_XCCELA_MODE*/
}

static void hal_psram_phy_dll_config(uint32_t clk)
{
    uint32_t phy_clk;
    uint32_t range;

#if CHIP_PSRAM_CTRL_VER >= 5
    uint32_t t_trans, t_precharge, t_pu, t_lock;
    /* warning: cycle count of controller clock. for 1502: controller clock freq = die clock freq */
#if (defined(CHIP_BEST1306P) || defined(CHIP_BEST1502P) || defined(CHIP_BEST1502X) || defined(CHIP_BEST1503) || defined(CHIP_BEST2009))
    // t_trans > 30ns
    t_trans = (clk / 1000000 * 30 + (1000 - 1)) / 1000;
    // t_precharge > 15us
    t_precharge = clk / 1000000 * 15 + 1;
    psram_phy->REG_060 = PSRAM_ULP_PHY_REG_T_TRANS(t_trans) | PSRAM_ULP_PHY_REG_T_PRECHARGE(t_precharge);
    // t_pu > 400ns
    t_pu = (clk / 1000000 * 400 + (1000 - 1)) / 1000;
    // t_lock > 800ns
    t_lock = (clk / 1000000 * 800 + (1000 - 1)) / 1000;
    psram_phy->REG_064 = PSRAM_ULP_PHY_REG_T_PU(t_pu) | PSRAM_ULP_PHY_REG_T_LOCK(t_lock);
#else
#error "WARNING: modify this reg for every CHIP"
#endif
#endif

    phy_clk = clk;
    if (phy_clk <= 100000000 / 2) {
        range = 3;
    } else if (phy_clk <= 150000000 / 2) {
        range = 2;
    } else if (phy_clk <= 300000000 / 2) {
        range = 1;
    } else {
        range = 1;
    }

    hal_psram_phy_set_range(range);
}

static void hal_psram_phy_init(uint32_t clk)
{
    uint32_t val;

    if (phy_bypass) {
        val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
        val |= PSRAM_ULP_PHY_REG_PSRAM_PU;  /////////////////////////////////
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TXDRV, 0x3);
        psram_phy->REG_04C = val;
        hal_sys_timer_delay_us(100);
        val = hal_psram_phy_read_reg(&psram_phy->REG_040);
        val |= (PSRAM_ULP_PHY_ANA_PHY_BYPASS | PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR);
        psram_phy->REG_040 = val;
        val = hal_psram_phy_read_reg(&psram_phy->REG_004);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_SQPI_SAMPLE_SEL, 1);
        psram_phy->REG_004 = val;
    } else {
        val = hal_psram_phy_read_reg(&psram_phy->REG_048);
        val |= PSRAM_ULP_PHY_REG_LDO_PU | PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
        psram_phy->REG_048 = val;
        hal_sys_timer_delay_us(100);

        val &= ~PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN1, 0xd);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN2, 0x7);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x2);
#if (CHIP_PSRAM_CTRL_VER >= 3)
#ifdef CHIP_BEST1503
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_RES, 0x8);
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IPTAT_RES, 0x5);
#else
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_RES, 0x4);
#endif
#endif
        psram_phy->REG_048 = val;
        hal_sys_timer_delay_us(100);

        hal_psramip_phy_ceb_pu_set(HAL_PSRAM_PHY_CHANNEL_0);

        val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
        val |= PSRAM_ULP_PHY_REG_PSRAM_PU;
#ifdef CHIP_BEST1503
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_SWRC, 0x2);
#else
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_SWRC, 0x0);
#endif
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TXDRV, 0x4);
        psram_phy->REG_04C = val;
        hal_sys_timer_delay_us(100);

        val = hal_psram_phy_read_reg(&psram_phy->REG_050);
        val |= PSRAM_ULP_PHY_REG_DLL_PU;
#ifdef CHIP_BEST1503
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_SWRC, 0x2);
#else
        val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_SWRC, 0x0);
#endif
        val &= ~PSRAM_ULP_PHY_REG_BYPASS_DECIMATION;
        psram_phy->REG_050 = val;
        hal_sys_timer_delay_us(100);

        val |= PSRAM_ULP_PHY_REG_DLL_RESETB;
        psram_phy->REG_050 = val;
        hal_sys_timer_delay_us(100);

        hal_psram_phy_dll_config(clk);
        hal_sys_timer_delay_us(500);

#ifdef SLEEP_WITH_DIGPHY
        psram_phy->REG_040 &= ~PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR;
#endif
    }
}

static void hal_psram_digphy_init()
{
#if (CHIP_PSRAM_CTRL_VER >= 3)
    uint32_t val;
    val = hal_psram_phy_read_reg(&psram_phy->REG_004);
#ifdef PSRAM_XCCELA_MODE
    val |= PSRAM_ULP_PHY_ADDR_4BYTES_EN | PSRAM_ULP_PHY_DQS_DM_MERGE_EN;
    val &= ~PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN;
#elif defined (PSRAM_HYPERBUS_MODE)
    val |= PSRAM_ULP_PHY_ADDR_4BYTES_EN | PSRAM_ULP_PHY_DQS_DM_MERGE_EN | PSRAM_ULP_PHY_REG_WR_WO_DM;
    val &= ~PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN;
#endif
    val |= PSRAM_ULP_PHY_CTRL_DELAY(1);
    val &= ~PSRAM_ULP_PHY_PHY_DLY_AUTO_EN;
    psram_phy->REG_004 = val;
#endif
}

POSSIBLY_UNUSED void hal_psram_phy_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    if (phy_bypass)
        return;

#if CHIP_PSRAM_CTRL_VER >= 5 && defined(SLEEP_WITH_DIGPHY)
    /* use dig phy because ana phy cs go low after ldo off */
    psram_phy->REG_040 |= PSRAM_ULP_PHY_ANA_PHY_BYPASS;
    hal_psramip_mc_busy_wait();
#else
    uint32_t val;

#ifdef CHIP_BEST2003
    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~PSRAM_ULP_PHY_REG_DLL_PU;
    psram_phy->REG_050 = val;

    if (pmu_get_chip_model() > CHIP_MODEL_2003H) {
        val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
        val &= ~PSRAM_ULP_PHY_REG_PSRAM_PU;
        psram_phy->REG_04C = val;

        val = hal_psram_phy_read_reg(&psram_phy->REG_048);
        val &= ~PSRAM_ULP_PHY_REG_LDO_PU;
        psram_phy->REG_048 = val;
    }

    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IDLL, 0);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IPSRAM, 2);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IPTAT_RES, 0);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_RES, 0);
    psram_phy->REG_048 = val;
#else
    val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
    val &= ~PSRAM_ULP_PHY_REG_PSRAM_PU;
    psram_phy->REG_04C = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~PSRAM_ULP_PHY_REG_DLL_PU;
    psram_phy->REG_050 = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val &= ~PSRAM_ULP_PHY_REG_LDO_PU;
    psram_phy->REG_048 = val;
#endif
#endif

#ifdef CHIP_BEST1502X
    analog_psram_phy_sleep();
#endif
}

POSSIBLY_UNUSED static void hal_psram_phy_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    uint32_t val;

    if (phy_bypass)
        return;

#ifdef CHIP_BEST1502X
    analog_psram_phy_wakeup();
#endif

#if CHIP_PSRAM_CTRL_VER >= 5 && defined(SLEEP_WITH_DIGPHY)
    /* use dig phy because ana phy cs go low after ldo off */
    psram_phy->REG_040 &= ~PSRAM_ULP_PHY_ANA_PHY_BYPASS;
    hal_sys_timer_delay_us(20);
    hal_psramip_mc_busy_wait();
#else
    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val |= PSRAM_ULP_PHY_REG_LDO_PU | PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
    psram_phy->REG_048 = val;
    hal_sys_timer_delay_us(10);

    val &= ~PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN1, 0xd);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN2, 0x7);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x2);
#if (CHIP_PSRAM_CTRL_VER >= 3)
#ifdef CHIP_BEST1503
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_RES, 0x8);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IPTAT_RES, 0x5);
#else
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_RES, 0x4);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IPTAT_RES, 8);
#endif
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IPSRAM, 8);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IDLL, 8);
#endif
    psram_phy->REG_048 = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
    val |= PSRAM_ULP_PHY_REG_PSRAM_PU;
#ifdef CHIP_BEST1503
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_SWRC, 0x2);
#else
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_SWRC, 0x0);
#endif
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TXDRV, 0x4);
    psram_phy->REG_04C = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val |= PSRAM_ULP_PHY_REG_DLL_PU;
#ifdef CHIP_BEST1503
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_SWRC, 0x2);
#else
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_SWRC, 0x0);
#endif
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(2);

    val |= PSRAM_ULP_PHY_REG_DLL_RESETB;
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(20);
#endif

    do {
        val = hal_psram_phy_read_reg(&psram_phy->REG_058);
    } while ((val & PSRAM_ULP_PHY_DLL_LOCK) == 0);
}
static void hal_psram_mc_set_timing(uint32_t clk, bool init)
{
    uint32_t val;
#ifdef PSRAM_XCCELA_MODE
    if (init) {
        val = PSRAM_ULP_MC_WRITE_LATENCY(1);
    } else {
        if (clk <= 66000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(3);
        } else if (clk <= 109000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(4);
        } else if (clk <= 133000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(5);
        } else if (clk <= 166000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(6);
        } else {
            val = PSRAM_ULP_MC_WRITE_LATENCY(7);
        }
    }
    psram_mc->REG_028 = val;
    if (clk <= 66000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(3);
    } else if (clk <= 109000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(4);
    } else if (clk <= 133000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(5);
    } else if (clk <= 166000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(6);
    } else {
        val = PSRAM_ULP_MC_READ_LATENCY(7);
    }
    psram_mc->REG_02C = val;
    // tCPH >= 24 ns
    val = (clk / 1000000 * 24 + (1000 - 1)) / 1000;
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR(val);
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW(val);
    psram_mc->REG_068 = PSRAM_ULP_MC_T_MRR(val);
    psram_mc->REG_06C = PSRAM_ULP_MC_T_MRS(val);
#elif defined(PSRAM_HYPERBUS_MODE)
    if (init) {
        val = 7;
    } else {
        if (clk <= 85000000) {
            val = 3;
        } else if (clk <= 104000000) {
            val = 4;
        } else if (clk <= 133000000) {
            val = 5;
        } else if (clk <= 166000000) {
            val = 6;
        } else  {
            val = 7;
        }
    }
    psram_mc->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(val);
    psram_mc->REG_02C = PSRAM_ULP_MC_READ_LATENCY(val);
    hal_psramip_hyperbus_var_lat_en(true);
#else /*PSRAM_XCCELA_MODE*/
    if (clk <= 166000000) {
        val = PSRAM_ULP_MC_WRITE_LATENCY(0);
    } else {
        val = PSRAM_ULP_MC_WRITE_LATENCY(2);
    }
    psram_mc->REG_028 = val;
    if (clk <= 66000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(2);
    } else if (clk <= 109000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(3);
    } else if (clk <= 133000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(4);
    } else if (clk <= 166000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(5);
    } else {
        val = PSRAM_ULP_MC_READ_LATENCY(6);
    }
    psram_mc->REG_02C = val;
    // tCPH
    val = 2;
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR(val);
    psram_mc->REG_068 = PSRAM_ULP_MC_T_MRR(val);
    val = 6;
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW(val);
#if CHIP_PSRAM_CTRL_VER == 2
    val += 1;
#endif
    psram_mc->REG_06C = PSRAM_ULP_MC_T_MRS(val);
#endif /*PSRAM_XCCELA_MODE*/

#ifdef PSRAM_HYPERBUS_MODE
    // tRC    60 ns
    val = (clk / 1000000 * 60 + (1000 - 1)) / 1000;
    psram_mc->REG_050 = PSRAM_ULP_MC_T_RC(val);
    //? tDQSCK 5.5 ns
    // tCPHR  28 ns
    // tCPHW  28 ns
    val = (clk / 1000000 * 28 + (1000 - 1)) / 1000;
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR(val > 2 ? (val - 2): val);
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW(val > 2 ? (val - 2): val);
    // tMRR   10 ns
    val = (clk / 1000000 * 10 + (1000 - 1)) / 1000;
    psram_mc->REG_068 = PSRAM_ULP_MC_T_MRR(val);
    // tMRS   400 ns
    val = (clk / 1000000 * 400 + (1000 - 1)) / 1000;
    psram_mc->REG_06C = PSRAM_ULP_MC_T_MRS(val);
    // tCEM   4000 ns
    val = clk / 1000000 * 4 + 1;
    psram_mc->REG_070 = PSRAM_ULP_MC_T_CEM(val);
    // tHS    3us
    val = clk / 1000000 * 3 + 1;
    psram_mc->REG_080 = PSRAM_ULP_MC_T_HS(val);
    // tXHS   100000 ns
    val = clk / 1000000 * 100 + 1;
    psram_mc->REG_088 = PSRAM_ULP_MC_T_XHS(val);
    // tXPHS  60 ns
    val = (clk / 1000000 * 60 + (1000 - 1)) / 1000;
    psram_mc->REG_084 = PSRAM_ULP_MC_T_XPHS(val);
    psram_mc->REG_09C = PSRAM_ULP_MC_WR_DMY_CYC(0);
    psram_mc->REG_0A0 = PSRAM_ULP_MC_STOP_CLK_IN_NOP | PSRAM_ULP_MC_STOP_CLK_IN_TCPH | PSRAM_ULP_MC_NOP_DMY_CYC(val);
#else
    // tRC >= 55 ns
    val = (clk / 1000000 * 55 + (1000 - 1)) / 1000;
    psram_mc->REG_050 = PSRAM_ULP_MC_T_RC(val);
    // tCEM <= 2.5 us
    val = clk / 1000000 * 25 / 10;
    psram_mc->REG_070 = PSRAM_ULP_MC_T_CEM(val);
    // tRST >= 2 us
    val = clk / 1000000 * 2 + 1;
    psram_mc->REG_074 = PSRAM_ULP_MC_T_RST(val);
    // tHS >= 150 us
    val = clk / 1000000 * 150 + 1;
    psram_mc->REG_080 = PSRAM_ULP_MC_T_HS(val);
    // tXPHS in [60 ns, 2 us]
    val = (clk / 1000000 * 60 + (1000 - 1)) / 1000;
    psram_mc->REG_084 = PSRAM_ULP_MC_T_XPHS(val);
    // tXHS >= 150 us
    val = clk / 1000000 * 150 + 1;
    psram_mc->REG_088 = PSRAM_ULP_MC_T_XHS(val);
#ifdef PSRAM_XCCELA_MODE
    psram_mc->REG_09C = PSRAM_ULP_MC_WR_DMY_CYC(0);
#else
    psram_mc->REG_09C = PSRAM_ULP_MC_WR_DMY_CYC(1);
#endif
    // NOP dummy cycles, same as tXPHS in [60 ns, 2 us]
    val = (clk / 1000000 * 60 + (1000 - 1)) / 1000;
    psram_mc->REG_0A0 = PSRAM_ULP_MC_STOP_CLK_IN_NOP |
#ifdef PSRAM_XCCELA_MODE
                        PSRAM_ULP_MC_STOP_CLK_IN_TCPH |
#endif
                        PSRAM_ULP_MC_NOP_DMY_CYC(val);
#endif
    psram_mc->REG_0A4 = PSRAM_ULP_MC_QUEUE_IDLE_CYCLE(5000);
}

static void hal_psram_phy_wait_lock()
{
#ifndef PROGRAMMER
    uint32_t val;
    uint32_t val_50;
    uint32_t count = 0;
    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val_50 = hal_psram_phy_read_reg(&psram_phy->REG_050);
#endif
    while (1) {
        uint32_t val_58;
        val_58 = hal_psram_phy_read_reg(&psram_phy->REG_058);
        if ((val_58 & PSRAM_ULP_PHY_DLL_LOCK))
            break;
#ifndef PROGRAMMER
        if (count++ > 20) {
            if (GET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE) == 0x2) {
                val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x1);
                psram_phy->REG_048 = val;
            } else if (GET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE) == 0x1) {
                val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x2);
                psram_phy->REG_048 = val;
                if (GET_BITFIELD(val_50, PSRAM_ULP_PHY_REG_DLL_RANGE) == 0x1) {
                    val_50 = SET_BITFIELD(val_50, PSRAM_ULP_PHY_REG_DLL_RANGE, 0x2);
                    psram_phy->REG_050 = val_50;
                }
            }
            count = 0;
        }
#endif
        hal_sys_timer_delay_us(100);
    }
}

POSSIBLY_UNUSED
static void hal_psram_init_calib(void)
{
    uint32_t val;
    uint32_t delay, tx_ceb;

    hal_psram_phy_wait_lock();
    val = hal_psram_phy_read_reg(&psram_phy->REG_058);
    delay = GET_BITFIELD(val, PSRAM_ULP_PHY_DLL_DLY_IN);
    //ASSERT(delay < (PSRAM_ULP_PHY_DLL_DLY_IN_MASK >> PSRAM_ULP_PHY_DLL_DLY_IN_SHIFT),
    //    "%s: Bad DLL_DLY_IN=0x%X reg=0x%08X", __func__, delay, psram_phy->REG_058);

    delay /= 2;

#ifdef PSRAM_HYPERBUS_MODE
    tx_ceb = 0;
#else
    tx_ceb = delay;
#endif
    psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(delay) |
                         PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(delay) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(delay);
    hal_sys_timer_delay_us(100);
}

static void hal_psram_mc_init(uint32_t clk)
{
    uint32_t val = 0;
#if (CHIP_PSRAM_CTRL_VER >= 3)
#if defined(PSRAM_DUAL_8BIT)
    val |= PSRAM_ULP_MC_CHIP_BIT;
#elif defined(PSRAM_X16_MODE)
    val |= PSRAM_ULP_MC_CHIP_X16;
#endif
#ifdef PSRAM_SQPI_MODE
    val |= PSRAM_ULP_MC_SQPI_MODE;
    val |= (PSRAM_ULP_PHY_IDLE_DQ_OEN | PSRAM_ULP_PHY_IDLE_DQS_OEN);
#elif defined(PSRAM_HYPERBUS_MODE)
    val |= PSRAM_ULP_MC_HYPERBUS | PSRAM_ULP_MC_OPI_MODE;
    val = SET_BITFIELD(val, PSRAM_ULP_MC_CHIP_CA_PATTERN, 0x1);
#else
    val |= PSRAM_ULP_MC_OPI_MODE;
    val = SET_BITFIELD(val, PSRAM_ULP_MC_CHIP_CA_PATTERN, 0x1);
#endif /* PSRAM_SQPI_MODE */
#else /* CHIP_PSRAM_CTRL_VER == 2 */
#if defined(PSRAM_DUAL_8BIT)
    val |= PSRAM_ULP_MC_CHIP_BIT;
#endif
#endif /* CHIP_PSRAM_CTRL_VER >= 3 */
    psram_mc->REG_000 = val;
    psram_mc->REG_020 = 0;
#if (CHIP_PSRAM_CTRL_VER >= 8)
    psram_mc->REG_024 = PSRAM_ULP_MC_AUTOWAKEUP_EN;
    psram_mc->REG_0D8 = PSRAM_ULP_MC_MRS_AS_PD_EN |
        PSRAM_ULP_MC_MRS_AS_PD_WRLAT_DR |
        PSRAM_ULP_MC_MRS_AS_PD_MR(6) |
        PSRAM_ULP_MC_MRS_AS_PD_MD(0xF0) |
        PSRAM_ULP_MC_MRS_AS_PD_WRLAT(1);
#else
    psram_mc->REG_024 = PSRAM_ULP_MC_AUTOWAKEUP_EN |
        PSRAM_ULP_MC_PD_MR(6) |
        PSRAM_ULP_MC_PD_CMD(0xF0) |
#if (CHIP_PSRAM_CTRL_VER == 7)
        PSRAM_ULP_MC_PD_LAT_DR |
        PSRAM_ULP_MC_PD_LAT(1) |
#endif
        0;
#endif

#ifdef PSRAM_WRAP_ENABLE
    // Burst len: 32 bytes, page: 1K
    val = PSRAM_ULP_MC_BURST_LENGTH(1);
#else
    if (0) {
#if defined(PSRAM_BURST_FIXED)
    } else if (hal_psramip_is_fixburst_support()) {
        //3: fixed 128B  2: fixed 64B  1: fix 32B
        val = PSRAM_ULP_MC_BURST_LENGTH(2);
#else
    } else {
        if (PSRAM_PAGE_SIZE == 512) {
            val = PSRAM_ULP_MC_BURST_LENGTH(7);
        } else if (PSRAM_PAGE_SIZE == 1024) {
            val = PSRAM_ULP_MC_BURST_LENGTH(4);
        } else if (PSRAM_PAGE_SIZE == 2048) {
            val = PSRAM_ULP_MC_BURST_LENGTH(5);
        } else {
            // 4K
            val = PSRAM_ULP_MC_BURST_LENGTH(6);
        }
#endif
    }
#endif /* PSRAM_WRAP_ENABLE */
    if (PSRAM_PAGE_SIZE == 512) {
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(3);
    } else if (PSRAM_PAGE_SIZE == 1024) {
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(0);
    } else if (PSRAM_PAGE_SIZE == 2048) {
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(1);
    } else {
        // 4K
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(2);
    }
    psram_mc->REG_034 = val;
    // AHB bus width: 32 bits
    psram_mc->REG_038 = 0;
    // Write buffer level with high priority: 0~7
#if CHIP_PSRAM_CTRL_VER == 2
    psram_mc->REG_03C = PSRAM_ULP_MC_HIGH_PRI_LEVEL(1);
#else
    psram_mc->REG_03C = PSRAM_ULP_MC_HIGH_PRI_LEVEL(7);
#endif
#ifdef PSRAM_WRAP_ENABLE
    psram_mc->REG_040 = PSRAM_ULP_MC_CP_WRAP_EN;
#else
    psram_mc->REG_040 = PSRAM_ULP_MC_WRAP_CRT_RET_EN;
#endif
    if (0) {
#if defined(PSRAM_BURST_FIXED)
    } else if (hal_psramip_is_fixburst_support()) {
        psram_mc->REG_044 = PSRAM_ULP_MC_BUFFERABLE_WB_EN | PSRAM_ULP_MC_RD_CANCEL_EN;
        psram_mc->REG_040 &= ~(PSRAM_ULP_MC_WRAP_CRT_RET_EN);
#else
    } else {
        psram_mc->REG_044 = 0;
#endif
    }
    psram_mc->REG_048 = 0;

    hal_psramip_set_reg_data_mask();
    hal_sys_timer_delay_us(100);

    hal_psram_mc_set_timing(clk, true);
    hal_sys_timer_delay_us(100);

    psram_mc->REG_400 = PSRAM_ULP_MC_INIT_COMPLETE;
    hal_sys_timer_delay_us(100);
}

POSSIBLY_UNUSED static void hal_psram_hyperram_sleep(void)
{
    uint32_t lc;
    uint32_t mr, md;

    mr = 0x1002;
    psram_read_reg(mr, &md);
    md = HAL_PSRAM_SWAP8BITS(md);
    lc = psram_mc->REG_028;
    hal_psramip_hyperbus_var_lat_en(false);
    psram_mc->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(1);
    __DSB();
    hal_psramip_mc_busy_wait();
    md |= 1 << 5;
    psram_write_reg(mr, HAL_PSRAM_SWAP8BITS(md) | (HAL_PSRAM_SWAP8BITS(md) << 16));
    hal_psramip_mc_busy_wait();
    psram_mc->REG_028 = lc;
    hal_psramip_hyperbus_var_lat_en(true);
    __DSB();
    hal_sys_timer_delay_us(3);
}

void psram_mc_set_freq(uint32_t clk)
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

#if defined(PSRAM_AUTO_LOW_POWER)
static void hal_psram_auto_low_power_enable(void)
{
    if (hal_psramip_is_auto_lp_support()) {
#if (CHIP_PSRAM_CTRL_VER <= 7)
        psram_mc->REG_024 &= ~(PSRAM_ULP_MC_PD_MR_MASK | PSRAM_ULP_MC_PD_CMD_MASK);
        psram_mc->REG_024 |=
#ifdef PSRAM_DIE_HALF_SLEEP_ENABLE
        PSRAM_ULP_MC_PD_MR(6) | PSRAM_ULP_MC_PD_CMD(0xf0);
#else
        PSRAM_ULP_MC_PD_MR(0x0a) | PSRAM_ULP_MC_PD_CMD(0x0a);
#endif
#endif

        psram_mc->REG_0A4 = PSRAM_ULP_MC_QUEUE_IDLE_CYCLE(psram_run_clk / 1000000 * PSRAM_QUEUE_IDLE_TIME_US);
        hal_psram_phy_auto_pd_en(true);
        psram_mc->REG_024 |= PSRAM_ULP_MC_ENTRY_SLEEP_IDLE;
    }
}

POSSIBLY_UNUSED static void hal_psram_auto_low_power_disable(void)
{
    if (hal_psramip_is_auto_lp_support()) {
#if (CHIP_PSRAM_CTRL_VER <= 7)
        psram_mc->REG_024 &= ~(PSRAM_ULP_MC_PD_MR_MASK | PSRAM_ULP_MC_PD_CMD_MASK | PSRAM_ULP_MC_ENTRY_SLEEP_IDLE);
        psram_mc->REG_024 |= PSRAM_ULP_MC_PD_MR(6) |  PSRAM_ULP_MC_PD_CMD(0xf0);
#endif
        hal_psram_phy_auto_pd_en(false);
    }
}
#endif

void hal_psram_suspend(void)
{
    hal_psramip_mc_busy_wait();

#ifdef PSRAM_AUTO_LOW_POWER
   hal_psram_auto_low_power_disable();
#endif

    if (!hal_psramip_mc_in_sleep()) {
        hal_psramip_mc_busy_wait();
#if (CHIP_PSRAM_CTRL_VER >= 3)
#ifdef PSRAM_XCCELA_MODE
        int val = psram_mc->REG_028;
        psram_mc->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(1);
        __DSB();
        hal_psramip_mc_busy_wait();
#endif /* PSRAM_XCCELA_MODE */
#ifdef PSRAM_HYPERBUS_MODE
        hal_psram_hyperram_sleep();
#else
        hal_psramip_set_cmd(MEMIF_PD);
        hal_psramip_mc_busy_wait();
#endif
#ifdef PSRAM_XCCELA_MODE
        psram_mc->REG_028 = val;
        hal_psramip_mc_busy_wait();
#endif /* PSRAM_XCCELA_MODE */
#else
        hal_psramip_set_cmd(MEMIF_PD);
        hal_psramip_mc_busy_wait();
#endif
    }
}

void hal_psram_resume(void)
{
    hal_psramip_set_cmd(MEMIF_NOP);
    hal_psramip_mc_busy_wait();

#if (CHIP_PSRAM_CTRL_VER >= 5 && defined(SLEEP_WITH_DIGPHY)) || defined(PSRAM_HYPERBUS_MODE)
    hal_sys_timer_delay_us(100);
#endif

#ifdef PSRAM_AUTO_LOW_POWER
   hal_psram_auto_low_power_enable();
#endif
}

#if !(defined(CHIP_SUBSYS_SENS))
#ifdef CHIP_BEST1502X
void hal_psram_change_speed(enum HAL_PSRAM_ID_T id, uint32_t psram_speed)
{
    uint32_t lock;
    lock = int_lock();

    hal_psramip_mc_busy_wait();
    hal_psram_suspend();
    pmu_psram_freq_config(psram_speed);
    hal_psram_resume();

    int_unlock(lock);
}
#endif
#endif

#ifdef PSRAM_CLOCK_GATE_ENABLE
static void hal_psram_clock_gate_enable(void)
{
#if (CHIP_PSRAM_CTRL_VER >= 7)
    psram_mc ->REG_0C4 = PSRAM_ULP_MC_CACTIVE_MASK(0);
#endif
}
#endif

static void hal_psram_clock_gate_disable(void)
{
#if (CHIP_PSRAM_CTRL_VER >= 7)
    psram_mc ->REG_0C4 = PSRAM_ULP_MC_CACTIVE_MASK(0xff);
#endif
}

void hal_psram_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    hal_psramip_mc_busy_wait();

#ifdef PSRAM_AUTO_LOW_POWER
   hal_psram_auto_low_power_disable();
#endif

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_clock_gate_disable();
#endif

    if (!hal_psramip_mc_in_sleep()) {
        hal_psramip_mc_busy_wait();
#if (CHIP_PSRAM_CTRL_VER >= 3)
#ifdef PSRAM_XCCELA_MODE
        int val = psram_mc->REG_028;
        psram_mc->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(1);
        __DSB();
        hal_psramip_mc_busy_wait();
#endif /* PSRAM_XCCELA_MODE */
#ifdef PSRAM_HYPERBUS_MODE
        hal_psram_hyperram_sleep();
#else
#if (CHIP_PSRAM_CTRL_VER <= 3)
        //Early SOC(psram die) can not sleep by MEMIF_PD
        psram_write_reg(6, 0xf0);
#else
        hal_psramip_set_cmd(MEMIF_PD);
#endif
        hal_psramip_mc_busy_wait();
#endif
#ifdef PSRAM_XCCELA_MODE
        psram_mc->REG_028 = val;
        hal_psramip_mc_busy_wait();
#endif /* PSRAM_XCCELA_MODE */
#else
        hal_psramip_set_cmd(MEMIF_PD);
        hal_psramip_mc_busy_wait();
#endif
    }

    if (0) {
#ifdef CORE_SLEEP_POWER_DOWN
    } else if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        ASSERT(hal_psramip_is_dig_pd_support(), "This psramip version do not suppoet digital logic power down yet");
        if(phy_bypass == 0) {
            psramphy_save_reg_054 = hal_psram_phy_read_reg(&psram_phy->REG_054);
            psramphy_save_range = hal_psram_phy_get_range_val(HAL_PSRAM_ID_0);
        }
#endif
    } else {
        hal_psram_phy_sleep(mode);
    }
}

void hal_psram_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    if (0) {
#ifdef CORE_SLEEP_POWER_DOWN
    } else if ((mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) && (hal_psramip_is_mc_reset())) {
#ifdef CHIP_BEST1502X
        analog_psram_phy_wakeup();
#endif
        hal_psram_phy_init(psram_run_clk);
        hal_psram_digphy_init();
        if (phy_bypass == 0) {
            uint32_t val;
            psram_phy->REG_054 = psramphy_save_reg_054;
            val = hal_psram_phy_read_reg(&psram_phy->REG_050);
            val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_RANGE, psramphy_save_range);
            psram_phy->REG_050 = val;
        }
        psram_mc_set_freq(psram_run_clk);
        hal_psram_mc_init(psram_run_clk);
        hal_sys_timer_delay_us(500);
        hal_psram_mc_set_timing(psram_run_clk, false);
        hal_sys_timer_delay_us(100);
#endif
    } else {
        hal_psram_phy_wakeup(mode);
    }

    hal_psramip_set_cmd(MEMIF_NOP);
    hal_psramip_mc_busy_wait();

    if (0) {
#ifdef CORE_SLEEP_POWER_DOWN
    } else if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        hal_sys_timer_delay_us(150);
#endif
#if ((CHIP_PSRAM_CTRL_VER >= 5 && defined(SLEEP_WITH_DIGPHY)) || defined(PSRAM_HYPERBUS_MODE) || (CHIP_PSRAM_CTRL_VER <= 3))
    } else {
        hal_sys_timer_delay_us(150);
#endif
    }

#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_clock_gate_enable();
#endif

#ifdef PSRAM_AUTO_LOW_POWER
   hal_psram_auto_low_power_enable();
#endif
}

static void psram_chip_timing_config(uint32_t clk, bool update_psram_first)
{
    enum HAL_CMU_FREQ_T freq;

    if (clk <= 52000000) {
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

    if (update_psram_first) {
        psram_set_timing(clk);
    }

    hal_cmu_mem_set_freq(freq);
    hal_sys_timer_delay_us(500);
    hal_psram_phy_dll_config(clk);

#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif
    hal_psram_mc_set_timing(clk, false);
    if (!update_psram_first) {
        psram_set_timing(clk);
    }
    hal_sys_timer_delay_us(100);
}

void hal_psram_snoop_enable(enum HAL_PSRAM_ID_T id)
{
#if (CHIP_PSRAM_CTRL_VER >= 3)
    psram_mc->REG_044 &= ~PSRAM_ULP_MC_SNP_DISABLE;
#endif
}

void hal_psram_snoop_disable(enum HAL_PSRAM_ID_T id)
{
    psram_mc->REG_044 |= PSRAM_ULP_MC_SNP_DISABLE;
}

void hal_psram_wait_write_buffer_empty(enum HAL_PSRAM_ID_T id)
{
    uint32_t level;
    uint32_t start = hal_sys_timer_get();

    __DSB();
    do {
        level = GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_WB_FILL_LEVEL);
        if (level == 0) {
            break;
        }
    } while (hal_sys_timer_get() - start < MS_TO_TICKS(10));

    ASSERT(level == 0, "%s: Drain timeout: level=%u status=0x%08X", __func__, level, psram_mc->REG_404);
}

static void hal_psram_phy_restore_range(uint32_t range)
{
    uint32_t val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY;
    psram_phy->REG_050 = val;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_RANGE, range);
    psram_phy->REG_050 = val;
    val |= PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY;
    psram_phy->REG_050 = val;
}

static void psram_mc_reset(uint32_t range)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_PSRAM);
    hal_cmu_reset_set(HAL_CMU_MOD_O_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_PSRAM);

    hal_cmu_mem_set_freq(HAL_CMU_FREQ_104M);
    hal_psram_phy_init(psram_cfg_clk);
    hal_sys_timer_delay_us(30);
    hal_psram_mc_init(psram_cfg_clk);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif
    psram_reset();
    psram_chip_timing_config(psram_run_clk, true);
    hal_psram_phy_restore_range(range);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif

    hal_psram_snoop_disable(HAL_PSRAM_ID_0);
}
POSSIBLY_UNUSED static bool psramphy_check_tx_ceb_valid(uint32_t range)
{
    int retry = 2;
    while (retry--) {
        int count = 0;
        hal_psramip_flush_all_fifo();
        hal_psramip_xfer_addr_len(0x100000, 8);
        hal_psramip_set_cmd(MEMIF_READ);
        while (hal_psramip_rx_fifo_empty()) {
            if (count++ > 10) {
                //PSRAM_TRACE(1, "%s, rx fifo empty, return false", __FUNCTION__);
                psram_mc_reset(range);
                hal_psram_snoop_disable(HAL_PSRAM_ID_0);
                return false;
            }
        }
    }
    return true;
}
POSSIBLY_UNUSED static bool psramphy_check_tx_clk_valid(uint32_t range)
{
    uint32_t time;
    uint32_t val;

    time = hal_fast_sys_timer_get();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR | PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psramip_xfer_addr_len(0xAA55AA55, 1);
    hal_psramip_set_cmd(MEMIF_READ);
    while (hal_psramip_rx_fifo_empty()) {
        if (FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time) > 5) {
            val = hal_psram_phy_read_reg(&psram_phy->REG_004);
            val |= PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = val;
            while (hal_psramip_mc_busy());
            val &= ~PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = val;
            return false;
        }
    }
    return true;
}

int psramphy_check_write_valid()
{
    uint32_t i;
    uint32_t val, val0, val1;
    uint32_t val2, val3;
    volatile uint32_t *psram_base = (volatile uint32_t *)PSRAM_NC_BASE;
    volatile uint32_t *psram_base1 = (volatile uint32_t *)(PSRAM_NC_BASE + 0x100000);

    for (i = 0; i < 0x100; ++i) {
        *(psram_base + i) = 0xffffffff;
        *(psram_base1 + i) = 0xffffffff;
    }
    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
        *(psram_base + i) = val0;
        *(psram_base1 + i) = val1;
    }

    hal_psramip_wb_busy_wait();
    hal_psramip_mc_busy_wait();

    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
#if defined(CHIP_BEST2009)
        val2 = *(psram_base + i);
        val3 = *(psram_base1 + i);
        // if ((hal_psram_mgr_read_4byte((uint32_t)(psram_base + i), &val2) < 0) ||
        //     (hal_psram_mgr_read_4byte((uint32_t)(psram_base1 + i), &val3) < 0)) {
        //     PSRAM_TRACE(3, "Read data time out");
        //     return HAL_PSRAM_DATA_READ_TIME_OUT;
        // }
#else
        // val2 = *(psram_base + i);
        // val3 = *(psram_base1 + i);
        if ((hal_psram_mgr_read_4byte((uint32_t)(psram_base + i), &val2) < 0) ||
            (hal_psram_mgr_read_4byte((uint32_t)(psram_base1 + i), &val3) < 0)) {
            PSRAM_TRACE(3, "Read data time out");
            return HAL_PSRAM_DATA_READ_TIME_OUT;
        }
#endif

        if (val2 != val0) {
            //PSRAM_TRACE(3, "%s, i:%d, 0x%x, 0x%x", __FUNCTION__, i, val2, val0);
            return 0;
        }
        if (val3 != val1) {
            //PSRAM_TRACE(3, "%s, i:%d, 0x%x, 0x%x", __FUNCTION__, i, val3, val1);
            return 0;
        }
    }

    return 1;
}

static int find_the_largest_window_of_array(void *p_matrix, uint8_t column_row_num, int byte_pos, struct HAL_PSRAM_RECTANGLE_T *rectangle)
{
    uint8_t (*matrix)[PSRAM_WINDOW_SIZE] = (uint8_t (*)[PSRAM_WINDOW_SIZE])p_matrix;
    uint8_t dp[PSRAM_WINDOW_SIZE][PSRAM_WINDOW_SIZE];
    int y_statr = 0, x_statr = 0, y_end = 0, x_end = 0;
    int max_size, min_height, size;
    uint8_t mask;

    y_statr = y_end = 0;
    x_statr = x_end = 0;
    max_size = min_height = size = 0;
    mask = (1 << byte_pos);

    for (int j = 0; j < column_row_num; j++) {
        dp[0][j] = !!(matrix[0][j] & mask);
    }
    for (int i = 1; i < column_row_num; i++) {
        dp[i][0] = (matrix[i][0] & mask) ? dp[i - 1][0] + 1 : 0;
    }
    for (int i = 1; i < column_row_num; i++) {
        for (int j = 1; j < column_row_num; j++) {
            if ((matrix[i][j] & mask)) {
                dp[i][j] = dp[i - 1][j] + 1;
            } else {
                dp[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < column_row_num; i++) {
        for (int j = 0; j < column_row_num; j++) {
            min_height = dp[i][j];
            for (int k = j; k >= 0 && dp[i][k] > 0; k--) {
                min_height = MIN(min_height, dp[i][k]);
                size = min_height * (j - k + 1);
                if (size > max_size) {
                    max_size = size;
                    y_statr = i - min_height + 1;
                    x_statr = k;
                    y_end = i;
                    x_end = j;
                }
            }
        }
    }
    rectangle->x_start = x_statr;
    rectangle->x_end = x_end;
    rectangle->y_start = y_statr;
    rectangle->y_end = y_end;

    return max_size;
}

static void hal_psram_phy_set_range(uint32_t range)
{
    uint32_t val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~(PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY);
    psram_phy->REG_050 = val;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_RANGE, range);
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(100);
    val |= (PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY);
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(100);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_phy_wait_lock();
#endif
}

uint32_t hal_psram_phy_get_range_val(enum HAL_PSRAM_ID_T id)
{
    uint32_t val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val = GET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_RANGE);

    return val;
}

uint32_t hal_psram_phy_get_dly_in_val(enum HAL_PSRAM_ID_T id)
{
    uint32_t val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_058);
    val = GET_BITFIELD(val, PSRAM_ULP_PHY_DLL_DLY_IN);

    return val;
}

void hal_psram_calib_range(uint32_t range)
{
    uint32_t val, delay;
    uint8_t inc_delay, volume;
    POSSIBLY_UNUSED uint8_t tx_dqs, rx_dqs, tx_ceb, tx_clk;
    int read_code, volume_lock, flag;
    uint32_t byte_pos;
    struct HAL_PSRAM_RECTANGLE_T valid_rectangle;
    bool cali_valid[PSRAM_WINDOW_SIZE][PSRAM_WINDOW_SIZE];

    while (1) {
        if (range > (PSRAM_ULP_PHY_REG_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG_DLL_RANGE_SHIFT)) {
            range = PSRAM_ULP_PHY_REG_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG_DLL_RANGE_SHIFT;
        }
        hal_psram_phy_set_range(range);
        flag = 0;
        val = hal_psram_phy_read_reg(&psram_phy->REG_058);
        if (val & PSRAM_ULP_PHY_DLL_ALL_ONE) {
            PSRAM_TRACE(2, "%s: dll all one", __func__);
            flag = 1;
        }

        delay = GET_BITFIELD(val, PSRAM_ULP_PHY_DLL_DLY_IN);
        PSRAM_TRACE(4, "%s, range:%d, T/4 = 0x%x(psram_phy->REG_058:0x%x)", __func__, range, delay / 2, val);
        if (delay > (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT)) {
            PSRAM_TRACE(2, "%s: bad delay (T/2 > 0x1f) ", __func__);
            flag = 1;
        }
        if (flag && (range < PSRAM_ULP_PHY_REG_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG_DLL_RANGE_SHIFT)) {
            PSRAM_TRACE(2, "%s: increase range=%d", __func__, range + 1);
            range++;
            continue;
        }
        break;
    }

    if (0) {
#ifdef PSRAM_WINDOW_TEST
    } else if (1) {
        inc_delay = 1;
#endif
    } else if (delay < 0x10) {
        inc_delay = 1;
    } else {
        inc_delay = delay / 8;
    }

#if (CHIP_PSRAM_CTRL_VER >= 3)
#ifdef PSRAM_HYPERBUS_MODE
    tx_ceb = 0;
#else
    tx_ceb = delay / 2;
#endif
    tx_clk = delay / 2 + 2;
#else /*CHIP_PSRAM_CTRL_VER == 2*/
    tx_ceb = delay * 5 / 6; // t/4 + t/6
    tx_clk = (delay * 11 + 8) / 16;// t/4 + t/8 - t/32
#endif

#ifdef PSRAM_HYPERBUS_MODE
    volume = (MIN(delay, (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT)) - 2) / inc_delay;
#else
    volume = MIN(delay, (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT)) / inc_delay;
#endif
    PSRAM_TRACE(2, "tx_ceb:0x%x, tx_clk:0x%x", tx_ceb, tx_clk);
    PSRAM_TRACE(2, "volume:%d, inc_delay:%d", volume, inc_delay);

    /// calibrate tx_dqs and rx_dqs
    volume_lock = 0;
#if defined(PSRAM_HYPERBUS_MODE) && defined(CHIP_BEST2003)
    tx_dqs = 2;
#else
    tx_dqs = 0;
#endif
    memset(cali_valid, 0, sizeof(cali_valid));
    for (/* tx_dqs = 0 */; tx_dqs <= volume; tx_dqs++) {
        for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
#if defined(PSRAM_XCCELA_MODE) || defined(PSRAM_HYPERBUS_MODE)
            psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_dqs * inc_delay) |
                                 PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(0) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs * inc_delay);
#else
            psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_clk) |
                                 PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(tx_dqs * inc_delay) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs * inc_delay);
#endif
            read_code = psramphy_check_write_valid();
            if (read_code < 0) {
                if (!volume_lock) {
                    volume = ((volume + 1) * inc_delay > 0x1f ? volume : volume + 1);
                }
                break;
            } else {
                volume_lock = 1;
                cali_valid[tx_dqs][rx_dqs] = read_code;
            }
            //PSRAM_TRACE(3, "tx_dqs:0x%x, rx_dqs:0x%x, valid:%d", tx_dqs*inc_delay, rx_dqs*inc_delay, cali_valid[tx_dqs][rx_dqs]);
        }
    }

    byte_pos = 0;
#ifdef PSRAM_DEBUG
    PSRAM_TRACE_LINEFEED();
    PSRAM_TRACE_LINEFEED();
    PSRAM_TRACENOCRLF_NOTS(TR_ATTR_IMM, " ----------------------------------------------------------------------");
    PSRAM_TRACE_LINEFEED();
    PSRAM_TRACENOCRLF_NOTS(TR_ATTR_IMM, "  rx_dqs        ");
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        PSRAM_TRACENOCRLF_NOTS(TR_ATTR_IMM, "  %02d ", tx_dqs * inc_delay);
    }
    PSRAM_TRACE_LINEFEED();
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        PSRAM_TRACENOCRLF_NOTS(1, "%d-tx_dqs (%2d) ", byte_pos, tx_dqs * inc_delay);
        for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
            PSRAM_TRACENOCRLF_NOTS(TR_ATTR_IMM, "    %d", !!(cali_valid[tx_dqs][rx_dqs] & (1 << byte_pos)));
        }
        PSRAM_TRACE_LINEFEED();
    }
    PSRAM_TRACENOCRLF_NOTS(TR_ATTR_IMM, " -------------------------------------------------------------------------- ");
    PSRAM_TRACE_LINEFEED();
#endif

    find_the_largest_window_of_array(cali_valid, volume + 1, byte_pos, &valid_rectangle);

#ifdef PSRAM_WINDOW_TEST
        ASSERT((volume + 1) <= PSRAM_WINDOW_SIZE ,"%s : volume+1 is larger than PSRAM_WINDOW_SIZE ,you may increase macro PSRAM_WINDOW_SIZE", __func__);
        if (hal_psram_window_2d_cb_func != NULL) {
            hal_psram_window_2d_cb_func(cali_valid, volume + 1, inc_delay, byte_pos, &valid_rectangle);
        }
        PSRAM_TRACE_LINEFEED();
#endif
        ASSERT((MIN(valid_rectangle.y_end - valid_rectangle.y_start, valid_rectangle.x_end - valid_rectangle.x_start)) > 1,
            "%s : dqs window is too small to ensure data access accurately ", __func__);

    rx_dqs = ((valid_rectangle.x_end + valid_rectangle.x_start) * inc_delay) >> 1;
    tx_dqs = ((valid_rectangle.y_end + valid_rectangle.y_start) * inc_delay) >> 1;
    PSRAM_TRACE_NOTS(2,"rx_dqs: %d , tx_dqs: %d ",rx_dqs, tx_dqs);

#if defined(PSRAM_XCCELA_MODE) || defined(PSRAM_HYPERBUS_MODE)
    psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_dqs) |
                         PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(0) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs);
#else
    psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_clk) |
                         PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(tx_dqs) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs);
#endif
}

void hal_psram_calib(uint32_t clk)
{
    uint32_t phy_clk;
    uint32_t range;
    PSRAM_TRACE(2, "%s, speed:%d", __func__, clk);
    phy_clk = clk;
    if (phy_clk <= 100000000 / 2) {
        range = 3;
    } else if (phy_clk <= 150000000 / 2) {
        range = 2;
    } else if (phy_clk <= 300000000 / 2) {
        range = 1;
    } else {
        range = 1;
    }

    hal_psram_calib_range(range);
}

void hal_psram_init(void)
{
    pmu_psram_freq_config(psram_run_clk);

#ifdef CHIP_BEST2001
    // For code AHB cache on 2001
    hal_cache_wrap_enable(HAL_CACHE_ID_I_CACHE);
#endif
    // For BES psram cache (or system AHB cache on 2001)
    hal_cache_wrap_enable(HAL_CACHE_ID_D_CACHE);

    hal_cmu_mem_set_freq(HAL_CMU_FREQ_104M);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_PSRAM);
    hal_cmu_clock_enable(HAL_CMU_MOD_H_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_PSRAM);

    if (phy_bypass) {
        hal_psram_phy_init(psram_cfg_clk);
        hal_psram_digphy_init();
        hal_psram_mc_init(psram_cfg_clk);
        hal_psramip_set_cmd(MEMIF_NOP);
        psram_reset();
        // psram_chip_timing_config(psram_run_clk, true);

        psram_set_timing(psram_run_clk);
        psram_mc_set_freq(psram_run_clk);
        hal_psram_mc_set_timing(psram_run_clk, false);
        hal_psram_snoop_enable(HAL_PSRAM_ID_0);
        return;
    }

    hal_psram_clock_gate_disable();
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_phy_init(psram_cfg_clk);
    hal_sys_timer_delay_us(100);
#endif
    hal_psram_digphy_init();
    hal_psram_mc_init(psram_cfg_clk);
    hal_psramip_set_cmd(MEMIF_NOP);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif

#ifdef PSRAM_RESET
    psram_reset();
    psram_chip_timing_config(psram_run_clk, true);
#else
    uint32_t reg;
    uint32_t val;

    reg = 4;
    psram_read_reg(reg, &val);
    if (val & MR4_WL) {
        psram_chip_timing_config(psram_run_clk, false);
    } else {
        psram_chip_timing_config(psram_run_clk, true);
    }
#endif

    hal_psram_snoop_disable(HAL_PSRAM_ID_0);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_calib(psram_run_clk);
#endif
    hal_psram_snoop_enable(HAL_PSRAM_ID_0);

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_auto_low_power_enable();
#endif
#ifdef PSRAM_CLOCK_GATE_ENABLE
    hal_psram_clock_gate_enable();
#endif
}

#endif

#endif // PSRAM_CTRL_BASE
