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
#ifndef __REG_PSRAM_MC_V12_H__
#define __REG_PSRAM_MC_V12_H__

#include "plat_types.h"

struct PSRAM_MC_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
    __IO uint32_t REG_030;
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
    __IO uint32_t REG_040;
    __IO uint32_t REG_044;
    __IO uint32_t REG_048;
    __IO uint32_t REG_04C;
    __IO uint32_t REG_050;
    __IO uint32_t REG_054;
    __IO uint32_t REG_058;
    __IO uint32_t REG_05C;
    __IO uint32_t REG_060;
    __IO uint32_t REG_064;
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
    __IO uint32_t REG_078;
    __IO uint32_t REG_07C;
    __IO uint32_t REG_080;
    __IO uint32_t REG_084;
    __IO uint32_t REG_088;
    __IO uint32_t REG_08C;
    __IO uint32_t REG_090;
    __IO uint32_t REG_094;
    __IO uint32_t REG_098;
    __IO uint32_t REG_09C;
    __IO uint32_t REG_0A0;
    __IO uint32_t REG_0A4;
    __IO uint32_t REG_0A8;
    __IO uint32_t REG_0AC;
    __IO uint32_t REG_0B0;
    __IO uint32_t REG_0B4;
    __IO uint32_t REG_0B8;
    __IO uint32_t REG_0BC;
    __IO uint32_t REG_0C0;
    __IO uint32_t REG_0C4;
    __IO uint32_t REG_0C8;
    __IO uint32_t REG_0CC;
    __IO uint32_t REG_0D0;
    __IO uint32_t REG_0D4;
    __IO uint32_t REG_0D8;
    __IO uint32_t REG_0DC;
    __IO uint32_t REG_0E0;
    __IO uint32_t REG_0E4;
    __IO uint32_t REG_0E8;
    __IO uint32_t REG_0EC;
    __IO uint32_t REG_0F0;
    __IO uint32_t REG_0F4;
    __IO uint32_t REG_0F8;
    __IO uint32_t REG_0FC;
    __IO uint32_t REG_100;
    __IO uint32_t REG_104;
    __IO uint32_t REG_108;
    __IO uint32_t REG_RESERVED_10C[0xD];
    __IO uint32_t REG_140;
    __IO uint32_t REG_RESERVED_144[0x3];
    __IO uint32_t REG_150;
    __IO uint32_t REG_RESERVED_154[0x3];
    __IO uint32_t REG_160;
    __IO uint32_t REG_164;
    __IO uint32_t REG_168;
    __IO uint32_t REG_16C;
    __IO uint32_t REG_RESERVED_170[0x4];
    __IO uint32_t REG_180;
    __IO uint32_t REG_184;
    __IO uint32_t REG_188;
    __IO uint32_t REG_18C;
    __IO uint32_t REG_190;
    __IO uint32_t REG_194;
    __IO uint32_t REG_198;
    __IO uint32_t REG_19C;
    __IO uint32_t REG_RESERVED_1A0[0x18];
    __IO uint32_t REG_200;
    __IO uint32_t REG_204;
    __IO uint32_t REG_208;
    __IO uint32_t REG_20C;
    __IO uint32_t REG_210;
    __IO uint32_t REG_214;
    __IO uint32_t REG_218;
    __IO uint32_t REG_21C;
    __IO uint32_t REG_220;
    __IO uint32_t REG_224;
    __IO uint32_t REG_228;
    __IO uint32_t REG_22C;
    __IO uint32_t REG_230;
    __IO uint32_t REG_234;
    __IO uint32_t REG_238;
    __IO uint32_t REG_23C;
    __IO uint32_t REG_240;
    __IO uint32_t REG_244;
    __IO uint32_t REG_248;
    __IO uint32_t REG_24C;
    __IO uint32_t REG_250;
    __IO uint32_t REG_254;
    __IO uint32_t REG_258;
    __IO uint32_t REG_25C;
    __IO uint32_t REG_260;
    __IO uint32_t REG_264;
    __IO uint32_t REG_268;
    __IO uint32_t REG_26C;
    __IO uint32_t REG_270;
    __IO uint32_t REG_274;
    __IO uint32_t REG_278;
    __IO uint32_t REG_27C;
    __IO uint32_t REG_280;
    __IO uint32_t REG_284;
    __IO uint32_t REG_288;
    __IO uint32_t REG_28C;
    __IO uint32_t REG_290;
    __IO uint32_t REG_294;
    __IO uint32_t REG_298;
    __IO uint32_t REG_29C;
    __IO uint32_t REG_2A0;
    __IO uint32_t REG_2A4;
    __IO uint32_t REG_2A8;
    __IO uint32_t REG_2AC;
    __IO uint32_t REG_2B0;
    __IO uint32_t REG_2B4;
    __IO uint32_t REG_2B8;
    __IO uint32_t REG_RESERVED_2AC[0x51];
    __IO uint32_t REG_400;
    __IO uint32_t REG_404;
    __IO uint32_t REG_408;
    __IO uint32_t REG_RESERVED_40C[0xD];
    __IO uint32_t REG_440;
    __IO uint32_t REG_444;
    __IO uint32_t REG_448;
    __IO uint32_t REG_44C;
    __IO uint32_t REG_450;
    __IO uint32_t REG_454;
    __IO uint32_t REG_458;
    __IO uint32_t REG_45C;
    __IO uint32_t REG_460;
    __IO uint32_t REG_464;
    __IO uint32_t REG_468;
    __IO uint32_t REG_46C;
    __IO uint32_t REG_470;
    __IO uint32_t REG_474;
    __IO uint32_t REG_478;
    __IO uint32_t REG_47C;
    __IO uint32_t REG_480;
    __IO uint32_t REG_484;
    __IO uint32_t REG_RESERVED_488[0x13];
    __IO uint32_t REG_4D4;
    __IO uint32_t REG_4D8;
    __IO uint32_t REG_RESERVED_4DC[0xC5];
    __IO uint32_t REG_7F0;
};


// reg_000
#define PSRAM_MC_CHIP_BIT                                   (1 << 0)
#define PSRAM_MC_CHIP_TYPE                                  (1 << 1)
#define PSRAM_MC_CHIP_X16                                   (1 << 2)
#define PSRAM_MC_CHIP_CA_MAP(n)                             (((n) & 0xF) << 3)
#define PSRAM_MC_CHIP_CA_MAP_MASK                           (0xF << 3)
#define PSRAM_MC_CHIP_CA_MAP_SHIFT                          (3)
#define PSRAM_MC_SQPI_MODE                                  (1 << 7)
#define PSRAM_MC_QPI_MODE                                   (1 << 8)
#define PSRAM_MC_OPI_MODE                                   (1 << 9)
#define PSRAM_MC_DTR_MODE(n)                                (((n) & 0x7) << 10)
#define PSRAM_MC_DTR_MODE_MASK                              (0x7 << 10)
#define PSRAM_MC_DTR_MODE_SHIFT                             (10)
#define PSRAM_MC_DQS_MODE                                   (1 << 13)
#define PSRAM_MC_LATENCY_TYPE                               (1 << 14)
#define PSRAM_MC_SQPI_CMD_SEQ_DEFINE_EN                     (1 << 15)
#define PSRAM_MC_HYPERBUS                                   (1 << 16)
#define PSRAM_MC_SEL_DIV_PSRAM_INN                          (1 << 17)
#define PSRAM_MC_CHIP_CMD_4BIT                              (1 << 18)
#define PSRAM_MC_CHIP_SWITCH                                (1 << 19)
#define PSRAM_MC_CHIP_DDP                                   (1 << 20)
#define PSRAM_MC_CHIP_CS0_REGION(n)                         (((n) & 0x7) << 21)
#define PSRAM_MC_CHIP_CS0_REGION_MASK                       (0x7 << 21)
#define PSRAM_MC_CHIP_CS0_REGION_SHIFT                      (21)
#define PSRAM_MC_CHIP_DDP_CLK_COMMON                        (1 << 24)

// reg_004
#define PSRAM_MC_MGR_CMD(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_MGR_CMD_MASK                               (0xFF << 0)
#define PSRAM_MC_MGR_CMD_SHIFT                              (0)

// reg_008
#define PSRAM_MC_MGR_ADDR(n)                                (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_MGR_ADDR_MASK                              (0xFFFFFFFF << 0)
#define PSRAM_MC_MGR_ADDR_SHIFT                             (0)

// reg_00c
#define PSRAM_MC_MGR_LEN(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_MGR_LEN_MASK                               (0xFF << 0)
#define PSRAM_MC_MGR_LEN_SHIFT                              (0)
#define PSRAM_MC_MGR_LEN_BYPASS                             (1 << 8)
#define PSRAM_MC_MRS_AS_PD_LEN(n)                           (((n) & 0xF) << 9)
#define PSRAM_MC_MRS_AS_PD_LEN_MASK                         (0xF << 9)
#define PSRAM_MC_MRS_AS_PD_LEN_SHIFT                        (9)
#define PSRAM_MC_MGR_MR_CH(n)                               (((n) & 0x3) << 13)
#define PSRAM_MC_MGR_MR_CH_MASK                             (0x3 << 13)
#define PSRAM_MC_MGR_MR_CH_SHIFT                            (13)

// reg_010
#define PSRAM_MC_MGR_WSTRB(n)                               (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_MGR_WSTRB_MASK                             (0xFFFFFFFF << 0)
#define PSRAM_MC_MGR_WSTRB_SHIFT                            (0)

// reg_014
#define PSRAM_MC_MGR_TX_FIFO(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_MGR_TX_FIFO_MASK                           (0xFFFFFFFF << 0)
#define PSRAM_MC_MGR_TX_FIFO_SHIFT                          (0)

// reg_018
#define PSRAM_MC_MGR_RX_FIFO(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_MGR_RX_FIFO_MASK                           (0xFFFFFFFF << 0)
#define PSRAM_MC_MGR_RX_FIFO_SHIFT                          (0)

// reg_01c
#define PSRAM_MC_MGR_TX_FIFO_CLR                            (1 << 0)
#define PSRAM_MC_MGR_RX_FIFO_CLR                            (1 << 1)

// reg_020
#define PSRAM_MC_MRS_AS_PD_EN                               (1 << 0)
#define PSRAM_MC_MRS_AS_PD_MR(n)                            (((n) & 0xFF) << 1)
#define PSRAM_MC_MRS_AS_PD_MR_MASK                          (0xFF << 1)
#define PSRAM_MC_MRS_AS_PD_MR_SHIFT                         (1)
#define PSRAM_MC_MRS_AS_PD_MD(n)                            (((n) & 0xFFFF) << 9)
#define PSRAM_MC_MRS_AS_PD_MD_MASK                          (0xFFFF << 9)
#define PSRAM_MC_MRS_AS_PD_MD_SHIFT                         (9)
#define PSRAM_MC_MRS_AS_PD_WRLAT_DR                         (1 << 25)
#define PSRAM_MC_MRS_AS_PD_WRLAT(n)                         (((n) & 0x1F) << 26)
#define PSRAM_MC_MRS_AS_PD_WRLAT_MASK                       (0x1F << 26)
#define PSRAM_MC_MRS_AS_PD_WRLAT_SHIFT                      (26)

