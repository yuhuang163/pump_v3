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
#include "spi_tester.h"

#define SPI_CMD(n)              {TO_STRING(n), CONCAT(spi_tester_, n)},
#define SPI_RX_TX_BITS          8

static uint32_t rate;
static uint8_t  mode;
static bool     cpol, cpha;

static void spi_tester_write(void)
{
    const uint8_t msg[] = {0xA1, 0xA3, 0xA5, 0xA7};

    CMD_TRACE("sending messages");
    int ret = hal_spi_send(msg, sizeof msg / sizeof msg[0]);
    ASSERT(!ret, "spi send error");
}

static void spi_tester_read(void)
{
    uint32_t rxbuf[] = {0};
    uint32_t buf[] = {0};

    int ret = hal_spi_recv(&rxbuf, buf, sizeof buf / sizeof buf[0]);
    CMD_TRACE("receving:\t\t0x%02X", buf[0]);
    ASSERT(!ret, "spi receive error");
}

static const spi_tester_handler_t spi_tester_handler[] = {
    SPI_CMD(write)
    SPI_CMD(read)
};

static void spi_tester_spi_init(void)
{
    /* spi port related attributions setting */
    struct HAL_SPI_CFG_T spiAttr = {
        .rate               = rate * 1000 * 1000,
        .clk_delay_half     = cpha,
        .clk_polarity       = cpol,
        .slave              = false,
        .dma_rx             = false,
        .dma_tx             = false,
        .rx_sep_line        = false,
        .ext_dss_rw         = false,
        .skip_en            = false,
        .cs                 = 0,
        .tx_bits            = SPI_RX_TX_BITS,
        .rx_bits            = SPI_RX_TX_BITS,
        .rx_frame_bits      = 0,
        .samp_delay         = 0,
    };

    /* spi port (spi0) initialization */
    hal_iomux_set_spi();
    int ret = hal_spi_open(&spiAttr);
    ASSERT(!ret, "spi port configuration error");
}

void spi_tester(const char **params)
{
    ASSIGN_VAL(rate, params[0]);
    ASSIGN_VAL(mode, params[1]);

    switch(mode) {
        case 0:  cpol = false; cpha = false; break;
        case 1:  cpol = false; cpha = true ; break;
        case 2:  cpol = true ; cpha = false; break;
        case 3:  cpol = true ; cpha = true ; break;
        default: PARAM_CHK(false);
    }

    spi_tester_spi_init();

    CMD_HANDLER(spi_tester, params[2]);
}