/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#include "string.h"
#include "hal_trace.h"
//#include "nandflash_drv.h"
#include "nandflash_gd5f4gq4.h"
#include "nandflash_w25m02gv.h"
#include "norflash_cfg.h"
//#include "system_utils.h"
#include "nandflash_ecc.h"

#define ECC_ST_BFC_LIST_LEN                     10

// Internal ecc cfg list.
struct ECC_ST_TYPE_T {
    uint8_t bfc[ECC_ST_BFC_LIST_LEN];   // bfc status list
    uint8_t bad;                        // bad status
    uint8_t ecc10_reg;                  // register to get eccs1-0
    uint8_t ecc10_mask : 4;             // ecc1-0 mask
    uint8_t ecc10_shift : 4;            // ecc1-0 shift
    uint8_t ecc32_reg;                  // register to get eccs3-2
    uint8_t ecc32_mask : 4;             // ecc3-2 mask
    uint8_t ecc32_shift : 4;            // ecc3-2 shift
};

const struct ECC_ST_TYPE_T eccs_type[] = {
    { // ecc3-ecc2: C0h: S5 S4;  ecc1-ecc0: F0h: S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_A_00XX_0,
                NANDFLASH_ECC_ST_A_0100_4,
                NANDFLASH_ECC_ST_A_0101_5,
                NANDFLASH_ECC_ST_A_0110_6,
                NANDFLASH_ECC_ST_A_0111_7,
                NANDFLASH_ECC_ST_A_11XX_8,
                0,
               },
        .bad = NANDFLASH_ECC_ST_A_10XX_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS3,
        .ecc10_mask = 0b11,
        .ecc10_shift = 4,
        .ecc32_reg = GD5F4GQ4_REG_STATUS,
        .ecc32_mask = 0b11,
        .ecc32_shift = 4,
    },
    { // ecc3-ecc2-ecc1-ecc0: C0h S5 S4 S3 S2
        .bfc = {
                NANDFLASH_ECC_ST_B_0001_1,
                NANDFLASH_ECC_ST_B_0010_2,
                NANDFLASH_ECC_ST_B_0011_3,
                NANDFLASH_ECC_ST_B_0100_4,
                NANDFLASH_ECC_ST_B_0101_5,
                NANDFLASH_ECC_ST_B_0110_6,
                NANDFLASH_ECC_ST_B_0111_7,
                NANDFLASH_ECC_ST_B_1000_8,
                0,
               },
        .bad = NANDFLASH_ECC_ST_B_11XX_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b11,
        .ecc10_shift = 2,
        .ecc32_reg = GD5F4GQ4_REG_STATUS,
        .ecc32_mask = 0b11,
        .ecc32_shift = 4,
    },
    { // ecc3-ecc2-ecc1-ecc0: C0h S7 S6 S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_C_0001_4,
                NANDFLASH_ECC_ST_C_0101_5,
                NANDFLASH_ECC_ST_C_1001_6,
                NANDFLASH_ECC_ST_C_1101_7,
                NANDFLASH_ECC_ST_C_XX11_8,
                0,
               },
        .bad = NANDFLASH_ECC_ST_C_XX10_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b11,
        .ecc10_shift = 4,
        .ecc32_reg = GD5F4GQ4_REG_STATUS,
        .ecc32_mask = 0b11,
        .ecc32_shift = 6,
    },
    { // 40h: S7    S6  S5  S4  S3  S2  S1  S0
      //      BFR7  6   5   4   3   2   1   0
      // 50h: S7    S6  S5  S4  S3  S2  S1  S0
      //      BFR15 14  13  12  11  10  9   8
      // SEE3 = BFR15 | BRR11 | BRF7 | BRF3
      // SEE2 = BFR14 | BRR10 | BRF6 | BRF2
      // SEE1 = BFR13 | BRR9  | BRF5 | BRF1
      // SEE0 = BFR12 | BRR8  | BRF4 | BRF0

        .bfc = {
                NANDFLASH_ECC_ST_D_0001_1,
                NANDFLASH_ECC_ST_D_0010_2,
                NANDFLASH_ECC_ST_D_0011_3,
                NANDFLASH_ECC_ST_D_0100_4,
                NANDFLASH_ECC_ST_D_0101_5,
                NANDFLASH_ECC_ST_D_0110_6,
                NANDFLASH_ECC_ST_D_0111_7,
                NANDFLASH_ECC_ST_D_1000_8,
                0,
               },
        .bad = NANDFLASH_ECC_ST_D_1111_BAD,
        .ecc10_reg = W25N02KW_REG_ECC_BFR1,
        .ecc10_mask = 0xF,
        .ecc10_shift =0xF,
        .ecc32_reg = W25N02KW_REG_ECC_BFR2,
        .ecc32_mask = 0xF,
        .ecc32_shift =0xF,
    },
    { // ecc1-ecc0: C0h S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_E_01_4,
                0,
               },
        .bad = NANDFLASH_ECC_ST_E_10_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b11,
        .ecc10_shift = 4,
        .ecc32_reg = 0x0,
    },
    { // ecc1-ecc0: C0h S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_F_01_1,
                0,
               },
        .bad = NANDFLASH_ECC_ST_F_10_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b11,
        .ecc10_shift = 4,
        .ecc32_reg = 0x0,
    },
    { // ecc2-ecc1-ecc0: C0h S6 S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_G_001_3,
                NANDFLASH_ECC_ST_G_010_4,
                NANDFLASH_ECC_ST_G_011_5,
                NANDFLASH_ECC_ST_G_100_6,
                NANDFLASH_ECC_ST_G_101_7,
                NANDFLASH_ECC_ST_G_110_8,
                0,
               },
        .bad = NANDFLASH_ECC_ST_G_111_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b111,
        .ecc10_shift = 4,
        .ecc32_reg = 0x0,
    },
    { // ecc2-ecc1-ecc0: C0h S6 S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_H_001_3,
                NANDFLASH_ECC_ST_H_011_6,
                NANDFLASH_ECC_ST_H_101_8,
                0,
               },
        .bad = NANDFLASH_ECC_ST_H_010_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b111,
        .ecc10_shift = 4,
        .ecc32_reg = 0x0,
    },
    { // ecc2-ecc1-ecc0: C0h S6 S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_I_001_5,
                NANDFLASH_ECC_ST_I_011_10,
                NANDFLASH_ECC_ST_I_101_13,
                0,
               },
        .bad = NANDFLASH_ECC_ST_I_010_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b111,
        .ecc10_shift = 4,
        .ecc32_reg = 0x0,
    },
    { // ecc1-ecc0: C0h S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_J_01_7,
                NANDFLASH_ECC_ST_J_10_8,
                0,
               },
        .bad = NANDFLASH_ECC_ST_J_11_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS,
        .ecc10_mask = 0b11,
        .ecc10_shift = 4,
        .ecc32_reg = 0x0,
    },
    { // ecc3-ecc2: C0h: S5 S4;  ecc1-ecc0: F0h: S5 S4
        .bfc = {
                NANDFLASH_ECC_ST_K_00XX_0,
                NANDFLASH_ECC_ST_K_0100_1,
                NANDFLASH_ECC_ST_K_0101_2,
                NANDFLASH_ECC_ST_K_0110_3,
                NANDFLASH_ECC_ST_K_0111_4,
                0,
               },
        .bad = NANDFLASH_ECC_ST_K_10XX_BAD,
        .ecc10_reg = GD5F4GQ4_REG_STATUS3,
        .ecc10_mask = 0b11,
        .ecc10_shift = 4,
        .ecc32_reg = GD5F4GQ4_REG_STATUS,
        .ecc32_mask = 0b11,
        .ecc32_shift = 4,
    },
};