// reg_024
#define PSRAM_MC_ENTRY_SLEEP_IDLE                           (1 << 0)
#define PSRAM_MC_ENTRY_SELF_REFRESH_IDLE                    (1 << 1)
#define PSRAM_MC_STOP_CLK_IDLE                              (1 << 2)
#define PSRAM_MC_AUTOWAKEUP_EN                              (1 << 3)
#define PSRAM_MC_SELF_REFRESH_LP_EN                         (1 << 4)
#define PSRAM_MC_MAN_CKE_EN                                 (1 << 5)
#define PSRAM_MC_REFRESH_MODE                               (1 << 6)
#define PSRAM_MC_BURST_REFRESH_EN                           (1 << 7)
#define PSRAM_MC_ENTRY_PHY_DOWN_IDLE                        (1 << 8)

// reg_028
#define PSRAM_MC_WRITE_LATENCY_0(n)                         (((n) & 0xFF) << 0)
#define PSRAM_MC_WRITE_LATENCY_0_MASK                       (0xFF << 0)
#define PSRAM_MC_WRITE_LATENCY_0_SHIFT                      (0)
#define PSRAM_MC_WRITE_LATENCY_MR_0(n)                      (((n) & 0xFF) << 8)
#define PSRAM_MC_WRITE_LATENCY_MR_0_MASK                    (0xFF << 8)
#define PSRAM_MC_WRITE_LATENCY_MR_0_SHIFT                   (8)

// reg_02c
#define PSRAM_MC_READ_LATENCY_0(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_READ_LATENCY_0_MASK                        (0xFF << 0)
#define PSRAM_MC_READ_LATENCY_0_SHIFT                       (0)
#define PSRAM_MC_READ_LATENCY1_0(n)                         (((n) & 0xFF) << 8)
#define PSRAM_MC_READ_LATENCY1_0_MASK                       (0xFF << 8)
#define PSRAM_MC_READ_LATENCY1_0_SHIFT                      (8)

// reg_030
#define PSRAM_MC_PREFETCH_16N                               (1 << 0)
#define PSRAM_MC_PREFETCH_32N                               (1 << 1)

// reg_034
#define PSRAM_MC_BURST_LENGTH_0(n)                          (((n) & 0x7) << 0)
#define PSRAM_MC_BURST_LENGTH_0_MASK                        (0x7 << 0)
#define PSRAM_MC_BURST_LENGTH_0_SHIFT                       (0)
#define PSRAM_MC_PAGE_BOUNDARY(n)                           (((n) & 0x3) << 3)
#define PSRAM_MC_PAGE_BOUNDARY_MASK                         (0x3 << 3)
#define PSRAM_MC_PAGE_BOUNDARY_SHIFT                        (3)
#define PSRAM_MC_CMD_ENTER_QUAD(n)                          (((n) & 0xFF) << 5)
#define PSRAM_MC_CMD_ENTER_QUAD_MASK                        (0xFF << 5)
#define PSRAM_MC_CMD_ENTER_QUAD_SHIFT                       (5)
#define PSRAM_MC_CMD_EXIT_QUAD(n)                           (((n) & 0xFF) << 13)
#define PSRAM_MC_CMD_EXIT_QUAD_MASK                         (0xFF << 13)
#define PSRAM_MC_CMD_EXIT_QUAD_SHIFT                        (13)

// reg_038
#define PSRAM_MC_BUS_WIDTH                                  (1 << 0)

// reg_03c
#define PSRAM_MC_HYPERBUS_REG_ADDR_MAP(n)                   (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_HYPERBUS_REG_ADDR_MAP_MASK                 (0xFFFFFFFF << 0)
#define PSRAM_MC_HYPERBUS_REG_ADDR_MAP_SHIFT                (0)

// reg_040
#define PSRAM_MC_CP_WRAP_EN_0                               (1 << 0)
#define PSRAM_MC_AUTO_PRECHARGE                             (1 << 1)
#define PSRAM_MC_TOGGLE_BURST_LEN_EN                        (1 << 2)
#define PSRAM_MC_TOGGLE_BURST_LEN_SET(n)                    (((n) & 0x7) << 3)
#define PSRAM_MC_TOGGLE_BURST_LEN_SET_MASK                  (0x7 << 3)
#define PSRAM_MC_TOGGLE_BURST_LEN_SET_SHIFT                 (3)
#define PSRAM_MC_TOGGLE_BURST_LEN_CMD(n)                    (((n) & 0xFF) << 6)
#define PSRAM_MC_TOGGLE_BURST_LEN_CMD_MASK                  (0xFF << 6)
#define PSRAM_MC_TOGGLE_BURST_LEN_CMD_SHIFT                 (6)
#define PSRAM_MC_TOGGLE_WRAP_EN                             (1 << 14)
#define PSRAM_MC_TOGGLE_WRAP_SET                            (1 << 15)

// reg_044
#define PSRAM_MC_WB_DRAIN                                   (1 << 0)
#define PSRAM_MC_WB_INVALID                                 (1 << 1)
#define PSRAM_MC_SNP_DISABLE                                (1 << 2)
#define PSRAM_MC_WB_BYPASS                                  (1 << 3)
#define PSRAM_MC_SNP_HIT_HALT_EN(n)                         (((n) & 0x3) << 4)
#define PSRAM_MC_SNP_HIT_HALT_EN_MASK                       (0x3 << 4)
#define PSRAM_MC_SNP_HIT_HALT_EN_SHIFT                      (4)

// reg_048
#define PSRAM_MC_FRE_RATIO(n)                               (((n) & 0x3) << 0)
#define PSRAM_MC_FRE_RATIO_MASK                             (0x3 << 0)
#define PSRAM_MC_FRE_RATIO_SHIFT                            (0)

// reg_04c
#define PSRAM_MC_T_REFI_0(n)                                (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_REFI_0_MASK                              (0xFFFF << 0)
#define PSRAM_MC_T_REFI_0_SHIFT                             (0)
#define PSRAM_MC_NUM_OF_BURST_RFS(n)                        (((n) & 0xFFFF) << 16)
#define PSRAM_MC_NUM_OF_BURST_RFS_MASK                      (0xFFFF << 16)
#define PSRAM_MC_NUM_OF_BURST_RFS_SHIFT                     (16)

// reg_050
#define PSRAM_MC_T_RC_0(n)                                  (((n) & 0xFF) << 0)
#define PSRAM_MC_T_RC_0_MASK                                (0xFF << 0)
#define PSRAM_MC_T_RC_0_SHIFT                               (0)

// reg_054
#define PSRAM_MC_T_RFC_0(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_T_RFC_0_MASK                               (0xFF << 0)
#define PSRAM_MC_T_RFC_0_SHIFT                              (0)

// reg_058
#define PSRAM_MC_T_CPHR_0(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHR_0_MASK                              (0x3F << 0)
#define PSRAM_MC_T_CPHR_0_SHIFT                             (0)

// reg_05c
#define PSRAM_MC_T_CPHR_AP_0(n)                             (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHR_AP_0_MASK                           (0x3F << 0)
#define PSRAM_MC_T_CPHR_AP_0_SHIFT                          (0)

// reg_060
#define PSRAM_MC_T_CPHW_0(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHW_0_MASK                              (0x3F << 0)
#define PSRAM_MC_T_CPHW_0_SHIFT                             (0)

// reg_064
#define PSRAM_MC_T_CPHW_AP_0(n)                             (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHW_AP_0_MASK                           (0x3F << 0)
#define PSRAM_MC_T_CPHW_AP_0_SHIFT                          (0)

// reg_068
#define PSRAM_MC_T_MRR_0(n)                                 (((n) & 0x3F) << 0)
#define PSRAM_MC_T_MRR_0_MASK                               (0x3F << 0)
#define PSRAM_MC_T_MRR_0_SHIFT                              (0)

// reg_06c
#define PSRAM_MC_T_MRS_0(n)                                 (((n) & 0x3FF) << 0)
#define PSRAM_MC_T_MRS_0_MASK                               (0x3FF << 0)
#define PSRAM_MC_T_MRS_0_SHIFT                              (0)

// reg_070
#define PSRAM_MC_T_CEM_0(n)                                 (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_CEM_0_MASK                               (0xFFFF << 0)
#define PSRAM_MC_T_CEM_0_SHIFT                              (0)

// reg_074
#define PSRAM_MC_T_RST_0(n)                                 (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_RST_0_MASK                               (0xFFFF << 0)
#define PSRAM_MC_T_RST_0_SHIFT                              (0)

// reg_078
#define PSRAM_MC_T_SRF_0(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_T_SRF_0_MASK                               (0xFF << 0)
#define PSRAM_MC_T_SRF_0_SHIFT                              (0)

// reg_07c
#define PSRAM_MC_T_XSR_0(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_T_XSR_0_MASK                               (0xFF << 0)
#define PSRAM_MC_T_XSR_0_SHIFT                              (0)

// reg_080
#define PSRAM_MC_T_HS_0(n)                                  (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_HS_0_MASK                                (0xFFFF << 0)
#define PSRAM_MC_T_HS_0_SHIFT                               (0)

// reg_084
#define PSRAM_MC_T_XPHS_0(n)                                (((n) & 0xFF) << 0)
#define PSRAM_MC_T_XPHS_0_MASK                              (0xFF << 0)
#define PSRAM_MC_T_XPHS_0_SHIFT                             (0)

// reg_088
#define PSRAM_MC_T_XHS_0(n)                                 (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_XHS_0_MASK                               (0xFFFFF << 0)
#define PSRAM_MC_T_XHS_0_SHIFT                              (0)

// reg_08c
#define PSRAM_MC_T_ZQCAL_0(n)                               (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_ZQCAL_0_MASK                             (0xFFFFF << 0)
#define PSRAM_MC_T_ZQCAL_0_SHIFT                            (0)

// reg_090
#define PSRAM_MC_T_ZQCRST_0(n)                              (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_ZQCRST_0_MASK                            (0xFFFFF << 0)
#define PSRAM_MC_T_ZQCRST_0_SHIFT                           (0)

// reg_094
#define PSRAM_MC_T_XCKD_0(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_XCKD_0_MASK                              (0x3F << 0)
#define PSRAM_MC_T_XCKD_0_SHIFT                             (0)

// reg_098
#define PSRAM_MC_T_ECKD_0(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_ECKD_0_MASK                              (0x3F << 0)
#define PSRAM_MC_T_ECKD_0_SHIFT                             (0)

// reg_09c
#define PSRAM_MC_WR_DMY_CYC(n)                              (((n) & 0xFF) << 0)
#define PSRAM_MC_WR_DMY_CYC_MASK                            (0xFF << 0)
#define PSRAM_MC_WR_DMY_CYC_SHIFT                           (0)
#define PSRAM_MC_CMD_DMY_CYC(n)                             (((n) & 0xFF) << 8)
#define PSRAM_MC_CMD_DMY_CYC_MASK                           (0xFF << 8)
#define PSRAM_MC_CMD_DMY_CYC_SHIFT                          (8)
#define PSRAM_MC_NOP_DMY_CYC(n)                             (((n) & 0xFF) << 16)
#define PSRAM_MC_NOP_DMY_CYC_MASK                           (0xFF << 16)
#define PSRAM_MC_NOP_DMY_CYC_SHIFT                          (16)

