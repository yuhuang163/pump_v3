
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
#ifndef NORFLASH_MX25U6432F_H
#define NORFLASH_MX25U6432F_H
/* device cmd */
#include "plat_types.h"

//total size
#define MX25U6432F_TOTAL_SIZE (8*1024*1024)

#define KH25L12833F_TOTAL_SIZE (16*1024*1024)
//total size

#define MX25U6432F_CMD_ID 0x9F
#define MX25U6432F_CMD_WRITE_ENABLE 0x06
#define MX25U6432F_CMD_PAGE_PROGRAM 0x02
#define MX25U6432F_CMD_QUAD_PAGE_PROGRAM 0x32
#define MX25U6432F_CMD_BLOCK_ERASE_32K 0x52
#define MX25U6432F_CMD_BLOCK_ERASE_64K 0xD8
#define MX25U6432F_CMD_SECTOR_ERASE 0x20
#define MX25U6432F_CMD_CHIP_ERASE 0x60
#define MX25U6432F_CMD_READ_STATUS_S0_S7 0x05
#define MX25U6432F_CMD_WRITE_STATUS_S0_S7 0x01
#define MX25U6432F_CMD_FAST_QUAD_IO_READ 0xEB
#define MX25U6432F_CMD_FAST_QUAD_OUTPUT_READ 0x6B
#define MX25U6432F_CMD_FAST_DUAL_IO_READ 0xBB
#define MX25U6432F_CMD_FAST_DUAL_OUTPUT_READ 0x3B
#define MX25U6432F_CMD_STANDARD_READ 0x03
#define MX25U6432F_CMD_STANDARD_FAST_READ 0x0B
#define MX25U6432F_CMD_DEEP_POWER_DOWN 0xB9
#define MX25U6432F_CMD_RELEASE_FROM_DP 0xAB
#define MX25U6432F_CMD_ENABLE_QPI 0x38
#define MX25U6432F_CMD_DISABLE_QPI 0xFF
#define MX25U6432F_CMD_HIGH_PERFORMANCE 0xA3
#define MX25U6432F_CMD_SET_BURST_WRAP 0xC0
#define MX25U6432F_CMD_READ_SFDP_REGISTER 0x5A
#define MX25U6432F_CMD_ENABLE_RESET 0x66
#define MX25U6432F_CMD_RESET 0x99
#define MX25U6432F_CMD_ENBALE_VOLATILE_STATUS 0x50
#define MX25U6432F_CMD_PROGRAM_ERASE_SUSPEND 0x75
#define MX25U6432F_CMD_PROGRAM_ERASE_RESUME 0x7A
#define MX25U6432F_CMD_SECURITY_REGISTER_PROGRAM 0x2F
#define MX25U6432F_CMD_SECURITY_REGISTER_READ 0x2B
/* status register _S0_S7*/
#define MX25U6432F_WIP_BIT_SHIFT 0
#define MX25U6432F_WIP_BIT_MASK ((0x1)<<MX25U6432F_WIP_BIT_SHIFT)
#define MX25U6432F_WEL_BIT_SHIFT 1
#define MX25U6432F_WEL_BIT_MASK ((0x1)<<MX25U6432F_WEL_BIT_SHIFT)
#define MX25U6432F_QE_BIT_SHIFT  6
#define MX25U6432F_QE_BIT_MASK ((0x1)<<MX25U6432F_QE_BIT_SHIFT)
#define MX25U6432F_SRWD_BIT_SHIFT 7
#define MX25U6432F_SRWD_BIT_MASK ((0x1)<<MX25U6432F_SRWD_BIT_SHIFT)
#define MX25U6432F_BP0_4_BIT_SHIFT 2
#define MX25U6432F_BP0_4_BIT_MASK ((0x1F)<<MX25U6432F_BP0_4_BIT_SHIFT)
#define MX25U6432F_BP0_4_BIT(n) (((n) & 0x1F)<<MX25U6432F_BP0_4_BIT_SHIFT)
// security register
#define    MX25U6432F_OTPLOCK_MASK     0x03
#define    MX25U6432F_4BYTE_MASK       0x04
#define    MX25U6432F_WPSEL_MASK       0x80
// configuration reigster
#define    MX25U6432F_DC_MASK          0x80
#define    MX25U6432F_CR_4BYTE_MASK    0x20
#define    MX25U6432F_DC_2BIT_MASK     0xC0
#define    MX25U6432F_DC_3BIT_MASK     0x07
/* device register */
#endif
