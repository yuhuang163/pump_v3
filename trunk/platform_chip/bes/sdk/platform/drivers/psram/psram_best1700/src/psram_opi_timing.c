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
#include "hal_location.h"
#include "psram_timing.h"
#include "hal_psram.h"
#include "hal_psramip.h"

// aps6408L
/// NOTE: If tRC ≥ (1/3) × tCEMmax: tCPH must also meet a minimum of 40ns for long write operations.
const BOOT_RODATA_FLASH_LOC struct PSRAM_AC_TIMING_T opi_ac_timing[] ={
    {.freq_mhz = 24,
    .wl = 0 + 2,
    .rl_min = 2 + 2,
    .rl_max = 4 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 4,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_48MHZ, 5.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    }, {
    .freq_mhz = 48,
    .wl = 0 + 2,
    .rl_min = 2 + 2,
    .rl_max = 4 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 4,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_48MHZ, 5.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    },{
    .freq_mhz = 100,
    .wl = 0 + 2,
    .rl_min = 3 + 2,
    .rl_max = 6 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 4,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_100MHZ,5.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    }, {
    .freq_mhz = 133,
    .wl = 0 + 2,
    .rl_min = 4 + 2,
    .rl_max = 8 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 4,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_133MHZ, 5.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    }, {
    .freq_mhz = 200,
    .wl = 2 + 2,
    .rl_min = 6 + 2,
    .rl_max = 12 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 6,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_200MHZ, 5.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 24),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 24),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    },
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_TIMING_T opi_device_timing = {
    //default freq: 133MHz
    .wl_def = 0 + 2,
    .rl_min_def = 4 + 2,
    .rl_max_def = 8 + 2,
    .is_mrs_wl_fixed = false,
    .fixed_wl_mrs = 0 + 2,  // not use
    // .bl_mrs = 4,  // ?
    .t_xphs_min_ns = 60,
    .t_dqsck_max_ns =5.5,

    .mrs_as_pd_en = 1,
    .mrs_as_pd_mr = 6,
    .mrs_as_pd_md = 0xf0,

    .ac_timing = opi_ac_timing,
    .ac_timing_count = ARRAY_SIZE(opi_ac_timing)
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_CMD_TABLE_T opi_cmd_table = {
    .cmd_array_rd = 0x00,
    .cmd_array_wr = 0x80,
    .cmd_reg_rd = 0x40,
    .cmd_reg_wr = 0xC0,
    .cmd_glbrst = 0xFF,
    .cmd_nop = 0,
    .cmd_self_refr = 0xD0
};

#endif