// reg_0a0
#define PSRAM_MC_STOP_CLK_IN_NOP                            (1 << 0)
#define PSRAM_MC_STOP_CLK_IN_TCPH                           (1 << 1)

// reg_0a4
#define PSRAM_MC_QUEUE_IDLE_CYCLE(n)                        (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_QUEUE_IDLE_CYCLE_MASK                      (0xFFFFFFFF << 0)
#define PSRAM_MC_QUEUE_IDLE_CYCLE_SHIFT                     (0)

// reg_0a8
#define PSRAM_MC_T_EXPANDRD(n)                              (((n) & 0x3F) << 0)
#define PSRAM_MC_T_EXPANDRD_MASK                            (0x3F << 0)
#define PSRAM_MC_T_EXPANDRD_SHIFT                           (0)

// reg_0ac
#define PSRAM_MC_RX_SYNC_BYPASS                             (1 << 0)

// reg_0b0
#define PSRAM_MC_TURN_LIMIT(n)                              (((n) & 0xF) << 0)
#define PSRAM_MC_TURN_LIMIT_MASK                            (0xF << 0)
#define PSRAM_MC_TURN_LIMIT_SHIFT                           (0)

// reg_0b4
#define PSRAM_MC_T_ZQCAS_0(n)                               (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_ZQCAS_0_MASK                             (0xFFFFF << 0)
#define PSRAM_MC_T_ZQCAS_0_SHIFT                            (0)

// reg_0b8
#define PSRAM_MC_T_NEW_HOLD(n)                              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_T_NEW_HOLD_MASK                            (0xFFFFFFFF << 0)
#define PSRAM_MC_T_NEW_HOLD_SHIFT                           (0)

// reg_0bc
#define PSRAM_MC_NEW_CMD_OP(n)                              (((n) & 0x7) << 0)
#define PSRAM_MC_NEW_CMD_OP_MASK                            (0x7 << 0)
#define PSRAM_MC_NEW_CMD_OP_SHIFT                           (0)

// reg_0c0
#define PSRAM_MC_T_REF_WIN_0(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_T_REF_WIN_0_MASK                           (0xFFFFFFFF << 0)
#define PSRAM_MC_T_REF_WIN_0_SHIFT                          (0)

// reg_0c4
#define PSRAM_MC_T_CSCKE(n)                                 (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_CSCKE_MASK                               (0xFFFF << 0)
#define PSRAM_MC_T_CSCKE_SHIFT                              (0)
#define PSRAM_MC_T_CSCKEH(n)                                (((n) & 0xFFFF) << 16)
#define PSRAM_MC_T_CSCKEH_MASK                              (0xFFFF << 16)
#define PSRAM_MC_T_CSCKEH_SHIFT                             (16)

// reg_0c8
#define PSRAM_MC_CFG_RD_CONTI_RANGE(n)                      (((n) & 0x7) << 0)
#define PSRAM_MC_CFG_RD_CONTI_RANGE_MASK                    (0x7 << 0)
#define PSRAM_MC_CFG_RD_CONTI_RANGE_SHIFT                   (0)
#define PSRAM_MC_CFG_RD_NSEQ_NUM(n)                         (((n) & 0xF) << 3)
#define PSRAM_MC_CFG_RD_NSEQ_NUM_MASK                       (0xF << 3)
#define PSRAM_MC_CFG_RD_NSEQ_NUM_SHIFT                      (3)
#define PSRAM_MC_CFG_RD_SEQ_NUM(n)                          (((n) & 0xF) << 7)
#define PSRAM_MC_CFG_RD_SEQ_NUM_MASK                        (0xF << 7)
#define PSRAM_MC_CFG_RD_SEQ_NUM_SHIFT                       (7)
#define PSRAM_MC_CFG_RD_NACC_NUM(n)                         (((n) & 0xFF) << 11)
#define PSRAM_MC_CFG_RD_NACC_NUM_MASK                       (0xFF << 11)
#define PSRAM_MC_CFG_RD_NACC_NUM_SHIFT                      (11)
#define PSRAM_MC_CFG_RD_ACC_NUM(n)                          (((n) & 0xFF) << 19)
#define PSRAM_MC_CFG_RD_ACC_NUM_MASK                        (0xFF << 19)
#define PSRAM_MC_CFG_RD_ACC_NUM_SHIFT                       (19)
#define PSRAM_MC_CFG_RD_CASE_SEL(n)                         (((n) & 0x3) << 27)
#define PSRAM_MC_CFG_RD_CASE_SEL_MASK                       (0x3 << 27)
#define PSRAM_MC_CFG_RD_CASE_SEL_SHIFT                      (27)

// reg_0cc
#define PSRAM_MC_CFG_RD_TRANS_NUM(n)                        (((n) & 0xFF) << 0)
#define PSRAM_MC_CFG_RD_TRANS_NUM_MASK                      (0xFF << 0)
#define PSRAM_MC_CFG_RD_TRANS_NUM_SHIFT                     (0)
#define PSRAM_MC_CFG_RD_DIFF_NUM(n)                         (((n) & 0xFF) << 8)
#define PSRAM_MC_CFG_RD_DIFF_NUM_MASK                       (0xFF << 8)
#define PSRAM_MC_CFG_RD_DIFF_NUM_SHIFT                      (8)
#define PSRAM_MC_CFG_RD_CASE_MASK(n)                        (((n) & 0x7) << 16)
#define PSRAM_MC_CFG_RD_CASE_MASK_MASK                      (0x7 << 16)
#define PSRAM_MC_CFG_RD_CASE_MASK_SHIFT                     (16)

// reg_0d0
#define PSRAM_MC_DYN_SWITCH_EN                              (1 << 0)
#define PSRAM_MC_DYN_SWITCH_MR_0(n)                         (((n) & 0xFF) << 1)
#define PSRAM_MC_DYN_SWITCH_MR_0_MASK                       (0xFF << 1)
#define PSRAM_MC_DYN_SWITCH_MR_0_SHIFT                      (1)
#define PSRAM_MC_DYN_SWITCH_MD_0(n)                         (((n) & 0xFF) << 9)
#define PSRAM_MC_DYN_SWITCH_MD_0_MASK                       (0xFF << 9)
#define PSRAM_MC_DYN_SWITCH_MD_0_SHIFT                      (9)

// reg_0d4
#define PSRAM_MC_DYN_SWITCH_MR_1(n)                         (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_SWITCH_MR_1_MASK                       (0xFF << 0)
#define PSRAM_MC_DYN_SWITCH_MR_1_SHIFT                      (0)
#define PSRAM_MC_DYN_SWITCH_MD_1(n)                         (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_SWITCH_MD_1_MASK                       (0xFF << 8)
#define PSRAM_MC_DYN_SWITCH_MD_1_SHIFT                      (8)

// reg_0d8
#define PSRAM_MC_T_CKE_0(n)                                 (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_CKE_0_MASK                               (0xFFFF << 0)
#define PSRAM_MC_T_CKE_0_SHIFT                              (0)

// reg_0dc
#define PSRAM_MC_T_CKE_1(n)                                 (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_CKE_1_MASK                               (0xFFFF << 0)
#define PSRAM_MC_T_CKE_1_SHIFT                              (0)

// reg_0e0
#define PSRAM_MC_RD_DF_EN                                   (1 << 0)
#define PSRAM_MC_RD_DF_DEC_EN                               (1 << 1)
#define PSRAM_MC_WR_DM_EN                                   (1 << 2)
#define PSRAM_MC_WR_DM_DEC_EN                               (1 << 3)
#define PSRAM_MC_RD_RIB_FIFO_POP_MERGE                      (1 << 4)
#define PSRAM_MC_PREFETCH_ENABLE                            (1 << 5)
#define PSRAM_MC_PREFETCH_SNP_EN                            (1 << 6)

// reg_0e4
#define PSRAM_MC_PHY_TURN_MASK_REQ_EN(n)                    (((n) & 0x1F) << 0)
#define PSRAM_MC_PHY_TURN_MASK_REQ_EN_MASK                  (0x1F << 0)
#define PSRAM_MC_PHY_TURN_MASK_REQ_EN_SHIFT                 (0)

// reg_0e8
#define PSRAM_MC_CACTIVE_M_MAN                              (1 << 0)
#define PSRAM_MC_CACTIVE_P_MAN                              (1 << 1)
#define PSRAM_MC_CACTIVE_B_MAN                              (1 << 2)
#define PSRAM_MC_CACTIVE_OUT_M_MAN                          (1 << 3)
#define PSRAM_MC_CACTIVE_OUT_H_MAN                          (1 << 4)

// reg_0ec
#define PSRAM_MC_TRANS_TIMEOUT_PRESCALAR(n)                 (((n) & 0x3) << 0)
#define PSRAM_MC_TRANS_TIMEOUT_PRESCALAR_MASK               (0x3 << 0)
#define PSRAM_MC_TRANS_TIMEOUT_PRESCALAR_SHIFT              (0)
#define PSRAM_MC_TRANS_TIMEOUT_MASK_EN                      (1 << 2)
#define PSRAM_MC_TRANS_SPLIT_MASK_EN                        (1 << 3)

// reg_0f0
#define PSRAM_MC_TEST_MODE                                  (1 << 0)
#define PSRAM_MC_LPBK_START                                 (1 << 1)
#define PSRAM_MC_LPBK_STATUS(n)                             (((n) & 0x7) << 2)
#define PSRAM_MC_LPBK_STATUS_MASK                           (0x7 << 2)
#define PSRAM_MC_LPBK_STATUS_SHIFT                          (2)

// reg_0f4
#define PSRAM_MC_WB_REQ_LEVEL(n)                            (((n) & 0xF) << 0)
#define PSRAM_MC_WB_REQ_LEVEL_MASK                          (0xF << 0)
#define PSRAM_MC_WB_REQ_LEVEL_SHIFT                         (0)

// reg_0f8
#define PSRAM_MC_TEST_EN                                    (1 << 0)

// reg_0fc
#define PSRAM_MC_RB_BYPASS                                  (1 << 0)
#define PSRAM_MC_RB_SQUELCH_DR                              (1 << 1)
#define PSRAM_MC_RB_SQUELCH(n)                              (((n) & 0xF) << 2)
#define PSRAM_MC_RB_SQUELCH_MASK                            (0xF << 2)
#define PSRAM_MC_RB_SQUELCH_SHIFT                           (2)
#define PSRAM_MC_WB_SQUELCH_DR                              (1 << 6)
#define PSRAM_MC_WB_SQUELCH(n)                              (((n) & 0xF) << 7)
#define PSRAM_MC_WB_SQUELCH_MASK                            (0xF << 7)
#define PSRAM_MC_WB_SQUELCH_SHIFT                           (7)

// reg_100
#define PSRAM_MC_MGR_BUS_HALT_EN                            (1 << 0)

