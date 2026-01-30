/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#include "plat_types.h"
#include "nandflash_drv.h"
#include "nandflash_gd5f4gq4.h"

// ----------------------
// GigaDevice
// ----------------------

const struct NANDFLASH_CFG_T gd5f1gm7_cfg = {
    .id = { 0xC8, 0x81, 0xC8, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_A,
        .st_bad = NANDFLASH_ECC_ST_A_10XX_BAD,
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0A,
        },
    },
    .page_size = GD5F1GM7_PAGE_SIZE,
    .page_spare_size = GD5F1GM7_PAGE_SPARE_SIZE,
    .block_size = GD5F1GM7_BLOCK_SIZE,
    .total_size = GD5F1GM7_TOTAL_SIZE,
    .max_speed = 133 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                NANDFLASH_OP_MODE_DEEP_POWER_DOWN |
                0),
};

const struct NANDFLASH_CFG_T gd5f4gq4_cfg = {
    .id = { 0xC8, 0xF4, 0xC8, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_A,
        .st_bad = NANDFLASH_ECC_ST_A_10XX_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x00,
            .page_num = 0x04,
        },
    },
    .page_size = GD5F4GQ4_PAGE_SIZE,
    .page_spare_size = GD5F4GQ4_PAGE_SPARE_SIZE,
    .block_size = GD5F4GQ4_BLOCK_SIZE,
    .total_size = GD5F4GQ4_TOTAL_SIZE,
    .max_speed = 120 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_DUAL_IO |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_IO |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

const struct NANDFLASH_CFG_T gd5f2gm7r_cfg = {
    .id = { 0xC8, 0x82, 0xC8, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_A,
        .st_bad = NANDFLASH_ECC_ST_A_10XX_BAD,
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0A,
        },
    },
    .page_size = GD5F2GM7R_PAGE_SIZE,
    .page_spare_size = GD5F2GM7R_PAGE_SPARE_SIZE,
    .block_size = GD5F2GM7R_BLOCK_SIZE,
    .total_size = GD5F2GM7R_TOTAL_SIZE,
    .max_speed = 133 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                NANDFLASH_OP_MODE_DEEP_POWER_DOWN |
                0),
};

const struct NANDFLASH_CFG_T gd5f4gm8r_cfg = {
    .id = { 0xC8, 0x85, 0xC8, },
    .plane_cfg = 0x2c, // plane_num:0x2, plane_offset:0xc
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_A,
        .st_bad = NANDFLASH_ECC_ST_A_10XX_BAD,
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x1E,
        },
    },
    .page_size = GD5F4GM8R_PAGE_SIZE,
    .page_spare_size = GD5F4GM8R_PAGE_SPARE_SIZE,
    .block_size = GD5F4GM8R_BLOCK_SIZE,
    .total_size = GD5F4GM8R_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                NANDFLASH_OP_MODE_DEEP_POWER_DOWN |
                0),
};

const struct NANDFLASH_CFG_T gd5f4gm5_cfg = {
    .id = { 0xB4, 0x68, 0xC8, },
    .rom_ecc_type = NANDFLASH_ECC_ST_3BITS_111_BAD,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_G,
        .st_bad = NANDFLASH_ECC_ST_G_111_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x00,
            .page_num = 0x04,
        },
    },
    .page_size = GD5F4GM5_PAGE_SIZE,
    .page_spare_size = GD5F4GM5_PAGE_SPARE_SIZE,
    .block_size = GD5F4GM5_BLOCK_SIZE,
    .total_size = GD5F4GM5_TOTAL_SIZE,
    .max_speed = 120 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                NANDFLASH_OP_MODE_READ_ADDR_24BIT |
                0),
};

const struct NANDFLASH_CFG_T gd5f1gq5_cfg = {
    .id = { 0xC8, 0x41, 0xC8, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_K,
        .st_bad = NANDFLASH_ECC_ST_K_10XX_BAD,
    },
    .dummy_cycle_cfg = {
        .s = {
            .quad_io_en = true,
            .quad_io_val = 4,
        },
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x00,
            .page_num = 0x04,
        },
    },
    .page_size = GD5F1GM7_PAGE_SIZE,
    .page_spare_size = GD5F1GM7_PAGE_SPARE_SIZE,
    .block_size = GD5F1GM7_BLOCK_SIZE,
    .total_size = GD5F1GM7_TOTAL_SIZE,
    .max_speed = 133 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_DUAL_IO |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_IO |
                NANDFLASH_OP_MODE_DTR |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

