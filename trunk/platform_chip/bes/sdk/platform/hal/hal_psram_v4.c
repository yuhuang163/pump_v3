/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#if defined(CHIP_HAS_PSRAM) && (CHIP_PSRAM_CTRL_VER == 4)

#include "plat_types.h"
#include "plat_addr_map.h"

#ifdef PSRAM_CTRL_BASE

#include "cmsis.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "reg_psram_mc_v4.h"
#include "reg_psram_phy_v4.h"
#include "string.h"

#define PSRAM_XCCELA_MODE

//#define PSRAM_XCCELA_DUAL_WORKAROUND
//#define PSRAM_DUAL_8BIT
//#define PSRAM_X16_MODE
//#define PSRAM_X8_MODE

#define PSRAM_DUAL_ENTRY
#define PSRAM_AUTO_LOW_POWER
//#define PSRAM_SNOOP_DISABLE

//#define PSRAM_CALIB_TEST
//#define PSRAM_DUAL_SWITCH

#ifdef PSRAM_XCCELA_DUAL_WORKAROUND
#undef PSRAM_X8_MODE
#undef PSRAM_X16_MODE
#if !defined(PSRAM_DUAL_8BIT)
#define PSRAM_DUAL_8BIT
#endif
#elif defined(PSRAM_DUAL_8BIT)
#undef PSRAM_X8_MODE
#undef PSRAM_X16_MODE
#elif defined(PSRAM_X8_MODE)
#undef PSRAM_X16_MODE
#elif !defined(PSRAM_X16_MODE)
#define PSRAM_X16_MODE
#endif

#ifndef PSRAM_APS3208K
    //#define PSRAM_WRAP_ENABLE
#endif

#ifdef PSRAM_1V2_ENABLE
#ifndef PSRAM_XCCELA_MODE
#define PSRAM_XCCELA_MODE
#endif
#endif

#ifdef PSRAM_XCCELA_MODE
#define PSRAM_TX_CEB_DLY_OFFSET             8
#endif

#if (defined(PSRAM_DUAL_8BIT) && !defined(PSRAM_XCCELA_DUAL_WORKAROUND))
#define PSRAM_PAGE_ZIZE_EXTEND  (2)
#else
#define PSRAM_PAGE_ZIZE_EXTEND  (1)
#endif

#ifndef PSRAM_PAGE_SIZE
#ifdef PSRAM_XCCELA_MODE
// APS128xxN APS256xxN APS512xxN
#define PSRAM_PAGE_SIZE                     (2048 * PSRAM_PAGE_ZIZE_EXTEND)
#else /* PSRAM_XCCELA_MODE */
// APS3208K APS6408L
#define PSRAM_PAGE_SIZE                     (1024 * PSRAM_PAGE_ZIZE_EXTEND)
#endif /* PSRAM_XCCELA_MODE */
#endif

#ifdef PSRAM_AUTO_LOW_POWER
#ifdef PSRAM_WINDOW_TEST
#undef PSRAM_AUTO_LOW_POWER
#else
#define PSRAM_QUEUE_IDLE_TIME_US            (10)
#endif
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

#define PSRAM_CALIB_TEST_CTRL               RAM_REGION_BASE

#define TX_FIFO_DEPTH                       8
#define RX_FIFO_DEPTH                       8

#define PHY_CA_CMD_CYCLE_HALF               (0 << 0)
#define PHY_CA_CMD_CYCLE_1                  (1 << 0)
#define PHY_CA_ADDR_BYTES_3                 (0 << 1)
#define PHY_CA_ADDR_BYTES_4                 (1 << 1)
#define PHY_CA_ADDR_MODE_SDR                (0 << 2)
#define PHY_CA_ADDR_MODE_DDR                (1 << 2)

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
#define MR8_RSVD_MASK                       (0x3 << MR8_RSVD_SHIFT)
#define MR8_RSVD(n)                         BITFIELD_VAL(MR8_RSVD, n)
#define MR8_X16                             (1 << 6)
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
static uint32_t psram_sleep_time;

#ifdef CORE_SLEEP_POWER_DOWN
uint32_t psramphy_save_range = 0;
uint32_t psramphy_save_reg_058 = 0;
uint32_t psramphy_save_reg_05c = 0;
#endif

#ifdef FPGA
    static const uint32_t psram_cfg_clk = 20 * 1000 * 1000;
    static const uint32_t psram_run_clk = 20 * 1000 * 1000;
#else /*FPGA*/
        static const uint32_t psram_cfg_clk =
    #ifdef PSRAM_CFG_SPEED
            PSRAM_CFG_SPEED * 1000 * 1000;
    #else
            24 * 1000 * 1000;
    #endif
    #if (PSRAM_SPEED != 0)
        static const uint32_t psram_run_clk = PSRAM_SPEED * 1000 * 1000;
    #else
        #error "invalid PSRAM_SPEED"
    #endif
#endif /*FPGA*/

#ifdef PSRAM_CALIB_ON_1D
static HAL_PSRAM_WINDOW_1D_CB_T hal_psram_window_1d_cb_func = NULL;

void hal_psram_window_1d_cb_register(enum HAL_PSRAM_ID_T id, HAL_PSRAM_WINDOW_1D_CB_T func)
{
    hal_psram_window_1d_cb_func = func;
}
#else
static HAL_PSRAM_WINDOW_2D_CB_T hal_psram_window_2d_cb_func = NULL;

void hal_psram_window_2d_cb_register(enum HAL_PSRAM_ID_T id, HAL_PSRAM_WINDOW_2D_CB_T func)
{
    hal_psram_window_2d_cb_func = func;
}
#endif

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


POSSIBLY_UNUSED void psram_read_reg(uint32_t reg, uint32_t *val)
{
    hal_psramip_flush_all_fifo();
    hal_psramip_xfer_addr_len(reg, 1);
    hal_psramip_set_cmd(MEMIF_MRR);
    while (hal_psramip_rx_fifo_empty());
    hal_psramip_read_fifo(val, 1);
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
    //uint32_t read_val;

    psram_send_cmd_reg(MEMIF_MRS, reg, val);
    //psram_read_reg(reg, &read_val);
    //TRACE(0, "MRS reg 0x%X: w=0x%08X r=0x%08X", reg, val, read_val);
}

static void psram_single_cmd(enum MEMIF_CMD_T cmd)
{
    hal_psramip_flush_all_fifo();
    hal_psramip_set_cmd(cmd);
    hal_psramip_mc_busy_wait();
}

static POSSIBLY_UNUSED void psram_reset(void)
{
    psram_single_cmd(MEMIF_RST);
}

