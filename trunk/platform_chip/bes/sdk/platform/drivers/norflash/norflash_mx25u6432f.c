
/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "norflash_drv.h"
#include "hal_norflaship.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "norflash_cfg.h"
#include "norflash_gd25lq32c.h"
#include "norflash_gd25q32c.h"
#include "norflash_mx25u6432f.h"
static void mx25u6432f_write_status_s0_s7(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint8_t status)
{
    bool volatile_reg = false;
    norflash_write_reg(id, volatile_reg, MX25U6432F_CMD_WRITE_STATUS_S0_S7, &status, 1);
}
static int mx25u6432f_write_status(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint32_t param)
{
    uint8_t status_s0_s7;
    uint32_t bp_mask = 0;
    if (type >= DRV_NORFLASH_W_STATUS_QTY) {
        return 1;
    }
    else if (type == DRV_NORFLASH_W_STATUS_INIT) {
        mx25u6432f_write_status_s0_s7(id, type, param & 0xFF);
        return 0;
    }
    else if (type == DRV_NORFLASH_W_STATUS_BP_INIT || type == DRV_NORFLASH_W_STATUS_BP) {
        bp_mask = norflash_get_block_protect_mask(id);
        status_s0_s7 = norflash_read_status_s0_s7(id);
        status_s0_s7 = (status_s0_s7 & ~bp_mask) | (param & bp_mask);
        mx25u6432f_write_status_s0_s7(id, type, status_s0_s7);
        if ((bp_mask & ~0xFF) == 0) {
            return 0;
        }
    }
	else if (type == DRV_NORFLASH_W_STATUS_QE) {
		status_s0_s7 = norflash_read_status_s0_s7(id);
        if (param) {
            status_s0_s7 |= MX25U6432F_QE_BIT_MASK;
            mx25u6432f_write_status_s0_s7(id, type, status_s0_s7);
            status_s0_s7 = norflash_read_status_s0_s7(id);
        } else {
            status_s0_s7 &= ~MX25U6432F_QE_BIT_MASK;
            mx25u6432f_write_status_s0_s7(id, type, status_s0_s7);
        }
	}
	else{
        status_s0_s7 = norflash_read_status_s0_s7(id);
        if (param) {
            status_s0_s7 |= MX25U6432F_SRWD_BIT_MASK;
            mx25u6432f_write_status_s0_s7(id, type, status_s0_s7);
        } else {
            status_s0_s7 &= ~MX25U6432F_SRWD_BIT_MASK;
            mx25u6432f_write_status_s0_s7(id, type, status_s0_s7);
        }
	}
    status_s0_s7 = norflash_read_status_s0_s7(id);
    return 0;
}
const struct NORFLASH_CFG_T mx25u6432f_cfg = {
    .id = { 0xC2, 0x25, 0x37, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_3_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_4_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = 0xA5,
    .crm_dis_bits = 0,
    .block_protect_mask = 0x3C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = MX25U6432F_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT|
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 85 * 1000 * 1000,
    .write_status = mx25u6432f_write_status,
};

const struct NORFLASH_CFG_T kh25l12833f_cfg = {
    .id = { 0xC2, 0x20, 0x18, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_8_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = GD25LE128E_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                // HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = mx25u6432f_write_status,
};