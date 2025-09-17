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

#ifndef __REG_APSRAM_CTRL_H__
#define __REG_APSRAM_CTRL_H__

#include "plat_types.h"

struct APSRAM_CTRL_T {
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
    __IO uint32_t REG_RESERVED_0C8[0xE];
    __IO uint32_t REG_100;
    __IO uint32_t REG_104;
    __IO uint32_t REG_108;
    __IO uint32_t REG_RESERVED_10C[0x3D];
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
};

struct APSRAM_SCAN_T {
    __IO uint32_t REG_RESERVED_00[0x80];
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
};

// reg_000
#define APSRAM_CTRL_MEMORY_WIDTH(n)                         (((n) & 0x3) << 0)
#define APSRAM_CTRL_MEMORY_WIDTH_MASK                       (0x3 << 0)
#define APSRAM_CTRL_MEMORY_WIDTH_SHIFT                      (0)
#define APSRAM_CTRL_BURST_LENGTH(n)                         (((n) & 0x7) << 2)
#define APSRAM_CTRL_BURST_LENGTH_MASK                       (0x7 << 2)
#define APSRAM_CTRL_BURST_LENGTH_SHIFT                      (2)
#define APSRAM_CTRL_CP_WRAP_EN                              (1 << 5)
#define APSRAM_CTRL_PAGE_BOUNDARY(n)                        (((n) & 0x3) << 6)
#define APSRAM_CTRL_PAGE_BOUNDARY_MASK                      (0x3 << 6)
#define APSRAM_CTRL_PAGE_BOUNDARY_SHIFT                     (6)
#define APSRAM_CTRL_AUTO_PRECHARGE                          (1 << 8)

// reg_004
#define APSRAM_CTRL_ENTRY_SLEEP_IDLE                        (1 << 0)
#define APSRAM_CTRL_ENTRY_SELF_REFRESH_IDLE                 (1 << 1)
#define APSRAM_CTRL_AUTOWAKEUP_EN                           (1 << 2)
#define APSRAM_CTRL_REFRESH_EN                              (1 << 3)
#define APSRAM_CTRL_PER_BANK_REFRESH                        (1 << 4)

// reg_008
#define APSRAM_CTRL_WB_DRAIN                                (1 << 0)
#define APSRAM_CTRL_WB_INVALID                              (1 << 1)
#define APSRAM_CTRL_SNP_DISABLE                             (1 << 2)
#define APSRAM_CTRL_WB_BYPASS                               (1 << 3)
#define APSRAM_CTRL_SNP_HIT_HALT_EN(n)                      (((n) & 0x3) << 4)
#define APSRAM_CTRL_SNP_HIT_HALT_EN_MASK                    (0x3 << 4)
#define APSRAM_CTRL_SNP_HIT_HALT_EN_SHIFT                   (4)

// reg_00c
#define APSRAM_CTRL_COLUMN_BITS(n)                          (((n) & 0xF) << 0)
#define APSRAM_CTRL_COLUMN_BITS_MASK                        (0xF << 0)
#define APSRAM_CTRL_COLUMN_BITS_SHIFT                       (0)
#define APSRAM_CTRL_ROW_BITS(n)                             (((n) & 0xF) << 4)
#define APSRAM_CTRL_ROW_BITS_MASK                           (0xF << 4)
#define APSRAM_CTRL_ROW_BITS_SHIFT                          (4)
#define APSRAM_CTRL_BANK_BITS(n)                            (((n) & 0x7) << 8)
#define APSRAM_CTRL_BANK_BITS_MASK                          (0x7 << 8)
#define APSRAM_CTRL_BANK_BITS_SHIFT                         (8)
#define APSRAM_CTRL_CHIP_BITS(n)                            (((n) & 0x3) << 11)
#define APSRAM_CTRL_CHIP_BITS_MASK                          (0x3 << 11)
#define APSRAM_CTRL_CHIP_BITS_SHIFT                         (11)
#define APSRAM_CTRL_CHANNEL_BITS(n)                         (((n) & 0x3) << 13)
#define APSRAM_CTRL_CHANNEL_BITS_MASK                       (0x3 << 13)
#define APSRAM_CTRL_CHANNEL_BITS_SHIFT                      (13)

