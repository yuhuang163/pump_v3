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
#ifndef __NANDFLASH_ECC_H__
#define __NANDFLASH_ECC_H__

#include "plat_types.h"
#include "hal_cmu.h"
#include "nandflash_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

// BFC: Bit Flip Count
// BFD: Bit Flip Detection

// ECC_ST =  ECC_ST | (!MASK << 4)
// NANDFLASH_ECC_ST_TYPE_ECCS3210_BFC  =  ECC_ST                       Number of BFC
enum NANDFLASH_ECC_ST_A_T {
   NANDFLASH_ECC_ST_A_00XX_0 = (0b0000 | (0b0011 << 4))                , // == 0
   NANDFLASH_ECC_ST_A_0100_4 = 0b0100                                  , // <= 4
   NANDFLASH_ECC_ST_A_0101_5 = 0b0101                                  , // == 5
   NANDFLASH_ECC_ST_A_0110_6 = 0b0110                                  , // == 6
   NANDFLASH_ECC_ST_A_0111_7 = 0b0111                                  , // == 7
   NANDFLASH_ECC_ST_A_11XX_8 = (0b1100 | (0b0011 << 4))                , // == 8
   NANDFLASH_ECC_ST_A_10XX_BAD = (0b1000 | (0b0011 << 4))              , //  > 8 BAD
};

enum NANDFLASH_ECC_ST_B_T {
   NANDFLASH_ECC_ST_B_0001_1 = 0b0001                                  , // == 1
   NANDFLASH_ECC_ST_B_0010_2 = 0b0010                                  , // == 2
   NANDFLASH_ECC_ST_B_0011_3 = 0b0011                                  , // == 3
   NANDFLASH_ECC_ST_B_0100_4 = 0b0100                                  , // == 4
   NANDFLASH_ECC_ST_B_0101_5 = 0b0101                                  , // == 5
   NANDFLASH_ECC_ST_B_0110_6 = 0b0110                                  , // == 6
   NANDFLASH_ECC_ST_B_0111_7 = 0b0111                                  , // == 7
   NANDFLASH_ECC_ST_B_1000_8 = 0b1000                                  , // == 8
   NANDFLASH_ECC_ST_B_11XX_BAD = (0b1100 | (0b0011 << 4))              , //  > 8 BAD
};

enum NANDFLASH_ECC_ST_C_T {
   NANDFLASH_ECC_ST_C_XX00_0 = (0b0000 | (0b1100 << 4))                , // == 0
   NANDFLASH_ECC_ST_C_0001_4 = 0b0001                                  , // <= 4
   NANDFLASH_ECC_ST_C_0101_5 = 0b0101                                  , // == 5
   NANDFLASH_ECC_ST_C_1001_6 = 0b1001                                  , // == 6
   NANDFLASH_ECC_ST_C_1101_7 = 0b1101                                  , // == 7
   NANDFLASH_ECC_ST_C_XX11_8 = (0b0011 | (0b1100 << 4))                , // == 8
   NANDFLASH_ECC_ST_C_XX10_BAD = (0b0010 | (0b1100 << 4))              , //  > 8 BAD
};

enum NANDFLASH_ECC_ST_D_T {
   NANDFLASH_ECC_ST_D_0001_1 = 0b0001                                  , // == 1
   NANDFLASH_ECC_ST_D_0010_2 = 0b0010                                  , // == 2
   NANDFLASH_ECC_ST_D_0011_3 = 0b0011                                  , // == 3
   NANDFLASH_ECC_ST_D_0100_4 = 0b0100                                  , // == 4
   NANDFLASH_ECC_ST_D_0101_5 = 0b0101                                  , // == 5
   NANDFLASH_ECC_ST_D_0110_6 = 0b0110                                  , // == 6
   NANDFLASH_ECC_ST_D_0111_7 = 0b0111                                  , // == 7
   NANDFLASH_ECC_ST_D_1000_8 = 0b1000                                  , // == 8
   NANDFLASH_ECC_ST_D_1111_BAD = 0b1111                                , //  > 8 BAD
};

enum NANDFLASH_ECC_ST_E_T {
   NANDFLASH_ECC_ST_E_01_4   = 0b01                                    , // <= 4
   NANDFLASH_ECC_ST_E_10_BAD = 0b10                                    , //  > 4 BAD
};

enum NANDFLASH_ECC_ST_F_T {
   NANDFLASH_ECC_ST_F_01_1   = 0b01                                    , // == 1
   NANDFLASH_ECC_ST_F_10_BAD = 0b10                                    , //  > 1 BAD
};

