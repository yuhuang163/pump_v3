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

#define DRIVE_STR_FULL                      0
#define DRIVE_STR_HALF                      1
#define DRIVE_STR_1_4                       2
#define DRIVE_STR_1_8                       3
#define LT_VARIABLE                         0
#define LT_FIXED                            1

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

#ifdef PSRAM_1V2_ENABLE
#define READ_LATENCY_33MHz                  2
#define READ_LATENCY_66MHz                  3
#define READ_LATENCY_109MHz                 4
#define READ_LATENCY_133MHz                 5
#define READ_LATENCY_200MHz                 7

#define WRITE_LATENCY_33MHz                 1
#define WRITE_LATENCY_66MHz                 5
#define WRITE_LATENCY_109MHz                3
#define WRITE_LATENCY_133MHz                7
#define WRITE_LATENCY_200MHz                4
#else
#define READ_LATENCY_33MHz                  0
#define READ_LATENCY_66MHz                  0
#define READ_LATENCY_109MHz                 1
#define READ_LATENCY_133MHz                 2
#define READ_LATENCY_200MHz                 4

#define WRITE_LATENCY_33MHz                 0
#define WRITE_LATENCY_66MHz                 0
#define WRITE_LATENCY_109MHz                4
#define WRITE_LATENCY_133MHz                2
#define WRITE_LATENCY_200MHz                1
#endif

#define PSRAM_33MHz_MR0                     (MR0_DRIVE_STR(DRIVE_STR_FULL) | MR0_READ_LATENCY(READ_LATENCY_33MHz) | MR0_FIXED_00(LT_VARIABLE))
#define PSRAM_33MHz_MR4                     (MR4_PASR(0) | MR4_RF_RATE(MR4_RF_RATE_HALF) | MR4_WRITE_LATENCY(WRITE_LATENCY_33MHz))
#define PSRAM_66MHz_MR0                     (MR0_DRIVE_STR(DRIVE_STR_FULL) | MR0_READ_LATENCY(READ_LATENCY_66MHz) | MR0_FIXED_00(LT_VARIABLE))
#define PSRAM_66MHz_MR4                     (MR4_PASR(0) | MR4_RF_RATE(MR4_RF_RATE_HALF) | MR4_WRITE_LATENCY(WRITE_LATENCY_66MHz))
#define PSRAM_100MHz_MR0                    (MR0_DRIVE_STR(DRIVE_STR_FULL) | MR0_READ_LATENCY(READ_LATENCY_109MHz) | MR0_FIXED_00(LT_VARIABLE))
#define PSRAM_100MHz_MR4                    (MR4_PASR(0) | MR4_RF_RATE(MR4_RF_RATE_HALF) | MR4_WRITE_LATENCY(WRITE_LATENCY_109MHz))
#define PSRAM_133MHz_MR0                    (MR0_DRIVE_STR(DRIVE_STR_FULL) | MR0_READ_LATENCY(WRITE_LATENCY_133MHz) | MR0_FIXED_00(LT_VARIABLE))
#define PSRAM_133MHz_MR4                    (MR4_PASR(0) | MR4_RF_RATE(MR4_RF_RATE_HALF) | MR4_WRITE_LATENCY(WRITE_LATENCY_133MHz))
#define PSRAM_200MHz_MR0                    (MR0_DRIVE_STR(DRIVE_STR_FULL) | MR0_READ_LATENCY(READ_LATENCY_200MHz) | MR0_FIXED_00(LT_VARIABLE))
#define PSRAM_200MHz_MR4                    (MR4_PASR(0) | MR4_RF_RATE(MR4_RF_RATE_HALF) | MR4_WRITE_LATENCY(WRITE_LATENCY_200MHz))
#define PSRAM_x8_MR8                        (MR8_BL(3))
#define PSRAM_x16_MR8                       (MR8_BL(3) | MR8_X16)