// reg_010
#define APSRAM_CTRL_ADDRESS_DECODE(n)                       (((n) & 0x3) << 0)
#define APSRAM_CTRL_ADDRESS_DECODE_MASK                     (0x3 << 0)
#define APSRAM_CTRL_ADDRESS_DECODE_SHIFT                    (0)
#define APSRAM_CTRL_STRIPE_DECODE(n)                        (((n) & 0xF) << 2)
#define APSRAM_CTRL_STRIPE_DECODE_MASK                      (0xF << 2)
#define APSRAM_CTRL_STRIPE_DECODE_SHIFT                     (2)
#define APSRAM_CTRL_BANK_DECODE                             (1 << 6)
#define APSRAM_CTRL_SUB_PAGE_RANGE                          (1 << 7)

// reg_014
#define APSRAM_CTRL_MODE_HCLK_ACCESS                        (1 << 0)
#define APSRAM_CTRL_PIO_MC_SEL                              (1 << 1)

// reg_018
#define APSRAM_CTRL_T_INIT0(n)                              (((n) & 0xFFFFF) << 0)
#define APSRAM_CTRL_T_INIT0_MASK                            (0xFFFFF << 0)
#define APSRAM_CTRL_T_INIT0_SHIFT                           (0)

// reg_01c
#define APSRAM_CTRL_T_INIT1(n)                              (((n) & 0xFFFFF) << 0)
#define APSRAM_CTRL_T_INIT1_MASK                            (0xFFFFF << 0)
#define APSRAM_CTRL_T_INIT1_SHIFT                           (0)

// reg_020
#define APSRAM_CTRL_T_RCD_WR(n)                             (((n) & 0xF) << 0)
#define APSRAM_CTRL_T_RCD_WR_MASK                           (0xF << 0)
#define APSRAM_CTRL_T_RCD_WR_SHIFT                          (0)
#define APSRAM_CTRL_T_RCD_RD(n)                             (((n) & 0xF) << 4)
#define APSRAM_CTRL_T_RCD_RD_MASK                           (0xF << 4)
#define APSRAM_CTRL_T_RCD_RD_SHIFT                          (4)

// reg_024
#define APSRAM_CTRL_T_RTP(n)                                (((n) & 0xF) << 0)
#define APSRAM_CTRL_T_RTP_MASK                              (0xF << 0)
#define APSRAM_CTRL_T_RTP_SHIFT                             (0)

// reg_028
#define APSRAM_CTRL_T_RRD(n)                                (((n) & 0x7) << 0)
#define APSRAM_CTRL_T_RRD_MASK                              (0x7 << 0)
#define APSRAM_CTRL_T_RRD_SHIFT                             (0)
#define APSRAM_CTRL_T_RRD_REF2N(n)                          (((n) & 0x7) << 3)
#define APSRAM_CTRL_T_RRD_REF2N_MASK                        (0x7 << 3)
#define APSRAM_CTRL_T_RRD_REF2N_SHIFT                       (3)

// reg_02c
#define APSRAM_CTRL_T_WR(n)                                 (((n) & 0xF) << 0)
#define APSRAM_CTRL_T_WR_MASK                               (0xF << 0)
#define APSRAM_CTRL_T_WR_SHIFT                              (0)

// reg_030
#define APSRAM_CTRL_T_RAS(n)                                (((n) & 0x1F) << 0)
#define APSRAM_CTRL_T_RAS_MASK                              (0x1F << 0)
#define APSRAM_CTRL_T_RAS_SHIFT                             (0)

// reg_034
#define APSRAM_CTRL_T_RP(n)                                 (((n) & 0xF) << 0)
#define APSRAM_CTRL_T_RP_MASK                               (0xF << 0)
#define APSRAM_CTRL_T_RP_SHIFT                              (0)

// reg_038
#define APSRAM_CTRL_T_RC(n)                                 (((n) & 0x1F) << 0)
#define APSRAM_CTRL_T_RC_MASK                               (0x1F << 0)
#define APSRAM_CTRL_T_RC_SHIFT                              (0)

// reg_03c
#define APSRAM_CTRL_T_RFC(n)                                (((n) & 0x3F) << 0)
#define APSRAM_CTRL_T_RFC_MASK                              (0x3F << 0)
#define APSRAM_CTRL_T_RFC_SHIFT                             (0)

// reg_040
#define APSRAM_CTRL_T_RFCI(n)                               (((n) & 0x1F) << 0)
#define APSRAM_CTRL_T_RFCI_MASK                             (0x1F << 0)
#define APSRAM_CTRL_T_RFCI_SHIFT                            (0)

