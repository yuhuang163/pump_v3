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
#include "hal_uart.h"

#ifdef UART0_BASE
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_trace.h"

//#define USED_UART_DMA                   (1)
#ifndef USED_UART_DMA
#define USED_UART_DIRECT                (1)
#endif

#if !((defined USED_UART_DIRECT) || (defined USED_UART_DMA))
#error "no either USED_UART_DIRECT or USED_UART_DMA."
#endif

#define UART_ID_DEFAULT                 (HAL_UART_ID_0)
#define UART_BAUD_DEFAULT               (1152000)

#define UART_TX_STRING_0                "THE TX STRING OF UART (NO DMA)...\r\n"
#define UART_TX_STRING_1                "THE TX STRING OF UART (BY DMA)...\r\n"

#if ((defined USED_UART_DIRECT) || (defined USED_UART_DMA))

int test_uart_iomux_init(void)
{
    if (0) {
    } else if (UART_ID_DEFAULT == HAL_UART_ID_0) {
        hal_iomux_set_uart0();
#ifdef UART1_BASE
    } else if (UART_ID_DEFAULT == HAL_UART_ID_1) {
        hal_iomux_set_uart1();
#endif
#ifdef UART2_BASE
    } else if (UART_ID_DEFAULT == HAL_UART_ID_2) {
        hal_iomux_set_uart2();
#endif
#ifdef UART3_BASE
    } else if (UART_ID_DEFAULT == HAL_UART_ID_3) {
        hal_iomux_set_uart3();
#endif
    } else {
        return -1;
    }

    return 0;
}

#endif

#if (defined USED_UART_DIRECT)

static int test_uart_direct(void)
{
    int ret = 0;
    int recv = 0;

	hal_trace_close();

    /* uart closed. */
    hal_uart_close(UART_ID_DEFAULT);

    /* IOMUX configuration. */
    test_uart_iomux_init();

    /* the direct mode. */
    struct HAL_UART_CFG_T uart_direct_cfg = {
        .parity             = HAL_UART_PARITY_NONE,
        .stop               = HAL_UART_STOP_BITS_1,
        .data               = HAL_UART_DATA_BITS_8,
        .flow               = HAL_UART_FLOW_CONTROL_NONE,
        .rx_level           = HAL_UART_FIFO_LEVEL_1_4,
        .tx_level           = HAL_UART_FIFO_LEVEL_1_2,
        .baud               = UART_BAUD_DEFAULT,
        .dma_rx             = false,
        .dma_tx             = false,
        .dma_rx_stop_on_err = false,                            /* ??? */
    };

    /* the uart is opened */
    ret = hal_uart_open(UART_ID_DEFAULT, &uart_direct_cfg);
    if (ret) {
        return ret;
    }

    /* send the string on 10 times. */
    char *pstr = "send the string on 10 times.\r\n";
    while (*pstr) {
        hal_uart_blocked_putc(UART_ID_DEFAULT, *pstr);
        pstr++;
    }

    for (uint8_t i=0; i<10; i++) {
        pstr = UART_TX_STRING_0;
        while (*pstr) {
            hal_uart_blocked_putc(UART_ID_DEFAULT, *pstr);
            pstr++;
        }
        osDelay(100);
    }

    /* loop to receive data of uart. */
    do {
        if (hal_uart_readable(UART_ID_DEFAULT)) {
            /* get the valid data. */
            recv = hal_uart_getc(UART_ID_DEFAULT);

            /* send the rx data. */
            hal_uart_blocked_putc(UART_ID_DEFAULT, recv);
        }
    } while(1);

    hal_uart_close(UART_ID_DEFAULT);
    return ret;
}

#endif /* #if (defined USED_UART_DIRECT) */

#if (defined USED_UART_DMA)

/* the rx process functions */
#define RX_BUF_MAX                      (256)

static uint8_t g_test_rx_buf[RX_BUF_MAX] = {0};
static bool g_tx_dma_done = 0;