// reg_104
#define PSRAM_MC_MGR_AUTO_WAKEUP_EN                         (1 << 0)

// reg_108
#define PSRAM_MC_CHIP_SEL_DR                                (1 << 0)
#define PSRAM_MC_CHIP_SEL                                   (1 << 1)

// reg_140
#define PSRAM_MC_CMD_TABLE_ARRAY_RD(n)                      (((n) & 0xFF) << 0)
#define PSRAM_MC_CMD_TABLE_ARRAY_RD_MASK                    (0xFF << 0)
#define PSRAM_MC_CMD_TABLE_ARRAY_RD_SHIFT                   (0)
#define PSRAM_MC_CMD_TABLE_ARRAY_WR(n)                      (((n) & 0xFF) << 8)
#define PSRAM_MC_CMD_TABLE_ARRAY_WR_MASK                    (0xFF << 8)
#define PSRAM_MC_CMD_TABLE_ARRAY_WR_SHIFT                   (8)
#define PSRAM_MC_CMD_TABLE_REG_RD(n)                        (((n) & 0xFF) << 16)
#define PSRAM_MC_CMD_TABLE_REG_RD_MASK                      (0xFF << 16)
#define PSRAM_MC_CMD_TABLE_REG_RD_SHIFT                     (16)
#define PSRAM_MC_CMD_TABLE_REG_WR(n)                        (((n) & 0xFF) << 24)
#define PSRAM_MC_CMD_TABLE_REG_WR_MASK                      (0xFF << 24)
#define PSRAM_MC_CMD_TABLE_REG_WR_SHIFT                     (24)

// reg_150
#define PSRAM_MC_CMD_TABLE_AUTO_REFR(n)                     (((n) & 0xFF) << 0)
#define PSRAM_MC_CMD_TABLE_AUTO_REFR_MASK                   (0xFF << 0)
#define PSRAM_MC_CMD_TABLE_AUTO_REFR_SHIFT                  (0)
#define PSRAM_MC_CMD_TABLE_SELF_REFR(n)                     (((n) & 0xFF) << 8)
#define PSRAM_MC_CMD_TABLE_SELF_REFR_MASK                   (0xFF << 8)
#define PSRAM_MC_CMD_TABLE_SELF_REFR_SHIFT                  (8)
#define PSRAM_MC_CMD_TABLE_HSLP_ENTRY(n)                    (((n) & 0xFF) << 16)
#define PSRAM_MC_CMD_TABLE_HSLP_ENTRY_MASK                  (0xFF << 16)
#define PSRAM_MC_CMD_TABLE_HSLP_ENTRY_SHIFT                 (16)
#define PSRAM_MC_CMD_TABLE_GLBRST(n)                        (((n) & 0xFF) << 24)
#define PSRAM_MC_CMD_TABLE_GLBRST_MASK                      (0xFF << 24)
#define PSRAM_MC_CMD_TABLE_GLBRST_SHIFT                     (24)

// reg_160
#define PSRAM_MC_CMD_TABLE_NOP(n)                           (((n) & 0xFF) << 0)
#define PSRAM_MC_CMD_TABLE_NOP_MASK                         (0xFF << 0)
#define PSRAM_MC_CMD_TABLE_NOP_SHIFT                        (0)

// reg_164
#define PSRAM_MC_CMD_TABLE_ORDER(n)                         (((n) & 0xFF) << 0)
#define PSRAM_MC_CMD_TABLE_ORDER_MASK                       (0xFF << 0)
#define PSRAM_MC_CMD_TABLE_ORDER_SHIFT                      (0)

// reg_168
#define PSRAM_MC_CMD_SEQ_DEFINE0(n)                         (((n) & 0x3FFFFFFF) << 0)
#define PSRAM_MC_CMD_SEQ_DEFINE0_MASK                       (0x3FFFFFFF << 0)
#define PSRAM_MC_CMD_SEQ_DEFINE0_SHIFT                      (0)

// reg_16c
#define PSRAM_MC_CMD_SEQ_DEFINE1(n)                         (((n) & 0x3FF) << 0)
#define PSRAM_MC_CMD_SEQ_DEFINE1_MASK                       (0x3FF << 0)
#define PSRAM_MC_CMD_SEQ_DEFINE1_SHIFT                      (0)

// reg_180
#define PSRAM_MC_CA_MAP_BIT0(n)                             (((n) & 0x1F) << 0)
#define PSRAM_MC_CA_MAP_BIT0_MASK                           (0x1F << 0)
#define PSRAM_MC_CA_MAP_BIT0_SHIFT                          (0)
#define PSRAM_MC_CA_MAP_BIT1(n)                             (((n) & 0x1F) << 5)
#define PSRAM_MC_CA_MAP_BIT1_MASK                           (0x1F << 5)
#define PSRAM_MC_CA_MAP_BIT1_SHIFT                          (5)
#define PSRAM_MC_CA_MAP_BIT2(n)                             (((n) & 0x1F) << 10)
#define PSRAM_MC_CA_MAP_BIT2_MASK                           (0x1F << 10)
#define PSRAM_MC_CA_MAP_BIT2_SHIFT                          (10)
#define PSRAM_MC_CA_MAP_BIT3(n)                             (((n) & 0x1F) << 15)
#define PSRAM_MC_CA_MAP_BIT3_MASK                           (0x1F << 15)
#define PSRAM_MC_CA_MAP_BIT3_SHIFT                          (15)
#define PSRAM_MC_CA_MAP_BIT4(n)                             (((n) & 0x1F) << 20)
#define PSRAM_MC_CA_MAP_BIT4_MASK                           (0x1F << 20)
#define PSRAM_MC_CA_MAP_BIT4_SHIFT                          (20)
#define PSRAM_MC_CA_MAP_BIT5(n)                             (((n) & 0x1F) << 25)
#define PSRAM_MC_CA_MAP_BIT5_MASK                           (0x1F << 25)
#define PSRAM_MC_CA_MAP_BIT5_SHIFT                          (25)

// reg_184
#define PSRAM_MC_CA_MAP_BIT6(n)                             (((n) & 0x1F) << 0)
#define PSRAM_MC_CA_MAP_BIT6_MASK                           (0x1F << 0)
#define PSRAM_MC_CA_MAP_BIT6_SHIFT                          (0)
#define PSRAM_MC_CA_MAP_BIT7(n)                             (((n) & 0x1F) << 5)
#define PSRAM_MC_CA_MAP_BIT7_MASK                           (0x1F << 5)
#define PSRAM_MC_CA_MAP_BIT7_SHIFT                          (5)
#define PSRAM_MC_CA_MAP_BIT8(n)                             (((n) & 0x1F) << 10)
#define PSRAM_MC_CA_MAP_BIT8_MASK                           (0x1F << 10)
#define PSRAM_MC_CA_MAP_BIT8_SHIFT                          (10)
#define PSRAM_MC_CA_MAP_BIT9(n)                             (((n) & 0x1F) << 15)
#define PSRAM_MC_CA_MAP_BIT9_MASK                           (0x1F << 15)
#define PSRAM_MC_CA_MAP_BIT9_SHIFT                          (15)
#define PSRAM_MC_CA_MAP_BIT10(n)                            (((n) & 0x1F) << 20)
#define PSRAM_MC_CA_MAP_BIT10_MASK                          (0x1F << 20)
#define PSRAM_MC_CA_MAP_BIT10_SHIFT                         (20)
#define PSRAM_MC_CA_MAP_BIT11(n)                            (((n) & 0x1F) << 25)
#define PSRAM_MC_CA_MAP_BIT11_MASK                          (0x1F << 25)
#define PSRAM_MC_CA_MAP_BIT11_SHIFT                         (25)

// reg_188
#define PSRAM_MC_CA_MAP_BIT12(n)                            (((n) & 0x1F) << 0)
#define PSRAM_MC_CA_MAP_BIT12_MASK                          (0x1F << 0)
#define PSRAM_MC_CA_MAP_BIT12_SHIFT                         (0)
#define PSRAM_MC_CA_MAP_BIT13(n)                            (((n) & 0x1F) << 5)
#define PSRAM_MC_CA_MAP_BIT13_MASK                          (0x1F << 5)
#define PSRAM_MC_CA_MAP_BIT13_SHIFT                         (5)
#define PSRAM_MC_CA_MAP_BIT14(n)                            (((n) & 0x1F) << 10)
#define PSRAM_MC_CA_MAP_BIT14_MASK                          (0x1F << 10)
#define PSRAM_MC_CA_MAP_BIT14_SHIFT                         (10)
#define PSRAM_MC_CA_MAP_BIT15(n)                            (((n) & 0x1F) << 15)
#define PSRAM_MC_CA_MAP_BIT15_MASK                          (0x1F << 15)
#define PSRAM_MC_CA_MAP_BIT15_SHIFT                         (15)
#define PSRAM_MC_CA_MAP_BIT16(n)                            (((n) & 0x1F) << 20)
#define PSRAM_MC_CA_MAP_BIT16_MASK                          (0x1F << 20)
#define PSRAM_MC_CA_MAP_BIT16_SHIFT                         (20)
#define PSRAM_MC_CA_MAP_BIT17(n)                            (((n) & 0x1F) << 25)
#define PSRAM_MC_CA_MAP_BIT17_MASK                          (0x1F << 25)
#define PSRAM_MC_CA_MAP_BIT17_SHIFT                         (25)

// reg_18c
#define PSRAM_MC_CA_MAP_BIT18(n)                            (((n) & 0x1F) << 0)
#define PSRAM_MC_CA_MAP_BIT18_MASK                          (0x1F << 0)
#define PSRAM_MC_CA_MAP_BIT18_SHIFT                         (0)
#define PSRAM_MC_CA_MAP_BIT19(n)                            (((n) & 0x1F) << 5)
#define PSRAM_MC_CA_MAP_BIT19_MASK                          (0x1F << 5)
#define PSRAM_MC_CA_MAP_BIT19_SHIFT                         (5)
#define PSRAM_MC_CA_MAP_BIT20(n)                            (((n) & 0x1F) << 10)
#define PSRAM_MC_CA_MAP_BIT20_MASK                          (0x1F << 10)
#define PSRAM_MC_CA_MAP_BIT20_SHIFT                         (10)
#define PSRAM_MC_CA_MAP_BIT21(n)                            (((n) & 0x1F) << 15)
#define PSRAM_MC_CA_MAP_BIT21_MASK                          (0x1F << 15)
#define PSRAM_MC_CA_MAP_BIT21_SHIFT                         (15)
#define PSRAM_MC_CA_MAP_BIT22(n)                            (((n) & 0x1F) << 20)
#define PSRAM_MC_CA_MAP_BIT22_MASK                          (0x1F << 20)
#define PSRAM_MC_CA_MAP_BIT22_SHIFT                         (20)
#define PSRAM_MC_CA_MAP_BIT23(n)                            (((n) & 0x1F) << 25)
#define PSRAM_MC_CA_MAP_BIT23_MASK                          (0x1F << 25)
#define PSRAM_MC_CA_MAP_BIT23_SHIFT                         (25)