static void psram_set_timing(uint32_t clk)
{
    uint32_t reg;
    uint32_t val;

#ifdef PSRAM_XCCELA_MODE
    reg = 8;
#ifdef PSRAM_WRAP_ENABLE
#if defined(PSRAM_X16_MODE) || defined(PSRAM_DUAL_8BIT)
    // Wrap 16x2
    val = MR8_BL(0);
#else
    // Wrap 32
    val = MR8_BL(1);
#endif
#else
    // Wrap page
    val = MR8_BL(3);
#endif
#if defined(PSRAM_X16_MODE) || defined(PSRAM_XCCELA_DUAL_WORKAROUND)
    val |= MR8_X16;
#endif
    psram_write_reg(reg, val);

    reg = 0;
#ifdef PSRAM_1V2_ENABLE
    if (clk <= 66000000) {
        val = 1;
    } else if (clk <= 109000000) {
        val = 2;
    } else if (clk <= 133000000) {
        val = 3;
    } else if (clk <= 166000000) {
        val = 4;
    } else {
        val = 5; // 6
    }
#else
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
#ifdef CHIP_BEST1600
    val = 4;
#endif
#endif
    // Latency type: Variable
    val = MR0_DRIVE_STR(3) | MR0_READ_LATENCY(val);
    psram_write_reg(reg, val);

    reg = 4;
#ifdef PSRAM_1V2_ENABLE
    if (clk <= 66000000) {
        val = 2;
    } else if (clk <= 109000000) {
        val = 6;
    } else if (clk <= 133000000) {
        val = 1;
    } else if (clk <= 166000000) {
        val = 5;
    } else {
        val = 3; // 7
    }
#else
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
#endif
    //enable 0.5x RF to lower power
    val = MR4_PASR(0) | MR4_WRITE_LATENCY(val) | MR4_RF_RATE(MR4_RF_RATE_HALF);
    psram_write_reg(reg, val);
#else /*PSRAM_XCCELA_MODE*/
#ifndef PSRAM_APS3208K
    reg = 8;
#ifdef PSRAM_WRAP_ENABLE
#ifdef PSRAM_DUAL_8BIT
    // Wrap 16x2
    val = MR8_BL(0);
#else
    // Wrap 32
    val = MR8_BL(1);
#endif
#else
    // Wrap page
    val = MR8_BL(3);
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
    //Fast Refresh,
    val = MR4_PASR(0) | MR4_WRITE_LATENCY(val);
    psram_write_reg(reg, val);
#endif /*PSRAM_XCCELA_MODE*/
}

POSSIBLY_UNUSED
static void hal_psram_phy_wait_lock()
{
    uint32_t start;

    start = hal_sys_timer_get();
    while ((psram_phy->REG_060 & PSRAM_ULP_PHY_PSRAM_DLL_LOCK) == 0 &&
            hal_sys_timer_get() - start < MS_TO_TICKS(2)) {};
    ASSERT((psram_phy->REG_060 & PSRAM_ULP_PHY_PSRAM_DLL_LOCK), "%s: Failed to wait lock", __func__);
}

static void hal_psram_phy_set_range(uint32_t range)
{
    hal_cmu_psram_dll_reset_set();
    hal_cmu_psram_dll_clock_disable();
    psram_phy->REG_048 = SET_BITFIELD(psram_phy->REG_048, PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE, range);
    hal_sys_timer_delay_us(10);
    hal_cmu_psram_dll_clock_enable();
    hal_cmu_psram_dll_reset_clear();
    hal_sys_timer_delay_us(200);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_phy_wait_lock();
#endif
}

uint32_t hal_psram_phy_get_range_val(enum HAL_PSRAM_ID_T id)
{
    uint32_t val;

    val = psram_phy->REG_048;
    val = GET_BITFIELD(val, PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE);

    return val;
}

uint32_t hal_psram_phy_get_dly_in_val(enum HAL_PSRAM_ID_T id)
{
    uint32_t val;

    val = psram_phy->REG_060;
    val = GET_BITFIELD(val, PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN);

    return val;
}

static uint32_t hal_psram_phy_get_init_range(uint32_t clk)
{
    uint32_t range;

    if (clk <= 150000000) {
        range = 3;
    } else if (clk <= 250000000) {
        range = 2;
    } else if (clk <= 350000000) {
        range = 1;
    } else {
        range = 0;
    }
    return range;
}

static void hal_psram_phy_dll_config(uint32_t clk)
{
    uint32_t range;

    range = hal_psram_phy_get_init_range(clk);
    hal_psram_phy_set_range(range);
}

static void hal_psram_phy_power_up(void)
{
    int dual = false;

#if defined(PSRAM_X16_MODE) || defined(PSRAM_DUAL_8BIT)
    dual = true;
#endif

    hal_cmu_psram_phy_init(dual);
}

static void hal_psram_phy_init(uint32_t clk)
{
    hal_psram_phy_power_up();
    hal_psram_phy_dll_config(clk);
}

static void hal_psram_digphy_init()
{
    uint32_t val;

    psram_phy->REG_050 = SET_BITFIELD(psram_phy->REG_050, PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL, 2);
    psram_phy->REG_054 = SET_BITFIELD(psram_phy->REG_054, PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL2, 2);

#ifdef  PSRAM_1V2_ENABLE
    psram_phy->REG_050 = SET_BITFIELD(psram_phy->REG_050, PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE, 15);
    psram_phy->REG_054 = SET_BITFIELD(psram_phy->REG_054, PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE2, 15);
#endif

    val = psram_phy->REG_004;
    val &= ~PSRAM_ULP_PHY_IDLE_DQ_OEN;
#ifdef PSRAM_XCCELA_MODE
    val |= PSRAM_ULP_PHY_DQS_DM_MERGE_EN;
    val &= ~PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN;
#endif
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_CTRL_DELAY, 0);
    val &= ~PSRAM_ULP_PHY_PHY_DLY_AUTO_EN;
    val &= ~(PSRAM_ULP_PHY_REG_WR_WO_DM | PSRAM_ULP_PHY_CS_POL);
    psram_phy->REG_004 = val;

#ifdef PSRAM_XCCELA_MODE
    uint32_t seq;

    seq = PHY_CA_CMD_CYCLE_1 | PHY_CA_ADDR_BYTES_4 | PHY_CA_ADDR_MODE_DDR;
    val = psram_phy->REG_020;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_CA_SEQ, seq);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_RX_DLY_OF_DESIGN, 2);
    psram_phy->REG_020 = val;
#endif
}

POSSIBLY_UNUSED void hal_psram_phy_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    hal_cmu_psram_phy_sleep();
}

POSSIBLY_UNUSED static void hal_psram_phy_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    int dual = false;

#if defined(PSRAM_X16_MODE) || defined(PSRAM_DUAL_8BIT)
    dual = true;
#endif
    hal_cmu_psram_phy_wakeup(dual);
}

