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
#include "nandflash_w25m02gv.h"

// ----------------------
// Winbond
// ----------------------

const struct NANDFLASH_CFG_T w25m02gv_cfg = {
    .id = { 0xEF, 0xAB, 0x21, },
    .die_num = 2,
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_RES,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_RESERVED,
        .st_type = NANDFLASH_ECC_ST_TYPE_E,
        .st_bad = NANDFLASH_ECC_ST_E_10_BAD,
    },
    .block_protect_mask = 0x7C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0A,
        },
    },
    .dummy_cycle_cfg = {
        .s = {
            .quad_io_en = true,
            .quad_io_val = 0x04,
        },
    },
    .page_size = W25M02GV_PAGE_SIZE,
    .page_spare_size = W25M02GV_PAGE_SPARE_SIZE,
    .block_size = W25M02GV_BLOCK_SIZE,
    .total_size = W25M02GV_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
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

const struct NANDFLASH_CFG_T w25n02kw_cfg = {
    .id = { 0xEF, 0xBA, 0x22, },
    .die_num = 1,
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_7,
        .st_type = NANDFLASH_ECC_ST_TYPE_D,
        .st_bad = NANDFLASH_ECC_ST_D_1111_BAD,
    },
    .block_protect_mask = 0x7C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0A,
        },
    },
    .dummy_cycle_cfg = {
        .s = {
            .quad_io_en = true,
            .quad_io_val = 0x04,
        },
    },
    .page_size = W25N02KV_PAGE_SIZE,
    .page_spare_size = W25N02KV_PAGE_SPARE_SIZE,
    .block_size = W25N02KV_BLOCK_SIZE,
    .total_size = W25N02KV_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
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

const struct NANDFLASH_CFG_T w25n04kw_cfg = {
    .id = { 0xEF, 0xBA, 0x23, },
    .die_num = 1,
    .rom_ecc_type = NANDFLASH_ECC_ST_2BITS_11_COR,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_7,
        .st_type = NANDFLASH_ECC_ST_TYPE_D,
        .st_bad = NANDFLASH_ECC_ST_D_1111_BAD,
    },
    .block_protect_mask = 0x7C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0A,
        },
    },
    .dummy_cycle_cfg = {
        .s = {
            .quad_io_en = true,
            .quad_io_val = 0x04,
        },
    },
    .page_size = W25N04KV_PAGE_SIZE,
    .page_spare_size = W25N04KV_PAGE_SPARE_SIZE,
    .block_size = W25N04KV_BLOCK_SIZE,
    .total_size = W25N04KV_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
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

const struct NANDFLASH_CFG_T w25n08lw_cfg = {
    .id = { 0xEF, 0xB3, 0x24, },
    .die_num = 1,
    .ecc_cfg = {
        .st_bfd = NANDFLASH_ECC_ST_BFD_7,
        .st_type = NANDFLASH_ECC_ST_TYPE_J,
        .st_bad = NANDFLASH_ECC_ST_J_11_BAD,
    },
    .block_protect_mask = 0x7C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .page_start = 0x02,
            .page_num = 0x0A,
        },
    },
    .dummy_cycle_cfg = {
        .s = {
            .quad_io_en = true,
            .quad_io_val = 0x04,
        },
    },
    .page_size = W25N08LW_PAGE_SIZE,
    .page_spare_size = W25N08LW_PAGE_SPARE_SIZE,
    .block_size = W25N08LW_BLOCK_SIZE,
    .total_size = W25N08LW_TOTAL_SIZE,
    .max_speed = 52 * 1000 * 1000,
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
