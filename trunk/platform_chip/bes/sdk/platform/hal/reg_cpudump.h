/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __REG_CPUDUMP_H__
#define __REG_CPUDUMP_H__

#include "plat_types.h"

// CPU DUMP Registers
struct CPUDUMP_T{
    __O  uint32_t PC_DATA[0x400];
    __O  uint32_t LR_DATA[0x400];
    __IO uint32_t CPUDUMP_EN;
    __IO uint32_t CPUDUMP_CTRL;
    __IO uint32_t CPUDUMP_CTS;
    __IO uint32_t CPUDUMP_FAULT_MASK;
};

// reg_2000
#define CMU_REG_DUMP_EN                                     (1 << 0)

// reg_2004
#define CMU_REG_DUMP_STOP                                   (1 << 0)
#define CMU_REG_DUMP_START                                  (1 << 1)

// reg_2008
#define CMU_R_DUMP_CST(n)                                   (((n) & 0x3) << 0)
#define CMU_R_DUMP_CST_MASK                                 (0x3 << 0)
#define CMU_R_DUMP_CST_SHIFT                                (0)

// reg_200c
#define CMU_REG_MM_FAULT                                    (1 << 0)
#define CMU_REG_BUS_FAULT                                   (1 << 1)
#define CMU_REG_USAGE_FAULT                                 (1 << 2)
#define CMU_REG_BPT_FAULT                                   (1 << 3)
#define CMU_REG_SV_FAULT                                    (1 << 4)
#define CMU_REG_SF_FAULT                                    (1 << 5)
#define CMU_REG_FAULT_MASK_EN                               (1 << 6)

// reg_2010
#define CMU_CURRENT_ADDR_PC(n)                              (((n) & 0x3F) << 0)
#define CMU_CURRENT_ADDR_PC_MASK                            (0x3F << 0)
#define CMU_CURRENT_ADDR_PC_SHIFT                           (0)
#define CMU_CURRENT_ADDR_LR(n)                              (((n) & 0x1F) << 16)
#define CMU_CURRENT_ADDR_LR_MASK                            (0x1F << 16)
#define CMU_CURRENT_ADDR_LR_SHIFT                           (16)

#endif