static void hal_psram_mc_set_timing(uint32_t clk, bool init)
{
    uint32_t val;

#ifdef PSRAM_XCCELA_MODE
    if (init) {
        val = PSRAM_ULP_MC_WRITE_LATENCY_0(1);
    } else {
#ifdef PSRAM_1V2_ENABLE
        if (clk <= 66000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(5);
        } else if (clk <= 109000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(6);
        } else if (clk <= 133000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(7);
        } else if (clk <= 166000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(8);
        } else {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(9); // 10
        }
#else
        if (clk <= 66000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(3);
        } else if (clk <= 109000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(4);
        } else if (clk <= 133000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(5);
        } else if (clk <= 166000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(6);
        } else {
            val = PSRAM_ULP_MC_WRITE_LATENCY_0(7);
        }
#endif
    }
    psram_mc->REG_028 = val;
#ifdef PSRAM_1V2_ENABLE
    if (clk <= 66000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(4);
    } else if (clk <= 109000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(5);
    } else if (clk <= 133000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(6);
    } else if (clk <= 166000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(7);
    } else {
        val = PSRAM_ULP_MC_READ_LATENCY_0(8); //9
    }
#else
    if (clk <= 66000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(3);
    } else if (clk <= 109000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(4);
    } else if (clk <= 133000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(5);
    } else if (clk <= 166000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(6);
    } else {
        val = PSRAM_ULP_MC_READ_LATENCY_0(7);
    }
#ifdef CHIP_BEST1600
    val = PSRAM_ULP_MC_READ_LATENCY_0(7);
#endif
#endif
    psram_mc->REG_02C = val;
    // tCPH >= 24 ns
    val = (clk / 1000000 * 24 + (1000 - 1)) / 1000;
#ifdef CHIP_BEST1600
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR_0((val > 2) ? (val - 2) : 0);
    // Avoid error when 2 writing operations arrive closely:
    uint32_t t_cphw_plus = 16;
    if (psram_run_clk <= 160 * 1000 * 1000) {
        t_cphw_plus = 11;
    }
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW_0(val + t_cphw_plus);
#else
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR_0(val);
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW_0(val);
#endif
    psram_mc->REG_068 = PSRAM_ULP_MC_T_MRR_0(val);
    psram_mc->REG_06C = PSRAM_ULP_MC_T_MRS_0(val);
#else // !PSRAM_XCCELA_MODE
    if (clk <= 166000000) {
        val = PSRAM_ULP_MC_WRITE_LATENCY_0(0);
    } else {
        val = PSRAM_ULP_MC_WRITE_LATENCY_0(2);
    }
    psram_mc->REG_028 = val;
    if (clk <= 66000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(2);
    } else if (clk <= 109000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(3);
    } else if (clk <= 133000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(4);
    } else if (clk <= 166000000) {
        val = PSRAM_ULP_MC_READ_LATENCY_0(5);
    } else {
        val = PSRAM_ULP_MC_READ_LATENCY_0(6);
    }
#ifdef CHIP_BEST1600
    val = PSRAM_ULP_MC_READ_LATENCY_0(6);
#endif
    psram_mc->REG_02C = val;
    // tCPH
    val = 2;
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR_0(val);
    psram_mc->REG_068 = PSRAM_ULP_MC_T_MRR_0(val);
    val = 6;
#ifdef CHIP_BEST1600
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW_0(val + 5);
#else
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW_0(val);
#endif
    psram_mc->REG_06C = PSRAM_ULP_MC_T_MRS_0(val);
#endif // !PSRAM_XCCELA_MODE
    // tRC >= 55 ns
    val = (clk / 1000000 * 55 + (1000 - 1)) / 1000;
    psram_mc->REG_050 = PSRAM_ULP_MC_T_RC_0(val);
    // tCEM <= 2.5 us
    val = clk / 1000000 * 25 / 10;
    psram_mc->REG_070 = PSRAM_ULP_MC_T_CEM_0(val);
    // tRST >= 2 us
    val = clk / 1000000 * 2 + 1;
    psram_mc->REG_074 = PSRAM_ULP_MC_T_RST_0(val);
    // tHS >= 150 us
    val = clk / 1000000 * 150 + 1;
    psram_mc->REG_080 = PSRAM_ULP_MC_T_HS_0(val);
    // tXPHS in [60 ns, 2 us]
    val = (clk / 1000000 * 60 + (1000 - 1)) / 1000;
    psram_mc->REG_084 = PSRAM_ULP_MC_T_XPHS_0(val);
    // tXHS >= 150us
    val = clk / 1000000 * 150 + 1;
    psram_mc->REG_088 = PSRAM_ULP_MC_T_XHS_0(val);
    psram_mc->REG_09C = PSRAM_ULP_MC_WR_DMY_CYC(1) | PSRAM_ULP_MC_CMD_DMY_CYC(2);
    // NOP dummy cycles, same as tXPHS in [60 ns, 2 us]
    val = (clk / 1000000 * 100 + (1000 - 1)) / 1000;
    psram_mc->REG_0A0 = PSRAM_ULP_MC_STOP_CLK_IN_NOP |
#ifdef PSRAM_XCCELA_MODE
                        PSRAM_ULP_MC_STOP_CLK_IN_TCPH |
#endif
                        PSRAM_ULP_MC_NOP_DMY_CYC(val);
    psram_mc->REG_0A4 = PSRAM_ULP_MC_QUEUE_IDLE_CYCLE(5000);

    psram_mc->REG_0C4 = PSRAM_ULP_MC_T_CSCKE(64) | PSRAM_ULP_MC_T_CSCKEH(64);
}

POSSIBLY_UNUSED
static void hal_psram_init_calib(void)
{
    uint32_t delay;

#if defined(FPGA) || defined(SIMU)
    delay = 10;
#else
    delay = GET_BITFIELD(psram_phy->REG_060, PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN);
    delay /= 2;
    delay = MIN(delay, (PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY_MASK>>PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY_SHIFT) / 2);
#endif

#ifdef PSRAM_XCCELA_MODE
    psram_phy->REG_058 = PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(delay > PSRAM_TX_CEB_DLY_OFFSET ? delay - PSRAM_TX_CEB_DLY_OFFSET : 0) |
        PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(delay) |
        PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY(0) |
        PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY(delay);
#else
    psram_phy->REG_058 = PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(delay) |
        PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(delay) |
        PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY(delay) |
        PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY(delay);
#endif
    psram_phy->REG_05C = psram_phy->REG_058;
    hal_sys_timer_delay_us(100);
}

#ifdef PSRAM_XCCELA_DUAL_WORKAROUND
static void hal_psram_ca_map_init_xccela(void)
{
    psram_mc->REG_180 =
        PSRAM_ULP_MC_CA_MAP_BIT0(31) |
        PSRAM_ULP_MC_CA_MAP_BIT1(0) |
        PSRAM_ULP_MC_CA_MAP_BIT2(1) |
        PSRAM_ULP_MC_CA_MAP_BIT3(2) |
        PSRAM_ULP_MC_CA_MAP_BIT4(3) |
        PSRAM_ULP_MC_CA_MAP_BIT5(4);
    psram_mc->REG_184 =
        PSRAM_ULP_MC_CA_MAP_BIT6(5) |
        PSRAM_ULP_MC_CA_MAP_BIT7(6) |
        PSRAM_ULP_MC_CA_MAP_BIT8(7) |
        PSRAM_ULP_MC_CA_MAP_BIT9(8) |
        PSRAM_ULP_MC_CA_MAP_BIT10(9) |
        PSRAM_ULP_MC_CA_MAP_BIT11(11);
    psram_mc->REG_188 =
        PSRAM_ULP_MC_CA_MAP_BIT12(12) |
        PSRAM_ULP_MC_CA_MAP_BIT13(13) |
        PSRAM_ULP_MC_CA_MAP_BIT14(14) |
        PSRAM_ULP_MC_CA_MAP_BIT15(15) |
        PSRAM_ULP_MC_CA_MAP_BIT16(16) |
        PSRAM_ULP_MC_CA_MAP_BIT17(17);
    psram_mc->REG_18C =
        PSRAM_ULP_MC_CA_MAP_BIT18(18) |
        PSRAM_ULP_MC_CA_MAP_BIT19(19) |
        PSRAM_ULP_MC_CA_MAP_BIT20(20) |
        PSRAM_ULP_MC_CA_MAP_BIT21(21) |
        PSRAM_ULP_MC_CA_MAP_BIT22(22) |
        PSRAM_ULP_MC_CA_MAP_BIT23(23);
    psram_mc->REG_190 =
        PSRAM_ULP_MC_CA_MAP_BIT24(24) |
        PSRAM_ULP_MC_CA_MAP_BIT25(25) |
        PSRAM_ULP_MC_CA_MAP_BIT26(26) |
        PSRAM_ULP_MC_CA_MAP_BIT27(27) |
        PSRAM_ULP_MC_CA_MAP_BIT28(28) |
        PSRAM_ULP_MC_CA_MAP_BIT29(29);
    psram_mc->REG_194 =
        PSRAM_ULP_MC_CA_MAP_BIT30(30) |
        PSRAM_ULP_MC_CA_MAP_BIT31(31);
}
#endif