static void psram_xopi_aps256_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val = 0;

    if (psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16) {
        val = hal_psram_mc_set_mgr_ch(id, HAL_PSRAM_MC_MGR_CHANEL_ALL);
    }

    if ((psram_cfg->psram_data_bus == PSRAM_DATA_BUS_X16) || psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16) {
        hal_psram_device_mr_write(id, 8, PSRAM_x16_MR8);
    } else {
        hal_psram_device_mr_write(id, 8, PSRAM_x8_MR8);
    }
    /// MRS  mr8,   0,   4
    //       0x43  0x24 0x80  100MHz
    //       0x43  0x28 0x40  133MHz
    //       0x43  0x30 0x20  200MHz
    if (psram_cfg->run_freq_mhz <=33) {
        hal_psram_device_mr_write(id, 0, PSRAM_33MHz_MR0);
        hal_psram_device_mr_write(id, 4, PSRAM_33MHz_MR4);
    } else if (psram_cfg->run_freq_mhz <=66) {
        hal_psram_device_mr_write(id, 0, PSRAM_66MHz_MR0);
        hal_psram_device_mr_write(id, 4, PSRAM_66MHz_MR4);
    } else if (psram_cfg->run_freq_mhz <=100) {
        hal_psram_device_mr_write(id, 0, PSRAM_100MHz_MR0);
        hal_psram_device_mr_write(id, 4, PSRAM_100MHz_MR4);
    } else if  (psram_cfg->run_freq_mhz <=133){
        hal_psram_device_mr_write(id, 0, PSRAM_133MHz_MR0);
        hal_psram_device_mr_write(id, 4, PSRAM_133MHz_MR4);
    } else {//if (psram_cfg->run_freq_mhz <=200)
        hal_psram_device_mr_write(id, 0, PSRAM_200MHz_MR0);
        hal_psram_device_mr_write(id, 4, PSRAM_200MHz_MR4);
    }

    if (psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16) {
       val = hal_psram_mc_set_mgr_ch(id, val);
    }
}

static void psram_xopi_aps256_mr_show(enum HAL_PSRAM_ID_T id)
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

static int psram_xopi_aps256_check(enum HAL_PSRAM_ID_T id)
{
    uint32_t md;
    int ret = 0;

    md = hal_psram_device_mr_read(id, 0);
    PSRAM_TRACE(0, "****MR8 = 0x%x", md);

    if ((md & 0xff) == 0x8) {
        ret = 0;
    } else if ((md & 0xffff) == 0xfa11) {
        ret = 0xfa11;
    } else {
        ret = 1;
    }

    return ret;
}

static void psram_xopi_aps256_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk)
{
    if ((psram_cfg->psram_data_bus == PSRAM_DATA_BUS_X16) || psram_cfg->psram_data_bus == PSRAM_DATA_BUS_2X16) {
        *reg = 8;
        *val = PSRAM_x16_MR8;
    } else {
        *reg = 8;
        *val = PSRAM_x8_MR8;
    }
    /// MRS  mr8,   0,   4
    //       0x43  0x10 0x78  100MHz
    //       0x43  0x14 0xf8  133MHz
    //       0x43  0x1c 0x98  200MHz
    if (clk <=33) {
        *(reg + 1) = 0;
        *(val + 1) = PSRAM_33MHz_MR0;
        *(reg + 2) = 4;
        *(val + 2) = PSRAM_33MHz_MR4;
    } else if (clk <=66) {
        *(reg + 1) = 0;
        *(val + 1) = PSRAM_66MHz_MR0;
        *(reg + 2) = 4;
        *(val + 2) = PSRAM_66MHz_MR4;
    } else if (clk <=100) {
        *(reg + 1) = 0;
        *(val + 1) = PSRAM_100MHz_MR0;
        *(reg + 2) = 4;
        *(val + 2) = PSRAM_100MHz_MR4;
    } else if  (clk <=133){
        *(reg + 1) = 0;
        *(val + 1) = PSRAM_133MHz_MR0;
        *(reg + 2) = 4;
        *(val + 2) = PSRAM_133MHz_MR4;
    } else {//if (clk <=200)
        *(reg + 1) = 0;
        *(val + 1) = PSRAM_200MHz_MR0;
        *(reg + 2) = 4;
        *(val + 2) = PSRAM_200MHz_MR4;
    }
}