// reg_044
#define APSRAM_CTRL_T_RFI(n)                                (((n) & 0xFFFF) << 0)
#define APSRAM_CTRL_T_RFI_MASK                              (0xFFFF << 0)
#define APSRAM_CTRL_T_RFI_SHIFT                             (0)

// reg_048
#define APSRAM_CTRL_T_WESR(n)                               (((n) & 0x3) << 0)
#define APSRAM_CTRL_T_WESR_MASK                             (0x3 << 0)
#define APSRAM_CTRL_T_WESR_SHIFT                            (0)
#define APSRAM_CTRL_T_WEHR(n)                               (((n) & 0x7) << 2)
#define APSRAM_CTRL_T_WEHR_MASK                             (0x7 << 2)
#define APSRAM_CTRL_T_WEHR_SHIFT                            (2)
#define APSRAM_CTRL_T_WESW(n)                               (((n) & 0x3) << 5)
#define APSRAM_CTRL_T_WESW_MASK                             (0x3 << 5)
#define APSRAM_CTRL_T_WESW_SHIFT                            (5)
#define APSRAM_CTRL_T_WEHW(n)                               (((n) & 0x3) << 7)
#define APSRAM_CTRL_T_WEHW_MASK                             (0x3 << 7)
#define APSRAM_CTRL_T_WEHW_SHIFT                            (7)

// reg_04c
#define APSRAM_CTRL_T_WTR(n)                                (((n) & 0x7) << 0)
#define APSRAM_CTRL_T_WTR_MASK                              (0x7 << 0)
#define APSRAM_CTRL_T_WTR_SHIFT                             (0)

// reg_050
#define APSRAM_CTRL_T_RTW(n)                                (((n) & 0x7) << 0)
#define APSRAM_CTRL_T_RTW_MASK                              (0x7 << 0)
#define APSRAM_CTRL_T_RTW_SHIFT                             (0)

// reg_054
#define APSRAM_CTRL_T_CK2DQ(n)                              (((n) & 0x7) << 0)
#define APSRAM_CTRL_T_CK2DQ_MASK                            (0x7 << 0)
#define APSRAM_CTRL_T_CK2DQ_SHIFT                           (0)

// reg_058
#define APSRAM_CTRL_T_XP(n)                                 (((n) & 0x7) << 0)
#define APSRAM_CTRL_T_XP_MASK                               (0x7 << 0)
#define APSRAM_CTRL_T_XP_SHIFT                              (0)
#define APSRAM_CTRL_T_SRX(n)                                (((n) & 0x3F) << 3)
#define APSRAM_CTRL_T_SRX_MASK                              (0x3F << 3)
#define APSRAM_CTRL_T_SRX_SHIFT                             (3)
#define APSRAM_CTRL_T_XDPD(n)                               (((n) & 0xFFFFF) << 9)
#define APSRAM_CTRL_T_XDPD_MASK                             (0xFFFFF << 9)
#define APSRAM_CTRL_T_XDPD_SHIFT                            (9)

// reg_05c
#define APSRAM_CTRL_T_PD(n)                                 (((n) & 0x3F) << 0)
#define APSRAM_CTRL_T_PD_MASK                               (0x3F << 0)
#define APSRAM_CTRL_T_PD_SHIFT                              (0)
#define APSRAM_CTRL_T_SRF(n)                                (((n) & 0x1FF) << 6)
#define APSRAM_CTRL_T_SRF_MASK                              (0x1FF << 6)
#define APSRAM_CTRL_T_SRF_SHIFT                             (6)
#define APSRAM_CTRL_T_DPD(n)                                (((n) & 0xFFFF) << 15)
#define APSRAM_CTRL_T_DPD_MASK                              (0xFFFF << 15)
#define APSRAM_CTRL_T_DPD_SHIFT                             (15)

// reg_060
#define APSRAM_CTRL_QUEUE_IDLE_CYCLE(n)                     (((n) & 0xFFFFF) << 0)
#define APSRAM_CTRL_QUEUE_IDLE_CYCLE_MASK                   (0xFFFFF << 0)
#define APSRAM_CTRL_QUEUE_IDLE_CYCLE_SHIFT                  (0)

// reg_064
#define APSRAM_CTRL_TURN_LIMIT(n)                           (((n) & 0xF) << 0)
#define APSRAM_CTRL_TURN_LIMIT_MASK                         (0xF << 0)
#define APSRAM_CTRL_TURN_LIMIT_SHIFT                        (0)

