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
#ifndef __REG_QSPI_H__
#define __REG_QSPI_H__

#include "plat_types.h"

struct QSPI_T {
    __IO uint32_t SCTRLR0;            // 0x000
    __IO uint32_t SCTRLR1;            // 0x004
    __IO uint32_t SSIENR;             // 0x008
    __IO uint32_t MWCR;               // 0x00C
    __IO uint32_t SER;                // 0x010
    __IO uint32_t BAUDR;              // 0x014
    __IO uint32_t TXFTLR;             // 0x018
    __IO uint32_t RXFTLR;             // 0x01C
         uint32_t reserved_020[0x02]; // 0x020
    __IO uint32_t SR;                 // 0x028
         uint32_t reserved_02C[0x08]; // 0x02C
    __IO uint32_t DMACR;              // 0x04C
    __IO uint32_t DMATDLR;            // 0x050
    __IO uint32_t DMARDLR;            // 0x054
         uint32_t reserved_058[0x02]; // 0x058
    __IO uint32_t DR;                 // 0x060
         uint32_t reserved_064[0x24]; // 0x064
    __IO uint32_t SPI_CTRLR0;         // 0x0F4
};

// 0x000    SCTRLR0
#define QSPI_SCTRLR0_SPI_HYPERBUS_EN        (1 << 24)
#define QSPI_SCTRLR0_SPI_FRF_SHIFT          (22)
#define QSPI_SCTRLR0_SPI_FRF_MASK           (0x3 << QSPI_SCTRLR0_SPI_FRF_SHIFT)
#define QSPI_SCTRLR0_SPI_FRF(n)             (((n) & 0x3) << QSPI_SCTRLR0_SPI_FRF_SHIFT)
#define QSPI_SCTRLR0_CFS_SHIFT              (16)
#define QSPI_SCTRLR0_CFS_MASK               (0xF << QSPI_SCTRLR0_CFS_SHIFT)
#define QSPI_SCTRLR0_CFS(n)                 (((n) & 0xF) << QSPI_SCTRLR0_CFS_SHIFT)
#define QSPI_SCTRLR0_SSTE                   (1 << 14)
#define QSPI_SCTRLR0_SRL                    (1 << 13)
#define QSPI_SCTRLR0_SLV_OE                 (1 << 12)
#define QSPI_SCTRLR0_TMOD_SHIFT             (10)
#define QSPI_SCTRLR0_TMOD_MASK              (0x3 << QSPI_SCTRLR0_TMOD_SHIFT)
#define QSPI_SCTRLR0_TMOD(n)                (((n) & 0x3) << QSPI_SCTRLR0_TMOD_SHIFT)
#define QSPI_SCTRLR0_SCPOL                  (1 << 9)
#define QSPI_SCTRLR0_SCPH                   (1 << 8)
#define QSPI_SCTRLR0_FRF_SHIFT              (6)
#define QSPI_SCTRLR0_FRF_MASK               (0x3 << QSPI_SCTRLR0_FRF_SHIFT)
#define QSPI_SCTRLR0_FRF(n)                 (((n) & 0x3) << QSPI_SCTRLR0_FRF_SHIFT)
#define QSPI_SCTRLR0_DFS_SHIFT              (0)
#define QSPI_SCTRLR0_DFS_MASK               (0x1F << QSPI_SCTRLR0_DFS_SHIFT)
#define QSPI_SCTRLR0_DFS(n)                 (((n) & 0x1F) << QSPI_SCTRLR0_DFS_SHIFT)

// 0x004    SCTRLR1
#define QSPI_SCTRLR1_NDF_SHIFT              (0)
#define QSPI_SCTRLR1_NDF_MASK               (0xFFFF << QSPI_SCTRLR1_NDF_SHIFT)
#define QSPI_SCTRLR1_NDF(n)                 (((n) & 0xFFFF) << QSPI_SCTRLR1_NDF_SHIFT)