// ----------------------
// XTS
// ----------------------
const struct NANDFLASH_CFG_T xt26g02a_cfg = {
    .id = { 0x0B, 0xE2, 0x0B, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_RES, // 0b10: BFC = 8,Corrected ; 0b11: BFC > 8,not corrected
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_B,
        .st_bad = NANDFLASH_ECC_ST_B_11XX_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x00,
            .page_num = 0x04,
        },
    },
    .page_size = GD5F4GQ4_PAGE_SIZE,
    .page_spare_size = GD5F4GQ4_PAGE_SPARE_SIZE,
    .block_size = GD5F4GQ4_BLOCK_SIZE,
    .total_size = XT26G02A_TOTAL_SIZE,
    .max_speed = 90 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_DUAL_IO |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_IO |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

const struct NANDFLASH_CFG_T xt26q01d_cfg = {
    .id = { 0x0B, 0x51, 0x00, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_C,
        .st_bad = NANDFLASH_ECC_ST_C_XX10_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x00,
            .page_num = 0x04,
        },
    },
    .page_size = XT26Q01D_PAGE_SIZE,
    .page_spare_size = XT26Q01D_PAGE_SPARE_SIZE,
    .block_size = XT26Q01D_BLOCK_SIZE,
    .total_size = XT26Q01D_TOTAL_SIZE,
    .max_speed = 108 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_DUAL_IO |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_IO |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC),
};

const struct NANDFLASH_CFG_T xt26q02d_cfg = {
    .id = { 0x0B, 0x52, 0x00, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_C,
        .st_bad = NANDFLASH_ECC_ST_C_XX10_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x00,
            .page_num = 0x04,
        },
    },
    .page_size = XT26Q02D_PAGE_SIZE,
    .page_spare_size = XT26Q02D_PAGE_SPARE_SIZE,
    .block_size = XT26Q02D_BLOCK_SIZE,
    .total_size = XT26Q02D_TOTAL_SIZE,
    .max_speed = 108 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_DUAL_IO |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_IO |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC),
};


const struct NANDFLASH_CFG_T xt26q04d_cfg = {
    .id = { 0x0B, 0x53, 0x00, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_C,
        .st_bad = NANDFLASH_ECC_ST_C_XX10_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x00,
            .page_num = 0x04,
        },
    },
    .page_size = XT26Q04D_PAGE_SIZE,
    .page_spare_size = XT26Q04D_PAGE_SPARE_SIZE,
    .block_size = XT26Q04D_BLOCK_SIZE,
    .total_size = XT26Q04D_TOTAL_SIZE,
    .max_speed = 108 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_DUAL_IO |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_IO |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC),
};

// ----------------------
// Macronix
// ----------------------

const struct NANDFLASH_CFG_T mx35lf1g_cfg = {
    .id = { 0xC2, 0x12, 0xC2, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_RES,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_E,
        .st_bad = NANDFLASH_ECC_ST_E_10_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x1E,
        },
    },
    .page_size = GD5F4GQ4_PAGE_SIZE,
    .page_spare_size = GD5F4GQ4_PAGE_SPARE_SIZE,
    .block_size = GD5F4GQ4_BLOCK_SIZE,
    .total_size = MX35LF1G_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

const struct NANDFLASH_CFG_T mt29f4g_cfg = {
    .id = { 0x2C, 0x35, 0x2C, },
    .rom_ecc_type = NANDFLASH_ECC_ST_3BITS_010_BAD,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_H,
        .st_bad = NANDFLASH_ECC_ST_H_010_BAD,
    },
    .block_protect_mask = 0x7C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0B,
        },
    },
    .dummy_cycle_cfg = {
        .s = {
            .quad_io_en = true,
            .quad_io_val = 4,
        },
     },
    .page_size = MT29F4G_PAGE_SIZE,
    .page_spare_size = MT29F4G_PAGE_SPARE_SIZE,
    .block_size = MT29F4G_BLOCK_SIZE,
    .total_size = MT29F4G_TOTAL_SIZE,
    .max_speed = 83 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
             NANDFLASH_OP_MODE_FAST_SPI |
             NANDFLASH_OP_MODE_DUAL_OUTPUT |
             NANDFLASH_OP_MODE_DUAL_IO |
             NANDFLASH_OP_MODE_QUAD_OUTPUT |
             NANDFLASH_OP_MODE_QUAD_IO |
             NANDFLASH_OP_MODE_PAGE_PROGRAM |
             NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
             NANDFLASH_OP_MODE_INTERNAL_ECC |
             NANDFLASH_OP_MODE_NO_QE_BIT |
             0),
};

// ----------------------
// Dosilicon
// ----------------------