// reg_068
#define APSRAM_CTRL_RX_POS_NEG_SEL                          (1 << 0)
#define APSRAM_CTRL_RX_DLY_SEL(n)                           (((n) & 0xF) << 1)
#define APSRAM_CTRL_RX_DLY_SEL_MASK                         (0xF << 1)
#define APSRAM_CTRL_RX_DLY_SEL_SHIFT                        (1)

// reg_070
#define APSRAM_CTRL_REG_RSTB_DR                             (1 << 0)
#define APSRAM_CTRL_REG_RSTB                                (1 << 1)
#define APSRAM_CTRL_REG_PDN_DR                              (1 << 2)
#define APSRAM_CTRL_REG_PDN(n)                              (((n) & 0x3) << 3)
#define APSRAM_CTRL_REG_PDN_MASK                            (0x3 << 3)
#define APSRAM_CTRL_REG_PDN_SHIFT                           (3)

// reg_074
#define APSRAM_CTRL_MGR_CMD(n)                              (((n) & 0xFF) << 0)
#define APSRAM_CTRL_MGR_CMD_MASK                            (0xFF << 0)
#define APSRAM_CTRL_MGR_CMD_SHIFT                           (0)

// reg_080
#define APSRAM_CTRL_RD_DF_EN                                (1 << 0)
#define APSRAM_CTRL_RD_DF_DEC_EN                            (1 << 1)
#define APSRAM_CTRL_WR_DM_EN                                (1 << 2)
#define APSRAM_CTRL_WR_DM_DEC_EN                            (1 << 3)
#define APSRAM_CTRL_RD_RIB_FIFO_POP_MERGE                   (1 << 4)
#define APSRAM_CTRL_PREFETCH_ENABLE                         (1 << 5)
#define APSRAM_CTRL_PREFETCH_SNP_EN                         (1 << 6)

// reg_084
#define APSRAM_CTRL_CACTIVE_OUT_M_MAN                       (1 << 0)
#define APSRAM_CTRL_CACTIVE_OUT_H_MAN                       (1 << 1)

// reg_088
#define APSRAM_CTRL_TRANS_TIMEOUT_PRESCALAR(n)              (((n) & 0x3) << 0)
#define APSRAM_CTRL_TRANS_TIMEOUT_PRESCALAR_MASK            (0x3 << 0)
#define APSRAM_CTRL_TRANS_TIMEOUT_PRESCALAR_SHIFT           (0)
#define APSRAM_CTRL_TRANS_TIMEOUT_MASK_EN                   (1 << 2)
#define APSRAM_CTRL_TRANS_SPLIT_MASK_EN                     (1 << 3)

// reg_08c
#define APSRAM_CTRL_WB_REQ_LEVEL(n)                         (((n) & 0xF) << 0)
#define APSRAM_CTRL_WB_REQ_LEVEL_MASK                       (0xF << 0)
#define APSRAM_CTRL_WB_REQ_LEVEL_SHIFT                      (0)

// reg_090
#define APSRAM_CTRL_RB_BYPASS                               (1 << 0)
#define APSRAM_CTRL_RB_SQUELCH_DR                           (1 << 1)
#define APSRAM_CTRL_RB_SQUELCH(n)                           (((n) & 0xF) << 2)
#define APSRAM_CTRL_RB_SQUELCH_MASK                         (0xF << 2)
#define APSRAM_CTRL_RB_SQUELCH_SHIFT                        (2)
#define APSRAM_CTRL_WB_SQUELCH_DR                           (1 << 6)
#define APSRAM_CTRL_WB_SQUELCH(n)                           (((n) & 0xF) << 7)
#define APSRAM_CTRL_WB_SQUELCH_MASK                         (0xF << 7)
#define APSRAM_CTRL_WB_SQUELCH_SHIFT                        (7)

// reg_094
#define APSRAM_CTRL_BUS_HALT_EN                             (1 << 0)
#define APSRAM_CTRL_BUS_HALT_STS(n)                         (((n) & 0x3) << 1)
#define APSRAM_CTRL_BUS_HALT_STS_MASK                       (0x3 << 1)
#define APSRAM_CTRL_BUS_HALT_STS_SHIFT                      (1)