// reg_190
#define PSRAM_MC_CA_MAP_BIT24(n)                            (((n) & 0x1F) << 0)
#define PSRAM_MC_CA_MAP_BIT24_MASK                          (0x1F << 0)
#define PSRAM_MC_CA_MAP_BIT24_SHIFT                         (0)
#define PSRAM_MC_CA_MAP_BIT25(n)                            (((n) & 0x1F) << 5)
#define PSRAM_MC_CA_MAP_BIT25_MASK                          (0x1F << 5)
#define PSRAM_MC_CA_MAP_BIT25_SHIFT                         (5)
#define PSRAM_MC_CA_MAP_BIT26(n)                            (((n) & 0x1F) << 10)
#define PSRAM_MC_CA_MAP_BIT26_MASK                          (0x1F << 10)
#define PSRAM_MC_CA_MAP_BIT26_SHIFT                         (10)
#define PSRAM_MC_CA_MAP_BIT27(n)                            (((n) & 0x1F) << 15)
#define PSRAM_MC_CA_MAP_BIT27_MASK                          (0x1F << 15)
#define PSRAM_MC_CA_MAP_BIT27_SHIFT                         (15)
#define PSRAM_MC_CA_MAP_BIT28(n)                            (((n) & 0x1F) << 20)
#define PSRAM_MC_CA_MAP_BIT28_MASK                          (0x1F << 20)
#define PSRAM_MC_CA_MAP_BIT28_SHIFT                         (20)
#define PSRAM_MC_CA_MAP_BIT29(n)                            (((n) & 0x1F) << 25)
#define PSRAM_MC_CA_MAP_BIT29_MASK                          (0x1F << 25)
#define PSRAM_MC_CA_MAP_BIT29_SHIFT                         (25)

// reg_194
#define PSRAM_MC_CA_MAP_BIT30(n)                            (((n) & 0x1F) << 0)
#define PSRAM_MC_CA_MAP_BIT30_MASK                          (0x1F << 0)
#define PSRAM_MC_CA_MAP_BIT30_SHIFT                         (0)
#define PSRAM_MC_CA_MAP_BIT31(n)                            (((n) & 0x1F) << 5)
#define PSRAM_MC_CA_MAP_BIT31_MASK                          (0x1F << 5)
#define PSRAM_MC_CA_MAP_BIT31_SHIFT                         (5)
#define PSRAM_MC_CA_MAP_BIT32(n)                            (((n) & 0x1F) << 10)
#define PSRAM_MC_CA_MAP_BIT32_MASK                          (0x1F << 10)
#define PSRAM_MC_CA_MAP_BIT32_SHIFT                         (10)

// reg_198
#define PSRAM_MC_RESERVED_0(n)                              (((n) & 0xFF) << 0)
#define PSRAM_MC_RESERVED_0_MASK                            (0xFF << 0)
#define PSRAM_MC_RESERVED_0_SHIFT                           (0)

// reg_19c
#define PSRAM_MC_SW_BUS_HALT_EN                             (1 << 0)
#define PSRAM_MC_SW_BUS_HALT_STS(n)                         (((n) & 0x3) << 1)
#define PSRAM_MC_SW_BUS_HALT_STS_MASK                       (0x3 << 1)
#define PSRAM_MC_SW_BUS_HALT_STS_SHIFT                      (1)

// reg_200
#define PSRAM_MC_DYN_FREQ_SET_POINT                         (1 << 0)
#define PSRAM_MC_DYN_FREQ_MR_VLD(n)                         (((n) & 0xFF) << 1)
#define PSRAM_MC_DYN_FREQ_MR_VLD_MASK                       (0xFF << 1)
#define PSRAM_MC_DYN_FREQ_MR_VLD_SHIFT                      (1)
#define PSRAM_MC_DYN_FREQ_STS                               (1 << 9)

// reg_204
#define PSRAM_MC_DYN_FREQ_MR0_0(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MR0_0_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MR0_0_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MR1_0(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MR1_0_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MR1_0_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MR2_0(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MR2_0_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MR2_0_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MR3_0(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MR3_0_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MR3_0_SHIFT                       (24)

// reg_208
#define PSRAM_MC_DYN_FREQ_MR4_0(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MR4_0_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MR4_0_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MR5_0(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MR5_0_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MR5_0_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MR6_0(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MR6_0_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MR6_0_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MR7_0(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MR7_0_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MR7_0_SHIFT                       (24)

// reg_20c
#define PSRAM_MC_DYN_FREQ_MD0_0(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MD0_0_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MD0_0_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MD1_0(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MD1_0_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MD1_0_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MD2_0(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MD2_0_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MD2_0_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MD3_0(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MD3_0_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MD3_0_SHIFT                       (24)

// reg_210
#define PSRAM_MC_DYN_FREQ_MD4_0(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MD4_0_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MD4_0_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MD5_0(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MD5_0_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MD5_0_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MD6_0(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MD6_0_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MD6_0_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MD7_0(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MD7_0_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MD7_0_SHIFT                       (24)

// reg_214
#define PSRAM_MC_DYN_FREQ_MR0_1(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MR0_1_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MR0_1_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MR1_1(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MR1_1_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MR1_1_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MR2_1(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MR2_1_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MR2_1_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MR3_1(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MR3_1_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MR3_1_SHIFT                       (24)

// reg_218
#define PSRAM_MC_DYN_FREQ_MR4_1(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MR4_1_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MR4_1_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MR5_1(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MR5_1_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MR5_1_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MR6_1(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MR6_1_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MR6_1_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MR7_1(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MR7_1_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MR7_1_SHIFT                       (24)

// reg_21c
#define PSRAM_MC_DYN_FREQ_MD0_1(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MD0_1_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MD0_1_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MD1_1(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MD1_1_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MD1_1_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MD2_1(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MD2_1_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MD2_1_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MD3_1(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MD3_1_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MD3_1_SHIFT                       (24)

// reg_220
#define PSRAM_MC_DYN_FREQ_MD4_1(n)                          (((n) & 0xFF) << 0)
#define PSRAM_MC_DYN_FREQ_MD4_1_MASK                        (0xFF << 0)
#define PSRAM_MC_DYN_FREQ_MD4_1_SHIFT                       (0)
#define PSRAM_MC_DYN_FREQ_MD5_1(n)                          (((n) & 0xFF) << 8)
#define PSRAM_MC_DYN_FREQ_MD5_1_MASK                        (0xFF << 8)
#define PSRAM_MC_DYN_FREQ_MD5_1_SHIFT                       (8)
#define PSRAM_MC_DYN_FREQ_MD6_1(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_DYN_FREQ_MD6_1_MASK                        (0xFF << 16)
#define PSRAM_MC_DYN_FREQ_MD6_1_SHIFT                       (16)
#define PSRAM_MC_DYN_FREQ_MD7_1(n)                          (((n) & 0xFF) << 24)
#define PSRAM_MC_DYN_FREQ_MD7_1_MASK                        (0xFF << 24)
#define PSRAM_MC_DYN_FREQ_MD7_1_SHIFT                       (24)

// reg_224
#define PSRAM_MC_WRITE_LATENCY_1(n)                         (((n) & 0xFF) << 0)
#define PSRAM_MC_WRITE_LATENCY_1_MASK                       (0xFF << 0)
#define PSRAM_MC_WRITE_LATENCY_1_SHIFT                      (0)
#define PSRAM_MC_WRITE_LATENCY_MR_1(n)                      (((n) & 0xFF) << 8)
#define PSRAM_MC_WRITE_LATENCY_MR_1_MASK                    (0xFF << 8)
#define PSRAM_MC_WRITE_LATENCY_MR_1_SHIFT                   (8)
#define PSRAM_MC_READ_LATENCY_1(n)                          (((n) & 0xFF) << 16)
#define PSRAM_MC_READ_LATENCY_1_MASK                        (0xFF << 16)
#define PSRAM_MC_READ_LATENCY_1_SHIFT                       (16)
#define PSRAM_MC_READ_LATENCY1_1(n)                         (((n) & 0xFF) << 24)
#define PSRAM_MC_READ_LATENCY1_1_MASK                       (0xFF << 24)
#define PSRAM_MC_READ_LATENCY1_1_SHIFT                      (24)

// reg_228
#define PSRAM_MC_CP_WRAP_EN_1                               (1 << 0)
#define PSRAM_MC_BURST_LENGTH_1(n)                          (((n) & 0x7) << 1)
#define PSRAM_MC_BURST_LENGTH_1_MASK                        (0x7 << 1)
#define PSRAM_MC_BURST_LENGTH_1_SHIFT                       (1)

// reg_22c
#define PSRAM_MC_T_REFI_1(n)                                (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_REFI_1_MASK                              (0xFFFF << 0)
#define PSRAM_MC_T_REFI_1_SHIFT                             (0)

// reg_230
#define PSRAM_MC_T_RC_1(n)                                  (((n) & 0xFF) << 0)
#define PSRAM_MC_T_RC_1_MASK                                (0xFF << 0)
#define PSRAM_MC_T_RC_1_SHIFT                               (0)

// reg_234
#define PSRAM_MC_T_RFC_1(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_T_RFC_1_MASK                               (0xFF << 0)
#define PSRAM_MC_T_RFC_1_SHIFT                              (0)

// reg_238
#define PSRAM_MC_T_CPHR_1(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHR_1_MASK                              (0x3F << 0)
#define PSRAM_MC_T_CPHR_1_SHIFT                             (0)

// reg_23c
#define PSRAM_MC_T_CPHR_AP_1(n)                             (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHR_AP_1_MASK                           (0x3F << 0)
#define PSRAM_MC_T_CPHR_AP_1_SHIFT                          (0)

// reg_240
#define PSRAM_MC_T_CPHW_1(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHW_1_MASK                              (0x3F << 0)
#define PSRAM_MC_T_CPHW_1_SHIFT                             (0)

// reg_244
#define PSRAM_MC_T_CPHW_AP_1(n)                             (((n) & 0x3F) << 0)
#define PSRAM_MC_T_CPHW_AP_1_MASK                           (0x3F << 0)
#define PSRAM_MC_T_CPHW_AP_1_SHIFT                          (0)

// reg_248
#define PSRAM_MC_T_MRR_1(n)                                 (((n) & 0x3F) << 0)
#define PSRAM_MC_T_MRR_1_MASK                               (0x3F << 0)
#define PSRAM_MC_T_MRR_1_SHIFT                              (0)

// reg_24c
#define PSRAM_MC_T_MRS_1(n)                                 (((n) & 0x3FF) << 0)
#define PSRAM_MC_T_MRS_1_MASK                               (0x3FF << 0)
#define PSRAM_MC_T_MRS_1_SHIFT                              (0)

// reg_250
#define PSRAM_MC_T_CEM_1(n)                                 (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_CEM_1_MASK                               (0xFFFF << 0)
#define PSRAM_MC_T_CEM_1_SHIFT                              (0)

// reg_254
#define PSRAM_MC_T_RST_1(n)                                 (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_RST_1_MASK                               (0xFFFF << 0)
#define PSRAM_MC_T_RST_1_SHIFT                              (0)