const struct NANDFLASH_CFG_T ds35m2gb_cfg = {
    .id = { 0xE5, 0xA2, 0xE5, },
    .plane_cfg = 0x2c, // plane_num:0x2, plane_offset:0xc
    .rom_ecc_type = NANDFLASH_ECC_ST_3BITS_010_BAD,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_H,
        .st_bad = NANDFLASH_ECC_ST_H_010_BAD,
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x1E,
        },
    },
    .page_size = DS35X2GB_PAGE_SIZE,
    .page_spare_size = DS35X2GB_PAGE_SPARE_SIZE,
    .block_size = DS35X2GB_BLOCK_SIZE,
    .total_size = DS35X2GB_TOTAL_SIZE,
    .max_speed = 83 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

const struct NANDFLASH_CFG_T ds35m4gm_cfg = {
    .id = { 0xE5, 0xA4, 0xE5, },
    .plane_cfg = 0x2c, // plane_num:0x2, plane_offset:0xc
    .rom_ecc_type = NANDFLASH_ECC_ST_3BITS_010_BAD,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_H,
        .st_bad = NANDFLASH_ECC_ST_H_010_BAD,
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x1E,
        },
    },
    .page_size = DS35X4GB_PAGE_SIZE,
    .page_spare_size = DS35X4GB_PAGE_SPARE_SIZE,
    .block_size = DS35X4GB_BLOCK_SIZE,
    .total_size = DS35X4GB_TOTAL_SIZE,
    .max_speed = 83 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

const struct NANDFLASH_CFG_T ds35m1ga_cfg = {
    .id = { 0xE5, 0x21, 0xE5, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_RES,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_E,
        .st_bad = NANDFLASH_ECC_ST_E_10_BAD,
    },
    .block_protect_mask = 0x3E,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x1E,
        },
    },
    .page_size = DS35X1GB_PAGE_SIZE,
    .page_spare_size = DS35X1GB_PAGE_SPARE_SIZE,
    .block_size = DS35X1GB_BLOCK_SIZE,
    .total_size = DS35X1GB_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

const struct NANDFLASH_CFG_T ds35m4gb_cfg = {
    .id = { 0xE5, 0x64, 0xE5, },
    .rom_ecc_type = NANDFLASH_ECC_ST_3BITS_010_BAD,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_H,
        .st_bad = NANDFLASH_ECC_ST_H_010_BAD,
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x1E,
        },
    },
    .page_size = DS35X4GB_PAGE_SIZE,
    .page_spare_size = DS35X4GB_PAGE_SPARE_SIZE,
    .block_size = DS35X4GB_BLOCK_SIZE,
    .total_size = DS35X4GB_TOTAL_SIZE,
    .max_speed = 83 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};

// ----------------------
// UNIM
// ----------------------
const struct NANDFLASH_CFG_T um19a1lisw_cfg = {
    .id = { 0xB0, 0x25, 0xB0, },
    .plane_cfg = 0x2C, // plane_num:0x2, plane_offset:0xc
    .rom_ecc_type = NANDFLASH_ECC_ST_3BITS_010_BAD,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_I,
        .st_bad = NANDFLASH_ECC_ST_I_010_BAD,
    },
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0A,
        },
    },
    .page_size = UM19A1LISW_PAGE_SIZE,
    .page_spare_size = UM19A1LISW_PAGE_SPARE_SIZE,
    .block_size = UM19A1LISW_BLOCK_SIZE,
    .total_size = UM19A1LISW_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};


// ----------------------
// FORESEE
// ----------------------
const struct NANDFLASH_CFG_T f35uqa001g_cfg = {
    .id = { 0xCD, 0x61, 0x61, },
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_RES,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_F,
        .st_bad = NANDFLASH_ECC_ST_F_10_BAD,
    },
    .block_protect_mask = 0x7C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x01,
            .page_num = 0x3E,
        },
    },
    .page_size = F35UQA001G_PAGE_SIZE,
    .page_spare_size = F35UQA001G_PAGE_SPARE_SIZE,
    .block_size = F35UQA001G_BLOCK_SIZE,
    .total_size = F35UQA001G_TOTAL_SIZE,
    .max_speed = 66 * 1000 * 1000,
    .mode = (NANDFLASH_OP_MODE_STAND_SPI |
                NANDFLASH_OP_MODE_FAST_SPI |
                NANDFLASH_OP_MODE_DUAL_OUTPUT |
                NANDFLASH_OP_MODE_QUAD_OUTPUT |
                NANDFLASH_OP_MODE_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                NANDFLASH_OP_MODE_INTERNAL_ECC |
                0),
};