// reg_0a0
#define APSRAM_CTRL_WR_QOS0_PRIORITY(n)                     (((n) & 0x7) << 0)
#define APSRAM_CTRL_WR_QOS0_PRIORITY_MASK                   (0x7 << 0)
#define APSRAM_CTRL_WR_QOS0_PRIORITY_SHIFT                  (0)
#define APSRAM_CTRL_WR_QOS1_PRIORITY(n)                     (((n) & 0x7) << 3)
#define APSRAM_CTRL_WR_QOS1_PRIORITY_MASK                   (0x7 << 3)
#define APSRAM_CTRL_WR_QOS1_PRIORITY_SHIFT                  (3)
#define APSRAM_CTRL_WR_QOS2_PRIORITY(n)                     (((n) & 0x7) << 6)
#define APSRAM_CTRL_WR_QOS2_PRIORITY_MASK                   (0x7 << 6)
#define APSRAM_CTRL_WR_QOS2_PRIORITY_SHIFT                  (6)
#define APSRAM_CTRL_WR_QOS3_PRIORITY(n)                     (((n) & 0x7) << 9)
#define APSRAM_CTRL_WR_QOS3_PRIORITY_MASK                   (0x7 << 9)
#define APSRAM_CTRL_WR_QOS3_PRIORITY_SHIFT                  (9)
#define APSRAM_CTRL_WR_QOS4_PRIORITY(n)                     (((n) & 0x7) << 12)
#define APSRAM_CTRL_WR_QOS4_PRIORITY_MASK                   (0x7 << 12)
#define APSRAM_CTRL_WR_QOS4_PRIORITY_SHIFT                  (12)
#define APSRAM_CTRL_WR_QOS5_PRIORITY(n)                     (((n) & 0x7) << 15)
#define APSRAM_CTRL_WR_QOS5_PRIORITY_MASK                   (0x7 << 15)
#define APSRAM_CTRL_WR_QOS5_PRIORITY_SHIFT                  (15)
#define APSRAM_CTRL_WR_QOS6_PRIORITY(n)                     (((n) & 0x7) << 18)
#define APSRAM_CTRL_WR_QOS6_PRIORITY_MASK                   (0x7 << 18)
#define APSRAM_CTRL_WR_QOS6_PRIORITY_SHIFT                  (18)
#define APSRAM_CTRL_WR_QOS7_PRIORITY(n)                     (((n) & 0x7) << 21)
#define APSRAM_CTRL_WR_QOS7_PRIORITY_MASK                   (0x7 << 21)
#define APSRAM_CTRL_WR_QOS7_PRIORITY_SHIFT                  (21)

// reg_0a4
#define APSRAM_CTRL_RD_QOS0_PRIORITY(n)                     (((n) & 0x7) << 0)
#define APSRAM_CTRL_RD_QOS0_PRIORITY_MASK                   (0x7 << 0)
#define APSRAM_CTRL_RD_QOS0_PRIORITY_SHIFT                  (0)
#define APSRAM_CTRL_RD_QOS1_PRIORITY(n)                     (((n) & 0x7) << 3)
#define APSRAM_CTRL_RD_QOS1_PRIORITY_MASK                   (0x7 << 3)
#define APSRAM_CTRL_RD_QOS1_PRIORITY_SHIFT                  (3)
#define APSRAM_CTRL_RD_QOS2_PRIORITY(n)                     (((n) & 0x7) << 6)
#define APSRAM_CTRL_RD_QOS2_PRIORITY_MASK                   (0x7 << 6)
#define APSRAM_CTRL_RD_QOS2_PRIORITY_SHIFT                  (6)
#define APSRAM_CTRL_RD_QOS3_PRIORITY(n)                     (((n) & 0x7) << 9)
#define APSRAM_CTRL_RD_QOS3_PRIORITY_MASK                   (0x7 << 9)
#define APSRAM_CTRL_RD_QOS3_PRIORITY_SHIFT                  (9)
#define APSRAM_CTRL_RD_QOS4_PRIORITY(n)                     (((n) & 0x7) << 12)
#define APSRAM_CTRL_RD_QOS4_PRIORITY_MASK                   (0x7 << 12)
#define APSRAM_CTRL_RD_QOS4_PRIORITY_SHIFT                  (12)
#define APSRAM_CTRL_RD_QOS5_PRIORITY(n)                     (((n) & 0x7) << 15)
#define APSRAM_CTRL_RD_QOS5_PRIORITY_MASK                   (0x7 << 15)
#define APSRAM_CTRL_RD_QOS5_PRIORITY_SHIFT                  (15)
#define APSRAM_CTRL_RD_QOS6_PRIORITY(n)                     (((n) & 0x7) << 18)
#define APSRAM_CTRL_RD_QOS6_PRIORITY_MASK                   (0x7 << 18)
#define APSRAM_CTRL_RD_QOS6_PRIORITY_SHIFT                  (18)
#define APSRAM_CTRL_RD_QOS7_PRIORITY(n)                     (((n) & 0x7) << 21)
#define APSRAM_CTRL_RD_QOS7_PRIORITY_MASK                   (0x7 << 21)
#define APSRAM_CTRL_RD_QOS7_PRIORITY_SHIFT                  (21)