// reg_258
#define PSRAM_MC_T_SRF_1(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_T_SRF_1_MASK                               (0xFF << 0)
#define PSRAM_MC_T_SRF_1_SHIFT                              (0)

// reg_25c
#define PSRAM_MC_T_XSR_1(n)                                 (((n) & 0xFF) << 0)
#define PSRAM_MC_T_XSR_1_MASK                               (0xFF << 0)
#define PSRAM_MC_T_XSR_1_SHIFT                              (0)

// reg_260
#define PSRAM_MC_T_HS_1(n)                                  (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_HS_1_MASK                                (0xFFFF << 0)
#define PSRAM_MC_T_HS_1_SHIFT                               (0)

// reg_264
#define PSRAM_MC_T_XPHS_1(n)                                (((n) & 0xFF) << 0)
#define PSRAM_MC_T_XPHS_1_MASK                              (0xFF << 0)
#define PSRAM_MC_T_XPHS_1_SHIFT                             (0)

// reg_268
#define PSRAM_MC_T_XHS_1(n)                                 (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_XHS_1_MASK                               (0xFFFFF << 0)
#define PSRAM_MC_T_XHS_1_SHIFT                              (0)

// reg_26c
#define PSRAM_MC_T_ZQCAL_1(n)                               (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_ZQCAL_1_MASK                             (0xFFFFF << 0)
#define PSRAM_MC_T_ZQCAL_1_SHIFT                            (0)

// reg_270
#define PSRAM_MC_T_ZQCRST_1(n)                              (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_ZQCRST_1_MASK                            (0xFFFFF << 0)
#define PSRAM_MC_T_ZQCRST_1_SHIFT                           (0)

// reg_274
#define PSRAM_MC_T_XCKD_1(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_XCKD_1_MASK                              (0x3F << 0)
#define PSRAM_MC_T_XCKD_1_SHIFT                             (0)

// reg_278
#define PSRAM_MC_T_ECKD_1(n)                                (((n) & 0x3F) << 0)
#define PSRAM_MC_T_ECKD_1_MASK                              (0x3F << 0)
#define PSRAM_MC_T_ECKD_1_SHIFT                             (0)

// reg_27c
#define PSRAM_MC_T_ZQCAS_1(n)                               (((n) & 0xFFFFF) << 0)
#define PSRAM_MC_T_ZQCAS_1_MASK                             (0xFFFFF << 0)
#define PSRAM_MC_T_ZQCAS_1_SHIFT                            (0)

// reg_280
#define PSRAM_MC_T_REF_WIN_1(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_T_REF_WIN_1_MASK                           (0xFFFFFFFF << 0)
#define PSRAM_MC_T_REF_WIN_1_SHIFT                          (0)

// reg_284
#define PSRAM_MC_T_CSCKE_1(n)                               (((n) & 0xFFFF) << 0)
#define PSRAM_MC_T_CSCKE_1_MASK                             (0xFFFF << 0)
#define PSRAM_MC_T_CSCKE_1_SHIFT                            (0)
#define PSRAM_MC_T_CSCKEH_1(n)                              (((n) & 0xFFFF) << 16)
#define PSRAM_MC_T_CSCKEH_1_MASK                            (0xFFFF << 16)
#define PSRAM_MC_T_CSCKEH_1_SHIFT                           (16)

// reg_288
#define PSRAM_MC_WR_QOS0_PRIORITY(n)                        (((n) & 0xF) << 0)
#define PSRAM_MC_WR_QOS0_PRIORITY_MASK                      (0xF << 0)
#define PSRAM_MC_WR_QOS0_PRIORITY_SHIFT                     (0)
#define PSRAM_MC_WR_QOS1_PRIORITY(n)                        (((n) & 0xF) << 4)
#define PSRAM_MC_WR_QOS1_PRIORITY_MASK                      (0xF << 4)
#define PSRAM_MC_WR_QOS1_PRIORITY_SHIFT                     (4)
#define PSRAM_MC_WR_QOS2_PRIORITY(n)                        (((n) & 0xF) << 8)
#define PSRAM_MC_WR_QOS2_PRIORITY_MASK                      (0xF << 8)
#define PSRAM_MC_WR_QOS2_PRIORITY_SHIFT                     (8)
#define PSRAM_MC_WR_QOS3_PRIORITY(n)                        (((n) & 0xF) << 12)
#define PSRAM_MC_WR_QOS3_PRIORITY_MASK                      (0xF << 12)
#define PSRAM_MC_WR_QOS3_PRIORITY_SHIFT                     (12)
#define PSRAM_MC_WR_QOS4_PRIORITY(n)                        (((n) & 0xF) << 16)
#define PSRAM_MC_WR_QOS4_PRIORITY_MASK                      (0xF << 16)
#define PSRAM_MC_WR_QOS4_PRIORITY_SHIFT                     (16)
#define PSRAM_MC_WR_QOS5_PRIORITY(n)                        (((n) & 0xF) << 20)
#define PSRAM_MC_WR_QOS5_PRIORITY_MASK                      (0xF << 20)
#define PSRAM_MC_WR_QOS5_PRIORITY_SHIFT                     (20)
#define PSRAM_MC_WR_QOS6_PRIORITY(n)                        (((n) & 0xF) << 24)
#define PSRAM_MC_WR_QOS6_PRIORITY_MASK                      (0xF << 24)
#define PSRAM_MC_WR_QOS6_PRIORITY_SHIFT                     (24)
#define PSRAM_MC_WR_QOS7_PRIORITY(n)                        (((n) & 0xF) << 28)
#define PSRAM_MC_WR_QOS7_PRIORITY_MASK                      (0xF << 28)
#define PSRAM_MC_WR_QOS7_PRIORITY_SHIFT                     (28)

// reg_28c
#define PSRAM_MC_RD_QOS0_PRIORITY(n)                        (((n) & 0xF) << 0)
#define PSRAM_MC_RD_QOS0_PRIORITY_MASK                      (0xF << 0)
#define PSRAM_MC_RD_QOS0_PRIORITY_SHIFT                     (0)
#define PSRAM_MC_RD_QOS1_PRIORITY(n)                        (((n) & 0xF) << 4)
#define PSRAM_MC_RD_QOS1_PRIORITY_MASK                      (0xF << 4)
#define PSRAM_MC_RD_QOS1_PRIORITY_SHIFT                     (4)
#define PSRAM_MC_RD_QOS2_PRIORITY(n)                        (((n) & 0xF) << 8)
#define PSRAM_MC_RD_QOS2_PRIORITY_MASK                      (0xF << 8)
#define PSRAM_MC_RD_QOS2_PRIORITY_SHIFT                     (8)
#define PSRAM_MC_RD_QOS3_PRIORITY(n)                        (((n) & 0xF) << 12)
#define PSRAM_MC_RD_QOS3_PRIORITY_MASK                      (0xF << 12)
#define PSRAM_MC_RD_QOS3_PRIORITY_SHIFT                     (12)
#define PSRAM_MC_RD_QOS4_PRIORITY(n)                        (((n) & 0xF) << 16)
#define PSRAM_MC_RD_QOS4_PRIORITY_MASK                      (0xF << 16)
#define PSRAM_MC_RD_QOS4_PRIORITY_SHIFT                     (16)
#define PSRAM_MC_RD_QOS5_PRIORITY(n)                        (((n) & 0xF) << 20)
#define PSRAM_MC_RD_QOS5_PRIORITY_MASK                      (0xF << 20)
#define PSRAM_MC_RD_QOS5_PRIORITY_SHIFT                     (20)
#define PSRAM_MC_RD_QOS6_PRIORITY(n)                        (((n) & 0xF) << 24)
#define PSRAM_MC_RD_QOS6_PRIORITY_MASK                      (0xF << 24)
#define PSRAM_MC_RD_QOS6_PRIORITY_SHIFT                     (24)
#define PSRAM_MC_RD_QOS7_PRIORITY(n)                        (((n) & 0xF) << 28)
#define PSRAM_MC_RD_QOS7_PRIORITY_MASK                      (0xF << 28)
#define PSRAM_MC_RD_QOS7_PRIORITY_SHIFT                     (28)

// reg_290
#define PSRAM_MC_WRITE_FILL_PRIORITY_1_16THS(n)             (((n) & 0xF) << 0)
#define PSRAM_MC_WRITE_FILL_PRIORITY_1_16THS_MASK           (0xF << 0)
#define PSRAM_MC_WRITE_FILL_PRIORITY_1_16THS_SHIFT          (0)
#define PSRAM_MC_WRITE_FILL_PRIORITY_2_16THS(n)             (((n) & 0xF) << 4)
#define PSRAM_MC_WRITE_FILL_PRIORITY_2_16THS_MASK           (0xF << 4)
#define PSRAM_MC_WRITE_FILL_PRIORITY_2_16THS_SHIFT          (4)
#define PSRAM_MC_WRITE_FILL_PRIORITY_3_16THS(n)             (((n) & 0xF) << 8)
#define PSRAM_MC_WRITE_FILL_PRIORITY_3_16THS_MASK           (0xF << 8)
#define PSRAM_MC_WRITE_FILL_PRIORITY_3_16THS_SHIFT          (8)
#define PSRAM_MC_WRITE_FILL_PRIORITY_4_16THS(n)             (((n) & 0xF) << 12)
#define PSRAM_MC_WRITE_FILL_PRIORITY_4_16THS_MASK           (0xF << 12)
#define PSRAM_MC_WRITE_FILL_PRIORITY_4_16THS_SHIFT          (12)
#define PSRAM_MC_WRITE_FILL_PRIORITY_5_16THS(n)             (((n) & 0xF) << 16)
#define PSRAM_MC_WRITE_FILL_PRIORITY_5_16THS_MASK           (0xF << 16)
#define PSRAM_MC_WRITE_FILL_PRIORITY_5_16THS_SHIFT          (16)
#define PSRAM_MC_WRITE_FILL_PRIORITY_6_16THS(n)             (((n) & 0xF) << 20)
#define PSRAM_MC_WRITE_FILL_PRIORITY_6_16THS_MASK           (0xF << 20)
#define PSRAM_MC_WRITE_FILL_PRIORITY_6_16THS_SHIFT          (20)
#define PSRAM_MC_WRITE_FILL_PRIORITY_7_16THS(n)             (((n) & 0xF) << 24)
#define PSRAM_MC_WRITE_FILL_PRIORITY_7_16THS_MASK           (0xF << 24)
#define PSRAM_MC_WRITE_FILL_PRIORITY_7_16THS_SHIFT          (24)

// reg_294
#define PSRAM_MC_RESERVED_294(n)                            (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_RESERVED_294_MASK                          (0xFFFFFFFF << 0)
#define PSRAM_MC_RESERVED_294_SHIFT                         (0)