static void hal_psram_mc_init(uint32_t clk)
{
    uint32_t val = 0;
#if defined(PSRAM_DUAL_8BIT)
    val |= PSRAM_ULP_MC_CHIP_BIT;
#ifdef PSRAM_DUAL_SWITCH
    val |= PSRAM_ULP_MC_CHIP_SWITCH;
#endif
#elif defined(PSRAM_X16_MODE)
    val |= PSRAM_ULP_MC_CHIP_X16;
#endif
#ifdef PSRAM_SQPI_MODE
    val |= PSRAM_ULP_MC_SQPI_MODE;
    val |= (PSRAM_ULP_PHY_IDLE_DQ_OEN | PSRAM_ULP_PHY_IDLE_DQS_OEN);
#else
    val |= PSRAM_ULP_MC_OPI_MODE;
    uint32_t addr_map_mode = 1;
#ifdef PSRAM_XCCELA_DUAL_WORKAROUND
    addr_map_mode = 0;
#elif defined(PSRAM_XCCELA_MODE)
    addr_map_mode = 2;
#endif
    val = SET_BITFIELD(val, PSRAM_ULP_MC_CHIP_CA_PATTERN, addr_map_mode);
#endif
    psram_mc->REG_000 = val;
    psram_mc->REG_020 = 0;
    val = 0;
#ifdef PSRAM_DUAL_ENTRY
    val |= PSRAM_ULP_MC_DUAL_ENTRY_MODE;
    val |= PSRAM_ULP_MC_DIS_OUT_OF_ORDER;
#endif
    psram_mc->REG_030 = val;
#ifdef PSRAM_WRAP_ENABLE
    // Burst len: 32 bytes
    val = PSRAM_ULP_MC_BURST_LENGTH_0(1);
#else
    // Burst len: page size
    if (PSRAM_PAGE_SIZE == 512) {
        val = PSRAM_ULP_MC_BURST_LENGTH_0(7);
    } else if (PSRAM_PAGE_SIZE == 1024) {
        val = PSRAM_ULP_MC_BURST_LENGTH_0(4);
    } else if (PSRAM_PAGE_SIZE == 2048) {
        val = PSRAM_ULP_MC_BURST_LENGTH_0(5);
    } else {
        // 4K
        val = PSRAM_ULP_MC_BURST_LENGTH_0(6);
    }
#endif
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
    // Write buffer level with high priority: 1~8
#ifdef PSRAM_SNOOP_DISABLE
    // Write operations must take precedence to avoid reading out old values
    psram_mc->REG_03C = PSRAM_ULP_MC_HIGH_PRI_LEVEL(1);
#else
    // Read operations take precedence if write level is low (for read performance)
    psram_mc->REG_03C = PSRAM_ULP_MC_HIGH_PRI_LEVEL(8);
#endif
#ifdef PSRAM_WRAP_ENABLE
    psram_mc->REG_040 = PSRAM_ULP_MC_CP_WRAP_EN_0;
#else
    psram_mc->REG_040 = 0; //PSRAM_ULP_MC_WRAP_CRT_RET_EN;
#endif
    psram_mc->REG_044 = PSRAM_ULP_MC_RD_CANCEL_EN; //0;

#ifdef PSRAM_XCCELA_DUAL_WORKAROUND
    hal_psram_ca_map_init_xccela();
#endif

    hal_psramip_set_reg_data_mask();

    hal_psram_mc_set_timing(clk, true);

    psram_mc->REG_400 = PSRAM_ULP_MC_INIT_COMPLETE;
}

#ifdef PSRAM_AUTO_LOW_POWER
static void hal_psram_auto_low_power_enable(void)
{
    psram_mc->REG_024 =
        PSRAM_ULP_MC_AUTOWAKEUP_EN |
#ifdef PSRAM_DIE_HALF_SLEEP_ENABLE
        PSRAM_ULP_MC_PD_MR(6) |
        PSRAM_ULP_MC_PD_CMD(0xf0);
#else
        PSRAM_ULP_MC_PD_MR(0x2) |
        PSRAM_ULP_MC_PD_CMD(0x6);
#endif
    psram_mc->REG_0A4 = PSRAM_ULP_MC_QUEUE_IDLE_CYCLE(psram_run_clk / 1000000 * PSRAM_QUEUE_IDLE_TIME_US);
    psram_mc->REG_198 |= PSRAM_ULP_MC_AUTO_PD_ENTER_EN;
    hal_cmu_psram_auto_lp_cfg_set();
    psram_mc->REG_024 |= PSRAM_ULP_MC_ENTRY_SLEEP_IDLE;

    psram_mc->REG_080 = PSRAM_ULP_MC_T_HS_0(psram_run_clk / 1000000 * 1 + 1);
    // tXHS >= 10us  wait for phy power up
    psram_mc->REG_088 = PSRAM_ULP_MC_T_XHS_0(psram_run_clk / 1000000 * 10 + 1);
}

static void hal_psram_auto_low_power_disable(void)
{
    uint32_t val;

    val = psram_run_clk / 1000000 * 150 + 1;
    psram_mc->REG_080 = PSRAM_ULP_MC_T_HS_0(val);
    val = psram_run_clk / 1000000 * 150 + 1;
    psram_mc->REG_088 = PSRAM_ULP_MC_T_XHS_0(val);

    psram_mc->REG_024 =
        PSRAM_ULP_MC_AUTOWAKEUP_EN |
        PSRAM_ULP_MC_PD_MR(6) |
        PSRAM_ULP_MC_PD_CMD(0xf0);
    // hal_sys_timer_delay_us(10);
    hal_psramip_set_cmd(MEMIF_NOP);
    hal_cmu_psram_auto_lp_cfg_clear();
}
#endif

static void psram_mc_set_freq(uint32_t clk)
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

void hal_psram_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        psramphy_save_reg_058 = psram_phy->REG_058;
        psramphy_save_reg_05c = psram_phy->REG_05C;
        psramphy_save_range = hal_psram_phy_get_range_val(HAL_PSRAM_ID_0);
    }
#endif

    __DSB();
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
    hal_psramip_wb_busy_wait();
    hal_psramip_mc_busy_wait();

    psram_mc->REG_19C |= PSRAM_ULP_MC_SW_BUS_HALT_EN;
    __DSB();
    while ((psram_mc->REG_19C & PSRAM_ULP_MC_SW_BUS_HALT_STS) == 0);

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_auto_low_power_disable();
#endif