// 0x008    SSIENR
#define QSPI_SSIENR_SSIC_EN                 (1 << 0)

// 0x00C    MWCR
#define QSPI_MWCR_MHS                       (1 << 2)
#define QSPI_MWCR_MDD                       (1 << 1)
#define QSPI_MWCR_MWMOD                     (1 << 0)

// 0x010    SER (slave select enable flag)
#define QSPI_SER_SHIFT                      (0)
#define QSPI_SER_MASK                       (0xFFFFFFFF << QSPI_SER_SHIFT)
#define QSPI_SER(n)                         (((n) & 0xFFFFFFFF) << QSPI_SER_SHIFT)

// 0x014    BAUDR
#define QSPI_BAUDR_SCKDV_SHIFT              (1)
#define QSPI_BAUDR_SCKDV_MASK               (0x7F << QSPI_BAUDR_SCKDV_SHIFT)
#define QSPI_BAUDR_SCKDV(n)                 (((n) & 0x7F) << QSPI_BAUDR_SCKDV_SHIFT)

// 0x018    TXFTLR
#define QSPI_TXFTLR_SHIFT                   (0)
#define QSPI_TXFTLR_MASK                    (0xFFFFFFFF << QSPI_TXFTLR_SHIFT)
#define QSPI_TXFTLR(n)                      (((n) & 0xFFFFFFFF) << QSPI_TXFTLR_SHIFT)

// 0x01C    RXFTLR
#define QSPI_RXFTLR_SHIFT                   (0)
#define QSPI_RXFTLR_MASK                    (0xFFFFFFFF << QSPI_RXFTLR_SHIFT)
#define QSPI_RXFTLR(n)                      (((n) & 0xFFFFFFFF) << QSPI_RXFTLR_SHIFT)

// 0x028    SR
#define QSPI_SR_DATA_COLLISION_ERR          (1 << 6)
#define QSPI_SR_TRANS_ERR                   (1 << 5)
#define QSPI_SR_RECEIVE_FIFO_FULL           (1 << 4)
#define QSPI_SR_RECEIVE_FIFO_NOT_EMPTY      (1 << 3)
#define QSPI_SR_TRANS_FIFO_EMPTY            (1 << 2)
#define QSPI_SR_TRANS_FIFO_NOT_FULL         (1 << 1)
#define QSPI_SR_SSI_BUSY                    (1 << 0)

// 0x04C    DMACR
#define QSPI_SPI_DMACR_APROT_SHIFT          (12)
#define QSPI_SPI_DMACR_APROT_MASK           (0x7 << QSPI_SPI_DMACR_APROT_SHIFT)
#define QSPI_SPI_DMACR_APROT(n)             (((n) & 0x7) << QSPI_SPI_DMACR_APROT_SHIFT)
#define QSPI_SPI_DMACR_ACACHE_SHIFT         (8)
#define QSPI_SPI_DMACR_ACACHE_MASK          (0xF << QSPI_SPI_DMACR_ACACHE_SHIFT)
#define QSPI_SPI_DMACR_ACACHE(n)            (((n) & 0xF) << QSPI_SPI_DMACR_ACACHE_SHIFT)
#define QSPI_SPI_DMACR_AINC                 (1 << 6)
#define QSPI_SPI_DMACR_ATW_SHIFT            (3)
#define QSPI_SPI_DMACR_ATW_MASK             (0x3 << QSPI_SPI_DMACR_ATW_SHIFT)
#define QSPI_SPI_DMACR_ATW(n)               (((n) & 0x3) << QSPI_SPI_DMACR_ATW_SHIFT)
#define QSPI_SPI_DMACR_IDMAE                (1 << 2)
#define QSPI_SPI_DMACR_TDMAE                (1 << 1)
#define QSPI_SPI_DMACR_RDMAE                (1 << 0)