// reg_0a8
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_1_8THS(n)           (((n) & 0x7) << 0)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_1_8THS_MASK         (0x7 << 0)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_1_8THS_SHIFT        (0)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_2_8THS(n)           (((n) & 0x7) << 3)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_2_8THS_MASK         (0x7 << 3)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_2_8THS_SHIFT        (3)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_3_8THS(n)           (((n) & 0x7) << 6)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_3_8THS_MASK         (0x7 << 6)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_3_8THS_SHIFT        (6)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_4_8THS(n)           (((n) & 0x7) << 9)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_4_8THS_MASK         (0x7 << 9)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_4_8THS_SHIFT        (9)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_5_8THS(n)           (((n) & 0x7) << 12)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_5_8THS_MASK         (0x7 << 12)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_5_8THS_SHIFT        (12)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_6_8THS(n)           (((n) & 0x7) << 15)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_6_8THS_MASK         (0x7 << 15)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_6_8THS_SHIFT        (15)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_7_8THS(n)           (((n) & 0x7) << 18)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_7_8THS_MASK         (0x7 << 18)
#define APSRAM_CTRL_WRITE_FILL_PRIORITY_7_8THS_SHIFT        (18)

// reg_0b0
#define APSRAM_CTRL_READ_FILL_PRIORITY_1_8THS(n)            (((n) & 0x7) << 0)
#define APSRAM_CTRL_READ_FILL_PRIORITY_1_8THS_MASK          (0x7 << 0)
#define APSRAM_CTRL_READ_FILL_PRIORITY_1_8THS_SHIFT         (0)
#define APSRAM_CTRL_READ_FILL_PRIORITY_2_8THS(n)            (((n) & 0x7) << 3)
#define APSRAM_CTRL_READ_FILL_PRIORITY_2_8THS_MASK          (0x7 << 3)
#define APSRAM_CTRL_READ_FILL_PRIORITY_2_8THS_SHIFT         (3)
#define APSRAM_CTRL_READ_FILL_PRIORITY_3_8THS(n)            (((n) & 0x7) << 6)
#define APSRAM_CTRL_READ_FILL_PRIORITY_3_8THS_MASK          (0x7 << 6)
#define APSRAM_CTRL_READ_FILL_PRIORITY_3_8THS_SHIFT         (6)
#define APSRAM_CTRL_READ_FILL_PRIORITY_4_8THS(n)            (((n) & 0x7) << 9)
#define APSRAM_CTRL_READ_FILL_PRIORITY_4_8THS_MASK          (0x7 << 9)
#define APSRAM_CTRL_READ_FILL_PRIORITY_4_8THS_SHIFT         (9)
#define APSRAM_CTRL_READ_FILL_PRIORITY_5_8THS(n)            (((n) & 0x7) << 12)
#define APSRAM_CTRL_READ_FILL_PRIORITY_5_8THS_MASK          (0x7 << 12)
#define APSRAM_CTRL_READ_FILL_PRIORITY_5_8THS_SHIFT         (12)
#define APSRAM_CTRL_READ_FILL_PRIORITY_6_8THS(n)            (((n) & 0x7) << 15)
#define APSRAM_CTRL_READ_FILL_PRIORITY_6_8THS_MASK          (0x7 << 15)
#define APSRAM_CTRL_READ_FILL_PRIORITY_6_8THS_SHIFT         (15)
#define APSRAM_CTRL_READ_FILL_PRIORITY_7_8THS(n)            (((n) & 0x7) << 18)
#define APSRAM_CTRL_READ_FILL_PRIORITY_7_8THS_MASK          (0x7 << 18)
#define APSRAM_CTRL_READ_FILL_PRIORITY_7_8THS_SHIFT         (18)

// reg_0b4
#define APSRAM_CTRL_TIMEOUT_PRESCALAR_0(n)                  (((n) & 0x3) << 0)
#define APSRAM_CTRL_TIMEOUT_PRESCALAR_0_MASK                (0x3 << 0)
#define APSRAM_CTRL_TIMEOUT_PRESCALAR_0_SHIFT               (0)
#define APSRAM_CTRL_TIMEOUT_PRESCALAR_1(n)                  (((n) & 0x3) << 2)
#define APSRAM_CTRL_TIMEOUT_PRESCALAR_1_MASK                (0x3 << 2)
#define APSRAM_CTRL_TIMEOUT_PRESCALAR_1_SHIFT               (2)
#define APSRAM_CTRL_TIMEOUT_ESCALATE_EN_0                   (1 << 4)
#define APSRAM_CTRL_TIMEOUT_ESCALATE_EN_1                   (1 << 5)