// reg_298
#define PSRAM_MC_READ_FILL_PRIORITY_1_16THS(n)              (((n) & 0xF) << 0)
#define PSRAM_MC_READ_FILL_PRIORITY_1_16THS_MASK            (0xF << 0)
#define PSRAM_MC_READ_FILL_PRIORITY_1_16THS_SHIFT           (0)
#define PSRAM_MC_READ_FILL_PRIORITY_2_16THS(n)              (((n) & 0xF) << 4)
#define PSRAM_MC_READ_FILL_PRIORITY_2_16THS_MASK            (0xF << 4)
#define PSRAM_MC_READ_FILL_PRIORITY_2_16THS_SHIFT           (4)
#define PSRAM_MC_READ_FILL_PRIORITY_3_16THS(n)              (((n) & 0xF) << 8)
#define PSRAM_MC_READ_FILL_PRIORITY_3_16THS_MASK            (0xF << 8)
#define PSRAM_MC_READ_FILL_PRIORITY_3_16THS_SHIFT           (8)
#define PSRAM_MC_READ_FILL_PRIORITY_4_16THS(n)              (((n) & 0xF) << 12)
#define PSRAM_MC_READ_FILL_PRIORITY_4_16THS_MASK            (0xF << 12)
#define PSRAM_MC_READ_FILL_PRIORITY_4_16THS_SHIFT           (12)
#define PSRAM_MC_READ_FILL_PRIORITY_5_16THS(n)              (((n) & 0xF) << 16)
#define PSRAM_MC_READ_FILL_PRIORITY_5_16THS_MASK            (0xF << 16)
#define PSRAM_MC_READ_FILL_PRIORITY_5_16THS_SHIFT           (16)
#define PSRAM_MC_READ_FILL_PRIORITY_6_16THS(n)              (((n) & 0xF) << 20)
#define PSRAM_MC_READ_FILL_PRIORITY_6_16THS_MASK            (0xF << 20)
#define PSRAM_MC_READ_FILL_PRIORITY_6_16THS_SHIFT           (20)
#define PSRAM_MC_READ_FILL_PRIORITY_7_16THS(n)              (((n) & 0xF) << 24)
#define PSRAM_MC_READ_FILL_PRIORITY_7_16THS_MASK            (0xF << 24)
#define PSRAM_MC_READ_FILL_PRIORITY_7_16THS_SHIFT           (24)

// reg_29c
#define PSRAM_MC_TIMEOUT_PRESCALAR_0(n)                     (((n) & 0x3) << 0)
#define PSRAM_MC_TIMEOUT_PRESCALAR_0_MASK                   (0x3 << 0)
#define PSRAM_MC_TIMEOUT_PRESCALAR_0_SHIFT                  (0)
#define PSRAM_MC_TIMEOUT_PRESCALAR_1(n)                     (((n) & 0x3) << 2)
#define PSRAM_MC_TIMEOUT_PRESCALAR_1_MASK                   (0x3 << 2)
#define PSRAM_MC_TIMEOUT_PRESCALAR_1_SHIFT                  (2)
#define PSRAM_MC_TIMEOUT_ESCALATE_EN_0                      (1 << 4)
#define PSRAM_MC_TIMEOUT_ESCALATE_EN_1                      (1 << 5)

// reg_2a0
#define PSRAM_MC_WRITE_THRESHOLD_ENABLE                     (1 << 0)
#define PSRAM_MC_READ_THRESHOLD_ENABLE                      (1 << 1)

// reg_2a4
#define PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_0(n)              (((n) & 0xF) << 0)
#define PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_0_MASK            (0xF << 0)
#define PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_0_SHIFT           (0)
#define PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_1(n)              (((n) & 0xF) << 4)
#define PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_1_MASK            (0xF << 4)
#define PSRAM_MC_RD_PREFETCH_TOKEN_TARGET_1_SHIFT           (4)
#define PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_0(n)              (((n) & 0xF) << 8)
#define PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_0_MASK            (0xF << 8)
#define PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_0_SHIFT           (8)
#define PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_1(n)              (((n) & 0xF) << 12)
#define PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_1_MASK            (0xF << 12)
#define PSRAM_MC_WR_PREFETCH_TOKEN_TARGET_1_SHIFT           (12)

// reg_2a8
#define PSRAM_MC_WVQOS0_DR_EN                               (1 << 0)
#define PSRAM_MC_WVQOS0(n)                                  (((n) & 0xF) << 1)
#define PSRAM_MC_WVQOS0_MASK                                (0xF << 1)
#define PSRAM_MC_WVQOS0_SHIFT                               (1)
#define PSRAM_MC_WVQOS1_DR_EN                               (1 << 5)
#define PSRAM_MC_WVQOS1(n)                                  (((n) & 0xF) << 6)
#define PSRAM_MC_WVQOS1_MASK                                (0xF << 6)
#define PSRAM_MC_WVQOS1_SHIFT                               (6)
#define PSRAM_MC_RVQOS0_DR_EN                               (1 << 10)
#define PSRAM_MC_RVQOS0(n)                                  (((n) & 0xF) << 11)
#define PSRAM_MC_RVQOS0_MASK                                (0xF << 11)
#define PSRAM_MC_RVQOS0_SHIFT                               (11)
#define PSRAM_MC_RVQOS1_DR_EN                               (1 << 15)
#define PSRAM_MC_RVQOS1(n)                                  (((n) & 0xF) << 16)
#define PSRAM_MC_RVQOS1_MASK                                (0xF << 16)
#define PSRAM_MC_RVQOS1_SHIFT                               (16)

// reg_2ac
#define PSRAM_MC_WR_QOS8_PRIORITY(n)                        (((n) & 0xF) << 0)
#define PSRAM_MC_WR_QOS8_PRIORITY_MASK                      (0xF << 0)
#define PSRAM_MC_WR_QOS8_PRIORITY_SHIFT                     (0)
#define PSRAM_MC_WR_QOS9_PRIORITY(n)                        (((n) & 0xF) << 4)
#define PSRAM_MC_WR_QOS9_PRIORITY_MASK                      (0xF << 4)
#define PSRAM_MC_WR_QOS9_PRIORITY_SHIFT                     (4)
#define PSRAM_MC_WR_QOS10_PRIORITY(n)                       (((n) & 0xF) << 8)
#define PSRAM_MC_WR_QOS10_PRIORITY_MASK                     (0xF << 8)
#define PSRAM_MC_WR_QOS10_PRIORITY_SHIFT                    (8)
#define PSRAM_MC_WR_QOS11_PRIORITY(n)                       (((n) & 0xF) << 12)
#define PSRAM_MC_WR_QOS11_PRIORITY_MASK                     (0xF << 12)
#define PSRAM_MC_WR_QOS11_PRIORITY_SHIFT                    (12)
#define PSRAM_MC_WR_QOS12_PRIORITY(n)                       (((n) & 0xF) << 16)
#define PSRAM_MC_WR_QOS12_PRIORITY_MASK                     (0xF << 16)
#define PSRAM_MC_WR_QOS12_PRIORITY_SHIFT                    (16)
#define PSRAM_MC_WR_QOS13_PRIORITY(n)                       (((n) & 0xF) << 20)
#define PSRAM_MC_WR_QOS13_PRIORITY_MASK                     (0xF << 20)
#define PSRAM_MC_WR_QOS13_PRIORITY_SHIFT                    (20)
#define PSRAM_MC_WR_QOS14_PRIORITY(n)                       (((n) & 0xF) << 24)
#define PSRAM_MC_WR_QOS14_PRIORITY_MASK                     (0xF << 24)
#define PSRAM_MC_WR_QOS14_PRIORITY_SHIFT                    (24)
#define PSRAM_MC_WR_QOS15_PRIORITY(n)                       (((n) & 0xF) << 28)
#define PSRAM_MC_WR_QOS15_PRIORITY_MASK                     (0xF << 28)
#define PSRAM_MC_WR_QOS15_PRIORITY_SHIFT                    (28)

// reg_2b0
#define PSRAM_MC_RD_QOS8_PRIORITY(n)                        (((n) & 0xF) << 0)
#define PSRAM_MC_RD_QOS8_PRIORITY_MASK                      (0xF << 0)
#define PSRAM_MC_RD_QOS8_PRIORITY_SHIFT                     (0)
#define PSRAM_MC_RD_QOS9_PRIORITY(n)                        (((n) & 0xF) << 4)
#define PSRAM_MC_RD_QOS9_PRIORITY_MASK                      (0xF << 4)
#define PSRAM_MC_RD_QOS9_PRIORITY_SHIFT                     (4)
#define PSRAM_MC_RD_QOS10_PRIORITY(n)                       (((n) & 0xF) << 8)
#define PSRAM_MC_RD_QOS10_PRIORITY_MASK                     (0xF << 8)
#define PSRAM_MC_RD_QOS10_PRIORITY_SHIFT                    (8)
#define PSRAM_MC_RD_QOS11_PRIORITY(n)                       (((n) & 0xF) << 12)
#define PSRAM_MC_RD_QOS11_PRIORITY_MASK                     (0xF << 12)
#define PSRAM_MC_RD_QOS11_PRIORITY_SHIFT                    (12)
#define PSRAM_MC_RD_QOS12_PRIORITY(n)                       (((n) & 0xF) << 16)
#define PSRAM_MC_RD_QOS12_PRIORITY_MASK                     (0xF << 16)
#define PSRAM_MC_RD_QOS12_PRIORITY_SHIFT                    (16)
#define PSRAM_MC_RD_QOS13_PRIORITY(n)                       (((n) & 0xF) << 20)
#define PSRAM_MC_RD_QOS13_PRIORITY_MASK                     (0xF << 20)
#define PSRAM_MC_RD_QOS13_PRIORITY_SHIFT                    (20)
#define PSRAM_MC_RD_QOS14_PRIORITY(n)                       (((n) & 0xF) << 24)
#define PSRAM_MC_RD_QOS14_PRIORITY_MASK                     (0xF << 24)
#define PSRAM_MC_RD_QOS14_PRIORITY_SHIFT                    (24)
#define PSRAM_MC_RD_QOS15_PRIORITY(n)                       (((n) & 0xF) << 28)
#define PSRAM_MC_RD_QOS15_PRIORITY_MASK                     (0xF << 28)
#define PSRAM_MC_RD_QOS15_PRIORITY_SHIFT                    (28)

