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
#include "hal_trace.h"
#include "pmu.h"
#include "reg_psram_mc_v6.h"
#include "reg_psram_phy_v6.h"
#include "string.h"

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_timing.h"
#include "psram_debug.h"
#include "psram_common.h"

void psram_opi_struct_init (struct PSRAM_MC_CFG_T *mc_cfg, struct PSRAM_PHY_CFG_T *phy_cfg, struct PSRAM_CFG_T *psram_cfg)
{
    psram_cfg->mc_cfg = mc_cfg;
    psram_cfg->phy_cfg = phy_cfg;
    psram_cfg->device_timing = &opi_device_timing;
    psram_cfg->cmd = &opi_cmd_table;

    mc_cfg->chip_ca_map = 1;     // enum PSRAM_CHIP_CA_MAP_t;
    mc_cfg->chip_cmd_4bit = 0;   // enum PSRAM_CHIP_CMD_BITS_T;

    mc_cfg->prefetch_width = PSRAM_PREFETCH_WIDTH_NULL;    // enum PSRAM_PREFETCH_WIDTH_T; 16n 32n
    mc_cfg->burst_length = 3;  // enum PSRAM_BL_T
    mc_cfg->page_boundary = 0; // enum PSRAM_PAGE_BOUNADRY_T
    mc_cfg->cp_wrap_en = 0;
    mc_cfg->auto_precharge = 0;
    mc_cfg->wb_bypass = 0;
    mc_cfg->wr_dummy_cyc = 1;
    mc_cfg->cmd_dummy_cyc = 4;
    mc_cfg->stop_clk_in_nop =1;
    mc_cfg->stop_clk_in_tcph = 0;

    phy_cfg->phy_pd_trans_mask = 0;

    phy_cfg->ca_pattern = 4;      // enum PSRAM_CA_PATTERN_T
    phy_cfg->hyperbus_sample_lat_edge_sel = 0;
    phy_cfg->hyperbus_cmd_dly_cnt = 0;
    phy_cfg->reg_ca_dq_lat_sw_en = 0;
    phy_cfg->man_tx_wr_push_en = 0;
    phy_cfg->auto_pwr_tx_wr_push_en = 0;
    phy_cfg->clk_preample_en = 0;
    phy_cfg->ca_preample_en = 0;
    phy_cfg->hyperbus_tcss = 1;
    phy_cfg->phy_dummy_cyc_en = 1;   // ?
    phy_cfg->phy_dummy_cyc_stop_dqs = 0;
    phy_cfg->idle_dq_oen = 1;
    phy_cfg->idle_dqs_oen = 1;
    phy_cfg->dqs_dm_merge_en = 0;
    phy_cfg->mrw_output_dm = 0;
    phy_cfg->cs_pol = 0;  // enum PSRAM_CA_POL_T
}

///TODO: change via MARCO or struct
void psram_opi_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    /// MRS  mr8,   0,   4
        //   0x03  0x2f 0x00  100MHz
        //   0x03  0x33 0x00  133MHz
        //   0x03  0x3b 0x80  200MHz
    if (psram_cfg->run_freq_mhz <= 50) {
        hal_psram_device_mr_write(id, 8, 0x03);
        hal_psram_device_mr_write(id, 8, 0x03);
        hal_psram_device_mr_write(id, 0, 0x2b);
        hal_psram_device_mr_write(id, 4, 0x00);
    } else if (psram_cfg->run_freq_mhz <= 100) {
        hal_psram_device_mr_write(id, 8, 0x03);
        hal_psram_device_mr_write(id, 8, 0x03);
        hal_psram_device_mr_write(id, 0, 0x2f);
        hal_psram_device_mr_write(id, 4, 0x00);
    } else  {//if (psram_cfg->run_freq_mhz <=100) {
        hal_psram_device_mr_write(id, 8, 0x03);
        hal_psram_device_mr_write(id, 8, 0x03);
        hal_psram_device_mr_write(id, 0, 0x3b);
        hal_psram_device_mr_write(id, 4, 0x80);
    }
}

void SRAM_TEXT_LOC psram_opi_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk)
{
    *reg = 8;
    *val = 0x03;

    if (clk <= 50) {
        *(reg + 1) = 0;
        *(val + 1) = 0x2b;
        *(reg + 2) = 4;
        *(val + 2) = 0x00;
    } else if (clk <= 100) {
        *(reg + 1) = 0;
        *(val + 1) = 0x2f;
        *(reg + 2) = 4;
        *(val + 2) = 0x00;
    } else  {//if (clk <=100) {
        *(reg + 1) = 0;
        *(val + 1) = 0x3b;
        *(reg + 2) = 4;
        *(val + 2) = 0x80;
    }
}

void psram_opi_set_dly(enum HAL_PSRAMPHY_ID_T phy_id, enum HAL_PSRAM_PHY_CHANNEL_T c, struct HLA_PSRAM_DLL_DLY_VAL_T *v)
{
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_TX_CEB, v->tx_ceb);
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_TX_CLK, v->tx_clk);
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_TX_DQS, v->tx_dqs);
    hal_psram_anaphy_set_dly(phy_id, c, HAL_PSRAM_DLL_DLY_RX_DQS, v->rx_dqs);
}

