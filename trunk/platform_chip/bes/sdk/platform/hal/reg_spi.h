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
#ifndef __REG_SPI_H__
#define __REG_SPI_H__

#include "plat_types.h"

struct SPI_T
{
    __IO uint32_t SSPCR0;           // 0x000
    __IO uint32_t SSPCR1;           // 0x004
    __IO uint32_t SSPDR;            // 0x008
    __I  uint32_t SSPSR;            // 0x00C
    __IO uint32_t SSPCPSR;          // 0x010
    __IO uint32_t SSPIMSC;          // 0x014
    __I  uint32_t SSPRIS;           // 0x018
    __I  uint32_t SSPMIS;           // 0x01C
    __O  uint32_t SSPICR;           // 0x020
    __IO uint32_t SSPDMACR;         // 0x024
    __IO uint32_t SSPSRCNT;         // 0x028
    __IO uint32_t SSPDLYCR;         // 0x02C
    __IO uint32_t SSPRXONLY;        // 0x030
    __IO uint32_t SSPTXFL;          // 0x034
    __IO uint32_t SSPRXFL;          // 0x038
         uint32_t reserved[0x13];   // 0x03C
    __IO uint32_t SSPRXCR;          // 0x088
};

struct SAVED_SPI_REGS_T {
    uint32_t SSPCR0;
    uint16_t SSPCR1;
    uint16_t SSPCPSR;
    uint16_t SSPDMACR;
    uint16_t SSPRXCR;
#if (CHIP_SPI_VER >= 6)
    uint32_t SSPSRCNT;
    uint32_t SSPDLYCR;
#endif
};

#define MAX_SCR                             (0xFF)
#define MIN_SCR                             (0)
#define MAX_DATA_BITS                       (32)
#define MIN_DATA_BITS                       (4)

#define SPI_SSPCR0_DSS(n)                   (((n) & 0x1F) << 16)
#define SPI_SSPCR0_DSS_MASK                 (0x1F << 16)
#define SPI_SSPCR0_DSS_SHIFT                (16)
#define SPI_SSPCR0_SCR(n)                   (((n) & 0xFF) << 8)
#define SPI_SSPCR0_SCR_MASK                 (0xFF << 8)
#define SPI_SSPCR0_SCR_SHIFT                (8)
#define SPI_SSPCR0_SPH                      (1 << 7)
#define SPI_SSPCR0_SPO                      (1 << 6)
#define SPI_SSPCR0_SPO_SHIFT                (6)
#define SPI_SSPCR0_FRF(n)                   (((n) & 3) << 4)
#define SPI_SSPCR0_FRF_MASK                 (3 << 4)
#define SPI_SSPCR0_FRF_SHIFT                (4)
#if (CHIP_SPI_VER >= 8)
#define SPI_SSPCR0_RX_BYPASS                (1 << 3)
#endif

#if (CHIP_SPI_VER >= 7)
#define SPI_SSPCR1_TX_BIG_ENDIAN            (1 << 13)
#define SPI_SSPCR1_RX_BIG_ENDIAN            (1 << 12)
#endif
#if (CHIP_SPI_VER >= 4)
#define SPI_SSPCR1_RX_SEL_EN                (1 << 11)
#define SPI_SSPCR1_SLAVE_ID_SHIFT           (7)
#define SPI_SSPCR1_SLAVE_ID_MASK            (0xF << SPI_SSPCR1_SLAVE_ID_SHIFT)
#define SPI_SSPCR1_SLAVE_ID(n)              BITFIELD_VAL(SPI_SSPCR1_SLAVE_ID, n)
#else
#define SPI_SSPCR1_RX_SEL_EN                (1 << 9)
#define SPI_SSPCR1_SLAVE_ID_SHIFT           (7)
#define SPI_SSPCR1_SLAVE_ID_MASK            (3 << SPI_SSPCR1_SLAVE_ID_SHIFT)
#define SPI_SSPCR1_SLAVE_ID(n)              BITFIELD_VAL(SPI_SSPCR1_SLAVE_ID, n)
#endif
#if (CHIP_SPI_VER >= 7)
#define SPI_SSPCR1_SLAVE_ASYNC              (1 << 5)
#endif
#define SPI_SSPCR1_LCD_DC_DATA              (1 << 4)
#define SPI_SSPCR1_SOD                      (1 << 3)
#define SPI_SSPCR1_MS                       (1 << 2)
#define SPI_SSPCR1_SSE                      (1 << 1)
#define SPI_SSPCR1_LBM                      (1 << 0)

#define SPI_SSPSR_BSY                       (1 << 4)
#define SPI_SSPSR_RFF                       (1 << 3)
#define SPI_SSPSR_RNE                       (1 << 2)
#define SPI_SSPSR_TNF                       (1 << 1)
#define SPI_SSPSR_TFE                       (1 << 0)

#define MAX_CPSDVSR                         (0xFE)
#define MIN_CPSDVSR                         (2)

