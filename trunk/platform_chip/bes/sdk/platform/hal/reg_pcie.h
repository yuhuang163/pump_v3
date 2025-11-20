/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __REG_PCIE_H__
#define __REG_PCIE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

struct PCIE_T {
    __IO uint32_t REG0;                  //0x00
    __IO uint32_t REG1;                  //0x04
    __IO uint32_t RESERVED[16];          //0x08^0x44
    __IO uint32_t CFG_BAR0;              //0x48
    __IO uint32_t CFG_BAR1;              //0x4C
    __IO uint32_t RESERVED1[4];          //0x50^0x5C
    __IO uint32_t STATUS;                //0x60
};

struct PCIE_APB_T {
    __IO uint32_t ID;                    //0x00
    __IO uint32_t RESERVED0[0x86];       //0x04^0x021C
    __IO uint32_t REG21C;                //0x021C
    __IO uint32_t RESERVED1[0x37C];      //0x0220^0x100c
    __IO uint32_t REG1010;               //0x1010
    __IO uint32_t GEN;                   //0x1014
    __IO uint32_t RESERVED2[0xB9];       //0x1018^0x12F8
    __IO uint32_t LTSSM_CTRL;            //0x12FC
};

// reg_000
#define PCIE_APB_VENDOR_ID(n)                               (((n) & 0xFFFF) << 0)
#define PCIE_APB_VENDOR_ID_MASK                             (0xFFFF << 0)
#define PCIE_APB_VENDOR_ID_SHIFT                            (0)
#define PCIE_APB_DEVICE_ID(n)                               (((n) & 0xFFFF) << 16)
#define PCIE_APB_DEVICE_ID_MASK                             (0xFFFF << 16)
#define PCIE_APB_DEVICE_ID_SHIFT                            (16)

// reg_12fc
#define PCIE_APB_EN_ACCESS                                  (1 << 12)

#endif
