/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __REG_AONSEC_BEST1503_H__
#define __REG_AONSEC_BEST1503_H__

#include "plat_types.h"

struct AONSEC_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
};

// reg_00
#define AON_SEC_CFG_NONSEC_I2C_SLV                          (1 << 0)
#define AON_SEC_CFG_NONSEC_SENS                             (1 << 1)
#define AON_SEC_MCU_SECURE_DUMP                             (1 << 2)

// reg_04
#define AON_SEC_CFG_NONSEC_PROT_APB0(n)                     (((n) & 0xFFFFFFFF) << 0)
#define AON_SEC_CFG_NONSEC_PROT_APB0_MASK                   (0xFFFFFFFF << 0)
#define AON_SEC_CFG_NONSEC_PROT_APB0_SHIFT                  (0)

// reg_08
#define AON_SEC_CFG_NONSEC_BYPASS_PROT_APB0(n)              (((n) & 0xFFFFFFFF) << 0)
#define AON_SEC_CFG_NONSEC_BYPASS_PROT_APB0_MASK            (0xFFFFFFFF << 0)
#define AON_SEC_CFG_NONSEC_BYPASS_PROT_APB0_SHIFT           (0)

// reg_0c
#define AON_SEC_SECURE_BOOT_JTAG                            (1 << 0)
#define AON_SEC_SECURE_BOOT_I2C                             (1 << 1)
#define AON_SEC_MCU_CORE0_VTOR_SEL                          (1 << 2)
#define AON_SEC_MCU_CORE0_CFG_SECEXT                        (1 << 3)
#define AON_SEC_MCU_CORE0_CFG_DBGEN                         (1 << 4)
#define AON_SEC_MCU_CORE0_CFG_NIDEN                         (1 << 5)
#define AON_SEC_MCU_CORE0_CFG_SPIDEN                        (1 << 6)
#define AON_SEC_MCU_CORE0_CFG_SPNIDEN                       (1 << 7)
#define AON_SEC_MCU_CORE1_VTOR_SEL                          (1 << 8)
#define AON_SEC_MCU_CORE1_CFG_SECEXT                        (1 << 9)
#define AON_SEC_MCU_CORE1_CFG_DBGEN                         (1 << 10)
#define AON_SEC_MCU_CORE1_CFG_NIDEN                         (1 << 11)
#define AON_SEC_MCU_CORE1_CFG_SPIDEN                        (1 << 12)
#define AON_SEC_MCU_CORE1_CFG_SPNIDEN                       (1 << 13)
#define AON_SEC_MCU_CORE0_IDAUEN                            (1 << 14)
#define AON_SEC_MCU_CORE1_IDAUEN                            (1 << 15)
#define AON_SEC_MCU_IDAU_CALL_EN                            (1 << 16)

// reg_10
#define AON_SEC_MCU_CORE0_VTOR_SEC(n)                       (((n) & 0xFFFFFF) << 0)
#define AON_SEC_MCU_CORE0_VTOR_SEC_MASK                     (0xFFFFFF << 0)
#define AON_SEC_MCU_CORE0_VTOR_SEC_SHIFT                    (0)

// reg_14
#define AON_SEC_MCU_CORE1_VTOR_SEC(n)                       (((n) & 0xFFFFFF) << 0)
#define AON_SEC_MCU_CORE1_VTOR_SEC_MASK                     (0xFFFFFF << 0)
#define AON_SEC_MCU_CORE1_VTOR_SEC_SHIFT                    (0)

// reg_18
#define AON_SEC_MCU_IDAU_CALL_ADDR(n)                       (((n) & 0xFFFFFF) << 0)
#define AON_SEC_MCU_IDAU_CALL_ADDR_MASK                     (0xFFFFFF << 0)
#define AON_SEC_MCU_IDAU_CALL_ADDR_SHIFT                    (0)

#endif