#define SPI_SSPCPSR_CPSDVSR(n)              (((n) & 0xFF) << 0)
#define SPI_SSPCPSR_CPSDVSR_MASK            (0xFF << 0)
#define SPI_SSPCPSR_CPSDVSR_SHIFT           (0)

#define SPI_SSPIMSC_TXIM                    (1 << 3)
#define SPI_SSPIMSC_RXIM                    (1 << 2)
#define SPI_SSPIMSC_RTIM                    (1 << 1)
#define SPI_SSPIMSC_RORIM                   (1 << 0)

#define SPI_SSPRIS_TXRIS                    (1 << 3)
#define SPI_SSPRIS_RXRIS                    (1 << 2)
#define SPI_SSPRIS_RTRIS                    (1 << 1)
#define SPI_SSPRIS_RORRIS                   (1 << 0)

#define SPI_SSPMIS_TXMIS                    (1 << 3)
#define SPI_SSPMIS_RXMIS                    (1 << 2)
#define SPI_SSPMIS_RTMIS                    (1 << 1)
#define SPI_SSPMIS_RORMIS                   (1 << 0)

#define SPI_SSPICR_RTIC                     (1 << 1)
#define SPI_SSPICR_RORIC                    (1 << 0)

#define SPI_SSPDMACR_TXDMAE                 (1 << 1)
#define SPI_SSPDMACR_RXDMAE                 (1 << 0)

#if (CHIP_SPI_VER >= 6)
#define SPI_SSPSRCNT_CNT_SHIFT              0
#define SPI_SSPSRCNT_CNT_MASK               (0xFFFF << SPI_SSPSRCNT_CNT_SHIFT)
#define SPI_SSPSRCNT_CNT(n)                 BITFIELD_VAL(SPI_SSPSRCNT_CNT, n)

#define SPI_SSPDLYCR_DLY_FINE_SHIFT         0
#define SPI_SSPDLYCR_DLY_FINE_MASK          (0xFFFF << SPI_SSPDLYCR_DLY_FINE_SHIFT)
#define SPI_SSPDLYCR_DLY_FINE(n)            BITFIELD_VAL(SPI_SSPDLYCR_DLY_FINE, n)
#define SPI_SSPDLYCR_DLY_COARSE_SHIFT       16
#define SPI_SSPDLYCR_DLY_COARSE_MASK        (0x3 << SPI_SSPDLYCR_DLY_COARSE_SHIFT)
#define SPI_SSPDLYCR_DLY_COARSE(n)          BITFIELD_VAL(SPI_SSPDLYCR_DLY_COARSE, n)
#define SPI_SSPDLYCR_DLY_SAMP_EN            (1 << 18)
#define SPI_SSPDLYCR_EXTEND_CS_EN           (1 << 19)
#endif

#if (CHIP_SPI_VER >= 7)
#define SPI_SSPRXONLY_RXONLY                (1 << 31)
#define SPI_SSPRXONLY_RXCNT_SHIFT           0
#define SPI_SSPRXONLY_RXCNT_MASK            (0x7FFFFFFF << SPI_SSPRXONLY_RXCNT_SHIFT)
#define SPI_SSPRXONLY_RXCNT(n)              BITFIELD_VAL(SPI_SSPRXONLY_RXCNT, n)

#define SPI_SSPTXFL_SSPTXFL_SHIFT           0
#define SPI_SSPTXFL_SSPTXFL_MASK            (0x3F << SPI_SSPTXFL_SSPTXFL_SHIFT)
#define SPI_SSPTXFL_SSPTXFL(n)              BITFIELD_VAL(SPI_SSPTXFL_SSPTXFL, n)

#define SPI_SSPRXFL_SSPRXFL_SHIFT           0
#define SPI_SSPRXFL_SSPRXFL_MASK            (0x3F << SPI_SSPRXFL_SSPRXFL_SHIFT)
#define SPI_SSPRXFL_SSPRXFL(n)              BITFIELD_VAL(SPI_SSPRXFL_SSPRXFL, n)
#endif

#if (CHIP_SPI_VER >= 6)
#define SPI_SSPRXCR_CMD_CTRL                (1 << 7)
#endif
#define SPI_SSPRXCR_EN                      (1 << 6)
#define SPI_SSPRXCR_OEN_POLARITY            (1 << 5)
#define SPI_SSPRXCR_RXBITS(n)               (((n) & 0x1F) << 0)
#define SPI_SSPRXCR_RXBITS_MASK             (0x1F << 0)
#define SPI_SSPRXCR_RXBITS_SHIFT            (0)

#if (CHIP_SPI_VER >= 7)
#define SPI_TX_FIFO_DEPTH                   32
#define SPI_RX_FIFO_DEPTH                   32
#else
#define SPI_TX_FIFO_DEPTH                   8
#define SPI_RX_FIFO_DEPTH                   8
#endif

#endif