#ifdef PSRAM_XCCELA_MODE
    int val = psram_mc->REG_028;
    psram_mc->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY_0(1);
    __DSB();
    hal_psramip_mc_busy_wait();
#endif

    hal_psramip_set_cmd(MEMIF_PD);
    hal_psramip_mc_busy_wait();

#ifdef PSRAM_XCCELA_MODE
    psram_mc->REG_028 = val;
    hal_psramip_mc_busy_wait();
#endif

    hal_psram_phy_sleep(mode);
    psram_sleep_time = hal_sys_timer_get();
}

void hal_psram_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    uint32_t val;

    hal_psram_phy_wakeup(mode);

    if (0) {
#ifdef CORE_SLEEP_POWER_DOWN
    } else if ((mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) && (hal_psramip_is_mc_reset())) {
        hal_cmu_psram_dll_reset_set();
        hal_cmu_psram_dll_clock_disable();
        psram_phy->REG_048 = SET_BITFIELD(psram_phy->REG_048, PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE, psramphy_save_range);
        hal_cmu_psram_dll_clock_enable();
        hal_cmu_psram_dll_reset_clear();

        hal_psram_digphy_init();
        psram_phy->REG_058 = psramphy_save_reg_058;
        psram_phy->REG_05C = psramphy_save_reg_05c;
        psram_mc_set_freq(psram_run_clk);
        hal_psram_mc_init(psram_run_clk);
        hal_sys_timer_delay_us(2);
        hal_psram_mc_set_timing(psram_run_clk, false);

        // tHS >= 150 us
        val = psram_mc->REG_080;
        if (hal_sys_timer_get() - psram_sleep_time >= US_TO_TICKS(150) + 1) {
            psram_mc->REG_080 = PSRAM_ULP_MC_T_HS_0(0);
        }
        hal_psramip_set_cmd(MEMIF_PD);
        hal_psramip_mc_busy_wait();
#endif
    } else {
        val = psram_mc->REG_080;
        // tHS >= 150 us
        if (hal_sys_timer_get() - psram_sleep_time >= US_TO_TICKS(150) + 1) {
            psram_mc->REG_080 = PSRAM_ULP_MC_T_HS_0(0);
        }
    }

    hal_psramip_set_cmd(MEMIF_NOP);
    hal_psramip_mc_busy_wait();

    psram_mc->REG_080 = val;

#ifdef PSRAM_AUTO_LOW_POWER
    // tXHS >= 150 us
    hal_sys_timer_delay_us(150);
    hal_psram_auto_low_power_enable();
#endif

    psram_mc->REG_19C &= ~PSRAM_ULP_MC_SW_BUS_HALT_EN;
    __DSB();
}