int nandflash_ecc_get_eccs(enum NANDFLASH_ECC_ST_TYPE_T st_type, uint8_t *ecc10_reg, uint8_t *ecc32_reg)
{
    ASSERT(st_type < NANDFLASH_ECC_ST_TYPE_QTY, "%s: st_type(%d) error!", __func__, st_type);

    *ecc10_reg = eccs_type[st_type].ecc10_reg;
    *ecc32_reg = eccs_type[st_type].ecc32_reg;
    return 0;
}

// return
// true:  No bit errors or bit errors were detected and corrected
// false:  Bit errors greater than ECC capability and not corrected
//
bool nandflash_ecc_check(enum NANDFLASH_ECC_ST_TYPE_T st_type, uint8_t status10, uint8_t status32, uint8_t st_bad)
{
    uint8_t status, status_cur, status1, status2, status_self;
    uint8_t bad_self, bad_cfg;
    uint8_t eccs10 = 0, eccs32 = 0;
    uint32_t i;

    ASSERT(st_type < NANDFLASH_ECC_ST_TYPE_QTY, "%s: st_type(%d) error!", __func__, st_type);

    if ((eccs_type[st_type].ecc10_mask & 0xF) == 0xF &&
        (eccs_type[st_type].ecc32_mask & 0xF) == 0xF) {
        status = ( \
                ((((status32 >> 7) | (status32 >> 3) | (status10 >> 7) | (status10 >> 3)) & 0x1) << 3) | \
                ((((status32 >> 6) | (status32 >> 2) | (status10 >> 6) | (status10 >> 2)) & 0x1) << 2) | \
                ((((status32 >> 5) | (status32 >> 1) | (status10 >> 5) | (status10 >> 1)) & 0x1) << 1) | \
                ((((status32 >> 4) | (status32 >> 0) | (status10 >> 4) | (status10 >> 0)) & 0x1)) \
                );
    } else {
        if (eccs_type[st_type].ecc10_reg == eccs_type[st_type].ecc32_reg) {
            eccs10 = ((status10 >> eccs_type[st_type].ecc10_shift) & eccs_type[st_type].ecc10_mask);
            eccs32 = ((status10 >> eccs_type[st_type].ecc32_shift) & eccs_type[st_type].ecc32_mask);
        } else {
            eccs10 = ((status10 >> eccs_type[st_type].ecc10_shift) & eccs_type[st_type].ecc10_mask);

            if (eccs_type[st_type].ecc32_reg != 0x0) {
                eccs32 = ((status32 >> eccs_type[st_type].ecc32_shift) & eccs_type[st_type].ecc32_mask);
            }
        }
        status = (eccs32 << 2) | eccs10;
    }

    status1 = status & ~((eccs_type[st_type].bad >> 4) & 0xF);
    status2 = status & ~((st_bad >> 4) & 0xF);
    bad_self = (eccs_type[st_type].bad & 0xF);
    bad_cfg = (st_bad & 0xF);

    if (status == 0) {
        return true;
    } else if (status1 == bad_self || status2 == bad_cfg) {
        return false;
    } else {
        for (i = 0; i < ECC_ST_BFC_LIST_LEN; i++) {
            if (eccs_type[st_type].bfc[i] == 0) {
                break;
            }
            status_cur = status & ~((eccs_type[st_type].bfc[i] >> 4) & 0xF);
            status_self = (eccs_type[st_type].bfc[i] & 0xF);
            if (status_cur == status_self) {
                return true;
            }
            if (status_self == bad_cfg) {
                return false;
            }
        }
    }

    return 1;
}

