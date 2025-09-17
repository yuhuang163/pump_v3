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
#include "hal_timer.h"
#include "pmu.h"
#include "reg_psram_mc_v6.h"
#include "reg_psram_phy_v6.h"
#include "string.h"

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_drv.h"
#include "psram_timing.h"
#include "psram_debug.h"
#include "psram_common.h"

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
#define MR4_RF_RATE_SHIFT                   (3)
#define MR4_RF_RATE_MASK                    (0x3 << MR4_RF_RATE_SHIFT)
#define MR4_RF_RATE(n)                      BITFIELD_VAL(MR4_RF_RATE, n)
#define MR4_WRITE_LATENCY_SHIFT             5
#define MR4_WRITE_LATENCY_MASK              (0x7 << MR4_WRITE_LATENCY_SHIFT)
#define MR4_WRITE_LATENCY(n)                BITFIELD_VAL(MR4_WRITE_LATENCY, n)

#define MR4_RF_RATE_HALF                    (3)
#define MR4_RF_RATE_1X                      (1)
#define MR4_RF_RATE_4X                      (0)

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


void SRAM_TEXT_LOC psram_xopi_struct_init (struct PSRAM_MC_CFG_T *mc_cfg, struct PSRAM_PHY_CFG_T *phy_cfg, struct PSRAM_CFG_T *psram_cfg)
{
    psram_cfg->mc_cfg = mc_cfg;
    psram_cfg->phy_cfg = phy_cfg;
    psram_cfg->device_timing = &xopi_device_timing;
    psram_cfg->cmd = &xopi_cmd_table;

    mc_cfg->chip_ca_map = 2;
    mc_cfg->chip_cmd_4bit = 0;

    mc_cfg->prefetch_width = PSRAM_PREFETCH_WIDTH_NULL;
    mc_cfg->burst_length = 3;
    mc_cfg->page_boundary = 1;
    mc_cfg->cp_wrap_en = 0;
    mc_cfg->auto_precharge = 0;
    mc_cfg->wb_bypass = 0;
    mc_cfg->wr_dummy_cyc = 0;
    mc_cfg->cmd_dummy_cyc = 4;
    mc_cfg->stop_clk_in_nop =1;
    mc_cfg->stop_clk_in_tcph = 1;

    phy_cfg->phy_pd_trans_mask = 0;
    phy_cfg->ca_pattern = 7;
    phy_cfg->hyperbus_sample_lat_edge_sel = 0;
    phy_cfg->hyperbus_cmd_dly_cnt = 0;
    phy_cfg->reg_ca_dq_lat_sw_en = 0;
    phy_cfg->man_tx_wr_push_en = 0;
    phy_cfg->auto_pwr_tx_wr_push_en = 0;
    phy_cfg->clk_preample_en = 0;
    phy_cfg->ca_preample_en = 0;
    phy_cfg->hyperbus_tcss = 0;
    phy_cfg->phy_dummy_cyc_en = 0;
    phy_cfg->phy_dummy_cyc_stop_dqs = 0;
    phy_cfg->idle_dq_oen = 1;
    phy_cfg->idle_dqs_oen = 1;
    phy_cfg->dqs_dm_merge_en = 1;
    phy_cfg->mrw_output_dm = 0;
    phy_cfg->cs_pol = 0;
}

/// TODO: Use macro to define MR value
void psram_xopi_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    if ((psram_cfg->psram_data_bus == PSRAM_DATA_BUS_X16) || psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16) {
        hal_psram_device_mr_write(id, 8, 0x43);
    } else {
        hal_psram_device_mr_write(id, 8, 0x03);
    }
    /// MRS  mr8,   0,   4
    //       0x43  0x24 0x80  100MHz
    //       0x43  0x28 0x40  133MHz
    //       0x43  0x30 0x20  200MHz
    if (psram_cfg->run_freq_mhz <=33) {
        hal_psram_device_mr_write(id, 0, 0x08);
        hal_psram_device_mr_write(id, 4, 0x38);
    } else if (psram_cfg->run_freq_mhz <=66) {
        hal_psram_device_mr_write(id, 0, 0x0c);
        hal_psram_device_mr_write(id, 4, 0xb8);
    } else if (psram_cfg->run_freq_mhz <=100) {
        hal_psram_device_mr_write(id, 0, 0x10);
        hal_psram_device_mr_write(id, 4, 0x78);
    } else if  (psram_cfg->run_freq_mhz <=133){
        hal_psram_device_mr_write(id, 0, 0x14);
        hal_psram_device_mr_write(id, 4, 0xf8);
    } else {//if (psram_cfg->run_freq_mhz <=200)
        hal_psram_device_mr_write(id, 0, 0x1c);
        hal_psram_device_mr_write(id, 4, 0x98);
    }
}

