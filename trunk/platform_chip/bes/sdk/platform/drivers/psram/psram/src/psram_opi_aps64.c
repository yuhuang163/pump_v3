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

#define PSRAM_66MHz_MR0                     MR0_DRIVE_STR(3) | MR0_READ_LATENCY(2)
#define PSRAM_109MHz_MR0                    MR0_DRIVE_STR(3) | MR0_READ_LATENCY(3)
#define PSRAM_133MHz_MR0                    MR0_DRIVE_STR(3) | MR0_READ_LATENCY(4)
#define PSRAM_166MHz_MR0                    MR0_DRIVE_STR(3) | MR0_READ_LATENCY(5)
#define PSRAM_200MHz_MR0                    MR0_DRIVE_STR(3) | MR0_READ_LATENCY(6)

#define PSRAM_166MHz_MR4                    (MR4_PASR(0) | MR4_RF_RATE(MR4_RF_RATE_HALF) | MR4_WRITE_LATENCY(0))
#define PSRAM_200MHz_MR4                    (MR4_PASR(0) | MR4_RF_RATE(MR4_RF_RATE_HALF) | MR4_WRITE_LATENCY(4))

#define PSRAM_MR8                           MR8_BL(3)

static void psram_opi_aps64_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    hal_psram_device_mr_write(id, 8, PSRAM_MR8);

    if (psram_cfg->run_freq_mhz <=66) {
        hal_psram_device_mr_write(id, 0, PSRAM_66MHz_MR0);
    } else if (psram_cfg->run_freq_mhz <=109) {
        hal_psram_device_mr_write(id, 0, PSRAM_109MHz_MR0);
    } else if (psram_cfg->run_freq_mhz <=133) {
        hal_psram_device_mr_write(id, 0, PSRAM_133MHz_MR0);
    } else if  (psram_cfg->run_freq_mhz <=166){
        hal_psram_device_mr_write(id, 0, PSRAM_166MHz_MR0);
    } else {//if (psram_cfg->run_freq_mhz <=200)
        hal_psram_device_mr_write(id, 0, PSRAM_200MHz_MR0);
    }

    if (psram_cfg->run_freq_mhz <=166) {
        hal_psram_device_mr_write(id, 4, PSRAM_166MHz_MR4);
    } else {//if (psram_cfg->run_freq_mhz <=200)
        hal_psram_device_mr_write(id, 4, PSRAM_200MHz_MR4);
    }
}

static void psram_opi_aps64_mr_show(enum HAL_PSRAM_ID_T id)
{
    uint32_t md;

    PSRAM_TRACE(0, "Psram%d MR as follow", id);
    md = hal_psram_device_mr_read(id, 8) & 0xff;
    PSRAM_TRACE(0, "****MR8 = 0x%x", md);
    md = hal_psram_device_mr_read(id, 0) & 0xff;
    PSRAM_TRACE(0, "****MR0 = 0x%x", md);
    md = hal_psram_device_mr_read(id, 4) & 0xff;
    PSRAM_TRACE(0, "****MR4 = 0x%x", md);
}

static int psram_opi_aps64_check(enum HAL_PSRAM_ID_T id)
{
    uint32_t md;
    int ret = 0;

    md = hal_psram_device_mr_read(id, 0);
    PSRAM_TRACE(0, "****MR0 = 0x%x", md);

    if ((md & 0xff) == 0x13) {
        ret = 0;
    } else if ((md & 0xffff) == 0xfa11) {
        ret = 0xfa11;
    } else {
        ret = 1;
    }

    return ret;
}

const BOOT_RODATA_FLASH_LOC struct PSRAM_AC_TIMING_T opi_ac_timing[] ={
    {.freq_mhz = 24,
    .wl = 0,
    .rl_min = 2,
    .rl_max = 4,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 6,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 2500),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 10000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 100),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 150000),
    .t_mrr = 2,
    .t_mrs = 2,
    .t_wr_dmy = 1,
    }, {
    .freq_mhz = 48,
    .wl = 0,
    .rl_min = 2,
    .rl_max = 4,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 6,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2500),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 10000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 100),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_mrr = 2,
    .t_mrs = 2,
    .t_wr_dmy = 1,
    }, {
    .freq_mhz = 100,
    .wl = 0,
    .rl_min = 3,
    .rl_max = 6 ,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 6,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 2500),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 10000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 100),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 150000),
    .t_mrr = 2,
    .t_mrs = 2,
    .t_wr_dmy = 1,
    }, {
    .freq_mhz = 133,
    .wl = 0,
    .rl_min = 4,
    .rl_max = 8,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 6,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 2500),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 10000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 100),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 150000),
    .t_mrr = 2,
    .t_mrs = 2,
    .t_wr_dmy = 1,
    }, {
    .freq_mhz = 200,
    .wl = 2,
    .rl_min = 6,
    .rl_max = 12,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 55),
    .t_cphr = 2,
    .t_cphw = 6,
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 2500),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 10000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 100),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 150000),
    .t_mrr = 2,
    .t_mrs = 2,
    .t_wr_dmy = 1,
    },
};

const SRAM_RODATA_LOC struct PSRAM_DEVICE_TIMING_T opi_device_timing = {
    //default freq: 133MHz
    .wl_def = 0,
    .rl_min_def = 4,
    .rl_max_def = 8,
    .t_xphs_min_ns = 60,
    .is_mrs_wl_fixed = 0,
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
    .cmd_self_refr = 0
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_CFG_T opi_device_cfg = {
    .device_cmd_4bit = 0,
    .wr_dummy_cyc = 1,
    .stop_clk_in_nop =1,
    .stop_clk_in_tcph = 0,
    .dqs_dm_merge_en = 0,
    .mrw_output_dm = 0,
    .addr_4bytes_en = 0,
    .page_boundary = 0,
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_FUNC_CFG_T opi_device_func_cfg = {
    .psram_set_mr = psram_opi_aps64_mr_set,
    .psram_get_mr = NULL,
    .psram_show_mr = psram_opi_aps64_mr_show,
    .psram_check_die = psram_opi_aps64_check,
};

static int psram_opi_aps64_inited_check(enum HAL_PSRAM_ID_T id)
{
    uint32_t md;
    int ret = 0;

    md = hal_psram_device_mr_read(id, 0);
    PSRAM_TRACE(0, "****MR0 = 0x%x", md);

    if ((md & 0xff) == (PSRAM_200MHz_MR0)) {
        ret = 0;
    } else if ((md & 0xffff) == 0xfa11) {
        ret = 0xfa11;
    } else {
        ret = 1;
    }

    return ret;
}

const SRAM_RODATA_LOC struct PSRAM_DEVICE_TIMING_T opi_inited_device_timing = {
    //default freq: 200MHz
    .wl_def = 2,
    .rl_min_def = 6,
    .rl_max_def = 12,
    .t_xphs_min_ns = 60,
    .is_mrs_wl_fixed = 0,
    .mrs_as_pd_en = 1,
    .mrs_as_pd_mr = 6,
    .mrs_as_pd_md = 0xf0,
    .ac_timing = opi_ac_timing,
    .ac_timing_count = ARRAY_SIZE(opi_ac_timing)
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_FUNC_CFG_T opi_inited_device_func_cfg = {
    .psram_set_mr = psram_opi_aps64_mr_set,
    .psram_get_mr = NULL,
    .psram_show_mr = psram_opi_aps64_mr_show,
    .psram_check_die = psram_opi_aps64_inited_check,
};

#endif