POSSIBLY_UNUSED static void psram_opi_calib_on_2d(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c, uint32_t clk)
{
    uint16_t dly_in, dly_inc;
    uint16_t volume;

    uint16_t tx_ceb, tx_dqs, tx_clk, rx_dqs;
    bool valid_square[PSRAM_WINDOW_SIZE][PSRAM_WINDOW_SIZE];
    struct HAL_PSRAM_RECTANGLE_T new_rectangle;

    dly_in = hal_psram_phy_dll_range_dyn_config(clk);

    dly_inc = dly_in / 8;
    if (dly_inc == 0) {
        dly_inc = 1;
    }

    volume = dly_in / dly_inc;
    PSRAM_TRACE(0, "*** dly_in = 0x%x, dly_inc = %x, volume = %d",dly_in, dly_inc, volume);
    memset(valid_square, 0, sizeof(valid_square));

    tx_clk = dly_in / 4;
    tx_ceb = dly_in / 4;

    /// TODO: why fixed "0"(first 0)
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CLK, tx_clk);
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CEB, tx_ceb);
    for(tx_dqs = 0; tx_dqs <= dly_in; tx_dqs += dly_inc) {
        for(rx_dqs = 0; rx_dqs <= dly_in; rx_dqs += dly_inc) {
            hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_DQS, tx_dqs);
            hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_RX_DQS, rx_dqs);
            __DSB();
            valid_square[tx_dqs / dly_inc][rx_dqs / dly_inc] = psram_check_wr_valid(id, c);
        }
    }

    PSRAM_TRACE_LINEFEED();
    for (int i = 0; i <=volume; i++){
        for (int j = 0; j<=volume; j++){
            PSRAM_TRACENOCRLF_NOTS(0, "  %d",valid_square[i][j]);
        }
        PSRAM_TRACE_LINEFEED();
    }
    PSRAM_TRACE_LINEFEED();

    find_the_largest_window_of_2d_array(valid_square, volume + 1, 0, &new_rectangle);
    if ((MIN(new_rectangle.y_end - new_rectangle.y_start, new_rectangle.x_end - new_rectangle.x_start)) < 2) {
        PSRAM_ASSERT(0, "%s : dqs window is too small to ensure data access accurately ", __func__);
    }
    rx_dqs = ((new_rectangle.x_end + new_rectangle.x_start) * dly_inc) >> 1;
    tx_dqs = ((new_rectangle.y_end + new_rectangle.y_start) * dly_inc) >> 1;
    PSRAM_TRACE(2,"rx_dqs: %d , tx_dqs: %d , tx_clk: %d, tx_ceb: %d",rx_dqs, tx_dqs, tx_clk, tx_ceb);

    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_DQS, tx_dqs);
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_RX_DQS, rx_dqs);

    //set frequency point 1 anaphy dly
    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_TX_CEB,  tx_ceb);
    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_TX_CLK,  tx_clk);
    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_TX_DQS,  tx_dqs);
    hal_psram_anaphy_set_dly_1(0, c, HAL_PSRAM_DLL_DLY_RX_DQS,  rx_dqs);
}

POSSIBLY_UNUSED static void psram_opi_calib_on_1d(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c, uint32_t clk)
{
    uint16_t dly_in, dly_inc;
    uint16_t volume;
    uint8_t windows_l = 0, windows_r = 0;
    uint32_t calib_val;

    POSSIBLY_UNUSED uint16_t tx_ceb, tx_dqs, tx_clk, rx_dqs;
    bool valid_square[PSRAM_WINDOW_SIZE];
    uint8_t valid_cnt[PSRAM_WINDOW_SIZE];

    dly_in = hal_psram_phy_dll_range_dyn_config(clk);

    dly_inc = dly_in / 15;
    if (dly_inc == 0) {
        dly_inc = 1;
    }

    volume = dly_in / dly_inc;
    PSRAM_TRACE(0, "*** dly_in = 0x%x, dly_inc = %x, volume = %d",dly_in, dly_inc, volume);
    memset(valid_square, 0, sizeof(valid_square));
    memset(valid_cnt, 0, sizeof(valid_cnt));

    tx_clk = dly_in / 4;
    tx_ceb = dly_in / 4;
    tx_dqs = dly_in / 2;

    /// TODO: why fixed "0"(first 0)
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CLK, tx_clk);
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_CEB, tx_ceb);
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_TX_DQS, tx_dqs);

    for(rx_dqs = 0; rx_dqs <= dly_in; rx_dqs += dly_inc) {
        hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_RX_DQS, rx_dqs);
        __DSB();
        valid_square[rx_dqs / dly_inc] = psram_check_wr_valid(id, c);
    }

    PSRAM_TRACE_LINEFEED();
    for (int i = 0; i <=volume; i++){
        PSRAM_TRACENOCRLF_NOTS(0, "  %d",valid_square[i]);
    }
    PSRAM_TRACE_LINEFEED();

    //find the longest rx_dqs window
    calib_val = find_the_largest_window_of_1d_array(valid_square, volume);
    windows_l = (calib_val >> 8) & 0xFF;
    windows_r = calib_val & 0xFF;

    PSRAM_TRACE(2,"windows_l: %d , windows_r: %d , dly_inc: %d",windows_l, windows_r, dly_inc);
    if ((windows_r - windows_l) < 2) {
        PSRAM_ASSERT(0, "%s : dqs window is too small to ensure data access accurately ", __func__);
    }
    rx_dqs = ((windows_r + windows_l) * dly_inc) >> 1;

    PSRAM_TRACE(2,"rx_dqs: %d , tx_dqs: %d , tx_clk: %d, tx_ceb: %d",rx_dqs, tx_dqs, tx_clk, tx_ceb);
    hal_psram_anaphy_set_dly(0, c, HAL_PSRAM_DLL_DLY_RX_DQS, rx_dqs);
}

void psram_opi_calib(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c, uint32_t clk)
{
    // psram_opi_calib_on_2d(id, c, clk);
    psram_opi_calib_on_1d(id, c, clk);
}

#endif
