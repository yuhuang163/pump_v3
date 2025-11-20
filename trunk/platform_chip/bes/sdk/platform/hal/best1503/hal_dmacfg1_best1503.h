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
#ifndef __HAL_DMACFG1_BEST1503_H__
#define __HAL_DMACFG1_BEST1503_H__

#include "plat_addr_map.h"

#define AUDMA_PERIPH_NUM                        16
#define GPDMA_PERIPH_NUM                        16

#define AUDMA_CHAN_NUM                          8
#define GPDMA_CHAN_NUM                          8

#define AUDMA_CHAN_START                        (0)
#define GPDMA_CHAN_START                        (0)

#ifdef AUDMA_BASE
static const uint32_t audma_fifo_addr[AUDMA_PERIPH_NUM] = {
    SPILCD_BASE + 0x008,    // SPILCD RX
    SPILCD_BASE + 0x008,    // SPILCD TX
    I2C3_BASE + 0x010,      // I2C3 RX
    I2C3_BASE + 0x010,      // I2C3 TX
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const enum HAL_DMA_PERIPH_T audma_fifo_periph[AUDMA_PERIPH_NUM] = {
    HAL_AUDMA_SPILCD_RX,
    HAL_AUDMA_SPILCD_TX,
    HAL_AUDMA_I2C3_RX,
    HAL_AUDMA_I2C3_TX,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
};
#endif

#ifdef GPDMA_BASE
static const uint32_t gpdma_fifo_addr[GPDMA_PERIPH_NUM] = {
    SDMMC0_BASE + 0x200,    // SDMMC0
    JPEG_BASE + 0xC000,     // JPEG PIX BUFFER
    I2C0_BASE + 0x010,      // I2C0 RX
    I2C0_BASE + 0x010,      // I2C0 TX
    SPI_BASE + 0x008,       // SPI RX
    SPI_BASE + 0x008,       // SPI TX
    FLASH1_CTRL_BASE + 0x008,
    JPEG_BASE + 0x3000,     // JPEG ECS BUFFER
    UART0_BASE + 0x000,     // UART0 RX
    UART0_BASE + 0x000,     // UART0 TX
    UART1_BASE + 0x000,     // UART1 RX
    UART1_BASE + 0x000,     // UART1 TX
    SPIDPD_BASE + 0x008,    // SPIDPD RX
    SPIDPD_BASE + 0x008,    // SPIDPD TX
    UART2_BASE + 0x000,     // UART2 RX
    UART2_BASE + 0x000,     // UART2 TX
};

static const enum HAL_DMA_PERIPH_T gpdma_fifo_periph[GPDMA_PERIPH_NUM] = {
    HAL_GPDMA_SDMMC0,
    HAL_GPDMA_JPEG_PIX,
    HAL_GPDMA_I2C0_RX,
    HAL_GPDMA_I2C0_TX,
    HAL_GPDMA_SPI_RX,
    HAL_GPDMA_SPI_TX,
    HAL_GPDMA_FLASH1,
    HAL_GPDMA_JPEG_ECS,
    HAL_GPDMA_UART0_RX,
    HAL_GPDMA_UART0_TX,
    HAL_GPDMA_UART1_RX,
    HAL_GPDMA_UART1_TX,
    HAL_GPDMA_SPIDPD_RX,
    HAL_GPDMA_SPIDPD_TX,
    HAL_GPDMA_UART2_RX,
    HAL_GPDMA_UART2_TX,
};
#endif

#endif