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
#ifndef __PSRAM_CFG_H__
#define __PSRAM_CFG_H__

#include "psramphy.h"

enum PSRAM_PHY_BYPASS_CFG_T {
    PHY_ANA_BYPASS,
    PHY_P2S_BYPASS,
    PHY_NOT_BYPASS,
};

struct PSRAM_PHY_CFG_T {
    enum PSRAM_PHY_BYPASS_CFG_T phy_bypass_cf;

    bool     phy_pd_trans_mask; /// ? can keep 0?

    uint8_t  ca_pattern;    // enum PSRAM_CA_PATTERN_T
    bool     hyperbus_sample_lat_edge_sel;
    uint8_t  hyperbus_cmd_dly_cnt;
    bool     reg_ca_dq_lat_sw_en;
    bool     man_tx_wr_push_en;
    bool     auto_pwr_tx_wr_push_en;
    // bool     auto_lock_tx_wr_push_en;
    bool     clk_preample_en;
    bool     ca_preample_en;
    uint8_t  hyperbus_tcss;

    bool     phy_dummy_cyc_en;
    bool     phy_dummy_cyc_stop_dqs;
    bool     idle_dq_oen;
    bool     idle_dqs_oen;
    bool     dqs_dm_merge_en;
    bool     mrw_output_dm;   // ? opi xopi uls uhs look like donot care dm  when mrw
    bool     cs_pol; // enum PSRAM_CA_POL_T
};

struct PSRAM_MC_CFG_T {
    uint8_t  chip_ca_map;     // enum PSRAM_CHIP_CA_MAP_t;
    uint8_t  chip_cmd_4bit;   // enum PSRAM_CHIP_CMD_BITS_T;

    uint8_t  prefetch_width;    // enum PREFETCH_WIDTH_T; 16n 32n

    uint8_t  burst_length;
    uint8_t  page_boundary;

    bool     cp_wrap_en;
    bool     auto_precharge;

    bool     wb_bypass;

    uint8_t  wr_dummy_cyc;
    uint8_t  cmd_dummy_cyc;
    uint8_t  nop_dummy_cyc;

    bool     stop_clk_in_nop;
    bool     stop_clk_in_tcph;
};

struct PSRAM_CFG_T {
    uint16_t run_freq_mhz;
    uint16_t cfg_freq_mhz;
    uint8_t  psram_type;
    uint8_t  psram_data_bus;
    enum PSRAM_PHY_BYPASS_CFG_T phy_bypass;
    struct PSRAM_PHY_CFG_T *phy_cfg;
    struct PSRAM_MC_CFG_T  *mc_cfg;
    const struct PSRAM_DEVICE_CMD_TABLE_T *cmd;
    const struct PSRAM_DEVICE_TIMING_T *device_timing;
};

#endif /* __PSRAM_CFG_H__ */