#ifdef PSRAM_1V2_ENABLE
/// APS256XXN_1V2:
const BOOT_RODATA_FLASH_LOC struct PSRAM_AC_TIMING_T xopi_ac_timing[] ={
    {.freq_mhz = 24,
    .wl = 7,
    .rl_min = 6,
    .rl_max = 11,
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
    }, {
    .freq_mhz = 48,
    .wl = 8,
    .rl_min = 7,
    .rl_max = 13,
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
    }, {
    .freq_mhz = 100,
    .wl = 9,
    .rl_min = 8,
    .rl_max = 15 ,
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
    }, {
    .freq_mhz = 133,
    .wl = 10,
    .rl_min = 9,
    .rl_max = 17,
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
    }, {
    .freq_mhz = 200,
    .wl = 12,
    .rl_min = 11,
    .rl_max = 21,
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
    },
};
#else
/// APS256XXN_1V8:
const BOOT_RODATA_FLASH_LOC struct PSRAM_AC_TIMING_T xopi_ac_timing[] ={
    {.freq_mhz = 24,
    .wl = 3,
    .rl_min = 3,
    .rl_max = 6,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 15),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 15),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_24MHZ, 6.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_24MHZ, 60),
    }, {
    .freq_mhz = 48,
    .wl = 3,
    .rl_min = 3,
    .rl_max = 6,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_48MHZ, 6.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_48MHZ, 60),
    }, {
    .freq_mhz = 100,
    .wl = 4,
    .rl_min = 4,
    .rl_max = 8,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 15),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 15),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_100MHZ, 6.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_100MHZ, 60),
    }, {
    .freq_mhz = 133,
    .wl = 5,
    .rl_min = 5,
    .rl_max = 10,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 15),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 15),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_133MHZ, 6.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 15),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 15),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_133MHZ, 60),
    }, {
    .freq_mhz = 200,
    .wl = 7,
    .rl_min = 7,
    .rl_max = 14,
    .t_rc = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_cphr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 24),
    .t_cphw = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 24),
    .t_cem = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 2000),
    .t_rst = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 2000),
    .t_hs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 150000),
    .t_xphs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_xhs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 150000),
    .t_dqsck =  T_NS_TO_NCK(PSRAM_CLK_200MHZ, 6.5),
    .t_mrr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 24),
    .t_mrs = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 24),
    .t_srf = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    .t_xsr = T_NS_TO_NCK(PSRAM_CLK_200MHZ, 60),
    },
};
#endif

const SRAM_RODATA_LOC struct PSRAM_DEVICE_TIMING_T xopi_device_timing = {
    //default freq: 133MHz
#ifdef PSRAM_1V2_ENABLE
    .wl_def = 10,
    .rl_min_def = 9,
    .rl_max_def = 17,
#else
    .wl_def = 5,
    .rl_min_def = 5,
    .rl_max_def = 10,
#endif
    .is_mrs_wl_fixed = true,
    .fixed_wl_mrs = 1,
    .bl_mrs = 4,  // ?
    .t_xphs_min_ns = 60,
    .t_dqsck_max_ns = 6.5,

    .mrs_as_pd_en = 1,
    .mrs_as_pd_mr = 6,
    .mrs_as_pd_md = 0xf0,

    .ac_timing = xopi_ac_timing,
    .ac_timing_count = ARRAY_SIZE(xopi_ac_timing)
};

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_CFG_T xopi_device_cfg = {
    //mc cfg with different device
    .device_ca_map = 2,
    .device_cmd_4bit = 0,
    .device_pattern = 7,
    .burst_length = 3,
    .page_boundary = 1,
    .device_wrap_en = 0,
    .device_auto_precharge = 0,
    .wr_dummy_cyc = 0,
    .cmd_dummy_cyc = 4,
    .stop_clk_in_nop =1,
    .stop_clk_in_tcph = 1,
    //phy cfg with different device
    .hyperbus_sample_lat_edge_sel = 0,
    .hyperbus_cmd_dly_cnt = 0,
    .clk_preample_en = 0,
    .device_preample_en = 0,
    .hyperbus_tcss = 0,
    .phy_dummy_cyc_en = 0,
    .phy_dummy_cyc_stop_dqs = 0,
    .dqs_dm_merge_en = 1,
    .mrw_output_dm = 0,
    .cs_pol = 0,
    .addr_4bytes_en = 1,
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

const BOOT_RODATA_FLASH_LOC struct PSRAM_DEVICE_FUNC_CFG_T xopi_device_func_cfg = {
    .psram_set_mr = psram_xopi_aps256_mr_set,
    .psram_get_mr = psram_xopi_aps256_mr_get,
    .psram_show_mr = psram_xopi_aps256_mr_show,
    .psram_check_die = psram_xopi_aps256_check,
};

#endif
