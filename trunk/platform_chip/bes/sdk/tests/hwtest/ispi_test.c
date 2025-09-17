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
#include "hwtest.h"
#include "hal_spi.h"
#include "hal_dma.h"
#include "hal_cmu.h"

//#define SPI_3_WIRE

// ISPI_DMA_TEST
#define ISPI_DMA_REG_NUM                6
#define PMU_ROM_READ_CMD(r)             ((1 << 31) | (((r) & 0x7F) << 24))
#define PMU_ROM_WRITE_CMD(r, v)         ((((r) & 0x7F) << 24) | (((v) & 0xFFFF) << 8))
#define PMU_ROM_READ_VAL(v)             (((v) >> 8) & 0xFFFF)

static uint32_t ispi_dma_tx_buf[ISPI_DMA_REG_NUM];
static uint32_t ispi_dma_rx_cmd[ISPI_DMA_REG_NUM];
static uint32_t ispi_dma_rx_data[ISPI_DMA_REG_NUM];

static void ispi_write_read_check_regs(int seq)
{
    unsigned int i;
    int ret;

    ret = hal_ispi_dma_send(ispi_dma_tx_buf, sizeof(ispi_dma_tx_buf), NULL);
    ASSERT(ret == 0, "[%d] Ispi dma send failed: %d", seq, ret);

    ret = hal_ispi_dma_recv(ispi_dma_rx_cmd, ispi_dma_rx_data, sizeof(ispi_dma_rx_data), NULL);
    ASSERT(ret == 0, "[%d] Ispi dma recv failed: %d", seq, ret);

    for (i = 0; i < ARRAY_SIZE(ispi_dma_tx_buf); i++) {
        if (PMU_ROM_READ_VAL(ispi_dma_tx_buf[i]) != PMU_ROM_READ_VAL(ispi_dma_rx_data[i])) {
            ASSERT(false, "[%d] Ispi dma r/w not equal: i=%d, write=0x%04x, read=0x%04x",
                seq, i, PMU_ROM_READ_VAL(ispi_dma_tx_buf[i]), PMU_ROM_READ_VAL(ispi_dma_rx_data[i]));
        }
    }
}

void ispi_dma_test(void)
{
    unsigned int i;
    struct HAL_SPI_CFG_T spi_cfg;
    int seq = 0;
    int ret;

    spi_cfg.clk_delay_half = false;
    spi_cfg.clk_polarity = false;
    spi_cfg.slave = false;
    spi_cfg.rate = 1300000;
    spi_cfg.dma_rx = true;
    spi_cfg.dma_tx = true;
    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;
    spi_cfg.tx_bits = 32;
#ifdef SPI_3_WIRE
    spi_cfg.rx_bits = 24;
    spi_cfg.rx_frame_bits = 32;
#else
    spi_cfg.rx_bits = 32;
    spi_cfg.rx_frame_bits = 0;
#endif

    hal_ispi_open(&spi_cfg);

    // ana interface
    for (i = 0; i < ARRAY_SIZE(ispi_dma_tx_buf); i++) {
        ispi_dma_tx_buf[i] = PMU_ROM_WRITE_CMD(0x32 + i, (i + 1) | (i + 2) << 8);
        ispi_dma_rx_cmd[i] = PMU_ROM_READ_CMD(0x32 + i);
    }
    ispi_write_read_check_regs(seq++);

    // pmu interface
    for (i = 0; i < ARRAY_SIZE(ispi_dma_tx_buf); i++) {
        ispi_dma_tx_buf[i] = PMU_ROM_WRITE_CMD(0x72 + i, 0xabcd);
        ispi_dma_rx_cmd[i] = PMU_ROM_READ_CMD(0x72 + i);
    }
    ispi_write_read_check_regs(seq++);

    // rf interface
    for (i = 0; i < ARRAY_SIZE(ispi_dma_tx_buf); i++) {
        ispi_dma_tx_buf[i] = PMU_ROM_WRITE_CMD(0x20 + i, 0x1234);
        ispi_dma_rx_cmd[i] = PMU_ROM_READ_CMD(0x20 + i);
    }
    ispi_write_read_check_regs(seq++);

    // rf interface next page
    ispi_dma_tx_buf[0] = PMU_ROM_WRITE_CMD(0x40, 0x80);
    ret = hal_ispi_dma_send(ispi_dma_tx_buf, 4, NULL);
    ASSERT(ret == 0, "Ispi write 0x40 failed: %d", ret);
    for (i = 0; i < ARRAY_SIZE(ispi_dma_tx_buf); i++) {
        ispi_dma_tx_buf[i] = PMU_ROM_WRITE_CMD(0xe0 + i, 0x6789);
        ispi_dma_rx_cmd[i] = PMU_ROM_READ_CMD(0xe0 + i);
    }
    ispi_write_read_check_regs(seq++);

    ispi_dma_rx_cmd[0] = PMU_ROM_READ_CMD(0x5E);
    ret = hal_ispi_dma_recv(ispi_dma_rx_cmd, ispi_dma_rx_data, 4, NULL);
    ASSERT(ret == 0, "Ispi dma recv failed: %d", ret);
    TRACE(1,"Reg 0x5E val 0x%04x", PMU_ROM_READ_VAL(ispi_dma_rx_data[0]));

    TRACE(0,"ISPI dma test passed");
    hal_cmu_simu_pass();
}