enum NANDFLASH_ECC_ST_G_T {
   NANDFLASH_ECC_ST_G_001_3 = 0b001                                    , // <= 3
   NANDFLASH_ECC_ST_G_010_4 = 0b010                                    , // == 4
   NANDFLASH_ECC_ST_G_011_5 = 0b011                                    , // == 5
   NANDFLASH_ECC_ST_G_100_6 = 0b100                                    , // == 6
   NANDFLASH_ECC_ST_G_101_7 = 0b101                                    , // == 7
   NANDFLASH_ECC_ST_G_110_8 = 0b110                                    , // == 8
   NANDFLASH_ECC_ST_G_111_BAD = 0b111                                  , //  > 8 BAD
};

enum NANDFLASH_ECC_ST_H_T {
   NANDFLASH_ECC_ST_H_001_3 =  0b001                                   , // <= 3
   NANDFLASH_ECC_ST_H_011_6 =  0b011                                   , // <= 6
   NANDFLASH_ECC_ST_H_101_8 =  0b101                                   , // <= 8
   NANDFLASH_ECC_ST_H_010_BAD = 0b010                                  , //  > 8 BAD
};

enum NANDFLASH_ECC_ST_I_T {
   NANDFLASH_ECC_ST_I_001_5 =  0b001                                   , // <= 5
   NANDFLASH_ECC_ST_I_011_10 = 0b011                                   , // <= 10
   NANDFLASH_ECC_ST_I_101_13 = 0b101                                   , // <= 13
   NANDFLASH_ECC_ST_I_010_BAD =  0b010                                 , //  > 13 BAD
};

enum NANDFLASH_ECC_ST_J_T {
   NANDFLASH_ECC_ST_J_01_7   = 0b01                                    , // <= 7
   NANDFLASH_ECC_ST_J_10_8   = 0b10                                    , // <= 8
   NANDFLASH_ECC_ST_J_11_BAD = 0b11                                    , //  > 8 BAD
};

enum NANDFLASH_ECC_ST_K_T {
   NANDFLASH_ECC_ST_K_00XX_0 = (0b0000 | (0b0011 << 4))                , // == 0
   NANDFLASH_ECC_ST_K_0100_1 = 0b0100                                  , // == 1
   NANDFLASH_ECC_ST_K_0101_2 = 0b0101                                  , // == 2
   NANDFLASH_ECC_ST_K_0110_3 = 0b0110                                  , // == 3
   NANDFLASH_ECC_ST_K_0111_4 = 0b0111                                  , // == 4
   NANDFLASH_ECC_ST_K_10XX_BAD = (0b1000 | (0b0011 << 4))              , //  > 4 BAD
};

enum NANDFLASH_ECC_ST_TYPE_T {
   NANDFLASH_ECC_ST_TYPE_A = 0,
   NANDFLASH_ECC_ST_TYPE_B,
   NANDFLASH_ECC_ST_TYPE_C,
   NANDFLASH_ECC_ST_TYPE_D,
   NANDFLASH_ECC_ST_TYPE_E,
   NANDFLASH_ECC_ST_TYPE_F,
   NANDFLASH_ECC_ST_TYPE_G,
   NANDFLASH_ECC_ST_TYPE_H,
   NANDFLASH_ECC_ST_TYPE_I,
   NANDFLASH_ECC_ST_TYPE_J,
   NANDFLASH_ECC_ST_TYPE_K,
   NANDFLASH_ECC_ST_TYPE_QTY,
};

enum NANDFLASH_ECC_ST_BFD_T {
    NANDFLASH_ECC_ST_BFD_RESERVED    = 0,
    NANDFLASH_ECC_ST_BFD_1           = 1,
    NANDFLASH_ECC_ST_BFD_2           = 2,
    NANDFLASH_ECC_ST_BFD_3           = 3,
    NANDFLASH_ECC_ST_BFD_4           = 4,
    NANDFLASH_ECC_ST_BFD_5           = 5,
    NANDFLASH_ECC_ST_BFD_6           = 6,
    NANDFLASH_ECC_ST_BFD_7           = 7,
};

struct NANDFLASH_ECC_ST_CFG_T {
    uint8_t st_bfd;
    uint8_t st_type;
    uint8_t st_bad;
};

int nandflash_ecc_get_eccs(enum NANDFLASH_ECC_ST_TYPE_T st_type, uint8_t *ecc10_reg, uint8_t *ecc32_reg);
bool nandflash_ecc_check(enum NANDFLASH_ECC_ST_TYPE_T st_type, uint8_t status10, uint8_t status32, uint8_t st_bad);

#ifdef __cplusplus
}
#endif

#endif