static void psram_chip_timing_config(uint32_t clk, bool update_psram_first)
{
    if (update_psram_first) {
        psram_set_timing(clk);
    }

    psram_mc_set_freq(clk);
    hal_sys_timer_delay_us(2);
    hal_psram_phy_dll_config(clk);
    hal_psram_init_calib();
    hal_psram_mc_set_timing(clk, false);
    if (!update_psram_first) {
        psram_set_timing(clk);
    }
    hal_sys_timer_delay_us(2);
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

void hal_psram_snoop_enable(enum HAL_PSRAM_ID_T id)
{
#ifndef PSRAM_SNOOP_DISABLE
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

POSSIBLY_UNUSED static void psram_mc_reset(uint32_t range)
{
    hal_cmu_psram_soft_reset();

    psram_mc_set_freq(psram_cfg_clk);
    hal_psram_phy_init(psram_cfg_clk);
    hal_sys_timer_delay_us(30);
    hal_psram_mc_init(psram_cfg_clk);
    hal_psram_init_calib();
    psram_reset();
    psram_chip_timing_config(psram_run_clk, true);
    hal_psram_phy_set_range(range);
    hal_psram_init_calib();
}

uint32_t hal_psram_ca_calib_result(void)
{
    uint32_t val;
    uint32_t toggling = 0;

    val = psram_phy->REG_00C;
    psram_phy->REG_00C = PSRAM_ULP_PHY_T_EXPANDRD(0x3F);
    psram_phy->REG_01C = PSRAM_ULP_PHY_CA_TR_EN;

    hal_psramip_flush_all_fifo();
    hal_psramip_xfer_addr_len(32, 1);
    hal_psramip_set_cmd(MEMIF_MRR);

    while (!(psram_mc->REG_01C & PSRAM_ULP_PHY_DQS_RDY));
    toggling = psram_mc->REG_01C & PSRAM_ULP_PHY_DQS_STS;
    psram_mc->REG_01C = 0;
    psram_phy->REG_00C = val;
    hal_psramip_mc_busy_wait();

    return !!toggling;
}

int hal_psram_change_clk(uint32_t clk)
{
    uint32_t range ,val ,delay;

    psram_chip_timing_config(clk, true);
    range = GET_BITFIELD(psram_phy->REG_048, PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE);
    while (1) {
        if (range > (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
            range = (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT);
        }
        hal_psram_phy_set_range(range);
        val = psram_phy->REG_060;
        delay = GET_BITFIELD(psram_phy->REG_060, PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN);
        if (val & PSRAM_ULP_PHY_PSRAM_DLL_ALL_ONE) {
            PSRAM_TRACE(2, "%s: range=%d all one", __func__, range);
            if (range < (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
                range++;
                continue;
            }
        }
        PSRAM_TRACE(4, "%s, range=%d T/4=0x%x (REG60=0x%x)", __func__, range, delay / 2, val);
        if (delay > (PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT)) {
            PSRAM_TRACE(2, "%s: range=%d bad delay (T/2 > max_tx_dqs_dly) T/2 = 0x%x", __func__, range , delay);
            if (range < (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
                range++;
                continue;
            }
        }
        break;
    }
    //return MIN(delay, (PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT));
    return delay;
}

#ifdef PSRAM_CALIB_ON_1D
static uint32_t pattern[0x10] = {0};

static void psram_init_pattern_array()
{
    pattern[0x0] = 0xffff;
    pattern[0x1] = 0xffff;
    pattern[0x2] = 0xffff;
    pattern[0x3] = 0xffff;
    pattern[0x4] = 0x0;
    pattern[0x5] = 0xffffffff;
    pattern[0x6] = 0x0;
    pattern[0x7] = 0xffffffff;
    pattern[0x8] = 0xa55aa55a;
    pattern[0x9] = 0x5aa55aa5;
    pattern[0xa] = 0x01234567;
    pattern[0xb] = 0x89abcdef;
    pattern[0xc] = 0x98f79d18;
    pattern[0xd] = 0xe416069e;
    pattern[0xe] = 0xa51a3e8e;
    pattern[0xf] = 0xd10dbbbd;
}

POSSIBLY_UNUSED void psram_write_pattern()
{
    volatile uint32_t *psram_base = (volatile uint32_t *)PSRAM_NC_BASE;

    for(int i = 0; i < ARRAY_SIZE(pattern); i++) {
       psram_base[i] =  pattern[i];
    }
    __DSB();
    hal_psram_wait_write_buffer_empty(HAL_PSRAM_ID_0);
    hal_psramip_wb_busy_wait();
    hal_psramip_mc_busy_wait();
}

//0  pass
//!0 fail
static int psram_read_pattern_and_check(bool clear_after_read)
{
    int ret = 0;
    volatile uint32_t *psram_base = (volatile uint32_t *)PSRAM_NC_BASE;

    for(int i = 0; i < ARRAY_SIZE(pattern); i++) {
        //TRACE(5,"%x:: 0x%08x, 0x%08x",i,psram_base[i] ,pattern[i]);
        if (psram_base[i] !=  pattern[i])  {
            ret = 1;
        }
        if (clear_after_read) {
            psram_base[i] = 0;
        }
    }
    return ret;
}

//return  0  low speed test pass
//        !0 low speed test fail
static int psram_pattern_init_check(void)
{
    psram_write_pattern();
    return (psram_read_pattern_and_check(false));
}

static bool psram_find_win_of_dqs(uint8_t const *dqs_array, struct HAL_PSRAM_DQS_BOUNDARY_T *boundar)
 {
    uint8_t stage = 0;
    uint8_t _l = 0, _r = PSRAM_WINDOW_SIZE - 1;
    bool is_legal = 1;

    for (int i = 0; i < PSRAM_WINDOW_SIZE; i++) {
        switch (stage)
        {
            case 0:
                _l = i;
                if (dqs_array[i] == 1) {
                    stage = 1;
                }
                break;
            case 1:
                if (dqs_array[i] == 0) {
                    _r = i -1;
                    stage = 2;
                }
                break;
            default:
                if (dqs_array[i] == 1) {
                    is_legal = 0;
                }
                break;
        }
    }
    if (_l >= _r) {
        is_legal = 0;
    }
    boundar ->l = _l;
    boundar ->r = _r;
    boundar ->is_legal = is_legal;
    return is_legal;
}

 POSSIBLY_UNUSED static uint32_t psram_rx_dqs_calib(void)
 {
    POSSIBLY_UNUSED uint8_t tx_ceb, tx_clk, tx_dqs, rx_dqs;
    uint8_t cali_valid_rx_dqs[PSRAM_WINDOW_SIZE];
    struct HAL_PSRAM_DQS_BOUNDARY_T boundary;

    memset(cali_valid_rx_dqs, 0, sizeof(cali_valid_rx_dqs));
    for (rx_dqs = 0; rx_dqs < PSRAM_WINDOW_SIZE; rx_dqs++) {
        psram_phy->REG_058 = SET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY, rx_dqs);
        psram_phy->REG_05C = psram_phy->REG_058;
        __DSB();
        cali_valid_rx_dqs[rx_dqs] = !psram_read_pattern_and_check(false);
#ifndef PSRAM_WINDOW_TEST
        if (rx_dqs) {
            if ((cali_valid_rx_dqs[rx_dqs - 1] == 1) && (cali_valid_rx_dqs[rx_dqs] == 0)) {
                break;
            }
        }
#endif
    }
    psram_find_win_of_dqs(cali_valid_rx_dqs, &boundary);
    if ((boundary.r > boundary.l)) {
        rx_dqs = ((boundary.l + boundary.r) >> 1);
    } else {
        ASSERT(0, "%s :rx_dqs calib fail", __func__);
    }
#ifdef PSRAM_WINDOW_TEST
    if (hal_psram_window_1d_cb_func != NULL) {
        struct HAL_PSRAM_WINDOW_1D_ANALYSIS_T a;
        a.range = GET_BITFIELD(psram_phy->REG_048, PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE);
        a.t_div_2 = GET_BITFIELD(psram_phy->REG_060, PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN);
        //a.t_div_2 = MIN(a.t_div_2, (PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT));
        a.item = PSRAM_CALIB_RX_DQS;
        a.raw_data = &cali_valid_rx_dqs[0];
        a.boundary = &boundary;
        hal_psram_window_1d_cb_func(&a);
    }
#endif
    // apply the calibrated rx_dqs
    psram_phy->REG_058 = SET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY, rx_dqs);
    psram_phy->REG_05C = psram_phy->REG_058;

    return rx_dqs;
}

POSSIBLY_UNUSED static uint32_t psram_tx_dqs_calib(void)
{
    POSSIBLY_UNUSED uint8_t tx_ceb, tx_clk, tx_dqs, rx_dqs;
    uint8_t cali_valid_tx_dqs[PSRAM_WINDOW_SIZE];
    struct HAL_PSRAM_DQS_BOUNDARY_T boundary;

    memset(cali_valid_tx_dqs, 0, sizeof(cali_valid_tx_dqs));
    for (tx_dqs = 0; tx_dqs < PSRAM_WINDOW_SIZE; tx_dqs++) {
#ifdef PSRAM_XCCELA_MODE
        psram_phy->REG_058 = SET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY, tx_dqs);
#else
        psram_phy->REG_058 = SET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY, tx_dqs);
#endif
        psram_phy->REG_05C = psram_phy->REG_058;
        __DSB();
        psram_write_pattern();
        cali_valid_tx_dqs[tx_dqs] = !psram_read_pattern_and_check(true);
#ifndef PSRAM_WINDOW_TEST
        if (tx_dqs) {
            if ((cali_valid_tx_dqs[tx_dqs - 1] == 1) && (cali_valid_tx_dqs[tx_dqs] == 0)) {
                break;
            }
        }
#endif
    }
    psram_find_win_of_dqs(cali_valid_tx_dqs, &boundary);
    if ((boundary.r > boundary.l)) {
        tx_dqs = ((boundary.l + boundary.r) >> 1);
    } else {
        ASSERT(0, "%s :tx_dqs calib fail", __func__);
    }

#ifdef PSRAM_WINDOW_TEST
     if (hal_psram_window_1d_cb_func != NULL) {
        struct HAL_PSRAM_WINDOW_1D_ANALYSIS_T a;
        a.range = GET_BITFIELD(psram_phy->REG_048, PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE);
        a.t_div_2 = GET_BITFIELD(psram_phy->REG_060, PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN);
        //a.t_div_2 = MIN(a.t_div_2, (PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT));
        a.item = PSRAM_CALIB_TX_DQS;
        a.raw_data = &cali_valid_tx_dqs[0];
        a.boundary = &boundary;
        hal_psram_window_1d_cb_func(&a);
    }
#endif
    // apply the calibrated tx_dqs
#ifdef PSRAM_XCCELA_MODE
    psram_phy->REG_058 = SET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY, tx_dqs);
#else
    psram_phy->REG_058 = SET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY, tx_dqs);
#endif
    psram_phy->REG_05C = psram_phy->REG_058;

#ifdef PSRAM_WINDOW_TEST
    psram_write_pattern();
#endif

    return tx_dqs;
}

POSSIBLY_UNUSED static uint32_t psram_tx_clk_calib(void)
{
    return GET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY);
}

POSSIBLY_UNUSED static uint32_t psram_tx_ceb_calib(void)
{
    return GET_BITFIELD(psram_phy->REG_058, PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY);
}