static void test_uart_dma_tx_irq_handler(uint32_t xfer_size, int dma_error)
{
    if (dma_error) {
        /* TODO: */
    }

    g_tx_dma_done = true;
}

static void test_uart_dma_rx_irq_handler(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    if (dma_error) {
        /* TODO: */
    } else if (status.BE || status.FE || status.OE || status.PE) {
        /* TODO: */
    } else {                /* RT */
        /* user application after received the data. */
        if (g_tx_dma_done) {
            g_tx_dma_done = 0;
            hal_uart_dma_send(UART_ID_DEFAULT, g_test_rx_buf, xfer_size, NULL, NULL);
        }
        
        {
            union HAL_UART_IRQ_T mask;
        
            mask.reg = 0;
            mask.BE = 1;
            mask.FE = 1;
            mask.OE = 1;
            mask.PE = 0;
            mask.RT = 1;
            hal_uart_dma_recv_mask(UART_ID_DEFAULT, g_test_rx_buf, RX_BUF_MAX, NULL, NULL, &mask);
        }

    }
}

static int test_uart_dma(void)
{
    int ret = 0;

    /* uart closed. */
    hal_uart_close(UART_ID_DEFAULT);

    /* IOMUX configuration. */
    test_uart_iomux_init();

    /* the dma mode. */
    struct HAL_UART_CFG_T uart_dma_cfg = {
        .parity             = HAL_UART_PARITY_NONE,
        .stop               = HAL_UART_STOP_BITS_1,
        .data               = HAL_UART_DATA_BITS_8,
        .flow               = HAL_UART_FLOW_CONTROL_NONE,
        .rx_level           = HAL_UART_FIFO_LEVEL_1_4,
        .tx_level           = HAL_UART_FIFO_LEVEL_1_2,
        .baud               = UART_BAUD_DEFAULT,
        .dma_rx             = true,
        .dma_tx             = true,
        .dma_rx_stop_on_err = false,                            /* ??? */
    };

    /* the uart is opened */
    ret = hal_uart_open(UART_ID_DEFAULT, &uart_dma_cfg);
    if (ret) {
        return ret;
    }
    
    /* irq configuration. */
    hal_uart_irq_set_dma_handler(UART_ID_DEFAULT, test_uart_dma_rx_irq_handler, test_uart_dma_tx_irq_handler);
    {
        union HAL_UART_IRQ_T mask;

        mask.reg = 0;
        mask.BE = 1;
        mask.FE = 1;
        mask.OE = 1;
        mask.PE = 0;
        mask.RT = 1;
        ret = hal_uart_dma_recv_mask(UART_ID_DEFAULT, g_test_rx_buf, RX_BUF_MAX, NULL, NULL, &mask);
        if (ret) {
            return ret;
        }
    }

    /* send the string on 10 times. */
    hal_uart_dma_send(UART_ID_DEFAULT, (uint8_t *)"send the string on 10 times.\r\n", 
                            sizeof("send the string on 10 times.\r\n"), NULL, NULL);
    for (uint8_t i=0; i<10; i++) {
        hal_uart_dma_send(UART_ID_DEFAULT, (uint8_t *)UART_TX_STRING_1, sizeof(UART_TX_STRING_1), NULL, NULL);
        osDelay(100);
    }

    /* loop to receive the data by the interruption mode. */
    do {
        osDelay(1000);
    } while(1);

    /* close */
    union HAL_UART_IRQ_T mask;
    mask.reg = 0;
    hal_uart_stop_dma_recv(UART_ID_DEFAULT);
    hal_uart_irq_set_mask(UART_ID_DEFAULT, mask);
    ret = hal_uart_close(UART_ID_DEFAULT);

    return ret;
}

#endif /* #if (defined USED_UART_DMA) */

int test_uart(void)
{
    #if (defined USED_UART_DMA)
        return test_uart_dma();
    #elif (defined USED_UART_DIRECT)
        return test_uart_direct();
    #else
    #endif
}

#endif