void SRAM_TEXT_LOC psram_xopi_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk)
{
    if ((psram_cfg->psram_data_bus == PSRAM_DATA_BUS_X16) || psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16) {
        *reg = 8;
        *val = 0x43;
    } else {
        *reg = 8;
        *val = 0x03;
    }
    /// MRS  mr8,   0,   4
    //       0x43  0x10 0x78  100MHz
    //       0x43  0x14 0xf8  133MHz
    //       0x43  0x1c 0x98  200MHz
    if (clk <=33) {
        *(reg + 1) = 0;
        *(val + 1) = 0x08;
        *(reg + 2) = 4;
        *(val + 2) = 0x38;
    } else if (clk <=66) {
        *(reg + 1) = 0;
        *(val + 1) = 0x0c;
        *(reg + 2) = 4;
        *(val + 2) = 0xb8;
    } else if (clk <=100) {
        *(reg + 1) = 0;
        *(val + 1) = 0x10;
        *(reg + 2) = 4;
        *(val + 2) = 0x78;
    } else if  (clk <=133){
        *(reg + 1) = 0;
        *(val + 1) = 0x14;
        *(reg + 2) = 4;
        *(val + 2) = 0xf8;
    } else {//if (clk <=200)
        *(reg + 1) = 0;
        *(val + 1) = 0x1c;
        *(reg + 2) = 4;
        *(val + 2) = 0x98;
    }
}

void psram_common_struct_init(struct PSRAM_MC_CFG_T *mc_cfg, struct PSRAM_PHY_CFG_T *phy_cfg, struct PSRAM_CFG_T *psram_cfg)
{

}

void psram_xopi_set_dly(enum HAL_PSRAMPHY_ID_T phy_id, enum HAL_PSRAM_PHY_CHANNEL_T c, struct HLA_PSRAM_DLL_DLY_VAL_T *v)
{
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_TX_CEB, v->tx_ceb);
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_TX_CLK, v->tx_clk);
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_TX_DQS, 0);
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_RX_DQS, v->rx_dqs);
}

void psram_xopi_calib(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c, uint32_t clk)
{
    uint16_t dly_in, dly_inc;
    uint16_t volume;
    uint16_t tx_ceb, tx_clk, rx_dqs;
    bool valid_square[PSRAM_WINDOW_SIZE][PSRAM_WINDOW_SIZE];
    struct HAL_PSRAM_RECTANGLE_T new_rectangle;

    dly_in = hal_psram_phy_dll_range_dyn_config(clk);

    dly_inc = dly_in / 8;

    if (0) {
#ifdef PSRAM_WINDOW_TEST
    } else if (1) {
        dly_inc = 1;
#endif
    } else if (dly_inc == 0) {
        dly_inc = 1;
    }

    volume = dly_in / dly_inc;
    PSRAM_TRACE(0, "*** dly_in = 0x%x, dly_inc = %x, volume = %d", dly_in, dly_inc, volume);
    memset(valid_square, 0, sizeof(valid_square));

    /// TODO: why fixed "0"(first 0)
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_DQS, 0);
    for(tx_clk = 0; tx_clk <= dly_in; tx_clk += dly_inc) {
        for(rx_dqs = 0; rx_dqs <= dly_in; rx_dqs += dly_inc) {
            tx_ceb = tx_clk > 4 ? tx_clk - 4 : tx_clk;
            hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CEB, tx_ceb);
            hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CLK, tx_clk);
            hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_RX_DQS, rx_dqs);
            __DSB();
            valid_square[tx_clk / dly_inc][rx_dqs / dly_inc] = psram_check_wr_valid(id, c);
        }
    }

    PSRAM_TRACE_LINEFEED();
    for (int i = 0; i <= volume; i++){
        for (int j = 0; j <= volume; j++){
            PSRAM_TRACENOCRLF_NOTS(0, "  %d",valid_square[i][j]);
        }
        PSRAM_TRACE_LINEFEED();
    }
    PSRAM_TRACE_LINEFEED();

    find_the_largest_window_of_2d_array(valid_square, volume + 1, 0, &new_rectangle);
    if ((MIN(new_rectangle.y_end - new_rectangle.y_start, new_rectangle.x_end - new_rectangle.x_start)) < 2) {
        PSRAM_ASSERT(0, "%s : dqs window is too small to ensure data access accurately ", __func__);
    }

#ifdef PSRAM_WINDOW_TEST
        ASSERT((volume + 1) <= PSRAM_WINDOW_SIZE ,"%s : volume+1 is larger than PSRAM_WINDOW_SIZE ,you may increase macro PSRAM_WINDOW_SIZE", __func__);
        psram_window_2d_cb_exec(valid_square, volume + 1, dly_inc, 0, &new_rectangle);
        PSRAM_TRACE_LINEFEED();
#endif

    rx_dqs = ((new_rectangle.x_end + new_rectangle.x_start) * dly_inc) >> 1;
    tx_clk = ((new_rectangle.y_end + new_rectangle.y_start) * dly_inc) >> 1;
    PSRAM_TRACE(2,"rx_dqs: %d , tx_clk: %d ",rx_dqs, tx_clk);

    tx_ceb = tx_clk >4 ? tx_clk - 4 : tx_clk;
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CEB, tx_ceb);
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CLK, tx_clk);
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_RX_DQS, rx_dqs);

    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_TX_DQS, 0);
    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_TX_CEB,  tx_ceb);
    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_TX_CLK,  tx_clk);
    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_RX_DQS,  rx_dqs);
}

#endif