// reg_2b4
#define PSRAM_MC_READ_FILL_PRIORITY_8_16THS(n)              (((n) & 0xF) << 0)
#define PSRAM_MC_READ_FILL_PRIORITY_8_16THS_MASK            (0xF << 0)
#define PSRAM_MC_READ_FILL_PRIORITY_8_16THS_SHIFT           (0)
#define PSRAM_MC_READ_FILL_PRIORITY_9_16THS(n)              (((n) & 0xF) << 4)
#define PSRAM_MC_READ_FILL_PRIORITY_9_16THS_MASK            (0xF << 4)
#define PSRAM_MC_READ_FILL_PRIORITY_9_16THS_SHIFT           (4)
#define PSRAM_MC_READ_FILL_PRIORITY_10_16THS(n)             (((n) & 0xF) << 8)
#define PSRAM_MC_READ_FILL_PRIORITY_10_16THS_MASK           (0xF << 8)
#define PSRAM_MC_READ_FILL_PRIORITY_10_16THS_SHIFT          (8)
#define PSRAM_MC_READ_FILL_PRIORITY_11_16THS(n)             (((n) & 0xF) << 12)
#define PSRAM_MC_READ_FILL_PRIORITY_11_16THS_MASK           (0xF << 12)
#define PSRAM_MC_READ_FILL_PRIORITY_11_16THS_SHIFT          (12)
#define PSRAM_MC_READ_FILL_PRIORITY_12_16THS(n)             (((n) & 0xF) << 16)
#define PSRAM_MC_READ_FILL_PRIORITY_12_16THS_MASK           (0xF << 16)
#define PSRAM_MC_READ_FILL_PRIORITY_12_16THS_SHIFT          (16)
#define PSRAM_MC_READ_FILL_PRIORITY_13_16THS(n)             (((n) & 0xF) << 20)
#define PSRAM_MC_READ_FILL_PRIORITY_13_16THS_MASK           (0xF << 20)
#define PSRAM_MC_READ_FILL_PRIORITY_13_16THS_SHIFT          (20)
#define PSRAM_MC_READ_FILL_PRIORITY_14_16THS(n)             (((n) & 0xF) << 24)
#define PSRAM_MC_READ_FILL_PRIORITY_14_16THS_MASK           (0xF << 24)
#define PSRAM_MC_READ_FILL_PRIORITY_14_16THS_SHIFT          (24)
#define PSRAM_MC_READ_FILL_PRIORITY_15_16THS(n)             (((n) & 0xF) << 28)
#define PSRAM_MC_READ_FILL_PRIORITY_15_16THS_MASK           (0xF << 28)
#define PSRAM_MC_READ_FILL_PRIORITY_15_16THS_SHIFT          (28)

// reg_2b8
#define PSRAM_MC_WRITE_FILL_PRIORITY_8_16THS(n)             (((n) & 0xF) << 0)
#define PSRAM_MC_WRITE_FILL_PRIORITY_8_16THS_MASK           (0xF << 0)
#define PSRAM_MC_WRITE_FILL_PRIORITY_8_16THS_SHIFT          (0)
#define PSRAM_MC_WRITE_FILL_PRIORITY_9_16THS(n)             (((n) & 0xF) << 4)
#define PSRAM_MC_WRITE_FILL_PRIORITY_9_16THS_MASK           (0xF << 4)
#define PSRAM_MC_WRITE_FILL_PRIORITY_9_16THS_SHIFT          (4)
#define PSRAM_MC_WRITE_FILL_PRIORITY_10_16THS(n)            (((n) & 0xF) << 8)
#define PSRAM_MC_WRITE_FILL_PRIORITY_10_16THS_MASK          (0xF << 8)
#define PSRAM_MC_WRITE_FILL_PRIORITY_10_16THS_SHIFT         (8)
#define PSRAM_MC_WRITE_FILL_PRIORITY_11_16THS(n)            (((n) & 0xF) << 12)
#define PSRAM_MC_WRITE_FILL_PRIORITY_11_16THS_MASK          (0xF << 12)
#define PSRAM_MC_WRITE_FILL_PRIORITY_11_16THS_SHIFT         (12)
#define PSRAM_MC_WRITE_FILL_PRIORITY_12_16THS(n)            (((n) & 0xF) << 16)
#define PSRAM_MC_WRITE_FILL_PRIORITY_12_16THS_MASK          (0xF << 16)
#define PSRAM_MC_WRITE_FILL_PRIORITY_12_16THS_SHIFT         (16)
#define PSRAM_MC_WRITE_FILL_PRIORITY_13_16THS(n)            (((n) & 0xF) << 20)
#define PSRAM_MC_WRITE_FILL_PRIORITY_13_16THS_MASK          (0xF << 20)
#define PSRAM_MC_WRITE_FILL_PRIORITY_13_16THS_SHIFT         (20)
#define PSRAM_MC_WRITE_FILL_PRIORITY_14_16THS(n)            (((n) & 0xF) << 24)
#define PSRAM_MC_WRITE_FILL_PRIORITY_14_16THS_MASK          (0xF << 24)
#define PSRAM_MC_WRITE_FILL_PRIORITY_14_16THS_SHIFT         (24)
#define PSRAM_MC_WRITE_FILL_PRIORITY_15_16THS(n)            (((n) & 0xF) << 28)
#define PSRAM_MC_WRITE_FILL_PRIORITY_15_16THS_MASK          (0xF << 28)
#define PSRAM_MC_WRITE_FILL_PRIORITY_15_16THS_SHIFT         (28)

// reg_400
#define PSRAM_MC_INIT_COMPLETE                              (1 << 0)

// reg_404
#define PSRAM_MC_BUSY                                       (1 << 0)
#define PSRAM_MC_MGR_RXFIFO_R_EMPTY                         (1 << 1)
#define PSRAM_MC_MGR_RXFIFO_FULL_CNT(n)                     (((n) & 0xF) << 2)
#define PSRAM_MC_MGR_RXFIFO_FULL_CNT_MASK                   (0xF << 2)
#define PSRAM_MC_MGR_RXFIFO_FULL_CNT_SHIFT                  (2)
#define PSRAM_MC_MGR_TXFIFO_W_FULL                          (1 << 6)
#define PSRAM_MC_MGR_TXFIFO_EMPTY_CNT(n)                    (((n) & 0xF) << 7)
#define PSRAM_MC_MGR_TXFIFO_EMPTY_CNT_MASK                  (0xF << 7)
#define PSRAM_MC_MGR_TXFIFO_EMPTY_CNT_SHIFT                 (7)
#define PSRAM_MC_WB_FILL_LEVEL(n)                           (((n) & 0x1F) << 11)
#define PSRAM_MC_WB_FILL_LEVEL_MASK                         (0x1F << 11)
#define PSRAM_MC_WB_FILL_LEVEL_SHIFT                        (11)
#define PSRAM_MC_CP_FSM_STATE(n)                            (((n) & 0xF) << 16)
#define PSRAM_MC_CP_FSM_STATE_MASK                          (0xF << 16)
#define PSRAM_MC_CP_FSM_STATE_SHIFT                         (16)
#define PSRAM_MC_MGR_FSM(n)                                 (((n) & 0xF) << 20)
#define PSRAM_MC_MGR_FSM_MASK                               (0xF << 20)
#define PSRAM_MC_MGR_FSM_SHIFT                              (20)
#define PSRAM_MC_WB_EMPTY                                   (1 << 24)
#define PSRAM_MC_RB_EMPTY                                   (1 << 25)
#define PSRAM_MC_LB_TEST_RESULT                             (1 << 26)

// reg_408
#define PSRAM_MC_RD_ADDR_STS(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_RD_ADDR_STS_MASK                           (0xFFFFFFFF << 0)
#define PSRAM_MC_RD_ADDR_STS_SHIFT                          (0)

// reg_440
#define PSRAM_MC_PMU_MONITOR_START                          (1 << 0)
#define PSRAM_MC_PMU_MONITOR_END                            (1 << 1)
#define PSRAM_MC_PMU_MONITOR_STS                            (1 << 2)

// reg_444
#define PSRAM_MC_PMU_TOL_MON_CLK_CYCLE0(n)                  (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_MON_CLK_CYCLE0_MASK                (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_MON_CLK_CYCLE0_SHIFT               (0)

// reg_448
#define PSRAM_MC_PMU_TOL_MON_CLK_CYCLE1(n)                  (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_MON_CLK_CYCLE1_MASK                (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_MON_CLK_CYCLE1_SHIFT               (0)

// reg_44c
#define PSRAM_MC_PMU_TOL_WR_DATA_BYTES0(n)                  (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_WR_DATA_BYTES0_MASK                (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_WR_DATA_BYTES0_SHIFT               (0)

// reg_450
#define PSRAM_MC_PMU_TOL_WR_DATA_BYTES1(n)                  (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_WR_DATA_BYTES1_MASK                (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_WR_DATA_BYTES1_SHIFT               (0)

// reg_454
#define PSRAM_MC_PMU_TOL_RD_DATA_BYTES0(n)                  (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_DATA_BYTES0_MASK                (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_DATA_BYTES0_SHIFT               (0)

// reg_458
#define PSRAM_MC_PMU_TOL_RD_DATA_BYTES1(n)                  (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_DATA_BYTES1_MASK                (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_DATA_BYTES1_SHIFT               (0)

// reg_45c
#define PSRAM_MC_PMU_TOL_RD_ACC_LATENCY0(n)                 (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_LATENCY0_MASK               (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_LATENCY0_SHIFT              (0)

// reg_460
#define PSRAM_MC_PMU_TOL_RD_ACC_LATENCY1(n)                 (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_LATENCY1_MASK               (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_LATENCY1_SHIFT              (0)

// reg_464
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM0(n)                     (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM0_MASK                   (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM0_SHIFT                  (0)

// reg_468
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM1(n)                     (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM1_MASK                   (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM1_SHIFT                  (0)

// reg_46c
#define PSRAM_MC_PMU_MAX_RD_ACC_LATENCY(n)                  (((n) & 0xFFFF) << 0)
#define PSRAM_MC_PMU_MAX_RD_ACC_LATENCY_MASK                (0xFFFF << 0)
#define PSRAM_MC_PMU_MAX_RD_ACC_LATENCY_SHIFT               (0)

// reg_470
#define PSRAM_MC_PMU_TOL_WR_ACC_NUM_TO_DIE(n)               (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_WR_ACC_NUM_TO_DIE_MASK             (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_WR_ACC_NUM_TO_DIE_SHIFT            (0)

// reg_474
#define PSRAM_MC_PMU_TOL_WR_CYCS_IN_MCLK_0(n)               (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_WR_CYCS_IN_MCLK_0_MASK             (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_WR_CYCS_IN_MCLK_0_SHIFT            (0)

// reg_478
#define PSRAM_MC_PMU_TOL_WR_CYCS_IN_MCLK_1(n)               (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_WR_CYCS_IN_MCLK_1_MASK             (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_WR_CYCS_IN_MCLK_1_SHIFT            (0)

// reg_47c
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM_TO_DIE(n)               (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM_TO_DIE_MASK             (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_ACC_NUM_TO_DIE_SHIFT            (0)

// reg_480
#define PSRAM_MC_PMU_TOL_RD_CYCS_IN_MCLK_0(n)               (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_CYCS_IN_MCLK_0_MASK             (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_CYCS_IN_MCLK_0_SHIFT            (0)

// reg_484
#define PSRAM_MC_PMU_TOL_RD_CYCS_IN_MCLK_1(n)               (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_PMU_TOL_RD_CYCS_IN_MCLK_1_MASK             (0xFFFFFFFF << 0)
#define PSRAM_MC_PMU_TOL_RD_CYCS_IN_MCLK_1_SHIFT            (0)

// reg_4d4
#define PSRAM_MC_WRITE_UNLOCK_CLR                           (1 << 0)

// reg_4d8
#define PSRAM_MC_WRITE_UNLOCK_SET                           (1 << 0)

// reg_7f0
#define PSRAM_MC_VERSION(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_MC_VERSION_MASK                               (0xFFFFFFFF << 0)
#define PSRAM_MC_VERSION_SHIFT                              (0)

#endif /* __REG_PSRAM_MC_V12_H__ */
