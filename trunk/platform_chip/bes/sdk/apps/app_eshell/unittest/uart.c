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

#include "cmsis.h"
#include "cmsis_os.h"
#include "eshell.h"
#include "hal_iomux.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_uart.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#ifdef UTILS_ESHELL_EN

/* the rx process functions */
#define RX_BUF_MAX (256)

static uint8_t g_test_rx_buf[RX_BUF_MAX] = {0};
static bool g_tx_dma_done = 0;

static void test_uart_dma_tx_irq_handler(uint32_t xfer_size, int dma_error)
{
    if (dma_error) {
        /* TODO: */
    }
    eshell_putstring("test_uart_dma_rx_irq_handler tx size %d\r\n", xfer_size);
    g_tx_dma_done = true;
}

static void test_uart_dma_rx_irq_handler(uint32_t xfer_size, int dma_error,
                                         union HAL_UART_IRQ_T status)
{
    if (dma_error) {
        /* TODO: */
    } else if (status.BE || status.FE || status.OE || status.PE) {
        /* TODO: */
    } else { /* RT */
        eshell_putstring("test_uart_dma_rx_irq_handler rx size %d\r\n", xfer_size);
        {
            union HAL_UART_IRQ_T mask;
            mask.reg = 0;
            mask.BE = 1;
            mask.FE = 1;
            mask.OE = 1;
            mask.PE = 0;
            mask.RT = 1;
            hal_uart_dma_recv_mask(HAL_UART_ID_1, g_test_rx_buf, RX_BUF_MAX, NULL, NULL, &mask);
        }

        hal_uart_dma_send(HAL_UART_ID_1, g_test_rx_buf, xfer_size, NULL, NULL);
    }
}

static int test_uart_iomux_init(uint8_t uart_channel)
{

    hal_iomux_set_uart1();
    return 0;
}

static void test_uart(uint8_t uart_channel, uint32_t uart_baud)
{
    g_tx_dma_done = true;

    /* IOMUX configuration. */
    test_uart_iomux_init(uart_channel);

    /* the direct mode. */
    struct HAL_UART_CFG_T uart_direct_cfg = {
        .parity = HAL_UART_PARITY_NONE,
        .stop = HAL_UART_STOP_BITS_1,
        .data = HAL_UART_DATA_BITS_8,
        .flow = HAL_UART_FLOW_CONTROL_NONE,
        .rx_level = HAL_UART_FIFO_LEVEL_1_4,
        .tx_level = HAL_UART_FIFO_LEVEL_1_2,
        .baud = uart_baud,
        .dma_rx = true,
        .dma_tx = true,
        .dma_rx_stop_on_err = false, /* ??? */
    };

    /* the uart is opened */
    hal_uart_open(HAL_UART_ID_1, &uart_direct_cfg);

    /* irq configuration. */
    hal_uart_irq_set_dma_handler(HAL_UART_ID_1, test_uart_dma_rx_irq_handler,
                                 test_uart_dma_tx_irq_handler);
    {
        union HAL_UART_IRQ_T mask;

        mask.reg = 0;
        mask.BE = 1;
        mask.FE = 1;
        mask.OE = 1;
        mask.PE = 0;
        mask.RT = 1;
        hal_uart_dma_recv_mask(HAL_UART_ID_1, g_test_rx_buf, RX_BUF_MAX, NULL, NULL, &mask);
    }
}

static void unitest_uart(int argc, char *argv[])
{
    uint32_t uart_baud;
    if (argc != 2) {
        eshell_putstring("ERROR_ARG\r\n");
        return;
    }
    uart_baud = atoi(argv[1]);
    test_uart(1, uart_baud);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_uart", "utest_uart baud", unitest_uart);

#endif