// reg_0b8
#define APSRAM_CTRL_WRITE_THRESHOLD_ENABLE                  (1 << 0)
#define APSRAM_CTRL_READ_THRESHOLD_ENABLE                   (1 << 1)

// reg_0bc
#define APSRAM_CTRL_RD_PREFETCH_TOKEN_TARGET_0(n)           (((n) & 0x7) << 0)
#define APSRAM_CTRL_RD_PREFETCH_TOKEN_TARGET_0_MASK         (0x7 << 0)
#define APSRAM_CTRL_RD_PREFETCH_TOKEN_TARGET_0_SHIFT        (0)
#define APSRAM_CTRL_RD_PREFETCH_TOKEN_TARGET_1(n)           (((n) & 0x7) << 3)
#define APSRAM_CTRL_RD_PREFETCH_TOKEN_TARGET_1_MASK         (0x7 << 3)
#define APSRAM_CTRL_RD_PREFETCH_TOKEN_TARGET_1_SHIFT        (3)
#define APSRAM_CTRL_WR_PREFETCH_TOKEN_TARGET_0(n)           (((n) & 0x7) << 6)
#define APSRAM_CTRL_WR_PREFETCH_TOKEN_TARGET_0_MASK         (0x7 << 6)
#define APSRAM_CTRL_WR_PREFETCH_TOKEN_TARGET_0_SHIFT        (6)
#define APSRAM_CTRL_WR_PREFETCH_TOKEN_TARGET_1(n)           (((n) & 0x7) << 9)
#define APSRAM_CTRL_WR_PREFETCH_TOKEN_TARGET_1_MASK         (0x7 << 9)
#define APSRAM_CTRL_WR_PREFETCH_TOKEN_TARGET_1_SHIFT        (9)

// reg_0c0
#define APSRAM_CTRL_WVQOS0_DR_EN                            (1 << 0)
#define APSRAM_CTRL_WVQOS0(n)                               (((n) & 0x7) << 1)
#define APSRAM_CTRL_WVQOS0_MASK                             (0x7 << 1)
#define APSRAM_CTRL_WVQOS0_SHIFT                            (1)
#define APSRAM_CTRL_WVQOS1_DR_EN                            (1 << 4)
#define APSRAM_CTRL_WVQOS1(n)                               (((n) & 0x7) << 5)
#define APSRAM_CTRL_WVQOS1_MASK                             (0x7 << 5)
#define APSRAM_CTRL_WVQOS1_SHIFT                            (5)
#define APSRAM_CTRL_RVQOS0_DR_EN                            (1 << 8)
#define APSRAM_CTRL_RVQOS0(n)                               (((n) & 0x7) << 9)
#define APSRAM_CTRL_RVQOS0_MASK                             (0x7 << 9)
#define APSRAM_CTRL_RVQOS0_SHIFT                            (9)
#define APSRAM_CTRL_RVQOS1_DR_EN                            (1 << 12)
#define APSRAM_CTRL_RVQOS1(n)                               (((n) & 0x7) << 13)
#define APSRAM_CTRL_RVQOS1_MASK                             (0x7 << 13)
#define APSRAM_CTRL_RVQOS1_SHIFT                            (13)

// reg_100
#define APSRAM_CTRL_BUSY                                    (1 << 0)
#define APSRAM_CTRL_WB_FILL_LEVEL(n)                        (((n) & 0x1F) << 1)
#define APSRAM_CTRL_WB_FILL_LEVEL_MASK                      (0x1F << 1)
#define APSRAM_CTRL_WB_FILL_LEVEL_SHIFT                     (1)
#define APSRAM_CTRL_WB_EMPTY                                (1 << 6)
#define APSRAM_CTRL_RB_EMPTY                                (1 << 7)
#define APSRAM_CTRL_CP_FSM_STATE(n)                         (((n) & 0xF) << 8)
#define APSRAM_CTRL_CP_FSM_STATE_MASK                       (0xF << 8)
#define APSRAM_CTRL_CP_FSM_STATE_SHIFT                      (8)
#define APSRAM_CTRL_CFG_STATUS(n)                           (((n) & 0x7) << 12)
#define APSRAM_CTRL_CFG_STATUS_MASK                         (0x7 << 12)
#define APSRAM_CTRL_CFG_STATUS_SHIFT                        (12)

