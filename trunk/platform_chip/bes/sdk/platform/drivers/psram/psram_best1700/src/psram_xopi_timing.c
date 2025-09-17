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

/// APS256XXN:
const BOOT_RODATA_FLASH_LOC struct PSRAM_AC_TIMING_T xopi_ac_timing[] ={
    {.freq_mhz = 24,
    .wl = 7 + 2,
    .rl_min = 6 + 2,
    .rl_max = 11 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 19.5),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 19.5),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 4000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 100),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 200000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_24MHZ, 7.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 19.5),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 19.5),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_precharge = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 4000),
    .t_dvdd = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 500),
    .t_avddq = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 3500),
    .t_lock = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 3200),
    }, {
    .freq_mhz = 48,
    .wl = 8 + 2,
    .rl_min = 7 + 2,
    .rl_max = 13 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 19.5),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 19.5),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 4000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 200000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_48MHZ, 7.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 19.5),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 19.5),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_precharge = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 4000),
    .t_dvdd = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 500),
    .t_avddq = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 3500),
    .t_lock = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 3200),
    }, {
    .freq_mhz = 100,
    .wl = 9 + 2,
    .rl_min = 8 + 2,
    .rl_max = 15 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 19.5),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 19.5),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 4000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 200000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_100MHZ, 7.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 19.5),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 19.5),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_precharge = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 4000),
    .t_dvdd = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 500),
    .t_avddq = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 3500),
    .t_lock = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 3200),
    }, {
    .freq_mhz = 133,
    .wl = 10 + 2,
    .rl_min = 9 + 2,
    .rl_max = 17 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 19.5),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 19.5),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 4000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 200000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_133MHZ, 7.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 19.5),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 19.5),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_precharge = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 4000),
    .t_dvdd = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 500),
    .t_avddq = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 3500),
    .t_lock = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 3200),
    }, {
    .freq_mhz = 200,
    .wl = 12 + 2,
    .rl_min = 11 + 2,
    .rl_max = 21 + 2,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 26),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 26),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 4000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 200000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_200MHZ, 7.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 26),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 26),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_precharge = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 4000),
    .t_dvdd = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 500),
    .t_avddq = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 3500),
    .t_lock = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 800),
    },
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_TIMING_T xopi_device_timing = {
    //default freq: 133MHz
    .wl_def = 10 + 2,
    .rl_min_def = 9 + 2,
    .rl_max_def = 17 + 2,
    .is_mrs_wl_fixed = true,
    .fixed_wl_mrs = 1 + 2,
    .bl_mrs = 4,  // ?
    .t_xphs_min_ns = 60,
    .t_dqsck_max_ns = 6.5,

    .mrs_as_pd_en = 1,
    .mrs_as_pd_mr = 6,
    .mrs_as_pd_md = 0xf0,

    .ac_timing = xopi_ac_timing,
    .ac_timing_count = ARRAY_SIZE(xopi_ac_timing)
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_CMD_TABLE_T xopi_cmd_table = {
    .cmd_array_rd = 0x00,
    .cmd_array_wr = 0x80,
    .cmd_reg_rd = 0x40,
    .cmd_reg_wr = 0xC0,
    .cmd_glbrst = 0xFF,
    .cmd_nop = 0,
    .cmd_self_refr = 0
};

#endif
