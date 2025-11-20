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
#ifdef PSRAM_CTRL_ENABLE

#include "plat_types.h"
#include "hal_location.h"
#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_debug.h"
#include "hal_timer.h"
#include "psram_drv.h"

const BOOT_RODATA_FLASH_LOC struct PSRAM_AC_TIMING_T hyper_ac_timing[] ={
    {.freq_mhz = 24,
    .wl = 3,
    .rl_min = 3,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 28),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 28),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 4000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 3000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 100000),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 10),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 400),
    .t_wr_dmy = 0,
    }, {
    .freq_mhz = 48,
    .wl = 3,
    .rl_min = 3,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 28),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 28),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 4000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 3000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 100000),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 10),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 400),
    .t_wr_dmy = 0,
    }, {
    .freq_mhz = 100,
    .wl = 4,
    .rl_min = 4,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 28),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 28),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 4000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 3000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 100000),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 10),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 400),
    .t_wr_dmy = 0,
    }, {
    .freq_mhz = 133,
    .wl = 5,
    .rl_min = 5,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 28),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 28),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 4000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 3000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 100000),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 10),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 400),
    .t_wr_dmy = 0,
    }, {
    .freq_mhz = 200,
    .wl = 7,
    .rl_min = 7,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 28),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 28),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 4000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 3000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 100000),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 10),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 400),
    .t_wr_dmy = 0,
    },
};

const SRAM_RODATA_LOC struct PSRAM_DEVICE_TIMING_T hyper_device_timing = {
    .wl_def = 7,
    .rl_min_def = 7,
    .is_mrs_wl_fixed = 1,
    .fixed_wl_mrs = 1,

    .mrs_as_pd_en = 1,
    .mrs_as_pd_mr = 2,
    .mrs_as_pd_md = 0xe1ff,

    .ac_timing = hyper_ac_timing,
    .ac_timing_count = ARRAY_SIZE(hyper_ac_timing)
};

void psram_hyper_w956_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t reg;
    uint32_t val;

    reg = 0x00;
    if (psram_cfg->run_freq_mhz <= 85) {
            val = 0xe;
        } else if (psram_cfg->run_freq_mhz <= 104) {
            val = 0xf;
        } else if (psram_cfg->run_freq_mhz <= 133) {
            val = 0x0;
        } else if (psram_cfg->run_freq_mhz <= 166) {
            val = 0x1;
        } else  {
            val = 0x2;
        }
    val = (val << 4) | (0x8f07);

    hal_psram_mc_var_lat_en(id, false);
    hal_psram_device_mr_write(id, reg, PSRAM_SWAP8BITS(val));
    hal_psram_mc_var_lat_en(id, true);
}

static int psram_xopi_w956_check(enum HAL_PSRAM_ID_T id)
{
    uint32_t md;
    int ret = 0;

    md = hal_psram_device_mr_read(id, 0) & 0xffff;
    md = PSRAM_SWAP8BITS(md);
    PSRAM_TRACE(0, "****Configuration register 0 = 0x%x", md);

    if (md == 0x8f2f) {
        ret = 0;
    } else if (md == 0x11fa) {
        ret = 0xfa11;
    } else {
        ret = 1;
    }

    return ret;
}

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_CMD_TABLE_T hyper_cmd_table = {
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_CFG_T hyper_device_cfg = {
    .device_cmd_4bit = 1,
    .wr_dummy_cyc = 0,
    .stop_clk_in_nop =1,
    .stop_clk_in_tcph = 1,
    .dqs_dm_merge_en = 1,
    .mrw_output_dm = 0,
    .addr_4bytes_en = 1,
    .page_boundary = 0,
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_FUNC_CFG_T hyper_device_func_cfg = {
    .psram_set_mr = psram_hyper_w956_mr_set,
    .psram_get_mr = NULL,
    .psram_show_mr = NULL,
    .psram_check_die = psram_xopi_w956_check,
};

#endif