// reg_104
#define APSRAM_CTRL_RD_ADDR_STS(n)                          (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_RD_ADDR_STS_MASK                        (0xFFFFFFFF << 0)
#define APSRAM_CTRL_RD_ADDR_STS_SHIFT                       (0)

// reg_108
#define APSRAM_CTRL_EN_CLK_DR                               (1 << 0)

// reg_200
#define APSRAM_CTRL_PMU_MONITOR_START                       (1 << 0)
#define APSRAM_CTRL_PMU_MONITOR_END                         (1 << 1)
#define APSRAM_CTRL_PMU_MONITOR_STS                         (1 << 2)

// reg_204
#define APSRAM_CTRL_PMU_TOL_MON_CLK_CYCLE0(n)               (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_MON_CLK_CYCLE0_MASK             (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_MON_CLK_CYCLE0_SHIFT            (0)

// reg_208
#define APSRAM_CTRL_PMU_TOL_MON_CLK_CYCLE1(n)               (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_MON_CLK_CYCLE1_MASK             (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_MON_CLK_CYCLE1_SHIFT            (0)

// reg_20c
#define APSRAM_CTRL_PMU_TOL_WR_DATA_BYTES0(n)               (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_WR_DATA_BYTES0_MASK             (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_WR_DATA_BYTES0_SHIFT            (0)

// reg_210
#define APSRAM_CTRL_PMU_TOL_WR_DATA_BYTES1(n)               (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_WR_DATA_BYTES1_MASK             (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_WR_DATA_BYTES1_SHIFT            (0)

// reg_214
#define APSRAM_CTRL_PMU_TOL_RD_DATA_BYTES0(n)               (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_RD_DATA_BYTES0_MASK             (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_RD_DATA_BYTES0_SHIFT            (0)

// reg_218
#define APSRAM_CTRL_PMU_TOL_RD_DATA_BYTES1(n)               (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_RD_DATA_BYTES1_MASK             (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_RD_DATA_BYTES1_SHIFT            (0)

// reg_224
#define APSRAM_CTRL_PMU_TOL_WR_ACC_NUM_TO_DIE(n)            (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_WR_ACC_NUM_TO_DIE_MASK          (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_WR_ACC_NUM_TO_DIE_SHIFT         (0)

// reg_228
#define APSRAM_CTRL_PMU_TOL_WR_CYCS_IN_MCLK_0(n)            (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_WR_CYCS_IN_MCLK_0_MASK          (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_WR_CYCS_IN_MCLK_0_SHIFT         (0)

// reg_22c
#define APSRAM_CTRL_PMU_TOL_WR_CYCS_IN_MCLK_1(n)            (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_WR_CYCS_IN_MCLK_1_MASK          (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_WR_CYCS_IN_MCLK_1_SHIFT         (0)

// reg_230
#define APSRAM_CTRL_PMU_TOL_RD_ACC_NUM_TO_DIE(n)            (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_RD_ACC_NUM_TO_DIE_MASK          (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_RD_ACC_NUM_TO_DIE_SHIFT         (0)

// reg_234
#define APSRAM_CTRL_PMU_TOL_RD_CYCS_IN_MCLK_0(n)            (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_RD_CYCS_IN_MCLK_0_MASK          (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_RD_CYCS_IN_MCLK_0_SHIFT         (0)

// reg_238
#define APSRAM_CTRL_PMU_TOL_RD_CYCS_IN_MCLK_1(n)            (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_PMU_TOL_RD_CYCS_IN_MCLK_1_MASK          (0xFFFFFFFF << 0)
#define APSRAM_CTRL_PMU_TOL_RD_CYCS_IN_MCLK_1_SHIFT         (0)

// reg_250
#define APSRAM_CTRL_WRITE_UNLOCK_CLR                        (1 << 0)

// reg_254
#define APSRAM_CTRL_WRITE_UNLOCK_SET                        (1 << 0)

// reg_7f0
#define APSRAM_CTRL_VERSION(n)                              (((n) & 0xFFFFFFFF) << 0)
#define APSRAM_CTRL_VERSION_MASK                            (0xFFFFFFFF << 0)
#define APSRAM_CTRL_VERSION_SHIFT                           (0)

#endif