// 0x050    DMATDLR
#define QSPI_DMATDLR_SHIFT                  (0)
#define QSPI_DMATDLR_MASK                   (0xFFFFFFFF << QSPI_DMATDLR_SHIFT)
#define QSPI_DMATDLR(n)                     (((n) & 0xFFFFFFFF) << QSPI_DMATDLR_SHIFT)

// 0x054    DMARDLR
#define QSPI_DMARDLR_SHIFT                  (0)
#define QSPI_DMARDLR_MASK                   (0xFFFFFFFF << QSPI_DMARDLR_SHIFT)
#define QSPI_DMARDLR(n)                     (((n) & 0xFFFFFFFF) << QSPI_DMARDLR_SHIFT)

// 0x0F4    SPI_CTRLR0
#define QSPI_SPI_CTRLR0_CLK_STRETCH_EN      (1 << 30)
#define QSPI_SPI_CTRLR0_XIP_PREFETCH_EN     (1 << 29)
#define QSPI_SPI_CTRLR0_XIP_MBL_SHIFT       (26)
#define QSPI_SPI_CTRLR0_XIP_MBL_MASK        (0x3 << QSPI_SPI_CTRLR0_XIP_MBL_SHIFT)
#define QSPI_SPI_CTRLR0_XIP_MBL(n)          (((n) & 0x3) << QSPI_SPI_CTRLR0_XIP_MBL_SHIFT)
#define QSPI_SPI_CTRLR0_SPI_RXDS_SIG_EN     (1 << 25)
#define QSPI_SPI_CTRLR0_SPI_DM_EN           (1 << 24)
#define QSPI_SPI_CTRLR0_SSIC_XIP_CONT_XFER_EN   (1 << 21)
#define QSPI_SPI_CTRLR0_XIP_INST_EN         (1 << 20)
#define QSPI_SPI_CTRLR0_XIP_DFS_HC          (1 << 19)
#define QSPI_SPI_CTRLR0_SPI_RXDS_EN         (1 << 18)
#define QSPI_SPI_CTRLR0_INST_DDR_EN         (1 << 17)
#define QSPI_SPI_CTRLR0_SPI_DDR_EN          (1 << 16)
#define QSPI_SPI_CTRLR0_WAIT_CYCLES_SHIFT   (11)
#define QSPI_SPI_CTRLR0_WAIT_CYCLES_MASK    (0x1F << QSPI_SPI_CTRLR0_WAIT_CYCLES_SHIFT)
#define QSPI_SPI_CTRLR0_WAIT_CYCLES(n)      (((n) & 0x1F) << QSPI_SPI_CTRLR0_WAIT_CYCLES_SHIFT)
#define QSPI_SPI_CTRLR0_INST_L_SHIFT        (8)
#define QSPI_SPI_CTRLR0_INST_L_MASK         (0x3 << QSPI_SPI_CTRLR0_INST_L_SHIFT)
#define QSPI_SPI_CTRLR0_INST_L(n)           (((n) & 0x3) << QSPI_SPI_CTRLR0_INST_L_SHIFT)
#define QSPI_SPI_CTRLR0_XIP_MD_BIT_EN       (1 << 7)
#define QSPI_SPI_CTRLR0_ADDR_L_SHIFT        (2)
#define QSPI_SPI_CTRLR0_ADDR_L_MASK         (0xF << QSPI_SPI_CTRLR0_ADDR_L_SHIFT)
#define QSPI_SPI_CTRLR0_ADDR_L(n)           (((n) & 0xF) << QSPI_SPI_CTRLR0_ADDR_L_SHIFT)
#define QSPI_SPI_CTRLR0_TRANS_TYPE_SHIFT    (0)
#define QSPI_SPI_CTRLR0_TRANS_TYPE_MASK     (0x3 << QSPI_SPI_CTRLR0_TRANS_TYPE_SHIFT)
#define QSPI_SPI_CTRLR0_TRANS_TYPE(n)       (((n) & 0x3) << QSPI_SPI_CTRLR0_TRANS_TYPE_SHIFT)

#endif