uint32_t static hal_psram_get_dll_delay(uint32_t range)
{
    uint32_t val, delay;

    while (1) {
        if (range > (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
            range = (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT);
        }
        hal_psram_phy_set_range(range);
        val = psram_phy->REG_060;
        delay = GET_BITFIELD(psram_phy->REG_060, PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN);
        if (val & PSRAM_ULP_PHY_PSRAM_DLL_ALL_ONE) {
            PSRAM_TRACE(2, "%s: range=%d all one", __func__, range);
            if (range < (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
                range++;
                continue;
            }
        }
        PSRAM_TRACE(4, "%s, range=%d T/4=0x%x (REG60=0x%x)", __func__, range, delay / 2, val);
        if (delay > (PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT)) {
            PSRAM_TRACE(2, "%s: range=%d bad delay (T/2 > max_tx_dqs_dly) T/2 = 0x%x", __func__, range , delay);
            if (range < (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
                range++;
                continue;
            }
        }
        break;
    }
    return MIN(delay, PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT);
}

static void hal_psram_calib_on_1d(uint32_t range)
{
    POSSIBLY_UNUSED uint8_t tx_ceb, tx_clk, tx_dqs, rx_dqs;
    uint32_t delay;
    tx_dqs = 0;
    rx_dqs = 0;
    delay = hal_psram_get_dll_delay(range);
    tx_ceb = delay / 2;
    tx_clk = delay / 2;
    psram_phy->REG_058 = PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(tx_clk);
    psram_phy->REG_05C = psram_phy->REG_058;
    __DSB();
    //note: rx tx must calib in order
    rx_dqs = psram_rx_dqs_calib();
    tx_dqs = psram_tx_dqs_calib();
    //tx_clk = psram_tx_clk_calib();
    //tx_ceb = psram_tx_ceb_calib();
    __DSB();
    //TRACE(0,"%d, %d",rx_dqs, tx_dqs);
}

#else

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

POSSIBLY_UNUSED static bool psramphy_check_tx_ceb_valid(uint32_t range)
{
    int retry = 2;

    hal_psram_wait_write_buffer_empty(HAL_PSRAM_ID_0);

    while (retry--) {
        int count = 0;
        hal_psramip_flush_all_fifo();
        hal_psramip_xfer_addr_len(0x100000, 8);
        hal_psramip_set_cmd(MEMIF_READ);
        while (hal_psramip_rx_fifo_empty()) {
            if (count++ > 10) {
                //PSRAM_TRACE(1, "%s, rx fifo empty, return false", __FUNCTION__);
                psram_mc_reset(range);
                return false;
            }
        }
    }
    return true;
}

POSSIBLY_UNUSED static bool psramphy_check_tx_clk_valid(uint32_t range)
{
    uint32_t time;

    hal_psram_wait_write_buffer_empty(HAL_PSRAM_ID_0);
    time = hal_fast_sys_timer_get();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR | PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psramip_xfer_addr_len(0xAA55AA55, 1);
    hal_psramip_set_cmd(MEMIF_READ);
    while (hal_psramip_rx_fifo_empty()) {
        if ((hal_fast_sys_timer_get() - time) > US_TO_FAST_TICKS(5)) {
            psram_phy->REG_004 |= PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            while (hal_psramip_mc_busy());
            psram_phy->REG_004 &= ~PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            return false;
        }
    }
    return true;
}

int hal_psram_mgr_write_4byte(uint32_t addr, uint32_t val)
{
    hal_psramip_set_mem_data_mask();
    hal_psramip_flush_all_fifo();
    hal_psramip_write_fifo(&val, 1);
    hal_psramip_xfer_addr_len(addr & 0x3ffffff, 1);
    hal_psramip_set_cmd(MEMIF_WRITE);

    while (hal_psramip_get_tx_fifo_free_len() != TX_FIFO_DEPTH) { }
    __DSB();
    hal_psram_wait_write_buffer_empty(HAL_PSRAM_ID_0);
    hal_psramip_wb_busy_wait();
    hal_psramip_mc_busy_wait();
    hal_psramip_set_reg_data_mask();

    return 0;
}

int hal_psram_mgr_read_4byte(uint32_t addr, uint32_t *val)
{
    hal_psramip_flush_all_fifo();
    hal_psramip_xfer_addr_len(addr & 0x3ffffff, 1); //64MB max
    hal_psramip_set_cmd(MEMIF_READ);
    while (hal_psramip_rx_fifo_empty()) { }
    hal_psramip_read_fifo(val, 1);

    return 0;
}

POSSIBLY_UNUSED static uint8_t psramphy_check_write_valid(void)
{
    uint32_t i;
    uint32_t val, val0, val1;
    uint32_t val2, val3;
    volatile uint32_t *psram_base = (volatile uint32_t *)PSRAM_NC_BASE;
    volatile uint32_t *psram_base1 = (volatile uint32_t *)(PSRAM_NC_BASE + 0x100000);
#if defined(PSRAM_DUAL_8BIT) || defined(PSRAM_X16_MODE)
    static const uint32_t mask_l = 0x00FF00FF;
    static const uint32_t mask_h = 0xFF00FF00;
    bool valid_l = true;
    bool valid_h = true;
#endif

    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
        *(psram_base + i) = val0;
        *(psram_base1 + i) = val1;
    }
    __DSB();
    hal_psram_wait_write_buffer_empty(HAL_PSRAM_ID_0);
    hal_psramip_wb_busy_wait();
    hal_psramip_mc_busy_wait();
    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
        val2 = *(psram_base + i);
        val3 = *(psram_base1 + i);
        // hal_psram_mgr_read_4byte((uint32_t)(psram_base + i), &val2);
        // hal_psram_mgr_read_4byte((uint32_t)(psram_base1 + i), &val3);
        if (val2 != val0) {
#if defined(PSRAM_DUAL_8BIT) || defined(PSRAM_X16_MODE)
            if (valid_l && ((val2 & mask_l) != (val0 & mask_l))) {
                PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val2, val0, mask_l);
                valid_l = false;
            }
            if (valid_h && ((val2 & mask_h) != (val0 & mask_h))) {
                PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val2, val0, mask_h);
                valid_h = false;
            }
            if (!valid_l && !valid_h) {
                return 0;
            }
#else
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x", __FUNCTION__, i, val2, val0);
            return 0;
#endif
        }
        if (val3 != val1) {
#if defined(PSRAM_DUAL_8BIT) || defined(PSRAM_X16_MODE)
            if (valid_l && ((val3 & mask_l) != (val1 & mask_l))) {
                PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val3, val1, mask_l);
                valid_l = false;
            }
            if (valid_h && ((val3 & mask_h) != (val1 & mask_h))) {
                PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val3, val1, mask_h);
                valid_h = false;
            }
            if (!valid_l && !valid_h) {
                return 0;
            }
#else
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x", __FUNCTION__, i, val3, val1);
            return 0;
#endif
        }
    }

#if defined(PSRAM_DUAL_8BIT) || defined(PSRAM_X16_MODE)
    uint8_t ret = 0;

    if (valid_l) {
        ret |= (1 << 0);
    }
    if (valid_h) {
        ret |= (1 << 1);
    }
    return ret;
#else
    return 1;
#endif
}

POSSIBLY_UNUSED static void hal_psram_calib_on_2d(uint32_t range)
{
    uint32_t val;
    uint32_t delay;
    POSSIBLY_UNUSED uint8_t tx_dqs, rx_dqs, tx_ceb, tx_clk, tx_dqs_temp;
    uint8_t inc_delay, volume;
    uint8_t cali_valid[PSRAM_WINDOW_SIZE][PSRAM_WINDOW_SIZE];
    struct HAL_PSRAM_RECTANGLE_T new_rectangle;

    while (1) {
#ifndef PSRAM_CALIB_TEST
        if (range > (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
            range = (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT);
        }
        hal_psram_phy_set_range(range);
#endif
        val = psram_phy->REG_060;
        delay = GET_BITFIELD(val, PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN);
        if (val & PSRAM_ULP_PHY_PSRAM_DLL_ALL_ONE) {
            PSRAM_TRACE(2, "%s: range=%d all one", __func__, range);
#ifndef PSRAM_CALIB_TEST
            if (range < (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
                range++;
                continue;
            }
#endif
        }
        PSRAM_TRACE(4, "%s, range=%d T/4=0x%x (REG=0x%x)", __func__, range, delay / 2, val);
        if (delay > (PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT)) {
            PSRAM_TRACE(2, "%s: range=%d bad delay (T/2 > max_tx_dqs_dly)", __func__, range);
#ifndef PSRAM_CALIB_TEST
            if (range < (PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT)) {
                range++;
                continue;
            }
#endif
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
    tx_ceb = delay / 2;
    tx_clk = delay / 2;
    PSRAM_TRACE(2, "tx_ceb:0x%x, tx_clk:0x%x", tx_ceb, tx_clk);
    volume = MIN(delay, (PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT)) / inc_delay;
    PSRAM_TRACE(2, "volume:%d, inc_delay:%d", volume, inc_delay);
    //calibrate tx_dqs and rx_dqs
    memset(cali_valid, 0, sizeof(cali_valid));
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
#ifdef PSRAM_XCCELA_MODE
            tx_dqs_temp = tx_dqs * inc_delay;
            psram_phy->REG_058 = PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(tx_dqs_temp > PSRAM_TX_CEB_DLY_OFFSET ? tx_dqs_temp - PSRAM_TX_CEB_DLY_OFFSET : 0) |
                                 PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(tx_dqs_temp) |
                                 PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY(0) |
                                 PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY(rx_dqs * inc_delay);
#else
            psram_phy->REG_058 = PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(tx_ceb) |
                                 PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(tx_clk) |
                                 PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY(tx_dqs * inc_delay) |
                                 PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY(rx_dqs * inc_delay);
#endif
            psram_phy->REG_05C = psram_phy->REG_058;
            __DSB();
            cali_valid[tx_dqs][rx_dqs] = psramphy_check_write_valid();
            //PSRAM_TRACE(0, "tx_dqs:0x%x, rx_dqs:0x%x, valid:%d", tx_dqs * inc_delay, rx_dqs * inc_delay, cali_valid[tx_dqs][rx_dqs]);
        }
    }

#if defined(PSRAM_DUAL_8BIT) || defined(PSRAM_X16_MODE)
    static const int calib_num = 2;
#else
    static const int calib_num = 1;
#endif
    volatile uint32_t *phy_delay_reg;
    uint32_t byte_pos;
    for (byte_pos = 0; byte_pos < calib_num; byte_pos++) {
        if (byte_pos == 0) {
            phy_delay_reg = &psram_phy->REG_058;
        } else {
            phy_delay_reg = &psram_phy->REG_05C;
        }
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
        find_the_largest_window_of_array(cali_valid, volume + 1, byte_pos, &new_rectangle);

#ifdef PSRAM_WINDOW_TEST
        ASSERT((volume + 1) <= PSRAM_WINDOW_SIZE ,"%s : volume+1 is larger than PSRAM_WINDOW_SIZE ,you may increase macro PSRAM_WINDOW_SIZE", __func__);
        if (hal_psram_window_2d_cb_func != NULL)
            hal_psram_window_2d_cb_func(cali_valid, volume + 1, inc_delay, byte_pos, &new_rectangle);
        PSRAM_TRACE_LINEFEED();
#endif
        ASSERT((MIN(new_rectangle.y_end - new_rectangle.y_start, new_rectangle.x_end - new_rectangle.x_start)) > 1,
                "%s : dqs window is too small to ensure data access accurately ", __func__);

        rx_dqs = ((new_rectangle.x_end + new_rectangle.x_start) * inc_delay) >> 1;
        tx_dqs = ((new_rectangle.y_end + new_rectangle.y_start) * inc_delay) >> 1;
        PSRAM_TRACE_NOTS(2,"rx_dqs: %d , tx_dqs: %d ",rx_dqs, tx_dqs);

#ifdef PSRAM_XCCELA_MODE
        *phy_delay_reg = PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(tx_dqs > PSRAM_TX_CEB_DLY_OFFSET ? tx_dqs - PSRAM_TX_CEB_DLY_OFFSET : 0) |
                         PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(tx_dqs) |
                         PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY(0) |
                         PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY(rx_dqs);
#else
        *phy_delay_reg = PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(tx_ceb) |
                         PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(tx_clk) |
                         PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY(tx_dqs) |
                         PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY(rx_dqs);
#endif
    }
}
#endif /* PSRAM_CALIB_ON_1D*/

void hal_psram_calib(uint32_t clk)
{
    uint32_t range;

    PSRAM_TRACE(2, "%s: speed = %d", __func__, clk);
    range = hal_psram_phy_get_init_range(clk);
#ifdef PSRAM_CALIB_TEST
    hal_psram_phy_set_range(range);
_retry:
    *(volatile uint32_t *)(PSRAM_CALIB_TEST_CTRL) = 1;
#endif

#ifdef PSRAM_CALIB_ON_1D
#ifdef PSRAM_XCCELA_MODE
    #error "xccela mode not support calibrate on 1D"
#endif
    hal_psram_calib_on_1d(range);
#else
    hal_psram_calib_on_2d(range);

#endif

#ifdef PSRAM_CALIB_TEST
    while (*(volatile uint32_t *)(PSRAM_CALIB_TEST_CTRL));
    PSRAM_TRACE(0, "retry calib");
    goto _retry;
#endif
    hal_cmu_psram_dll_reset_set();
}

void hal_psram_init(void)
{
    pmu_psram_freq_config(psram_run_clk);

    hal_cache_wrap_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_wrap_enable(HAL_CACHE_ID_D_CACHE);

    psram_mc_set_freq(psram_cfg_clk);

    hal_cmu_psram_clock_enable();

#ifndef FPGA
    hal_psram_phy_init(psram_cfg_clk);
    hal_sys_timer_delay_us(100);
#endif
    hal_psram_digphy_init();
    hal_psram_mc_init(psram_cfg_clk);
    hal_psram_init_calib();
    psram_reset();
    hal_psram_snoop_disable(HAL_PSRAM_ID_0);

#if !defined(FPGA) && !defined(SIMU) && defined(PSRAM_CALIB_ON_1D)
    psram_init_pattern_array();
    psram_set_timing(psram_cfg_clk);
    hal_psram_mc_set_timing(psram_cfg_clk, false);
    ASSERT(!psram_pattern_init_check(), "%s:psram write and read fail psram_cfg_clk = %u", __func__, psram_cfg_clk);
#endif

    psram_chip_timing_config(psram_run_clk, true);

#if !defined(FPGA) && !defined(SIMU)
    hal_psram_calib(psram_run_clk);
#endif

    hal_psram_snoop_enable(HAL_PSRAM_ID_0);

#ifdef PSRAM_AUTO_LOW_POWER
    hal_psram_auto_low_power_enable();
#endif
}

#endif // PSRAM_CTRL_BASE

#endif
